/**
 * @file
 * @brief     Parses and provides detailed information about forex currencies and symbols (instruments).
 * @details   Symbols are broken down into prefix, base currency, separator, quote currency, and suffix.
 * @details   This allows for many non-standardized symbol names to be identified reliably.
 * @details   Detailed information can be obtained for a currency including the currency name, number, 
 * @details   the number of digits after the decimal separator, and the locations where the currency is used.
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

#ifndef SYMBOL_ANALYZER_H_
#define SYMBOL_ANALYZER_H_
#pragma once

#ifndef ASIRIKUY_DEFINES_H_
  #include "AsirikuyDefines.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif

/**
* Parse a symbol and extract the prefix, base currency,
* separator, quote currency, and suffix.
*
* This function is a superset of all the following functions:
* getCurrencyPairPrefix, getBaseCurrency, getCurrencyPairSeparator, 
* getQuoteCurrency and getCurrencyPairSuffix.
*
* @param const char* pSymbol
*   The symbol from which to extract the prefix.
*
* @param char* pPrefix
*   The buffer where the prefix will be stored.
*   It should be already allocated with at least 10 bytes.
*
* @param char* pBaseCurrency
*   The buffer where the base currency will be stored.
*   It should be already allocated with at least 4 bytes.
*
* @param char* pSeparator
*   The buffer where the separator will be stored.
*   It should be already allocated with at least 10 bytes.
*
* @param char* pQuoteCurrency
*   The buffer where the quote currency will be stored.
*   It should be already allocated with at least 4 bytes.
*
* @param char* pSuffix
*   The buffer where the suffix will be stored.
*   It should be already allocated with at least 10 bytes.
*
* @return enum AsirikuyReturnCode
*   An enum indicating success or the type of failure that occured.
*/
AsirikuyReturnCode parseSymbol(const char* pSymbol, char* pPrefix, char* pBaseCurrency, char* pSeparator, char* pQuoteCurrency, char* pSuffix);

/**
* Normalize a symbol by removing any prefix, suffix, and separator.
*
* Eg. "pEUR/USDs" becomes "EURUSD".
*
* @param char* pSymbol
*   The symbol to be normalized.
*
* @return enum AsirikuyReturnCode
*   An enum indicating success or the type of failure that occured.
*/
AsirikuyReturnCode normalizeSymbol(char* pSymbol);

/**
* Normalize a currency by removing any prefix or suffix. If the currency is
* not recognized then it will be returned as "USD". This will happen in some
* cent accounts where names such as "CNT" are used.
*
* Eg. "AUDC" becomes "AUD". "CNT" is not recognized so it becomes "USD".
*
* @param char* pCurrency
*   The currency to be normalized.
*
* @return enum AsirikuyReturnCode
*   An enum indicating success or the type of failure that occured.
*/
AsirikuyReturnCode normalizeCurrency(char* pCurrency);

/**
* Generate symbols to be used for converting profits and losses into account currency.
*
* Eg. If pSymbol is "EUR/USD" and pAccountCurrency is "AUD"
* then pBaseConversionSymbol will be "EUR/AUD" and pQuoteConversionSymbol
* will be "AUD/USD".
*
* @param const char* pSymbol
*   The symbol from which pBaseConversionSymbol & 
*   pQuoteConversionSymbol are generated.
*
* @param char* pAccountCurrency
*   The account currency from which pBaseConversionSymbol & 
*   pQuoteConversionSymbol are generated.
*
* @param char* pBaseConversionSymbol
*   The buffer where the base conversion symbol will be stored.
*   It should be already allocated with at least 16 bytes.
*
* @param char* pQuoteConversionSymbol
*   The buffer where the quote conversion symbol will be stored.
*   It should be already allocated with at least 16 bytes.
*
* @return enum AsirikuyReturnCode
*   An enum indicating success or the type of failure that occured.
*/
AsirikuyReturnCode getConversionSymbols(const char* pSymbol, char* pAccountCurrency, char* pBaseConversionSymbol, char* pQuoteConversionSymbol);

/**
* Extract the prefix from a symbol.
*
* Eg. In "pEURUSD" The prefix is "p".
*
* @param const char* pSymbol
*   The symbol from which to extract the prefix.
*
* @param char* pPrefix
*   The buffer where the prefix will be stored.
*   It should be already allocated with at least 10 bytes.
*
* @return enum AsirikuyReturnCode
*   An enum indicating success or the type of failure that occured.
*/
AsirikuyReturnCode getCurrencyPairPrefix(const char* pSymbol, char* pPrefix);

/**
* Extract the base currency from a symbol.
*
* Eg. In "EURUSD" The base currency is "EUR".
*
* @param const char* pSymbol
*   The symbol from which to extract the base currency.
*
* @param char* pBaseCurrency
*   The buffer where the base currency will be stored.
*   It should be already allocated with at least 4 bytes.
*
* @return enum AsirikuyReturnCode
*   An enum indicating success or the type of failure that occured.
*/
AsirikuyReturnCode getBaseCurrency(const char* pSymbol, char* pBaseCurrency);

