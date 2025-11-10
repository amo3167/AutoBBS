#include "Precompiled.h"
#include "StrategyUserInterface.h"
#include "Logging.h"

/* Minimal stubs for required structs to exercise harvestStrategyTelemetry.
 * In the full codebase these are defined elsewhere; for this isolated unit
 * test we only declare the fields we touch. Adjust includes if full type
 * definitions become available during migration.
 */

struct StrategyParamsStub {
    double settings[128];
};

struct IndicatorsStub {
    int    executionTrend;
    int    entrySignal;
    int    exitSignal;
    double risk;
    int    splitTradeMode;
    int    tpMode;
    double entryPrice;
    double stopLossPrice;
    double atr_euro_range;
};

struct BaseIndicatorsStub {
    double pWeeklyPredictATR;
    double pWeeklyPredictMaxATR;
    double pDailyMaxATR;
};

/* Forward declare harvestStrategyTelemetry with stub types mapped to real
 * names via typedef to satisfy linker when using the C implementation. */
extern "C" void harvestStrategyTelemetry(struct StrategyParams* pParams,
                                          struct Indicators* pIndicators,
                                          struct Base_Indicators* pBase);

/* Map stub structs to expected names (only layout matters here). */
typedef StrategyParamsStub StrategyParams;
typedef IndicatorsStub Indicators;
typedef BaseIndicatorsStub Base_Indicators;

static void resetUIBuffer() {
    // Force a flush with dummy instance to clear buffer if anything remained.
    flushUserInterfaceValues(999, TRUE); // isBackTesting TRUE prevents file IO.
}

static bool assertEqual(const char* name, double expected) {
    BOOL found = FALSE;
    double v = getUIValue(name, &found);
    if (!found) {
        pantheios_logprintf(PANTHEIOS_SEV_CRITICAL, (PAN_CHAR_T*)"Test FAILED: '%s' not found", name);
        return false;
    }
    if (fabs(v - expected) > 1e-9) {
        pantheios_logprintf(PANTHEIOS_SEV_CRITICAL, (PAN_CHAR_T*)"Test FAILED: '%s' value %lf != expected %lf", name, v, expected);
        return false;
    }
    return true;
}

int main() {
    resetUIBuffer();

    StrategyParams params{};
    params.settings[STRATEGY_INSTANCE_ID] = 1234; // make sure instance id surfaces

    Indicators ind{};
    ind.executionTrend = 1;
    ind.entrySignal = 1;
    ind.exitSignal = 2;
    ind.risk = 0.75;
    ind.splitTradeMode = 19;
    ind.tpMode = 3;
    ind.entryPrice = 2100.5;
    ind.stopLossPrice = 2080.5;
    ind.atr_euro_range = 45.0;

    Base_Indicators base{};
    base.pWeeklyPredictATR = 120.0;
    base.pWeeklyPredictMaxATR = 180.0;
    base.pDailyMaxATR = 65.0;

    harvestStrategyTelemetry(&params, &ind, &base);

    // Assertions
    bool ok = true;
    ok &= assertEqual("executionTrend", 1.0);
    ok &= assertEqual("entrySignal", 1.0);
    ok &= assertEqual("exitSignal", 2.0);
    ok &= assertEqual("risk", 0.75);
    ok &= assertEqual("splitTradeMode", 19.0);
    ok &= assertEqual("tpMode", 3.0);
    ok &= assertEqual("entryPrice", 2100.5);
    ok &= assertEqual("stopLossPrice", 2080.5);
    ok &= assertEqual("atr_euro_range", 45.0);
    ok &= assertEqual("pWeeklyPredictATR", 120.0);
    ok &= assertEqual("pWeeklyPredictMaxATR", 180.0);
    ok &= assertEqual("pDailyMaxATR", 65.0);
    ok &= assertEqual("strategyInstanceId", 1234.0);

    if (!ok) {
        pantheios_logputs(PANTHEIOS_SEV_CRITICAL, (PAN_CHAR_T*)"TrendStrategyTelemetryTests FAILED");
        return 1;
    }

    pantheios_logputs(PANTHEIOS_SEV_INFORMATIONAL, (PAN_CHAR_T*)"TrendStrategyTelemetryTests PASSED");
    return 0;
}
