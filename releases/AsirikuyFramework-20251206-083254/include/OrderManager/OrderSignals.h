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

#ifndef ORDER_SIGNALS_H_
#define ORDER_SIGNALS_H_
#pragma once

#ifndef ASIRIKUY_DEFINES_H_
  #include "AsirikuyDefines.h"
#endif

typedef enum tradingSignal_t
{
  SIGNAL_NONE             = 0x00000000,
  SIGNAL_OPEN_BUY         = 0x00000001,
  SIGNAL_CLOSE_BUY        = 0x00000002,
  SIGNAL_UPDATE_BUY       = 0x00000004,
  SIGNAL_OPEN_BUYLIMIT    = 0x00000008,
  SIGNAL_CLOSE_BUYLIMIT   = 0x00000010,
  SIGNAL_UPDATE_BUYLIMIT  = 0x00000020,
  SIGNAL_OPEN_BUYSTOP     = 0x00000040,
  SIGNAL_CLOSE_BUYSTOP    = 0x00000080,
  SIGNAL_UPDATE_BUYSTOP   = 0x00000100,
  SIGNAL_OPEN_SELL        = 0x00010000,
  SIGNAL_CLOSE_SELL       = 0x00020000,
  SIGNAL_UPDATE_SELL      = 0x00040000,
  SIGNAL_OPEN_SELLLIMIT   = 0x00080000,
  SIGNAL_CLOSE_SELLLIMIT  = 0x00100000,
  SIGNAL_UPDATE_SELLLIMIT = 0x00200000,
  SIGNAL_OPEN_SELLSTOP    = 0x00400000,
  SIGNAL_CLOSE_SELLSTOP   = 0x00800000,
  SIGNAL_UPDATE_SELLSTOP  = 0x01000000
} TradingSignal;

