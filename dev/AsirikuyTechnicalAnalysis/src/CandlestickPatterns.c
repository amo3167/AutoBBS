/**
 * @file
 * @brief     Candlestick patterns including Asirikuy specific, non-standard, and standard patterns which are not available in TaLib.
 * @details   The main difference between these patterns and the TaLib equivalents is a volatility adjustment using the average true range indicator.
 *
 * @author    Daniel Fernandez (Initial implementation)
 * @author    Morgan Doel (Ported the candlestick patterns to the F4 framework)
 * @author    Maxim Feinshtein (Assisted with design and code styling)
 * @version   F4.x.x
 * @date      2012
 *
 * @copyright END-USER LICENSE AGREEMENT FOR ASIRIKUY SOFTWARE. IMPORTANT PLEASE READ THE TERMS AND CONDITIONS OF THIS LICENSE AGREEMENT CAREFULLY BEFORE USING THIS SOFTWARE: 
 * @copyright Asirikuy's End-User License Agreement ("EULA") is a legal agreement between you (either an individual or a single entity) and Asirikuy for the use of the Asirikuy Framework in both source and binary forms. By installing, copying, or otherwise using the Asirikuy Framework, you agree to be bound by the terms of this EULA. This license agreement represents the entire agreement concerning the program between you and Asirikuy, (referred to as "licenser"), and it supersedes any prior proposal, representation, or understanding between the parties. If you do not agree to the terms of this EULA, do not install or use the Asirikuy Framework.
 * @copyright The Asirikuy Framework is protected by copyright laws and international copyright treaties, as well as other intellectual property laws and treaties. The Asirikuy Framework is licensed, not sold.
 * @copyright 1. GRANT OF LICENSE.
 * @copyright The Asirikuy Framework is licensed as follows:
 * @copyright (a) Installation and Use.
 * @copyright Asirikuy grants you the right to install and use copies of the Asirikuy Framework in both source and binary forms for personal and business use. You may also make modifications to the source code.
 * @copyright (b) Backup Copies.
 * @copyright You may make copies of the Asirikuy Framework as may be necessary for backup and archival purposes.
 * @copyright 2. DESCRIPTION OF OTHER RIGHTS AND LIMITATIONS.
 * @copyright (a) Maintenance of Copyright Notices.
 * @copyright You must not remove or alter any copyright notices on any and all copies of the Asirikuy Framework.
 * @copyright (b) Distribution.
 * @copyright You may not distribute copies of the Asirikuy Framework in binary or source forms to third parties outside of the Asirikuy community.
 * @copyright (c) Rental.
 * @copyright You may not rent, lease, or lend the Asirikuy Framework.
 * @copyright (d) Compliance with Applicable Laws.
 * @copyright You must comply with all applicable laws regarding use of the Asirikuy Framework.
 * @copyright 3. TERMINATION
 * @copyright Without prejudice to any other rights, Asirikuy may terminate this EULA if you fail to comply with the terms and conditions of this EULA. In such event, you must destroy all copies of the Asirikuy Framework in your possession.
 * @copyright 4. COPYRIGHT
 * @copyright All title, including but not limited to copyrights, in and to the Asirikuy Framework and any copies thereof are owned by Asirikuy or its suppliers. All title and intellectual property rights in and to the content which may be accessed through use of the Asirikuy Framework is the property of the respective content owner and may be protected by applicable copyright or other intellectual property laws and treaties. This EULA grants you no rights to use such content. All rights not expressly granted are reserved by Asirikuy.
 * @copyright 5. NO WARRANTIES
 * @copyright Asirikuy expressly disclaims any warranty for the Asirikuy Framework. The Asirikuy Framework is provided 'As Is' without any express or implied warranty of any kind, including but not limited to any warranties of merchantability, noninfringement, or fitness of a particular purpose. Asirikuy does not warrant or assume responsibility for the accuracy or completeness of any information, text, graphics, links or other items contained within the Asirikuy Framework. Asirikuy makes no warranties respecting any harm that may be caused by the transmission of a computer virus, worm, time bomb, logic bomb, or other such computer program. Asirikuy further expressly disclaims any warranty or representation to Authorized Users or to any third party.
 * @copyright 6. LIMITATION OF LIABILITY
 * @copyright In no event shall Asirikuy or any contributors to the Asirikuy Framework be liable for any damages (including, without limitation, lost profits, business interruption, or lost information) rising out of 'Authorized Users' use of or inability to use the Asirikuy Framework, even if Asirikuy has been advised of the possibility of such damages. In no event will Asirikuy or any contributors to the Asirikuy Framework be liable for loss of data or for indirect, special, incidental, consequential (including lost profit), or other damages based in contract, tort or otherwise. Asirikuy and contributors to the Asirikuy Framework shall have no liability with respect to the content of the Asirikuy Framework or any part thereof, including but not limited to errors or omissions contained therein, libel, infringements of rights of publicity, privacy, trademark rights, business interruption, personal injury, loss of privacy, moral rights or the disclosure of confidential information.
 */

