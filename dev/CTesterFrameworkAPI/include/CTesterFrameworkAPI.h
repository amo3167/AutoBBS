//
//  astFramework.h
//  ast
//
//  Created by Jorge Ferrando on 12/04/13.
//  Copyright (c) 2013 Jorge Ferrando. All rights reserved.
//
#pragma once

#include "CTesterFrameworkDefines.h"

#ifdef __cplusplus
extern "C" {
#endif

void __stdcall getCTesterFrameworkVersion(int* pMajor, int* pMinor, int* pBugfix);
int  __stdcall initCTesterFramework(char* pAsirikuyTesterLog, int severityLevel);

#ifdef __cplusplus
} /* extern "C" */
#endif