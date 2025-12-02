/**
 * @file
 * @brief     The Easy Trading library implements function wrappers to make coding with F4 more intuitive
 * @details   This library implements functions similar to those in MT4, for example you can get the value of open by using open(1) instead of calling the pParams structure
 *
 * @author    Daniel Fernandez (Initial implementation)
 * @author    Morgan Doel (Assisted with design and review)
 * @author    Maxim Feinshtein (Assisted with design and code styling)
 * @version   F4.x.x
 * @date      2013
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

#include "Precompiled.hpp"
#include "EasyTradeCWrapper.hpp"
#include "EasyTrade.hpp"

namespace
{
  boost::thread_specific_ptr<EasyTrade> easyTradePtr;
}

StrategyParams* getParams()
{
  return easyTradePtr->getParams();
}

int totalOrdersCount()
{
  return easyTradePtr->totalOrdersCount();
}

int buyOrdersCount()
{
  return easyTradePtr->buyOrdersCount();
}

int sellOrdersCount()
{
  return easyTradePtr->sellOrdersCount();
}

tickData addTickArray()
{
  return easyTradePtr->addTickArray();
}

AsirikuyReturnCode saveTickData()
{
  return easyTradePtr->saveTickData();
}

AsirikuyReturnCode openSingleSellStopEasy(double entryPrice, double takeProfit, double stopLoss, double lotSize)
{
  return easyTradePtr->openSingleSellStopEasy(entryPrice, takeProfit, stopLoss, lotSize);
}

AsirikuyReturnCode openSingleSellLimitEasy(double entryPrice, double takeProfit, double stopLoss, double lotSize,double risk)
{
  return easyTradePtr->openSingleSellLimitEasy(entryPrice, takeProfit, stopLoss, lotSize,risk);
}

AsirikuyReturnCode openSingleBuyStopEasy(double entryPrice, double takeProfit, double stopLoss, double lotSize)
{
  return easyTradePtr->openSingleBuyStopEasy(entryPrice, takeProfit, stopLoss, lotSize);
}

AsirikuyReturnCode openSingleBuyLimitEasy(double entryPrice, double takeProfit, double stopLoss, double lotSize,double risk)
{
	return easyTradePtr->openSingleBuyLimitEasy(entryPrice, takeProfit, stopLoss, lotSize, risk);
}

AsirikuyReturnCode openSingleShortEasy(double takeProfit, double stopLoss, double lotSize,double risk)
{
  return easyTradePtr->openSingleShortEasy(takeProfit, stopLoss, lotSize,risk);
}

AsirikuyReturnCode openSingleLongEasy(double takeProfit, double stopLoss, double lotSize,double risk)
{
  return easyTradePtr->openSingleLongEasy(takeProfit, stopLoss, lotSize,risk);
}

double spread()
{
  return easyTradePtr->spread();
}

int barsCount(int ratesArrayIndex)
{
  return easyTradePtr->barsCount(ratesArrayIndex);
}

AsirikuyReturnCode initEasyTradeLibrary(StrategyParams* pInputParams)
{
  easyTradePtr.reset(new EasyTrade());
  return easyTradePtr->initEasyTradeLibrary(pInputParams);
}

double parameter(int parameterIndex)
{
  return easyTradePtr->parameter(parameterIndex);
}

AsirikuyReturnCode addNewDailyRates(char* ratesName, time_t intFromDate, int ratesIndex)
{
  return easyTradePtr->addNewDailyRates(ratesName,intFromDate,ratesIndex);
}

AsirikuyReturnCode addNewDailyRatesQuandl(char* token, char* dataset, char* ratesName, time_t intFromDate, int ratesIndex)
{
  return easyTradePtr->addNewDailyRatesQuandl(token, dataset, ratesName,intFromDate,ratesIndex);
}
	
AsirikuyReturnCode addNewDailyRatesQuandlOpenOnly(char* token, char* dataset, char* ratesName, time_t intFromDate, int ratesIndex)
{
  return easyTradePtr->addNewDailyRatesQuandlOpenOnly(token, dataset, ratesName,intFromDate,ratesIndex);
}

AsirikuyReturnCode addDistortedArray(int sourceRateIndex, int ratesIndex, double maxDistorsion)
{
  return easyTradePtr->addDistortedArray(sourceRateIndex, ratesIndex, maxDistorsion);
}

AsirikuyReturnCode freeRates(int ratesIndex)
{
  return easyTradePtr->freeRates(ratesIndex);
}

BOOL addValueToUI(char* valueName, double valueToAdd)
{
  return easyTradePtr->addValueToUI(valueName, valueToAdd);
}

int findShift(int finalArrayIndex, int originalArrayIndex, int shift)
{
 return easyTradePtr->findShift(finalArrayIndex, originalArrayIndex, shift);
}

time_t openTime(int shift)
{
  return easyTradePtr->openTime(shift);
}

double cOpen(int shift)
{
  return easyTradePtr->open(shift);
}

double volume(int shift)
{
  return easyTradePtr->volume(shift);
}

double high(int shift)
{
  return easyTradePtr->high(shift);
}

double low(int shift)
{
  return easyTradePtr->low(shift);
}

double cClose(int shift)
{
  return easyTradePtr->close(shift);
}

double iVolume(int rateIndex, int shift)
{
	return easyTradePtr->iVolume(rateIndex, shift);
}

double iClose(int rateIndex, int shift)
{
  return easyTradePtr->iClose(rateIndex, shift);
}

double iOpen(int rateIndex, int shift)
{
  return easyTradePtr->iOpen(rateIndex, shift);
}

double iHigh(int rateIndex, int shift)
{
  return easyTradePtr->iHigh(rateIndex, shift);
}

double iLow(int rateIndex, int shift)
{
  return easyTradePtr->iLow(rateIndex, shift);
}

time_t iOpenTime(int rateIndex, int shift)
{
  return easyTradePtr->iOpenTime(rateIndex, shift);
}


double range(int shift)
{
  return easyTradePtr->range(shift);
}

double body(int shift)
{
  return easyTradePtr->body(shift);
}

double absBody(int shift)
{
  return easyTradePtr->absBody(shift);
}

int hour()
{
  return easyTradePtr->hour();
}

int minute()
{
  return easyTradePtr->minute();
}

int dayOfWeek()
{
  return easyTradePtr->dayOfWeek();
}

int month()
{
  return easyTradePtr->month();
}

int year()
{
  return easyTradePtr->year();
}

int dayOfyear()
{
  return easyTradePtr->dayOfYear();
}

int dayOfMonth()
{
  return easyTradePtr->dayOfMonth();
}

double iSTO(int ratesArrayIndex, int period, int k, int d, int signal, int shift)
{
  return easyTradePtr->iSTO(ratesArrayIndex, period, k, d, signal, shift);
}

double iSMI(int ratesArrayIndex, int period_Q, int period_R, int period_S, int signal, int shift)
{
  return easyTradePtr->iSMI(ratesArrayIndex, period_Q, period_R, period_S, signal, shift);
}

double iMACD(int ratesArrayIndex, int fastPeriod, int slowPeriod, int signalPeriod, int signal, int shift)
{
  return easyTradePtr->iMACD(ratesArrayIndex, fastPeriod, slowPeriod, signalPeriod, signal, shift);
}

double iMACDAll(int ratesArrayIndex, int fastPeriod, int slowPeriod, int signalPeriod, int shift, double *pMacd, double *pMmacdSignal, double *pMacdHist)
{
	return easyTradePtr->iMACDAll(ratesArrayIndex, fastPeriod, slowPeriod, signalPeriod, shift, pMacd, pMmacdSignal, pMacdHist);
}
double iBBands(int ratesArrayIndex, int bb_period, double bb_deviation, int signal, int shift)
{
  return easyTradePtr->iBBands(ratesArrayIndex, bb_period, bb_deviation, signal, shift);
}

double iBBandStop(int ratesArrayIndex, int bb_period, double bb_deviation, int * signal, double * stopPrice,int * index)
{
	return easyTradePtr->iBBandStop(ratesArrayIndex, bb_period, bb_deviation, signal, stopPrice,index);
}

double iStdev(int ratesArrayIndex, int type, int period, int shift)
{
  return easyTradePtr->iStdev(ratesArrayIndex, type, period, shift);
}

double iCCI(int ratesArrayIndex, int period, int shift)
{
  return easyTradePtr->iCCI(ratesArrayIndex, period, shift);
}

double iMA(int type, int ratesArrayIndex, int period, int shift)
{
  return easyTradePtr->iMA(type, ratesArrayIndex, period, shift);
}

double iRSI(int ratesArrayIndex, int period, int shift)
{
  return easyTradePtr->iRSI(ratesArrayIndex, period, shift);
}

double iRangeAverage(int ratesArrayIndex, int period, int shift)
{
  return easyTradePtr->iRangeAverage(ratesArrayIndex, period, shift);
}

int getHourShift(int rateIndex, int shift)
{
	return easyTradePtr->getHourShift(rateIndex, shift);
}

double iRangeSafeShiftZero(int period)
{
  return easyTradePtr->iRangeSafeShiftZero(period);
}

double iAtrSafeShiftZero(int period)
{
  return easyTradePtr->iAtrSafeShiftZero(period);
}

double iAtrDailyByHourInterval(int period, int firstHour, int lastHour)
{
  return easyTradePtr->iAtrDailyByHourInterval(period, firstHour, lastHour);
}

int Period()
{
  return easyTradePtr->Period();
}

double iAtrWholeDaysSimple(int rateIndex, int periodATR)
{
  return easyTradePtr->iAtrWholeDaysSimple(rateIndex, periodATR);
}

double iAtrWholeDaysSimpleShift(int rateIndex, int periodATR, int shift)
{
  return easyTradePtr->iAtrWholeDaysSimpleShift(rateIndex, periodATR, shift);
}

int Digits()
{
  return easyTradePtr->Digits();
}

double iAtrSafeShiftZeroWholeDays(int period)
{
  return easyTradePtr->iAtrSafeShiftZeroWholeDays(period);
}

double iAtr(int ratesArrayIndex, int period, int shift)
{
  return easyTradePtr->iAtr(ratesArrayIndex, period, shift);
}

void print(double valueToPrint)
{
  return easyTradePtr->print(valueToPrint);
}

AsirikuyReturnCode checkOrders(double takeProfit, double stopLoss)
{
  return easyTradePtr->checkOrders(takeProfit, stopLoss);
}

AsirikuyReturnCode openOrUpdateLongEasy(double takeProfit, double stopLoss,double risk)
{
	return easyTradePtr->openOrUpdateLongEasy(takeProfit, stopLoss, risk);
}

AsirikuyReturnCode openOrUpdateShortEasy(double takeProfit, double stopLoss,double risk)
{
	return easyTradePtr->openOrUpdateShortEasy(takeProfit, stopLoss, risk);
}

AsirikuyReturnCode closeAllBuyLimitOrdersEasy(time_t currentTime)
{
	return easyTradePtr->closeAllBuyLimitOrders(currentTime);
}

AsirikuyReturnCode closeAllBuyStopOrdersEasy(time_t currentTime)
{
	return easyTradePtr->closeAllBuyStopOrders(currentTime);
}

AsirikuyReturnCode closeAllLimitAndStopOrdersEasy(time_t currentTime)
{
	return easyTradePtr->closeAllLimitAndStopOrders(currentTime);
}

AsirikuyReturnCode closeAllSellLimitOrdersEasy(time_t currentTime)
{
	return easyTradePtr->closeAllSellLimitOrders(currentTime);
}

AsirikuyReturnCode closeAllSellStopOrdersEasy(time_t currentTime)
{
	return easyTradePtr->closeAllSellStopOrders(currentTime);
}


AsirikuyReturnCode closeAllWithNegativeEasy(int tradeMode, time_t currentTime,int days)
{
	easyTradePtr->closeAllLongsWithNegative(tradeMode, currentTime, days);
	easyTradePtr->closeAllShortsWithNegative(tradeMode, currentTime, days);
	return SUCCESS;
}

AsirikuyReturnCode closeAllLongs()
{
  return easyTradePtr->closeAllLongs();
}

AsirikuyReturnCode closeAllShorts()
{
  return easyTradePtr->closeAllShorts();
}

AsirikuyReturnCode closeAllLongTermLongsEasy()
{
	return easyTradePtr->closeAllLongTermLongs();
}

AsirikuyReturnCode closeAllLongTermShortsEasy()
{
	return easyTradePtr->closeAllLongTermShorts();
}

AsirikuyReturnCode closeLongEasy(int orderTicket)
{
  return easyTradePtr->closeLongEasy(orderTicket);
}

AsirikuyReturnCode closeShortEasy(int orderTicket)
{
  return easyTradePtr->closeShortEasy(orderTicket);
}

AsirikuyReturnCode closeAllCurrentDayShortTermOrdersEasy(int tradeMode, time_t currentTime)
{
	return easyTradePtr->closeAllCurrentDayShortTermOrders(tradeMode, currentTime);
}

AsirikuyReturnCode closeWinningPositionsEasy(double total, double target)
{
	return easyTradePtr->closeWinningPositions(total,target);
}

double caculateStrategyPNLEasy(BOOL isIgnoredLockedProfit)
{
	return easyTradePtr->caculateStrategyPNL(isIgnoredLockedProfit);
}

double caculateStrategyRiskEasy(BOOL isIgnoredLockedProfit)
{
	return easyTradePtr->caculateStrategyRisk(isIgnoredLockedProfit);
}

double caculateStrategyVolRiskEasy(double dailyATR)
{
	return easyTradePtr->caculateStrategyVolRisk(dailyATR);
}

double caculateStrategyVolRiskForNoTPOrdersEasy(double dailyATR)
{
	return easyTradePtr->caculateStrategyVolRiskForNoTPOrders(dailyATR);
}

int getSamePricePendingNoTPOrdersEasy(double entryPrice, double limit)
{
	return easyTradePtr->getSamePricePendingNoTPOrders(entryPrice, limit);
}

double isSamePricePendingOrderEasy(double entryPrice, double limit)
{
	return easyTradePtr->isSamePricePendingOrder(entryPrice, limit);
}

double isSameDaySamePricePendingOrderEasy(double entryPrice, double limit, time_t currentTime)
{
	return easyTradePtr->isSameDaySamePricePendingOrder(entryPrice, limit, currentTime);
}

double isSameDaySamePricePendingOrderEasy_with_TP(double entryPrice, double limit, time_t currentTime)
{
	return easyTradePtr->isSameDaySamePricePendingOrder_with_TP(entryPrice, limit, currentTime);
}

double isSameWeekSamePricePendingOrderEasy(double entryPrice, double limit, time_t currentTime)
{
	return easyTradePtr->isSameWeekSamePricePendingOrder(entryPrice, limit, currentTime);
}

double isSamePriceBuyLimitOrderEasy(double entryPrice, time_t currentTime,double gap)
{
	return easyTradePtr->isSamePriceBuyLimitOrder(entryPrice, currentTime,gap);
}

double isSamePriceSellLimitOrderEasy(double entryPrice, time_t currentTime,double gap)
{
	return easyTradePtr->isSamePriceSellLimitOrder(entryPrice, currentTime,gap);
}

double isSamePriceBuyStopOrderEasy(double entryPrice, time_t currentTime, double gap)
{
	return easyTradePtr->isSamePriceBuyStopOrder(entryPrice, currentTime, gap);
}

double isSamePriceSellStopOrderEasy(double entryPrice, time_t currentTime, double gap)
{
	return easyTradePtr->isSamePriceSellStopOrder(entryPrice, currentTime, gap);
}

AsirikuyReturnCode modifyTradeEasy_DayTrading(int orderType, int orderTicket, double stopLoss1, double stopLoss2, double takeProfit, int tpMode, time_t currentTime, double adjust, BOOL stopMovingbackSL)
{
	if (orderTicket == -1 && orderType == BUY) // All tickets
		return easyTradePtr->modifyAllLongs_DayTrading(stopLoss1, stopLoss2, takeProfit, tpMode, currentTime, adjust, stopMovingbackSL);
	else if (orderTicket == -1 && orderType == SELL) // All tickets
		return easyTradePtr->modifyAllShorts_DayTrading(stopLoss1, stopLoss2, takeProfit, tpMode, currentTime, adjust, stopMovingbackSL);
	else
		return easyTradePtr->modifyTradeEasy(orderType, orderTicket, stopLoss1, takeProfit);
}

/*
tpMode = 0: current way
tpMode = 1: New day, if break event, TP = 0
*/
AsirikuyReturnCode modifyTradeEasy_new(int orderType, int orderTicket, double stopLoss, double takeProfit, int tpMode,BOOL stopMovingbackSL)
{
	if (orderTicket == -1 && orderType == BUY) // All tickets
		return easyTradePtr->modifyAllLongs(stopLoss, takeProfit, tpMode, stopMovingbackSL);
	else if (orderTicket == -1 && orderType == SELL) // All tickets
		return easyTradePtr->modifyAllShorts(stopLoss, takeProfit, tpMode, stopMovingbackSL);
	else
		return easyTradePtr->modifyTradeEasy(orderType, orderTicket, stopLoss, takeProfit);
}