#include "Precompiled.h"

#include <float.h>

#include "CandlestickPatterns.h"


BOOL detectThreeBlackCrows(double atrMultiplier, double atr, const double* pOpen, const double* pHigh, const double* pLow, const double* pClose, int arraySize)
{
  int    arrayIndex1    = arraySize - 2;
  int    arrayIndex2    = arraySize - 3;
  int    arrayIndex3    = arraySize - 4;
  double atrCoefficient = atr * atrMultiplier;
  double bar1Range, bar1Body, bar2Body;
  BOOL   patternDetected;

  if(pOpen == NULL)
  {
    fprintf(stderr, "[CRITICAL] detectThreeBlackCrows() failed. pOpen = NULL\n");
    return NULL_POINTER;
  }

  if(pHigh == NULL)
  {
    fprintf(stderr, "[CRITICAL] detectThreeBlackCrows() failed. pHigh = NULL\n");
    return NULL_POINTER;
  }

  if(pLow == NULL)
  {
    fprintf(stderr, "[CRITICAL] detectThreeBlackCrows() failed. pLow = NULL\n");
    return NULL_POINTER;
  }

  if(pClose == NULL)
  {
    fprintf(stderr, "[CRITICAL] detectThreeBlackCrows() failed. pClose = NULL\n");
    return NULL_POINTER;
  }

  if(arraySize < 4)
  {
    fprintf(stderr, "[ERROR] threeBlackCrows() failed. arraySize < 4\n");
    return FALSE;
  }

  bar1Range = fabs(pHigh[arrayIndex1] - pLow[arrayIndex1]);
  bar1Body  = fabs(pOpen[arrayIndex1] - pClose[arrayIndex1]);
  bar2Body  = fabs(pOpen[arrayIndex2] - pClose[arrayIndex2]);

  /* Prevent the possibility of dividing by zero. */
  if(bar1Body < DBL_EPSILON) bar1Body = DBL_EPSILON;

  patternDetected = ((pClose[arrayIndex3] < pOpen[arrayIndex3])
    && (pClose[arrayIndex2] < pOpen[arrayIndex2])
    && (pClose[arrayIndex1] < pOpen[arrayIndex1])
    && (bar2Body > atrCoefficient)
    && (bar1Body > atrCoefficient)
    && (bar1Range / bar1Body < 3));

  return patternDetected;
}

