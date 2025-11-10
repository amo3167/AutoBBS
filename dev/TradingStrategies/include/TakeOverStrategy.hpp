#pragma once

#include "StrategyTypes.h"
#include "AsirikuyDefines.h"
#include "OrderManagement.h"
#include "EasyTradeCWrapper.hpp"
#include "StrategyUserInterface.h"
#include "NumericLoggingHelpers.hpp"

extern "C" {
AsirikuyReturnCode runTakeOverStrategy(StrategyParams* pParams);
}
