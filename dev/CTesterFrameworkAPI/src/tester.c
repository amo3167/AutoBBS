//
//  tester.c
//  ast
//
//  Created by Jorge Ferrando on 11/04/13.
//  Copyright (c) 2013 Jorge Ferrando. All rights reserved.
//

#include "tester.h"
#include "OrderSignals.h"
#include "CTesterDefines.h"
#include "CTesterTradingStrategiesAPI.h"
#include "CTesterSymbolAnalyserAPI.h"
#include "stdlib.h"
#include "Precompiled.h"
#include "AsirikuyTime.h"
#include "AsirikuyDefines.h"

#define MATHEMATICAL_EXPECTANCY_LIMIT 50
#define MATHEMATICAL_EXPECTANCY_DIVISION 5

static void (*globalSignalUpdate)(TradeSignal signal);

static void sleepMilliseconds(int milliseconds)
{
#if defined _WIN32 || defined _WIN64
  Sleep(milliseconds); 
#elif defined __APPLE__ || defined __linux__
  usleep(milliseconds * 1000);
#else
  #error "Unsupported operating system"
#endif
}

const int SecondsPerMinute = 60;
const int SecondsPerHour = 3600;
const int SecondsPerDay = 86400;
const int DaysOfMonth[12] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

BOOL IsLeapYear(short year)
{
    if (year % 4 != 0) return false;
    if (year % 100 != 0) return true;
    return (year % 400) == 0;
}

time_t mkgmtime(short year, short month, short day, short hour, short minute, short second)
{
    time_t secs = 0;
	short y;
	short m;

    for (y = 1970; y < year; ++y)
        secs += (IsLeapYear(y)? 366: 365) * SecondsPerDay;
    for (m = 1; m < month; ++m) {
        secs += DaysOfMonth[m - 1] * SecondsPerDay;
        if (m == 2 && IsLeapYear(year)) secs += SecondsPerDay;
    }
    secs += (day - 1) * SecondsPerDay;
    secs += hour * SecondsPerHour;
    secs += minute * SecondsPerMinute;
    secs += second;
    return secs;
}


double standardDeviation(double data[], int n)
{
    double seriesCumulativeSquareReturns =0;
    double seriesCumulativeReturns =0;
    double sigma = 0;
    int i;
 
    for(i=0; i<n;++i){
    seriesCumulativeSquareReturns += data[i]*data[i];
    seriesCumulativeReturns += data[i];
    }
    
    sigma=sqrt((i*seriesCumulativeSquareReturns-seriesCumulativeReturns*seriesCumulativeReturns)/(i*(i-1)));
	
    return sigma;           
}

/* rounds double to n decimal places */
double roundN(double x, int prec)
{
	double power = 1.0;
	int i;

	if (prec > 0)
	for (i = 0; i < prec; i++)
	power *= 10.0;
	else if (prec < 0)
	for (i = 0; i < prec; i++)
	power /= 10.0;

	if (x > 0)
	x = floor(x * power + 0.5) / power;
	else if (x < 0)
	x = ceil(x * power - 0.5) / power;

	if (x == -0)
	x = 0;

	return x;
} 

//Calculates the profit of a closed Order
double getOrderProfit(COrderInfo order, char* tradeSymbol, double contractSize, double tickConversion){

	if (order.type != BUY && order.type != SELL)
		return 0;

	if (order.type == BUY)
		return (order.closePrice - order.openPrice) * order.lots * contractSize * tickConversion;
	if (order.type == SELL)
		return (order.openPrice - order.closePrice) * order.lots * contractSize * tickConversion;
	
	else return 0;
}

int getDecimals(double minLotSize){
	int i = 0;
	int decimals = 0;

	if (minLotSize >= 1) return(0);

	while (decimals == 0)
	{
		if (minLotSize*pow(10.0, i) >= 1){ 
			decimals = i; 
		}

		i++;
	}

	return(decimals);
}

int openOrder(StrategyResults* strategyResults, int* openOrdersCount, COrderInfo* openOrders, int instanceId, int* ticketNumber, int openTime, double openPrice, int type, TradeSignal lastSignal, int *numSignals, double currentBalance, double minLotSize, double minimumStop){
	int orderIndex = openOrdersCount[BUY] + openOrdersCount[SELL];

	if (strategyResults->lots >= minLotSize)
	{
		openOrders[orderIndex].lots = roundN(strategyResults->lots, getDecimals(minLotSize));
	}
	else {
		pantheios_logprintf(PANTHEIOS_SEV_NOTICE, (PAN_CHAR_T*)"Order %d, lots = %lf, below minimum lot size (%lf). Order was NOT opened (rounding up is prevented to avoid increasing risk)", (int)openOrders[orderIndex].ticket,  (double)strategyResults->lots, (double) minLotSize);
		return false;
	}

	openOrders[orderIndex].ticket = *ticketNumber + 1;
	openOrders[orderIndex].instanceId = instanceId;
	openOrders[orderIndex].openTime = openTime;

	if(type == BUY || type == SELL){
		openOrders[orderIndex].openPrice = openPrice;
	} else {
		openOrders[orderIndex].openPrice= strategyResults->entryPrice;
		openPrice = openOrders[orderIndex].openPrice;
	}

	openOrders[orderIndex].isOpen = true;
	openOrders[orderIndex].swap = 0;
	openOrders[orderIndex].profit = 0;
	openOrders[orderIndex].stopLoss = 0;
	openOrders[orderIndex].takeProfit = 0;

	if (type == BUY  || type == BUYLIMIT || type == BUYSTOP){

		openOrders[orderIndex].type = type;

		if (strategyResults->brokerSL != 0){
			if (openPrice - strategyResults->brokerSL <= openPrice - minimumStop && strategyResults->brokerSL != 0){
				openOrders[orderIndex].stopLoss = openPrice - strategyResults->brokerSL;
			} else {
				pantheios_logprintf(PANTHEIOS_SEV_ERROR, (PAN_CHAR_T*)"OpenOrder. Invalid SL requested. Current ask is %lf, minimum allowed SL is %lf, attempted SL is %lf. This SL modification has been aborted.", openPrice, openPrice - minimumStop, openPrice - strategyResults->brokerSL);
			}
		}
		
		if (strategyResults->brokerTP != 0){
			if (openPrice + strategyResults->brokerTP >= openPrice + minimumStop && strategyResults->brokerTP != 0){
				openOrders[orderIndex].takeProfit = openPrice + strategyResults->brokerTP;
			} else {
				pantheios_logprintf(PANTHEIOS_SEV_ERROR, (PAN_CHAR_T*)"OpenOrder. Invalid TP requested. Current ask is %lf, minimum allowed TP is %lf, attempted TP is %lf. This TP modification has been aborted.", openPrice, openPrice + minimumStop, openPrice + strategyResults->brokerTP);
			}
		}

		openOrdersCount[BUY]++;
	}
	else {
		openOrders[orderIndex].type = type;
		openOrders[orderIndex].stopLoss = 0;
		openOrders[orderIndex].takeProfit = 0;

		if (strategyResults->brokerSL != 0){
			if (openPrice + strategyResults->brokerSL >= openPrice + minimumStop && strategyResults->brokerSL != 0){
				openOrders[orderIndex].stopLoss = openPrice + strategyResults->brokerSL;
			} else {
				pantheios_logprintf(PANTHEIOS_SEV_ERROR, (PAN_CHAR_T*)"OpenOrder. Invalid SL requested. Current bid is %lf, minimum allowed SL is %lf, attempted SL is %lf. This SL modification has been aborted.", openPrice, openPrice + minimumStop, openPrice + strategyResults->brokerSL);
			}
		}

		if (strategyResults->brokerTP != 0){
			if (openPrice - strategyResults->brokerTP <= openPrice - minimumStop && strategyResults->brokerTP != 0){
				openOrders[orderIndex].takeProfit = openPrice - strategyResults->brokerTP;
			} else {
				pantheios_logprintf(PANTHEIOS_SEV_ERROR, (PAN_CHAR_T*)"OpenOrder. Invalid TP requested. Current bid is %lf, minimum allowed TP is %lf, attempted TP is %lf. This TP modification has been aborted.", openPrice, openPrice - minimumStop, openPrice - strategyResults->brokerTP);
			}
		}
		
		openOrdersCount[SELL]++;
	}

	if (strategyResults->brokerTP == 0) openOrders[orderIndex].takeProfit = 0;
	if (strategyResults->brokerSL == 0) openOrders[orderIndex].stopLoss   = 0;

	if(globalSignalUpdate!=NULL){
		lastSignal.no = numSignals[lastSignal.testId];
		numSignals[lastSignal.testId]++;
		if (type == BUY) lastSignal.type = SIGNAL_BUY;
		if (type == SELL) lastSignal.type = SIGNAL_SELL;
		lastSignal.orderId = (int)openOrders[orderIndex].ticket;
		lastSignal.price = openOrders[orderIndex].openPrice;
		lastSignal.lots = openOrders[orderIndex].lots;
		lastSignal.sl = openOrders[orderIndex].stopLoss;
		lastSignal.tp = openOrders[orderIndex].takeProfit;
		lastSignal.profit = 0;
		lastSignal.balance = currentBalance;
		globalSignalUpdate(lastSignal);
	}

	*ticketNumber = *ticketNumber + 1;
	return true;
}

double calculateAccountEquity(double* pInAccountInfo, int* openOrdersCount, COrderInfo* openOrders, double tickConversion){
	
	double accountEquity;
	int orderIndex = openOrdersCount[BUY] + openOrdersCount[SELL];
	int i, found = false;
	
	accountEquity = pInAccountInfo[IDX_BALANCE];

	for(i=0; i<orderIndex; i++){
		if (openOrders[orderIndex].isOpen == 1){
			accountEquity -= openOrders[orderIndex].lots*tickConversion*pInAccountInfo[IDX_CONTRACT_SIZE]/pInAccountInfo[IDX_LEVERAGE];
		}
	}

	return accountEquity ;
}