AsirikuyReturnCode modifyAllOrdersOnSameDateEasy(int orderIndex, double stopLoss, double takePrice, BOOL stopMovingbackSL)
{
	return easyTradePtr->modifyAllOrdersOnSameDate(orderIndex, stopLoss, takePrice, stopMovingbackSL);
}

AsirikuyReturnCode modifyTradeEasy(int orderType, int orderTicket, double stopLoss, double takeProfit)
{
	if (orderTicket == -1 && orderType == BUY) // All tickets
		return easyTradePtr->modifyAllLongs(stopLoss, takeProfit, 0, TRUE);
	else if (orderTicket == -1 && orderType == SELL) // All tickets
		return easyTradePtr->modifyAllShorts(stopLoss, takeProfit, 0, TRUE);
	else
		return easyTradePtr->modifyTradeEasy(orderType, orderTicket, stopLoss, takeProfit);
}


AsirikuyReturnCode addNewRenkoRates(int originalRatesIndex, int ratesIndex, double renkoSize)
{
  return easyTradePtr->addNewRenkoRates(originalRatesIndex, ratesIndex, renkoSize);
}

AsirikuyReturnCode addNewConstantVolumeRates(int originalRatesIndex, int ratesIndex, int volumeRequired)
{
  return easyTradePtr->addNewConstantVolumeRates(originalRatesIndex, ratesIndex, volumeRequired);
}

