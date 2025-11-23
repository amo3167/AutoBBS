/**
* @file
* @brief     An MQL API for the Trading Strategies library.
* 
* @author    Daniel Fernandez (initial implementation)
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
#include "AsirikuyLogger.h"
#include "AsirikuyDefines.h"
#include "AsirikuyFrameworkAPI.h"
#include "MQLParameters.h"
#include "ContiguousRatesCircBuf.h"
#include "Logging.h"
#include "AsirikuyStrategies.h"
#include "AsirikuyTime.h"
#include "StrategyUserInterface.h"

static AsirikuyReturnCode verifyPointers(
  MQLVersion mqlVersion,
  void*      pInSettings,
  void*      pInTradeSymbol,
  void*      pInAccountCurrency,
  void*      pInBrokerName,
  void*      pInRefBrokerName,
  void*      pInCurrentBrokerTime,
  void*      pInOpenOrdersCount,
  void*      pInOrderInfo,
  void*      pInAccountInfo,
  void*      pInBidAsk,
  void*      pInRatesInfo,
  void*      pInRates_0,
  void*      pInRates_1,
  void*      pInRates_2,
  void*      pInRates_3,
  void*      pInRates_4,
  void*      pInRates_5,
  void*      pInRates_6,
  void*      pInRates_7,
  void*      pInRates_8,
  void*      pInRates_9,
  void*      pOutResults)
{
  if(pInSettings == NULL)
  {
    logCritical("verifyPointers() failed. pInSettings = NULL\n");
    return NULL_POINTER;
  }

  if(pInTradeSymbol == NULL)
  {
    logCritical("verifyPointers() failed. pInTradeSymbol = NULL\n");
    return NULL_POINTER;
  }

  if(pInAccountCurrency == NULL)
  { 
    logCritical("verifyPointers() failed. pInAccountCurrency = NULL\n");
    return NULL_POINTER;
  }

  if(pInBrokerName == NULL)
  {
    logCritical("verifyPointers() failed. pInBrokerName = NULL\n");
    return NULL_POINTER;
  }

  if(pInRefBrokerName == NULL)
  {
    logCritical("verifyPointers() failed. pInRefBrokerName = NULL\n");
    return NULL_POINTER;
  }
  
  if(pInCurrentBrokerTime == NULL)
  {
    logCritical("verifyPointers() failed. pInCurrentBrokerTime = NULL\n");
    return NULL_POINTER;
  }

  if(pInOpenOrdersCount == NULL)
  {
    logCritical("verifyPointers() failed. pInOpenOrdersCount = NULL\n");
    return NULL_POINTER;
  }

  if(pInOrderInfo == NULL)
  {
    logCritical("verifyPointers() failed. pInOrderInfo = NULL\n");
    return NULL_POINTER;
  }

  if(pInAccountInfo == NULL)
  {
    logCritical("verifyPointers() failed. pInAccountInfo = NULL\n");
    return NULL_POINTER;
  }

  if(pInBidAsk == NULL)
  {
    logCritical("verifyPointers() failed. pInBidAsk = NULL\n");
    return NULL_POINTER;
  }

  if(pInRatesInfo == NULL)
  {
    logCritical("verifyPointers() failed. pInRatesInfo = NULL\n");
    return NULL_POINTER;
  }

  if(pInRates_0 == NULL)
  {
    logCritical("verifyPointers() failed. pInRates_0 = NULL\n");
    return NULL_POINTER;
  }
  
  if(pOutResults == NULL)
  {
    logCritical("verifyPointers() failed. pOutResults = NULL\n");
    return NULL_POINTER;
  }

  return SUCCESS;
}

#ifdef __cplusplus
extern "C" {
#endif

  int __stdcall mql_runStrategy(
    MQLVersion    mqlVersion,
    double*       pInSettings,
    char*         pInTradeSymbol,
    char*         pInAccountCurrency,
    char*         pInBrokerName,
    char*         pInRefBrokerName,
    int*          pInCurrentBrokerTime,
    int*          pInOpenOrdersCount,
    MqlOrderInfo* pInOrderInfo,
    double*       pInAccountInfo,
    double*       pInBidAsk,
    MqlRatesInfo* pInRatesInfo,
    void*         pInRates_0,
    void*         pInRates_1,
    void*         pInRates_2,
    void*         pInRates_3,
    void*         pInRates_4,
    void*         pInRates_5,
    void*         pInRates_6,
    void*         pInRates_7,
    void*         pInRates_8,
    void*         pInRates_9,
    double*       pOutResults)
  {
    int result = SUCCESS;
    StrategyParams params;

    /* If any string pointers are NULL return now to avoid a memory access violation */
    result = verifyPointers(mqlVersion, pInSettings, pInTradeSymbol, pInAccountCurrency, pInBrokerName, pInRefBrokerName, pInCurrentBrokerTime, pInOpenOrdersCount, pInOrderInfo, pInAccountInfo, 
      pInBidAsk, pInRatesInfo, pInRates_0, pInRates_1, pInRates_2, pInRates_3, pInRates_4, pInRates_5, pInRates_6, pInRates_7, pInRates_8, pInRates_9, pOutResults);
    
    if(result != SUCCESS)
    {
      logAsirikuyError("mql_runStrategy()", (AsirikuyReturnCode)result);
      return result;
    }

    if(pInSettings[OPERATIONAL_MODE] == MODE_DISABLE)
    {
      return result;
    }

    result = allocateOrderInfo(&params, (int)pInSettings[ORDERINFO_ARRAY_SIZE]);

    if(result == SUCCESS)
    {
      result = convertMqlParameters(mqlVersion, pInSettings, pInTradeSymbol, pInAccountCurrency, pInBrokerName, pInRefBrokerName, pInCurrentBrokerTime, pInOpenOrdersCount, pInOrderInfo, pInAccountInfo, 
        pInBidAsk, pInRatesInfo, (Mql5Rates*)pInRates_0, (Mql5Rates*)pInRates_1, (Mql5Rates*)pInRates_2, (Mql5Rates*)pInRates_3, (Mql5Rates*)pInRates_4, (Mql5Rates*)pInRates_5, (Mql5Rates*)pInRates_6, (Mql5Rates*)pInRates_7, (Mql5Rates*)pInRates_8, (Mql5Rates*)pInRates_9, (StrategyResults*)pOutResults, &params);
    }

	saveUserHeartBeat((int)params.settings[STRATEGY_INSTANCE_ID], (BOOL)params.settings[IS_BACKTESTING] );

    if(result == SUCCESS)
    {
      result = runStrategy(&params);
    }

    if(result != SUCCESS)
    {
      logAsirikuyError("mql_runStrategy()", (AsirikuyReturnCode)result);
      freeOrderInfo(&params);
      return result;
    }

    result = freeOrderInfo(&params);
    if(result != SUCCESS)
    {
      logAsirikuyError("mql_runStrategy()", (AsirikuyReturnCode)result);
    }

    return result;
  }

  int __stdcall mql4_runStrategy(
    double*       pInSettings,
    Mql4String*   pInTradeSymbol,
    Mql4String*   pInAccountCurrency,
    Mql4String*   pInBrokerName,
	  Mql4String*   pInRefBrokerName,
    int*          pInCurrentBrokerTime,
    int*          pInOpenOrdersCount,
    MqlOrderInfo* pInOrderInfo,
    double*       pInAccountInfo,
    double*       pInBidAsk,
    MqlRatesInfo* pInRatesInfo,
    Mql4Rates*    pInRates_0,
    Mql4Rates*    pInRates_1,
    Mql4Rates*    pInRates_2,
    Mql4Rates*    pInRates_3,
    Mql4Rates*    pInRates_4,
    Mql4Rates*    pInRates_5,
    Mql4Rates*    pInRates_6,
    Mql4Rates*    pInRates_7,
    Mql4Rates*    pInRates_8,
    Mql4Rates*    pInRates_9,
    double*       pOutResults)
  {
    return mql_runStrategy(MQL4, pInSettings, pInTradeSymbol[0].string, pInAccountCurrency[0].string, pInBrokerName[0].string, pInRefBrokerName[0].string, 
      pInCurrentBrokerTime, pInOpenOrdersCount, pInOrderInfo, pInAccountInfo, pInBidAsk, pInRatesInfo, 
      pInRates_0, pInRates_1, pInRates_2, pInRates_3, pInRates_4, pInRates_5, pInRates_6, pInRates_7, pInRates_8, pInRates_9, pOutResults);
  }

  int __stdcall mql5_runStrategy(
    double*       pInSettings,
    char*         pInTradeSymbol,
    char*         pInAccountCurrency,
    char*         pInBrokerName,
    char*         pInRefBrokerName,
    int*          pInCurrentBrokerTime,
    int*          pInOpenOrdersCount,
    MqlOrderInfo* pInOrderInfo,
    double*       pInAccountInfo,
    double*       pInBidAsk,
    MqlRatesInfo* pInRatesInfo,
    Mql5Rates*    pInRates_0,
    Mql5Rates*    pInRates_1,
    Mql5Rates*    pInRates_2,
    Mql5Rates*    pInRates_3,
    Mql5Rates*    pInRates_4,
    Mql5Rates*    pInRates_5,
    Mql5Rates*    pInRates_6,
    Mql5Rates*    pInRates_7,
    Mql5Rates*    pInRates_8,
    Mql5Rates*    pInRates_9,
    double*       pOutResults)
  {
    return mql_runStrategy(MQL5, pInSettings, pInTradeSymbol, pInAccountCurrency, pInBrokerName, pInRefBrokerName, 
      pInCurrentBrokerTime, pInOpenOrdersCount, pInOrderInfo, pInAccountInfo, pInBidAsk, pInRatesInfo, 
      pInRates_0, pInRates_1, pInRates_2, pInRates_3, pInRates_4, pInRates_5, pInRates_6, pInRates_7, pInRates_8, pInRates_9, pOutResults);
  }

#ifdef __cplusplus
} /* extern "C" */
#endif