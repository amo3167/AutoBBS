//
//  astDefs.h
//  ast
//
//  Created by Jorge Ferrando on 11/04/13.
//  Copyright (c) 2013 Jorge Ferrando. All rights reserved.
//

#pragma once

#define MAX_ERROR_LENGTH 200
#ifndef MAX_LINE_LENGTH
#define MAX_LINE_LENGTH 1024
#endif

//Remove io compiler deprecated warnings
#ifdef _MSC_VER
#define _CRT_SECURE_NO_WARNINGS
#endif

//Arrays
#define ARRAY_SIZE(a) (sizeof(a) / sizeof(a[0]))

//Indexes
#define BIDASK_ARRAY_SIZE	 6

//Includes

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#if defined _WIN32 || defined _WIN64
#include <windows.h>
#endif

//Types
#ifndef true
    #define false 0;
    #define true 1;
#endif

typedef struct ast_rates_t
{
  double   open;
  double   high;
  double   low;
  double   close;
  double   volume;
  double   swapLong;
  double   swapShort;
  int	   time;
} ASTRates;

#include "AsirikuyConfig.h"
#include "AsirikuyDefines.h"
#include "CTesterDefines.h"
#include "AsirikuyFrameworkAPI.h"

#include "CTesterFrameworkAPI.h"

#include "optimizer.h"
#include "tester.h"
#include "version.h"