AsirikuyReturnCode iPivot(int ratesArrayIndex, int shift,double *pPivot, double *pS1, double *pR1, double *pS2, double * pR2, double *pS3, double *pR3)
{
	return easyTradePtr->iPivot(ratesArrayIndex, shift, pPivot, pS1, pR1,pS2,pR2,pS3,pR3);
}

AsirikuyReturnCode validateHourlyBarsEasy(StrategyParams* pParams, int primary_rate, int hourly_bar)
{
	
	return easyTradePtr->validateHourlyBars(pParams, primary_rate, hourly_bar);
}

AsirikuyReturnCode validateDailyBarsEasy(StrategyParams* pParams, int primary_rate, int daily_rate)
{

	return easyTradePtr->validateDailyBars(pParams, primary_rate, daily_rate);
}

AsirikuyReturnCode validateSecondaryBarsEasy(StrategyParams* pParams, int primary_rate, int daily_rate, int secondary_tf, int rateErrorTimes)
{

	return easyTradePtr->validateSecondaryBars(pParams, primary_rate, daily_rate, secondary_tf, rateErrorTimes);
}

AsirikuyReturnCode validateCurrentTimeEasy(StrategyParams* pParams, int primary_rate)
{

	return easyTradePtr->validateCurrentTime(pParams, primary_rate);
}

