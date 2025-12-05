/** @file  historics.h
    @brief Historic OHLC data management
*/

#pragma once
#include "CTesterFrameworkDefines.h"

#ifdef __cplusplus
extern "C" {
#endif

/** int readHistoricFile(char *historicPath, struct HSTRateInfo **rates, unsigned int *candleNumber);
 @brief Reads historical data from a file and returns an array of rates
 @param historicPath Path to the historics file
 @param rates Array of HSTRateInfo struct contanining the OHLC data
 @param candleNumbers Number of candles returned in the array
 @param error description if any
*/
int __stdcall readHistoricFile(char *historicPath, Rates **rates, char **error);

#ifdef __cplusplus
} /* extern "C" */
#endif
