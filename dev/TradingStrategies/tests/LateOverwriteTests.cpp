#include "Precompiled.h"
#include "StrategyUserInterface.h"
#include "Logging.h"
#include <cmath>

/* -------------------------------------------------------------------------
 * Late Overwrite Block Tests
 * -------------------------------------------------------------------------
 * Validates that the two-phase UI emission pattern works correctly:
 * 1. Early strategy code adds preliminary values
 * 2. Late overwrite block updates mutable risk/volatility metrics
 * 3. Final flush emits accurate end-of-tick snapshot
 *
 * Mock risk calculators simulate the late refresh behavior.
 * ------------------------------------------------------------------------- */

// Mock risk calculation functions (simulate actual framework helpers)
static double g_mockStrategyRisk = 0.0;
static double g_mockRiskPNL = 0.0;
static double g_mockDailyATR = 0.0;

double mockCalculateStrategyRisk(BOOL includeLockedProfit) {
    return includeLockedProfit ? g_mockStrategyRisk : g_mockStrategyRisk * 0.8;
}

double mockCalculateStrategyPNL(BOOL includeLockedProfit) {
    return includeLockedProfit ? g_mockRiskPNL : g_mockRiskPNL * 0.9;
}

double mockCalculateDailyATR() {
    return g_mockDailyATR;
}

static void resetUIBuffer() {
    flushUserInterfaceValues(999, TRUE); // Backtest mode prevents file I/O
}

static bool assertValue(const char* name, double expected, const char* testName) {
    BOOL found = FALSE;
    double actual = getUIValue(name, &found);
    if (!found) {
        pantheios_logprintf(PANTHEIOS_SEV_CRITICAL,
            (PAN_CHAR_T*)"[%s] FAILED: '%s' not found in UI buffer", testName, name);
        return false;
    }
    if (fabs(actual - expected) > 1e-9) {
        pantheios_logprintf(PANTHEIOS_SEV_CRITICAL,
            (PAN_CHAR_T*)"[%s] FAILED: '%s' = %lf, expected %lf", testName, name, actual, expected);
        return false;
    }
    return true;
}

// Test 1: Early add, late overwrite updates correctly
bool testEarlyAddLateOverwrite() {
    resetUIBuffer();
    
    // Simulate early strategy code adding preliminary value
    addValueToUI("strategyRisk", 100.0);
    
    // Verify early value
    if (!assertValue("strategyRisk", 100.0, "EarlyAdd")) {
        return false;
    }
    
    // Simulate late overwrite (risk recalculated after order management)
    g_mockStrategyRisk = 150.0;
    double updatedRisk = mockCalculateStrategyRisk(FALSE);
    updateOrAddValueToUI("strategyRisk", updatedRisk);
    
    // Verify overwrite succeeded
    if (!assertValue("strategyRisk", 120.0, "LateOverwrite")) { // 150 * 0.8 = 120
        return false;
    }
    
    pantheios_logputs(PANTHEIOS_SEV_INFORMATIONAL,
        (PAN_CHAR_T*)"[EarlyAddLateOverwrite] PASSED");
    return true;
}

// Test 2: Multiple overwrites of same field retain last value
bool testMultipleOverwrites() {
    resetUIBuffer();
    
    addValueToUI("riskPNL", 50.0);
    updateOrAddValueToUI("riskPNL", 75.0);
    updateOrAddValueToUI("riskPNL", 100.0);
    
    if (!assertValue("riskPNL", 100.0, "MultipleOverwrites")) {
        return false;
    }
    
    pantheios_logputs(PANTHEIOS_SEV_INFORMATIONAL,
        (PAN_CHAR_T*)"[MultipleOverwrites] PASSED");
    return true;
}

// Test 3: Late overwrite adds new field if not present
bool testLateOverwriteAppendsIfAbsent() {
    resetUIBuffer();
    
    addValueToUI("entryPrice", 1.2000);
    
    // Attempt to overwrite field that was never added
    g_mockDailyATR = 65.5;
    updateOrAddValueToUI("dailyATR", g_mockDailyATR);
    
    // Should append (not fail)
    if (!assertValue("dailyATR", 65.5, "LateAppend")) {
        return false;
    }
    if (!assertValue("entryPrice", 1.2000, "LateAppend")) {
        return false;
    }
    
    pantheios_logputs(PANTHEIOS_SEV_INFORMATIONAL,
        (PAN_CHAR_T*)"[LateOverwriteAppendsIfAbsent] PASSED");
    return true;
}