int updateOrder(int instanceId, StrategyResults* strategyResults, int* openOrdersCount, COrderInfo* openOrders, double  *bidAsk, int type, TradeSignal lastSignal, int *numSignals, double currentBalance, double minimumStop){
	int orderIndex = openOrdersCount[BUY] + openOrdersCount[SELL];
	int i, found = false;

	for(i=0; i<orderIndex; i++){

		if((openOrders[i].instanceId == instanceId) && (openOrders[i].ticket == strategyResults->ticketNumber || strategyResults->ticketNumber == -1) && (openOrders[i].type == type)){
			
			if(openOrders[i].type == BUY || openOrders[i].type == BUYSTOP || openOrders[i].type == BUYLIMIT ){

				if (strategyResults->brokerSL != 0 && openOrders[i].type == BUY){
					if (bidAsk[IDX_ASK] - strategyResults->brokerSL <= bidAsk[IDX_ASK] - minimumStop && strategyResults->brokerSL != 0){
					openOrders[i].stopLoss = bidAsk[IDX_ASK] - strategyResults->brokerSL;
					} else {
						pantheios_logprintf(PANTHEIOS_SEV_ERROR, (PAN_CHAR_T*)"OrderModify. Invalid SL requested. Current ask is %lf, minimum allowed SL is %lf, attempted SL is %lf. This SL modification has been aborted.", bidAsk[IDX_ASK], bidAsk[IDX_ASK] - minimumStop, bidAsk[IDX_ASK] - strategyResults->brokerSL);
					}
				}

				if (strategyResults->brokerTP != 0 && openOrders[i].type == BUY){
					if (bidAsk[IDX_ASK] + strategyResults->brokerTP >= bidAsk[IDX_ASK] + minimumStop && strategyResults->brokerTP != 0){
					openOrders[i].takeProfit = bidAsk[IDX_ASK] + strategyResults->brokerTP;
					} else {
						pantheios_logprintf(PANTHEIOS_SEV_ERROR, (PAN_CHAR_T*)"OrderModify. Invalid TP requested. Current ask is %lf, minimum allowed TP is %lf, attempted TP is %lf. This TP modification has been aborted.", bidAsk[IDX_ASK], bidAsk[IDX_ASK] + minimumStop, bidAsk[IDX_ASK] + strategyResults->brokerTP);
					}
				}

				if (strategyResults->brokerSL != 0 && (openOrders[i].type == BUYLIMIT || openOrders[i].type == BUYSTOP)){
					if (openOrders[i].openPrice - strategyResults->brokerSL <= openOrders[i].openPrice - minimumStop && strategyResults->brokerSL != 0){
					openOrders[i].stopLoss = openOrders[i].openPrice - strategyResults->brokerSL;
					} else {
						pantheios_logprintf(PANTHEIOS_SEV_ERROR, (PAN_CHAR_T*)"OrderModify. Invalid SL requested. Current ask is %lf, minimum allowed SL is %lf, attempted SL is %lf. This SL modification has been aborted.", openOrders[i].openPrice, openOrders[i].openPrice - minimumStop, openOrders[i].openPrice - strategyResults->brokerSL);
					}
				}

				if (strategyResults->brokerTP != 0 && (openOrders[i].type == BUYLIMIT || openOrders[i].type == BUYSTOP)){
					if (openOrders[i].openPrice + strategyResults->brokerTP >= openOrders[i].openPrice + minimumStop && strategyResults->brokerTP != 0){
					openOrders[i].takeProfit = openOrders[i].openPrice + strategyResults->brokerTP;
					} else {
						pantheios_logprintf(PANTHEIOS_SEV_ERROR, (PAN_CHAR_T*)"OrderModify. Invalid TP requested. Current ask is %lf, minimum allowed TP is %lf, attempted TP is %lf. This TP modification has been aborted.", openOrders[i].openPrice, openOrders[i].openPrice + minimumStop, openOrders[i].openPrice + strategyResults->brokerTP);
					}
				}

				if(globalSignalUpdate!=NULL){
					lastSignal.no = numSignals[lastSignal.testId];
					numSignals[lastSignal.testId]++;
					lastSignal.orderId = (int)openOrders[i].ticket;		
					lastSignal.lots = openOrders[i].lots;
					lastSignal.sl = openOrders[i].stopLoss;
					lastSignal.tp = openOrders[i].takeProfit;
					lastSignal.type = SIGNAL_MODIFY;
					lastSignal.profit = 0;
					lastSignal.price = bidAsk[IDX_ASK];
					lastSignal.balance = currentBalance;
					globalSignalUpdate(lastSignal);
				}

				pantheios_logprintf(PANTHEIOS_SEV_DEBUG, (PAN_CHAR_T*)"Update Order. ticket = %lf, instanceID = %lf, Entry = %lf, SL = %lf, TP =%lf", openOrders[i].ticket, openOrders[i].instanceId, openOrders[i].openPrice, openOrders[i].stopLoss, openOrders[i].takeProfit);
			}
			if(openOrders[i].type == SELL || openOrders[i].type == SELLSTOP || openOrders[i].type == SELLLIMIT){

				if (strategyResults->brokerSL != 0 && openOrders[i].type == SELL){
					if (bidAsk[IDX_BID] + strategyResults->brokerSL >= bidAsk[IDX_BID] + minimumStop){
						openOrders[i].stopLoss = bidAsk[IDX_BID] + strategyResults->brokerSL;
					} else {
						pantheios_logprintf(PANTHEIOS_SEV_ERROR, (PAN_CHAR_T*)"OrderModify. Invalid SL requested. Current bid is %lf, minimum allowed SL is %lf, attempted SL is %lf. This SL modification has been aborted.", bidAsk[IDX_BID], bidAsk[IDX_BID] + minimumStop, bidAsk[IDX_BID] + strategyResults->brokerSL);
					}
				}

				if (strategyResults->brokerTP != 0 && openOrders[i].type == SELL){
					if (bidAsk[IDX_BID] - strategyResults->brokerTP <= bidAsk[IDX_BID] - minimumStop){
						openOrders[i].takeProfit = bidAsk[IDX_BID] - strategyResults->brokerTP;
					} else {
						pantheios_logprintf(PANTHEIOS_SEV_ERROR, (PAN_CHAR_T*)"OrderModify. Invalid TP requested. Current bid is %lf, minimum allowed TP is %lf, attempted TP is %lf. This TP modification has been aborted.", bidAsk[IDX_BID], bidAsk[IDX_BID] - minimumStop, bidAsk[IDX_BID] - strategyResults->brokerTP);
					}
				}

				if (strategyResults->brokerSL != 0 && (openOrders[i].type == SELLLIMIT || openOrders[i].type == SELLSTOP)){
					if (openOrders[i].openPrice + strategyResults->brokerSL >= openOrders[i].openPrice + minimumStop){
						openOrders[i].stopLoss = openOrders[i].openPrice + strategyResults->brokerSL;
					} else {
						pantheios_logprintf(PANTHEIOS_SEV_ERROR, (PAN_CHAR_T*)"OrderModify. Invalid SL requested. Current bid is %lf, minimum allowed SL is %lf, attempted SL is %lf. This SL modification has been aborted.", openOrders[i].openPrice, openOrders[i].openPrice + minimumStop, openOrders[i].openPrice + strategyResults->brokerSL);
					}
				}

				if (strategyResults->brokerTP != 0 && (openOrders[i].type == SELLLIMIT || openOrders[i].type == SELLSTOP)){
					if (openOrders[i].openPrice - strategyResults->brokerTP <= openOrders[i].openPrice - minimumStop){
						openOrders[i].takeProfit = openOrders[i].openPrice - strategyResults->brokerTP;
					} else {
						pantheios_logprintf(PANTHEIOS_SEV_ERROR, (PAN_CHAR_T*)"OrderModify. Invalid TP requested. Current bid is %lf, minimum allowed TP is %lf, attempted TP is %lf. This TP modification has been aborted.", openOrders[i].openPrice, openOrders[i].openPrice - minimumStop, openOrders[i].openPrice - strategyResults->brokerTP);
					}
				}


				if(globalSignalUpdate!=NULL){
					lastSignal.no = numSignals[lastSignal.testId];
					numSignals[lastSignal.testId]++;
					lastSignal.orderId = (int)openOrders[i].ticket;	
					lastSignal.lots = openOrders[i].lots;
					lastSignal.sl = openOrders[i].stopLoss;
					lastSignal.tp = openOrders[i].takeProfit;		
					lastSignal.profit = 0;
					lastSignal.type = SIGNAL_MODIFY;
					lastSignal.price = bidAsk[IDX_BID];
					lastSignal.balance = currentBalance;
					globalSignalUpdate(lastSignal);
				}

				pantheios_logprintf(PANTHEIOS_SEV_DEBUG, (PAN_CHAR_T*)"Update Order. ticket = %lf, instanceID = %lf, Entry = %lf, SL = %lf, TP =%lf", openOrders[i].ticket, openOrders[i].instanceId, openOrders[i].openPrice, openOrders[i].stopLoss, openOrders[i].takeProfit);
			}

			if (strategyResults->brokerTP == 0) openOrders[i].takeProfit = 0;
			if (strategyResults->brokerSL == 0) openOrders[i].stopLoss   = 0;
		}
	}

	return true;
}


double closeOrder(StrategyResults* strategyResults, int* openOrdersCount, COrderInfo* openOrders, int instanceId, int closeTime, double closePrice, COrderInfo* result, char* tradeSymbol, int type, double *profit, double contractSize, void (*globalSignalUpdate)(TradeSignal signal), TradeSignal lastSignal, int *numSignals, double currentBalance, double tickConversion, double* avgTradeDuration){
	int orderIndex = openOrdersCount[BUY] + openOrdersCount[SELL];
	int i, found;
	double totalProfit = 0;

	found = false;
	//For all the open orders
	for(i=0; i<orderIndex; i++){
		if((openOrders[i].instanceId == instanceId) && (openOrders[i].ticket == strategyResults->ticketNumber || strategyResults->ticketNumber == -1) && openOrders[i].type == type && openOrders[i].isOpen){
			
			// if the close time is before the open time then continue
            if (openOrders[i].openTime > closeTime){
                pantheios_logprintf(PANTHEIOS_SEV_ERROR, (PAN_CHAR_T*)"Close time is earlier than open time. Ignoring close order. OpenTime = %lf, CloseTime = %d", openOrders[i].openTime, closeTime);				
                return false;
            }

			openOrders[i].closeTime = closeTime;
			openOrders[i].closePrice = closePrice;		
			openOrders[i].isOpen = false;

			if (openOrders[i].type == BUY || openOrders[i].type == SELL){
				*avgTradeDuration += openOrders[i].closeTime-openOrders[i].openTime;
				openOrders[i].profit = getOrderProfit(openOrders[i], tradeSymbol, contractSize, tickConversion);
			} else {
				openOrders[i].profit = 0;
			}

			openOrders[i].isOpen = false;
			memcpy(result, &openOrders[i], sizeof(COrderInfo));
			totalProfit+= openOrders[i].profit+openOrders[i].swap;

			if (openOrders[i].type == BUY || openOrders[i].type == BUYLIMIT || openOrders[i].type == BUYSTOP)
				openOrdersCount[BUY]--;
			else
				openOrdersCount[SELL]--;
			found = true;

			if(globalSignalUpdate!=NULL){
				lastSignal.no = numSignals[lastSignal.testId];
				numSignals[lastSignal.testId]++;
				lastSignal.orderId = (int)openOrders[i].ticket;	
				lastSignal.lots = openOrders[i].lots;
				lastSignal.sl = openOrders[i].stopLoss;
				lastSignal.tp = openOrders[i].takeProfit;		
				lastSignal.profit = openOrders[i].profit+openOrders[i].swap;
				lastSignal.type = SIGNAL_CLOSE;
				lastSignal.price = closePrice;
				lastSignal.balance = currentBalance + lastSignal.profit;

				globalSignalUpdate(lastSignal);
			}

			//Move the closed position to the last position of the array
			memmove(&openOrders[i], &openOrders[i+1], sizeof(COrderInfo)*(MAX_ORDERS - i - 1));
			memcpy(&openOrders[MAX_ORDERS-1], result, sizeof(COrderInfo));
			i--;
		}
	}
	*profit = totalProfit;

	if(found)  {
		return true;
	} else {
		return false;
    }
}

