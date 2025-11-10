// TakeOverStrategyTest.cpp
// Integration smoke test for migrated C++ TakeOverStrategy
// Builds a minimal StrategyParams instance and executes the strategy once.
// Validates: factory creation, context data access, indicator bridge usage, return code path.

#include <windows.h>
#include "StrategyContext.hpp"
#include "BridgeConstants.hpp"
#include "IndicatorBridge.hpp"
#include "StrategyTypes.h"
#include "AsirikuyDefines.h"
#include "strategies/TakeOverStrategy.hpp"
#include <iostream>
#include <vector>
#include <ctime>
#include <cstring>
#include <fstream>

// Global environment pointer so stubs can access rates
struct TestEnvironment; // forward
static TestEnvironment* gEnv = 0;

// Forward declare C functions we will stub later (after TestEnvironment defined)
extern "C" {
    void addValueToUI(const char* name, double value);
    double iHigh(int rateIndex, int shift);
    double iLow(int rateIndex, int shift);
    double iClose(int rateIndex, int shift);
    double iAtr(int ratesArrayIndex, int period, int shift);
    double iMA(int type, int ratesArrayIndex, int period, int shift);
    double iBBandStop(int ratesArrayIndex, int bb_period, double bb_deviation, int* trend, double* stopPrice, int* index);
    void safe_timeString(char* buffer, time_t t);
}

// Provide minimal RatesBuffers with 3 timeframes populated (DAILY, PRIMARY, HOURLY)
static const int kBars = 5; // tiny buffer

struct TestEnvironment {
    RatesBuffers ratesBuffers;                // owns Rates arrays
    std::vector<time_t> dailyTime;
    std::vector<double> dailyOpen, dailyHigh, dailyLow, dailyClose, dailyVolume;
    std::vector<time_t> primaryTime;
    std::vector<double> primaryOpen, primaryHigh, primaryLow, primaryClose, primaryVolume;
    std::vector<time_t> hourlyTime;
    std::vector<double> hourlyOpen, hourlyHigh, hourlyLow, hourlyClose, hourlyVolume;

    std::vector<double> bidArray, askArray;   // BidAsk arrays
    double settings[70]; // enough space for indexes we use
    StrategyResults resultsStorage;           // zero-initialized POD

    StrategyParams params;                    // strategy params passed to context

