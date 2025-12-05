#pragma once

#ifndef AUTOBBS_H_
#define AUTOBBS_H_

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
	AsirikuyReturnCode runAutoBBS(StrategyParams* pParams);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* AUTOBBS_H_ */
