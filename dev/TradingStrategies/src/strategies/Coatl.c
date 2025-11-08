/**
 * @file
 * @brief     The Coatl trading system.
 * @details   Coatl uses a genetic algorithm to generate trading logic on a 1 hour, 4 hour, or daily timeframe.
 * 
 * @author    Daniel Fernandez (Original idea, initial implementation, and optimization)
 * @author    Maxim Feinshtein (Contributed to initial implementation, code styling, error handling, and user interface)
 * @author    Morgan Doel (Ported Coatl to the F4 framework)
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

#include <ta_libc.h>

#include "StrategyUserInterface.h"
#include "AsirikuyStrategies.h"
#include "InstanceStates.h"
#include "AsirikuyTime.h"
#include "Logging.h"
#include "OrderSignals.h"
#include "OrderManagement.h"
#include "AsirikuyTechnicalAnalysis.h"

#define USE_INTERNAL_SL FALSE
#define USE_INTERNAL_TP TRUE

#define SHORT_TERM_RSI_PERIOD           20
#define LONG_TERM_RSI_PERIOD            40

#define ABSOLUTE_PRICE_OSCILLATOR_FAST_PERIOD 5

#define SHORT_TERM_STOCH_SLOW_K_PERIOD  3
#define SHORT_TERM_STOCH_SLOW_D_PERIOD  1
#define SHORT_TERM_STOCH_SLOW_K_MA_TYPE TA_MAType_SMA
#define SHORT_TERM_STOCH_SLOW_D_MA_TYPE TA_MAType_SMA

#define LONG_TERM_STOCH_SLOW_K_PERIOD   3
#define LONG_TERM_STOCH_SLOW_D_PERIOD   1
#define LONG_TERM_STOCH_SLOW_K_MA_TYPE  TA_MAType_SMA
#define LONG_TERM_STOCH_SLOW_D_MA_TYPE  TA_MAType_SMA

#define PSAR_ACCELERATION               0.02
#define PSAR_MAXIMUM                    0.2

#define BBANDS_DEVIATION                2
#define BBANDS_MA_TYPE                  TA_MAType_SMA

#define MACD_FAST_PERIOD                12
#define MACD_SIGNAL_PERIOD              9

#define CCI_PERIOD                      20

typedef enum additionalSettingsIndexes_t
{
  IDX_ENTRY_LOGIC      = ADDITIONAL_PARAM_1,
  IDX_EXIT_LOGIC       = ADDITIONAL_PARAM_2,
  IDX_ENTRY_SYMMETRY   = ADDITIONAL_PARAM_3,
  IDX_EXIT_SYMMETRY    = ADDITIONAL_PARAM_4,
  IDX_INDICATOR_PERIOD = ADDITIONAL_PARAM_5,
  IDX_ENTRY_LOGIC_2     = ADDITIONAL_PARAM_6,
  IDX_EXIT_LOGIC_2      = ADDITIONAL_PARAM_7,
  IDX_ENTRY_SYMMETRY_2  = ADDITIONAL_PARAM_8,
  IDX_EXIT_SYMMETRY_2   = ADDITIONAL_PARAM_9,
  IDX_INDICATOR_PERIOD_2= ADDITIONAL_PARAM_10
} AdditionalSettingsIndexes;

typedef enum ratesIndexes_t
{
  PRIMARY_RATES = 0,
  DAILY_RATES   = 1
} RatesIndexes;

typedef enum logicParameters_t
{
  LOGIC_IDX_ENTRY_LOGIC    = 0,
  LOGIC_IDX_EXIT_LOGIC     = 1,
  LOGIC_IDX_ENTRY_SYMMETRY = 2,
  LOGIC_IDX_EXIT_SYMMETRY  = 3,
  LOGIC_IDX_ATR            = 4,
  LOGIC_IDX_OPEN_1         = 5,
  LOGIC_IDX_CLOSE_1        = 6,
  LOGIC_IDX_CLOSE_2        = 7,
  LOGIC_IDX_BAR_SIZE       = 8,
  LOGIC_IDX_RSI_1          = 11,
  LOGIC_IDX_RSI_2          = 12,
  LOGIC_IDX_STOCH_1        = 13,
  LOGIC_IDX_STOCH_2        = 14,
  LOGIC_IDX_PSAR_1         = 15,
  LOGIC_IDX_PSAR_2         = 16,
  LOGIC_IDX_BBANDS_UPPER_1 = 17,
  LOGIC_IDX_BBANDS_UPPER_2 = 18,
  LOGIC_IDX_BBANDS_LOWER_1 = 19,
  LOGIC_IDX_BBANDS_LOWER_2 = 20,
  LOGIC_IDX_MACD_1         = 21,
  LOGIC_IDX_MACD_2         = 22,
  LOGIC_IDX_CCI_1          = 23,
  LOGIC_IDX_CCI_2          = 24,
  LOGIC_IDX_APO_1          = 25,
  LOGIC_IDX_APO_2          = 26,
  LOGIC_IDX_AROON_UP_1     = 27,
  LOGIC_IDX_AROON_UP_2     = 28,
  LOGIC_IDX_AROON_DOWN_1   = 29,
  LOGIC_IDX_AROON_DOWN_2   = 30,
  TOTAL_LOGIC_PARAMETERS   = 31
} LogicParameters;

typedef enum logicSettings_t
{
  NO_LOGIC        = -1,
  RSI_NORMAL      = 0,
  RSI_REVERSE     = 1,
  STOCH_NORMAL    = 2,
  STOCH_REVERSE	  = 3,
  MACD_NORMAL     = 4,
  MACD_REVERSE    = 5,
  CCI_NORMAL      = 6,
  CCI_REVERSE     = 7,
  BB_NORMAL       = 8,
  BB_REVERSE      = 9,
  PSAR_NORMAL     = 10,
  PSAR_REVERSE    = 11,
  BIG_BAR_NORMAL  = 12,
  BIG_BAR_REVERSE = 13,
  APO_NORMAL	    = 14,
  APO_REVERSE	    = 15,
  AROON_NORMAL	  = 16,
  AROON_REVERSE	  = 17
} LogicSettings;

static AsirikuyReturnCode setUIValues(StrategyParams* pParams, double* pLogicParams)
{
  AsirikuyReturnCode returnCode = SUCCESS;
  double userInterfaceValues[TOTAL_UI_VALUES];
  char   *userInterfaceVariableNames[TOTAL_UI_VALUES] = {
    "ATR",
    "Entry Logic", 
    "Exit Logic",
    "",
    "",
    "",
    "",
    "",
    "",
    ""};

  if(pParams == NULL)
  {
    pantheios_logputs(PANTHEIOS_SEV_CRITICAL, (PAN_CHAR_T*)"setUIValues() failed. pParams = NULL");
    return NULL_POINTER;
  }

  if(pLogicParams == NULL)
  {
    pantheios_logputs(PANTHEIOS_SEV_CRITICAL, (PAN_CHAR_T*)"setUIValues() failed. pLogicParams = NULL");
    return NULL_POINTER;
  }

  userInterfaceValues[0] = pLogicParams[LOGIC_IDX_ATR];
  userInterfaceValues[1] = pLogicParams[LOGIC_IDX_ENTRY_LOGIC];
  userInterfaceValues[2] = pLogicParams[LOGIC_IDX_EXIT_LOGIC];
  userInterfaceValues[3] = 0;
  userInterfaceValues[4] = 0;
  userInterfaceValues[5] = 0;
  userInterfaceValues[6] = 0;
  userInterfaceValues[7] = 0;
  userInterfaceValues[8] = 0;
  userInterfaceValues[9] = 0;

	return saveUserInterfaceValues(userInterfaceVariableNames, userInterfaceValues, TOTAL_UI_VALUES, (int)pParams->settings[STRATEGY_INSTANCE_ID], (BOOL)pParams->settings[IS_BACKTESTING]);
}

static AsirikuyReturnCode loadIndicators(StrategyParams* pParams, double* pLogicParams, int selection)
{
  TA_RetCode retCode;
  char       timeString[MAX_TIME_STRING_SIZE] = "";
  double     notUsed;
  int        outBegIdx, outNBElement;
  int        primaryShift0Index, primaryShift1Index, primaryShift2Index, dailyShift1Index, indicatorSlowPeriod;
  Rates      *primaryRates, *dailyRates;

  if(pParams == NULL)
  {
    pantheios_logputs(PANTHEIOS_SEV_CRITICAL, (PAN_CHAR_T*)"loadIndicators() failed. pParams = NULL");
    return NULL_POINTER;
  }

  primaryRates        = &pParams->ratesBuffers->rates[PRIMARY_RATES];
  dailyRates          = &pParams->ratesBuffers->rates[DAILY_RATES];
  primaryShift0Index  = primaryRates->info.arraySize - 1;
  primaryShift1Index  = primaryRates->info.arraySize - 2;
  primaryShift2Index  = primaryRates->info.arraySize - 3;
  dailyShift1Index    = dailyRates->info.arraySize - 2;
  pLogicParams[LOGIC_IDX_OPEN_1]         = primaryRates->open[primaryShift1Index];
  pLogicParams[LOGIC_IDX_CLOSE_1]        = primaryRates->close[primaryShift1Index];
  pLogicParams[LOGIC_IDX_CLOSE_2]        = primaryRates->close[primaryShift2Index];
  pLogicParams[LOGIC_IDX_BAR_SIZE]       = fabs(primaryRates->close[primaryShift1Index] - primaryRates->open[primaryShift1Index]);

  if (selection == 1)

  {
 
  indicatorSlowPeriod = (int)pParams->settings[IDX_INDICATOR_PERIOD];
  pLogicParams[LOGIC_IDX_ENTRY_LOGIC]    = pParams->settings[IDX_ENTRY_LOGIC];
  pLogicParams[LOGIC_IDX_EXIT_LOGIC]     = pParams->settings[IDX_EXIT_LOGIC];
  pLogicParams[LOGIC_IDX_ENTRY_SYMMETRY] = pParams->settings[IDX_ENTRY_SYMMETRY];
  pLogicParams[LOGIC_IDX_EXIT_SYMMETRY]  = pParams->settings[IDX_EXIT_SYMMETRY];

  }

  if (selection == 2)

  {

  indicatorSlowPeriod = (int)pParams->settings[IDX_INDICATOR_PERIOD_2];
  pLogicParams[LOGIC_IDX_ENTRY_LOGIC]    = pParams->settings[IDX_ENTRY_LOGIC_2];
  pLogicParams[LOGIC_IDX_EXIT_LOGIC]     = pParams->settings[IDX_EXIT_LOGIC_2];
  pLogicParams[LOGIC_IDX_ENTRY_SYMMETRY] = pParams->settings[IDX_ENTRY_SYMMETRY_2];
  pLogicParams[LOGIC_IDX_EXIT_SYMMETRY]  = pParams->settings[IDX_EXIT_SYMMETRY_2];
 
  }

  retCode = TA_ATR(dailyShift1Index, dailyShift1Index, dailyRates->high, dailyRates->low, dailyRates->close, (int)pParams->settings[ATR_AVERAGING_PERIOD], &outBegIdx, &outNBElement, &pLogicParams[LOGIC_IDX_ATR]);
  if(retCode != TA_SUCCESS)
  {
    return logTALibError("TA_ATR()", retCode);
  }

  if(pLogicParams[LOGIC_IDX_EXIT_LOGIC] == AROON_NORMAL || pLogicParams[LOGIC_IDX_EXIT_LOGIC] == AROON_REVERSE ||  pLogicParams[LOGIC_IDX_ENTRY_LOGIC] == AROON_NORMAL || pLogicParams[LOGIC_IDX_ENTRY_LOGIC] == AROON_REVERSE)
  {
    retCode =  TA_AROON( primaryShift1Index,primaryShift1Index,primaryRates->high, primaryRates->low,indicatorSlowPeriod, &outBegIdx,&outNBElement,&pLogicParams[LOGIC_IDX_AROON_UP_1], &pLogicParams[LOGIC_IDX_AROON_DOWN_1]);
    if(retCode != TA_SUCCESS)
    {
      return logTALibError("TA_AROON()", retCode);
    }

    retCode =  TA_AROON( primaryShift2Index,primaryShift2Index,primaryRates->high, primaryRates->low,indicatorSlowPeriod, &outBegIdx,&outNBElement,&pLogicParams[LOGIC_IDX_AROON_UP_2], &pLogicParams[LOGIC_IDX_AROON_DOWN_2]);
    if(retCode != TA_SUCCESS)
    {
      return logTALibError("TA_AROON()", retCode);
    }

    pantheios_logprintf(PANTHEIOS_SEV_INFORMATIONAL, (PAN_CHAR_T*)"AROON_UP_1 = %lf, AROON_UP_2 = %lf, AROON_DOWN_1 = %lf, AROON_DOWN_2 = %lf", pLogicParams[LOGIC_IDX_AROON_UP_1], pLogicParams[LOGIC_IDX_AROON_UP_2], pLogicParams[LOGIC_IDX_AROON_DOWN_1], pLogicParams[LOGIC_IDX_AROON_DOWN_2]);
  }

  if(pLogicParams[LOGIC_IDX_EXIT_LOGIC] == APO_NORMAL || pLogicParams[LOGIC_IDX_EXIT_LOGIC] == APO_REVERSE ||  pLogicParams[LOGIC_IDX_ENTRY_LOGIC] == APO_NORMAL || pLogicParams[LOGIC_IDX_ENTRY_LOGIC] == APO_REVERSE)
  {
    retCode = TA_APO( primaryShift1Index,primaryShift1Index,primaryRates->close,ABSOLUTE_PRICE_OSCILLATOR_FAST_PERIOD, indicatorSlowPeriod, (TA_MAType)0, &outBegIdx, &outNBElement, &pLogicParams[LOGIC_IDX_APO_1]);
    if(retCode != TA_SUCCESS)
    {
      return logTALibError("TA_APO()", retCode);
    }

    retCode = TA_APO( primaryShift2Index,primaryShift2Index,primaryRates->close,ABSOLUTE_PRICE_OSCILLATOR_FAST_PERIOD, indicatorSlowPeriod, (TA_MAType)0, &outBegIdx, &outNBElement, &pLogicParams[LOGIC_IDX_APO_2]);
    if(retCode != TA_SUCCESS)
    {
      return logTALibError("TA_APO()", retCode);
    }

    pantheios_logprintf(PANTHEIOS_SEV_INFORMATIONAL, (PAN_CHAR_T*)"APO1 = %lf, APO2 = %lf", pLogicParams[LOGIC_IDX_APO_1], pLogicParams[LOGIC_IDX_APO_2]);
  }

  if(pLogicParams[LOGIC_IDX_EXIT_LOGIC] == RSI_NORMAL || pLogicParams[LOGIC_IDX_EXIT_LOGIC] == RSI_REVERSE ||  pLogicParams[LOGIC_IDX_ENTRY_LOGIC] == RSI_NORMAL || pLogicParams[LOGIC_IDX_ENTRY_LOGIC] == RSI_REVERSE)
  {
    retCode = TA_RSI(primaryShift1Index, primaryShift1Index, primaryRates->close, indicatorSlowPeriod, &outBegIdx, &outNBElement, &pLogicParams[LOGIC_IDX_RSI_1]);
    if(retCode != TA_SUCCESS)
    {
      return logTALibError("TA_RSI()", retCode);
    }

    retCode = TA_RSI(primaryShift2Index, primaryShift2Index, primaryRates->close, indicatorSlowPeriod, &outBegIdx, &outNBElement, &pLogicParams[LOGIC_IDX_RSI_2]);
    if(retCode != TA_SUCCESS)
    {
      return logTALibError("TA_RSI()", retCode);
    }

    pantheios_logprintf(PANTHEIOS_SEV_INFORMATIONAL, (PAN_CHAR_T*)"RSI1 = %lf, RSI2 = %lf", pLogicParams[LOGIC_IDX_RSI_1], pLogicParams[LOGIC_IDX_RSI_2]);
  }

  if(pLogicParams[LOGIC_IDX_EXIT_LOGIC] == STOCH_NORMAL || pLogicParams[LOGIC_IDX_EXIT_LOGIC] == STOCH_REVERSE ||  pLogicParams[LOGIC_IDX_ENTRY_LOGIC] == STOCH_NORMAL || pLogicParams[LOGIC_IDX_ENTRY_LOGIC] == STOCH_REVERSE)
  {
    retCode = TA_STOCH(primaryShift1Index, primaryShift1Index, primaryRates->high, primaryRates->low, primaryRates->close, indicatorSlowPeriod, SHORT_TERM_STOCH_SLOW_K_PERIOD, SHORT_TERM_STOCH_SLOW_K_MA_TYPE, 
      SHORT_TERM_STOCH_SLOW_D_PERIOD, SHORT_TERM_STOCH_SLOW_D_MA_TYPE, &outBegIdx, &outNBElement, &pLogicParams[LOGIC_IDX_STOCH_1], &notUsed);
    if(retCode != TA_SUCCESS)
    {
      return logTALibError("TA_STOCH()", retCode);
    }

    retCode = TA_STOCH(primaryShift2Index, primaryShift2Index, primaryRates->high, primaryRates->low, primaryRates->close, indicatorSlowPeriod, SHORT_TERM_STOCH_SLOW_K_PERIOD, SHORT_TERM_STOCH_SLOW_K_MA_TYPE, 
      SHORT_TERM_STOCH_SLOW_D_PERIOD, SHORT_TERM_STOCH_SLOW_D_MA_TYPE, &outBegIdx, &outNBElement, &pLogicParams[LOGIC_IDX_STOCH_2], &notUsed);
    if(retCode != TA_SUCCESS)
    {
      return logTALibError("TA_STOCH()", retCode);
    }

    pantheios_logprintf(PANTHEIOS_SEV_INFORMATIONAL, (PAN_CHAR_T*)"STOCH1 = %lf, STOCH2 = %lf", pLogicParams[LOGIC_IDX_STOCH_1], pLogicParams[LOGIC_IDX_STOCH_2]);
  }

  if(pLogicParams[LOGIC_IDX_EXIT_LOGIC] == PSAR_NORMAL || pLogicParams[LOGIC_IDX_EXIT_LOGIC] == PSAR_REVERSE ||  pLogicParams[LOGIC_IDX_ENTRY_LOGIC] == PSAR_NORMAL || pLogicParams[LOGIC_IDX_ENTRY_LOGIC] == PSAR_REVERSE)
  {
    retCode = TA_SAR(primaryShift1Index, primaryShift1Index, primaryRates->high, primaryRates->low, PSAR_ACCELERATION, PSAR_MAXIMUM, &outBegIdx, &outNBElement, &pLogicParams[LOGIC_IDX_PSAR_1]);
    if(retCode != TA_SUCCESS)
    {
      return logTALibError("TA_SAR()", retCode);
    }

    retCode = TA_SAR(primaryShift2Index, primaryShift2Index, primaryRates->high, primaryRates->low, PSAR_ACCELERATION, PSAR_MAXIMUM, &outBegIdx, &outNBElement, &pLogicParams[LOGIC_IDX_PSAR_2]);
    if(retCode != TA_SUCCESS)
    {
      return logTALibError("TA_SAR()", retCode);
    }

    pantheios_logprintf(PANTHEIOS_SEV_INFORMATIONAL, (PAN_CHAR_T*)"PSAR1 = %lf, PSAR2 = %lf", pLogicParams[LOGIC_IDX_PSAR_1], pLogicParams[LOGIC_IDX_PSAR_2]);
  }

  if(pLogicParams[LOGIC_IDX_EXIT_LOGIC] == BB_NORMAL || pLogicParams[LOGIC_IDX_EXIT_LOGIC] == BB_REVERSE ||  pLogicParams[LOGIC_IDX_ENTRY_LOGIC] == BB_NORMAL || pLogicParams[LOGIC_IDX_ENTRY_LOGIC] == BB_REVERSE)
  {
    retCode = TA_BBANDS(primaryShift1Index, primaryShift1Index, primaryRates->close, indicatorSlowPeriod, BBANDS_DEVIATION, BBANDS_DEVIATION, BBANDS_MA_TYPE, 
      &outBegIdx, &outNBElement, &pLogicParams[LOGIC_IDX_BBANDS_UPPER_1], &notUsed, &pLogicParams[LOGIC_IDX_BBANDS_LOWER_1]);
    if(retCode != TA_SUCCESS)
    {
      return logTALibError("TA_BBANDS()", retCode);
    }

    retCode = TA_BBANDS(primaryShift2Index, primaryShift2Index, primaryRates->close, indicatorSlowPeriod, BBANDS_DEVIATION, BBANDS_DEVIATION, BBANDS_MA_TYPE, 
      &outBegIdx, &outNBElement, &pLogicParams[LOGIC_IDX_BBANDS_UPPER_2], &notUsed, &pLogicParams[LOGIC_IDX_BBANDS_LOWER_2]);
    if(retCode != TA_SUCCESS)
    {
      return logTALibError("TA_BBANDS()", retCode);
    }

    pantheios_logprintf(PANTHEIOS_SEV_INFORMATIONAL, (PAN_CHAR_T*)"BB_UP_1 = %lf, BB_UP_2= %lf, BB_DOWN_1 = %lf, BB_DOWN_2= %lf", pLogicParams[LOGIC_IDX_BBANDS_LOWER_1], pLogicParams[LOGIC_IDX_BBANDS_UPPER_1], pLogicParams[LOGIC_IDX_BBANDS_LOWER_2], pLogicParams[LOGIC_IDX_BBANDS_UPPER_2]);
  }

  if(pLogicParams[LOGIC_IDX_EXIT_LOGIC] == MACD_NORMAL || pLogicParams[LOGIC_IDX_EXIT_LOGIC] == MACD_REVERSE ||  pLogicParams[LOGIC_IDX_ENTRY_LOGIC] == MACD_NORMAL || pLogicParams[LOGIC_IDX_ENTRY_LOGIC] == MACD_REVERSE)
  {


    retCode = TA_MACD(primaryShift1Index, primaryShift1Index, primaryRates->open, MACD_FAST_PERIOD, (int)indicatorSlowPeriod, MACD_SIGNAL_PERIOD, &outBegIdx, &outNBElement, &pLogicParams[LOGIC_IDX_MACD_2], &notUsed, &notUsed);
    if(retCode != TA_SUCCESS)
    {
      return logTALibError("TA_MACD()", retCode);
    }

    retCode = TA_MACD(primaryShift0Index, primaryShift0Index, primaryRates->open, MACD_FAST_PERIOD, (int)indicatorSlowPeriod, MACD_SIGNAL_PERIOD, &outBegIdx, &outNBElement, &pLogicParams[LOGIC_IDX_MACD_1], &notUsed, &notUsed);
    if(retCode != TA_SUCCESS)
    {
      return logTALibError("TA_MACD()", retCode);
    }

    pantheios_logprintf(PANTHEIOS_SEV_INFORMATIONAL, (PAN_CHAR_T*)"MACD1 = %lf, MACD2 = %lf", pLogicParams[LOGIC_IDX_MACD_1], pLogicParams[LOGIC_IDX_MACD_2]);
  }

  if(pLogicParams[LOGIC_IDX_EXIT_LOGIC] == CCI_NORMAL || pLogicParams[LOGIC_IDX_EXIT_LOGIC] == CCI_REVERSE ||  pLogicParams[LOGIC_IDX_ENTRY_LOGIC] == CCI_NORMAL || pLogicParams[LOGIC_IDX_ENTRY_LOGIC] == CCI_REVERSE)
  {
    retCode = TA_CCI(primaryShift1Index, primaryShift1Index, primaryRates->high, primaryRates->low, primaryRates->close, CCI_PERIOD, &outBegIdx, &outNBElement, &pLogicParams[LOGIC_IDX_CCI_1]);
    if(retCode != TA_SUCCESS)
    {
      return logTALibError("TA_CCI()", retCode);
    }

    retCode = TA_CCI(primaryShift2Index, primaryShift2Index, primaryRates->high, primaryRates->low, primaryRates->close, CCI_PERIOD, &outBegIdx, &outNBElement, &pLogicParams[LOGIC_IDX_CCI_2]);
    if(retCode != TA_SUCCESS)
    {
      return logTALibError("TA_CCI()", retCode);
    }

    pantheios_logprintf(PANTHEIOS_SEV_INFORMATIONAL, (PAN_CHAR_T*)"CCI1 = %lf, CCI2 = %lf", pLogicParams[LOGIC_IDX_CCI_1], pLogicParams[LOGIC_IDX_CCI_2]);
  }

  safe_timeString(timeString, pParams->ratesBuffers->rates[PRIMARY_RATES].time[primaryShift0Index]);
  pantheios_logprintf(PANTHEIOS_SEV_INFORMATIONAL, (PAN_CHAR_T*)"Coatl InstanceID = %d, BarTime = %s, ATR = %lf", (int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, pLogicParams[LOGIC_IDX_ATR]);

  return setUIValues(pParams, pLogicParams);
}

static BOOL longEntry(const double* pLogicParams)
{
  return (((pLogicParams[LOGIC_IDX_ENTRY_LOGIC] == BIG_BAR_NORMAL)  && (pLogicParams[LOGIC_IDX_BAR_SIZE]       >       pLogicParams[LOGIC_IDX_ATR])            && (pLogicParams[LOGIC_IDX_CLOSE_1]        >        pLogicParams[LOGIC_IDX_OPEN_1]))
    ||    ((pLogicParams[LOGIC_IDX_ENTRY_LOGIC] == BIG_BAR_REVERSE) && (pLogicParams[LOGIC_IDX_BAR_SIZE]       >       pLogicParams[LOGIC_IDX_ATR])            && (pLogicParams[LOGIC_IDX_CLOSE_1]        <        pLogicParams[LOGIC_IDX_OPEN_1]))
    ||    ((pLogicParams[LOGIC_IDX_ENTRY_LOGIC] == RSI_NORMAL)      && (pLogicParams[LOGIC_IDX_RSI_1]          >       pLogicParams[LOGIC_IDX_ENTRY_SYMMETRY]) && (pLogicParams[LOGIC_IDX_RSI_2]          <=       pLogicParams[LOGIC_IDX_ENTRY_SYMMETRY]))
    ||    ((pLogicParams[LOGIC_IDX_ENTRY_LOGIC] == RSI_REVERSE)     && (pLogicParams[LOGIC_IDX_RSI_1]          < 100 - pLogicParams[LOGIC_IDX_ENTRY_SYMMETRY]) && (pLogicParams[LOGIC_IDX_RSI_2]          >= 100 - pLogicParams[LOGIC_IDX_ENTRY_SYMMETRY]))
    ||    ((pLogicParams[LOGIC_IDX_ENTRY_LOGIC] == STOCH_NORMAL)    && (pLogicParams[LOGIC_IDX_STOCH_1]        >       pLogicParams[LOGIC_IDX_ENTRY_SYMMETRY]) && (pLogicParams[LOGIC_IDX_STOCH_2]        <=       pLogicParams[LOGIC_IDX_ENTRY_SYMMETRY]))
    ||    ((pLogicParams[LOGIC_IDX_ENTRY_LOGIC] == STOCH_REVERSE)   && (pLogicParams[LOGIC_IDX_STOCH_1]        < 100 - pLogicParams[LOGIC_IDX_ENTRY_SYMMETRY]) && (pLogicParams[LOGIC_IDX_STOCH_2]        >= 100 - pLogicParams[LOGIC_IDX_ENTRY_SYMMETRY]))
    ||    ((pLogicParams[LOGIC_IDX_ENTRY_LOGIC] == PSAR_NORMAL)     && (pLogicParams[LOGIC_IDX_PSAR_1]         <       pLogicParams[LOGIC_IDX_CLOSE_1])        && (pLogicParams[LOGIC_IDX_PSAR_2]         >=       pLogicParams[LOGIC_IDX_CLOSE_2]))
    ||    ((pLogicParams[LOGIC_IDX_ENTRY_LOGIC] == PSAR_REVERSE)    && (pLogicParams[LOGIC_IDX_PSAR_1]         >       pLogicParams[LOGIC_IDX_CLOSE_1])        && (pLogicParams[LOGIC_IDX_PSAR_2]         <=       pLogicParams[LOGIC_IDX_CLOSE_2]))
    ||    ((pLogicParams[LOGIC_IDX_ENTRY_LOGIC] == BB_NORMAL)       && (pLogicParams[LOGIC_IDX_BBANDS_UPPER_1] <       pLogicParams[LOGIC_IDX_CLOSE_1])        && (pLogicParams[LOGIC_IDX_BBANDS_UPPER_2] >=       pLogicParams[LOGIC_IDX_CLOSE_2]))
    ||    ((pLogicParams[LOGIC_IDX_ENTRY_LOGIC] == BB_REVERSE)      && (pLogicParams[LOGIC_IDX_BBANDS_LOWER_1] >       pLogicParams[LOGIC_IDX_CLOSE_1])        && (pLogicParams[LOGIC_IDX_BBANDS_LOWER_2] <=       pLogicParams[LOGIC_IDX_CLOSE_2]))
    ||    ((pLogicParams[LOGIC_IDX_ENTRY_LOGIC] == MACD_NORMAL)     && (pLogicParams[LOGIC_IDX_MACD_1]         >       0)                                      && (pLogicParams[LOGIC_IDX_MACD_2]         <=       0))
    ||    ((pLogicParams[LOGIC_IDX_ENTRY_LOGIC] == MACD_REVERSE)    && (pLogicParams[LOGIC_IDX_MACD_1]         <       0)                                      && (pLogicParams[LOGIC_IDX_MACD_2]         >=       0))
    ||    ((pLogicParams[LOGIC_IDX_ENTRY_LOGIC] == CCI_NORMAL)      && (pLogicParams[LOGIC_IDX_CCI_1]          >       0)                                      && (pLogicParams[LOGIC_IDX_CCI_2]          <=       0))
    ||    ((pLogicParams[LOGIC_IDX_ENTRY_LOGIC] == CCI_REVERSE)     && (pLogicParams[LOGIC_IDX_CCI_1]          <       0)                                      && (pLogicParams[LOGIC_IDX_CCI_2]          >=       0))
    ||    ((pLogicParams[LOGIC_IDX_ENTRY_LOGIC] == APO_NORMAL)      && (pLogicParams[LOGIC_IDX_APO_1]          >       0)                                      && (pLogicParams[LOGIC_IDX_APO_2]          <=       0))
    ||    ((pLogicParams[LOGIC_IDX_ENTRY_LOGIC] == APO_REVERSE)     && (pLogicParams[LOGIC_IDX_APO_1]          <       0)                                      && (pLogicParams[LOGIC_IDX_APO_2]          >=       0))
    ||    ((pLogicParams[LOGIC_IDX_ENTRY_LOGIC] == AROON_NORMAL)    && (pLogicParams[LOGIC_IDX_AROON_UP_1]     >       pLogicParams[LOGIC_IDX_AROON_DOWN_1])   && (pLogicParams[LOGIC_IDX_AROON_UP_2]     <=       pLogicParams[LOGIC_IDX_AROON_DOWN_2]))
    ||    ((pLogicParams[LOGIC_IDX_ENTRY_LOGIC] == AROON_REVERSE)   && (pLogicParams[LOGIC_IDX_AROON_UP_1]     <       pLogicParams[LOGIC_IDX_AROON_DOWN_1])   && (pLogicParams[LOGIC_IDX_AROON_UP_2]     >=       pLogicParams[LOGIC_IDX_AROON_DOWN_2])));
}

static BOOL shortEntry(const double* pLogicParams)
{
  return (((pLogicParams[LOGIC_IDX_ENTRY_LOGIC] == BIG_BAR_REVERSE) && (pLogicParams[LOGIC_IDX_BAR_SIZE]       >       pLogicParams[LOGIC_IDX_ATR])            && (pLogicParams[LOGIC_IDX_CLOSE_1]        >        pLogicParams[LOGIC_IDX_OPEN_1]))
    ||    ((pLogicParams[LOGIC_IDX_ENTRY_LOGIC] == BIG_BAR_NORMAL)  && (pLogicParams[LOGIC_IDX_BAR_SIZE]       >       pLogicParams[LOGIC_IDX_ATR])            && (pLogicParams[LOGIC_IDX_CLOSE_1]        <        pLogicParams[LOGIC_IDX_OPEN_1]))
    ||    ((pLogicParams[LOGIC_IDX_ENTRY_LOGIC] == RSI_REVERSE)     && (pLogicParams[LOGIC_IDX_RSI_1]          >       pLogicParams[LOGIC_IDX_ENTRY_SYMMETRY]) && (pLogicParams[LOGIC_IDX_RSI_2]          <=       pLogicParams[LOGIC_IDX_ENTRY_SYMMETRY]))
    ||    ((pLogicParams[LOGIC_IDX_ENTRY_LOGIC] == RSI_NORMAL)      && (pLogicParams[LOGIC_IDX_RSI_1]          < 100 - pLogicParams[LOGIC_IDX_ENTRY_SYMMETRY]) && (pLogicParams[LOGIC_IDX_RSI_2]          >= 100 - pLogicParams[LOGIC_IDX_ENTRY_SYMMETRY]))
    ||    ((pLogicParams[LOGIC_IDX_ENTRY_LOGIC] == STOCH_REVERSE)   && (pLogicParams[LOGIC_IDX_STOCH_1]        >       pLogicParams[LOGIC_IDX_ENTRY_SYMMETRY]) && (pLogicParams[LOGIC_IDX_STOCH_2]        <=       pLogicParams[LOGIC_IDX_ENTRY_SYMMETRY]))
    ||    ((pLogicParams[LOGIC_IDX_ENTRY_LOGIC] == STOCH_NORMAL)    && (pLogicParams[LOGIC_IDX_STOCH_1]        < 100 - pLogicParams[LOGIC_IDX_ENTRY_SYMMETRY]) && (pLogicParams[LOGIC_IDX_STOCH_2]        >= 100 - pLogicParams[LOGIC_IDX_ENTRY_SYMMETRY]))
    ||    ((pLogicParams[LOGIC_IDX_ENTRY_LOGIC] == PSAR_REVERSE)    && (pLogicParams[LOGIC_IDX_PSAR_1]         <       pLogicParams[LOGIC_IDX_CLOSE_1])        && (pLogicParams[LOGIC_IDX_PSAR_2]         >=       pLogicParams[LOGIC_IDX_CLOSE_2]))
    ||    ((pLogicParams[LOGIC_IDX_ENTRY_LOGIC] == PSAR_NORMAL)     && (pLogicParams[LOGIC_IDX_PSAR_1]         >       pLogicParams[LOGIC_IDX_CLOSE_1])        && (pLogicParams[LOGIC_IDX_PSAR_2]         <=       pLogicParams[LOGIC_IDX_CLOSE_2]))
    ||    ((pLogicParams[LOGIC_IDX_ENTRY_LOGIC] == BB_REVERSE)      && (pLogicParams[LOGIC_IDX_BBANDS_UPPER_1] <       pLogicParams[LOGIC_IDX_CLOSE_1])        && (pLogicParams[LOGIC_IDX_BBANDS_UPPER_2] >=       pLogicParams[LOGIC_IDX_CLOSE_2]))
    ||    ((pLogicParams[LOGIC_IDX_ENTRY_LOGIC] == BB_NORMAL)       && (pLogicParams[LOGIC_IDX_BBANDS_LOWER_1] >       pLogicParams[LOGIC_IDX_CLOSE_1])        && (pLogicParams[LOGIC_IDX_BBANDS_LOWER_2] <=       pLogicParams[LOGIC_IDX_CLOSE_2]))
    ||    ((pLogicParams[LOGIC_IDX_ENTRY_LOGIC] == MACD_REVERSE)    && (pLogicParams[LOGIC_IDX_MACD_1]         >       0)                                      && (pLogicParams[LOGIC_IDX_MACD_2]         <=       0))
    ||    ((pLogicParams[LOGIC_IDX_ENTRY_LOGIC] == MACD_NORMAL)     && (pLogicParams[LOGIC_IDX_MACD_1]         <       0)                                      && (pLogicParams[LOGIC_IDX_MACD_2]         >=       0))
    ||    ((pLogicParams[LOGIC_IDX_ENTRY_LOGIC] == CCI_REVERSE)     && (pLogicParams[LOGIC_IDX_CCI_1]          >       0)                                      && (pLogicParams[LOGIC_IDX_CCI_2]          <=       0))
    ||    ((pLogicParams[LOGIC_IDX_ENTRY_LOGIC] == CCI_NORMAL)      && (pLogicParams[LOGIC_IDX_CCI_1]          <       0)                                      && (pLogicParams[LOGIC_IDX_CCI_2]          >=       0))
    ||    ((pLogicParams[LOGIC_IDX_ENTRY_LOGIC] == APO_REVERSE)     && (pLogicParams[LOGIC_IDX_APO_1]          >       0)                                      && (pLogicParams[LOGIC_IDX_APO_2]          <=       0))
    ||    ((pLogicParams[LOGIC_IDX_ENTRY_LOGIC] == APO_NORMAL)      && (pLogicParams[LOGIC_IDX_APO_1]          <       0)                                      && (pLogicParams[LOGIC_IDX_APO_2]          >=       0))
    ||    ((pLogicParams[LOGIC_IDX_ENTRY_LOGIC] == AROON_REVERSE)   && (pLogicParams[LOGIC_IDX_AROON_UP_1]     >       pLogicParams[LOGIC_IDX_AROON_DOWN_1])   && (pLogicParams[LOGIC_IDX_AROON_UP_2]     <=       pLogicParams[LOGIC_IDX_AROON_DOWN_2]))
    ||    ((pLogicParams[LOGIC_IDX_ENTRY_LOGIC] == AROON_NORMAL)    && (pLogicParams[LOGIC_IDX_AROON_UP_1]     <       pLogicParams[LOGIC_IDX_AROON_DOWN_1])   && (pLogicParams[LOGIC_IDX_AROON_UP_2]     >=       pLogicParams[LOGIC_IDX_AROON_DOWN_2])));
}

static BOOL shortExit(const double* pLogicParams)
{
  return (((pLogicParams[LOGIC_IDX_EXIT_LOGIC]  == BIG_BAR_NORMAL)  && (pLogicParams[LOGIC_IDX_BAR_SIZE]       >       pLogicParams[LOGIC_IDX_ATR])            && (pLogicParams[LOGIC_IDX_CLOSE_1]        >        pLogicParams[LOGIC_IDX_OPEN_1]))
    ||    ((pLogicParams[LOGIC_IDX_EXIT_LOGIC]  == BIG_BAR_REVERSE) && (pLogicParams[LOGIC_IDX_BAR_SIZE]       >       pLogicParams[LOGIC_IDX_ATR])            && (pLogicParams[LOGIC_IDX_CLOSE_1]        <        pLogicParams[LOGIC_IDX_OPEN_1]))
    ||    ((pLogicParams[LOGIC_IDX_EXIT_LOGIC]  == RSI_NORMAL)      && (pLogicParams[LOGIC_IDX_RSI_1]          >       pLogicParams[LOGIC_IDX_EXIT_SYMMETRY])  && (pLogicParams[LOGIC_IDX_RSI_2]          <=       pLogicParams[LOGIC_IDX_EXIT_SYMMETRY]))
    ||    ((pLogicParams[LOGIC_IDX_EXIT_LOGIC]  == RSI_REVERSE)     && (pLogicParams[LOGIC_IDX_RSI_1]          < 100 - pLogicParams[LOGIC_IDX_EXIT_SYMMETRY])  && (pLogicParams[LOGIC_IDX_RSI_2]          >= 100 - pLogicParams[LOGIC_IDX_EXIT_SYMMETRY]))
    ||    ((pLogicParams[LOGIC_IDX_EXIT_LOGIC]  == STOCH_NORMAL)    && (pLogicParams[LOGIC_IDX_STOCH_1]        >       pLogicParams[LOGIC_IDX_EXIT_SYMMETRY])  && (pLogicParams[LOGIC_IDX_STOCH_2]        <=       pLogicParams[LOGIC_IDX_EXIT_SYMMETRY]))
    ||    ((pLogicParams[LOGIC_IDX_EXIT_LOGIC]  == STOCH_REVERSE)   && (pLogicParams[LOGIC_IDX_STOCH_1]        < 100 - pLogicParams[LOGIC_IDX_EXIT_SYMMETRY])  && (pLogicParams[LOGIC_IDX_STOCH_2]        >= 100 - pLogicParams[LOGIC_IDX_EXIT_SYMMETRY]))
    ||    ((pLogicParams[LOGIC_IDX_EXIT_LOGIC]  == PSAR_NORMAL)     && (pLogicParams[LOGIC_IDX_PSAR_1]         <       pLogicParams[LOGIC_IDX_CLOSE_1])        && (pLogicParams[LOGIC_IDX_PSAR_2]         >=       pLogicParams[LOGIC_IDX_CLOSE_2]))
    ||    ((pLogicParams[LOGIC_IDX_EXIT_LOGIC]  == PSAR_REVERSE)    && (pLogicParams[LOGIC_IDX_PSAR_1]         >       pLogicParams[LOGIC_IDX_CLOSE_1])        && (pLogicParams[LOGIC_IDX_PSAR_2]         <=       pLogicParams[LOGIC_IDX_CLOSE_2]))
    ||    ((pLogicParams[LOGIC_IDX_EXIT_LOGIC]  == BB_NORMAL)       && (pLogicParams[LOGIC_IDX_BBANDS_UPPER_1] <       pLogicParams[LOGIC_IDX_CLOSE_1])        && (pLogicParams[LOGIC_IDX_BBANDS_UPPER_2] >=       pLogicParams[LOGIC_IDX_CLOSE_2]))
    ||    ((pLogicParams[LOGIC_IDX_EXIT_LOGIC]  == BB_REVERSE)      && (pLogicParams[LOGIC_IDX_BBANDS_LOWER_1] >       pLogicParams[LOGIC_IDX_CLOSE_1])        && (pLogicParams[LOGIC_IDX_BBANDS_LOWER_2] <=       pLogicParams[LOGIC_IDX_CLOSE_2]))
    ||    ((pLogicParams[LOGIC_IDX_EXIT_LOGIC]  == MACD_NORMAL)     && (pLogicParams[LOGIC_IDX_MACD_1]         >       0)                                      && (pLogicParams[LOGIC_IDX_MACD_2]         <=       0))
    ||    ((pLogicParams[LOGIC_IDX_EXIT_LOGIC]  == MACD_REVERSE)    && (pLogicParams[LOGIC_IDX_MACD_1]         <       0)                                      && (pLogicParams[LOGIC_IDX_MACD_2]         >=       0))
    ||    ((pLogicParams[LOGIC_IDX_EXIT_LOGIC]  == CCI_NORMAL)      && (pLogicParams[LOGIC_IDX_CCI_1]          >       0)                                      && (pLogicParams[LOGIC_IDX_CCI_2]          <=       0))
    ||    ((pLogicParams[LOGIC_IDX_EXIT_LOGIC]  == CCI_REVERSE)     && (pLogicParams[LOGIC_IDX_CCI_1]          <       0)                                      && (pLogicParams[LOGIC_IDX_CCI_2]          >=       0))
    ||    ((pLogicParams[LOGIC_IDX_EXIT_LOGIC]  == APO_NORMAL)      && (pLogicParams[LOGIC_IDX_APO_1]          >       0)                                      && (pLogicParams[LOGIC_IDX_APO_2]          <=       0))
    ||    ((pLogicParams[LOGIC_IDX_EXIT_LOGIC]  == APO_REVERSE)     && (pLogicParams[LOGIC_IDX_APO_1]          <       0)                                      && (pLogicParams[LOGIC_IDX_APO_2]          >=       0))
    ||    ((pLogicParams[LOGIC_IDX_EXIT_LOGIC]  == AROON_NORMAL)    && (pLogicParams[LOGIC_IDX_AROON_UP_1]     >       pLogicParams[LOGIC_IDX_AROON_DOWN_1])   && (pLogicParams[LOGIC_IDX_AROON_UP_2]     <=       pLogicParams[LOGIC_IDX_AROON_DOWN_2]))
    ||    ((pLogicParams[LOGIC_IDX_EXIT_LOGIC]  == AROON_REVERSE)   && (pLogicParams[LOGIC_IDX_AROON_UP_1]     <       pLogicParams[LOGIC_IDX_AROON_DOWN_1])   && (pLogicParams[LOGIC_IDX_AROON_UP_2]     >=       pLogicParams[LOGIC_IDX_AROON_DOWN_2])));
}

static BOOL longExit(const double* pLogicParams)
{
  return (((pLogicParams[LOGIC_IDX_EXIT_LOGIC]  == BIG_BAR_REVERSE) && (pLogicParams[LOGIC_IDX_BAR_SIZE]       >       pLogicParams[LOGIC_IDX_ATR])            && (pLogicParams[LOGIC_IDX_CLOSE_1]        >        pLogicParams[LOGIC_IDX_OPEN_1]))
    ||    ((pLogicParams[LOGIC_IDX_EXIT_LOGIC]  == BIG_BAR_NORMAL)  && (pLogicParams[LOGIC_IDX_BAR_SIZE]       >       pLogicParams[LOGIC_IDX_ATR])            && (pLogicParams[LOGIC_IDX_CLOSE_1]        <        pLogicParams[LOGIC_IDX_OPEN_1]))
    ||    ((pLogicParams[LOGIC_IDX_EXIT_LOGIC]  == RSI_REVERSE)     && (pLogicParams[LOGIC_IDX_RSI_1]          >       pLogicParams[LOGIC_IDX_EXIT_SYMMETRY])  && (pLogicParams[LOGIC_IDX_RSI_2]          <=       pLogicParams[LOGIC_IDX_EXIT_SYMMETRY]))
    ||    ((pLogicParams[LOGIC_IDX_EXIT_LOGIC]  == RSI_NORMAL)      && (pLogicParams[LOGIC_IDX_RSI_1]          < 100 - pLogicParams[LOGIC_IDX_EXIT_SYMMETRY])  && (pLogicParams[LOGIC_IDX_RSI_2]          >= 100 - pLogicParams[LOGIC_IDX_EXIT_SYMMETRY]))
    ||    ((pLogicParams[LOGIC_IDX_EXIT_LOGIC]  == STOCH_REVERSE)   && (pLogicParams[LOGIC_IDX_STOCH_1]        >       pLogicParams[LOGIC_IDX_EXIT_SYMMETRY])  && (pLogicParams[LOGIC_IDX_STOCH_2]        <=       pLogicParams[LOGIC_IDX_EXIT_SYMMETRY]))
    ||    ((pLogicParams[LOGIC_IDX_EXIT_LOGIC]  == STOCH_NORMAL)    && (pLogicParams[LOGIC_IDX_STOCH_1]        < 100 - pLogicParams[LOGIC_IDX_EXIT_SYMMETRY])  && (pLogicParams[LOGIC_IDX_STOCH_2]        >= 100 - pLogicParams[LOGIC_IDX_EXIT_SYMMETRY]))
    ||    ((pLogicParams[LOGIC_IDX_EXIT_LOGIC]  == PSAR_REVERSE)    && (pLogicParams[LOGIC_IDX_PSAR_1]         <       pLogicParams[LOGIC_IDX_CLOSE_1])        && (pLogicParams[LOGIC_IDX_PSAR_2]         >=       pLogicParams[LOGIC_IDX_CLOSE_2]))
    ||    ((pLogicParams[LOGIC_IDX_EXIT_LOGIC]  == PSAR_NORMAL)     && (pLogicParams[LOGIC_IDX_PSAR_1]         >       pLogicParams[LOGIC_IDX_CLOSE_1])        && (pLogicParams[LOGIC_IDX_PSAR_2]         <=       pLogicParams[LOGIC_IDX_CLOSE_2]))
    ||    ((pLogicParams[LOGIC_IDX_EXIT_LOGIC]  == BB_REVERSE)      && (pLogicParams[LOGIC_IDX_BBANDS_UPPER_1] <       pLogicParams[LOGIC_IDX_CLOSE_1])        && (pLogicParams[LOGIC_IDX_BBANDS_UPPER_2] >=       pLogicParams[LOGIC_IDX_CLOSE_2]))
    ||    ((pLogicParams[LOGIC_IDX_EXIT_LOGIC]  == BB_NORMAL)       && (pLogicParams[LOGIC_IDX_BBANDS_LOWER_1] >       pLogicParams[LOGIC_IDX_CLOSE_1])        && (pLogicParams[LOGIC_IDX_BBANDS_LOWER_2] <=       pLogicParams[LOGIC_IDX_CLOSE_2]))
    ||    ((pLogicParams[LOGIC_IDX_EXIT_LOGIC]  == MACD_REVERSE)    && (pLogicParams[LOGIC_IDX_MACD_1]         >       0)                                      && (pLogicParams[LOGIC_IDX_MACD_2]         <=       0))
    ||    ((pLogicParams[LOGIC_IDX_EXIT_LOGIC]  == MACD_NORMAL)     && (pLogicParams[LOGIC_IDX_MACD_1]         <       0)                                      && (pLogicParams[LOGIC_IDX_MACD_2]         >=       0))
    ||    ((pLogicParams[LOGIC_IDX_EXIT_LOGIC]  == CCI_REVERSE)     && (pLogicParams[LOGIC_IDX_CCI_1]          >       0)                                      && (pLogicParams[LOGIC_IDX_CCI_2]          <=       0))
    ||    ((pLogicParams[LOGIC_IDX_EXIT_LOGIC]  == CCI_NORMAL)      && (pLogicParams[LOGIC_IDX_CCI_1]          <       0)                                      && (pLogicParams[LOGIC_IDX_CCI_2]          >=       0))
    ||    ((pLogicParams[LOGIC_IDX_EXIT_LOGIC]  == APO_REVERSE)     && (pLogicParams[LOGIC_IDX_APO_1]          >       0)                                      && (pLogicParams[LOGIC_IDX_APO_2]          <=       0))
    ||    ((pLogicParams[LOGIC_IDX_EXIT_LOGIC]  == APO_NORMAL)      && (pLogicParams[LOGIC_IDX_APO_1]          <       0)                                      && (pLogicParams[LOGIC_IDX_APO_2]          >=       0))
    ||    ((pLogicParams[LOGIC_IDX_EXIT_LOGIC]  == AROON_REVERSE)   && (pLogicParams[LOGIC_IDX_AROON_UP_1]     >       pLogicParams[LOGIC_IDX_AROON_DOWN_1])   && (pLogicParams[LOGIC_IDX_AROON_UP_2]     <=       pLogicParams[LOGIC_IDX_AROON_DOWN_2]))
    ||    ((pLogicParams[LOGIC_IDX_EXIT_LOGIC]  == AROON_NORMAL)    && (pLogicParams[LOGIC_IDX_AROON_UP_1]     <       pLogicParams[LOGIC_IDX_AROON_DOWN_1])   && (pLogicParams[LOGIC_IDX_AROON_UP_2]     >=       pLogicParams[LOGIC_IDX_AROON_DOWN_2])));
}

static AsirikuyReturnCode handleTradeExits(StrategyParams* pParams, double* pLogicParams1, double* pLogicParams2, double takeProfit)
{
  AsirikuyReturnCode returnCode = SUCCESS;

  if(pParams == NULL)
  {
    pantheios_logputs(PANTHEIOS_SEV_CRITICAL, (PAN_CHAR_T*)"handleTradeExits() failed. pParams = NULL");
    return NULL_POINTER;
  }

  if(pLogicParams1 == NULL)
  {
    pantheios_logputs(PANTHEIOS_SEV_CRITICAL, (PAN_CHAR_T*)"handleTradeExits() failed. pLogicParams = NULL");
    return NULL_POINTER;
  }

  if(pLogicParams2 == NULL)
  {
    pantheios_logputs(PANTHEIOS_SEV_CRITICAL, (PAN_CHAR_T*)"handleTradeExits() failed. pLogicParams = NULL");
    return NULL_POINTER;
  }
  
  if(hasEntrySignal((int)pParams->results[0].tradingSignals) || hasUpdateSignal((int)pParams->results[0].tradingSignals))
  {
    return SUCCESS;
  }
  
  returnCode = checkInternalTP(pParams, PRIMARY_RATES, 0, takeProfit);
  if(returnCode != SUCCESS)
  {
    return logAsirikuyError("handleTradeExits()", returnCode);
  }

  returnCode = checkTimedExit(pParams, PRIMARY_RATES, 0, USE_INTERNAL_SL, USE_INTERNAL_TP);
  if(returnCode != SUCCESS)
  {
    return logAsirikuyError("handleTradeExits()", returnCode);
  }

  if((totalOpenOrders(pParams, BUY) != 0) && (((longExit(pLogicParams1)) && (longExit(pLogicParams2))) || ((longExit(pLogicParams1)) && (pLogicParams2[LOGIC_IDX_EXIT_LOGIC] == -1))))
  {
    return closeLongTrade(pParams, 0);
  }
  else if((totalOpenOrders(pParams, SELL) != 0) && (((shortExit(pLogicParams1)) && (shortExit(pLogicParams2))) || ((shortExit(pLogicParams1)) && (pLogicParams2[LOGIC_IDX_EXIT_LOGIC] == -1))))
  {
    return closeShortTrade(pParams, 0);
  }

  return SUCCESS;
}

static AsirikuyReturnCode handleTradeEntries(StrategyParams* pParams, double* pLogicParams1, double* pLogicParams2, double stopLoss, double takeProfit)
{
  AsirikuyReturnCode returnCode = SUCCESS;

  if(pParams == NULL)
  {
    pantheios_logputs(PANTHEIOS_SEV_CRITICAL, (PAN_CHAR_T*)"handleTradeEntries() failed. pParams = NULL");
    return NULL_POINTER;
  }

  if(pLogicParams1 == NULL)
  {
    pantheios_logputs(PANTHEIOS_SEV_CRITICAL, (PAN_CHAR_T*)"handleTradeEntries() failed. pLogicParams = NULL");
    return NULL_POINTER;
  }

  if(pLogicParams2 == NULL)
  {
    pantheios_logputs(PANTHEIOS_SEV_CRITICAL, (PAN_CHAR_T*)"handleTradeEntries() failed. pLogicParams = NULL");
    return NULL_POINTER;
  }

  if((longEntry(pLogicParams1) && longEntry(pLogicParams2)) || (((longEntry(pLogicParams1)) && (pLogicParams2[LOGIC_IDX_ENTRY_LOGIC] == -1))) )
  {
    return openOrUpdateLongTrade(pParams, PRIMARY_RATES, 0, stopLoss, takeProfit, 1,USE_INTERNAL_SL, USE_INTERNAL_TP);
  }
  else if((shortEntry(pLogicParams1) && shortEntry(pLogicParams2)) || (((shortEntry(pLogicParams1)) && (pLogicParams2[LOGIC_IDX_ENTRY_LOGIC] == -1))) )
  {
    return openOrUpdateShortTrade(pParams, PRIMARY_RATES, 0, stopLoss, takeProfit,1, USE_INTERNAL_SL, USE_INTERNAL_TP);
  }

  return SUCCESS;
}

AsirikuyReturnCode runCoatl(StrategyParams* pParams)
{
  AsirikuyReturnCode returnCode = SUCCESS;
  int    shift0Index = pParams->ratesBuffers->rates[PRIMARY_RATES].info.arraySize - 1;
  double stopLoss, takeProfit, logicParams1[TOTAL_LOGIC_PARAMETERS], logicParams2[TOTAL_LOGIC_PARAMETERS];
  
  if(pParams == NULL)
  {
    pantheios_logputs(PANTHEIOS_SEV_CRITICAL, (PAN_CHAR_T*)"runCoatl() failed. pParams = NULL");
    return NULL_POINTER;
  }

  returnCode = loadIndicators(pParams, logicParams1, 1);
  if(returnCode != SUCCESS)
  {
    return logAsirikuyError("runCoatl()", returnCode);
  }

  returnCode = loadIndicators(pParams, logicParams2, 2);
  if(returnCode != SUCCESS)
  {
    return logAsirikuyError("runCoatl()", returnCode);
  }
	
  stopLoss   = logicParams1[LOGIC_IDX_ATR] * pParams->settings[SL_ATR_MULTIPLIER];
  takeProfit = logicParams1[LOGIC_IDX_ATR] * pParams->settings[TP_ATR_MULTIPLIER];

  /* if orders were not set properly (SL or TP = 0) then set stops so
     that the front-end can properly set these values.            */ 
  if(!areOrdersCorrect(pParams, pParams->settings[USE_SL], pParams->settings[USE_TP]))
  {
    setStops(pParams, PRIMARY_RATES, 0, stopLoss, takeProfit, USE_INTERNAL_SL, USE_INTERNAL_TP);
  }

  returnCode = handleTradeEntries(pParams, logicParams1, logicParams2, stopLoss, takeProfit);
  if(returnCode != SUCCESS)
  {
    return logAsirikuyError("runCoatl()", returnCode);
  }
  
  returnCode = handleTradeExits(pParams, logicParams1, logicParams2, takeProfit);
  if(returnCode != SUCCESS)
  {
    return logAsirikuyError("runCoatl()", returnCode);
  }
  
  return SUCCESS;
}
