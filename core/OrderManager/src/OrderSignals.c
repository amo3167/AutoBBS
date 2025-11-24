/**
 * @file
 * @brief     Functions for generating order entry and exit signals.
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

#include "AsirikuyLogger.h"
#include "OrderSignals.h"
#include "OrderManagement.h"
#include "Logging.h"

typedef enum signalType_t
{
  ENTRY_SIGNAL  = 0,
  EXIT_SIGNAL   = 1,
  UPDATE_SIGNAL = 2
} SignalType;

TradingSignal mapOrderAndSignalTypeToTradingSignal(OrderType orderType, SignalType signalType)
{
  switch(orderType)
  {
  case BUY:
    {
      switch(signalType)
      {
      case ENTRY_SIGNAL:
        {
          return SIGNAL_OPEN_BUY;
        }
      case EXIT_SIGNAL:
        {
          return SIGNAL_CLOSE_BUY;
        }
      case UPDATE_SIGNAL:
        {
          return SIGNAL_UPDATE_BUY;
        }
      default:
        {
          return SIGNAL_NONE;
        }
      }
    }
  case SELL:
    {
      switch(signalType)
      {
      case ENTRY_SIGNAL:
        {
          return SIGNAL_OPEN_SELL;
        }
      case EXIT_SIGNAL:
        {
          return SIGNAL_CLOSE_SELL;
        }
      case UPDATE_SIGNAL:
        {
          return SIGNAL_UPDATE_SELL;
        }
      default:
        {
          return SIGNAL_NONE;
        }
      }
    }
  case BUYLIMIT:
    {
      switch(signalType)
      {
      case ENTRY_SIGNAL:
        {
          return SIGNAL_OPEN_BUYLIMIT;
        }
      case EXIT_SIGNAL:
        {
          return SIGNAL_CLOSE_BUYLIMIT;
        }
      case UPDATE_SIGNAL:
        {
          return SIGNAL_UPDATE_BUYLIMIT;
        }
      default:
        {
          return SIGNAL_NONE;
        }
      }
    }
  case SELLLIMIT:
    {
      switch(signalType)
      {
      case ENTRY_SIGNAL:
        {
          return SIGNAL_OPEN_SELLLIMIT;
        }
      case EXIT_SIGNAL:
        {
          return SIGNAL_CLOSE_SELLLIMIT;
        }
      case UPDATE_SIGNAL:
        {
          return SIGNAL_UPDATE_SELLLIMIT;
        }
      default:
        {
          return SIGNAL_NONE;
        }
      }
    }
  case BUYSTOP:
    {
      switch(signalType)
      {
      case ENTRY_SIGNAL:
        {
          return SIGNAL_OPEN_BUYSTOP;
        }
      case EXIT_SIGNAL:
        {
          return SIGNAL_CLOSE_BUYSTOP;
        }
      case UPDATE_SIGNAL:
        {
          return SIGNAL_UPDATE_BUYSTOP;
        }
      default:
        {
          return SIGNAL_NONE;
        }
      }
    }
  case SELLSTOP:
    {
      switch(signalType)
      {
      case ENTRY_SIGNAL:
        {
          return SIGNAL_OPEN_SELLSTOP;
        }
      case EXIT_SIGNAL:
        {
          return SIGNAL_CLOSE_SELLSTOP;
        }
      case UPDATE_SIGNAL:
        {
          return SIGNAL_UPDATE_SELLSTOP;
        }
      default:
        {
          return SIGNAL_NONE;
        }
      }
    }
  default:
    {
      return SIGNAL_NONE;
    }
  }
}

AsirikuyReturnCode addOrderEntrySignal(OrderType orderType, StrategyParams* pParams, int* pTradingSignals)
{
  if(pParams == NULL)
  {
    logCritical("addOrderEntrySignal() failed. pParams = NULL\n\n");
    return NULL_POINTER;
  }

  if(pTradingSignals == NULL)
  {
    logCritical("addOrderEntrySignal() failed. pTradingSignals = NULL\n\n");
    return NULL_POINTER;
  }

  /* If operational mode is set to MONITOR don't generate an entry signal */
  if(pParams->settings[OPERATIONAL_MODE] == MODE_MONITOR)
  {
    return SUCCESS;
  }

  /* Check if the strategy has reached it's monti-carlo derived worst case scenario */
  if(pParams->accountInfo.largestDrawdownPercent >= pParams->settings[MAX_DRAWDOWN_PERCENT])
  {
    logAsirikuyError("addOrderEntrySignal()", WORST_CASE_SCENARIO);
    return WORST_CASE_SCENARIO;
  }

  /* Check if opening another order will risk a stopout by the broker. */
  /*if(!isEnoughFreeMargin(pParams, orderType))
  {
    logAsirikuyError("addOrderEntrySignal()", NOT_ENOUGH_MARGIN);
    return NOT_ENOUGH_MARGIN;
  }*/

  /* Check if the spread is too wide. */
  if(pParams->bidAsk.ask[0] - pParams->bidAsk.bid[0] > pParams->settings[MAX_SPREAD])
  {
    logAsirikuyError("addOrderEntrySignal()", SPREAD_TOO_WIDE);
    return SPREAD_TOO_WIDE;
  }

  *pTradingSignals |= mapOrderAndSignalTypeToTradingSignal(orderType, ENTRY_SIGNAL);
  return SUCCESS;
}

