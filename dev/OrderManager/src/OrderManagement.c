/**
 * @file
 * @brief     Order management functions including order size and margin requirement calculations.
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

#include <math.h>

#include "Precompiled.h"
#include "OrderManagement.h"
#include "OrderSignals.h"
#include "SymbolAnalyzer.h"
#include "AsirikuyTime.h"
#include "Logging.h"
#include "AsirikuyTechnicalAnalysis.h"
#include "StrategyUserInterface.h"
#include "InstanceStates.h"
#include "TradingWeekBoundaries.h"
#include "EasyTradeCWrapper.hpp"

#define STOPS_REFERENCE_POINTS 5000 /* A high enough value that the broker SL or TP should never be hit but can be used as a benchmark for calculating the internal SL or TP. */
#define ELLIPTICAL_SL 0
#define ELLIPTICAL_TP 1

// Forward declaration
static int backup(char * source_file);

int totalOpenOrders(StrategyParams* pParams, OrderType orderType)
{
  int i, total = 0;

  for(i = 0; i < pParams->settings[ORDERINFO_ARRAY_SIZE]; i++)
  {
    if(!pParams->orderInfo[i].isOpen)
    {
      continue;
    }

    if((pParams->orderInfo[i].type == orderType) || (orderType == NONE))
    {
      total++;
    }
  }

  return total;
}

int totalClosedOrders(StrategyParams* pParams, OrderType orderType)
{
  int i, total = 0;

  for(i = 0; i < pParams->settings[ORDERINFO_ARRAY_SIZE]; i++)
  {
    if(pParams->orderInfo[i].isOpen)
    {
      continue;
    }

    if((pParams->orderInfo[i].type == orderType) || (orderType == NONE))
    {
      total++;
    }
  }

  return total;
}

double maxLossPerLot(const StrategyParams* pParams, OrderType orderType, double entryPrice, double stopLoss)
{
  double lossInQuoteCurrency       = stopLoss * pParams->accountInfo.contractSize;
  double conversionRateBid         = 0;
  double conversionRateAsk         = 0;
  char   baseTradeCurrency      [] = "1........";
  char   quoteTradeCurrency     [] = "2..";
  char   baseConversionCurrency [] = "3..";
  char   quoteConversionCurrency[] = "4..";
  char   baseConversionSymbol   [] = "5..............";
  char   quoteConversionSymbol  [] = "6..............";
  double atr, sumTrueRange = 0, high, low, previousClose; 
  int shift1Index = pParams->ratesBuffers->rates[0].info.arraySize - 2;
  int	 i;

  if (stopLoss == 0)
  {
		for (i = 0; i < (int)pParams->settings[ATR_AVERAGING_PERIOD]; i++)
		{
			high = pParams->ratesBuffers->rates[0].high[shift1Index];
			low  = pParams->ratesBuffers->rates[0].low[shift1Index];
			previousClose = pParams->ratesBuffers->rates[0].close[shift1Index-1]; 
			sumTrueRange += max( high-low, max(fabs(high-previousClose),fabs(low-previousClose)));
		}

	  atr = (1440/(pParams->settings[TIMEFRAME]))*sumTrueRange/(int)pParams->settings[ATR_AVERAGING_PERIOD];

	  lossInQuoteCurrency = atr*pParams->accountInfo.contractSize;
  }

  if (pParams->settings[IS_SPREAD_BETTING] == 1)
  lossInQuoteCurrency *= pParams->bidAsk.ask[0];

  getBaseCurrency(pParams->tradeSymbol, baseTradeCurrency);
  getQuoteCurrency(pParams->tradeSymbol, quoteTradeCurrency);

  /* Conversions are made out using a pair with QUOTE/DEPOSIT structure. For example to get profit of a CHF
  account trading the EURJPY you need to multiply by JPY/CHF. Which can be calculated as 1/(CHF/JPY).*/
  
 /* If deposit and quote currency match the conversion does not require information from other pairs since
 the multiplication would be X/X or 1.*/
  if(strcmp(pParams->accountInfo.currency, quoteTradeCurrency) == 0)
  {
    return(lossInQuoteCurrency);
  }
  
  /* Get conversion symbols that relate base with deposit and quote with deposit */
  getConversionSymbols(pParams->tradeSymbol, pParams->accountInfo.currency, baseConversionSymbol, quoteConversionSymbol);

  /* Now we get the base and quote portions of the quote conversion symbol (the symbol that relates the quote with the deposit currencies).*/
    getBaseCurrency(quoteConversionSymbol, baseConversionCurrency);
    getQuoteCurrency(quoteConversionSymbol, quoteConversionCurrency);
    conversionRateBid = pParams->bidAsk.quoteConversionBid;
    conversionRateAsk = pParams->bidAsk.quoteConversionAsk;
    
  /* If the quote symbol's base matches the deposit currency (like a USD account trading the USDJPY pair) then we multiply for 1/quoteSymbol.*/
  if((strcmp(pParams->accountInfo.currency, baseConversionCurrency) == 0))
  {
	fprintf(stderr, "[DEBUG] conversionRateBid= %lf, conversionRateAsk = %lf", conversionRateBid,conversionRateAsk);
	if (conversionRateAsk <= 0) // something wrong on MT4 feed
		conversionRateAsk = pParams->bidAsk.ask[0];	
	return(lossInQuoteCurrency / conversionRateAsk);

  }
  /* If we have a case where the quote of the quote symbol matches the deposit currency then we make a straight multiplication (like a CHF account trading EURUSD,
     in this case the quote symbol is USDCHF where the quote of this symbol matches the deposit currency).*/
  else if((strcmp(pParams->accountInfo.currency, quoteConversionCurrency) == 0))
  {
	  if (conversionRateBid <= 0) // something wrong on MT4 feed
		  conversionRateBid = pParams->bidAsk.bid[0];
    return(lossInQuoteCurrency * conversionRateBid);
  }

  fprintf(stderr, "[ERROR] maxLossPerLot() failed. Order size calculation will not be accurate. Account currency = %s, Trade symbol = %s.", pParams->accountInfo.currency, pParams->tradeSymbol);
  return lossInQuoteCurrency;
}

double calculateOrderSizeWithSpecificRisk(const StrategyParams* pParams, OrderType orderType, double entryPrice, double stopLoss,double risk)
{

	double mLP;
	double equity = pParams->accountInfo.equity;
	double orderSize = 0.0;
	
	mLP = maxLossPerLot(pParams, orderType, entryPrice, stopLoss);

	if ((int)pParams->settings[DISABLE_COMPOUNDING] == TRUE)
	{
		equity = pParams->settings[ORIGINAL_EQUITY];
	}

	orderSize = 0.01 * risk * equity / mLP;
	orderSize = max(0.01, orderSize);

	fprintf(stderr, "[INFO] Risk = %lf, Equity = %lf, maxLossPerLot =%lf,OrderSize = %lf", risk, equity, mLP, orderSize);
	return orderSize;

}