BOOL detectThreeWhiteSoldiers(double atrMultiplier, double atr, const double* pOpen, const double* pHigh, const double* pLow, const double* pClose, int arraySize)
{
  int    arrayIndex1    = arraySize - 2;
  int    arrayIndex2    = arraySize - 3;
  int    arrayIndex3    = arraySize - 4;
  double atrCoefficient = atr * atrMultiplier;
  double bar1Range, bar1Body, bar2Body;
  BOOL   patternDetected;

  if(pOpen == NULL)
  {
    fprintf(stderr, "[CRITICAL] detectThreeWhiteSoldiers() failed. pOpen = NULL\n");
    return NULL_POINTER;
  }

  if(pHigh == NULL)
  {
    fprintf(stderr, "[CRITICAL] detectThreeWhiteSoldiers() failed. pHigh = NULL\n");
    return NULL_POINTER;
  }

  if(pLow == NULL)
  {
    fprintf(stderr, "[CRITICAL] detectThreeWhiteSoldiers() failed. pLow = NULL\n");
    return NULL_POINTER;
  }

  if(pClose == NULL)
  {
    fprintf(stderr, "[CRITICAL] detectThreeWhiteSoldiers() failed. pClose = NULL\n");
    return NULL_POINTER;
  }

  if(arraySize < 4)
  {
    fprintf(stderr, "[ERROR] threeWhiteSoldiers() failed. arraySize < 4\n");
    return FALSE;
  }

  bar1Range = fabs(pHigh[arrayIndex1] - pLow[arrayIndex1]);
  bar1Body  = fabs(pOpen[arrayIndex1] - pClose[arrayIndex1]);
  bar2Body  = fabs(pOpen[arrayIndex2] - pClose[arrayIndex2]);

  /* Prevent the possibility of dividing by zero. */
  if(bar1Body < DBL_EPSILON) bar1Body = DBL_EPSILON;

  patternDetected = ((pClose[arrayIndex3] > pOpen[arrayIndex3])
    && (pClose[arrayIndex2] > pOpen[arrayIndex2])
    && (pClose[arrayIndex1] > pOpen[arrayIndex1])
    && (bar2Body > atrCoefficient)
    && (bar1Body > atrCoefficient)
    && (bar1Range / bar1Body < 3));

  return patternDetected;
}

BOOL detectHangingMan(double atrMultiplier, double atr, const double* pOpen, const double* pHigh, const double* pLow, const double* pClose, int arraySize)
{
  int    arrayIndex1    = arraySize - 2;
  double atrCoefficient = atr * atrMultiplier;
  double bar1Range, bar1Body;
  BOOL   patternDetected;

  if(pOpen == NULL)
  {
    fprintf(stderr, "[CRITICAL] detectHangingMan() failed. pOpen = NULL\n");
    return NULL_POINTER;
  }

  if(pHigh == NULL)
  {
    fprintf(stderr, "[CRITICAL] detectHangingMan() failed. pHigh = NULL\n");
    return NULL_POINTER;
  }

  if(pLow == NULL)
  {
    fprintf(stderr, "[CRITICAL] detectHangingMan() failed. pLow = NULL\n");
    return NULL_POINTER;
  }

  if(pClose == NULL)
  {
    fprintf(stderr, "[CRITICAL] detectHangingMan() failed. pClose = NULL\n");
    return NULL_POINTER;
  }

  if(arraySize < 2)
  {
    fprintf(stderr, "[ERROR] hangingMan() failed. arraySize < 2\n");
    return FALSE;
  }

  bar1Range = fabs(pHigh[arrayIndex1] - pLow[arrayIndex1]);
  bar1Body  = fabs(pOpen[arrayIndex1] - pClose[arrayIndex1]);

  /* Prevent the possibility of dividing by zero. */
  if(bar1Body < DBL_EPSILON) bar1Body = DBL_EPSILON;

  patternDetected = ((pHigh[arrayIndex1] - pClose[arrayIndex1]) > atrCoefficient && (bar1Range / bar1Body > 3));

  return patternDetected;
}

