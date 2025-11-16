/**
 * @file
 * @brief     MQL specific definitions, enums, & structs.
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

#ifndef MQL_DEFINES_H_
#define MQL_DEFINES_H_
#pragma once

#define REQUIRED_RATES_INDEXES 6

#pragma pack(push, 1)
typedef struct mql4Rates_t
{
#if defined _WIN32 || defined _WIN64
	__time32_t time;
#else
    unsigned int time;
#endif
	double     open;
	double     low;
	double     high;
	double     close;
	double     volume;
} Mql4Rates;

typedef struct Mql5Rates_t
  {
#if defined _WIN32 || defined _WIN64
   __time64_t    time;        // Period start time
#else
   unsigned long time;
#endif
   double		     open;        // Open price
   double		     high;        // The highest price of the period
   double		     low;         // The lowest price of the period
   double		     close;       // Close price
   long long int tick_volume; // Tick volume
   long int      spread;      // Spread
   long long int real_volume; // Trade volume
  } Mql5Rates;
#pragma pack(pop)

typedef struct mql4String_t
{
	long  length;
	char* string;
} Mql4String;

typedef enum MQLVersion_t
{
  MQL4 = 4,
  MQL5 = 5
} MQLVersion;

typedef enum bidAskIndex_t
{
  IDX_BID                  = 0,
  IDX_ASK                  = 1,
  IDX_BASE_CONVERSION_BID  = 2,
  IDX_BASE_CONVERSION_ASK  = 3,
  IDX_QUOTE_CONVERSION_BID = 4,
  IDX_QUOTE_CONVERSION_ASK = 5
} BidAskIndex;

typedef enum accountInfoIndex_t
{
  IDX_ACCOUNT_NUMBER                = 0,
  IDX_BALANCE                       = 1,
  IDX_EQUITY                        = 2,
  IDX_MARGIN                        = 3,
  IDX_LEVERAGE                      = 4,
  IDX_CONTRACT_SIZE                 = 5,
  IDX_MINIMUM_STOP                  = 6,
  IDX_STOPOUT_PERCENT               = 7,
  IDX_TOTAL_OPEN_TRADE_RISK_PERCENT = 8,
  IDX_LARGEST_DRAWDOWN_PERCENT      = 9
} AccountInfoIndex;

typedef struct mqlOrderInfo_t
{
  double ticket;
  double instanceId;
  double type;
  double openTime;
  double closeTime;
  double stopLoss;
  double takeProfit;
  double expiriation;
  double openPrice;
  double closePrice;
  double lots;
  double profit;
  double commission;
  double swap;
  double isOpen;
} MqlOrderInfo;

typedef struct mqlRatesInfo_t
{
  double isEnabled;
  double requiredTimeframe;
  double totalBarsRequired;
  double actualTimeframe;
  double ratesArraySize;
  double point;
  double digits;
} MqlRatesInfo;

#endif /* MQL_DEFINES_H_ */
