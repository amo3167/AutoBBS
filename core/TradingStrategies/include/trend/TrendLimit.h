#pragma once

#ifndef TRENDLIMIT_H_
#define TRENDLIMIT_H_
#pragma once

#ifndef ASIRIKUY_DEFINES_H_
#include "AsirikuyDefines.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif

	/**
	* Runs the BBS trading strategy.
	*
	* @param StrategyParams* pParams
	*   The structure containing all necessary parameters.
	*
	* @return enum AsirikuyReturnCode
	*   An enum indicating success or the type of failure that occured.
	*/
	AsirikuyReturnCode runTrendLimit(StrategyParams* pParams);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* BBS_H_ */