double calculateOrderSize(const StrategyParams* pParams, OrderType orderType, double entryPrice, double stopLoss)
{

  double mLP;
  double equity = pParams->accountInfo.equity;
  double orderSize = 0.0;
    
  mLP = maxLossPerLot(pParams, orderType, entryPrice, stopLoss);

  if((int)pParams->settings[DISABLE_COMPOUNDING] == TRUE)
  {
	equity = pParams->settings[ORIGINAL_EQUITY];    
  } 	

  orderSize = 0.01 * pParams->settings[ACCOUNT_RISK_PERCENT] * equity / mLP;
  orderSize = max(0.01, orderSize);
    
  fprintf(stderr, "[WARNING] Risk = %lf, Equity = %lf, maxLossPerLot =%lf,OrderSize = %lf", pParams->settings[ACCOUNT_RISK_PERCENT], equity, mLP, orderSize);
  return orderSize;

}

static double calculateMarginRequired(const StrategyParams* pParams, OrderType orderType, double lotSize)
{
  double conversionBid             = 0;
  double conversionAsk             = 0;
  char   baseCurrency           [] = "1..";
  char   quoteCurrency          [] = "2..";
  char   baseConversionCurrency [] = "3..";
  char   quoteConversionCurrency[] = "4..";
  char   baseConversionSymbol   [] = "5...............";
  char   quoteConversionSymbol  [] = "6...............";

  getBaseCurrency(pParams->tradeSymbol, baseCurrency);
  getQuoteCurrency(pParams->tradeSymbol, quoteCurrency);
  getConversionSymbols(pParams->tradeSymbol, pParams->accountInfo.currency, baseConversionSymbol, quoteConversionSymbol);

  if(pParams->bidAsk.baseConversionBid > 0 && pParams->bidAsk.baseConversionAsk > 0)
  {
    conversionBid = pParams->bidAsk.baseConversionBid;
    conversionAsk = pParams->bidAsk.baseConversionAsk;
    getBaseCurrency(baseConversionSymbol, baseConversionCurrency);
    getQuoteCurrency(baseConversionSymbol, quoteConversionCurrency);
  }
  else if(pParams->bidAsk.quoteConversionBid > 0 && pParams->bidAsk.quoteConversionAsk > 0)
  {
    conversionBid = pParams->bidAsk.quoteConversionBid;
    conversionAsk = pParams->bidAsk.quoteConversionAsk;
    getBaseCurrency(quoteConversionSymbol, baseConversionCurrency);
    getQuoteCurrency(quoteConversionSymbol, quoteConversionCurrency);
  }

  if(strcmp(pParams->accountInfo.currency, baseCurrency) == 0)
  {
    return(lotSize * pParams->accountInfo.contractSize / pParams->accountInfo.leverage);
  }

  if(strcmp(pParams->accountInfo.currency, quoteCurrency) == 0)
  {
    if(  (orderType == BUY)
      || (orderType == BUYSTOP)
      || (orderType == BUYLIMIT))
    {
      return(lotSize * pParams->bidAsk.ask[0] * pParams->accountInfo.contractSize / pParams->accountInfo.leverage);
    }
    else if((orderType == SELL)
      ||    (orderType == SELLSTOP)
      ||    (orderType == SELLLIMIT))
    {
      return(lotSize * pParams->bidAsk.bid[0] * pParams->accountInfo.contractSize / pParams->accountInfo.leverage);
    }
  }

  if((strcmp(pParams->accountInfo.currency, baseConversionCurrency) == 0) && (conversionAsk > 0))
  {
    return(lotSize * conversionAsk * pParams->accountInfo.contractSize / pParams->accountInfo.leverage);
  }
  else if((strcmp(pParams->accountInfo.currency, quoteConversionCurrency) == 0) && (conversionBid > 0))
  {
    return(lotSize * conversionBid * pParams->accountInfo.contractSize / pParams->accountInfo.leverage);
  }

  fprintf(stderr, "[ERROR] accurateMarginRequired() failed. Margin protection is not active. Account currency = %s, Trade symbol = %s.", pParams->accountInfo.currency, pParams->tradeSymbol);
  return 0;
}

BOOL isEnoughFreeMargin(const StrategyParams* pParams, OrderType orderType, double lotSize)
{
  double marginRequired = pParams->accountInfo.margin + calculateMarginRequired(pParams, orderType, lotSize);
  double worstCaseEquity = pParams->accountInfo.balance - (pParams->accountInfo.balance * (pParams->settings[ACCOUNT_RISK_PERCENT] + pParams->accountInfo.totalOpenTradeRiskPercent) / 100);

  if(worstCaseEquity <= (marginRequired * pParams->accountInfo.stopoutPercent / 100))
  {
    return FALSE;
  }

  return TRUE;
}

double getPointDigitsAdjustment(StrategyParams* pParams, int ratesIndex)
{
  double pointDigitsAdjustment ;

  if ((pParams->ratesBuffers->rates[ratesIndex].info.digits == 5) ||
	  (pParams->ratesBuffers->rates[ratesIndex].info.digits == 3))
  {
  pointDigitsAdjustment = 1;
  }

  if ((pParams->ratesBuffers->rates[ratesIndex].info.digits == 4) ||
	  (pParams->ratesBuffers->rates[ratesIndex].info.digits == 2) ||
	  (pParams->ratesBuffers->rates[ratesIndex].info.digits == 1))
  {
  pointDigitsAdjustment = 0.1;
  }

  return pointDigitsAdjustment;
}