#ifdef __cplusplus
extern "C" {
#endif

/**
* Adds an order entry signal.
*
* @param OrderType orderType
*   The type of entry signal to add.
*
* @param const StrategyParams* pParams
*   The strategy parameters
*
* @param int* pTradingSignals
*   A pointer to the integer where trading signals are stored.
*
* @return AsirikuyReturnCode
*   An enum indicating success or the type of failure that occured.
*/
AsirikuyReturnCode addOrderEntrySignal(OrderType orderType, StrategyParams* pParams, int* pTradingSignals);

/**
* Adds an order exit signal.
*
* @param OrderType orderType
*   The type of exit signal to add.
*
* @param const StrategyParams* pParams
*   The strategy parameters
*
* @param int* pTradingSignals
*   A pointer to the integer where trading signals are stored.
*
* @return AsirikuyReturnCode
*   An enum indicating success or the type of failure that occured.
*/
AsirikuyReturnCode addOrderExitSignal(OrderType orderType, StrategyParams* pParams, int* pTradingSignals);

/**
* Adds an order update signal.
*
* @param OrderType orderType
*   The type of update signal to add.
*
* @param const StrategyParams* pParams
*   The strategy parameters
*
* @param int* pTradingSignals
*   A pointer to the integer where trading signals are stored.
*
* @return AsirikuyReturnCode
*   An enum indicating success or the type of failure that occured.
*/
AsirikuyReturnCode addOrderUpdateSignal(OrderType orderType, StrategyParams* pParams, int* pTradingSignals);

/**
* Removes an order entry signal.
*
* @param OrderType orderType
*   The type of entry signal to remove.
*
* @param const StrategyParams* pParams
*   The strategy parameters
*
* @param int* pTradingSignals
*   A pointer to the integer where trading signals are stored.
*
* @return AsirikuyReturnCode
*   An enum indicating success or the type of failure that occured.
*/
AsirikuyReturnCode removeOrderEntrySignal(OrderType orderType, StrategyParams* pParams, int* pTradingSignals);

/**
* Removes all the order entry signals.
*
* @param int* pTradingSignals
*   This is a temporary integer for trading signals to be accumulated.
*
* @return AsirikuyReturnCode
*   An enum indicating success or the type of failure that occured.
*/
AsirikuyReturnCode removeAllOrderEntrySignals(int* pTradingSignals);

/**
* Removes an order exit signal.
*
* @param OrderType orderType
*   The type of exit signal to remove.
*
* @param const StrategyParams* pParams
*   The strategy parameters
*
* @param int* pTradingSignals
*   A pointer to the integer where trading signals are stored.
*
* @return AsirikuyReturnCode
*   An enum indicating success or the type of failure that occured.
*/
AsirikuyReturnCode removeOrderExitSignal(OrderType orderType, StrategyParams* pParams, int* pTradingSignals);

/**
* Removes all the order exit signals.
*
* @param int* pTradingSignals
*   This is a temporary integer for trading signals to be accumulated.
*
* @return AsirikuyReturnCode
*   An enum indicating success or the type of failure that occured.
*/
AsirikuyReturnCode removeAllOrderExitSignals(int* pTradingSignals);

/**
* Removes an order update signal.
*
* @param OrderType orderType
*   The type of update signal to remove.
*
* @param const StrategyParams* pParams
*   The strategy parameters
*
* @param int* pTradingSignals
*   A pointer to the integer where trading signals are stored.
*
* @return AsirikuyReturnCode
*   An enum indicating success or the type of failure that occured.
*/
AsirikuyReturnCode removeOrderUpdateSignal(OrderType orderType, StrategyParams* pParams, int* pTradingSignals);

/**
* Removes all the order update signals.
*
* @param int* pTradingSignals
*   This is a temporary integer for trading signals to be accumulated.
*
* @return AsirikuyReturnCode
*   An enum indicating success or the type of failure that occured.
*/
AsirikuyReturnCode removeAllOrderUpdateSignals(int* pTradingSignals);

void addTradingSignal(TradingSignal signal, int* tradingSignals);

void removeTradingSignal(TradingSignal signal, int* tradingSignals);

/**
* Checks if the trading signals buffer contains an entry signal for the specified order type.
*
* @param OrderType orderType
*   The type of order to check for.
*
* @param int tradingSignals
*   The buffer in which trading signals are stored.
*
* @return BOOL
*   Returns TRUE if the signal exists. Otherwise returns FALSE.
*/
BOOL hasEntryOrderType(OrderType orderType, int tradingSignals);


/**
* Checks if the trading signals buffer contains an update signal for the specified order type.
*
* @param OrderType orderType
*   The type of order to check for.
*
* @param int tradingSignals
*   The buffer in which trading signals are stored.
*
* @return BOOL
*   Returns TRUE if the signal exists. Otherwise returns FALSE.
*/
BOOL hasUpdateOrderType(OrderType orderType, int tradingSignals);

/**
* Checks if the trading signals buffer contains an exit signal for the specified order type.
*
* @param OrderType orderType
*   The type of order to check for.
*
* @param int tradingSignals
*   The buffer in which trading signals are stored.
*
* @return BOOL
*   Returns TRUE if the signal exists. Otherwise returns FALSE.
*/
BOOL hasExitOrderType(OrderType orderType, int tradingSignals);

/**
* Checks if the trading signals buffer contains the specified signal.
*
* @param TradingSignal signal
*   The type of signal to check for.
*
* @param int tradingSignals
*   The buffer in which trading signals are stored.
*
* @return BOOL
*   Returns TRUE if the signal exists. Otherwise returns FALSE.
*/
BOOL hasTradingSignal(TradingSignal signal, int tradingSignals);

/**
* Checks if the trading signals buffer contains an open signal.
*
* @param int tradingSignals
*   The buffer in which trading signals are stored.
*
* @return BOOL
*   Returns TRUE if the signal exists. Otherwise returns FALSE.
*/
BOOL hasEntrySignal(int tradingSignals);

/**
* Checks if the trading signals buffer contains an update signal.
*
* @param int tradingSignals
*   The buffer in which trading signals are stored.
*
* @return BOOL
*   Returns TRUE if the signal exists. Otherwise returns FALSE.
*/
BOOL hasUpdateSignal(int tradingSignals);

/**
* Checks if the trading signals buffer contains a close signal.
*
* @param int tradingSignals
*   The buffer in which trading signals are stored.
*
* @return BOOL
*   Returns TRUE if the signal exists. Otherwise returns FALSE.
*/
BOOL hasExitSignal(int tradingSignals);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* ORDER_SIGNALS_H_ */