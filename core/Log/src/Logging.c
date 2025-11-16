/**
#include <stdio.h>
 * @file
 * @brief     Common logging functions used by multiple projects.
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

#include "Precompiled.h"
#include "AsirikuyLogger.h"
#include "Logging.h"
#include "AsirikuyTime.h"
#include "ta_libc.h"
#include "EasyTradeCWrapper.hpp"

#define EQUITY_LOG_FILENAME "ExtendedEntryLog.csv"

static BOOL  gEnableEntryBarLog = FALSE;
static char  gEntryBarLogPath[MAX_FILE_PATH_CHARS] = "";
static int   gBarNumber = 0;

void initExtendedEntryBarLog(BOOL enableEntryBarLog, int barNumber, const char* folderName)
{
  char line[50000] = "";
  char inside[50] = "";
  FILE *fp;
  int i;

  strcat(gEntryBarLogPath, folderName);
  strcat(gEntryBarLogPath, "/");
  strcat(gEntryBarLogPath, EQUITY_LOG_FILENAME);
  gEnableEntryBarLog = enableEntryBarLog;
  gBarNumber = barNumber;

  if(gEnableEntryBarLog)
  {

	fp =fopen(gEntryBarLogPath, "w");

	for (i=0;i<barNumber;i++){

	sprintf(inside,"hour%d,", i);
	strcat(line, inside);
	sprintf(inside,"dayOfMonth%d,", i);
	strcat(line, inside);
	sprintf(inside,"month%d,", i);
	strcat(line, inside);
	sprintf(inside,"dayOfWeek%d,", i);
	strcat(line, inside);
	sprintf(inside,"range%d,", i);
	strcat(line, inside);
	sprintf(inside,"body%d,", i);
	strcat(line, inside);
	sprintf(inside,"closeToHigh%d,", i);
	strcat(line, inside);
	sprintf(inside,"closeToLow%d,", i);
	strcat(line, inside);
	sprintf(inside,"5RSI%d,", i);
	strcat(line, inside);
	sprintf(inside,"10RSI%d,",i);
	strcat(line, inside);
	sprintf(inside,"20RSI%d,", i);
	strcat(line, inside);
	sprintf(inside,"50RSI%d,", i);
	strcat(line, inside);
	sprintf(inside,"5STO%d,", i);
	strcat(line, inside);
	sprintf(inside,"10STO%d,", i);
	strcat(line, inside);
	sprintf(inside,"20STO%d,", i);
	strcat(line, inside);
	sprintf(inside,"50STO%d,", i);
	strcat(line, inside);

	sprintf(inside,"5MA%d,", i);
	strcat(line, inside);
	sprintf(inside,"10MA%d,", i);
	strcat(line, inside);
	sprintf(inside,"20MA%d,", i);
	strcat(line, inside);
	sprintf(inside,"50MA%d,", i);
	strcat(line, inside);
	sprintf(inside,"5CCI%d,",i);
	strcat(line, inside);
	sprintf(inside,"10CCI%d,", i);
	strcat(line, inside);
	sprintf(inside,"20CCI%d,", i);
	strcat(line, inside);
	sprintf(inside,"50CCI%d,", i);
	strcat(line, inside);

	sprintf(inside,"20BB%d,",i);
	strcat(line, inside);
	sprintf(inside,"5Envelopes%d,",i);
	strcat(line, inside);
	sprintf(inside,"10Envelopes%d,",i);
	strcat(line, inside);
	sprintf(inside,"20Envelopes%d,", i);
	strcat(line, inside);

	sprintf(inside,"5MACD%d,", i);
	strcat(line, inside);
	sprintf(inside,"10MACD%d,", i);
	strcat(line, inside);
	sprintf(inside,"20MACD%d,", i);
	strcat(line, inside);
	sprintf(inside,"20ATRPredDiff%d,",i);
	strcat(line, inside);

	}

	fprintf(fp, "%s\n", line);

	fclose(fp);  
  
  }
  else
  {
    logNotice("Entry bar log is not enabled.\n");
  }



}

void recordData(StrategyParams* pParams, int positionType)
{
	FILE *fp;
	char line[50000] = "";
	char inside[50] = "";
	int max_len; 
	char* buff; 
	char* final;
	int  mult, i;
	int shift = 2;
	struct tm timeInfo;
	double atr = iAtr(DAILY_RATES,(int)parameter(ATR_AVERAGING_PERIOD), 1);
	int outBegIdx, outNBElement, shiftDay;
	double notUsed, bbUp, bbDown;
	int  shift0Index      = pParams->ratesBuffers->rates[PRIMARY_RATES].info.arraySize - 1;
	char *last_newline;
	char *last_line;

	
	if(gEnableEntryBarLog == FALSE) return;

	max_len = gBarNumber*550;
	buff = (char*)malloc(max_len * sizeof(char));
	final = (char*)malloc(max_len * sizeof(char));

	if (positionType == BUY) mult = 1;
	if (positionType == SELL) mult = -1;

	if ((fp = fopen(gEntryBarLogPath, "rb")) != NULL)  {      

    fseek(fp, -max_len, SEEK_END);            // set pointer to the end of file minus the length you need. Presumably there can be more than one new line caracter
    fread(buff, max_len-1, 1, fp);            // read the contents of the file starting from where fseek() positioned us
    fclose(fp);                               // close the file

	//buff[max_len-1] = '\0';
    last_newline = strrchr(buff, '\n'); // find last occurrence of newlinw 
    last_line = last_newline+1;         // jump to it

	logDebug("last line in entry bar log = %s", last_line);

	}

	fp =fopen(gEntryBarLogPath, "a");

	for (i=0;i<gBarNumber;i++){

	safe_gmtime(&timeInfo, openTime(shift));
	//logCritical("%d", size);

	sprintf(inside,"%d,", timeInfo.tm_hour);
	strcat(line, inside);
	sprintf(inside,"%d,", timeInfo.tm_mday);
	strcat(line, inside);
	sprintf(inside,"%d,", timeInfo.tm_mon);
	strcat(line, inside);
	sprintf(inside,"%d,", timeInfo.tm_wday);
	strcat(line, inside);
	sprintf(inside,"%lf,", fabs(high(shift) - low(shift))/atr);
	strcat(line, inside);
	sprintf(inside,"%lf,", mult*(cClose(shift) - cOpen(shift))/atr);
	strcat(line, inside);
	sprintf(inside,"%lf,", fabs(cClose(shift) - high(shift))/atr);
	strcat(line, inside);
	sprintf(inside,"%lf,", fabs(cClose(shift) - low(shift))/atr);
	strcat(line, inside);
	sprintf(inside,"%lf,", (iRSI(PRIMARY_RATES,5,shift)-50)*mult);
	strcat(line, inside);
	sprintf(inside,"%lf,", (iRSI(PRIMARY_RATES,10,shift)-50)*mult);
	strcat(line, inside);
	sprintf(inside,"%lf,", (iRSI(PRIMARY_RATES,20,shift)-50)*mult);
	strcat(line, inside);
	sprintf(inside,"%lf,", (iRSI(PRIMARY_RATES,50,shift)-50)*mult);
	strcat(line, inside);
	sprintf(inside,"%lf,", (iSTO(PRIMARY_RATES,5,1,1,0,shift)-50)*mult);
	strcat(line, inside);
	sprintf(inside,"%lf,", (iSTO(PRIMARY_RATES,10,1,1,0,shift)-50)*mult);
	strcat(line, inside);
	sprintf(inside,"%lf,", (iSTO(PRIMARY_RATES,20,1,1,0,shift)-50)*mult);
	strcat(line, inside);
	sprintf(inside,"%lf,", (iSTO(PRIMARY_RATES,50,1,1,0,shift)-50)*mult);
	strcat(line, inside);

	TA_BBANDS(shift0Index-shift, shift0Index-shift, pParams->ratesBuffers->rates[PRIMARY_RATES].close, 20, 2, 2, TA_MAType_SMA, &outBegIdx, &outNBElement, &bbUp, &notUsed, &bbDown);

	sprintf(inside,"%lf,", mult*(cClose(shift)-iMA(3,PRIMARY_RATES,5,shift))/atr);
	strcat(line, inside);
	sprintf(inside,"%lf,", mult*(cClose(shift)-iMA(3,PRIMARY_RATES,10,shift))/atr);
	strcat(line, inside);
	sprintf(inside,"%lf,", mult*(cClose(shift)-iMA(3,PRIMARY_RATES,20,shift))/atr);
	strcat(line, inside);
	sprintf(inside,"%lf,", mult*(cClose(shift)-iMA(3,PRIMARY_RATES,50,shift))/atr);
	strcat(line, inside);
	sprintf(inside,"%lf,", mult*(iCCI(PRIMARY_RATES,5,shift)));
	strcat(line, inside);
	sprintf(inside,"%lf,", mult*(iCCI(PRIMARY_RATES,10,shift)));
	strcat(line, inside);
	sprintf(inside,"%lf,", mult*(iCCI(PRIMARY_RATES,20,shift)));
	strcat(line, inside);
	sprintf(inside,"%lf,", mult*(iCCI(PRIMARY_RATES,50,shift)));
	strcat(line, inside);


	if (positionType == BUY){
	sprintf(inside,"%lf,", (cClose(shift) - bbDown)/atr);
	strcat(line, inside);
	sprintf(inside,"%lf,", mult*(cClose(shift) - (iMA(3,PRIMARY_RATES,5,shift)-0.02*iMA(3,PRIMARY_RATES,5,shift)))/atr);
	strcat(line, inside);
	sprintf(inside,"%lf,", mult*(cClose(shift) - (iMA(3,PRIMARY_RATES,10,shift)-0.02*iMA(3,PRIMARY_RATES,5,shift)))/atr);
	strcat(line, inside);
	sprintf(inside,"%lf,", mult*(cClose(shift) - (iMA(3,PRIMARY_RATES,20,shift)-0.02*iMA(3,PRIMARY_RATES,5,shift)))/atr);
	strcat(line, inside);
	}

	if (positionType == SELL){
	sprintf(inside,"%lf,", mult*(cClose(shift) - bbUp)/atr);
	strcat(line, inside);
	sprintf(inside,"%lf,", mult*(cClose(shift) + (iMA(3,PRIMARY_RATES,5,shift)-0.02*iMA(3,PRIMARY_RATES,5,shift)))/atr);
	strcat(line, inside);
	sprintf(inside,"%lf,", mult*(cClose(shift) + (iMA(3,PRIMARY_RATES,10,shift)-0.02*iMA(3,PRIMARY_RATES,5,shift)))/atr);
	strcat(line, inside);
	sprintf(inside,"%lf,", mult*(cClose(shift) + (iMA(3,PRIMARY_RATES,20,shift)-0.02*iMA(3,PRIMARY_RATES,5,shift)))/atr);
	strcat(line, inside);
	}

	sprintf(inside,"%lf,", mult*(iMACD(PRIMARY_RATES,5,10,6,0,shift)));
	strcat(line, inside);
	sprintf(inside,"%lf,", mult*(iMACD(PRIMARY_RATES,10,20,6,0,shift)));
	strcat(line, inside);
	sprintf(inside,"%lf,", mult*(iMACD(PRIMARY_RATES,20,40,6,0,shift)));
	strcat(line, inside);

	shiftDay = findShift(DAILY_RATES, PRIMARY_RATES, shift);

	sprintf(inside,"%lf,", iAtr(DAILY_RATES,20,shiftDay+2)-(iHigh(DAILY_RATES,shiftDay+1)-iLow(DAILY_RATES,shiftDay+1)));
	strcat(line, inside);

	shift += 1;

	}

	sprintf(final, "%s\n", line);

	logDebug("Line to add to entry bar log = %s", final);

	if (strncmp(final , last_line, 20))
	fprintf(fp, "%s\n", line);

	fclose(fp);

	free(buff);
	free(final);
}

char* asirikuyReturnCodeToString(AsirikuyReturnCode returnCode, char* pBuffer, int bufferLength)
{
  if(pBuffer == NULL)
  {
    logCritical("asirikuyReturnCodeToString() failed. pBuffer = NULL");
    return pBuffer;
  }

  if(bufferLength < MAX_ERROR_STRING_SIZE)
  {
    logError("asirikuyReturnCodeToString() failed. bufferLength < MAX_ERROR_STRING_SIZE");
    return pBuffer;
  }

  pBuffer[0] = '\0';

  switch(returnCode)
  {
  case SUCCESS              : return strncpy(pBuffer, "Success", bufferLength);
  case INVALID_CURRENCY     : return strncpy(pBuffer, "Invalid Currency", bufferLength);
  case UNKNOWN_SYMBOL       : return strncpy(pBuffer, "Unknown Symbol", bufferLength);
  case SYMBOL_TOO_SHORT     : return strncpy(pBuffer, "Symbol too short", bufferLength);
  case PARSE_SYMBOL_FAILED  : return strncpy(pBuffer, "Failed to parse symbol", bufferLength);
  case NO_CONVERSION_SYMBOLS: return strncpy(pBuffer, "No conversion symbols found", bufferLength);
  case NULL_POINTER         : return strncpy(pBuffer, "Null pointer", bufferLength);
  case INVALID_STRATEGY     : return strncpy(pBuffer, "Invalid strategy", bufferLength);
  case NOT_ENOUGH_MARGIN    : return strncpy(pBuffer, "Not enough margin", bufferLength);
  case SPREAD_TOO_WIDE      : return strncpy(pBuffer, "Spread too wide", bufferLength);
  case NOT_ENOUGH_RATES_DATA: return strncpy(pBuffer, "Not enough rates data", bufferLength);
  case WORST_CASE_SCENARIO  : return strncpy(pBuffer, "Worst case scenario", bufferLength);
  case NORMALIZE_BARS_FAILED: return strncpy(pBuffer, "Failed to normalize rates", bufferLength);
  case INIT_LOG_FAILED      : return strncpy(pBuffer, "Failed to initialize log", bufferLength);
  case DEINIT_LOG_FAILED    : return strncpy(pBuffer, "Failed to close log", bufferLength);
  case ZERO_DIVIDE          : return strncpy(pBuffer, "Zero divide", bufferLength);
  case TA_LIB_ERROR         : return strncpy(pBuffer, "TALib error", bufferLength);
  case INVALID_TIME_OFFSET  : return strncpy(pBuffer, "Invalid time offset type", bufferLength);
  case INVALID_PARAMETER    : return strncpy(pBuffer, "Invalid parameter", bufferLength);
  case NN_TRAINING_FAILED   : return strncpy(pBuffer, "Neural net training failed", bufferLength);
  case UNKNOWN_TIMEZONE     : return strncpy(pBuffer, "Missing timezone info", bufferLength);
  case LOCAL_TZ_MISMATCH    : return strncpy(pBuffer, "Local timezone mismatch", bufferLength);
  case BROKER_TZ_MISMATCH   : return strncpy(pBuffer, "Broker timezone mismatch", bufferLength);
  case TOO_MANY_INSTANCES   : return strncpy(pBuffer, "Maximum instances exceeded", bufferLength);
  case INVALID_CONFIG       : return strncpy(pBuffer, "Framework config is invalid", bufferLength);
  case MISSING_CONFIG       : return strncpy(pBuffer, "Can\'t find the config file", bufferLength);
  case INIT_XML_FAILED      : return strncpy(pBuffer, "Failed to init xml parser", bufferLength);
  case UNKNOWN_INSTANCE_ID  : return strncpy(pBuffer, "Unknown instance Id", bufferLength);
  case INSUFFICIENT_MEMORY  : return strncpy(pBuffer, "Not enough RAM available", bufferLength);
  case WAIT_FOR_INIT        : return strncpy(pBuffer, "Not yet initialized", bufferLength);
  default                   : return strncpy(pBuffer, "Invalid return code", bufferLength);
  }
}

char* taRetCodeToString(TA_RetCode retCode, char* pBuffer, int bufferLength)
{
  if(pBuffer == NULL)
  {
    logCritical("taRetCodeToString() failed. pBuffer = NULL");
    return pBuffer;
  }

  if(bufferLength < MAX_ERROR_STRING_SIZE)
  {
    logError("taRetCodeToString() failed. bufferLength < MAX_ERROR_STRING_SIZE");
    return pBuffer;
  }

  pBuffer[0] = '\0';

  switch(retCode)
  {
  case TA_SUCCESS                  : return strncpy(pBuffer, "Success", bufferLength);
  case TA_LIB_NOT_INITIALIZE       : return strncpy(pBuffer, "TA_Initialize failed", bufferLength);
  case TA_BAD_PARAM                : return strncpy(pBuffer, "A parameter is out of range", bufferLength);
  case TA_ALLOC_ERR                : return strncpy(pBuffer, "Possibly out of memory", bufferLength);
  case TA_GROUP_NOT_FOUND          : return strncpy(pBuffer, "Group not found", bufferLength);
  case TA_FUNC_NOT_FOUND           : return strncpy(pBuffer, "Func not found", bufferLength);
  case TA_INVALID_HANDLE           : return strncpy(pBuffer, "Invalid handle", bufferLength);
  case TA_INVALID_PARAM_HOLDER     : return strncpy(pBuffer, "Invalid param holder", bufferLength);
  case TA_INVALID_PARAM_HOLDER_TYPE: return strncpy(pBuffer, "Invalid param holder type", bufferLength);
  case TA_INVALID_PARAM_FUNCTION   : return strncpy(pBuffer, "Invalid param function", bufferLength);
  case TA_INPUT_NOT_ALL_INITIALIZE : return strncpy(pBuffer, "Input not all initialized", bufferLength);
  case TA_OUTPUT_NOT_ALL_INITIALIZE: return strncpy(pBuffer, "Output not all initialized", bufferLength);
  case TA_OUT_OF_RANGE_START_INDEX : return strncpy(pBuffer, "Out of range start index", bufferLength);
  case TA_INVALID_LIST_TYPE        : return strncpy(pBuffer, "Invalid list type", bufferLength);
  case TA_BAD_OBJECT               : return strncpy(pBuffer, "Bad object", bufferLength);
  case TA_NOT_SUPPORTED            : return strncpy(pBuffer, "Not supported", bufferLength);
  case TA_INTERNAL_ERROR           : return strncpy(pBuffer, "Internal error", bufferLength);
  case TA_UNKNOWN_ERR              : return strncpy(pBuffer, "Unknown error", bufferLength);
  default                          : return strncpy(pBuffer, "Invalid return code", bufferLength);
  }
}

AsirikuyReturnCode logAsirikuyError(const char* pFunctionName, AsirikuyReturnCode returnCode)
{
  char errorCodeBuf[MAX_ERROR_STRING_SIZE] = "";
  char errorBuf[MAX_ERROR_STRING_SIZE] = "";

  if(pFunctionName == NULL)
  {
    logCritical("logAsirikuyError() failed. pFunctionName = NULL\n");
    return NULL_POINTER;
  }

  strcpy(errorCodeBuf, asirikuyReturnCodeToString(returnCode, errorCodeBuf, MAX_ERROR_STRING_SIZE));
  strcpy(errorBuf, pFunctionName);
  strcat(errorBuf, errorCodeBuf);
  strcat(errorBuf, " ");
  logError("%s\n", errorBuf);

  return returnCode;
}

AsirikuyReturnCode logTALibError(const char* pFunctionName, TA_RetCode code)
{
  char errorCodeBuf[MAX_ERROR_STRING_SIZE] = "";
  char errorBuf[MAX_ERROR_STRING_SIZE] = "";

  if(pFunctionName == NULL)
  {
    logCritical("logTALibError() failed. pFunctionName = NULL\n");
    return NULL_POINTER;
  }

  strcpy(errorCodeBuf, taRetCodeToString(code, errorCodeBuf, MAX_ERROR_STRING_SIZE));
  strcpy(errorBuf, pFunctionName);
  strcat(errorBuf, errorCodeBuf);
  strcat(errorBuf, " ");
  logError("%s\n", errorBuf);

  return TA_LIB_ERROR;
}