/**
* Extract the separator from a symbol.
*
* Eg. In "EUR/USD" The separator is "/".
*
* @param const char* pSymbol
*   The symbol from which to extract the separator.
*
* @param char* pSeparator
*   The buffer where the separator will be stored.
*   It should be already allocated with at least 10 bytes.
*
* @return enum AsirikuyReturnCode
*   An enum indicating success or the type of failure that occured.
*/
AsirikuyReturnCode getCurrencyPairSeparator(const char* pSymbol, char* pSeparator);

/**
* Extract the quote currency from a symbol.
*
* Eg. In "EURUSD" The quote currency is "USD".
*
* @param const char* pSymbol
*   The symbol from which to extract the quote currency.
*
* @param char* pQuoteCurrency
*   The buffer where the quote currency will be stored.
*   It should be already allocated with at least 4 bytes.
*
* @return enum AsirikuyReturnCode
*   An enum indicating success or the type of failure that occured.
*/
AsirikuyReturnCode getQuoteCurrency(const char* pSymbol, char* pQuoteCurrency);

/**
* Extract the suffix from a symbol.
*
* Eg. In "EURUSDs" The suffix is "s".
*
* @param const char* pSymbol
*   The symbol from which to extract the suffix.
*
* @param char* pSuffix
*   The buffer where the suffix will be stored.
*   It should be already allocated with at least 10 bytes.
*
* @return enum AsirikuyReturnCode
*   An enum indicating success or the type of failure that occured.
*/
AsirikuyReturnCode getCurrencyPairSuffix(const char* pSymbol, char* pSuffix);

/**
* Get information about a particular currency.
*
* This function is a superset of all the following functions:
* getCurrencyNumber, getNumDigitsAfterDecimal, getCurrencyName, and getCurrencyLocations.
*
* @param char* pCurrencyCode
*   A 3 character currency code. All capital letters.
*
* @param char* pCurrencyNumber
*   The buffer where the resulting string will be stored.
*   It should be already allocated with at least 4 bytes.
*
* @param char* pDigitsAfterDecimal
*   The buffer where the resulting string will be stored.
*   It should be already allocated with at least 2 bytes.
*
* @param char* pCurrencyName
*   The buffer where the name of the currency will be stored.
*   It should be already allocated with at least 250 bytes.
*
* @param char* pCurrencyLocations
*   The buffer where the list of currency locations will be stored.
*   It should be already allocated with at least 250 bytes.
*
* @return enum AsirikuyReturnCode
*   An enum indicating success or the type of failure that occured.
*/
AsirikuyReturnCode getCurrencyInfo(char* pCurrencyCode, char* pCurrencyNumber, char* pDigitsAfterDecimal, char* pCurrencyName, char* pCurrencyLocations);

/**
* Get the currency number for a particular currency.
*
* The number is stored as a string.
*
* @param char* pCurrencyCode
*   A 3 character currency code. All capital letters.
*
* @param char* pCurrencyNumber
*   The buffer where the resulting string will be stored.
*   It should be already allocated with at least 4 bytes.
*
* @return enum AsirikuyReturnCode
*   An enum indicating success or the type of failure that occured.
*/
AsirikuyReturnCode getCurrencyNumber(char* pCurrencyCode, char* pCurrencyNumber);

/**
* Get the number of digits after the decimal point for a particular currency.
*
* The number of digits is stored as a string.
*
* @param char* pCurrencyCode
*   A 3 character currency code. All capital letters.
*
* @param char* pDigitsAfterDecimal
*   The buffer where the resulting string will be stored.
*   It should be already allocated with at least 2 bytes.
*
* @return enum AsirikuyReturnCode
*   An enum indicating success or the type of failure that occured.
*/
AsirikuyReturnCode getNumDigitsAfterDecimal(char* pCurrencyCode, char* pDigitsAfterDecimal);

/**
* Get the name of a currency.
*
* @param char* pCurrencyCode
*   A 3 character currency code. All capital letters.
*
* @param char* pCurrencyName
*   The buffer where the name of the currency will be stored.
*   It should be already allocated with at least 250 bytes.
*
* @return enum AsirikuyReturnCode
*   An enum indicating success or the type of failure that occured.
*/
AsirikuyReturnCode getCurrencyName(char* pCurrencyCode, char* pCurrencyName);

/**
* Get the list of countries where a particular currency is used.
*
* The list is a single string with countries separated by commas.
*
* @param char* pCurrencyCode
*   A 3 character currency code. All capital letters.
*
* @param char* pCurrencyLocations
*   The buffer where the list of currency locations will be stored.
*   It should be already allocated with at least 250 bytes.
*
* @return enum AsirikuyReturnCode
*   An enum indicating success or the type of failure that occured.
*/
AsirikuyReturnCode getCurrencyLocations(char* pCurrencyCode, char* pCurrencyLocations);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* SYMBOL_ANALYZER_H_ */