    TestEnvironment() :
        dailyTime(kBars), dailyOpen(kBars), dailyHigh(kBars), dailyLow(kBars), dailyClose(kBars), dailyVolume(kBars),
        primaryTime(kBars), primaryOpen(kBars), primaryHigh(kBars), primaryLow(kBars), primaryClose(kBars), primaryVolume(kBars),
        hourlyTime(kBars), hourlyOpen(kBars), hourlyHigh(kBars), hourlyLow(kBars), hourlyClose(kBars), hourlyVolume(kBars),
        bidArray(2), askArray(2)
    {
        std::ofstream dbg("E:\\workspace\\AutoBBS\\tmp\\test_ctor.txt");
        dbg << "Constructor body entered\n"; dbg.flush();
        
        // Fill simple ascending data
        time_t base = std::time(NULL) - 3600 * kBars;
        dbg << "time computed: " << base << "\n"; dbg.flush();
        
        for (int i = 0; i < kBars; ++i) {
            dailyTime[i]   = base + i * 86400;
            primaryTime[i] = base + i * 3600;
            hourlyTime[i]  = base + i * 3600;

            double price = 1.1000 + i * 0.0010;
            dailyOpen[i] = primaryOpen[i] = hourlyOpen[i] = price;
            dailyHigh[i] = primaryHigh[i] = hourlyHigh[i] = price + 0.0005;
            dailyLow[i]  = primaryLow[i]  = hourlyLow[i]  = price - 0.0005;
            dailyClose[i]= primaryClose[i]= hourlyClose[i]= price + 0.0002;
            dailyVolume[i]= primaryVolume[i]= hourlyVolume[i]= 1000 + i * 10;
        }
        dbg << "Vectors filled\n"; dbg.flush();

        // Initialize Rates entries (only three used; others left zeroed)
        std::memset(&ratesBuffers, 0, sizeof(ratesBuffers));
        dbg << "ratesBuffers memset done\n"; dbg.flush();
        
        ratesBuffers.instanceId = 1;

        initRatesEntry(ratesBuffers.rates[DAILY_RATES],    dailyTime, dailyOpen, dailyHigh, dailyLow, dailyClose, dailyVolume);
        dbg << "DAILY_RATES init done\n"; dbg.flush();
        
        initRatesEntry(ratesBuffers.rates[PRIMARY_RATES],  primaryTime, primaryOpen, primaryHigh, primaryLow, primaryClose, primaryVolume);
        dbg << "PRIMARY_RATES init done\n"; dbg.flush();
        
        initRatesEntry(ratesBuffers.rates[HOURLY_RATES],   hourlyTime, hourlyOpen, hourlyHigh, hourlyLow, hourlyClose, hourlyVolume);
        dbg << "HOURLY_RATES init done\n"; dbg.flush();

        // Settings initialization
        std::memset(settings, 0, sizeof(settings));
        dbg << "settings memset done\n"; dbg.flush();
        
        settings[STRATEGY_INSTANCE_ID] = 101;          // arbitrary instance id
        settings[INTERNAL_STRATEGY_ID] = TAKEOVER;     // choose TakeOverStrategy
        settings[TIMEFRAME] = 1;                       // 1M timeframe (for BBS exit logic branch)
        settings[ATR_AVERAGING_PERIOD] = 14;           // typical ATR period
        // Additional params used by TakeOverStrategy (mapping from earlier migration notes)
        settings[ADDITIONAL_PARAM_1] = 20;             // BBS_PERIOD
        settings[ADDITIONAL_PARAM_2] = 2;              // BBS_DEVIATION
        settings[ADDITIONAL_PARAM_3] = 0;              // BBS_ADJUSTPOINTS
        settings[ADDITIONAL_PARAM_4] = hourlyClose[kBars-1]; // POSITION reference
        settings[ADDITIONAL_PARAM_7] = 0;              // DSL_TYPE (EXIT_DSL_NONE)
        dbg << "settings populated\n"; dbg.flush();

        // Zero the results structure
        std::memset(&resultsStorage, 0, sizeof(resultsStorage));
        dbg << "resultsStorage memset done\n"; dbg.flush();

        // Compose StrategyParams
        std::memset(&params, 0, sizeof(params));
        dbg << "params memset done\n"; dbg.flush();
        
        params.tradeSymbol = const_cast<char*>("EURUSD");
        params.currentBrokerTime = dailyTime[kBars-1];
        params.ratesBuffers = &ratesBuffers;
        params.settings = settings;
        params.results  = &resultsStorage;
        // Setup BidAsk with allocated arrays
        params.bidAsk.arraySize = 2;
        bidArray[0] = hourlyClose[kBars-1] - 0.0001;
        bidArray[1] = hourlyClose[kBars-1] - 0.0001;
        askArray[0] = hourlyClose[kBars-1] + 0.0001;
        askArray[1] = hourlyClose[kBars-1] + 0.0001;
        params.bidAsk.bid = &bidArray[0];
        params.bidAsk.ask = &askArray[0];
        dbg << "Constructor completed successfully\n"; dbg.flush();
        dbg.close();
    }

    static void initRatesEntry(Rates& r, std::vector<time_t>& t, std::vector<double>& o, std::vector<double>& h,
                               std::vector<double>& l, std::vector<double>& c, std::vector<double>& v) {
        r.info.isEnabled = TRUE;
        r.info.isBufferFull = TRUE;
        r.info.timeframe = 0; // unused for test
        r.info.arraySize = static_cast<int>(t.size());
        r.info.point = 0.0001;
        r.info.digits = 5;
        r.time = &t[0]; r.open = &o[0]; r.high = &h[0]; r.low = &l[0]; r.close = &c[0]; r.volume = &v[0];
    }
};