BOOL detectHammer(double atrMultiplier, double atr, const double* pOpen, const double* pHigh, const double* pLow, const double* pClose, int arraySize)
{
  int    arrayIndex1    = arraySize - 2;
  double atrCoefficient = atr * atrMultiplier;
  double bar1Range, bar1Body;
  BOOL   patternDetected;

  if(pOpen == NULL)
  {
    fprintf(stderr, "[CRITICAL] detectHammer() failed. pOpen = NULL\n");
    return NULL_POINTER;
  }

  if(pHigh == NULL)
  {
    fprintf(stderr, "[CRITICAL] detectHammer() failed. pHigh = NULL\n");
    return NULL_POINTER;
  }

  if(pLow == NULL)
  {
    fprintf(stderr, "[CRITICAL] detectHammer() failed. pLow = NULL\n");
    return NULL_POINTER;
  }

  if(pClose == NULL)
  {
    fprintf(stderr, "[CRITICAL] detectHammer() failed. pClose = NULL\n");
    return NULL_POINTER;
  }

  if(arraySize < 2)
  {
    fprintf(stderr, "[ERROR] hammer() failed. arraySize < 2\n");
    return FALSE;
  }

  bar1Range = fabs(pHigh[arrayIndex1] - pLow[arrayIndex1]);
  bar1Body  = fabs(pOpen[arrayIndex1] - pClose[arrayIndex1]);

  /* Prevent the possibility of dividing by zero. */
  if(bar1Body < DBL_EPSILON) bar1Body = DBL_EPSILON;

  patternDetected = ((pClose[arrayIndex1] - pLow[arrayIndex1]) > atrCoefficient && (bar1Range / bar1Body > 3));

  return patternDetected;
}

int detectBearishEngulfing(double atrMultiplier, double atr, const double* pOpen, const double* pHigh, const double* pLow, const double* pClose, int arraySize)
{
  int    arrayIndex1    = arraySize - 2;
  int    arrayIndex2    = arraySize - 3;
  double atrCoefficient = atr * atrMultiplier;
  double bar1Range, bar1Body, bar2Range, bar2Body;
  BOOL   patternDetected;

  if(pOpen == NULL)
  {
    fprintf(stderr, "[CRITICAL] detectBearishEngulfing() failed. pOpen = NULL\n");
    return NULL_POINTER;
  }

  if(pHigh == NULL)
  {
    fprintf(stderr, "[CRITICAL] detectBearishEngulfing() failed. pHigh = NULL\n");
    return NULL_POINTER;
  }

  if(pLow == NULL)
  {
    fprintf(stderr, "[CRITICAL] detectBearishEngulfing() failed. pLow = NULL\n");
    return NULL_POINTER;
  }

  if(pClose == NULL)
  {
    fprintf(stderr, "[CRITICAL] detectBearishEngulfing() failed. pClose = NULL\n");
    return NULL_POINTER;
  }

  if(arraySize < 3)
  {
    fprintf(stderr, "[ERROR] bearishEngulfing() failed. arraySize < 3\n");
    return FALSE;
  }

  bar1Range = fabs(pHigh[arrayIndex1] - pLow[arrayIndex1]);
  bar2Range = fabs(pHigh[arrayIndex2] - pLow[arrayIndex2]);
  bar1Body  = fabs(pOpen[arrayIndex1] - pClose[arrayIndex1]);
  bar2Body  = fabs(pOpen[arrayIndex2] - pClose[arrayIndex2]);

  patternDetected = ((pOpen[arrayIndex1]  > pClose[arrayIndex2])
    && (pClose[arrayIndex1] < pOpen[arrayIndex2])
    && (pOpen[arrayIndex2] < pOpen[arrayIndex1])
    && (pClose[arrayIndex1] < pClose[arrayIndex2])
    && ((bar2Range - bar2Body) < atrCoefficient)
    && ((bar1Range - bar1Body) < atrCoefficient)
    && ((bar1Body - bar2Body) > atrCoefficient));

  return patternDetected;
}