AsirikuyReturnCode addOrderExitSignal(OrderType orderType, StrategyParams* pParams, int* pTradingSignals)
{
  if(pParams == NULL)
  {
    logCritical("addOrderExitSignal() failed. pParams = NULL\n");
    return NULL_POINTER;
  }

  if(pTradingSignals == NULL)
  {
    logCritical("addOrderExitSignal() failed. pTradingSignals = NULL\n");
    return NULL_POINTER;
  }

  *pTradingSignals |= mapOrderAndSignalTypeToTradingSignal(orderType, EXIT_SIGNAL);
  return SUCCESS;
}

AsirikuyReturnCode addOrderUpdateSignal(OrderType orderType, StrategyParams* pParams, int* pTradingSignals)
{
  if(pParams == NULL)
  {
    logCritical("addOrderUpdateSignal() failed. pParams = NULL\n");
    return NULL_POINTER;
  }

  if(pTradingSignals == NULL)
  {
    logCritical("addOrderUpdateSignal() failed. pTradingSignals = NULL\n");
    return NULL_POINTER;
  }

  *pTradingSignals |= mapOrderAndSignalTypeToTradingSignal(orderType, UPDATE_SIGNAL);
  return SUCCESS;
}

AsirikuyReturnCode removeOrderEntrySignal(OrderType orderType, StrategyParams* pParams, int* pTradingSignals)
{
  if(pParams == NULL)
  {
    logCritical("removeOrderEntrySignal() failed. pParams = NULL\n");
    return NULL_POINTER;
  }

  if(pTradingSignals == NULL)
  {
    logCritical("removeOrderEntrySignal() failed. pTradingSignals = NULL\n");
    return NULL_POINTER;
  }

  *pTradingSignals &= ~mapOrderAndSignalTypeToTradingSignal(orderType, ENTRY_SIGNAL);
  return SUCCESS;
}

AsirikuyReturnCode removeAllOrderEntrySignals(int* pTradingSignals)
{
  if(pTradingSignals == NULL)
  {
    logCritical("removeAllOrderEntrySignals() failed. pTradingSignals = NULL\n");
    return NULL_POINTER;
  }

  *pTradingSignals &= ~mapOrderAndSignalTypeToTradingSignal(BUY      , ENTRY_SIGNAL);
  *pTradingSignals &= ~mapOrderAndSignalTypeToTradingSignal(SELL     , ENTRY_SIGNAL);
  *pTradingSignals &= ~mapOrderAndSignalTypeToTradingSignal(BUYLIMIT , ENTRY_SIGNAL);
  *pTradingSignals &= ~mapOrderAndSignalTypeToTradingSignal(SELLLIMIT, ENTRY_SIGNAL);
  *pTradingSignals &= ~mapOrderAndSignalTypeToTradingSignal(BUYSTOP  , ENTRY_SIGNAL);
  *pTradingSignals &= ~mapOrderAndSignalTypeToTradingSignal(SELLSTOP , ENTRY_SIGNAL);

  return SUCCESS;
}

AsirikuyReturnCode removeOrderExitSignal(OrderType orderType, StrategyParams* pParams, int* pTradingSignals)
{
  if(pParams == NULL)
  {
    logCritical("removeOrderExitSignal() failed. pParams = NULL\n");
    return NULL_POINTER;
  }

  if(pTradingSignals == NULL)
  {
    logCritical("removeOrderExitSignal() failed. pTradingSignals = NULL\n");
    return NULL_POINTER;
  }

  *pTradingSignals &= ~mapOrderAndSignalTypeToTradingSignal(orderType, EXIT_SIGNAL);
  return SUCCESS;
}

AsirikuyReturnCode removeAllOrderExitSignals(int* pTradingSignals)
{
  if(pTradingSignals == NULL)
  {
    logCritical("removeAllOrderExitSignals() failed. pTradingSignals = NULL\n");
    return NULL_POINTER;
  }

  *pTradingSignals &= ~mapOrderAndSignalTypeToTradingSignal(BUY      , EXIT_SIGNAL);
  *pTradingSignals &= ~mapOrderAndSignalTypeToTradingSignal(SELL     , EXIT_SIGNAL);
  *pTradingSignals &= ~mapOrderAndSignalTypeToTradingSignal(BUYLIMIT , EXIT_SIGNAL);
  *pTradingSignals &= ~mapOrderAndSignalTypeToTradingSignal(SELLLIMIT, EXIT_SIGNAL);
  *pTradingSignals &= ~mapOrderAndSignalTypeToTradingSignal(BUYSTOP  , EXIT_SIGNAL);
  *pTradingSignals &= ~mapOrderAndSignalTypeToTradingSignal(SELLSTOP , EXIT_SIGNAL);

  return SUCCESS;
}