int getLossTimesInWeekEasy(time_t currentTime,double * total_lost_pips)
{
	return easyTradePtr->getLossTimesInWeek(currentTime, total_lost_pips);
}
int getWinTimesInWeekEasy( time_t currentTime)
{
	return easyTradePtr->getWinTimesInWeek(currentTime);
}

int hasOpenOrder()
{
	return easyTradePtr->hasOpenOrder();
}

int hasSameDayOrderExcludeBreakeventOrdersEasy(time_t currentTime, BOOL * pIsOpen,double points)
{
	return easyTradePtr->hasSameDayOrderExcludeBreakeventOrders(currentTime, pIsOpen, points);
}

int hasSameDayOrderEasy(time_t currentTime,BOOL * pIsOpen)
{
	return easyTradePtr->hasSameDayOrder(currentTime, pIsOpen);
}

int hasSameWeekOrderEasy(time_t currentTime, BOOL * pIsOpen)
{
	return easyTradePtr->hasSameWeekOrder(currentTime, pIsOpen);
}

int getLossTimesInPreviousDaysEasy(time_t currentTime, double *pTotal_Lost_Pips)
{
	return easyTradePtr->getLossTimesInPreviousDays(currentTime, pTotal_Lost_Pips);
}

int getLossTimesFromNowsEasy(time_t currentTime, double *pTotal_Lost_Pips)
{
	return easyTradePtr->getLossTimesFromNow(currentTime, pTotal_Lost_Pips);
}