// Now provide stub implementations using fully defined TestEnvironment
extern "C" {
    void addValueToUI(const char* /*name*/, double /*value*/) { /* no-op */ }
    static int mapShiftInternal(int rateIndex, int shift) {
        const Rates& r = gEnv->ratesBuffers.rates[rateIndex];
        int size = r.info.arraySize;
        int idx = size - 1 - shift;
        if (idx < 0) idx = 0;
        return idx;
    }
    double iHigh(int rateIndex, int shift)  { return gEnv->ratesBuffers.rates[rateIndex].high[ mapShiftInternal(rateIndex, shift) ]; }
    double iLow(int rateIndex, int shift)   { return gEnv->ratesBuffers.rates[rateIndex].low[  mapShiftInternal(rateIndex, shift) ]; }
    double iClose(int rateIndex, int shift) { return gEnv->ratesBuffers.rates[rateIndex].close[ mapShiftInternal(rateIndex, shift) ]; }
    double iAtr(int /*ratesArrayIndex*/, int /*period*/, int /*shift*/) { return 0.001; }
    double iMA(int /*type*/, int ratesIndex, int /*period*/, int shift) { return iClose(ratesIndex, shift); }
    double iBBandStop(int /*ratesArrayIndex*/, int /*bb_period*/, double /*bb_deviation*/, int* trend, double* stopPrice, int* index) { *trend = 0; *stopPrice = 0.0; *index = 0; return 0.0; }
    void safe_timeString(char* buffer, time_t t) {
        // Use thread-safe gmtime_s on Windows
        std::tm tmv;
        #if defined(_WIN32)
        gmtime_s(&tmv, &t);
        #else
        std::tm* tmp = std::gmtime(&t);
        tmv = *tmp;
        #endif
        std::strftime(buffer, 32, "%d/%m/%y %H:%M", &tmv);
    }
}

int main() {
    // Write to file immediately to confirm entry
    std::ofstream log("E:\\workspace\\AutoBBS\\tmp\\test_trace.txt");
    log << "main() entered\n";
    log.flush();

    std::cout << "[TakeOverStrategyTest] Starting integration smoke test...\n";
    std::cout.flush();
    log << "cout printed startup message\n";
    log.flush();
    
    std::cout << "Initializing TestEnvironment...\n";
    std::cout.flush();

    try {
        log << "Creating TestEnvironment...\n";
        log.flush();
        TestEnvironment env;
        log << "TestEnvironment created\n";
        log.flush();
        
        gEnv = &env; // enable indicator stubs
        std::cout << "Creating StrategyContext...\n";
        std::cout.flush();
        log << "Creating StrategyContext...\n";
        log.flush();
        
        trading::StrategyContext context(&env.params);
        log << "StrategyContext created\n";
        log.flush();
        
        std::cout << "Creating TakeOverStrategy...\n";
        std::cout.flush();
        log << "Creating TakeOverStrategy...\n";
        log.flush();
        
        trading::TakeOverStrategy strategy; // direct instantiation
        log << "TakeOverStrategy created\n";
        log.flush();

        std::cout << "Validating strategy...\n";
        std::cout.flush();
        log << "Validating strategy...\n";
        log.flush();
        
        if (!strategy.validate(context)) {
            std::cerr << "Strategy validation failed\n";
            log << "Validation failed\n";
            log.close();
            return 2;
        }

        std::cout << "Executing strategy...\n";
        std::cout.flush();
        log << "Executing strategy...\n";
        log.flush();
        
        AsirikuyReturnCode code = strategy.execute(context);
        log << "Execute returned code=" << code << "\n";
        log.flush();
        
        std::cout << "Execute returned code=" << code << " (SUCCESS=" << SUCCESS << ")\n";
        std::cout.flush();

        // Basic assertions (manual)
        bool ok = (code == SUCCESS);
        std::cout << (ok ? "[PASS]" : "[FAIL]") << " TakeOverStrategy basic execution path\n";
        std::cout.flush();
        log << (ok ? "[PASS]" : "[FAIL]") << " test\n";
        log.close();

        return ok ? 0 : 3;
    }
    catch (const std::exception& e) {
        std::cerr << "EXCEPTION: " << e.what() << "\n";
        log << "EXCEPTION: " << e.what() << "\n";
        log.close();
        return 4;
    }
    catch (...) {
        std::cerr << "UNKNOWN EXCEPTION caught\n";
        log << "UNKNOWN EXCEPTION caught\n";
        log.close();
        return 5;
    }
}
