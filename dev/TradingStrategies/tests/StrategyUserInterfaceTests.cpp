#define BOOST_TEST_MODULE StrategyUserInterfaceTests
#include <boost/test/unit_test.hpp>

extern "C" {
#include "StrategyUserInterface.h"
}

/*
 * Tests for reconstructed UI buffering layer (TradingStrategies scope).
 * Placed under dev/TradingStrategies/tests to keep all strategy-specific
 * tests together (other files: TradingStrategiesTests.cpp, StrategyContextTests.cpp, etc.).
 */

BOOST_AUTO_TEST_CASE(AddThenOverwriteValue)
{
    BOOST_CHECK_EQUAL(getUICount(), 0);

    addValueToUI("strategyRisk", 10.0);
    BOOL found = FALSE;
    double v = getUIValue("strategyRisk", &found);
    BOOST_CHECK(found == TRUE);
    BOOST_CHECK_CLOSE(v, 10.0, 1e-6);

    updateOrAddValueToUI("strategyRisk", 12.5);
    found = FALSE;
    v = getUIValue("strategyRisk", &found);
    BOOST_CHECK(found == TRUE);
    BOOST_CHECK_CLOSE(v, 12.5, 1e-6);
}

BOOST_AUTO_TEST_CASE(AppendViaUpdateOrAdd)
{
    int initialCount = getUICount();
    BOOL found = FALSE;
    double v = getUIValue("pWeeklyPredictATR", &found);
    BOOST_CHECK(found == FALSE);

    updateOrAddValueToUI("pWeeklyPredictATR", 55.0);
    BOOST_CHECK_EQUAL(getUICount(), initialCount + 1);
    v = getUIValue("pWeeklyPredictATR", &found);
    BOOST_CHECK(found == TRUE);
    BOOST_CHECK_CLOSE(v, 55.0, 1e-6);
}

BOOST_AUTO_TEST_CASE(BufferCapacityRespected)
{
    int count = getUICount();
    for (int i = count; i < TOTAL_UI_VALUES; ++i)
    {
        char name[32];
        sprintf(name, "tmpField_%d", i);
        addValueToUI(name, (double)i);
    }
    BOOST_CHECK_EQUAL(getUICount(), TOTAL_UI_VALUES);

    int before = getUICount();
    updateOrAddValueToUI("overflowField", 999.0);
    BOOST_CHECK_EQUAL(getUICount(), before);

    BOOL found = FALSE;
    double v = getUIValue("overflowField", &found);
    BOOST_CHECK(found == FALSE);
}

BOOST_AUTO_TEST_CASE(PredictiveATRProxy)
{
    addValueToUI("pDailyPredictATR", 30.0);
    BOOL found = FALSE;
    double v = getUIValue("pDailyPredictATR", &found);
    BOOST_CHECK(found == TRUE);
    BOOST_CHECK_CLOSE(v, 30.0, 1e-6);

    updateOrAddValueToUI("pDailyPredictATR", 32.25);
    v = getUIValue("pDailyPredictATR", &found);
    BOOST_CHECK(found == TRUE);
    BOOST_CHECK_CLOSE(v, 32.25, 1e-6);
}