int getOrderCountTodayEasy(time_t currentTime)
{
	return easyTradePtr->getOrderCountToday(currentTime);
}

int getOrderCountForCurrentWeekEasy(time_t currentTime)
{
	return easyTradePtr->getOrderCountForCurrentWeek(currentTime);
}

int getOrderCountTodayExcludeBreakeventOrdersEasy(time_t currentTime, double points)
{
	return easyTradePtr->getOrderCountTodayExcludeBreakeventOrders(currentTime, points);
}

int getOrderCountEasy()
{
	return easyTradePtr->getOrderCount();
}

int getLossTimesInDayExcludeBreakeventOrdersEasy(time_t currentTime, double * total_lost_pips, double points)
{
	return easyTradePtr->getLossTimesInDayExcludeBreakeventOrders(currentTime, total_lost_pips,points);
}

int getLossTimesInDayEasy( time_t currentTime, double * total_lost_pips)
{
	return easyTradePtr->getLossTimesInDay(currentTime,total_lost_pips);
}
int getLossTimesInDayCloseOrderEasy(time_t currentTime, double * total_lost_pips)
{
	return easyTradePtr->getLossTimesInDayCloseOrder(currentTime, total_lost_pips);
}

int getWinTimesInDayEasy(time_t currentTime)
{
	return easyTradePtr->getWinTimesInDay(currentTime);
}