BOOL detectBullishEngulfing(double atrMultiplier, double atr, const double* pOpen, const double* pHigh, const double* pLow, const double* pClose, int arraySize)
{
  int    arrayIndex1    = arraySize - 2;
  int    arrayIndex2    = arraySize - 3;
  double atrCoefficient = atr * atrMultiplier;
  double bar1Range, bar1Body, bar2Range, bar2Body;
  BOOL   patternDetected;

  if(pOpen == NULL)
  {
    fprintf(stderr, "[CRITICAL] detectBullishEngulfing() failed. pOpen = NULL\n");
    return NULL_POINTER;
  }

  if(pHigh == NULL)
  {
    fprintf(stderr, "[CRITICAL] detectBullishEngulfing() failed. pHigh = NULL\n");
    return NULL_POINTER;
  }

  if(pLow == NULL)
  {
    fprintf(stderr, "[CRITICAL] detectBullishEngulfing() failed. pLow = NULL\n");
    return NULL_POINTER;
  }

  if(pClose == NULL)
  {
    fprintf(stderr, "[CRITICAL] detectBullishEngulfing() failed. pClose = NULL\n");
    return NULL_POINTER;
  }

  if(arraySize < 3)
  {
    fprintf(stderr, "[ERROR] bullishEngulfing() failed. arraySize < 3\n");
    return FALSE;
  }

  bar1Range = fabs(pHigh[arrayIndex1] - pLow[arrayIndex1]);
  bar2Range = fabs(pHigh[arrayIndex2] - pLow[arrayIndex2]);
  bar1Body  = fabs(pOpen[arrayIndex1] - pClose[arrayIndex1]);
  bar2Body  = fabs(pOpen[arrayIndex2] - pClose[arrayIndex2]);

  patternDetected = ((pOpen[arrayIndex1] < pClose[arrayIndex2])
    && (pClose[arrayIndex1] > pOpen[arrayIndex2])
    && (pOpen[arrayIndex2] > pOpen[arrayIndex1])
    && (pClose[arrayIndex1] > pClose[arrayIndex2])
    && ((bar2Range - bar2Body) < atrCoefficient)
    && ((bar1Range - bar1Body) < atrCoefficient)
    && ((bar1Body -  bar2Body) > atrCoefficient));

  return patternDetected;
}

BOOL detectBearishRapidTp(double atrMultiplier, double atr, const double* pOpen, const double* pClose, int arraySize)
{
  int    arrayIndex1    = arraySize - 2;
  int    arrayIndex2    = arraySize - 3;
  double atrCoefficient = atr * atrMultiplier;
  double bar1Body;
  BOOL   patternDetected;

  if(pOpen == NULL)
  {
    fprintf(stderr, "[CRITICAL] detectBearishRapidTp() failed. pOpen = NULL\n");
    return NULL_POINTER;
  }

  if(pClose == NULL)
  {
    fprintf(stderr, "[CRITICAL] detectBearishRapidTp() failed. pClose = NULL\n");
    return NULL_POINTER;
  }

  if(arraySize < 3)
  {
    fprintf(stderr, "[ERROR] bearishRapidTp() failed. arraySize < 3\n");
    return FALSE;
  }

  bar1Body = fabs(pOpen[arrayIndex1] - pClose[arrayIndex1]);

  patternDetected = (pClose[arrayIndex1] > pOpen[arrayIndex1] 
  && pClose[arrayIndex2] < pOpen[arrayIndex2]
  && bar1Body > atrCoefficient);

  return patternDetected;
}

BOOL detectBullishRapidTp(double atrMultiplier, double atr, const double* pOpen, const double* pClose, int arraySize)
{
  int    arrayIndex1    = arraySize - 2;
  int    arrayIndex2    = arraySize - 3;
  double atrCoefficient = atr * atrMultiplier;
  double bar1Body;
  BOOL   patternDetected;

  if(pOpen == NULL)
  {
    fprintf(stderr, "[CRITICAL] detectBullishRapidTp() failed. pOpen = NULL\n");
    return NULL_POINTER;
  }

  if(pClose == NULL)
  {
    fprintf(stderr, "[CRITICAL] detectBullishRapidTp() failed. pClose = NULL\n");
    return NULL_POINTER;
  }

  if(arraySize < 3)
  {
    fprintf(stderr, "[ERROR] bullishRapidTp() failed. arraySize < 3\n");
    return FALSE;
  }

  bar1Body = fabs(pOpen[arrayIndex1] - pClose[arrayIndex1]);

  patternDetected = (pClose[arrayIndex1] < pOpen[arrayIndex1] 
  && pClose[arrayIndex2] > pOpen[arrayIndex2] 
  && bar1Body > atrCoefficient);

  return patternDetected;
}