//Checks if open orders touch the SL or TP
double checkTPSL(double bid, double ask, int i, int shift1, CRates *rates0, int* openOrdersCount, COrderInfo* openOrders, int instanceId, int closeTime, COrderInfo* result, char* tradeSymbol, double *profit, double contractSize, TradeSignal lastSignal, int *numSignals, double currentBalance, double tickConversion, double* avgTradeDuration){
	int found, touchedTPSL, triggeredSL;
	double totalProfit = 0;
	double spread = fabs(ask-bid);

	found = false;
	//For all the open orders
		touchedTPSL = false;
		triggeredSL = false;
		if(openOrders[i].type == BUY && openOrders[i].isOpen && openOrders[i].instanceId == instanceId){

			if ((openOrders[i].stopLoss>=rates0[shift1].low || openOrders[i].stopLoss>=bid) && openOrders[i].stopLoss != 0) {
				
				if (openOrders[i].stopLoss>=bid && openOrders[i].stopLoss<rates0[shift1].low){
					openOrders[i].closePrice = bid;
				} else {
					openOrders[i].closePrice = openOrders[i].stopLoss;
				}

				pantheios_logprintf(PANTHEIOS_SEV_DEBUG, (PAN_CHAR_T*)"BUY Order hit SL. Ticket = %f", openOrders[i].ticket);
				touchedTPSL = true;
				triggeredSL = true;
			}
			
			if ((openOrders[i].takeProfit<=rates0[shift1].high || openOrders[i].takeProfit<=bid) && !triggeredSL && openOrders[i].takeProfit != 0){
				openOrders[i].closePrice = openOrders[i].takeProfit;
				pantheios_logprintf(PANTHEIOS_SEV_DEBUG, (PAN_CHAR_T*)"BUY Order hit TP. Ticket = %f", openOrders[i].ticket);
				touchedTPSL = true;
			}
		}
		if(openOrders[i].type == SELL && openOrders[i].isOpen && openOrders[i].instanceId == instanceId){

			if ((openOrders[i].stopLoss<=rates0[shift1].high+spread  || openOrders[i].stopLoss<=ask) && openOrders[i].stopLoss != 0) {
				
				if (openOrders[i].stopLoss<=ask && openOrders[i].stopLoss > rates0[shift1].high+spread){
					openOrders[i].closePrice = ask;
				} else {
					openOrders[i].closePrice = openOrders[i].stopLoss;
				}

				pantheios_logprintf(PANTHEIOS_SEV_DEBUG, (PAN_CHAR_T*)"SELL Order hit SL. Ticket = %f", openOrders[i].ticket);
				triggeredSL = true;
				touchedTPSL = true;
			}

			if((openOrders[i].takeProfit>=rates0[shift1].low+spread || openOrders[i].takeProfit>=ask) && !triggeredSL && openOrders[i].takeProfit != 0){
				pantheios_logprintf(PANTHEIOS_SEV_DEBUG, (PAN_CHAR_T*)"SELL Order hit TP. Ticket = %f", openOrders[i].ticket);
				openOrders[i].closePrice = openOrders[i].takeProfit;
				touchedTPSL = true;
			}
		}
		if(touchedTPSL){

			// if the close time is before the open time then continue
            if (openOrders[i].openTime > closeTime){
                pantheios_logprintf(PANTHEIOS_SEV_ERROR, (PAN_CHAR_T*)"Close time is earlier than open time. Ignoring SL/TP hit. OpenTime = %lf, CloseTime = %d", openOrders[i].openTime, closeTime);				
                return false;
            }

			openOrders[i].closeTime = closeTime;
			openOrders[i].profit = getOrderProfit(openOrders[i], tradeSymbol, contractSize, tickConversion);
			openOrders[i].isOpen = false;
			*avgTradeDuration += openOrders[i].closeTime-openOrders[i].openTime;
			memcpy(result, &openOrders[i], sizeof(COrderInfo));
			totalProfit+= openOrders[i].profit+openOrders[i].swap;
			if (openOrders[i].type == BUY)
				openOrdersCount[BUY]--;
			else
				openOrdersCount[SELL]--;
			found = true;

			if(globalSignalUpdate!=NULL){
				lastSignal.no = numSignals[lastSignal.testId];
				numSignals[lastSignal.testId]++;
				lastSignal.orderId = (int)openOrders[i].ticket;	
				lastSignal.lots = openOrders[i].lots;
				lastSignal.sl = openOrders[i].stopLoss;
				lastSignal.tp = openOrders[i].takeProfit;		
				lastSignal.profit = openOrders[i].profit+openOrders[i].swap;
				if (openOrders[i].profit > 0) lastSignal.type = SIGNAL_CLOSE_TP;
				else lastSignal.type = SIGNAL_CLOSE_SL;
				lastSignal.price = openOrders[i].closePrice;
				lastSignal.balance = currentBalance + lastSignal.profit;
				globalSignalUpdate(lastSignal);
			}
			//Move the closed position to the last position of the array
			memmove(&openOrders[i], &openOrders[i+1], sizeof(COrderInfo)*(MAX_ORDERS - i - 1));
			memcpy(&openOrders[MAX_ORDERS-1], result, sizeof(COrderInfo));
			i--;
		}

	*profit = totalProfit;

	if(found) {
		return true;
	} else {
		return false;
    }
}

int addInterest(int* openOrdersCount, COrderInfo* openOrders, int instanceId, int currentTime, double contractSize, double swapLong, double swapShort, double  *bidAsk, int lastInterestAdditionTime){
	int orderIndex = openOrdersCount[BUY] + openOrdersCount[SELL];
	int i;
	double swapInterest = 0;
	int newAdditionTime;
	struct tm  timeInfo;

	newAdditionTime = lastInterestAdditionTime;

	safe_gmtime(&timeInfo, currentTime);

	//For all the open orders
	for(i=0; i<orderIndex; i++){
		if(
			(openOrders[i].instanceId == instanceId) && 
			//(timeInfo.tm_hour == 17 ) && 
			//(timeInfo.tm_min == 0) &&
			(currentTime - lastInterestAdditionTime > 3600)
			){	
			//if(openOrders[i].type == BUY)  swapInterest = (swapLong/(365*100))*contractSize*openOrders[i].lots;
			//if(openOrders[i].type == SELL) swapInterest = (swapShort/(365*100))*contractSize*openOrders[i].lots;

			if (openOrders[i].type == BUY)  swapInterest = swapLong *openOrders[i].lots;
			if (openOrders[i].type == SELL) swapInterest = swapShort *openOrders[i].lots;


			if(timeInfo.tm_wday == 3) swapInterest *= 3;	

			pantheios_logprintf(PANTHEIOS_SEV_DEBUG, (PAN_CHAR_T*)"Adding Swap interest = %lf, swapLong = %lf, swapShort = %lf, hours = %d, dayOfweek = %d, contractSize = %lf, volume = %lf BidAsk = %lf/%lf", swapInterest, swapLong, swapShort, timeInfo.tm_hour, timeInfo.tm_wday, contractSize, openOrders[i].lots, bidAsk[IDX_BID], bidAsk[IDX_ASK]);
			openOrders[i].swap += swapInterest; 
			newAdditionTime = currentTime;
		}
	}

	return(newAdditionTime);
}

void addStatisticValue(StatisticItem **statistics, int *statisticsSize, double profit, double finalBalance, int currentBrokerTime){
	if(*statisticsSize > MIN_STATISTICS_SIZE - 1){
		StatisticItem *temp;
		temp = (StatisticItem*)realloc(*statistics, (*statisticsSize + 1)*sizeof(StatisticItem)); /* give the pointer some memory */
		if ( temp != NULL ) {
			*statistics=temp;
		}
		else free(statistics);
	}
	(*statistics)[*statisticsSize].balance = finalBalance;
	(*statistics)[*statisticsSize].profit = profit;
	(*statistics)[*statisticsSize].time = currentBrokerTime;
	(*statisticsSize)++;
}

void initialize_me(int is_calculate_expectancy){
	FILE* fp;
	int p;
	char mathematicalExpectancyLabels[MAX_FILE_PATH_CHARS] = "";
	char label[MAX_FILE_PATH_CHARS] = "";

	if (is_calculate_expectancy == FALSE) return;

	fp=fopen("ME_analysis.csv","w");

	for(p=1;  p < MATHEMATICAL_EXPECTANCY_LIMIT+1; p++){
			if (p % MATHEMATICAL_EXPECTANCY_DIVISION == 0){
				sprintf(label, "ME_%d,MFE_%d,MAE_%d", p, p, p);
				strcat(mathematicalExpectancyLabels, label);	

				if (p != MATHEMATICAL_EXPECTANCY_LIMIT){
					strcat(mathematicalExpectancyLabels, ",");
				}
			}
	}

	fprintf(fp,"OpenPrice,OrderType,%s\n",mathematicalExpectancyLabels);
	fclose(fp);
}

void calculate_mathematical_expectancy(int orderType, int numCandles, int shift, double me_entry, ASTRates* rates, double spread, int is_calculate_expectancy){

	char mathematicalExpectancyString[MAX_FILE_PATH_CHARS*2] = "";
	char me_string[MAX_FILE_PATH_CHARS*2] = "";
	int p;
	FILE* fp;
	double me_mae;
	double me_mfe;

	if (is_calculate_expectancy == FALSE) return;

	me_mae = 0;
	me_mfe = 0;

					if(shift+MATHEMATICAL_EXPECTANCY_LIMIT < numCandles-2){

						fp=fopen("ME_analysis.csv","a");

						for(p=1;  p < MATHEMATICAL_EXPECTANCY_LIMIT+1; p++){

							if (orderType == BUY){

								if (rates[shift+p].high-me_entry > me_mfe){
									me_mfe = rates[shift+p].high-me_entry;
								}
					
								if (me_entry-rates[shift+p].low > me_mae){
									me_mae = me_entry-rates[shift+p].low;
								}	

							}

							if (orderType == SELL){

								if ((rates[shift+p].high+spread)-me_entry > me_mae){
									me_mae = (rates[shift+p].high+spread)-me_entry;
								}
								if (me_entry-(rates[shift+p].low+spread) > me_mfe){
									me_mfe = me_entry-(rates[shift+p].low+spread);
								}	

							}

							if (p % MATHEMATICAL_EXPECTANCY_DIVISION == 0){
								sprintf(me_string, "%lf,%lf,%lf", me_mfe-me_mae, me_mfe, me_mae);
								strcat(mathematicalExpectancyString, me_string);

								if (p != MATHEMATICAL_EXPECTANCY_LIMIT){
									strcat(mathematicalExpectancyString, ",");
								}
							}					
						}
						
						if (orderType == BUY) fprintf(fp,"%f,BUY,%s\n",me_entry, mathematicalExpectancyString);
						if (orderType == SELL) fprintf(fp,"%f,SELL,%s\n",me_entry, mathematicalExpectancyString);
						fclose(fp);
					}
}