OrderType getLastestOpenOrderTypeEasy_GBPJPY(int rateIndex, double *pHigh, double *pLow)
{
	return easyTradePtr->getLastestOpenOrderType_GBPJPY(rateIndex, pHigh, pLow);
}

OrderType getLastestOpenOrderTypeEasy(int rateIndex, double *pHigh, double *pLow)
{
	return easyTradePtr->getLastestOpenOrderType(rateIndex, pHigh,pLow);
}

int getSameSideTradesInCurrentTrendEasy(int rateIndex, OrderType type)
{
	return easyTradePtr->getSameSideTradesInCurrentTrend(rateIndex, type);
}

int getSameSideWonTradesInCurrentTrendEasy(int rateIndex, OrderType type)
{
	return easyTradePtr->getSameSideWonTradesInCurrentTrend(rateIndex, type);
}

int getWinTimesInDaywithSamePriceEasy(time_t currentTime, double openPrice, double limit)
{
	return easyTradePtr->getWinTimesInDaywithSamePrice(currentTime, openPrice,limit);
}

int getLossTimesInDaywithSamePriceEasy(time_t currentTime, double openPrice, double limit)
{
	return easyTradePtr->getLossTimesInDaywithSamePrice(currentTime, openPrice, limit);
}

OrderType getLastestOrderTypeEasy(int rateIndex, double *pHigh, double *pLow, BOOL * pIsOpen)
{
	return easyTradePtr->getLastestOrderType(rateIndex, pHigh, pLow, pIsOpen);
}

