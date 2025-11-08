#ifndef STRATEGY_API_H_
#define STRATEGY_API_H_
#pragma once

#ifndef ASIRIKUY_DEFINES_H_
  #include "AsirikuyDefines.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Centralized prototypes reserved for truly shared, non-static helpers.
 * Removed per-strategy internal helpers (loadIndicators, etc.) because those
 * are defined static within their source files and do not need global
 * declarations. Avoids mismatched prototypes and redefinition issues.
 * Add only cross-TU functions here.
 */

/* Common time utility (already declared elsewhere but kept here if needed). */
void safe_timeString(char* dest, time_t t);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* STRATEGY_API_H_ */
