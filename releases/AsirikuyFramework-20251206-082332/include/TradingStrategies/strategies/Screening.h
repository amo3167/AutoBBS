
#ifndef SCREENING_H_
#define SCREENING_H_
#pragma once

#ifndef ASIRIKUY_DEFINES_H_
  #include "AsirikuyDefines.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif

/**
* Runs the TestEA trading strategy.
*
* @param StrategyParams* pParams
*   The structure containing all necessary parameters.
*
* @return enum AsirikuyReturnCode
*   An enum indicating success or the type of failure that occured.
*/
AsirikuyReturnCode runScreening(StrategyParams* pParams);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* TEST_EA_H_ */
