#pragma once

#ifndef TAKEOVER_H_
#define TAKEOVER_H_
#pragma once

#ifndef ASIRIKUY_DEFINES_H_
#include "AsirikuyDefines.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif

	/**
	* Runs the TakeOver trading strategy.
	*
	* @param StrategyParams* pParams
	*   The structure containing all necessary parameters.
	*
	* @return enum AsirikuyReturnCode
	*   An enum indicating success or the type of failure that occured.
	*/
	AsirikuyReturnCode runTakeOver(StrategyParams* pParams);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* TAKEOVER_H_ */