// Test 4: Buffer capacity audit - verify current field count fits
bool testBufferCapacityAudit() {
    resetUIBuffer();
    
    // Simulate full telemetry harvesting + late overwrite
    // Based on harvestStrategyTelemetry (13 fields) + late overwrite (13 fields)
    
    // Harvest phase
    addValueToUI("executionTrend", 1.0);
    addValueToUI("entrySignal", 1.0);
    addValueToUI("exitSignal", 2.0);
    addValueToUI("risk", 0.5);
    addValueToUI("splitTradeMode", 19.0);
    addValueToUI("tpMode", 3.0);
    addValueToUI("entryPrice", 2100.5);
    addValueToUI("stopLossPrice", 2080.5);
    addValueToUI("atr_euro_range", 45.0);
    addValueToUI("pWeeklyPredictATR", 120.0);
    addValueToUI("pWeeklyPredictMaxATR", 180.0);
    addValueToUI("pDailyMaxATR", 65.0);
    addValueToUI("strategyInstanceId", 1234.0);
    
    // Late overwrite phase
    updateOrAddValueToUI("strategyRisk", 100.0);
    updateOrAddValueToUI("strategyRiskNLP", 90.0);
    updateOrAddValueToUI("riskPNL", 110.0);
    updateOrAddValueToUI("riskPNLNLP", 95.0);
    updateOrAddValueToUI("StrategyVolRisk", 10.0);
    updateOrAddValueToUI("weeklyATR", 125.0);
    updateOrAddValueToUI("weeklyMaxATR", 185.0);
    updateOrAddValueToUI("dailyATR", 60.0);
    updateOrAddValueToUI("strategyMarketVolRisk", 15.0);
    updateOrAddValueToUI("strategyMarketVolRiskNoTP", 12.0);
    updateOrAddValueToUI("AccountRisk", 2.5);
    updateOrAddValueToUI("pWeeklyPredictATR", 130.0); // Overwrite
    updateOrAddValueToUI("pDailyPredictATR", 62.0);
    
    int finalCount = getUICount();
    
    // Expected: 13 harvest + 11 unique late fields (2 overwrites) = 24 unique
    // But some are overwrites, actual unique count:
    // Harvest: 13 fields
    // Late unique adds: 11 new fields (pWeeklyPredictATR & pDailyPredictATR may be new or overwrite)
    // Total unique: ~22-24 fields
    
    if (finalCount > 20) {
        pantheios_logprintf(PANTHEIOS_SEV_CRITICAL,
            (PAN_CHAR_T*)"[BufferCapacityAudit] WARNING: Buffer count %d exceeds TOTAL_UI_VALUES=20. "
            "Consider expanding buffer or reducing telemetry fields.", finalCount);
        return false;
    }
    
    pantheios_logprintf(PANTHEIOS_SEV_INFORMATIONAL,
        (PAN_CHAR_T*)"[BufferCapacityAudit] PASSED - Current usage: %d/20 fields", finalCount);
    return true;
}

// Test 5: Overwrite semantics - derived fields updated correctly
bool testDerivedFieldOverwrite() {
    resetUIBuffer();
    
    // Simulate initial risk values
    addValueToUI("strategyRisk", 100.0);
    addValueToUI("riskPNL", 150.0);
    
    // Compute derived field early
    double volRisk = 150.0 - 100.0; // riskPNL - strategyRisk
    addValueToUI("StrategyVolRisk", volRisk);
    
    if (!assertValue("StrategyVolRisk", 50.0, "DerivedEarly")) {
        return false;
    }
    
    // Late recalculation changes base values
    g_mockStrategyRisk = 120.0;
    g_mockRiskPNL = 180.0;
    
    updateOrAddValueToUI("strategyRisk", g_mockStrategyRisk);
    updateOrAddValueToUI("riskPNL", g_mockRiskPNL);
    
    // Update derived field
    double updatedVolRisk = g_mockRiskPNL - g_mockStrategyRisk;
    updateOrAddValueToUI("StrategyVolRisk", updatedVolRisk);
    
    // Verify all updated correctly
    if (!assertValue("strategyRisk", 120.0, "DerivedLate")) {
        return false;
    }
    if (!assertValue("riskPNL", 180.0, "DerivedLate")) {
        return false;
    }
    if (!assertValue("StrategyVolRisk", 60.0, "DerivedLate")) {
        return false;
    }
    
    pantheios_logputs(PANTHEIOS_SEV_INFORMATIONAL,
        (PAN_CHAR_T*)"[DerivedFieldOverwrite] PASSED");
    return true;
}

int main() {
    bool allPassed = true;
    
    pantheios_logputs(PANTHEIOS_SEV_NOTICE,
        (PAN_CHAR_T*)"=== Late Overwrite Block Tests ===");
    
    allPassed &= testEarlyAddLateOverwrite();
    allPassed &= testMultipleOverwrites();
    allPassed &= testLateOverwriteAppendsIfAbsent();
    allPassed &= testBufferCapacityAudit();
    allPassed &= testDerivedFieldOverwrite();
    
    if (allPassed) {
        pantheios_logputs(PANTHEIOS_SEV_NOTICE,
            (PAN_CHAR_T*)"=== ALL LATE OVERWRITE TESTS PASSED ===");
        return 0;
    } else {
        pantheios_logputs(PANTHEIOS_SEV_CRITICAL,
            (PAN_CHAR_T*)"=== SOME LATE OVERWRITE TESTS FAILED ===");
        return 1;
    }
}