BOOL detectDarkCloudCover(double atrMultiplier, double atr, const double* pOpen, const double* pHigh, const double* pLow, const double* pClose, int arraySize)
{
  int    arrayIndex1    = arraySize - 2;
  int    arrayIndex2    = arraySize - 3;
  double atrCoefficient = atr * atrMultiplier;
  double bar1Body, bar2Median;
  BOOL   patternDetected;

  if(pOpen == NULL)
  {
    fprintf(stderr, "[CRITICAL] detectDarkCloudCover() failed. pOpen = NULL\n");
    return NULL_POINTER;
  }

  if(pHigh == NULL)
  {
    fprintf(stderr, "[CRITICAL] detectDarkCloudCover() failed. pHigh = NULL\n");
    return NULL_POINTER;
  }

  if(pLow == NULL)
  {
    fprintf(stderr, "[CRITICAL] detectDarkCloudCover() failed. pLow = NULL\n");
    return NULL_POINTER;
  }

  if(pClose == NULL)
  {
    fprintf(stderr, "[CRITICAL] detectDarkCloudCover() failed. pClose = NULL\n");
    return NULL_POINTER;
  }

  if(arraySize < 3)
  {
    fprintf(stderr, "[ERROR] darkCloudCover() failed. arraySize < 3\n");
    return FALSE;
  }

  bar1Body   = fabs(pOpen[arrayIndex1] - pClose[arrayIndex1]);
  bar2Median = (pLow[arrayIndex2] + pHigh[arrayIndex2]) / 2;

  patternDetected = ((pClose[arrayIndex2] > pOpen[arrayIndex2])
    && (pOpen[arrayIndex1] > pClose[arrayIndex1])
    && (pOpen[arrayIndex1] >= pClose[arrayIndex2])
    && (pClose[arrayIndex1] <= bar2Median)
    && (bar1Body >= atrCoefficient));

  return patternDetected;
}

BOOL detectPiercing(double atrMultiplier, double atr, const double* pOpen, const double* pHigh, const double* pLow, const double* pClose, int arraySize)
{
  int    arrayIndex1    = arraySize - 2;
  int    arrayIndex2    = arraySize - 3;
  double atrCoefficient = atr * atrMultiplier;
  double bar1Body, bar2Median;
  BOOL   patternDetected;

  if(pOpen == NULL)
  {
    fprintf(stderr, "[CRITICAL] detectPiercing() failed. pOpen = NULL\n");
    return NULL_POINTER;
  }

  if(pHigh == NULL)
  {
    fprintf(stderr, "[CRITICAL] detectPiercing() failed. pHigh = NULL\n");
    return NULL_POINTER;
  }

  if(pLow == NULL)
  {
    fprintf(stderr, "[CRITICAL] detectPiercing() failed. pLow = NULL\n");
    return NULL_POINTER;
  }

  if(pClose == NULL)
  {
    fprintf(stderr, "[CRITICAL] detectPiercing() failed. pClose = NULL\n");
    return NULL_POINTER;
  }

  if(arraySize < 3)
  {
    fprintf(stderr, "[ERROR] piercing() failed. arraySize < 3\n");
    return FALSE;
  }

  bar1Body   = fabs(pOpen[arrayIndex1] - pClose[arrayIndex1]);
  bar2Median = (pLow[arrayIndex2] + pHigh[arrayIndex2]) / 2;

  patternDetected = ((pOpen[arrayIndex2] > pClose[arrayIndex2])
    && (pClose[arrayIndex1] > pOpen[arrayIndex1])
    && (pOpen[arrayIndex1] <= pClose[arrayIndex2])
    && (pClose[arrayIndex1] >= bar2Median)
    && (bar1Body >= atrCoefficient));

  return patternDetected;
}