OrderType getLastestOrderTypeXAUUSDEasy(int rateIndex, double *pHigh, double *pLow, BOOL * pIsOpen)
{
	return easyTradePtr->getLastestOrderType_XAUUSD(rateIndex, pHigh, pLow, pIsOpen);
}

double getLastestOrderPriceEasy(int rateIndex, BOOL * pIsOpen)
{
	return easyTradePtr->getLastestOrderPrice(rateIndex, pIsOpen);
}

int getLastestOrderIndexEasy(int rateIndex)
{
	return easyTradePtr->getLastestOrderIndex(rateIndex);
}

int getLastestOrderIndexExceptLimitAndStopOrdersEasy(int rateIndex, BOOL isClosedOnly)
{
	return easyTradePtr->getLastestOrderIndexExceptLimitAndStopOrders(rateIndex, isClosedOnly);
}

int getOldestOpenOrderIndexEasy(int rateIndex)
{
	return easyTradePtr->getOldestOpenOrderIndex(rateIndex);
}

double caculateStrategyWeeklyPNLEasy(time_t currentTime)
{
	return easyTradePtr->caculateStrategyWeeklyPNL(currentTime);
}

double adjustTakePrice_Weekly_Swing_Easy(int ratesArrayIndex, double init_takePrice)
{
	return easyTradePtr->adjustTakePrice_Weekly_Swing(ratesArrayIndex,init_takePrice);
}

double hasSameDayDayTradingOrderEasy(int rateIndex, OrderInfo * pOrder, double * pHigh, double * pLow)
{
	return easyTradePtr->hasSameDayDayTradingOrder(rateIndex,pOrder, pHigh, pLow);
}

AsirikuyReturnCode getHighLowEasy(int ratesArrayIndex, int shfitIndex, int shift, double *pHigh, double *pLow)
{
	return easyTradePtr->getHighLow(ratesArrayIndex, shfitIndex, shift, pHigh, pLow);
}

int iMACDTrendBeiLiEasy(int ratesArrayIndex, int fastPeriod, int slowPeriod, int signalPeriod, int startShift, double macdLimit, OrderType orderType, int *pTruningPointIndex, double *pTurningPoint, int * pMinPointIndex, double *pMinPoint)
{
	return easyTradePtr->iMACDTrendBeiLi(ratesArrayIndex, fastPeriod, slowPeriod, signalPeriod, startShift, macdLimit, orderType, pTruningPointIndex, pTurningPoint, pMinPointIndex, pMinPoint);
}

double iASIEasy(int ratesArrayIndex, int mode, int length, int smooth, double * outBull, double *outBear)
{
	return easyTradePtr->iASI(ratesArrayIndex, mode, length, smooth, outBull, outBear);
}

double caculateFreeMarginEasy(){
	return easyTradePtr->caculateFreeMargin();
}