AsirikuyReturnCode checkInternalSL(StrategyParams* pParams, int ratesIndex, int resultsIndex, double internalSL)
{
  char timeString[MAX_TIME_STRING_SIZE] = "";
  int i, shift0Index, tradingSignals;
  double currentPrice;
  double pointDigitsAdjustment = getPointDigitsAdjustment(pParams, ratesIndex);

  if(pParams == NULL)
  {
    fprintf(stderr, "[CRITICAL] checkInternalSL() failed. pParams = NULL\n");
    return NULL_POINTER;
  }

  tradingSignals = (int)pParams->results[resultsIndex].tradingSignals;
  if(hasEntrySignal(tradingSignals) || hasUpdateSignal(tradingSignals))
  {
    fprintf(stderr, "[WARNING] checkInternalSL() An entry or update signal already exists. This function should be called before generating entry or update signals. InstanceID = %d", (int)pParams->settings[STRATEGY_INSTANCE_ID]);
  }

  shift0Index  = pParams->ratesBuffers->rates[ratesIndex].info.arraySize - 1;
  currentPrice = pParams->ratesBuffers->rates[ratesIndex].open[shift0Index];

  for(i = 0; i < pParams->settings[ORDERINFO_ARRAY_SIZE]; i++)
  {
    if(!pParams->orderInfo[i].isOpen)
    {
      continue;
    }

    if(pParams->orderInfo[i].stopLoss == 0)
    {
      safe_timeString(timeString, pParams->ratesBuffers->rates[ratesIndex].time[shift0Index]);
      fprintf(stderr, "[WARNING] checkInternalSL() Broker SL = 0. The internal SL cannot be calculated. InstanceID = %d", (int)pParams->settings[STRATEGY_INSTANCE_ID]);
      continue;
    }

    if(pParams->orderInfo[i].type == BUY)
    {
      double internalTradeOpenPrice = pParams->orderInfo[i].stopLoss + (pointDigitsAdjustment * STOPS_REFERENCE_POINTS * pParams->ratesBuffers->rates[ratesIndex].info.point);
      if((internalTradeOpenPrice - currentPrice) >= internalSL)
      {
        safe_timeString(timeString, pParams->ratesBuffers->rates[ratesIndex].time[shift0Index]);
        fprintf(stderr, "[INFO] TradeSignal(Internal SL): Close BUY. InstanceID = %d, BarTime = %s, InternalTradeOpenPrice = %lf, CurrentBarOpenPrice = %lf, TP = %lf", (int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, internalTradeOpenPrice, currentPrice, internalSL);
        addTradingSignal(SIGNAL_CLOSE_BUY, &tradingSignals);
      }
    }
    else if(pParams->orderInfo[i].type == SELL)
    {
      double internalTradeOpenPrice = pParams->orderInfo[i].stopLoss - (pointDigitsAdjustment * STOPS_REFERENCE_POINTS * pParams->ratesBuffers->rates[ratesIndex].info.point);
      if((currentPrice - internalTradeOpenPrice) >= internalSL)
      {
        safe_timeString(timeString, pParams->ratesBuffers->rates[ratesIndex].time[shift0Index]);
        fprintf(stderr, "[INFO] TradeSignal(Internal SL): Close SELL. InstanceID = %d, BarTime = %s, InternalTradeOpenPrice = %lf, CurrentBarOpenPrice = %lf, TP = %lf", (int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, internalTradeOpenPrice, currentPrice, internalSL);
        addTradingSignal(SIGNAL_CLOSE_SELL, &tradingSignals);
      }
    }
  }

  pParams->results[resultsIndex].tradingSignals = tradingSignals;
  return SUCCESS;
}

AsirikuyReturnCode checkInternalTP(StrategyParams* pParams, int ratesIndex, int resultsIndex, double internalTP)
{
  char timeString[MAX_TIME_STRING_SIZE] = "";
  int i, shift0Index, tradingSignals;
  double currentPrice;
  double pointDigitsAdjustment = getPointDigitsAdjustment(pParams, ratesIndex);

  if(pParams == NULL)
  {
    fprintf(stderr, "[CRITICAL] checkInternalTP() failed. pParams = NULL\n");
    return NULL_POINTER;
  }

  tradingSignals = (int)pParams->results[resultsIndex].tradingSignals;
  if(hasEntrySignal(tradingSignals) || hasUpdateSignal(tradingSignals))
  {
    fprintf(stderr, "[WARNING] checkInternalTP() An entry or update signal already exists. This function should be called before generating entry or update signals. InstanceID = %d", (int)pParams->settings[STRATEGY_INSTANCE_ID]);
  }

  shift0Index  = pParams->ratesBuffers->rates[ratesIndex].info.arraySize - 1;
  currentPrice = pParams->ratesBuffers->rates[ratesIndex].open[shift0Index];

  for(i = 0; i < pParams->settings[ORDERINFO_ARRAY_SIZE]; i++)
  {
    if(!pParams->orderInfo[i].isOpen)
    {
      continue;
    }

    if(pParams->orderInfo[i].takeProfit == 0)
    {
      safe_timeString(timeString, pParams->ratesBuffers->rates[ratesIndex].time[shift0Index]);
      fprintf(stderr, "[WARNING] checkInternalTP() Broker TP = 0. The internal TP cannot be calculated. InstanceID = %d", (int)pParams->settings[STRATEGY_INSTANCE_ID]);
      continue;
    }

    if(pParams->orderInfo[i].type == BUY)
    {
      double internalTradeOpenPrice = pParams->orderInfo[i].takeProfit - (pointDigitsAdjustment * STOPS_REFERENCE_POINTS * pParams->ratesBuffers->rates[ratesIndex].info.point);
      if((currentPrice - internalTradeOpenPrice) >= internalTP)
      {
        safe_timeString(timeString, pParams->ratesBuffers->rates[ratesIndex].time[shift0Index]);
        fprintf(stderr, "[INFO] TradeSignal(Internal TP)   : Close BUY. InstanceID = %d, BarTime = %s, TradeOpenPrice = %lf, CurrentBarOpenPrice = %lf, TP = %lf", (int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, pParams->orderInfo[i].openPrice, currentPrice, internalTP);
        addTradingSignal(SIGNAL_CLOSE_BUY, &tradingSignals);
      }
    }
    else if(pParams->orderInfo[i].type == SELL)
    {
      double internalTradeOpenPrice = pParams->orderInfo[i].takeProfit + (pointDigitsAdjustment * STOPS_REFERENCE_POINTS * pParams->ratesBuffers->rates[ratesIndex].info.point);
      if((internalTradeOpenPrice - currentPrice) >= internalTP)
      {
        safe_timeString(timeString, pParams->ratesBuffers->rates[ratesIndex].time[shift0Index]);
        fprintf(stderr, "[INFO] TradeSignal(Internal TP)   : Close SELL. InstanceID = %d, BarTime = %s, TradeOpenPrice = %lf, CurrentBarOpenPrice = %lf, TP = %lf", (int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, pParams->orderInfo[i].openPrice, currentPrice, internalTP);
        addTradingSignal(SIGNAL_CLOSE_SELL, &tradingSignals);
      }
    }
  }
  
  pParams->results[resultsIndex].tradingSignals = tradingSignals;
  return SUCCESS;
}


AsirikuyReturnCode checkTimedExit(StrategyParams* pParams, int ratesIndex, int resultsIndex, BOOL usingInternalSL, BOOL usingInternalTP)
{
  AsirikuyReturnCode returnCode = SUCCESS;
  char timeString[MAX_TIME_STRING_SIZE] = "";
  int    shift0Index = pParams->ratesBuffers->rates[ratesIndex].info.arraySize - 1;
  int    i, tradingSignals, barsSinceVirtualOrderEntry;
  time_t virtualOrderEntryTime;

  if(pParams == NULL)
  {
    fprintf(stderr, "[CRITICAL] checkTimedExit() failed. pParams = NULL\n");
    return NULL_POINTER;
  }

  virtualOrderEntryTime = getInstanceState((int)pParams->settings[STRATEGY_INSTANCE_ID])->lastOrderUpdateTime;
  tradingSignals        = (int)pParams->results[resultsIndex].tradingSignals;
  
  if(hasEntrySignal(tradingSignals) || hasUpdateSignal(tradingSignals))
  {
    fprintf(stderr, "[WARNING] checkTimedExit() An entry or update signal already exists. This function should be called before generating entry or update signals. InstanceID = %d", (int)pParams->settings[STRATEGY_INSTANCE_ID]);
  }

  for(i = 0; i < pParams->settings[ORDERINFO_ARRAY_SIZE]; i++)
  {
    if(!pParams->orderInfo[i].isOpen)
    {
      continue;
    }

    if(virtualOrderEntryTime <= 0)
    {
      virtualOrderEntryTime = setLastOrderUpdateTime((int)pParams->settings[STRATEGY_INSTANCE_ID], pParams->orderInfo[i].openTime, (BOOL)pParams->settings[IS_BACKTESTING]);
    }

    returnCode = barsToPreviousTime(pParams->ratesBuffers->rates[ratesIndex].time, virtualOrderEntryTime, shift0Index, &barsSinceVirtualOrderEntry);
    if(returnCode != SUCCESS)
    {
      return logAsirikuyError("checkTimedExit()", returnCode);
    }

    if((barsSinceVirtualOrderEntry > pParams->settings[TIMED_EXIT_BARS]) && (pParams->settings[TIMED_EXIT_BARS] > 0))
    {
      if(pParams->orderInfo[i].type == BUY)
      {
        safe_timeString(timeString, virtualOrderEntryTime);
        fprintf(stderr, "[INFO] TradeSignal(Timed exit)    : Close BUY. InstanceID = %d, virtual order entry time = %s", (int)pParams->settings[STRATEGY_INSTANCE_ID], timeString);
        addTradingSignal(SIGNAL_CLOSE_BUY, &tradingSignals);
      }
      else if(pParams->orderInfo[i].type == SELL)
      {
        safe_timeString(timeString, virtualOrderEntryTime);
        fprintf(stderr, "[INFO] TradeSignal(Timed exit)    : Close SELL. InstanceID = %d, virtual order entry time = %s", (int)pParams->settings[STRATEGY_INSTANCE_ID], timeString);
        addTradingSignal(SIGNAL_CLOSE_SELL, &tradingSignals);
      }
    }
  }
  
  pParams->results[resultsIndex].tradingSignals = tradingSignals;
  return SUCCESS;
}

AsirikuyReturnCode closeLongTrade(StrategyParams* pParams, int resultsIndex)
{
  int tradingSignals;

  if(pParams == NULL)
  {
    fprintf(stderr, "[CRITICAL] closeLongTrade() failed. pParams = NULL\n");
    return NULL_POINTER;
  }

  tradingSignals = (int)pParams->results[resultsIndex].tradingSignals;
  if(hasEntrySignal(tradingSignals) || hasUpdateSignal(tradingSignals))
  {
    fprintf(stderr, "[WARNING] closeLongTrade() An entry or update signal already exists. This function should be called before generating entry or update signals. InstanceID = %d", (int)pParams->settings[STRATEGY_INSTANCE_ID]);
  }

  fprintf(stderr, "[INFO] TradeSignal(Exit criteria) : Close BUY. InstanceID = %d", (int)pParams->settings[STRATEGY_INSTANCE_ID]);
	addTradingSignal(SIGNAL_CLOSE_BUY, &tradingSignals);
  pParams->results[resultsIndex].tradingSignals = tradingSignals;

  return SUCCESS;
}

AsirikuyReturnCode closeShortTrade(StrategyParams* pParams, int resultsIndex)
{
  int tradingSignals;

  if(pParams == NULL)
  {
    fprintf(stderr, "[CRITICAL] closeShortTrade() failed. pParams = NULL\n");
    return NULL_POINTER;
  }

  tradingSignals = (int)pParams->results[resultsIndex].tradingSignals;
  if(hasEntrySignal(tradingSignals) || hasUpdateSignal(tradingSignals))
  {
    fprintf(stderr, "[WARNING] closeShortTrade() An entry or update signal already exists. This function should be called before generating entry or update signals. InstanceID = %d", (int)pParams->settings[STRATEGY_INSTANCE_ID]);
  }

  fprintf(stderr, "[INFO] TradeSignal(Exit criteria) : Close SELL. InstanceID = %d", (int)pParams->settings[STRATEGY_INSTANCE_ID]);
	addTradingSignal(SIGNAL_CLOSE_SELL, &tradingSignals);
  pParams->results[resultsIndex].tradingSignals = tradingSignals;

  return SUCCESS;
}

AsirikuyReturnCode setStops(StrategyParams* pParams, int ratesIndex, int resultsIndex, double stopLoss, double takeProfit, BOOL useInternalSL, BOOL useInternalTP)
{
  double pointDigitsAdjustment = getPointDigitsAdjustment(pParams, ratesIndex);

  if(pParams == NULL)
  {
    fprintf(stderr, "[CRITICAL] setStops() failed. pParams = NULL\n");
    return NULL_POINTER;
  }

  if((useInternalSL) && (stopLoss != 0))
  {
    pParams->results[resultsIndex].brokerSL   = pointDigitsAdjustment * STOPS_REFERENCE_POINTS * pParams->ratesBuffers->rates[ratesIndex].info.point;
    pParams->results[resultsIndex].internalSL = stopLoss;
  }
  else
  {
    pParams->results[resultsIndex].brokerSL   = stopLoss;
    pParams->results[resultsIndex].internalSL = 0;
  }

  if((useInternalTP) && (takeProfit != 0))
  {
    pParams->results[resultsIndex].brokerTP   = pointDigitsAdjustment * STOPS_REFERENCE_POINTS * pParams->ratesBuffers->rates[ratesIndex].info.point;
    pParams->results[resultsIndex].internalTP = takeProfit;
  }
  else
  {
    pParams->results[resultsIndex].brokerTP   = takeProfit;
    pParams->results[resultsIndex].internalTP = 0;
  }

  return SUCCESS;
}

static AsirikuyReturnCode validateNewTrade(StrategyParams* pParams, BOOL* pIsNewTradeAllowed)
{
  if(pParams == NULL)
  {
    fprintf(stderr, "[CRITICAL] validateNewTrade() failed. pParams = NULL\n");
    return NULL_POINTER;
  }

  if(pIsNewTradeAllowed == NULL)
  {
    fprintf(stderr, "[CRITICAL] validateNewTrade() failed. pIsNewTradeAllowed = NULL\n");
    return NULL_POINTER;
  }

  *pIsNewTradeAllowed = FALSE;

  if(pParams->settings[OPERATIONAL_MODE] == MODE_MONITOR)
  {
    return SUCCESS;
  }

  /*if(pParams->accountInfo.largestDrawdownPercent >= pParams->settings[MAX_DRAWDOWN_PERCENT])
  {
    fprintf(stderr, "[ERROR] validateNewTrade() Maximum drawdown exceeded. Drawdown = %lf%%, Maximum = %lf%%", pParams->accountInfo.largestDrawdownPercent, pParams->settings[MAX_DRAWDOWN_PERCENT]);
    return WORST_CASE_SCENARIO;
  }*/

  /* Check if opening another order will risk a stopout by the broker. */
  /*if(!isEnoughFreeMargin(pParams, orderType))
  {
    logAsirikuyError("validateNewTrade()", NOT_ENOUGH_MARGIN);
    return NOT_ENOUGH_MARGIN;
  }*/

  if((pParams->bidAsk.ask[0] - pParams->bidAsk.bid[0]) > pParams->settings[MAX_SPREAD])
  {
    fprintf(stderr, "[ERROR] validateNewTrade() Maximum spread exceeded. Spread = %lf, Maximum = %lf", pParams->bidAsk.ask[0] - pParams->bidAsk.bid[0], pParams->settings[MAX_SPREAD]);
    return SPREAD_TOO_WIDE;
  }

  *pIsNewTradeAllowed = TRUE;
  return SUCCESS;
}

AsirikuyReturnCode openOrUpdateLongTrade(StrategyParams* pParams, int ratesIndex, int resultsIndex, double stopLoss, double takeProfit, double risk, BOOL useInternalSL, BOOL useInternalTP)
{
  AsirikuyReturnCode returnCode = SUCCESS;
  BOOL isNewTradeAllowed;
  int tradingSignals;

  if(pParams == NULL)
  {
    fprintf(stderr, "[CRITICAL] openOrUpdateLongTrade() failed. pParams = NULL\n");
    return NULL_POINTER;
  }

  returnCode = validateNewTrade(pParams, &isNewTradeAllowed);
  if(returnCode != SUCCESS)
  {
    return logAsirikuyError("openOrUpdateLongTrade()", returnCode);
  }

  if(!isNewTradeAllowed)
  {
    return SUCCESS;
  }

  returnCode = setStops(pParams, ratesIndex, resultsIndex, stopLoss, takeProfit, useInternalSL, useInternalTP);
  if(returnCode != SUCCESS)
  {
    return logAsirikuyError("openOrUpdateLongTrade()", returnCode);
  }

  pParams->results[resultsIndex].entryPrice = pParams->bidAsk.ask[0];
  pParams->results[resultsIndex].lots = calculateOrderSize(pParams, BUY, pParams->results[resultsIndex].entryPrice, stopLoss) * risk;

  tradingSignals = (int)pParams->results[resultsIndex].tradingSignals;

  addTradingSignal(SIGNAL_CLOSE_SELL, &tradingSignals);
  removeTradingSignal(SIGNAL_CLOSE_BUY, &tradingSignals);

  if(totalOpenOrders(pParams, BUY) == 0)
  {
    fprintf(stderr, "[INFO] TradeSignal(Entry criteria): Close SELL & Open BUY. InstanceID = %d, EntryPrice = %lf, Lots = %lf, SL = %lf, TP = %lf", (int)pParams->settings[STRATEGY_INSTANCE_ID], pParams->results[resultsIndex].entryPrice, pParams->results[resultsIndex].lots, stopLoss, takeProfit);
    addTradingSignal(SIGNAL_OPEN_BUY, &tradingSignals);

	setLastOrderUpdateTime((int)pParams->settings[STRATEGY_INSTANCE_ID], pParams->ratesBuffers->rates[ratesIndex].time[pParams->ratesBuffers->rates[ratesIndex].info.arraySize - 1], (BOOL)pParams->settings[IS_BACKTESTING]);
  }
  else
  {
    fprintf(stderr, "[INFO] TradeSignal(Entry criteria): Close SELL & Update BUY. InstanceID = %d, EntryPrice = %lf, Lots = %lf, SL = %lf, TP = %lf", (int)pParams->settings[STRATEGY_INSTANCE_ID], pParams->results[resultsIndex].entryPrice, pParams->results[resultsIndex].lots, stopLoss, takeProfit);
    addTradingSignal(SIGNAL_UPDATE_BUY, &tradingSignals);
  }

  pParams->results[resultsIndex].tradingSignals = tradingSignals;
//  setLastOrderUpdateTime((int)pParams->settings[STRATEGY_INSTANCE_ID], pParams->ratesBuffers->rates[ratesIndex].time[pParams->ratesBuffers->rates[ratesIndex].info.arraySize - 1], (BOOL)pParams->settings[IS_BACKTESTING]);

  return SUCCESS;
}

AsirikuyReturnCode openOrUpdateShortTrade(StrategyParams* pParams, int ratesIndex, int resultsIndex, double stopLoss, double takeProfit,double risk, BOOL useInternalSL, BOOL useInternalTP)
{
  AsirikuyReturnCode returnCode = SUCCESS;
  BOOL isNewTradeAllowed;
  int tradingSignals;

  if(pParams == NULL)
  {
    fprintf(stderr, "[CRITICAL] openOrUpdateShortTrade() failed. pParams = NULL\n");
    return NULL_POINTER;
  }

  returnCode = validateNewTrade(pParams, &isNewTradeAllowed);
  if(returnCode != SUCCESS)
  {
    return logAsirikuyError("openOrUpdateShortTrade()", returnCode);
  }

  if(!isNewTradeAllowed)
  {
    return SUCCESS;
  }

  returnCode = setStops(pParams, ratesIndex, resultsIndex, stopLoss, takeProfit, useInternalSL, useInternalTP);
  if(returnCode != SUCCESS)
  {
    return logAsirikuyError("openOrUpdateShortTrade()", returnCode);
  }
  
  pParams->results[resultsIndex].entryPrice = pParams->bidAsk.bid[0];
  pParams->results[resultsIndex].lots       = calculateOrderSize(pParams, SELL, pParams->results[resultsIndex].entryPrice, stopLoss) * risk;

  tradingSignals = (int)pParams->results[resultsIndex].tradingSignals;

  addTradingSignal(SIGNAL_CLOSE_BUY, &tradingSignals);
  removeTradingSignal(SIGNAL_CLOSE_SELL, &tradingSignals);

  if(totalOpenOrders(pParams, SELL) == 0)
  {
    fprintf(stderr, "[INFO] TradeSignal(Entry criteria): Close BUY & Open SELL. InstanceID = %d, EntryPrice = %lf, Lots = %lf, SL = %lf, TP = %lf", (int)pParams->settings[STRATEGY_INSTANCE_ID], pParams->results[resultsIndex].entryPrice, pParams->results[resultsIndex].lots, stopLoss, takeProfit);
    addTradingSignal(SIGNAL_OPEN_SELL, &tradingSignals);

	//only setLastOrderUpdateTime on open a new trade
	setLastOrderUpdateTime((int)pParams->settings[STRATEGY_INSTANCE_ID], pParams->ratesBuffers->rates[ratesIndex].time[pParams->ratesBuffers->rates[ratesIndex].info.arraySize - 1], (BOOL)pParams->settings[IS_BACKTESTING]);

  }
  else
  {
    fprintf(stderr, "[INFO] TradeSignal(Entry criteria): Close BUY & Update SELL. InstanceID = %d, EntryPrice = %lf, Lots = %lf, SL = %lf, TP = %lf", (int)pParams->settings[STRATEGY_INSTANCE_ID], pParams->results[resultsIndex].entryPrice, pParams->results[resultsIndex].lots, stopLoss, takeProfit);
    addTradingSignal(SIGNAL_UPDATE_SELL, &tradingSignals);
  }
  
  pParams->results[resultsIndex].tradingSignals = tradingSignals;
//  setLastOrderUpdateTime((int)pParams->settings[STRATEGY_INSTANCE_ID], pParams->ratesBuffers->rates[ratesIndex].time[pParams->ratesBuffers->rates[ratesIndex].info.arraySize - 1], (BOOL)pParams->settings[IS_BACKTESTING]);

  return SUCCESS;
}

BOOL areOrdersCorrect(StrategyParams* pParams, double stopLoss, double takeProfit)
{
  int i;
	char tempFilePath[MAX_FILE_PATH_CHARS] ;
	char instanceIDName[TOTAL_UI_VALUES];
	char buffer[MAX_FILE_PATH_CHARS] = "";
	char extension[] = "_orderFail.of" ;
	FILE *fp;
	
  if(pParams == NULL)
  {
    fprintf(stderr, "[CRITICAL] areOrdersCorrect() failed. pParams = NULL\n");
    return NULL_POINTER;
  }

	if((BOOL)pParams->settings[IS_BACKTESTING])
	{
    /* Don't run this check when back-testing (execution is perfect) */
    return TRUE;
  }

  for(i = 0; i < pParams->settings[ORDERINFO_ARRAY_SIZE]; i++)
  {
    if((!pParams->orderInfo[i].isOpen) || (pParams->orderInfo[i].instanceId != (int)pParams->settings[STRATEGY_INSTANCE_ID]))
    {
      continue;
    }

    if((pParams->orderInfo[i].takeProfit == 0) && (takeProfit != 0))
    {
      fprintf(stderr, "[WARNING] areOrdersCorrect() TP detected to be 0, assuming modification failure, re-running system on bar to correct", (int)pParams->settings[STRATEGY_INSTANCE_ID]);
      return FALSE;
    }

    if((pParams->orderInfo[i].stopLoss == 0) && (stopLoss != 0))
    {
      fprintf(stderr, "[WARNING] areOrdersCorrect() SL detected to be 0, assuming modification failure, re-running system on bar to correct", (int)pParams->settings[STRATEGY_INSTANCE_ID]);
      return FALSE;
    }
  }

  requestTempFileFolderPath(tempFilePath);

    sprintf(instanceIDName, "%d", (int)pParams->settings[STRATEGY_INSTANCE_ID]);
	strcat(buffer, tempFilePath);
	strcat(buffer, instanceIDName);
	strcat(buffer, extension);
   
  fp = fopen(buffer, "r");

  if(fp != NULL)
  {
    fprintf(stderr, "[CRITICAL] areOrdersCorrect() Order error message found, re-running system\n");	
    fclose(fp);	

	// backup the failure order file	
	if (backup(buffer) <0)
		fprintf(stderr, "[ERROR] Fail to backup file %s ", buffer);

	
    remove(buffer);
    return FALSE;
  }

	return TRUE;
}

static int backup(char * source_file)
{
	char ch;
	char target_file[MAX_FILE_PATH_CHARS] = "";
	FILE *source, *target;

	strcat(target_file, source_file);
	strcat(target_file, ".bak\n");

	source = fopen(source_file, "r\n");

	if (source == NULL)
	{
		fprintf(stderr, "[ERROR] Fail to load source file %s", source_file);
		return -1;
	}

	target = fopen(target_file, "w\n");

	if (target == NULL)
	{
		fclose(source);		
		fprintf(stderr, "[ERROR] Fail to load target file %s", target_file);
		return -1;
	}

	while ((ch = fgetc(source)) != EOF)
		fputc(ch, target);	

	fclose(source);
	fclose(target);

	return 0;
}


int logOrderFailFile(FILE *pFile)
{
	long lSize;
	char * buffer;
	size_t result;

	// obtain file size:
	fseek(pFile, 0, SEEK_END);
	lSize = ftell(pFile);
	rewind(pFile);

	// allocate memory to contain the whole file:
	buffer = (char*)malloc(sizeof(char)*lSize);
	if (buffer == NULL) 
	{ 
		return -1;
	}

	// copy the file into the buffer:
	result = fread(buffer, 1, lSize, pFile);
	if (result != lSize)
	{
		return -1;
	}

	/* the whole file is now loaded in the memory buffer. */
	fprintf(stderr, "[CRITICAL] buffer error\n");

	// terminate	
	free(buffer);
	return 0;
}

//Return how many previous bars...not the time exactly.
int getOrderAge(StrategyParams* pParams, int ratesIndex)
{
  AsirikuyReturnCode returnCode = SUCCESS;
  int    shift0Index = pParams->ratesBuffers->rates[ratesIndex].info.arraySize - 1;
  int    barsSinceVirtualOrderEntry;
  time_t virtualOrderEntryTime;

  virtualOrderEntryTime = getInstanceState((int)pParams->settings[STRATEGY_INSTANCE_ID])->lastOrderUpdateTime;

  fprintf(stderr, "[INFO] Testing12 : InstanceID = %d, virtualOrderEntryTime = %d", (int)pParams->settings[STRATEGY_INSTANCE_ID],virtualOrderEntryTime);

  returnCode = barsToPreviousTime(pParams->ratesBuffers->rates[ratesIndex].time, virtualOrderEntryTime, shift0Index, &barsSinceVirtualOrderEntry);

  fprintf(stderr, "[INFO] Testing22223....Kantu System InstanceID = %d", (int)pParams->settings[STRATEGY_INSTANCE_ID]);

  if(returnCode != SUCCESS)
  {
    return logAsirikuyError("checkTimedExit()", returnCode);
  }

  return(barsSinceVirtualOrderEntry); 
}

int getOrderAgeByPosition(StrategyParams* pParams, int ratesIndex, int position)
{
  AsirikuyReturnCode returnCode = SUCCESS;
  int    shift0Index = pParams->ratesBuffers->rates[ratesIndex].info.arraySize - 1;
  int    barsSinceVirtualOrderEntry;
  time_t virtualOrderEntryTime;

  virtualOrderEntryTime = pParams->orderInfo[position].openTime;

  returnCode = barsToPreviousTime(pParams->ratesBuffers->rates[ratesIndex].time, virtualOrderEntryTime, shift0Index, &barsSinceVirtualOrderEntry);
  if(returnCode != SUCCESS)
  {
    return logAsirikuyError("checkTimedExit()", returnCode);
  }

  return(barsSinceVirtualOrderEntry); 
}

AsirikuyReturnCode updateLongTrade(StrategyParams* pParams, int ratesIndex, int resultsIndex, double stopLoss, double takeProfit, BOOL useInternalSL, BOOL useInternalTP)
{
  AsirikuyReturnCode returnCode = SUCCESS;
  int tradingSignals;

  if(pParams == NULL)
  {
    fprintf(stderr, "[CRITICAL] updateLongTrade() failed. pParams = NULL\n");
    return NULL_POINTER;
  }

  returnCode = setStops(pParams, ratesIndex, resultsIndex, stopLoss, takeProfit, useInternalSL, useInternalTP);
  if(returnCode != SUCCESS)
  {
    return logAsirikuyError("updateLongTrade()", returnCode);
  }

  pParams->results[resultsIndex].entryPrice = pParams->bidAsk.ask[0];
  pParams->results[resultsIndex].lots       = calculateOrderSize(pParams, BUY, pParams->results[resultsIndex].entryPrice, stopLoss);

  tradingSignals = (int)pParams->results[resultsIndex].tradingSignals;

  fprintf(stderr, "[INFO] TradeSignal(Only Update): Update BUY. InstanceID = %d, EntryPrice = %lf, Lots = %lf, SL = %lf, TP = %lf", (int)pParams->settings[STRATEGY_INSTANCE_ID], pParams->results[resultsIndex].entryPrice, pParams->results[resultsIndex].lots, stopLoss, takeProfit);
  addTradingSignal(SIGNAL_UPDATE_BUY, &tradingSignals);

  pParams->results[resultsIndex].tradingSignals = tradingSignals;

  return SUCCESS;
}

AsirikuyReturnCode updateShortTrade(StrategyParams* pParams, int ratesIndex, int resultsIndex, double stopLoss, double takeProfit, BOOL useInternalSL, BOOL useInternalTP)
{
  AsirikuyReturnCode returnCode = SUCCESS;
  int tradingSignals;

  if(pParams == NULL)
  {
    fprintf(stderr, "[CRITICAL] updateShortTrade() failed. pParams = NULL\n");
    return NULL_POINTER;
  }

  returnCode = setStops(pParams, ratesIndex, resultsIndex, stopLoss, takeProfit, useInternalSL, useInternalTP);
  if(returnCode != SUCCESS)
  {
    return logAsirikuyError("updateShortTrade()", returnCode);
  }

  pParams->results[resultsIndex].entryPrice = pParams->bidAsk.bid[0];
  pParams->results[resultsIndex].lots       = calculateOrderSize(pParams, SELL, pParams->results[resultsIndex].entryPrice, stopLoss);

  tradingSignals = (int)pParams->results[resultsIndex].tradingSignals;

  fprintf(stderr, "[INFO] TradeSignal(Only Update): Update SELL. InstanceID = %d, EntryPrice = %lf, Lots = %lf, SL = %lf, TP = %lf", (int)pParams->settings[STRATEGY_INSTANCE_ID], pParams->results[resultsIndex].entryPrice, pParams->results[resultsIndex].lots, stopLoss, takeProfit);
  addTradingSignal(SIGNAL_UPDATE_SELL, &tradingSignals);

  pParams->results[resultsIndex].tradingSignals = tradingSignals;
  return SUCCESS;
}

AsirikuyReturnCode trailOpenTrades(StrategyParams* pParams, int ratesIndex, double trailStart, double trailDistance, BOOL useInternalSL, BOOL useInternalTP)
{
	AsirikuyReturnCode returnCode = SUCCESS;
	int i, j = 0;
	int shift0Index = pParams->ratesBuffers->rates[ratesIndex].info.arraySize - 1;

	if((trailStart == 0) || (trailDistance == 0))
	{
		return SUCCESS;
	}

	/* avoid trailing the stop if we are outside of threshold hours*/
	if (!isValidTradingTime(pParams,pParams->currentBrokerTime))
	{
		return SUCCESS;
	}

	if (trailDistance < pParams->accountInfo.minimumStop)
	{
		trailDistance = pParams->accountInfo.minimumStop;
	}

	for(i = 0; i < pParams->settings[ORDERINFO_ARRAY_SIZE]; i++)
  {
		if((!pParams->orderInfo[i].isOpen) || (pParams->orderInfo[i].instanceId != (int)pParams->settings[STRATEGY_INSTANCE_ID]))
		{
		  continue;
		}

		pParams->results[j].ticketNumber = pParams->orderInfo[i].ticket;

		pParams->results[j].useTrailingSL = 1;

		/* make sure these values are set to zero
	     so that we don't trail to a previous value defined by the calling system */

		pParams->results[j].brokerSL   = 0;
		pParams->results[j].internalSL = 0;
		pParams->results[j].brokerTP   = 0;
		pParams->results[j].internalTP = 0;		

		if((fabs(pParams->bidAsk.ask[0] - pParams->ratesBuffers->rates[ratesIndex].open[shift0Index-getOrderAge(pParams, ratesIndex)]) > trailStart ) &&
			(pParams->bidAsk.ask[0]> pParams->ratesBuffers->rates[ratesIndex].open[shift0Index-getOrderAge(pParams, ratesIndex)]) &&
			(pParams->bidAsk.ask[0]-trailDistance > pParams->orderInfo[i].stopLoss) &&
			(pParams->orderInfo[i].type == BUY))
		{

			fprintf(stderr, "[INFO] Trail BUY Stop. new SL = %lf, old SL = %lf, old TP = %lf, ask = %lf, TD = %lf, TS = %lf, minStop = %lf", (pParams->bidAsk.ask[0]-trailDistance), pParams->orderInfo[i].stopLoss, pParams->orderInfo[i].takeProfit, pParams->bidAsk.ask[0], trailDistance, trailStart, pParams->accountInfo.minimumStop);
			pParams->results[j].brokerSL   = trailDistance;
			pParams->results[j].internalSL = 0;

		}

		if((fabs(pParams->bidAsk.bid[0] - pParams->ratesBuffers->rates[ratesIndex].open[shift0Index-getOrderAge(pParams, ratesIndex)]) > trailStart ) &&
			(pParams->bidAsk.bid[0] < pParams->ratesBuffers->rates[ratesIndex].open[shift0Index-getOrderAge(pParams, ratesIndex)]) &&
			(pParams->bidAsk.bid[0]+trailDistance < pParams->orderInfo[i].stopLoss) &&
			(pParams->orderInfo[i].type == SELL))
		{	
			
			fprintf(stderr, "[INFO] Trail SELL Stop. new SL = %lf, old SL = %lf, old TP = %lf, bid = %lf, TD = %lf, TS = %lf, minStop = %lf", (pParams->bidAsk.bid[0]+trailDistance), pParams->orderInfo[i].stopLoss, pParams->orderInfo[i].takeProfit, pParams->bidAsk.bid[0], trailDistance, trailStart, pParams->accountInfo.minimumStop);
			pParams->results[j].brokerSL   = trailDistance;
			pParams->results[j].internalSL = 0;

		}

		j++ ;
	} 

	return SUCCESS;
}

double roundN(double value, int to)
{
  double places = pow(10.0, to);
  return ((int)(value * places) / places);
}

double iVarOnArray(double arrayForCalculation[], int numItems )
{
  int j;
  double sum = 0, var = 0, average = 0;

  for(j= 0;j < numItems;j++)
  {
    sum+=arrayForCalculation[j];
  }

  average = sum/numItems;

  sum = 0 ;

  for (j = 0; j < numItems; j++)
  {
    sum += (arrayForCalculation[j] - average)*(arrayForCalculation[j] - average);
  }

  var =  sum / (numItems-1);

  return(var);
}

double CalculateVar(StrategyParams* pParams, int maxHoldingTime)
{
  double *varianceInput, varianceResult ;
  int j;
  int shift1Index = pParams->ratesBuffers->rates[0].info.arraySize - 2;

  varianceInput = (double *)malloc(sizeof(double)*maxHoldingTime);

  for (j = 0; j < maxHoldingTime; j++)
  {
    varianceInput[j] = pParams->ratesBuffers->rates[0].close[shift1Index-j] - pParams->ratesBuffers->rates[0].open[shift1Index-j];
  }

  varianceResult = iVarOnArray (varianceInput, maxHoldingTime);

  free(varianceInput);

  return (varianceResult);
}

double ValueTransformExp(double value)
{
  value = exp (value) - 1.;
  return (value);
}

double ValueTransformLog(double value)
{
  value = log (value + 1.);
  return (value);
}

double SampleCumSDBBEstimate(long double variance, double z, int maxHoldingTime, int orderBarsAge, int limitType)
{
  double cumsd;
  double e2v;
  double t = orderBarsAge / (maxHoldingTime * 1.);

  e2v = ValueTransformLog(variance) * orderBarsAge;

  switch(limitType)
  {
  case ELLIPTICAL_SL: cumsd = ValueTransformExp (z * sqrt (e2v / t) * sqrt (t * (1 - t) ));
  break;
  case ELLIPTICAL_TP: cumsd = ValueTransformExp (z * sqrt (e2v / t) * sqrt (1 - t) );
  break;
  default: cumsd = 0;
	break;
  }

  return (cumsd);
}

double CalculateEllipticalStopLoss(StrategyParams* pParams, double target, int maxHoldingTime, double z, int orderBarsAge)
{
  double ellipticalStopLoss;
  double cumsd, y;
  double variance;

  variance = roundN(CalculateVar(pParams, maxHoldingTime), 8);

  if (orderBarsAge > maxHoldingTime)
  {
    orderBarsAge = maxHoldingTime;
  }

  cumsd = SampleCumSDBBEstimate(variance, z, maxHoldingTime, orderBarsAge, ELLIPTICAL_SL);
  y = 1. * orderBarsAge / (maxHoldingTime * 1.) * target;

  ellipticalStopLoss = cumsd - y ;

 // fprintf(stderr, "[INFO] SL -- cumsd = %lf y = %lf var= %10.10lf", cumsd, y, variance);

  return (ellipticalStopLoss);
}

double CalculateEllipticalTakeProfit(StrategyParams* pParams, double target, int maxHoldingTime, double z, int orderBarsAge)
{
  // Print ("FdF.Inst.BBridge.Calc\n");
   double variance;
   double ellipticalTakeProfit;
   double cumsd, y;
   variance = roundN(CalculateVar(pParams, maxHoldingTime), 8);

	if (orderBarsAge > maxHoldingTime)
	{
    orderBarsAge = maxHoldingTime;
	}

   cumsd = SampleCumSDBBEstimate(variance, z, maxHoldingTime, orderBarsAge, ELLIPTICAL_TP);
   y = 1. * orderBarsAge / (maxHoldingTime * 1.) * target;

   ellipticalTakeProfit = cumsd - y;

 // fprintf(stderr, "[INFO] TP -- cumsd = %lf y = %lf var = %10.10lf, maxhold = %d", cumsd, y, variance, maxHoldingTime, orderBarsAge);

//   Print (target, " ", sl, " ", cumsd);
   return (ellipticalTakeProfit);
}