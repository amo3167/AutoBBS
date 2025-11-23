/**
 * @file
 * @brief     Main test runner for TradingStrategies unit tests
 * 
 * This file is included in the main UnitTests executable which defines
 * BOOST_TEST_MODULE in AsirikuyFrameworkTestModule.cpp. Individual test
 * suites are defined in separate .cpp files:
 * - StrategyContextTests.cpp
 * - StrategyFactoryTests.cpp
 * - BaseStrategyTests.cpp
 * 
 * @author    Morgan Doel (Initial implementation)
 * @version   F4.x.x
 * @date      2025
 *
 * @copyright END-USER LICENSE AGREEMENT FOR ASIRIKUY SOFTWARE
 */

#include <boost/test/unit_test.hpp>

// The test suites are automatically registered by the BOOST_AUTO_TEST_SUITE
// macros in the individual test files. The main() entry point is provided
// by AsirikuyFrameworkTestModule.cpp
