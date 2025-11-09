/**
 * @file
 * @brief     Shared test fixtures for TradingStrategies unit tests
 * 
 * @author    Morgan Doel (Initial implementation)
 * @version   F4.x.x
 * @date      2025
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

#ifndef TRADING_STRATEGIES_TEST_FIXTURES_HPP
#define TRADING_STRATEGIES_TEST_FIXTURES_HPP

#include "StrategyContext.hpp"
#include <cstring>

/**
 * @brief Test fixture providing mock StrategyParams data
 * 
 * This fixture sets up complete mock data for testing strategies, including:
 * - Bid/Ask price arrays (10 ticks)
 * - Account information (balance, equity, margin)
 * - Order information
 * - Strategy results structure
 * - Default symbol: EURUSD
 * - Default strategy: RECORD_BARS
 */
struct StrategyContextFixture {
    StrategyParams params;
    BidAsk bidAsk;
    AccountInfo accountInfo;
    RatesBuffers ratesBuffers;
    OrderInfo orderInfo;
    StrategyResults results;
    
    // Storage for bid/ask arrays
    double bidArray[10];
    double askArray[10];
    
    StrategyContextFixture() {
        // Zero-initialize everything
        std::memset(&params, 0, sizeof(params));
        std::memset(&bidAsk, 0, sizeof(bidAsk));
        std::memset(&accountInfo, 0, sizeof(accountInfo));
        std::memset(&ratesBuffers, 0, sizeof(ratesBuffers));
        std::memset(&orderInfo, 0, sizeof(orderInfo));
        std::memset(&results, 0, sizeof(results));
        std::memset(bidArray, 0, sizeof(bidArray));
        std::memset(askArray, 0, sizeof(askArray));
        
        // Set up bid/ask data
        bidAsk.arraySize = 10;
        bidAsk.bid = bidArray;
        bidAsk.ask = askArray;
        
        // Populate some bid/ask values
        for (int i = 0; i < 10; ++i) {
            bidArray[i] = 1.2000 + i * 0.0001;
            askArray[i] = 1.2010 + i * 0.0001;
        }
        
        // Set up account info
        accountInfo.balance = 10000.0;
        accountInfo.equity = 10500.0;
        accountInfo.margin = 9500.0;
        
        // Set up order info (no orderCount field exists)
        // orderInfo is just a pointer, actual data managed by OrderManager
        
        // Link pointers
        params.bidAsk = bidAsk;
        params.accountInfo = accountInfo;
        params.ratesBuffers = &ratesBuffers;
        params.orderInfo = &orderInfo;
        params.results = &results;
        
        // Set symbol
        std::strcpy(params.tradeSymbol, "EURUSD");
        
        // Set strategy ID (use RECORD_BARS as a valid strategy)
        params.settings[INTERNAL_STRATEGY_ID] = static_cast<double>(RECORD_BARS);
        
        // Set some settings
        params.settings[IS_BACKTESTING] = 1.0;
        params.settings[MAX_OPEN_ORDERS] = 5.0;
        
        // Set current time
        params.currentBrokerTime = 1234567890;
    }
};

#endif // TRADING_STRATEGIES_TEST_FIXTURES_HPP
