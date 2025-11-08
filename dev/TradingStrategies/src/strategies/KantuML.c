/**
 * @file
 * @brief     The KantuML machine learning trading system.
 * @details   KantuML is a machine learning strategy that uses ensemble predictions from multiple ML algorithms (Linear Regression, Neural Network, K-Nearest Neighbor) to generate trading signals.
 * 
 * @author    Daniel Fernandez (Original idea, initial implementation, and optimization)
 * @version   F4.x.x
 * @date      2015
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
#include <stdlib.h>

#include "AsirikuyMachineLearningCWrapper.hpp"
#include "EasyTradeCWrapper.hpp"
#include "OrderSignals.h"
#include "OrderManagement.h"
#include "AsirikuyTime.h"
#include "StrategyUserInterface.h"
#include "KantuStrategies.h"

typedef enum additionalSettingsML_t
{
    SELECT_KANTU_SYSTEM_FILE_ML = ADDITIONAL_PARAM_1,
	LEARNING_PERIOD_ML          = ADDITIONAL_PARAM_2,
	BARS_USED_ML                = ADDITIONAL_PARAM_3,
	TRADING_HOUR_ML             = ADDITIONAL_PARAM_4,
	DSL_BREAKEVEN_BARS_ML       = ADDITIONAL_PARAM_5,
	FRONTIER_ML			        = ADDITIONAL_PARAM_6,
	DSL_TYPE_ML			        = ADDITIONAL_PARAM_7,
	ML_ALGO_TYPE_ML		        = ADDITIONAL_PARAM_8,
    TL_ATR_MULTIPLIER_ML        = ADDITIONAL_PARAM_9,
    USE_AFMTL_ML                = ADDITIONAL_PARAM_10,
    TIMED_EXIT_BARS_ML          = ADDITIONAL_PARAM_11,
    DAY_FILTER_ML               = ADDITIONAL_PARAM_12
} AdditionalSettingsML;

typedef enum dslTypes_t
{
  DSL_NONE = 0,
  DSL_LINEAR = 1,
  DSL_LOG = 2,
  DSL_PARABOLIC = 3,
  DSL_SQUARE = 4,
  DSL_SQUARE_WITH_SKIP = 5,
  DSL_X4 = 6,
  DSL_X025 = 7
} dslTypes;

typedef enum mlalgoTypes_t
{
  LINEAR_REGRESSION = 0,
  NEURAL_NETWORK	= 1,
  K_NEAREST_NEIGHBOR = 2
} mlalgoTypes;

AsirikuyReturnCode runKantuML(StrategyParams* pParams)
{
  AsirikuyReturnCode returnCode = SUCCESS;
  FILE* f=NULL;
  uint8_t ensemblePrediction = 0;
  char filenameString[1024] = "";
  double stopLoss, takeProfit, trailingStop, atr;
  double currentStopLoss, currentTakeProfit;
  double spread;
  double minStop;
  double newSL;
  int orderAge;
  int i=0;
  int ml_algo_type;
  int is_file_closed=1;
  int writeFile = FALSE;
  char timeString[MAX_TIME_STRING_SIZE] = "";
  double userInterfaceValues[10];
  char   *userInterfaceVariableNames[10] = {
    "ATR",
    "HourToTrade",
	"CurrentHour",
    "initial SL",
    "SL_Type",
    "BE",
    " ",
    " ",
    " ",
    " "};
    
  if ( parameter(SELECT_KANTU_SYSTEM_FILE_ML) != 0){
      ensemblePrediction = getKantuSignalML(pParams, (int) parameter(SELECT_KANTU_SYSTEM_FILE_ML));
  } else {
      writeFile = TRUE;
  }
  
  if (strstr(pParams->tradeSymbol, "JPY") != NULL){
	minStop = 0.15;
  } else {
	minStop = 0.0015;
  }

  atr = iAtrWholeDaysSimple(PRIMARY_RATES, (int)pParams->settings[ATR_AVERAGING_PERIOD]);
	
  stopLoss     = atr * parameter(SL_ATR_MULTIPLIER);
  takeProfit   = atr * pParams->settings[TP_ATR_MULTIPLIER];
  trailingStop = atr * pParams->settings[TL_ATR_MULTIPLIER_ML];

    userInterfaceValues[0] = atr;
	userInterfaceValues[1] = parameter(TRADING_HOUR_ML);
	userInterfaceValues[2] = hour();
	userInterfaceValues[3] = stopLoss;
	userInterfaceValues[4] = parameter(DSL_TYPE_ML);
    userInterfaceValues[5] = parameter(DSL_BREAKEVEN_BARS_ML);
	userInterfaceValues[6] = 0;
	userInterfaceValues[7] = 0;
	userInterfaceValues[8] = 0;
	userInterfaceValues[9] = 0;

	saveUserInterfaceValues(userInterfaceVariableNames, userInterfaceValues, TOTAL_UI_VALUES, (int)pParams->settings[STRATEGY_INSTANCE_ID], (BOOL)pParams->settings[IS_BACKTESTING]);

 // if we are generating binary files then just save predictions and exit
  if(writeFile == TRUE){

	ml_algo_type = (int)parameter(ML_ALGO_TYPE_ML);

	if (ml_algo_type == LINEAR_REGRESSION){
	ensemblePrediction = LR_Prediction_i_simpleReturn_o_mlemse((int)parameter(LEARNING_PERIOD_ML), (int)parameter(BARS_USED_ML), (int)parameter(FRONTIER_ML));
	}

	if (ml_algo_type == NEURAL_NETWORK){
	ensemblePrediction = NN_Prediction_i_simpleReturn_o_mlemse((int)parameter(LEARNING_PERIOD_ML), (int)parameter(BARS_USED_ML), (int)parameter(FRONTIER_ML));
    }

	if (ml_algo_type == K_NEAREST_NEIGHBOR){
	ensemblePrediction = KNN_Prediction_i_simpleReturn_o_mlemse((int)parameter(LEARNING_PERIOD_ML), (int)parameter(BARS_USED_ML), (int)parameter(FRONTIER_ML));
    }
    
    if(writeFile == TRUE){
        sprintf(filenameString, "%s%03d%03d%03d%03d.bin", pParams->tradeSymbol, (int)parameter(LEARNING_PERIOD_ML), (int)parameter(BARS_USED_ML), (int)parameter(FRONTIER_ML), (int)parameter(ML_ALGO_TYPE_ML));
        
		f = NULL;

		while (f == NULL){
		f=fopen(filenameString,"ab");
		}

        fwrite(&ensemblePrediction,sizeof(ensemblePrediction),1,f);

		while(is_file_closed != 0){
        is_file_closed = fclose(f);
		}

        return(SUCCESS);
    }

	}

  safe_timeString(timeString, openTime(0));
  pantheios_logprintf(PANTHEIOS_SEV_INFORMATIONAL, (PAN_CHAR_T*)"pKantuML: InstanceID = %d, BarTime = %s, ATR = %lf ensemblePrediction = %d", (int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, atr, ensemblePrediction);
  

  //if not generating files then proceed to trade according to prediction
  if(ensemblePrediction == 1)
	{
    return openOrUpdateLongEasy(takeProfit, stopLoss,1);
  }
	else if(ensemblePrediction == 2)
	{
    return openOrUpdateShortEasy(takeProfit, stopLoss,1);
  }

  if(totalOrdersCount()==0) return SUCCESS;

  currentStopLoss = pParams->orderInfo[0].stopLoss ;
  currentTakeProfit = pParams->orderInfo[0].takeProfit ;
  orderAge = getOrderAge(pParams, 0);

  if (trailingStop != 0){

	if ((cOpen(orderAge)-pParams->bidAsk.ask[0] > trailingStop) && (totalOpenOrders(pParams, SELL) > 0)){

		if(currentTakeProfit != 0)
		modifyTradeEasy(SELL , -1, trailingStop, fabs(currentTakeProfit-pParams->bidAsk.ask[0])); else   
		modifyTradeEasy(SELL , -1, trailingStop, 0);
	}

	if ((pParams->bidAsk.bid[0]-cOpen(orderAge) > trailingStop) && (totalOpenOrders(pParams, BUY) > 0) ){

		if(currentTakeProfit != 0)
		modifyTradeEasy(BUY , -1, trailingStop, fabs(currentTakeProfit-pParams->bidAsk.bid[0])); else 
	    modifyTradeEasy(BUY , -1, trailingStop, 0);
	}

  }

  if (parameter(USE_AFMTL_ML) == TRUE){

	if ((pParams->bidAsk.ask[0]+stopLoss < pParams->orderInfo[0].stopLoss) &&
		(totalOpenOrders(pParams, SELL) > 0)){

		if(currentTakeProfit != 0)
		modifyTradeEasy(SELL , -1, stopLoss, fabs(currentTakeProfit-pParams->bidAsk.ask[0])); else   
		modifyTradeEasy(SELL , -1, stopLoss, 0);
	}

	if ((pParams->bidAsk.bid[0]-stopLoss > pParams->orderInfo[0].stopLoss) &&
		(totalOpenOrders(pParams, BUY) > 0) ){

		if(currentTakeProfit != 0)
		modifyTradeEasy(BUY , -1, stopLoss, fabs(currentTakeProfit-pParams->bidAsk.bid[0])); else 
	    modifyTradeEasy(BUY , -1, stopLoss, 0);
	}

  }

  spread = fabs(pParams->bidAsk.ask[0]-pParams->bidAsk.bid[0]);

  if (parameter(DSL_TYPE_ML) == DSL_LINEAR){

	if ((pParams->bidAsk.ask[0]+minStop < cOpen(orderAge)+ spread + (orderAge*(-stopLoss/parameter(DSL_BREAKEVEN_BARS_ML))+stopLoss)) &&
		(totalOpenOrders(pParams, SELL) > 0)){

		if(currentTakeProfit != 0)
		modifyTradeEasy(SELL , -1, cOpen(orderAge)+spread-pParams->bidAsk.ask[0] + (orderAge*(-stopLoss/parameter(DSL_BREAKEVEN_BARS_ML))+stopLoss), fabs(currentTakeProfit-pParams->bidAsk.ask[0])); else   
		modifyTradeEasy(SELL , -1, cOpen(orderAge)+spread-pParams->bidAsk.ask[0] + (orderAge*(-stopLoss/parameter(DSL_BREAKEVEN_BARS_ML))+stopLoss), 0);
	}

	if ((pParams->bidAsk.bid[0]-minStop > cOpen(orderAge) - (orderAge*(-stopLoss/parameter(DSL_BREAKEVEN_BARS_ML))+stopLoss)) &&
		(totalOpenOrders(pParams, BUY) > 0)){

		if(currentTakeProfit != 0)
		modifyTradeEasy(BUY , -1, pParams->bidAsk.bid[0]-cOpen(orderAge) + (orderAge*(-stopLoss/parameter(DSL_BREAKEVEN_BARS_ML))+stopLoss), fabs(currentTakeProfit-pParams->bidAsk.bid[0])); else 
	    modifyTradeEasy(BUY , -1, pParams->bidAsk.bid[0]-cOpen(orderAge) + (orderAge*(-stopLoss/parameter(DSL_BREAKEVEN_BARS_ML))+stopLoss), 0);
	}

  }

  if (parameter(DSL_TYPE_ML ) == DSL_LOG){

	newSL = stopLoss+(stopLoss/(-log(parameter(DSL_BREAKEVEN_BARS_ML))))*log((double)orderAge+1);
  
	if ((pParams->bidAsk.ask[0]+minStop < cOpen(orderAge)+ spread + newSL) &&
		(totalOpenOrders(pParams, SELL) > 0)){

		if(currentTakeProfit != 0)
		modifyTradeEasy(SELL , -1, cOpen(orderAge)+spread-pParams->bidAsk.ask[0] + newSL, fabs(currentTakeProfit-pParams->bidAsk.ask[0])); else   
		modifyTradeEasy(SELL , -1, cOpen(orderAge)+spread-pParams->bidAsk.ask[0] + newSL, 0);
	}

	if ((pParams->bidAsk.bid[0]-minStop > cOpen(orderAge) - newSL) &&
		(totalOpenOrders(pParams, BUY) > 0)){

		if(currentTakeProfit != 0)
		modifyTradeEasy(BUY , -1, pParams->bidAsk.bid[0]-cOpen(orderAge) + newSL, fabs(currentTakeProfit-pParams->bidAsk.bid[0])); else 
	    modifyTradeEasy(BUY , -1, pParams->bidAsk.bid[0]-cOpen(orderAge) + newSL, 0);
	}

  }

  if (parameter(DSL_TYPE_ML ) == DSL_PARABOLIC){

	newSL = stopLoss+(-stopLoss/(parameter(DSL_BREAKEVEN_BARS_ML)*parameter(DSL_BREAKEVEN_BARS_ML)))*((double)orderAge)*((double)orderAge);
  
	if ((pParams->bidAsk.ask[0]+minStop < cOpen(orderAge)+ spread + newSL) &&
		(totalOpenOrders(pParams, SELL) > 0)){

		if(currentTakeProfit != 0)
		modifyTradeEasy(SELL , -1, cOpen(orderAge)+spread-pParams->bidAsk.ask[0] + newSL, fabs(currentTakeProfit-pParams->bidAsk.ask[0])); else   
		modifyTradeEasy(SELL , -1, cOpen(orderAge)+spread-pParams->bidAsk.ask[0] + newSL, 0);
	}

	if ((pParams->bidAsk.bid[0]-minStop > cOpen(orderAge) - newSL) &&
		(totalOpenOrders(pParams, BUY) > 0)){

		if(currentTakeProfit != 0)
		modifyTradeEasy(BUY , -1, pParams->bidAsk.bid[0]-cOpen(orderAge) + newSL, fabs(currentTakeProfit-pParams->bidAsk.bid[0])); else 
	    modifyTradeEasy(BUY , -1, pParams->bidAsk.bid[0]-cOpen(orderAge) + newSL, 0);
	}

  }

  if (parameter(DSL_TYPE_ML ) == DSL_SQUARE){

	newSL = stopLoss+(-stopLoss/(sqrt(parameter(DSL_BREAKEVEN_BARS_ML))))*sqrt((double)orderAge);
  
	if ((pParams->bidAsk.ask[0]+minStop < cOpen(orderAge)+ spread + newSL) &&
		(totalOpenOrders(pParams, SELL) > 0)){

		if(currentTakeProfit != 0)
		modifyTradeEasy(SELL , -1, cOpen(orderAge)+spread-pParams->bidAsk.ask[0] + newSL, fabs(currentTakeProfit-pParams->bidAsk.ask[0])); else   
		modifyTradeEasy(SELL , -1, cOpen(orderAge)+spread-pParams->bidAsk.ask[0] + newSL, 0);
	}

	if ((pParams->bidAsk.bid[0]-minStop > cOpen(orderAge) - newSL) &&
		(totalOpenOrders(pParams, BUY) > 0)){

		if(currentTakeProfit != 0)
		modifyTradeEasy(BUY , -1, pParams->bidAsk.bid[0]-cOpen(orderAge) + newSL, fabs(currentTakeProfit-pParams->bidAsk.bid[0])); else 
	    modifyTradeEasy(BUY , -1, pParams->bidAsk.bid[0]-cOpen(orderAge) + newSL, 0);
	}

  }

  if (parameter(DSL_TYPE_ML ) == DSL_SQUARE_WITH_SKIP){

	newSL = stopLoss+(-stopLoss/(sqrt(parameter(DSL_BREAKEVEN_BARS_ML))))*sqrt((double)orderAge);

	for(i=1;i<orderAge;i++){

		if(totalOpenOrders(pParams, SELL) > 0){
			if(cOpen(i)>cClose(i)){
				newSL -= 0.5*fabs(cClose(i)-cOpen(1)) ;
			}
		}

		if(totalOpenOrders(pParams, BUY) > 0){
			if(cOpen(i)<cClose(i)){
				newSL -= 0.5*fabs(cClose(i)-cOpen(1)) ;
			}
		}
	}
  
	if ((pParams->bidAsk.ask[0]+minStop < cOpen(orderAge)+ spread + newSL) &&
		(totalOpenOrders(pParams, SELL) > 0)){

		if(currentTakeProfit != 0)
		modifyTradeEasy(SELL , -1, cOpen(orderAge)+spread-pParams->bidAsk.ask[0] + newSL, fabs(currentTakeProfit-pParams->bidAsk.ask[0])); else   
		modifyTradeEasy(SELL , -1, cOpen(orderAge)+spread-pParams->bidAsk.ask[0] + newSL, 0);
	}

	if ((pParams->bidAsk.bid[0]-minStop > cOpen(orderAge) - newSL) &&
		(totalOpenOrders(pParams, BUY) > 0)){

		if(currentTakeProfit != 0)
		modifyTradeEasy(BUY , -1, pParams->bidAsk.bid[0]-cOpen(orderAge) + newSL, fabs(currentTakeProfit-pParams->bidAsk.bid[0])); else 
	    modifyTradeEasy(BUY , -1, pParams->bidAsk.bid[0]-cOpen(orderAge) + newSL, 0);
	}

  }

  if (parameter(DSL_TYPE_ML ) == DSL_X4){

	newSL = stopLoss+(-stopLoss/(pow(parameter(DSL_BREAKEVEN_BARS_ML), 4)))*pow((double)orderAge, 4);
  
	if ((pParams->bidAsk.ask[0]+minStop < cOpen(orderAge)+ spread + newSL) &&
		(totalOpenOrders(pParams, SELL) > 0)){

		if(currentTakeProfit != 0)
		modifyTradeEasy(SELL , -1, cOpen(orderAge)+spread-pParams->bidAsk.ask[0] + newSL, fabs(currentTakeProfit-pParams->bidAsk.ask[0])); else   
		modifyTradeEasy(SELL , -1, cOpen(orderAge)+spread-pParams->bidAsk.ask[0] + newSL, 0);
	}

	if ((pParams->bidAsk.bid[0]-minStop > cOpen(orderAge) - newSL) &&
		(totalOpenOrders(pParams, BUY) > 0)){

		if(currentTakeProfit != 0)
		modifyTradeEasy(BUY , -1, pParams->bidAsk.bid[0]-cOpen(orderAge) + newSL, fabs(currentTakeProfit-pParams->bidAsk.bid[0])); else 
	    modifyTradeEasy(BUY , -1, pParams->bidAsk.bid[0]-cOpen(orderAge) + newSL, 0);
	}

  }

	if (parameter(DSL_TYPE_ML ) == DSL_X025){

	newSL = stopLoss+(-stopLoss/(pow(parameter(DSL_BREAKEVEN_BARS_ML),0.25)))*pow((double)orderAge, 0.25);
  
	if ((pParams->bidAsk.ask[0]+minStop < cOpen(orderAge)+ spread + newSL) &&
		(totalOpenOrders(pParams, SELL) > 0)){

		if(currentTakeProfit != 0)
		modifyTradeEasy(SELL , -1, cOpen(orderAge)+spread-pParams->bidAsk.ask[0] + newSL, fabs(currentTakeProfit-pParams->bidAsk.ask[0])); else   
		modifyTradeEasy(SELL , -1, cOpen(orderAge)+spread-pParams->bidAsk.ask[0] + newSL, 0);
	}

	if ((pParams->bidAsk.bid[0]-minStop > cOpen(orderAge) - newSL) &&
		(totalOpenOrders(pParams, BUY) > 0)){

		if(currentTakeProfit != 0)
		modifyTradeEasy(BUY , -1, pParams->bidAsk.bid[0]-cOpen(orderAge) + newSL, fabs(currentTakeProfit-pParams->bidAsk.bid[0])); else 
	    modifyTradeEasy(BUY , -1, pParams->bidAsk.bid[0]-cOpen(orderAge) + newSL, 0);
	}

  }


  return SUCCESS;
}