//Checks if pending orders are triggered
void checkPending(double bid, double ask, int i, COrderInfo* openOrders, int instanceId, int openTime, int numCandles, int shift, ASTRates* rates, int testUpdate, int is_calculate_expectancy){

	//For all the open orders check whether pending orders have been triggered
		if(openOrders[i].type == BUYLIMIT && openOrders[i].isOpen && openOrders[i].instanceId == instanceId && rates[shift].low<openOrders[i].openPrice){
		   openOrders[i].openTime = openTime;
		   openOrders[i].type = BUY;
		   if(testUpdate == TRUE) calculate_mathematical_expectancy(BUY, numCandles, shift, ask, rates, fabs(ask-bid), is_calculate_expectancy) ;

		}

		if(openOrders[i].type == BUYSTOP && openOrders[i].isOpen && openOrders[i].instanceId == instanceId && rates[shift].high>openOrders[i].openPrice){
		   openOrders[i].openTime = openTime;
		   openOrders[i].type = BUY;
		   if(testUpdate== TRUE) calculate_mathematical_expectancy(BUY, numCandles, shift, ask, rates, fabs(ask-bid), is_calculate_expectancy) ;
		}

		if(openOrders[i].type == SELLLIMIT && openOrders[i].isOpen && openOrders[i].instanceId == instanceId && rates[shift].high>openOrders[i].openPrice){
		   openOrders[i].openTime = openTime;
		   openOrders[i].type = SELL;
		   if(testUpdate== TRUE) calculate_mathematical_expectancy(SELL, numCandles, shift, bid, rates, fabs(ask-bid), is_calculate_expectancy) ;
		}

		if(openOrders[i].type == SELLSTOP && openOrders[i].isOpen && openOrders[i].instanceId == instanceId && rates[shift].low<openOrders[i].openPrice){
		   openOrders[i].openTime = openTime;
		   openOrders[i].type = SELL;
		   if(testUpdate== TRUE) calculate_mathematical_expectancy(SELL, numCandles, shift, bid, rates, fabs(ask-bid), is_calculate_expectancy) ;
		}
	
}

void save_openorder_to_file(){
	
	FILE* openOrderFile;	
	char   timeString[MAX_TIME_STRING_SIZE] = "";
	int orderIndex;
	int index;
	
	openOrderFile = fopen("results.open", "w");
	if (openOrderFile == NULL)
		pantheios_logputs(PANTHEIOS_SEV_CRITICAL, (PAN_CHAR_T*)"Failed to save openOrderFile.");

	fprintf(openOrderFile, "Open order exist!");

	fclose(openOrderFile);
	
}

    
void save_statistics_to_file(TestResult testResult, 
                             double finalBalance,
                             double initialBalance){
    // save all statistics to file
            FILE* statisticsFile;
            
			statisticsFile = fopen("allStatistics.csv","w");
			if(statisticsFile == NULL)
			pantheios_logputs(PANTHEIOS_SEV_CRITICAL, (PAN_CHAR_T*)"Failed to save general statistics.");


			fprintf(statisticsFile, "total_return, %f\n", finalBalance/initialBalance);
			fprintf(statisticsFile, "max_dd, %f\n", testResult.maxDDDepth);
			fprintf(statisticsFile, "max_dd_length, %f\n", testResult.maxDDLength/SecondsPerDay);
			fprintf(statisticsFile, "profit_factor, %f\n", testResult.pf);
			fprintf(statisticsFile, "cagr, %f\n", testResult.cagr);
			fprintf(statisticsFile, "cagr_maxdd, %f\n", testResult.cagr/testResult.maxDDDepth);
			fprintf(statisticsFile, "sharpe_ratio, %f\n", testResult.sharpe);
			fprintf(statisticsFile, "num_trades, %d\n", testResult.totalTrades);
			fprintf(statisticsFile, "num_longs, %d\n", testResult.numLongs);
			fprintf(statisticsFile, "num_shorts, %d\n", testResult.numShorts);
			fprintf(statisticsFile, "trades_week, %f\n", testResult.totalTrades/(testResult.yearsTraded*52));
			fprintf(statisticsFile, "trade_duration, %f\n", testResult.avgTradeDuration/3600);
			fprintf(statisticsFile, "r2, %f\n", testResult.r2);
			fprintf(statisticsFile, "winning, %f\n", testResult.winning);
			fprintf(statisticsFile, "risk_reward, %f\n", testResult.risk_reward);

			fclose(statisticsFile) ;
}

void calculate_trade_by_trade_statistics(StatisticItem *statistics, 
                                         int statisticsSize, 
                                         double initialBalance, 
                                         int is_compounding_disabled, 
                                         int totalTrades,
                                         TestResult *testResult, 
                                         int numSystems,
                                         int is_optimization){
    //Calculate statistics
    // variables for jonathan worst case definition
	
    double maxBalance = initialBalance;
	double avgBalanceLog = 0;
	double maxDDDepth = 0;
	int maxDDLength = 0;
	int ddStartTime = statistics[0].time;
	double averageWinningTrade = 0;
	double averageLosingTrade = 0;
    double seriesCumulativeSquareReturns = 0;
	double seriesCumulativeReturns = 0;
	double sigma = 0;
    double *regressionResiduals;
    double sumBalanceTime = 0;
    double timeSqrSum = 0;
    double tradeReturn = 0;
    double previousBalance;
    double  profit;
    double totalWin = 0;
    double totalLose = 0;
    double avgTime=0;
    double avgBalance=0;
    double totalWinningTrades = 0;
    double totalLosingTrades = 0;
    double maxDDDepthTemp = 0;
    double maxDDLengthTemp;
    double yPs = 0;
	double yRs = 0;
    double linearRegressionSlope;
    double linearRegressionIntercept;
    double regressionResidualsStandardDeviation;
    
    
    int j;
    
	for(j=0; j < statisticsSize; j++){
        
        timeSqrSum += pow((double)statistics[j].time-statistics[0].time, 2) ;

        // calculations needed for linear regression
		if(is_compounding_disabled == TRUE){
			sumBalanceTime += (statistics[j].time-statistics[0].time)*(statistics[j].balance-statistics[0].balance);
		} else {
			sumBalanceTime += (statistics[j].time-statistics[0].time)*(log(statistics[j].balance)- log(statistics[0].balance));
		}
        
        avgTime += (statistics[j].time-statistics[0].time) / statisticsSize;
		avgBalance += (statistics[j].balance-statistics[0].balance) / statisticsSize;
		avgBalanceLog += (log(statistics[j].balance)-log(statistics[0].balance)) / statisticsSize;

		//Calculate profit factor
		if(statistics[j].profit > 0){
			totalWin += statistics[j].profit;
			totalWinningTrades +=1;
			averageWinningTrade = averageWinningTrade*(totalWinningTrades-1)/totalWinningTrades + fabs((statistics[j].profit/statistics[j].balance)/totalWinningTrades);
		} else { 
			totalLose += fabs(statistics[j].profit);
			totalLosingTrades +=1;
			averageLosingTrade = averageLosingTrade*(totalLosingTrades-1)/totalLosingTrades + fabs((statistics[j].profit/statistics[j].balance)/totalLosingTrades);
		}

		//calculate maxdrawdown depth and max drawdown length
		if(statistics[j].balance < maxBalance){

			if(is_compounding_disabled == TRUE){
				maxDDDepthTemp  = ((maxBalance-statistics[j].balance)/initialBalance)*100 ;
			} else {
				maxDDDepthTemp  = ((maxBalance-statistics[j].balance)/maxBalance)*100 ;
			}

			maxDDLengthTemp = fabs(difftime(statistics[j].time, ddStartTime));
		}
		else //if(statistics[j].balance > maxBalance)
		{
		   maxBalance = statistics[j].balance;
		   maxDDDepthTemp = 0;
		   maxDDLengthTemp = 0;
		   ddStartTime = statistics[j].time;
		}


		if(maxDDDepthTemp > maxDDDepth){
			maxDDDepth = maxDDDepthTemp ;
		}

		if(maxDDLengthTemp > maxDDLength){
			maxDDLength = maxDDLengthTemp ;
		}
	}

	linearRegressionSlope = (sumBalanceTime) / timeSqrSum ;
	linearRegressionIntercept = statistics[0].balance;
    
	yPs = 0;
	yRs = 0;

	regressionResiduals = (double*)malloc(statisticsSize * sizeof(double));

	// determination coefficient calculation
	for(j=0; j < statisticsSize; j++){
		if(is_compounding_disabled == TRUE){
			regressionResiduals[j] = linearRegressionSlope*(statistics[j].time-statistics[0].time) - (statistics[j].balance-statistics[0].balance) ;
			yPs += pow((linearRegressionSlope*(statistics[j].time-statistics[0].time) - (statistics[j].balance-statistics[0].balance)), 2);
			yRs += pow(((statistics[j].balance-statistics[0].balance) - avgBalance), 2);
		} else {
			regressionResiduals[j] = linearRegressionSlope*(statistics[j].time-statistics[0].time) - (log(statistics[j].balance)- log(statistics[0].balance)) ;
			yPs += pow((linearRegressionSlope*(statistics[j].time-statistics[0].time) - (log(statistics[j].balance)- log(statistics[0].balance))), 2);
			yRs += pow(((log(statistics[j].balance)- log(statistics[0].balance)) - avgBalanceLog), 2);
		}
	}

	testResult->maxDDDepth = maxDDDepth;
	testResult->maxDDLength = maxDDLength;
    testResult->winning = totalWinningTrades/totalTrades*100;
    
	if (totalLose == 0) testResult->pf = 0;
	else testResult->pf = totalWin / totalLose;

	if ((yRs == 0) || (1-yPs/yRs) < 0) testResult->r2 = 0; 
	else testResult->r2 = (1-yPs/yRs);
	
	testResult->risk_reward= averageWinningTrade/averageLosingTrade;
	testResult->avgTradeDuration /= totalTrades;
    
    if (testResult->maxDDDepth > 100){
		testResult->maxDDDepth = 100;
	}
    
    regressionResidualsStandardDeviation = standardDeviation(regressionResiduals, statisticsSize);
    free(regressionResiduals); regressionResiduals = NULL;
    
}