AsirikuyReturnCode removeOrderUpdateSignal(OrderType orderType, StrategyParams* pParams, int* pTradingSignals)
{
  if(pParams == NULL)
  {
    logCritical("removeOrderUpdateSignal() failed. pParams = NULL\n");
    return NULL_POINTER;
  }

  if(pTradingSignals == NULL)
  {
    logCritical("removeOrderUpdateSignal() failed. pTradingSignals = NULL\n");
    return NULL_POINTER;
  }

  *pTradingSignals &= ~mapOrderAndSignalTypeToTradingSignal(orderType, UPDATE_SIGNAL);
  return SUCCESS;
}

AsirikuyReturnCode removeAllOrderUpdateSignals(int* pTradingSignals)
{
  if(pTradingSignals == NULL)
  {
    logCritical("removeAllOrderUpdateSignals() failed. pTradingSignals = NULL\n");
    return NULL_POINTER;
  }

  *pTradingSignals &= ~mapOrderAndSignalTypeToTradingSignal(BUY      , UPDATE_SIGNAL);
  *pTradingSignals &= ~mapOrderAndSignalTypeToTradingSignal(SELL     , UPDATE_SIGNAL);
  *pTradingSignals &= ~mapOrderAndSignalTypeToTradingSignal(BUYLIMIT , UPDATE_SIGNAL);
  *pTradingSignals &= ~mapOrderAndSignalTypeToTradingSignal(SELLLIMIT, UPDATE_SIGNAL);
  *pTradingSignals &= ~mapOrderAndSignalTypeToTradingSignal(BUYSTOP  , UPDATE_SIGNAL);
  *pTradingSignals &= ~mapOrderAndSignalTypeToTradingSignal(SELLSTOP , UPDATE_SIGNAL);

  return SUCCESS;
}

void addTradingSignal(TradingSignal signal, int* tradingSignals)
{
  *tradingSignals |= signal;
}

void removeTradingSignal(TradingSignal signal, int* tradingSignals)
{
  *tradingSignals &= ~signal;
}

BOOL hasEntryOrderType(OrderType orderType, int tradingSignals)
{
  return (mapOrderAndSignalTypeToTradingSignal(orderType, ENTRY_SIGNAL) & tradingSignals) != SIGNAL_NONE;
}

BOOL hasUpdateOrderType(OrderType orderType, int tradingSignals)
{
  return (mapOrderAndSignalTypeToTradingSignal(orderType, UPDATE_SIGNAL) & tradingSignals) != SIGNAL_NONE;
}

BOOL hasExitOrderType(OrderType orderType, int tradingSignals)
{
  return (mapOrderAndSignalTypeToTradingSignal(orderType, EXIT_SIGNAL) & tradingSignals) != SIGNAL_NONE;
}

BOOL hasTradingSignal(TradingSignal signal, int tradingSignals)
{
  return (signal & tradingSignals) != SIGNAL_NONE;
}

BOOL hasEntrySignal(int tradingSignals)
{
  return (((SIGNAL_OPEN_BUY    & tradingSignals) != SIGNAL_NONE)
    || ((SIGNAL_OPEN_BUYLIMIT  & tradingSignals) != SIGNAL_NONE)
    || ((SIGNAL_OPEN_BUYSTOP   & tradingSignals) != SIGNAL_NONE)
    || ((SIGNAL_OPEN_SELL      & tradingSignals) != SIGNAL_NONE)
    || ((SIGNAL_OPEN_SELLLIMIT & tradingSignals) != SIGNAL_NONE)
    || ((SIGNAL_OPEN_SELLSTOP  & tradingSignals) != SIGNAL_NONE));
}

BOOL hasUpdateSignal(int tradingSignals)
{
  return (((SIGNAL_UPDATE_BUY    & tradingSignals) != SIGNAL_NONE)
    || ((SIGNAL_UPDATE_BUYLIMIT  & tradingSignals) != SIGNAL_NONE)
    || ((SIGNAL_UPDATE_BUYSTOP   & tradingSignals) != SIGNAL_NONE)
    || ((SIGNAL_UPDATE_SELL      & tradingSignals) != SIGNAL_NONE)
    || ((SIGNAL_UPDATE_SELLLIMIT & tradingSignals) != SIGNAL_NONE)
    || ((SIGNAL_UPDATE_SELLSTOP  & tradingSignals) != SIGNAL_NONE));
}

BOOL hasExitSignal(int tradingSignals)
{
  return (((SIGNAL_CLOSE_BUY    & tradingSignals) != SIGNAL_NONE)
    || ((SIGNAL_CLOSE_BUYLIMIT  & tradingSignals) != SIGNAL_NONE)
    || ((SIGNAL_CLOSE_BUYSTOP   & tradingSignals) != SIGNAL_NONE)
    || ((SIGNAL_CLOSE_SELL      & tradingSignals) != SIGNAL_NONE)
    || ((SIGNAL_CLOSE_SELLLIMIT & tradingSignals) != SIGNAL_NONE)
    || ((SIGNAL_CLOSE_SELLSTOP  & tradingSignals) != SIGNAL_NONE));
}