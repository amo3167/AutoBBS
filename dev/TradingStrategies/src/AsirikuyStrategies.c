/**
 * @file
 * @brief     An interface for all of the trading strategies.
 * @details   Entry point for all strategies and some common functions. Call runStrategy() instead of running an individual strategy directly.
 * 
 * @author    Morgan Doel (Initial implementation)
 * @author    Daniel Fernandez (Assisted with design and code styling)
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
#include "AsirikuyStrategies.h"
#include "CriticalSection.h"
#include "Logging.h"
#include "EquityLog.h"
#include "EasyTradeCWrapper.hpp"
#include "InstanceStates.h"
#include "OrderSignals.h"
#include "TradingWeekBoundaries.h"
#include "OrderManagement.h"
#include "StrategyUserInterface.h"
#include "AsirikuyTime.h"

#include "RecordBars.h"
#include "Screening.h"
#include "TakeOver.h"
#include "Screening.h"
#include "AutoBBS.h"
#include "TrendLimit.h"

typedef enum strategyId_t
{
  RECORD_BARS       = 19,
  TAKEOVER			= 26,
  SCREENING			= 27,
  AUTOBBS			= 29,
  AUTOBBSWEEKLY		= 30,
  TRENDLIMIT		= 31
} StrategyId;

AsirikuyReturnCode getStrategyFunctions(StrategyParams* pParams, AsirikuyReturnCode(**runStrategyFunc)(StrategyParams*))
{
  switch((int)pParams->settings[INTERNAL_STRATEGY_ID])
  {
  case RECORD_BARS:
    {
      *runStrategyFunc            = &runRecordBars;
      return SUCCESS;
    }
  case TAKEOVER:
  {
	  *runStrategyFunc = &runTakeOver;
	  return SUCCESS;
  }
  case SCREENING:
  {
	  *runStrategyFunc = &runScreening;
	  return SUCCESS;
  }
  case AUTOBBS:
  {
	  *runStrategyFunc = &runAutoBBS;
	  return SUCCESS;
  }
  case AUTOBBSWEEKLY:
  {
	  *runStrategyFunc = &runAutoBBS;
	  return SUCCESS;
  }
  case TRENDLIMIT:
  {
	  *runStrategyFunc = &runTrendLimit;
	  return SUCCESS;
  }
  default:
    {
      return INVALID_STRATEGY;
    }
  }
}

static AsirikuyReturnCode validateCommonStrategySettings(StrategyParams* pParams)
{
  if(pParams == NULL)
  {
    fprintf(stderr, "[CRITICAL] validateCommonStrategySettings() failed. pParams = NULL\n\n");
    return NULL_POINTER;
  }

  if((int)pParams->settings[MAX_OPEN_ORDERS] <= -EPSILON)
  {
    fprintf(stderr, "[ERROR] validateCommonStrategySettings() failed. MAX_OPEN_ORDERS must be greater than 0. MAX_OPEN_ORDERS = ", (int)pParams->settings[MAX_OPEN_ORDERS]);
    return INVALID_PARAMETER;
  }

  if(((int)pParams->settings[IS_BACKTESTING] != TRUE) && ((int)pParams->settings[IS_BACKTESTING] != FALSE))
  {
    fprintf(stderr, "[ERROR] validateCommonStrategySettings() failed. IS_BACKTESTING must be set to 0 or 1. IS_BACKTESTING = %d", (int)pParams->settings[IS_BACKTESTING]);
    return INVALID_PARAMETER;
  }

  if(((int)pParams->settings[DISABLE_COMPOUNDING] != TRUE) && ((int)pParams->settings[DISABLE_COMPOUNDING] != FALSE))
  {
    fprintf(stderr, "[ERROR] validateCommonStrategySettings() failed. DISABLE_COMPOUNDING must be set to 0 or 1. DISABLE_COMPOUNDING = %d", (int)pParams->settings[DISABLE_COMPOUNDING]);
    return INVALID_PARAMETER;
  }

  if((int)pParams->settings[TIMED_EXIT_BARS] < -EPSILON)
  {
    fprintf(stderr, "[ERROR] validateCommonStrategySettings() failed. TIMED_EXIT_BARS must be greater than or equal to 0. TIMED_EXIT_BARS = %d", (int)pParams->settings[TIMED_EXIT_BARS]);
    return INVALID_PARAMETER;
  }

  if(((int)pParams->settings[OPERATIONAL_MODE] < MODE_DISABLE) || ((int)pParams->settings[OPERATIONAL_MODE] > MODE_MONITOR))
  {
    fprintf(stderr, "[ERROR] validateCommonStrategySettings() failed. OPERATIONAL_MODE must be set to 0, 1, or 2. OPERATIONAL_MODE = %d", (int)pParams->settings[OPERATIONAL_MODE]);
    return INVALID_PARAMETER;
  }

  if((int)pParams->settings[STRATEGY_INSTANCE_ID] < -EPSILON)
  {
    fprintf(stderr, "[ERROR] validateCommonStrategySettings() failed. STRATEGY_INSTANCE_ID must be greater than or equal to 0. STRATEGY_INSTANCE_ID = ", (int)pParams->settings[STRATEGY_INSTANCE_ID]);
    return INVALID_PARAMETER;
  }

  if((int)pParams->settings[TIMEFRAME] <= -EPSILON)
  {
    fprintf(stderr, "[ERROR] validateCommonStrategySettings() failed. TIMEFRAME must be greater than 0. TIMEFRAME = %d", (int)pParams->settings[TIMEFRAME]);
    return INVALID_PARAMETER;
  }

  if(pParams->settings[ACCOUNT_RISK_PERCENT] <= -EPSILON)
  {
    fprintf(stderr, "[ERROR] validateCommonStrategySettings() failed. ACCOUNT_RISK_PERCENT must be greater than 0. ACCOUNT_RISK_PERCENT = %lf", pParams->settings[ACCOUNT_RISK_PERCENT]);
    return INVALID_PARAMETER;
  }

  if(pParams->settings[MAX_DRAWDOWN_PERCENT] <= -EPSILON)
  {
    fprintf(stderr, "[ERROR] validateCommonStrategySettings() failed. MAX_DRAWDOWN_PERCENT must be greater than 0. MAX_DRAWDOWN_PERCENT = %lf", pParams->settings[MAX_DRAWDOWN_PERCENT]);
    return INVALID_PARAMETER;
  }

  if(pParams->settings[MAX_SPREAD] <= -EPSILON)
  {
    fprintf(stderr, "[ERROR] validateCommonStrategySettings() failed. MAX_SPREAD must be greater than 0. MAX_SPREAD = %lf", pParams->settings[MAX_SPREAD]);
    return INVALID_PARAMETER;
  }

  if(pParams->settings[SL_ATR_MULTIPLIER] < -EPSILON)
  {
    fprintf(stderr, "[ERROR] validateCommonStrategySettings() failed. SL_ATR_MULTIPLIER must be greater than or equal to 0. SL_ATR_MULTIPLIER = %lf", pParams->settings[SL_ATR_MULTIPLIER]);
    return INVALID_PARAMETER;
  }

  if(pParams->settings[TP_ATR_MULTIPLIER] < -EPSILON)
  {
    fprintf(stderr, "[ERROR] validateCommonStrategySettings() failed. TP_ATR_MULTIPLIER must be greater than or equal to 0. TP_ATR_MULTIPLIER = %lf", pParams->settings[TP_ATR_MULTIPLIER]);
    return INVALID_PARAMETER;
  }

  if((int)pParams->settings[ATR_AVERAGING_PERIOD] < -EPSILON)
  {
    fprintf(stderr, "[ERROR] validateCommonStrategySettings() failed. ATR_AVERAGING_PERIOD must be greater than 0. ATR_AVERAGING_PERIOD = %d", (int)pParams->settings[ATR_AVERAGING_PERIOD]);
    return INVALID_PARAMETER;
  }

  if((int)pParams->settings[ORDERINFO_ARRAY_SIZE] < -EPSILON)
  {
    fprintf(stderr, "[ERROR] validateCommonStrategySettings() failed. ORDERINFO_ARRAY_SIZE must be greater than or equal to 0. ORDERINFO_ARRAY_SIZE = %d", (int)pParams->settings[ORDERINFO_ARRAY_SIZE]);
    return INVALID_PARAMETER;
  }

  return SUCCESS;
}

AsirikuyReturnCode runStrategy(StrategyParams* pParams)
{
  AsirikuyReturnCode returnCode;
  AsirikuyReturnCode (*runStrategyFunc)(StrategyParams*);
  int i;
  struct tm timeInfo1;
  char   timeString[MAX_TIME_STRING_SIZE] = "";
  if(pParams == NULL)
  {
    fprintf(stderr, "[CRITICAL] runStrategy() failed. pParams = NULL\n\n");
    return NULL_POINTER;
  }

  /* Check that Bid/Ask are not zero */
  if(pParams->bidAsk.bid[0] < EPSILON || pParams->bidAsk.ask[0] < EPSILON)
  {
	fprintf(stderr, "[CRITICAL] runStrategy() failed. Bid or Ask is zero\n\n");
    return BID_ASK_IS_ZERO;
  }

  /* initialize the easy trade library */
  initEasyTradeLibrary(pParams);
  //As XAUUSD quote on 1am, but trading on 1:02 am, it causes some gap 
  if (strstr(pParams->tradeSymbol, "XAUUSD") != NULL && (BOOL)pParams->settings[IS_BACKTESTING] == FALSE)
  {
	  safe_gmtime(&timeInfo1, pParams->currentBrokerTime);
	  safe_timeString(timeString, pParams->currentBrokerTime);

	  if (timeInfo1.tm_hour == 1 && timeInfo1.tm_min < 2)
	  {
		  fprintf(stderr, "[INFO] Start skiping tick on XAUUSD on %s", timeString);
		  return SUCCESS;
	  }

  }

  if (!isValidTradingTime(pParams,pParams->currentBrokerTime))
  {

    safe_timeString(timeString, pParams->currentBrokerTime);
    fprintf(stderr, "[CRITICAL] runStrategy() failed. Invalid trading time: %s", timeString);
	return SUCCESS;
  }
    
  /* Save tick data */
  if(pParams->settings[SAVE_TICK_DATA] != 0) {
	saveTickData();
  }

  /* Log additional information to study entries */
  // TODO: here it only loads current strategy orders...... we may need to load all orders. 
  for(i = 0; i < pParams->settings[ORDERINFO_ARRAY_SIZE]; i++)
  {
	  if (pParams->orderInfo[i].openTime == openTime(1) && pParams->orderInfo[i].instanceId == pParams->settings[STRATEGY_INSTANCE_ID]){
	  recordData(pParams,(int)pParams->orderInfo[i].type);
	  }
  }

  // Control when should run the strategy....
  // Add special rule for XAUUSD, it is open on 1am, but only start trade from 1��02 am. 
  if(!pParams->settings[RUN_EVERY_TICK] 
    && areOrdersCorrect(pParams, pParams->settings[USE_SL], pParams->settings[USE_TP]) 
    && hasInstanceRunOnCurrentBar((int)pParams->settings[STRATEGY_INSTANCE_ID], pParams->ratesBuffers->rates[PRIMARY_RATES_INDEX].time[pParams->ratesBuffers->rates[PRIMARY_RATES_INDEX].info.arraySize - 1], (BOOL)pParams->settings[IS_BACKTESTING]))
  {	  
    return SUCCESS;
  }

  returnCode = getStrategyFunctions(pParams, &runStrategyFunc);
  if(returnCode != SUCCESS)
  {
    return logAsirikuyError("runStrategy()", returnCode);
  }

  returnCode = validateCommonStrategySettings(pParams);
  if(returnCode != SUCCESS)
  {
    return logAsirikuyError("runStrategy()", returnCode);
  }

  saveUserHeartBeat((int)pParams->settings[STRATEGY_INSTANCE_ID], (BOOL)pParams->settings[IS_BACKTESTING] );
  
  /* Run the strategy. */
  returnCode = clearStrategyResults(pParams);
  if(returnCode != SUCCESS)
  {
    return logAsirikuyError("runStrategy()", returnCode);
  }
  
  returnCode = runStrategyFunc(pParams);
  if(returnCode != SUCCESS)
  {
    return logAsirikuyError("runStrategy()", returnCode);
  }

  returnCode = writeEquityLog(pParams->currentBrokerTime, pParams->accountInfo.equity, pParams);
  if(returnCode != SUCCESS)
  {
    return logAsirikuyError("runStrategy()", returnCode);
  }

  return SUCCESS;
}

AsirikuyReturnCode clearStrategyResults(StrategyParams* pParams)
{
  int i;

  if(pParams == NULL)
  {
    fprintf(stderr, "[CRITICAL] clearStrategyResults() failed. pParams = NULL\n\n");
    return NULL_POINTER;
  }

  for(i = 0; i < pParams->settings[MAX_OPEN_ORDERS]; i++)
  {
    pParams->results[i].brokerSL       = 0;
    pParams->results[i].brokerTP       = 0;
    pParams->results[i].entryPrice     = 0;
    pParams->results[i].expirationTime = 0;
    pParams->results[i].internalSL     = 0;
    pParams->results[i].internalTP     = 0;
    pParams->results[i].lots           = 0;
    pParams->results[i].ticketNumber   = ALL_ORDER_TICKETS;
    pParams->results[i].tradingSignals = SIGNAL_NONE;
    pParams->results[i].useTrailingSL  = FALSE;	 
  }

  return SUCCESS;
}