void calculate_weekly_statistics(StatisticItem *statistics, 
                                 int statisticsSize, 
                                 double initialBalance,
                                 int is_compounding_disabled,
                                 int lastDate,
                                 TestResult *testResult){
   
/* Ulcer Index, Sharpe and martin ratio calculations */

	int n = 0;
	int j = 0;
	int lastTradeIndex = 0;
	double sumSqrt = 0;
	double cumWeekReturn = 0;
	double cumWeekReturnSquare = 0;
	double maxBalance = initialBalance;

	double meanWeekly;
	double sigmaWeekly;
	double weekReturn;
    
	
    int startDate = statistics[0].time;
	int analysisTime = statistics[0].time;
    double startWeekBalance = initialBalance;
	double lastWeekBalance = startWeekBalance;

	while (analysisTime < lastDate)
	{
		n++;
		analysisTime += 604800;

		while ((statistics[j].time < analysisTime) & (j < statisticsSize))
		{   
		   lastWeekBalance = statistics[j].balance;
		   lastTradeIndex = j;
		   j++;	   
		}
        
		if(is_compounding_disabled == TRUE){
			weekReturn = (lastWeekBalance-startWeekBalance)/initialBalance;
		} else {
			weekReturn = (lastWeekBalance-startWeekBalance)/startWeekBalance;
		}

		cumWeekReturn += weekReturn;
		cumWeekReturnSquare += weekReturn*weekReturn;
        startWeekBalance = statistics[j-1].balance;
		
		if (lastWeekBalance > maxBalance) {
		maxBalance = lastWeekBalance;
		}
		else {
		sumSqrt += pow((100 * ((lastWeekBalance / maxBalance) -1)), 2); 
		}

	}

	/* if above 100 make the UlcerIndex 100 (all strategies above 100 are useless) */
	if (sqrt(sumSqrt/n) > 100)
	testResult->ulcerIndex = 100; else
	testResult->ulcerIndex = sqrt(sumSqrt/n);

	meanWeekly = cumWeekReturn/n;
	sigmaWeekly = sqrt((n*cumWeekReturnSquare-cumWeekReturn*cumWeekReturn)/(n*(n-1)));

	testResult->sharpe = 7.2111103*(meanWeekly/sigmaWeekly);
	testResult->martin = testResult->cagr/testResult->ulcerIndex; 
    
}

TestResult __stdcall runPortfolioTest (
	int				testId,
	double**		pInSettings,
	char**			pInTradeSymbol,
    char*			pInAccountCurrency,
    char*			pInBrokerName,
	char*			pInRefBrokerName,
    double**			pInAccountInfo,
	TestSettings	*testSettings,
	CRatesInfo**	pRatesInfo,
	int				numCandles,
	int				numSystems,
    ASTRates***		pRates,
	double			minLotSize,
	void			(*testUpdate)(int testId, double percentageOfTestCompleted, COrderInfo lastOrder, double currentBalance, char* symbol), 
	void			(*testFinished)(TestResult testResults), 
	void			(*signalUpdate)(TradeSignal signal)
	)
{ 
	//Test variables
	int		j, n, m, s, openOrdersCount[2] = {0}, openOrdersCountSystem[2] = {0}, operation, tries;
	int		sourceIndex = 0, currentBrokerTime = 0, totalTrades = 0, numShorts = 0, numLongs = 0;
	int*     lastProcessedBar;
	struct	parameterInfo_t;
	double	percentageCompleted, swapLong, swapShort;
	int		result, lastDate;
	char	error_t[MAX_ERROR_LENGTH];
	double  bidAsk[BIDASK_ARRAY_SIZE];
	int    **numBarsRequired;
	int     maxNumbarsRequired;
    int     is_optimization = FALSE;
	BOOL    abortTest;
	CRates   ***rates;
	StrategyResults *strategyResults={0};
	COrderInfo openOrders[MAX_ORDERS]={0};
	COrderInfo systemOrders[MAX_ORDERS]={0};
	COrderInfo lastOrder;
	TestResult testResult = {0};
	int* i;
	int* testsFinished;
	int lastInterestAdditionTime = 0;
	//Statistics variables
    double conversionRate;
	double  finalBalance;
    double  previousBalance;
	double  initialBalance;
	StatisticItem *statistics;	
	int statisticsSize;
	int totalOrders = 0;
	int lastTradeIndex;
	int *numSignals;
	
	TradeSignal lastSignal = {0};
	
    char quoteSymbolTemp[MAX_FILE_PATH_CHARS] = "";
    char baseSymbolTemp[MAX_FILE_PATH_CHARS] = "";
	char buffer[MAX_FILE_PATH_CHARS] = "";
    char quoteConversionCurrency[MAX_FILE_PATH_CHARS] = "";
	int finishedCount;
	char **baseSymbols;
	char **quoteSymbols;
    double profit;

	// tick file data
	FILE** tickFiles;
	FILE** baseFiles;
	FILE** quoteFiles;
	
	char data[200] = "";
	char *ptr;
	char *timeString;
	struct tm lDate;
	char date[20] = "";
	time_t currentDateTime;
    char *strtokSave;

	int updateOrderType;

	int orderIndex;
	int index;

	if(testUpdate != NULL) is_optimization = FALSE; else is_optimization = TRUE;
    
    if(is_optimization == FALSE) initialize_me(testSettings[0].is_calculate_expectancy);

	// assign tick file array size
	tickFiles = (FILE**)malloc(numSystems * sizeof(FILE*));

	for (n=0; n<numSystems; n++){
		sprintf (buffer, "%s_TICK.csv", pInTradeSymbol[n]);
		pantheios_logprintf(PANTHEIOS_SEV_DEBUG, (PAN_CHAR_T*)"Searching for tick data: %s", buffer);
		tickFiles[n] = NULL;
		tickFiles[n] = fopen(buffer , "r+" );
	}

	
	//Variable initialization
	initialBalance =pInAccountInfo[0][IDX_BALANCE];
	finalBalance = pInAccountInfo[0][IDX_BALANCE];
	previousBalance  = finalBalance;
	statisticsSize = 0;
	testResult.avgTradeDuration = 0;
	statistics = (StatisticItem*)malloc(MIN_STATISTICS_SIZE * sizeof(StatisticItem));
	numBarsRequired = (int**)malloc(numSystems * sizeof(int*));
	i = (int*)malloc(numSystems * sizeof(int));
	testsFinished = (int*)malloc(numSystems * sizeof(int));
	lastProcessedBar = (int*)malloc(numSystems * sizeof(int));
	maxNumbarsRequired = 0;

	quoteFiles = (FILE**)malloc(numSystems * sizeof(FILE*));
	baseFiles = (FILE**)malloc(numSystems * sizeof(FILE*));
	quoteSymbols = (char**)malloc(numSystems * sizeof(char*));
	baseSymbols = (char**)malloc(numSystems * sizeof(char*));

	
	rates = (CRates***)malloc(sizeof(CRates**) * numSystems);

	for (n=0; n<numSystems; n++)
	rates[n] = (CRates**)malloc(sizeof(CRates*) * 10);
	

	if(signalUpdate != NULL) { 
		numSignals = (int*)malloc(numSystems * sizeof(int));
		globalSignalUpdate = signalUpdate;
	}

	//Get the historical data array for all systems and init framework
	for (j=0;j<numSystems;j++){

		numBarsRequired[j] = (int*)malloc(10 * sizeof(int));

		for (n=0;n<10;n++){
			numBarsRequired[j][n] = 0;
			if (pRatesInfo[j][n].totalBarsRequired != 0)
			numBarsRequired[j][n] = (int)(pRatesInfo[j][n].totalBarsRequired * 1.2 * pRatesInfo[j][n].requiredTimeframe/pRatesInfo[j][n].actualTimeframe);
		}

		pantheios_logprintf(PANTHEIOS_SEV_DEBUG, (PAN_CHAR_T*)"Bar requirements for all rates:");

		for (n=0;n<10;n++){
			pantheios_logprintf(PANTHEIOS_SEV_DEBUG, (PAN_CHAR_T*)"rates %d = %d bars", n, numBarsRequired[j][n]);
			pRatesInfo[j][n].ratesArraySize = numBarsRequired[j][n];
			if (numBarsRequired[j][n] > maxNumbarsRequired) maxNumbarsRequired = numBarsRequired[j][n];
		}

		pantheios_logprintf(PANTHEIOS_SEV_DEBUG, (PAN_CHAR_T*)"-- strategy settings --");
		for (n=0;n<64;n++){
			pantheios_logprintf(PANTHEIOS_SEV_DEBUG, (PAN_CHAR_T*)"Setting No.%d = %lf", n, pInSettings[j][n]);
		}

		pantheios_logprintf(PANTHEIOS_SEV_DEBUG, (PAN_CHAR_T*)"-- Pairs loaded --");
		for (n=0;n<numSystems;n++){
			pantheios_logprintf(PANTHEIOS_SEV_DEBUG, (PAN_CHAR_T*)"Symbol No.%d= %s", n, pInTradeSymbol[n]);
		}

	if(signalUpdate != NULL) numSignals[j] = 1;

	//Init the framework
	if (numSystems > 1)
		pInSettings[j][STRATEGY_INSTANCE_ID] = j+1;		

		result = WAIT_FOR_INIT;
		tries = 0;

		while(result == WAIT_FOR_INIT && tries <3){
			result = initInstanceC ((int)pInSettings[j][STRATEGY_INSTANCE_ID], 1, "./config/AsirikuyConfig.xml", "");
			sleepMilliseconds(500);
			tries++;
		}
		if(result!=SUCCESS){
			switch (result){
				case UNKNOWN_INSTANCE_ID:
					sprintf(error_t, "Wrong instance ID\n");
					break;
				case (INVALID_CONFIG):
					sprintf(error_t, "Wrong Asirikuy Framework config file\n");
					break;
				case (MISSING_CONFIG):
					sprintf(error_t, "Missing Asirikuy Framework config file\n");
					break;
				case UNKNOWN_TIMEZONE:
					sprintf(error_t, "Unknown timezone");
					break;
				default:
					sprintf(error_t, "Error %d initiating strategy\n", result);
					break;
			}
			pantheios_logprintf(PANTHEIOS_SEV_NOTICE, (PAN_CHAR_T*)"%s", error_t);
		}
	}

	// get base/quote symbols for all systems
    pantheios_logprintf(PANTHEIOS_SEV_DEBUG, (PAN_CHAR_T*)"Starting base/quote symbol extraction");
	
	for (n=0; n<numSystems; n++){

		pantheios_logprintf(PANTHEIOS_SEV_INFORMATIONAL, (PAN_CHAR_T*)"For system No.%d,pInSettings[n][ADDITIONAL_PARAM_8] = %lf",n,pInSettings[n][ADDITIONAL_PARAM_8]);

		baseSymbols[n] = (char*)malloc(MAX_FILE_PATH_CHARS * sizeof(char));
		quoteSymbols[n] = (char*)malloc(MAX_FILE_PATH_CHARS * sizeof(char));

		sprintf (baseSymbols[n], "%s", "");
		sprintf (quoteSymbols[n], "%s", "");
        sprintf (baseSymbolTemp, "%s", "");
		sprintf (quoteSymbolTemp, "%s", "");

		mql5_getConversionSymbols(pInTradeSymbol[n], pInAccountCurrency, baseSymbolTemp, quoteSymbolTemp);
          
        // cut the symbol names to remove any suffixes
        if (strlen(baseSymbolTemp) != 0){
            strncpy(baseSymbols[n], baseSymbolTemp, 6);
            baseSymbols[n][6] = '\0';
            pantheios_logprintf(PANTHEIOS_SEV_INFORMATIONAL, (PAN_CHAR_T*)"For system No.%d, base symbol is %s", n, baseSymbols[n]);
        }
        
        if (strlen(quoteSymbolTemp) != 0){
            strncpy(quoteSymbols[n], quoteSymbolTemp, 6);
            quoteSymbols[n][6] = '\0';
            pantheios_logprintf(PANTHEIOS_SEV_INFORMATIONAL, (PAN_CHAR_T*)"For system No.%d, quote symbol is %s", n, quoteSymbols[n]);
        }
        
		baseFiles[n] = NULL;
		quoteFiles[n] = NULL;

		if(strlen(baseSymbols[n]) != 0){
			pantheios_logprintf(PANTHEIOS_SEV_INFORMATIONAL, (PAN_CHAR_T*)"Opening base file");
			sprintf (buffer, "%s_QUOTES.csv", baseSymbols[n]);			
			baseFiles[n] = fopen(buffer , "r" );
			if (baseFiles[n] == NULL) {
    			pantheios_logprintf(PANTHEIOS_SEV_EMERGENCY, (PAN_CHAR_T*)"Error. Quotes for base symbol %s not found. Trading results will be inaccurate.", baseSymbols[n]);
			}
		}

		if(strlen(quoteSymbols[n]) != 0){
			pantheios_logprintf(PANTHEIOS_SEV_INFORMATIONAL, (PAN_CHAR_T*)"Opening quotes file");
			sprintf (buffer, "%s_QUOTES.csv", quoteSymbols[n]);		
			quoteFiles[n] = fopen(buffer , "r" );
			if (quoteFiles[n] == NULL) {
    			pantheios_logprintf(PANTHEIOS_SEV_EMERGENCY, (PAN_CHAR_T*)"Error. Quotes for quote symbol %s not found. Trading results will be inaccurate.",  quoteSymbols[n]);
			}
		}
	}

	pantheios_logprintf(PANTHEIOS_SEV_DEBUG, (PAN_CHAR_T*)"Starting main test loop. Max numbars required = %d, numCandles = %d", maxNumbarsRequired, numCandles);
	pantheios_logprintf(PANTHEIOS_SEV_DEBUG, (PAN_CHAR_T*)"Requested testing limits. StartDate = %d, EndDate = %d", testSettings[0].fromDate, testSettings[0].toDate);

	for(s = 0; s<numSystems; s++){
	rates[s][0] = (CRates*)malloc(sizeof(CRates) * numBarsRequired[s][0]);
	i[s] = maxNumbarsRequired - 1;
	lastProcessedBar[s] = 0;
	testsFinished[s] = 0;

		for (n = 1; n < 10; n++){
			if (numBarsRequired[s][n] > 0){
			rates[s][n] = (CRates*)malloc(sizeof(CRates) * numBarsRequired[s][n]);
			} else {
			rates[s][n] = (CRates*)malloc(sizeof(CRates) * 1);
			}
		}
	}
	
	finishedCount = 0;

	//Run the test for each candle
	while(finishedCount < numSystems){

		//run each bar for all systems
		for(s = 0; s<numSystems; s++)
		{

			// if this test is done then continue
			if (testsFinished[s] == 1) continue;

			if (i[s] >= numCandles-2){
				testsFinished[s] = 1;
				continue;
			}

		pantheios_logprintf(PANTHEIOS_SEV_DEBUG, (PAN_CHAR_T*)"bar = %d, finishedCount = %d, numSystem = %d", i[s], finishedCount, s);

		currentBrokerTime = 0;

		if(tickFiles[s] != NULL)
		{		

			while ( (int)currentBrokerTime < (int)pRates[s][0][i[s]].time){

			if (fgets(data, 200, tickFiles[s]) == NULL) break;

			timeString = strtok_r(data, ",", &strtokSave);
			ptr = strtok_r(NULL, ",", &strtokSave);
			sscanf(ptr,"%lf",&bidAsk[IDX_BID]);
			ptr = strtok_r(NULL, ",", &strtokSave);
			sscanf(ptr,"%lf",&bidAsk[IDX_ASK]);
            
			ptr = strtok_r(timeString, "-", &strtokSave);
			sscanf(ptr,"%d",&lDate.tm_mday);
			ptr = strtok_r(NULL, "-", &strtokSave);
			sscanf(ptr,"%d",&lDate.tm_mon);
			ptr = strtok_r(NULL, "-", &strtokSave);
			sscanf(ptr,"%d",&lDate.tm_year);
			ptr = strtok_r(NULL, "-", &strtokSave);
			sscanf(ptr,"%d",&lDate.tm_hour);
			ptr = strtok_r(NULL, "-", &strtokSave);
			sscanf(ptr,"%d",&lDate.tm_min);
			ptr = strtok_r(NULL, "-", &strtokSave);
			sscanf(ptr,"%d",&lDate.tm_sec);

			currentDateTime = mkgmtime(lDate.tm_year, lDate.tm_mon, lDate.tm_mday, lDate.tm_hour, lDate.tm_min, lDate.tm_sec);
			strftime(date, 20, "%d/%m/%Y %H:%M:%S", gmtime(&currentDateTime));
			
			currentBrokerTime = (int)currentDateTime;
			}

			if (i[s]<numCandles-1){
                if ((int)currentBrokerTime > (int)pRates[s][0][i[s]+1].time)
					i[s]++;
			}
		}

		lastSignal.testId = s;

		if(pRates[s][0][i[s]].time == -1){
			testsFinished[s] = 1;
			continue;
		}
		
		lastDate = pRates[s][0][i[s]].time; 

		if (lastDate > testSettings[s].toDate){
			lastDate = testSettings[s].toDate; 
		}

		abortTest = false;

		// reset strategy results

		strategyResults = (StrategyResults*)malloc(sizeof(StrategyResults) * (int)pInSettings[s][MAX_OPEN_ORDERS]);

		for(m = 0; m < (int)pInSettings[s][MAX_OPEN_ORDERS]; m++){
			memset( (double *) &strategyResults[m], 0, 10 * sizeof(double)); }

		//Update Variables
		if (currentBrokerTime == 0){
		bidAsk[IDX_BID] = pRates[s][0][i[s]].open;
		bidAsk[IDX_ASK] = pRates[s][0][i[s]].open + testSettings[s].spread;
		currentBrokerTime = pRates[s][0][i[s]].time;
		}

		// we should now get quote currency values if necessary

		if(quoteFiles[s] != NULL){	

			currentDateTime = 0;

			while ( (int)currentDateTime < (int)pRates[s][0][i[s]].time){

			if (fgets(data, 200, quoteFiles[s]) == NULL) break;

			timeString = strtok_r(data, ",", &strtokSave);

			ptr = strtok_r(NULL, ",", &strtokSave);
			sscanf(ptr,"%lf",&bidAsk[IDX_QUOTE_CONVERSION_BID]);
			bidAsk[IDX_QUOTE_CONVERSION_ASK] = bidAsk[IDX_QUOTE_CONVERSION_BID];

			ptr = strtok_r(timeString, "/", &strtokSave);
			sscanf(ptr,"%d",&lDate.tm_mday);
			ptr = strtok_r(NULL, "/", &strtokSave);
			sscanf(ptr,"%d",&lDate.tm_mon);
			ptr = strtok_r(NULL, " ", &strtokSave);
			sscanf(ptr,"%d",&lDate.tm_year);
			ptr = strtok_r(NULL, ":", &strtokSave);
			sscanf(ptr,"%d",&lDate.tm_hour);
			ptr = strtok_r(NULL, " ", &strtokSave);
			sscanf(ptr,"%d",&lDate.tm_min);

			lDate.tm_sec = 0;

			if(lDate.tm_year < 50) lDate.tm_year += 2000 ; else lDate.tm_year += 1900;

			currentDateTime = mkgmtime(lDate.tm_year, lDate.tm_mon, lDate.tm_mday, lDate.tm_hour, lDate.tm_min, lDate.tm_sec);
			strftime(date, 20, "%d/%m/%Y %H:%M:%S", gmtime(&currentDateTime));
			}
		} else {
            bidAsk[IDX_QUOTE_CONVERSION_ASK] = 1;  
            bidAsk[IDX_QUOTE_CONVERSION_BID] = 1; 
		}

		// we should now get base currency values if necessary

		if(baseFiles[s] != NULL){	

			currentDateTime = 0;

			while ( (int)currentDateTime < (int)pRates[s][0][i[s]].time){

			if (fgets(data, 200, baseFiles[s]) == NULL) break;

			timeString = strtok_r(data, ",", &strtokSave);

			ptr = strtok_r(NULL, ",", &strtokSave);
			sscanf(ptr,"%lf",&bidAsk[IDX_BASE_CONVERSION_BID]);
			bidAsk[IDX_BASE_CONVERSION_ASK] = bidAsk[IDX_BASE_CONVERSION_BID];

			ptr = strtok_r(timeString, "/", &strtokSave);
			sscanf(ptr,"%d",&lDate.tm_mday);
			ptr = strtok_r(NULL, "/", &strtokSave);
			sscanf(ptr,"%d",&lDate.tm_mon);
			ptr = strtok_r(NULL, " ", &strtokSave);
			sscanf(ptr,"%d",&lDate.tm_year);
			ptr = strtok_r(NULL, ":", &strtokSave);
			sscanf(ptr,"%d",&lDate.tm_hour);
			ptr = strtok_r(NULL, " ", &strtokSave);
			sscanf(ptr,"%d",&lDate.tm_min);

			lDate.tm_sec = 0;

			if(lDate.tm_year < 50) lDate.tm_year += 2000 ; else lDate.tm_year += 1900;

			currentDateTime = mkgmtime(lDate.tm_year, lDate.tm_mon, lDate.tm_mday, lDate.tm_hour, lDate.tm_min, lDate.tm_sec);
			strftime(date, 20, "%d/%m/%Y %H:%M:%S", gmtime(&currentDateTime));
			}
		} else {
            bidAsk[IDX_BASE_CONVERSION_ASK] = 1;  
            bidAsk[IDX_BASE_CONVERSION_BID] = 1; 
		}

		if (signalUpdate != NULL) lastSignal.time = currentBrokerTime;


		pInAccountInfo[s][IDX_BALANCE] = finalBalance;
		profit = 0;
		percentageCompleted = (double)(i[s]/numCandles)*100;
		//Fill rates arrays

		// only shift arrays if the bar is new
		if (i[s] != lastProcessedBar[s]){

		lastProcessedBar[s] = i[s];

		for(j=0; j<numBarsRequired[s][0]; j++){
			sourceIndex = i[s] - numBarsRequired[s][0] + j + 1;
			if(j==(numBarsRequired[s][0]-1)){   //Current Bar
				rates[s][0][j].high	 = pRates[s][0][sourceIndex].open;
				rates[s][0][j].low	 = pRates[s][0][sourceIndex].open;
				rates[s][0][j].close  = pRates[s][0][sourceIndex].open;
				rates[s][0][j].volume = 0;	
				swapLong  = pRates[s][0][sourceIndex].swapLong;
				swapShort = pRates[s][0][sourceIndex].swapShort;

			}
			else{		//Past Bars
				rates[s][0][j].high	 = pRates[s][0][sourceIndex].high;
				rates[s][0][j].low	 = pRates[s][0][sourceIndex].low;
				rates[s][0][j].close  = pRates[s][0][sourceIndex].close;
				rates[s][0][j].volume = pRates[s][0][sourceIndex].volume;
			}
			rates[s][0][j].time = pRates[s][0][sourceIndex].time;
			rates[s][0][j].open = pRates[s][0][sourceIndex].open;

			if (pRates[s][0][sourceIndex].time == -1) abortTest = true;
		}

		for (n = 1; n < 10; n++){
		if (numBarsRequired[s][n] > 0){
			for(j=0; j<numBarsRequired[s][n]; j++){
				sourceIndex = i[s] - numBarsRequired[s][n] + j + 1;
				if(j==(numBarsRequired[s][n]-1)){   //Current Bar
					rates[s][n][j].high   = pRates[s][n][sourceIndex].open;
					rates[s][n][j].low	   = pRates[s][n][sourceIndex].open;
					rates[s][n][j].close  = pRates[s][n][sourceIndex].open;
					rates[s][n][j].volume = 0;	
				}
				else{		//Past Bars
					rates[s][n][j].high   = pRates[s][n][sourceIndex].high;
					rates[s][n][j].low	   = pRates[s][n][sourceIndex].low;
					rates[s][n][j].close  = pRates[s][n][sourceIndex].close;
					rates[s][n][j].volume = pRates[s][n][sourceIndex].volume;
				}
				rates[s][n][j].time = pRates[s][n][sourceIndex].time;
				rates[s][n][j].open = pRates[s][n][sourceIndex].open;

				if (pRates[s][n][sourceIndex].time == -1) abortTest = true;
			}
		}
		}
		} else {

		// update data on new tick 
		if (bidAsk[IDX_BID] > rates[s][0][numBarsRequired[s][0]-1].high)
		rates[s][0][numBarsRequired[s][0]-1].high	= bidAsk[IDX_BID];

		if (bidAsk[IDX_BID] < rates[s][0][numBarsRequired[s][0]-1].low)
		rates[s][0][numBarsRequired[s][0]-1].low	= bidAsk[IDX_BID];

		rates[s][0][numBarsRequired[s][0]-1].close  = bidAsk[IDX_BID];
		rates[s][0][numBarsRequired[s][0]-1].volume += 1;	

		for (n = 1; n < 10; n++){
			if (numBarsRequired[s][n] > 0){
			if (bidAsk[IDX_BID] > rates[s][n][numBarsRequired[s][n]-1].high)
			rates[s][n][numBarsRequired[s][n]-1].high	= bidAsk[IDX_BID];

			if (bidAsk[IDX_BID] < rates[s][n][numBarsRequired[s][n]-1].low)
			rates[s][n][numBarsRequired[s][n]-1].low	= bidAsk[IDX_BID];

			rates[s][n][numBarsRequired[s][n]-1].close  = bidAsk[IDX_BID];
			rates[s][n][numBarsRequired[s][n]-1].volume += 1;	
			}
		}


		}

		if (abortTest){
			continue;
		}
        
        conversionRate = 1;
        strncpy(quoteSymbols[s], quoteConversionCurrency, 3);
        quoteSymbols[s][3] = '\0';
        
        if((strcmp(pInAccountCurrency, quoteConversionCurrency) == 0))
        {
            conversionRate = bidAsk[IDX_QUOTE_CONVERSION_BID];
        } else {
            conversionRate = 1/bidAsk[IDX_QUOTE_CONVERSION_ASK];
        }

		pInAccountInfo[s][IDX_EQUITY] = calculateAccountEquity(pInAccountInfo[s], openOrdersCount, openOrders, conversionRate);
        
		for(m=0; m<openOrdersCount[BUY] + openOrdersCount[SELL]; m++){
			checkPending(bidAsk[IDX_BID], bidAsk[IDX_ASK], m, openOrders, (int)pInSettings[s][STRATEGY_INSTANCE_ID], currentBrokerTime, numCandles, i[s]-1, pRates[s][0], testUpdate != NULL, testSettings[0].is_calculate_expectancy);
			if(checkTPSL(bidAsk[IDX_BID], bidAsk[IDX_ASK], m, numBarsRequired[s][0]-2, rates[s][0], openOrdersCount, openOrders, (int)pInSettings[s][STRATEGY_INSTANCE_ID], currentBrokerTime, &lastOrder, pInTradeSymbol[s], &profit, pInAccountInfo[s][IDX_CONTRACT_SIZE],lastSignal, numSignals, finalBalance, conversionRate, &testResult.avgTradeDuration)){
					finalBalance += profit;
					if(is_optimization == FALSE){
						testUpdate(s, percentageCompleted, lastOrder, finalBalance, pInTradeSymbol[s]);
                    }
                    addStatisticValue(&statistics, &statisticsSize, profit, finalBalance, currentBrokerTime);
				}
		}

		lastInterestAdditionTime = addInterest(openOrdersCount, openOrders, (int)pInSettings[s][STRATEGY_INSTANCE_ID], (int)currentBrokerTime, (int)pInAccountInfo[s][IDX_CONTRACT_SIZE], swapLong, swapShort, bidAsk, lastInterestAdditionTime);

		for (m=0;m<MAX_ORDERS;m++){
			systemOrders[m].instanceId = 0;
			systemOrders[m].isOpen     = 0;
			systemOrders[m].swap       = 0;
			systemOrders[m].profit     = 0;
			}

		openOrdersCountSystem[BUY] = 0;
		openOrdersCountSystem[SELL] = 0;

		n = 0;
		m = 0;
		while (m < MAX_ORDERS ){
			if ((openOrders[m].instanceId == pInSettings[s][STRATEGY_INSTANCE_ID])){
			systemOrders[n] = openOrders[m] ;
			if ((openOrders[m].type ==BUY || openOrders[m].type ==BUYLIMIT || openOrders[m].type ==BUYSTOP) && (openOrders[m].isOpen == 1)) openOrdersCountSystem[BUY]++;
			if ((openOrders[m].type ==SELL || openOrders[m].type ==SELLLIMIT || openOrders[m].type == SELLSTOP) && (openOrders[m].isOpen == 1)) openOrdersCountSystem[SELL]++;
			n++;	
			}

			m++;
		}

		result = SUCCESS;


		//Run Strategy
		if((currentBrokerTime > testSettings[s].fromDate) && (currentBrokerTime < testSettings[s].toDate)){
		result = c_runStrategy(pInSettings[s], pInTradeSymbol[s], pInAccountCurrency, pInBrokerName, pInRefBrokerName, &currentBrokerTime, openOrdersCountSystem, systemOrders,
								pInAccountInfo[s], bidAsk, pRatesInfo[s], rates[s][0], rates[s][1], rates[s][2], rates[s][3], rates[s][4], rates[s][5], rates[s][6], rates[s][7], rates[s][8], rates[s][9], (double *)strategyResults);
		}

		if(result!=SUCCESS){
			switch (result){
				case NULL_POINTER:
					pantheios_logprintf(PANTHEIOS_SEV_ERROR, (PAN_CHAR_T*)"Null Pointer. Check the Asirikuy Framework log for more detail");
					break;
				case NOT_ENOUGH_RATES_DATA:
					pantheios_logprintf(PANTHEIOS_SEV_ERROR, (PAN_CHAR_T*)"Not enough rates. Check the Asirikuy Framework log for more detail");
					break;
				default:
					pantheios_logprintf(PANTHEIOS_SEV_ERROR, (PAN_CHAR_T*)"Error %d running strategy\n", result);
					break;
			}
		}
		else {
			for(m = 0; m < (int)pInSettings[s][MAX_OPEN_ORDERS]; m++){

			if(strategyResults[m].tradingSignals > 0){	
				operation = (int)strategyResults[m].tradingSignals;
				//BUY
				if((operation & SIGNAL_OPEN_BUY) != 0 || (operation & SIGNAL_OPEN_BUYSTOP) != 0 || (operation & SIGNAL_OPEN_BUYLIMIT) != 0)
				{
					if ((operation & SIGNAL_OPEN_BUY) != 0) updateOrderType = BUY;
					if ((operation & SIGNAL_OPEN_BUYLIMIT) != 0) updateOrderType = BUYLIMIT;
					if ((operation & SIGNAL_OPEN_BUYSTOP) != 0) updateOrderType = BUYSTOP;

					pantheios_logprintf(PANTHEIOS_SEV_DEBUG, (PAN_CHAR_T*)"BUY signal type %d. Instance ID = %d", (int)updateOrderType, (int)pInSettings[s][STRATEGY_INSTANCE_ID]);
					openOrder(&strategyResults[m], openOrdersCount, openOrders, (int)pInSettings[s][STRATEGY_INSTANCE_ID], &totalTrades, currentBrokerTime, bidAsk[IDX_ASK], (int)updateOrderType,lastSignal, numSignals, finalBalance, minLotSize, pInAccountInfo[s][IDX_MINIMUM_STOP]);
					pantheios_logprintf(PANTHEIOS_SEV_DEBUG, (PAN_CHAR_T*)"Open Order. ticket = %lf, instanceID = %lf, Entry = %lf, SL = %lf, TP =%lf", openOrders[openOrdersCount[BUY]+openOrdersCount[SELL]-1].ticket, openOrders[openOrdersCount[BUY]+openOrdersCount[SELL]-1].instanceId, openOrders[openOrdersCount[BUY]+openOrdersCount[SELL]-1].openPrice, openOrders[openOrdersCount[BUY]+openOrdersCount[SELL]-1].stopLoss, openOrders[openOrdersCount[BUY]+openOrdersCount[SELL]-1].takeProfit);
					numLongs++;


					if(is_optimization == FALSE && updateOrderType == BUY) calculate_mathematical_expectancy(BUY, numCandles, i[s], bidAsk[IDX_ASK], pRates[s][0], fabs(bidAsk[IDX_ASK]-bidAsk[IDX_BID]), testSettings[0].is_calculate_expectancy) ;

				}

				if((operation & SIGNAL_CLOSE_BUY) != 0 || (operation & SIGNAL_CLOSE_BUYLIMIT) != 0 || (operation & SIGNAL_CLOSE_BUYSTOP) != 0)
				{

					if ((operation & SIGNAL_CLOSE_BUY) != 0) updateOrderType = BUY;
					if ((operation & SIGNAL_CLOSE_BUYLIMIT) != 0) updateOrderType = BUYLIMIT;
					if ((operation & SIGNAL_CLOSE_BUYSTOP) != 0) updateOrderType = BUYSTOP;

					pantheios_logprintf(PANTHEIOS_SEV_DEBUG, (PAN_CHAR_T*)"Close BUY Signal. Instance ID = %d", (int)pInSettings[s][STRATEGY_INSTANCE_ID]);
					if (closeOrder(&strategyResults[m], openOrdersCount, openOrders, (int)pInSettings[s][STRATEGY_INSTANCE_ID], currentBrokerTime, bidAsk[IDX_BID], &lastOrder, pInTradeSymbol[s], (int)updateOrderType, &profit, pInAccountInfo[s][IDX_CONTRACT_SIZE], globalSignalUpdate,lastSignal, numSignals, finalBalance, conversionRate, &testResult.avgTradeDuration)){
						finalBalance += profit;
						if(is_optimization == FALSE){
                            testUpdate(s, percentageCompleted, lastOrder, finalBalance, pInTradeSymbol[s]);
                        }
						addStatisticValue(&statistics, &statisticsSize, profit, finalBalance, currentBrokerTime);
					}

					//totalTrades,numShorts,numLongs should be counted on real open orders, excclude those stop and limit orders.
					if (updateOrderType == BUYLIMIT || updateOrderType == BUYSTOP)
					{
						numLongs--;
						totalTrades--;
					}
					
				}
				if((operation & SIGNAL_UPDATE_BUY) != 0 || (operation & SIGNAL_UPDATE_BUYLIMIT) != 0 || (operation & SIGNAL_UPDATE_BUYSTOP) != 0)
				{
					pantheios_logprintf(PANTHEIOS_SEV_DEBUG, (PAN_CHAR_T*)"Update BUY Signal. Instance ID = %d", (int)pInSettings[s][STRATEGY_INSTANCE_ID]);
					
					if ((operation & SIGNAL_UPDATE_BUY) != 0) updateOrderType = BUY;
					if ((operation & SIGNAL_UPDATE_BUYLIMIT) != 0) updateOrderType = BUYLIMIT;
					if ((operation & SIGNAL_UPDATE_BUYSTOP) != 0) updateOrderType = BUYSTOP;
					
					updateOrder((int)pInSettings[s][STRATEGY_INSTANCE_ID], &strategyResults[m], openOrdersCount, openOrders, bidAsk, (int)updateOrderType,lastSignal, numSignals, finalBalance, pInAccountInfo[s][IDX_MINIMUM_STOP]);
				}

				//SELL
				if((operation & SIGNAL_OPEN_SELL) != 0 || (operation & SIGNAL_OPEN_SELLSTOP) != 0 || (operation & SIGNAL_OPEN_SELLLIMIT) != 0)
				{
					if ((operation & SIGNAL_OPEN_SELL) != 0) updateOrderType = SELL;
					if ((operation & SIGNAL_OPEN_SELLLIMIT) != 0) updateOrderType = SELLLIMIT;
					if ((operation & SIGNAL_OPEN_SELLSTOP) != 0) updateOrderType = SELLSTOP;

					pantheios_logprintf(PANTHEIOS_SEV_DEBUG, (PAN_CHAR_T*)"SELL signal type %d. Instance ID = %d", (int)updateOrderType, (int)pInSettings[s][STRATEGY_INSTANCE_ID]);
					openOrder(&strategyResults[m], openOrdersCount, openOrders, (int)pInSettings[s][STRATEGY_INSTANCE_ID], &totalTrades, currentBrokerTime, bidAsk[IDX_BID], (int)updateOrderType,lastSignal, numSignals, finalBalance, minLotSize, pInAccountInfo[s][IDX_MINIMUM_STOP]);
					pantheios_logprintf(PANTHEIOS_SEV_DEBUG, (PAN_CHAR_T*)"Open Order. ticket = %lf, instanceID = %lf, Entry = %lf, SL = %lf, TP =%lf", openOrders[openOrdersCount[BUY]+openOrdersCount[SELL]-1].ticket, openOrders[openOrdersCount[BUY]+openOrdersCount[SELL]-1].instanceId, openOrders[openOrdersCount[BUY]+openOrdersCount[SELL]-1].openPrice, openOrders[openOrdersCount[BUY]+openOrdersCount[SELL]-1].stopLoss, openOrders[openOrdersCount[BUY]+openOrdersCount[SELL]-1].takeProfit);
					numShorts++;

					if(is_optimization == FALSE && updateOrderType == SELL) calculate_mathematical_expectancy(SELL, numCandles, i[s], bidAsk[IDX_BID], pRates[s][0], fabs(bidAsk[IDX_ASK]-bidAsk[IDX_BID]), testSettings[0].is_calculate_expectancy) ;
				}
				if((operation & SIGNAL_CLOSE_SELL) != 0 || (operation & SIGNAL_CLOSE_SELLSTOP) != 0 || (operation & SIGNAL_CLOSE_SELLLIMIT) != 0)
				{

					if ((operation & SIGNAL_CLOSE_SELL) != 0) updateOrderType = SELL;
					if ((operation & SIGNAL_CLOSE_SELLLIMIT) != 0) updateOrderType = SELLLIMIT;
					if ((operation & SIGNAL_CLOSE_SELLSTOP) != 0) updateOrderType = SELLSTOP;

					pantheios_logprintf(PANTHEIOS_SEV_DEBUG, (PAN_CHAR_T*)"Close SELL Signal. Instance ID = %d", (int)pInSettings[s][STRATEGY_INSTANCE_ID]);
					if (closeOrder(&strategyResults[m], openOrdersCount, openOrders, (int)pInSettings[s][STRATEGY_INSTANCE_ID], currentBrokerTime, bidAsk[IDX_ASK], &lastOrder, pInTradeSymbol[s], (int)updateOrderType, &profit, pInAccountInfo[s][IDX_CONTRACT_SIZE], globalSignalUpdate,lastSignal, numSignals, finalBalance, conversionRate, &testResult.avgTradeDuration)){
						finalBalance += profit;
						if(is_optimization == FALSE){
                            testUpdate(s, percentageCompleted, lastOrder, finalBalance, pInTradeSymbol[s]);						
                        }
						addStatisticValue(&statistics, &statisticsSize, profit, finalBalance, currentBrokerTime);

						//totalTrades,numShorts,numLongs should be counted on real open orders, excclude those stop and limit orders.
						if (updateOrderType == SELLLIMIT || updateOrderType == SELLSTOP)
						{
							totalTrades--;
							numShorts--;
						}
					}
				}
				if((operation & SIGNAL_UPDATE_SELL) != 0 || (operation & SIGNAL_UPDATE_SELLLIMIT) != 0 || (operation & SIGNAL_UPDATE_SELLSTOP) != 0)
				{
					pantheios_logprintf(PANTHEIOS_SEV_DEBUG, (PAN_CHAR_T*)"Update SELL Signal. Instance ID = %d", (int)pInSettings[s][STRATEGY_INSTANCE_ID]);

					if ((operation & SIGNAL_UPDATE_SELL) != 0) updateOrderType = SELL;
					if ((operation & SIGNAL_UPDATE_SELLLIMIT) != 0) updateOrderType = SELLLIMIT;
					if ((operation & SIGNAL_UPDATE_SELLSTOP) != 0) updateOrderType = SELLSTOP;

					updateOrder((int)pInSettings[s][STRATEGY_INSTANCE_ID], &strategyResults[m], openOrdersCount, openOrders, bidAsk, (int)updateOrderType, lastSignal, numSignals, finalBalance, pInAccountInfo[s][IDX_MINIMUM_STOP]);
				}
			}
			} //If trading signals
			
		} //else of result!=SUCCESS

		previousBalance = finalBalance;

		free(strategyResults); strategyResults = NULL;

		if(tickFiles[s] == NULL)
			i[s]++;

		}	

		finishedCount = 0;

		for(s = 0; s<numSystems; s++){
			finishedCount += testsFinished[s];
		}

		if (finalBalance <= 0){
			finalBalance = 0;
			break;
		}
	}

	
	//save_openorder_to_file(pInTradeSymbol[0], openOrders, openOrdersCount);

	orderIndex = openOrdersCount[BUY] + openOrdersCount[SELL];
	if (orderIndex > 0)
		save_openorder_to_file();

	//For all the open orders
	for (index = 0; index<orderIndex; index++){
		if (openOrders[index].isOpen){
			openOrders[index].closeTime = currentBrokerTime;
			openOrders[index].closePrice = 0;
			testUpdate(0, percentageCompleted, openOrders[index], finalBalance, pInTradeSymbol[0]);
		}
	}


	//reset initial balance
	pInAccountInfo[0][IDX_BALANCE] = initialBalance;
	pInAccountInfo[0][IDX_EQUITY] = initialBalance; 
    
	

    testResult.totalTrades = totalTrades;
	testResult.finalBalance = finalBalance;
	testResult.numShorts = numShorts;
	testResult.numLongs = numLongs;
    testResult.yearsTraded = fabs(difftime(statistics[0].time, lastDate)/(3600*24*365));
	testResult.cagr = 100*(pow(finalBalance/initialBalance, 1/testResult.yearsTraded)-1);
	

    calculate_trade_by_trade_statistics(statistics, 
                                        statisticsSize, 
                                        initialBalance, 
                                        pInSettings[0][DISABLE_COMPOUNDING],
                                        totalTrades,
                                        &testResult, 
                                        numSystems,
                                        is_optimization);
	
    calculate_weekly_statistics(statistics, 
                                statisticsSize, 
                                initialBalance,
                                pInSettings[0][DISABLE_COMPOUNDING],
                                lastDate,
                                &testResult);
    
    for(s=0;s<numSystems;s++){
		strcat (testResult.symbol, pInTradeSymbol[s]);
		if (s != numSystems-1) strcat (testResult.symbol, ",");
	}

	testResult.testId = s;
    
    if ((pInSettings[0][DISABLE_COMPOUNDING] == FALSE) && (is_optimization == FALSE)){
    save_statistics_to_file(testResult, finalBalance, initialBalance);
        }
    
    if(testFinished!=NULL) {
		testFinished(testResult); // callback finish test function
	}

	for(s = 0; s<numSystems; s++){

		for (n = 0; n < 10; n++){
			free(rates[s][n]); rates[s][n] = NULL;
		}
		free(rates[s]); rates[s] = NULL;

		if (tickFiles[s] != NULL)
		fclose(tickFiles[s]);

		if (quoteFiles[s] != NULL)
		fclose(quoteFiles[s]);

		if (baseFiles[s] != NULL)
		fclose(baseFiles[s]);

		free(baseSymbols[s]); baseSymbols[s] = NULL;
		free(quoteSymbols[s]); quoteSymbols[s] = NULL;
	}

	free(i); i = NULL;
	free(testsFinished); testsFinished = NULL;
	free(tickFiles); tickFiles = NULL;
	free(quoteFiles); quoteFiles = NULL;
	free(baseFiles); baseFiles = NULL;
	free(baseSymbols); baseSymbols = NULL;
	free(quoteSymbols); quoteSymbols = NULL;
	free(rates); rates = NULL;
	free(lastProcessedBar); lastProcessedBar = NULL;
    
	free(statistics); statistics = NULL;
	

	for(s=0;s<numSystems;s++){ 
		free(numBarsRequired[s]);
		numBarsRequired[s] = NULL;

	}
	
	free(numBarsRequired); numBarsRequired = NULL;

	if(signalUpdate != NULL) { 
	free(numSignals); numSignals = NULL;
	}

    return testResult;
}
