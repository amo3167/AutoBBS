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

#include "Precompiled.h"

#include <float.h>
#include <math.h>
#include <ta_libc.h>

#include "AsirikuyTime.h"
#include "EasyTrade.hpp"
#include "AsirikuyStrategies.h"
#include "StrategyUserInterface.h"
#include "InstanceStates.h"
#include "Logging.h"
#include "AsirikuyLogger.h"
#include "OrderManagement.h"
#include "OrderSignals.h"
#include "AsirikuyTechnicalAnalysis.h"
#include "tradingweekboundaries.h"
#include "curl/curl.h"
#include "TimeZoneOffsets.h"
#include "Broker-tz.h"

#define STOPS_REFERENCE_POINTS      5000
#define INDICATOR_CALCULATION_ERROR -1
#define SELECT_ALL_TRADES			      -1
#define DAILY_RATES                 1 // TODO: iterate to find the daily rates index instead of assuming its on index 1.


size_t my_write_func(void *ptr, size_t size, size_t nmemb, FILE *stream)
{
    return fwrite(ptr, size, nmemb, stream);
}

int countLinesInCSV(char* fileName)
{
	FILE                *fp;
    int                 c;             
    unsigned long       newline_count = 0;

        /* count the newline characters */

	fp = fopen(fileName, "r" );

	if(fp == NULL){
			 logEmergency("NO_FILE!");
			return 0;
	   }

    while ( (c=fgetc(fp)) != EOF ) {
        if ( c == '\n' )
            newline_count++;
    }

    fclose(fp);

	return newline_count;
}

AsirikuyReturnCode freeTickData(tickData loadedData)
{
	free(loadedData.time);
	free(loadedData.bid);
	free(loadedData.ask);

	return SUCCESS;
}


EasyTrade::EasyTrade()
{

}

EasyTrade::~EasyTrade()
{

}

StrategyParams* EasyTrade::getParams()
{
  return(pParams);
}

int EasyTrade::totalOrdersCount()
{
  return (totalOpenOrders(pParams, BUY) + totalOpenOrders(pParams, SELL));
}

int EasyTrade::buyOrdersCount()
{
  return totalOpenOrders(pParams, BUY);
}

double r2()
{
    return ((rand() % 101 + (-100))*0.01) ;
}

int EasyTrade::getHourShift(int rateIndex, int shift){

	time_t dayTime = (time_t)(iOpenTime(rateIndex, shift) % SECONDS_PER_DAY);
	int hourValue = (int)(dayTime/ SECONDS_PER_HOUR);
	return (hourValue);
}

double EasyTrade::iAtrWholeDaysSimpleShift(int rateIndex, int periodATR, int shift)
{
  double average;
  int i;
  double highValue, lowValue;
  int shift0Index = pParams->ratesBuffers->rates[rateIndex].info.arraySize - 1 - shift;

  average = 0;

  for (i=0; i<periodATR; i++)
  {
    highValue   =  pParams->ratesBuffers->rates[rateIndex].high[shift0Index-(int)(i*((1440/2)/Period())+(1440/Period()))];
   lowValue    =  pParams->ratesBuffers->rates[rateIndex].low[shift0Index-(int)(i*((1440/2)/Period())+1)];
    average +=  fabs(highValue-lowValue)/periodATR ; 
  }

  return average;
}

int EasyTrade::sellOrdersCount()
{
  return totalOpenOrders(pParams, SELL);
}

AsirikuyReturnCode EasyTrade::addDistortedArray(int sourceRateIndex, int ratesIndex, double maxDistorsion){

	int arraySize = pParams->ratesBuffers->rates[sourceRateIndex].info.arraySize;
	int i = 0;

    pParams->ratesBuffers->rates[ratesIndex].info.arraySize = arraySize;
	pParams->ratesBuffers->rates[ratesIndex].time   =  (time_t*)malloc(arraySize * sizeof(time_t));
	pParams->ratesBuffers->rates[ratesIndex].open   =  (double*)malloc(arraySize * sizeof(double));
	pParams->ratesBuffers->rates[ratesIndex].high   =  (double*)malloc(arraySize * sizeof(double));
	pParams->ratesBuffers->rates[ratesIndex].low    =  (double*)malloc(arraySize * sizeof(double));
	pParams->ratesBuffers->rates[ratesIndex].close  =  (double*)malloc(arraySize * sizeof(double));
	pParams->ratesBuffers->rates[ratesIndex].volume =  (double*)malloc(arraySize * sizeof(double));

	for(i=0;i<arraySize;i++){
		pParams->ratesBuffers->rates[ratesIndex].time[i]   =  pParams->ratesBuffers->rates[sourceRateIndex].time[i]  ;
		pParams->ratesBuffers->rates[ratesIndex].open[i]   =  pParams->ratesBuffers->rates[sourceRateIndex].open[i] + r2()*maxDistorsion;
		pParams->ratesBuffers->rates[ratesIndex].high[i]   =  pParams->ratesBuffers->rates[sourceRateIndex].high[i] + r2()*maxDistorsion;
		pParams->ratesBuffers->rates[ratesIndex].low[i]    =  pParams->ratesBuffers->rates[sourceRateIndex].low[i]  + r2()*maxDistorsion;
		pParams->ratesBuffers->rates[ratesIndex].close[i]  =  pParams->ratesBuffers->rates[sourceRateIndex].close[i]+ r2()*maxDistorsion ;
		pParams->ratesBuffers->rates[ratesIndex].volume[i] =  pParams->ratesBuffers->rates[sourceRateIndex].volume[i];

		if (pParams->ratesBuffers->rates[ratesIndex].open[i] > pParams->ratesBuffers->rates[ratesIndex].high[i])
			pParams->ratesBuffers->rates[ratesIndex].high[i] = pParams->ratesBuffers->rates[ratesIndex].open[i];

		if (pParams->ratesBuffers->rates[ratesIndex].close[i] > pParams->ratesBuffers->rates[ratesIndex].high[i])
			pParams->ratesBuffers->rates[ratesIndex].high[i] = pParams->ratesBuffers->rates[ratesIndex].close[i];

		if (pParams->ratesBuffers->rates[ratesIndex].open[i] < pParams->ratesBuffers->rates[ratesIndex].low[i])
			pParams->ratesBuffers->rates[ratesIndex].low[i] = pParams->ratesBuffers->rates[ratesIndex].open[i];

		if (pParams->ratesBuffers->rates[ratesIndex].close[i] < pParams->ratesBuffers->rates[ratesIndex].low[i])
			pParams->ratesBuffers->rates[ratesIndex].low[i] = pParams->ratesBuffers->rates[ratesIndex].close[i];

	}

	return SUCCESS;

}

AsirikuyReturnCode EasyTrade::freeRates(int ratesIndex)
{
   free(pParams->ratesBuffers->rates[ratesIndex].time);
   free(pParams->ratesBuffers->rates[ratesIndex].open);
   free(pParams->ratesBuffers->rates[ratesIndex].high);
   free(pParams->ratesBuffers->rates[ratesIndex].low);
   free(pParams->ratesBuffers->rates[ratesIndex].close);
   free(pParams->ratesBuffers->rates[ratesIndex].volume);

   return SUCCESS;
}

AsirikuyReturnCode EasyTrade::saveTickData()
{
   char buffer[MAX_FILE_PATH_CHARS] = "";
   char tempPath[MAX_FILE_PATH_CHARS] = "";
   FILE *fp;
   tickData currentTickData;
   int lastTickTime=0;
   int i=0;

   currentTickData = addTickArray();

   requestTempFileFolderPath(tempPath);
   sprintf(buffer, "%s%d_%s.csv", tempPath, (int)pParams->settings[STRATEGY_INSTANCE_ID], pParams->tradeSymbol);

   if (currentTickData.arraySize > 0){
	   lastTickTime = (int)currentTickData.time[currentTickData.arraySize-1];

	   // if the tick array contains future data, start from scratch 
	   // this may happen if we start a back-test with a previously created file
	   if(lastTickTime >= (int)pParams->currentBrokerTime){

			fp = fopen(buffer,"w\n");
			if(fp == NULL){
				logInfo("Could not write tick data file.");
				freeTickData(currentTickData);
				return FILE_WRITING_ERROR;
			}

			fprintf(fp,"\n");
			fclose(fp);
		}
   }

   // if we pass the storage limit then rewrite file starting from second item
   if(currentTickData.arraySize >= TICK_DATA_STORAGE_LIMIT){

	   fp = fopen(buffer,"w\n");
	   if(fp == NULL){
			logInfo("Could not write tick data file.");
			freeTickData(currentTickData);
			return FILE_WRITING_ERROR;
	   }

	   for(i=1;i<currentTickData.arraySize;i++){
			fprintf(fp,"%d,%lf,%lf\n",currentTickData.time[i], currentTickData.bid[i], currentTickData.ask[i]);
	   }

	   fclose(fp);
   }

  fp = fopen(buffer,"a\n");
  if(fp == NULL){
    logInfo("Could not write tick data file.");
	freeTickData(currentTickData);
    return FILE_WRITING_ERROR;
  }

  fseek(fp,0, SEEK_END);

  if((int)pParams->currentBrokerTime > lastTickTime){
	fprintf(fp,"%d,%lf,%lf\n",(int)pParams->currentBrokerTime, pParams->bidAsk.bid[0], pParams->bidAsk.ask[0]);
  }

  fclose(fp);

  freeTickData(currentTickData);

  return SUCCESS;
}

tickData EasyTrade::addTickArray()
{
  char buffer[MAX_FILE_PATH_CHARS] = "";
  char tempPath[MAX_FILE_PATH_CHARS] = "";
  char data[200] = "";
  int arraySize;
  tickData loadedTickData;
  int i;
  char *ptr;
  FILE *fp;
  char *strtokSave;


  requestTempFileFolderPath(tempPath);
  sprintf(buffer, "%s%d_%s.csv", tempPath,  (int)pParams->settings[STRATEGY_INSTANCE_ID], pParams->tradeSymbol);
  loadedTickData.arraySize = 0;

  arraySize = countLinesInCSV(buffer);

  loadedTickData.arraySize = arraySize;
  loadedTickData.time  =  (int*)malloc(arraySize * sizeof(int));
  loadedTickData.bid   =  (double*)malloc(arraySize * sizeof(double));
  loadedTickData.ask   =  (double*)malloc(arraySize * sizeof(double));

  i = 0;

  fp = fopen(buffer,"r\n");
  if(fp == NULL){
    logInfo("No tick data present yet.");
    return loadedTickData;
  }

    while ( fgets(data, 200, fp ) != NULL) {
	
        ptr = strtok_r(data, ",", &strtokSave);
		sscanf(ptr,"%d",&loadedTickData.time[i]);
		ptr = strtok_r(NULL, ",", &strtokSave);
		sscanf(ptr,"%lf",&loadedTickData.bid[i]);
		ptr = strtok_r(NULL, ",", &strtokSave);
		sscanf(ptr,"%lf",&loadedTickData.ask[i]);
		i++;
    }


    fclose(fp);

	return loadedTickData;

}

AsirikuyReturnCode EasyTrade::addNewDailyRates(char* ratesName, time_t intFromDate, int ratesIndex)
{
	char url[250] = "";
	char str[20] = "";
	char data[100] = "";
	char date[20] = "";
	char *ptr;
	char *timeString;
	struct tm finalDate;
	struct tm fromDate;
	CURL *curl;
	FILE *fp;
	CURLcode res;
	int i;
	struct tm lDate;
	char outfilename[250];
	int arraySize;
	char *strtokSave;
	safe_gmtime(&finalDate, pParams->currentBrokerTime);
	safe_gmtime(&fromDate, intFromDate);

	sprintf(outfilename, "%s_%d.csv", ratesName, pParams->settings[STRATEGY_INSTANCE_ID]);

	strcat(url, "http://ichart.yahoo.com/table.csv?s=\n");
	strcat(url, ratesName);

	strcat(url, "&a=\n");
	sprintf(str,"%d",fromDate.tm_mon+1);
	strcat(url, str);

	strcat(url, "&b=\n");
	sprintf(str,"%d",fromDate.tm_mday);
	strcat(url, str);

	strcat(url, "&c=\n");
	sprintf(str,"%d",fromDate.tm_year+1900);
	strcat(url, str);

	strcat(url, "&d=\n");
	sprintf(str,"%d",finalDate.tm_mon+1);
	strcat(url, str);

	strcat(url, "&e=\n");
	sprintf(str,"%d",finalDate.tm_mday);
	strcat(url, str);

	strcat(url, "&f=\n");
	sprintf(str,"%d",finalDate.tm_year+1900);
	strcat(url, str);

	strcat(url, "&ignore=.csv\n");

	logDebug("URL To parse for rate addition of symbol %s: %s", ratesName, url);

	curl_global_init(CURL_GLOBAL_ALL);
    curl = curl_easy_init();

    if (curl) {
        fp = fopen(outfilename,"w\n");
        curl_easy_setopt(curl, CURLOPT_URL, url);
		curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, fp);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, my_write_func);
        res = curl_easy_perform(curl);

		if(res != CURLE_OK){
		logError("curl_easy_perform() failed while getting rates: %s", curl_easy_strerror(res));
		return ERROR_IN_RATES_RETRIEVAL;
		}
		/* always cleanup */

        curl_easy_cleanup(curl);
		curl_global_cleanup();
        fclose(fp);
    }

	arraySize = countLinesInCSV(outfilename)-1;
	pParams->ratesBuffers->rates[ratesIndex].info.arraySize = arraySize;
	pParams->ratesBuffers->rates[ratesIndex].time   =  (time_t*)malloc(arraySize * sizeof(time_t));
	pParams->ratesBuffers->rates[ratesIndex].open   =  (double*)malloc(arraySize * sizeof(double));
	pParams->ratesBuffers->rates[ratesIndex].high   =  (double*)malloc(arraySize * sizeof(double));
	pParams->ratesBuffers->rates[ratesIndex].low    =  (double*)malloc(arraySize * sizeof(double));
	pParams->ratesBuffers->rates[ratesIndex].close  =  (double*)malloc(arraySize * sizeof(double));
	pParams->ratesBuffers->rates[ratesIndex].volume =  (double*)malloc(arraySize * sizeof(double));

	i = 0;

	fp = fopen( outfilename, "r+" );
    while ( fgets(data, 100, fp ) != NULL) {
	
		if (i > 0){

        timeString = strtok_r(data, ",", &strtokSave);
		ptr = strtok_r(NULL, ",", &strtokSave);
		sscanf(ptr,"%lf",&pParams->ratesBuffers->rates[ratesIndex].open[arraySize-i]);
		ptr = strtok_r(NULL, ",", &strtokSave);
		sscanf(ptr,"%lf",&pParams->ratesBuffers->rates[ratesIndex].high[arraySize-i]);
		ptr = strtok_r(NULL, ",", &strtokSave);
		sscanf(ptr,"%lf",&pParams->ratesBuffers->rates[ratesIndex].low[arraySize-i]);
		ptr = strtok_r(NULL, ",", &strtokSave);
		sscanf(ptr,"%lf",&pParams->ratesBuffers->rates[ratesIndex].close[arraySize-i]);
		ptr = strtok_r(NULL, ",", &strtokSave);
		sscanf(ptr,"%lf",&pParams->ratesBuffers->rates[ratesIndex].volume[arraySize-i]);

		if (i == 1){
			pParams->ratesBuffers->rates[ratesIndex].high[arraySize-i] = pParams->ratesBuffers->rates[ratesIndex].open[arraySize-i];
			pParams->ratesBuffers->rates[ratesIndex].low[arraySize-i] = pParams->ratesBuffers->rates[ratesIndex].open[arraySize-i];
			pParams->ratesBuffers->rates[ratesIndex].close[arraySize-i] = pParams->ratesBuffers->rates[ratesIndex].open[arraySize-i];
			pParams->ratesBuffers->rates[ratesIndex].volume[arraySize-i] = 1;
		}

		ptr = strtok_r(timeString, "-", &strtokSave);
		sscanf(ptr,"%d",&lDate.tm_year);
		ptr = strtok_r(NULL, "-", &strtokSave);
		sscanf(ptr,"%d",&lDate.tm_mon);
		ptr = strtok_r(NULL, "-", &strtokSave);
		sscanf(ptr,"%d",&lDate.tm_mday);

		lDate.tm_year -= 1900;
		lDate.tm_mon -= 1;
		lDate.tm_sec = 0;  
		lDate.tm_min = 0;  
		lDate.tm_hour = 0;    

		pParams->ratesBuffers->rates[ratesIndex].time[arraySize-i] = mktime(&lDate);
		strftime(date, 20, "%d/%m/%Y %H:%M:%S", localtime(&pParams->ratesBuffers->rates[ratesIndex].time[arraySize-i]));

		logDebug("Rate item addition -> %s, %lf, %lf, %lf, %lf", date, pParams->ratesBuffers->rates[ratesIndex].open[arraySize-i], pParams->ratesBuffers->rates[ratesIndex].high[arraySize-i], pParams->ratesBuffers->rates[ratesIndex].low[arraySize-i], pParams->ratesBuffers->rates[ratesIndex].close[arraySize-i]);

		}

		i++;
    }

    fclose(fp);

	if ((int)(pParams->currentBrokerTime)-(int)iOpenTime(ratesIndex,0) > SECONDS_PER_DAY){

		for(i=0;i<arraySize-2;i++){
			pParams->ratesBuffers->rates[ratesIndex].time[i]   =  pParams->ratesBuffers->rates[ratesIndex].time[i+1] ;
			pParams->ratesBuffers->rates[ratesIndex].open[i]   =  pParams->ratesBuffers->rates[ratesIndex].open[i+1] ;
			pParams->ratesBuffers->rates[ratesIndex].high[i]   =  pParams->ratesBuffers->rates[ratesIndex].high[i+1] ;
			pParams->ratesBuffers->rates[ratesIndex].low[i]    =  pParams->ratesBuffers->rates[ratesIndex].low[i+1] ;
			pParams->ratesBuffers->rates[ratesIndex].close[i]  =  pParams->ratesBuffers->rates[ratesIndex].close[i+1] ;
			pParams->ratesBuffers->rates[ratesIndex].volume[i] =  pParams->ratesBuffers->rates[ratesIndex].volume[i+1] ;
		}
	}

	pParams->ratesBuffers->rates[ratesIndex].time[arraySize-1] = iOpenTime(0,0);
	pParams->ratesBuffers->rates[ratesIndex].open[arraySize-1] = 0;
	pParams->ratesBuffers->rates[ratesIndex].high[arraySize-1] = 0;
	pParams->ratesBuffers->rates[ratesIndex].low[arraySize-1] = 0;
	pParams->ratesBuffers->rates[ratesIndex].close[arraySize-1] = 0;
	pParams->ratesBuffers->rates[ratesIndex].volume[arraySize-1] = 1;

	return SUCCESS;
}

AsirikuyReturnCode EasyTrade::addNewDailyRatesQuandl(char* token, char* dataset, char* ratesName, time_t intFromDate, int ratesIndex)
{
	char url[250] = "";
	char str[20] = "";
	char data[100] = "";
	char date[20] = "";
	char *ptr;
	char *timeString;
	struct tm finalDate;
	struct tm fromDate;
	CURL *curl;
	FILE *fp;
	CURLcode res;
	int i;
	struct tm lDate;
	char outfilename[250];
	int arraySize;
	char *strtokSave;

	safe_gmtime(&finalDate, pParams->currentBrokerTime);
	safe_gmtime(&fromDate, intFromDate);

	sprintf(outfilename, "%s_%d.csv", ratesName, pParams->settings[STRATEGY_INSTANCE_ID]);

	strcat(url, "http://www.quandl.com/api/v1/datasets/\n");
	strcat(url, dataset);
	strcat(url, "/\n");
	strcat(url, ratesName);

	strcat(url, ".csv?auth_token=\n");
	strcat(url, token);
	strcat(url, "&trim_start=\n");
	sprintf(str,"%d-%d-%d",fromDate.tm_mday, fromDate.tm_mon+1, fromDate.tm_year+1900);
	strcat(url, str);

	strcat(url, "&trim_end=\n");
	sprintf(str,"%d-%d-%d",finalDate.tm_mday, finalDate.tm_mon+1, finalDate.tm_year+1900);
	strcat(url, str);

	logDebug("URL To parse for rate addition of symbol %s: %s", ratesName, url);

	curl_global_init(CURL_GLOBAL_ALL);
    curl = curl_easy_init();

    if (curl) {
        fp = fopen(outfilename,"w\n");
        curl_easy_setopt(curl, CURLOPT_URL, url);
		curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, fp);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, my_write_func);
        res = curl_easy_perform(curl);

		if(res != CURLE_OK){
		logError("curl_easy_perform() failed while getting rates: %s", curl_easy_strerror(res));
		return ERROR_IN_RATES_RETRIEVAL;
		}
		/* always cleanup */

        curl_easy_cleanup(curl);
		curl_global_cleanup();
        fclose(fp);
    }

	arraySize = countLinesInCSV(outfilename)-1;
	pParams->ratesBuffers->rates[ratesIndex].info.arraySize = arraySize;
	pParams->ratesBuffers->rates[ratesIndex].time   =  (time_t*)malloc(arraySize * sizeof(time_t));
	pParams->ratesBuffers->rates[ratesIndex].open   =  (double*)malloc(arraySize * sizeof(double));
	pParams->ratesBuffers->rates[ratesIndex].high   =  (double*)malloc(arraySize * sizeof(double));
	pParams->ratesBuffers->rates[ratesIndex].low    =  (double*)malloc(arraySize * sizeof(double));
	pParams->ratesBuffers->rates[ratesIndex].close  =  (double*)malloc(arraySize * sizeof(double));
	pParams->ratesBuffers->rates[ratesIndex].volume =  (double*)malloc(arraySize * sizeof(double));

	i = 0;

	fp = fopen( outfilename, "r+" );
    while ( fgets(data, 100, fp ) != NULL) {
	
		if (i > 0){

        timeString = strtok_r(data, ",", &strtokSave);

		ptr = strtok_r(NULL, ",", &strtokSave);
		sscanf(ptr,"%lf",&pParams->ratesBuffers->rates[ratesIndex].open[arraySize-i]);
		ptr = strtok_r(NULL, ",", &strtokSave);
		sscanf(ptr,"%lf",&pParams->ratesBuffers->rates[ratesIndex].high[arraySize-i]);
		ptr = strtok_r(NULL, ",", &strtokSave);
		sscanf(ptr,"%lf",&pParams->ratesBuffers->rates[ratesIndex].low[arraySize-i]);
		ptr = strtok_r(NULL, ",", &strtokSave);
		sscanf(ptr,"%lf",&pParams->ratesBuffers->rates[ratesIndex].close[arraySize-i]);
		ptr = strtok_r(NULL, ",", &strtokSave);
		sscanf(ptr,"%lf",&pParams->ratesBuffers->rates[ratesIndex].volume[arraySize-i]);

		if (i == 1){
			pParams->ratesBuffers->rates[ratesIndex].high[arraySize-i] = pParams->ratesBuffers->rates[ratesIndex].open[arraySize-i];
			pParams->ratesBuffers->rates[ratesIndex].low[arraySize-i] = pParams->ratesBuffers->rates[ratesIndex].open[arraySize-i];
			pParams->ratesBuffers->rates[ratesIndex].close[arraySize-i] = pParams->ratesBuffers->rates[ratesIndex].open[arraySize-i];
			pParams->ratesBuffers->rates[ratesIndex].volume[arraySize-i] = 1;
		}

		ptr = strtok_r(timeString, "-", &strtokSave);
		sscanf(ptr,"%d",&lDate.tm_year);
		ptr = strtok_r(NULL, "-", &strtokSave);
		sscanf(ptr,"%d",&lDate.tm_mon);
		ptr = strtok_r(NULL, "-", &strtokSave);
		sscanf(ptr,"%d",&lDate.tm_mday);

		lDate.tm_year -= 1900;
		lDate.tm_mon -= 1;
		lDate.tm_sec = 0;  
		lDate.tm_min = 0;  
		lDate.tm_hour = 0;    

		pParams->ratesBuffers->rates[ratesIndex].time[arraySize-i] = mktime(&lDate);
		strftime(date, 20, "%d/%m/%Y %H:%M:%S", localtime(&pParams->ratesBuffers->rates[ratesIndex].time[arraySize-i]));

		logDebug("Rate item addition -> %s, %lf, %lf, %lf, %lf", date, pParams->ratesBuffers->rates[ratesIndex].open[arraySize-i], pParams->ratesBuffers->rates[ratesIndex].high[arraySize-i], pParams->ratesBuffers->rates[ratesIndex].low[arraySize-i], pParams->ratesBuffers->rates[ratesIndex].close[arraySize-i]);

		}

		i++;
    }

	if ((int)(pParams->currentBrokerTime)-(int)iOpenTime(ratesIndex,0) > SECONDS_PER_DAY){

		for(i=0;i<arraySize-2;i++){
			pParams->ratesBuffers->rates[ratesIndex].time[i]   =  pParams->ratesBuffers->rates[ratesIndex].time[i+1] ;
			pParams->ratesBuffers->rates[ratesIndex].open[i]   =  pParams->ratesBuffers->rates[ratesIndex].open[i+1] ;
			pParams->ratesBuffers->rates[ratesIndex].high[i]   =  pParams->ratesBuffers->rates[ratesIndex].high[i+1] ;
			pParams->ratesBuffers->rates[ratesIndex].low[i]    =  pParams->ratesBuffers->rates[ratesIndex].low[i+1] ;
			pParams->ratesBuffers->rates[ratesIndex].close[i]  =  pParams->ratesBuffers->rates[ratesIndex].close[i+1] ;
			pParams->ratesBuffers->rates[ratesIndex].volume[i] =  pParams->ratesBuffers->rates[ratesIndex].volume[i+1] ;
		}
	}
	
	pParams->ratesBuffers->rates[ratesIndex].time[arraySize-1] = iOpenTime(0,0);
	pParams->ratesBuffers->rates[ratesIndex].open[arraySize-1] = 0;
	pParams->ratesBuffers->rates[ratesIndex].high[arraySize-1] = 0;
	pParams->ratesBuffers->rates[ratesIndex].low[arraySize-1] = 0;
	pParams->ratesBuffers->rates[ratesIndex].close[arraySize-1] = 0;
	pParams->ratesBuffers->rates[ratesIndex].volume[arraySize-1] = 1;

    fclose(fp);

	return SUCCESS;
}


AsirikuyReturnCode EasyTrade::addNewDailyRatesQuandlOpenOnly(char* token, char* dataset, char* ratesName, time_t intFromDate, int ratesIndex)
{
	char url[250] = "";
	char str[20] = "";
	char data[100] = "";
	char date[20] = "";
	char *ptr;
	char *timeString;
	struct tm finalDate;
	struct tm fromDate;
	CURL *curl;
	FILE *fp;
	CURLcode res;
	int i;
	struct tm lDate;
	char outfilename[250];
	int arraySize;
	char *strtokSave;

	safe_gmtime(&finalDate, pParams->currentBrokerTime);
	safe_gmtime(&fromDate, intFromDate);

	sprintf(outfilename, "%s_%d.csv", ratesName, pParams->settings[STRATEGY_INSTANCE_ID]);

	strcat(url, "http://www.quandl.com/api/v1/datasets/\n");
	strcat(url, dataset);
	strcat(url, "/\n");
	strcat(url, ratesName);

	strcat(url, ".csv?auth_token=\n");
	strcat(url, token);
	strcat(url, "&trim_start=\n");
	sprintf(str,"%d-%d-%d",fromDate.tm_mday, fromDate.tm_mon+1, fromDate.tm_year+1900);
	strcat(url, str);

	strcat(url, "&trim_end=\n");
	sprintf(str,"%d-%d-%d",finalDate.tm_mday, finalDate.tm_mon+1, finalDate.tm_year+1900);
	strcat(url, str);

	logDebug("URL To parse for rate addition of symbol %s: %s", ratesName, url);

	curl_global_init(CURL_GLOBAL_ALL);
    curl = curl_easy_init();

    if (curl) {
        fp = fopen(outfilename,"w\n");
        curl_easy_setopt(curl, CURLOPT_URL, url);
		curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, fp);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, my_write_func);
        res = curl_easy_perform(curl);

		if(res != CURLE_OK){
		logError("curl_easy_perform() failed while getting rates: %s", curl_easy_strerror(res));
		return ERROR_IN_RATES_RETRIEVAL;
		}
		/* always cleanup */

        curl_easy_cleanup(curl);
		curl_global_cleanup();
        fclose(fp);
    }

	arraySize = countLinesInCSV(outfilename)-1;
	pParams->ratesBuffers->rates[ratesIndex].info.arraySize = arraySize;
	pParams->ratesBuffers->rates[ratesIndex].time   =  (time_t*)malloc(arraySize * sizeof(time_t));
	pParams->ratesBuffers->rates[ratesIndex].open   =  (double*)malloc(arraySize * sizeof(double));
	pParams->ratesBuffers->rates[ratesIndex].high   =  (double*)malloc(arraySize * sizeof(double));
	pParams->ratesBuffers->rates[ratesIndex].low    =  (double*)malloc(arraySize * sizeof(double));
	pParams->ratesBuffers->rates[ratesIndex].close  =  (double*)malloc(arraySize * sizeof(double));
	pParams->ratesBuffers->rates[ratesIndex].volume =  (double*)malloc(arraySize * sizeof(double));

	i = 0;

	fp = fopen( outfilename, "r+" );
    while ( fgets(data, 100, fp ) != NULL) {
	
		if (i > 0){

        timeString = strtok_r(data, ",", &strtokSave);
		ptr = strtok_r(NULL, ",", &strtokSave);
		sscanf(ptr,"%lf",&pParams->ratesBuffers->rates[ratesIndex].open[arraySize-i]);
		
		pParams->ratesBuffers->rates[ratesIndex].high[arraySize-i] = pParams->ratesBuffers->rates[ratesIndex].open[arraySize-i];
		pParams->ratesBuffers->rates[ratesIndex].low[arraySize-i] = pParams->ratesBuffers->rates[ratesIndex].open[arraySize-i];
		pParams->ratesBuffers->rates[ratesIndex].close[arraySize-i] = pParams->ratesBuffers->rates[ratesIndex].open[arraySize-i];
		pParams->ratesBuffers->rates[ratesIndex].volume[arraySize-i] = 1;

		ptr = strtok_r(timeString, "-", &strtokSave);
		sscanf(ptr,"%d",&lDate.tm_year);
		ptr = strtok_r(NULL, "-", &strtokSave);
		sscanf(ptr,"%d",&lDate.tm_mon);
		ptr = strtok_r(NULL, "-", &strtokSave);
		sscanf(ptr,"%d",&lDate.tm_mday);

		lDate.tm_year -= 1900;
		lDate.tm_mon -= 1;
		lDate.tm_sec = 0;  
		lDate.tm_min = 0;  
		lDate.tm_hour = 0;    

		pParams->ratesBuffers->rates[ratesIndex].time[arraySize-i] = mktime(&lDate);
		strftime(date, 20, "%d/%m/%Y %H:%M:%S", localtime(&pParams->ratesBuffers->rates[ratesIndex].time[arraySize-i]));

		logDebug("Rate item addition -> %s, %lf, %lf, %lf, %lf", date, pParams->ratesBuffers->rates[ratesIndex].open[arraySize-i], pParams->ratesBuffers->rates[ratesIndex].high[arraySize-i], pParams->ratesBuffers->rates[ratesIndex].low[arraySize-i], pParams->ratesBuffers->rates[ratesIndex].close[arraySize-i]);

		}

		i++;
    }

	if ((int)(pParams->currentBrokerTime)-(int)iOpenTime(ratesIndex,0) > SECONDS_PER_DAY){

		for(i=0;i<arraySize-2;i++){
			pParams->ratesBuffers->rates[ratesIndex].time[i]   =  pParams->ratesBuffers->rates[ratesIndex].time[i+1] ;
			pParams->ratesBuffers->rates[ratesIndex].open[i]   =  pParams->ratesBuffers->rates[ratesIndex].open[i+1] ;
			pParams->ratesBuffers->rates[ratesIndex].high[i]   =  pParams->ratesBuffers->rates[ratesIndex].high[i+1] ;
			pParams->ratesBuffers->rates[ratesIndex].low[i]    =  pParams->ratesBuffers->rates[ratesIndex].low[i+1] ;
			pParams->ratesBuffers->rates[ratesIndex].close[i]  =  pParams->ratesBuffers->rates[ratesIndex].close[i+1] ;
			pParams->ratesBuffers->rates[ratesIndex].volume[i] =  pParams->ratesBuffers->rates[ratesIndex].volume[i+1] ;
		}
	}

	pParams->ratesBuffers->rates[ratesIndex].time[arraySize-1] = iOpenTime(0,0);
	pParams->ratesBuffers->rates[ratesIndex].open[arraySize-1] = 0;
	pParams->ratesBuffers->rates[ratesIndex].high[arraySize-1] = 0;
	pParams->ratesBuffers->rates[ratesIndex].low[arraySize-1] = 0;
	pParams->ratesBuffers->rates[ratesIndex].close[arraySize-1] = 0;
	pParams->ratesBuffers->rates[ratesIndex].volume[arraySize-1] = 1;

    fclose(fp);

	return SUCCESS;
}

AsirikuyReturnCode EasyTrade::addNewRenkoRates(int originalRatesIndex, int ratesIndex, double renkoSize)
{

	int arraySize, i, newCandle;
	double cumulativeUp, cumulativeDown, cumulativeVolume;
	double currentOpen, lowest, highest;
	time_t currentTime;

    arraySize = pParams->ratesBuffers->rates[originalRatesIndex].info.arraySize;

	pParams->ratesBuffers->rates[ratesIndex].info.arraySize = 0 ;

	cumulativeUp = 0;
	cumulativeDown = 0;
	newCandle = TRUE;
	cumulativeVolume = 0;

	for(i=0;i<arraySize;i++)
	{

		if (newCandle == TRUE)
		{
			if (pParams->ratesBuffers->rates[ratesIndex].info.arraySize == 0) 
			currentOpen = pParams->ratesBuffers->rates[originalRatesIndex].open[i] ; else
			currentOpen = pParams->ratesBuffers->rates[ratesIndex].close[pParams->ratesBuffers->rates[ratesIndex].info.arraySize-1] ;

			currentTime  = pParams->ratesBuffers->rates[originalRatesIndex].time[i];
			lowest = currentOpen;
			highest = currentOpen;
			cumulativeVolume = 0;
			newCandle = FALSE;
		}

		cumulativeDown = currentOpen-pParams->ratesBuffers->rates[originalRatesIndex].low[i];
	    cumulativeUp   = pParams->ratesBuffers->rates[originalRatesIndex].high[i]-currentOpen;
		cumulativeVolume += pParams->ratesBuffers->rates[originalRatesIndex].volume[i]; 

		/*if((cumulativeUp > renkoSize) && (cumulativeDown > renkoSize))
		{
			logCritical("addNewRenkoRates() failed. Requested renko bar size is too small for current data resolution. Use a lower time frame as source or use a larger renko bar size for array generation.");
			return SUCCESS;
		}*/

		if (pParams->ratesBuffers->rates[originalRatesIndex].low[i] < lowest) lowest = pParams->ratesBuffers->rates[originalRatesIndex].low[i];
		if (pParams->ratesBuffers->rates[originalRatesIndex].high[i] > highest) highest = pParams->ratesBuffers->rates[originalRatesIndex].high[i];

		if ((cumulativeDown > renkoSize) || (cumulativeUp > renkoSize) || (i == arraySize-1))
		{
			
			pParams->ratesBuffers->rates[ratesIndex].info.arraySize += 1 ;

			if (pParams->ratesBuffers->rates[ratesIndex].info.arraySize > 1) 
			{
				pParams->ratesBuffers->rates[ratesIndex].time   =  (time_t*)realloc(pParams->ratesBuffers->rates[ratesIndex].time, pParams->ratesBuffers->rates[ratesIndex].info.arraySize * sizeof(time_t));
				pParams->ratesBuffers->rates[ratesIndex].open   =  (double*)realloc(pParams->ratesBuffers->rates[ratesIndex].open, pParams->ratesBuffers->rates[ratesIndex].info.arraySize * sizeof(double));
				pParams->ratesBuffers->rates[ratesIndex].high   =  (double*)realloc(pParams->ratesBuffers->rates[ratesIndex].high, pParams->ratesBuffers->rates[ratesIndex].info.arraySize * sizeof(double));
				pParams->ratesBuffers->rates[ratesIndex].low    =  (double*)realloc(pParams->ratesBuffers->rates[ratesIndex].low, pParams->ratesBuffers->rates[ratesIndex].info.arraySize * sizeof(double));
				pParams->ratesBuffers->rates[ratesIndex].close  =  (double*)realloc(pParams->ratesBuffers->rates[ratesIndex].close, pParams->ratesBuffers->rates[ratesIndex].info.arraySize * sizeof(double));
				pParams->ratesBuffers->rates[ratesIndex].volume =  (double*)realloc(pParams->ratesBuffers->rates[ratesIndex].volume, pParams->ratesBuffers->rates[ratesIndex].info.arraySize * sizeof(double));
			}
			else
			{		
				pParams->ratesBuffers->rates[ratesIndex].time   =  (time_t*)malloc(pParams->ratesBuffers->rates[ratesIndex].info.arraySize * sizeof(time_t));
				pParams->ratesBuffers->rates[ratesIndex].open   =  (double*)malloc(pParams->ratesBuffers->rates[ratesIndex].info.arraySize * sizeof(double));
				pParams->ratesBuffers->rates[ratesIndex].high   =  (double*)malloc(pParams->ratesBuffers->rates[ratesIndex].info.arraySize * sizeof(double));
				pParams->ratesBuffers->rates[ratesIndex].low    =  (double*)malloc(pParams->ratesBuffers->rates[ratesIndex].info.arraySize * sizeof(double));
				pParams->ratesBuffers->rates[ratesIndex].close  =  (double*)malloc(pParams->ratesBuffers->rates[ratesIndex].info.arraySize * sizeof(double));
				pParams->ratesBuffers->rates[ratesIndex].volume =  (double*)malloc(pParams->ratesBuffers->rates[ratesIndex].info.arraySize * sizeof(double));
			}

			if (cumulativeDown > renkoSize)
			{
				pParams->ratesBuffers->rates[ratesIndex].time[pParams->ratesBuffers->rates[ratesIndex].info.arraySize-1] = currentTime;
				pParams->ratesBuffers->rates[ratesIndex].open[pParams->ratesBuffers->rates[ratesIndex].info.arraySize-1] = currentOpen;
				pParams->ratesBuffers->rates[ratesIndex].high[pParams->ratesBuffers->rates[ratesIndex].info.arraySize-1] = highest;
				pParams->ratesBuffers->rates[ratesIndex].low[pParams->ratesBuffers->rates[ratesIndex].info.arraySize-1] = currentOpen-renkoSize;
				pParams->ratesBuffers->rates[ratesIndex].close[pParams->ratesBuffers->rates[ratesIndex].info.arraySize-1] = currentOpen-renkoSize;
				pParams->ratesBuffers->rates[ratesIndex].volume[pParams->ratesBuffers->rates[ratesIndex].info.arraySize-1] = cumulativeVolume;
			}

			if (cumulativeUp > renkoSize)
			{
				pParams->ratesBuffers->rates[ratesIndex].time[pParams->ratesBuffers->rates[ratesIndex].info.arraySize-1] = currentTime;
				pParams->ratesBuffers->rates[ratesIndex].open[pParams->ratesBuffers->rates[ratesIndex].info.arraySize-1] = currentOpen;
				pParams->ratesBuffers->rates[ratesIndex].high[pParams->ratesBuffers->rates[ratesIndex].info.arraySize-1] = currentOpen+renkoSize;
				pParams->ratesBuffers->rates[ratesIndex].low[pParams->ratesBuffers->rates[ratesIndex].info.arraySize-1] = lowest;
				pParams->ratesBuffers->rates[ratesIndex].close[pParams->ratesBuffers->rates[ratesIndex].info.arraySize-1] = currentOpen+renkoSize;
				pParams->ratesBuffers->rates[ratesIndex].volume[pParams->ratesBuffers->rates[ratesIndex].info.arraySize-1] = cumulativeVolume;
			}

			if ((i == arraySize-1) && (cumulativeDown < renkoSize) && (cumulativeUp < renkoSize))
			{
				pParams->ratesBuffers->rates[ratesIndex].time[pParams->ratesBuffers->rates[ratesIndex].info.arraySize-1] = currentTime;
				pParams->ratesBuffers->rates[ratesIndex].open[pParams->ratesBuffers->rates[ratesIndex].info.arraySize-1] = currentOpen;
				pParams->ratesBuffers->rates[ratesIndex].high[pParams->ratesBuffers->rates[ratesIndex].info.arraySize-1] = highest;
				pParams->ratesBuffers->rates[ratesIndex].low[pParams->ratesBuffers->rates[ratesIndex].info.arraySize-1] = lowest;
				pParams->ratesBuffers->rates[ratesIndex].close[pParams->ratesBuffers->rates[ratesIndex].info.arraySize-1] = pParams->ratesBuffers->rates[originalRatesIndex].close[i];
				pParams->ratesBuffers->rates[ratesIndex].volume[pParams->ratesBuffers->rates[ratesIndex].info.arraySize-1] = cumulativeVolume;
			}

			newCandle = TRUE;
		
		}


	}


	return SUCCESS;
}

AsirikuyReturnCode EasyTrade::addNewConstantVolumeRates(int originalRatesIndex, int ratesIndex, int volumeRequired)
{

	int arraySize, i , newCandle;
	double cumulativeVolume;
	double currentOpen, lowest, highest;
	time_t currentTime;

    arraySize = pParams->ratesBuffers->rates[originalRatesIndex].info.arraySize;

	pParams->ratesBuffers->rates[ratesIndex].info.arraySize = 0 ;

	newCandle = TRUE;
	cumulativeVolume = 0;

	for(i=0;i<arraySize;i++)
	{

		if (newCandle == TRUE)
		{
			if (pParams->ratesBuffers->rates[ratesIndex].info.arraySize == 0) 
			currentOpen = pParams->ratesBuffers->rates[originalRatesIndex].open[i] ; else
			currentOpen = pParams->ratesBuffers->rates[ratesIndex].close[pParams->ratesBuffers->rates[ratesIndex].info.arraySize-1] ;

			currentTime  = pParams->ratesBuffers->rates[originalRatesIndex].time[i];
			lowest = currentOpen;
			highest = currentOpen;
			cumulativeVolume = 0;
			newCandle = FALSE;
		}

		cumulativeVolume += pParams->ratesBuffers->rates[originalRatesIndex].volume[i]; 

		if (pParams->ratesBuffers->rates[originalRatesIndex].low[i] < lowest) lowest = pParams->ratesBuffers->rates[originalRatesIndex].low[i];
		if (pParams->ratesBuffers->rates[originalRatesIndex].high[i] > highest) highest = pParams->ratesBuffers->rates[originalRatesIndex].high[i];

		if ((cumulativeVolume >= volumeRequired) || (i == arraySize-1))
		{
			
			pParams->ratesBuffers->rates[ratesIndex].info.arraySize += 1 ;

			if (pParams->ratesBuffers->rates[ratesIndex].info.arraySize > 1) 
			{
				pParams->ratesBuffers->rates[ratesIndex].time   =  (time_t*)realloc(pParams->ratesBuffers->rates[ratesIndex].time, pParams->ratesBuffers->rates[ratesIndex].info.arraySize * sizeof(time_t));
				pParams->ratesBuffers->rates[ratesIndex].open   =  (double*)realloc(pParams->ratesBuffers->rates[ratesIndex].open, pParams->ratesBuffers->rates[ratesIndex].info.arraySize * sizeof(double));
				pParams->ratesBuffers->rates[ratesIndex].high   =  (double*)realloc(pParams->ratesBuffers->rates[ratesIndex].high, pParams->ratesBuffers->rates[ratesIndex].info.arraySize * sizeof(double));
				pParams->ratesBuffers->rates[ratesIndex].low    =  (double*)realloc(pParams->ratesBuffers->rates[ratesIndex].low, pParams->ratesBuffers->rates[ratesIndex].info.arraySize * sizeof(double));
				pParams->ratesBuffers->rates[ratesIndex].close  =  (double*)realloc(pParams->ratesBuffers->rates[ratesIndex].close, pParams->ratesBuffers->rates[ratesIndex].info.arraySize * sizeof(double));
				pParams->ratesBuffers->rates[ratesIndex].volume =  (double*)realloc(pParams->ratesBuffers->rates[ratesIndex].volume, pParams->ratesBuffers->rates[ratesIndex].info.arraySize * sizeof(double));
			}
			else
			{		
				pParams->ratesBuffers->rates[ratesIndex].time   =  (time_t*)malloc(pParams->ratesBuffers->rates[ratesIndex].info.arraySize * sizeof(time_t));
				pParams->ratesBuffers->rates[ratesIndex].open   =  (double*)malloc(pParams->ratesBuffers->rates[ratesIndex].info.arraySize * sizeof(double));
				pParams->ratesBuffers->rates[ratesIndex].high   =  (double*)malloc(pParams->ratesBuffers->rates[ratesIndex].info.arraySize * sizeof(double));
				pParams->ratesBuffers->rates[ratesIndex].low    =  (double*)malloc(pParams->ratesBuffers->rates[ratesIndex].info.arraySize * sizeof(double));
				pParams->ratesBuffers->rates[ratesIndex].close  =  (double*)malloc(pParams->ratesBuffers->rates[ratesIndex].info.arraySize * sizeof(double));
				pParams->ratesBuffers->rates[ratesIndex].volume =  (double*)malloc(pParams->ratesBuffers->rates[ratesIndex].info.arraySize * sizeof(double));
			}


				pParams->ratesBuffers->rates[ratesIndex].time[pParams->ratesBuffers->rates[ratesIndex].info.arraySize-1] = currentTime;
				pParams->ratesBuffers->rates[ratesIndex].open[pParams->ratesBuffers->rates[ratesIndex].info.arraySize-1] = currentOpen;
				pParams->ratesBuffers->rates[ratesIndex].high[pParams->ratesBuffers->rates[ratesIndex].info.arraySize-1] = highest;
				pParams->ratesBuffers->rates[ratesIndex].low[pParams->ratesBuffers->rates[ratesIndex].info.arraySize-1] = lowest;
				pParams->ratesBuffers->rates[ratesIndex].close[pParams->ratesBuffers->rates[ratesIndex].info.arraySize-1] = pParams->ratesBuffers->rates[originalRatesIndex].close[i];
				pParams->ratesBuffers->rates[ratesIndex].volume[pParams->ratesBuffers->rates[ratesIndex].info.arraySize-1] = cumulativeVolume;

			newCandle = TRUE;
		
		}


	}

	return SUCCESS;
}

AsirikuyReturnCode EasyTrade::openSingleSellLimitEasy(double entryPrice, double takeProfit, double stopLoss, double lotSize,double risk)
{
  int i, tradingSignals = 0, resultIndex = -1;

  for (i=0;i < (int)pParams->settings[MAX_OPEN_ORDERS]; i++)
  {
    if (pParams->results[i].tradingSignals == SIGNAL_NONE)
    {
      resultIndex = i;
      break;
    }
  }

  if (entryPrice < pParams->bidAsk.bid[0]){
	  logError("Cannot set a sell limit below current price");
      return SUCCESS; 
  }

  if (resultIndex == -1)
  {
    logInfo("Results array assignations already full");
    return SUCCESS;
  }

  pParams->results[resultIndex].entryPrice = entryPrice;
  pParams->results[resultIndex].lots       = lotSize;

  if(lotSize == 0 )
  {
    pParams->results[resultIndex].lots = calculateOrderSize(pParams, SELL, entryPrice, stopLoss) * risk;
  }

  pParams->results[resultIndex].brokerSL   = stopLoss;
  pParams->results[resultIndex].internalSL = 0;
  pParams->results[resultIndex].brokerTP   = takeProfit;
  pParams->results[resultIndex].internalTP = 0;

  logInfo("Sell Limit Signal, EntryPrice =%lf,TP =%lf, SL=%lf, lots=%lf", pParams->results[resultIndex].entryPrice,takeProfit, stopLoss, pParams->results[resultIndex].lots);
  addTradingSignal(SIGNAL_OPEN_SELLLIMIT, &tradingSignals);
  pParams->results[resultIndex].tradingSignals = tradingSignals;

  setLastOrderUpdateTime((int)pParams->settings[STRATEGY_INSTANCE_ID], pParams->ratesBuffers->rates[0].time[pParams->ratesBuffers->rates[0].info.arraySize - 1], (BOOL)pParams->settings[IS_BACKTESTING]);

  return SUCCESS;
}

AsirikuyReturnCode EasyTrade::openSingleSellStopEasy(double entryPrice, double takeProfit, double stopLoss, double lotSize)
{
  int i, tradingSignals = 0, resultIndex = -1;

  for (i=0;i < (int)pParams->settings[MAX_OPEN_ORDERS]; i++)
  {
    if (pParams->results[i].tradingSignals == SIGNAL_NONE)
    {
      resultIndex = i;
      break;
    }
  }

  if (entryPrice > pParams->bidAsk.bid[0]){
	  logError("Cannot set a sell stop above current price");
      return SUCCESS; 
  }

  if (resultIndex == -1)
  {
    logInfo("Results array assignations already full");
    return SUCCESS;
  }

  pParams->results[resultIndex].entryPrice = entryPrice;
  pParams->results[resultIndex].lots       = lotSize;

  if(lotSize == 0 )
  {
    pParams->results[resultIndex].lots = calculateOrderSize(pParams, SELL, entryPrice, stopLoss);
  }

  pParams->results[resultIndex].brokerSL   = stopLoss;
  pParams->results[resultIndex].internalSL = 0;
  pParams->results[resultIndex].brokerTP   = takeProfit;
  pParams->results[resultIndex].internalTP = 0;

  logInfo("Sell Stop Signal, EntryPrice =%lf,TP =%lf, SL=%lf, lots=%lf", pParams->results[resultIndex].entryPrice,takeProfit, stopLoss, pParams->results[resultIndex].lots);
  addTradingSignal(SIGNAL_OPEN_SELLSTOP, &tradingSignals);
  pParams->results[resultIndex].tradingSignals = tradingSignals;

  setLastOrderUpdateTime((int)pParams->settings[STRATEGY_INSTANCE_ID], pParams->ratesBuffers->rates[0].time[pParams->ratesBuffers->rates[0].info.arraySize - 1], (BOOL)pParams->settings[IS_BACKTESTING]);

  return SUCCESS;
}

AsirikuyReturnCode EasyTrade::openSingleBuyStopEasy(double entryPrice, double takeProfit, double stopLoss, double lotSize)
{
  int i, tradingSignals = 0, resultIndex = -1;

  for (i=0;i < (int)pParams->settings[MAX_OPEN_ORDERS]; i++)
  {
    if (pParams->results[i].tradingSignals == SIGNAL_NONE)
    {
      resultIndex = i;
      break;
    }
  }

  if (entryPrice < pParams->bidAsk.ask[0]){
	  logError("Cannot set a buy stop below current price");
      return SUCCESS; 
  }

  if (resultIndex == -1)
  {
    logInfo("Results array assignations already full");
    return SUCCESS;
  }

  pParams->results[resultIndex].entryPrice = entryPrice;
  pParams->results[resultIndex].lots       = lotSize;

  if(lotSize == 0 )
  {
    pParams->results[resultIndex].lots = calculateOrderSize(pParams, BUY, entryPrice, stopLoss);
  }

  pParams->results[resultIndex].brokerSL   = stopLoss;
  pParams->results[resultIndex].internalSL = 0;
  pParams->results[resultIndex].brokerTP   = takeProfit;
  pParams->results[resultIndex].internalTP = 0;

  logInfo("Buy Stop Signal, EntryPrice =%lf,TP =%lf, SL=%lf, lots=%lf", pParams->results[resultIndex].entryPrice, takeProfit, stopLoss, pParams->results[resultIndex].lots);
  addTradingSignal(SIGNAL_OPEN_BUYSTOP, &tradingSignals);
  pParams->results[resultIndex].tradingSignals = tradingSignals;

  setLastOrderUpdateTime((int)pParams->settings[STRATEGY_INSTANCE_ID], pParams->ratesBuffers->rates[0].time[pParams->ratesBuffers->rates[0].info.arraySize - 1], (BOOL)pParams->settings[IS_BACKTESTING]);

  return SUCCESS;
}

AsirikuyReturnCode EasyTrade::openSingleBuyLimitEasy(double entryPrice, double takeProfit, double stopLoss, double lotSize, double risk)
{
  int i, tradingSignals = 0, resultIndex = -1;

  for (i=0;i < (int)pParams->settings[MAX_OPEN_ORDERS]; i++)
  {
    if (pParams->results[i].tradingSignals == SIGNAL_NONE)
    {
      resultIndex = i;
      break;
    }
  }

  if (entryPrice > pParams->bidAsk.ask[0]){
	  logError("Cannot set a buy limit above current price");
      return SUCCESS; 
  }

  if (resultIndex == -1)
  {
    logInfo("Results array assignations already full");
    return SUCCESS;
  }

  pParams->results[resultIndex].entryPrice = entryPrice;
  pParams->results[resultIndex].lots       = lotSize;

  if(lotSize == 0 )
  {
    pParams->results[resultIndex].lots = calculateOrderSize(pParams, BUY, entryPrice, stopLoss) * risk;
  }

  pParams->results[resultIndex].brokerSL   = stopLoss;
  pParams->results[resultIndex].internalSL = 0;
  pParams->results[resultIndex].brokerTP   = takeProfit;
  pParams->results[resultIndex].internalTP = 0;

  logInfo("%d Buy Limit Signal, EntryPrice = %lf,TP =%lf, SL=%lf, lots=%lf", resultIndex, pParams->results[resultIndex].entryPrice, takeProfit, stopLoss, pParams->results[resultIndex].lots);
  addTradingSignal(SIGNAL_OPEN_BUYLIMIT, &tradingSignals);
  pParams->results[resultIndex].tradingSignals = tradingSignals;

  setLastOrderUpdateTime((int)pParams->settings[STRATEGY_INSTANCE_ID], pParams->ratesBuffers->rates[0].time[pParams->ratesBuffers->rates[0].info.arraySize - 1], (BOOL)pParams->settings[IS_BACKTESTING]);

  return SUCCESS;
}

AsirikuyReturnCode EasyTrade::openSingleShortEasy(double takeProfit, double stopLoss, double lotSize,double risk)
{
  int i, tradingSignals = 0, resultIndex = -1;

  for (i=0;i < (int)pParams->settings[MAX_OPEN_ORDERS]; i++)
  {
    if (pParams->results[i].tradingSignals == SIGNAL_NONE)
    {
      resultIndex = i;
      break;
    }
  }

  if (resultIndex == -1)
  {
    logInfo("Results array assignations already full");
    return SUCCESS;
  }

  pParams->results[resultIndex].entryPrice = pParams->bidAsk.bid[0];
  pParams->results[resultIndex].lots       = lotSize;

  if(lotSize == 0 )
  {
	  pParams->results[resultIndex].lots = calculateOrderSize(pParams, SELL, pParams->results[resultIndex].entryPrice, stopLoss)*risk;
  }

  pParams->results[resultIndex].brokerSL   = stopLoss;
  pParams->results[resultIndex].internalSL = 0;
  pParams->results[resultIndex].brokerTP   = takeProfit;
  pParams->results[resultIndex].internalTP = 0;

  logInfo("%d Short Entry Signal, EntryPrice = %lf,TP =%lf, SL=%lf, lots=%lf", resultIndex, pParams->results[resultIndex].entryPrice, takeProfit, stopLoss, pParams->results[resultIndex].lots);
  addTradingSignal(SIGNAL_OPEN_SELL, &tradingSignals);
  pParams->results[resultIndex].tradingSignals = tradingSignals;
  setLastOrderUpdateTime((int)pParams->settings[STRATEGY_INSTANCE_ID], pParams->ratesBuffers->rates[0].time[pParams->ratesBuffers->rates[0].info.arraySize - 1], (BOOL)pParams->settings[IS_BACKTESTING]);

  return SUCCESS;
}

AsirikuyReturnCode EasyTrade::openSingleLongEasy(double takeProfit, double stopLoss, double lotSize,double risk)
{
  int i, tradingSignals = 0, resultIndex = -1;
  

  for(i=0;i < (int)pParams->settings[MAX_OPEN_ORDERS]; i++)
  {
    if(pParams->results[i].tradingSignals == SIGNAL_NONE)
    {
      resultIndex = i;
      break;
    }
  }

  if(resultIndex == -1)
  {
    logInfo("Results array assignations already full");
    return SUCCESS;
  }

  pParams->results[resultIndex].entryPrice = pParams->bidAsk.ask[0];
  pParams->results[resultIndex].lots       = lotSize;

  if(lotSize == 0 )
  {
	  pParams->results[resultIndex].lots = max( calculateOrderSize(pParams, BUY, pParams->results[resultIndex].entryPrice, stopLoss) * risk,0.01);
  }

  pParams->results[resultIndex].brokerSL   = stopLoss;
  pParams->results[resultIndex].internalSL = 0;
  pParams->results[resultIndex].brokerTP   = takeProfit;
  pParams->results[resultIndex].internalTP = 0;
  pParams->results[resultIndex].ticketNumber = -1;
  
  logInfo("%d, Long Entry Signal, EntryPrice = %lf,TP =%lf, SL=%lf, lots=%lf", resultIndex, pParams->results[resultIndex].entryPrice, pParams->results[resultIndex].brokerTP, pParams->results[resultIndex].brokerSL, pParams->results[resultIndex].lots);

  addTradingSignal(SIGNAL_OPEN_BUY, &tradingSignals);
  pParams->results[resultIndex].tradingSignals = tradingSignals;
  setLastOrderUpdateTime((int)pParams->settings[STRATEGY_INSTANCE_ID], pParams->ratesBuffers->rates[0].time[pParams->ratesBuffers->rates[0].info.arraySize - 1], (BOOL)pParams->settings[IS_BACKTESTING]);

  return SUCCESS;
}

double EasyTrade::spread()
{
  return(fabs(pParams->bidAsk.ask[0] - pParams->bidAsk.bid[0]));
}

int EasyTrade::barsCount(int ratesArrayIndex)
{
  return(pParams->ratesBuffers->rates[ratesArrayIndex].info.arraySize);
}

AsirikuyReturnCode EasyTrade::initEasyTradeLibrary(StrategyParams* pInputParams)
{
  int i;

  pParams = pInputParams;	

  for (i=0; i < TOTAL_UI_VALUES; i++)
  {
    userInterfaceVariableNames[i] = (char*)"";
    userInterfaceValues[i] = 0;
  }

  return SUCCESS;
}

double EasyTrade::parameter(int parameterIndex)
{
  return pParams->settings[parameterIndex];
}

BOOL EasyTrade::addValueToUI(char* valueName, double valueToAdd)
{
  int i;

  for (i=0; i < TOTAL_UI_VALUES; i++)
  {
    if (userInterfaceVariableNames[i] == "")
    {
      userInterfaceVariableNames[i] = valueName;
      userInterfaceValues[i] = valueToAdd;
      saveUserInterfaceValues(userInterfaceVariableNames, userInterfaceValues, 20,  (int)pParams->settings[STRATEGY_INSTANCE_ID], (BOOL)pParams->settings[IS_BACKTESTING]);
      return TRUE;
    }
  }

  return FALSE;
}

int EasyTrade::findShift(int finalArrayIndex, int originalArrayIndex, int shift)
{

  int shift0IndexFinal = pParams->ratesBuffers->rates[finalArrayIndex].info.arraySize - 1 ;
  int shift0IndexOriginal = pParams->ratesBuffers->rates[originalArrayIndex].info.arraySize - 1 ;
  int i = 0;
  

  while (pParams->ratesBuffers->rates[finalArrayIndex].time[shift0IndexFinal-i] > pParams->ratesBuffers->rates[originalArrayIndex].time[shift0IndexOriginal-shift])
	  i++;

  return(i);
}

time_t EasyTrade::openTime(int shift)
{
  int shift0Index = pParams->ratesBuffers->rates[0].info.arraySize - 1 ;

  return pParams->ratesBuffers->rates[0].time[shift0Index-shift];
}

double EasyTrade::open(int shift)
{
  int shift0Index = pParams->ratesBuffers->rates[0].info.arraySize - 1 ;

  return pParams->ratesBuffers->rates[0].open[shift0Index-shift];
}

double EasyTrade::volume(int shift)
{
  int shift0Index = pParams->ratesBuffers->rates[0].info.arraySize - 1 ;

  return pParams->ratesBuffers->rates[0].volume[shift0Index-shift];
}

double EasyTrade::iVolume(int rateIndex, int shift)
{
	int shift0Index = pParams->ratesBuffers->rates[rateIndex].info.arraySize - 1;

	return pParams->ratesBuffers->rates[rateIndex].volume[shift0Index - shift];
}

double EasyTrade::high(int shift)
{
  int shift0Index = pParams->ratesBuffers->rates[0].info.arraySize - 1 ;

  return pParams->ratesBuffers->rates[0].high[shift0Index-shift];
}

double EasyTrade::low(int shift)
{
  int shift0Index = pParams->ratesBuffers->rates[0].info.arraySize - 1 ;

  return pParams->ratesBuffers->rates[0].low[shift0Index-shift];
}

double EasyTrade::close(int shift)
{
  int shift0Index = pParams->ratesBuffers->rates[0].info.arraySize - 1 ;

  return pParams->ratesBuffers->rates[0].close[shift0Index-shift];
}

double EasyTrade::iClose(int rateIndex, int shift)
{
  int shift0Index = pParams->ratesBuffers->rates[rateIndex].info.arraySize - 1 ;

  return pParams->ratesBuffers->rates[rateIndex].close[shift0Index-shift];
}

double EasyTrade::iOpen(int rateIndex, int shift)
{
  int shift0Index = pParams->ratesBuffers->rates[rateIndex].info.arraySize - 1 ;

  return pParams->ratesBuffers->rates[rateIndex].open[shift0Index-shift];
}

double EasyTrade::iHigh(int rateIndex, int shift)
{
  int shift0Index = pParams->ratesBuffers->rates[rateIndex].info.arraySize - 1 ;

  return pParams->ratesBuffers->rates[rateIndex].high[shift0Index-shift];
}

double EasyTrade::iLow(int rateIndex, int shift)
{
  int shift0Index = pParams->ratesBuffers->rates[rateIndex].info.arraySize - 1 ;

  return pParams->ratesBuffers->rates[rateIndex].low[shift0Index-shift];
}

time_t EasyTrade::iOpenTime(int rateIndex, int shift)
{
  int shift0Index = pParams->ratesBuffers->rates[rateIndex].info.arraySize - 1 ;

  return pParams->ratesBuffers->rates[rateIndex].time[shift0Index-shift];
}

double EasyTrade::range(int shift)
{
  int shift0Index = pParams->ratesBuffers->rates[0].info.arraySize - 1 ;

  return fabs(pParams->ratesBuffers->rates[0].high[shift0Index-shift]-pParams->ratesBuffers->rates[0].low[shift0Index-shift]);
}

double EasyTrade::body(int shift)
{
  int shift0Index = pParams->ratesBuffers->rates[0].info.arraySize - 1 ;
  double body = pParams->ratesBuffers->rates[0].close[shift0Index-shift]-pParams->ratesBuffers->rates[0].open[shift0Index-shift];

  if (body == 0)
  {
    body = 0.00000001;
  }

  return (body);
}

double EasyTrade::absBody(int shift)
{
  int shift0Index = pParams->ratesBuffers->rates[0].info.arraySize - 1 ;
  double body = fabs(pParams->ratesBuffers->rates[0].close[shift0Index-shift]-pParams->ratesBuffers->rates[0].open[shift0Index-shift]);

  if (body == 0)
  {
    body = 0.00000001;
  }

  return (body);
}

int EasyTrade::hour()
{
  struct tm timeInfo;
  int shift0Index = pParams->ratesBuffers->rates[0].info.arraySize - 1 ;
  time_t time = pParams->ratesBuffers->rates[0].time[shift0Index ] ;
  safe_gmtime(&timeInfo, time);
  return timeInfo.tm_hour;
}

int EasyTrade::minute()
{
  struct tm timeInfo;
  int shift0Index = pParams->ratesBuffers->rates[0].info.arraySize - 1 ;
  time_t time = pParams->ratesBuffers->rates[0].time[shift0Index ] ;
  safe_gmtime(&timeInfo, time);
  return timeInfo.tm_min;
}

int EasyTrade::dayOfWeek()
{
  struct tm timeInfo;
  int shift0Index = pParams->ratesBuffers->rates[0].info.arraySize - 1 ;
  time_t time = pParams->ratesBuffers->rates[0].time[shift0Index ] ;
  safe_gmtime(&timeInfo, time);
  return timeInfo.tm_wday;
}

int EasyTrade::month()
{
  struct tm timeInfo;
  int shift0Index = pParams->ratesBuffers->rates[0].info.arraySize - 1 ;
  time_t time = pParams->ratesBuffers->rates[0].time[shift0Index ] ;
  safe_gmtime(&timeInfo, time);
  return timeInfo.tm_mon;
}

int EasyTrade::year()
{
  struct tm timeInfo;
  int shift0Index = pParams->ratesBuffers->rates[0].info.arraySize - 1 ;
  time_t time = pParams->ratesBuffers->rates[0].time[shift0Index ] ;
  safe_gmtime(&timeInfo, time);
  return timeInfo.tm_year;
}

int EasyTrade::dayOfYear()
{
  struct tm timeInfo;
  int shift0Index = pParams->ratesBuffers->rates[0].info.arraySize - 1 ;
  time_t time = pParams->ratesBuffers->rates[0].time[shift0Index ] ;
  safe_gmtime(&timeInfo, time);
  return timeInfo.tm_yday;
}

int EasyTrade::dayOfMonth()
{
  struct tm timeInfo;
  int shift0Index = pParams->ratesBuffers->rates[0].info.arraySize - 1 ;
  time_t time = pParams->ratesBuffers->rates[0].time[shift0Index ] ;
  safe_gmtime(&timeInfo, time);
  return timeInfo.tm_mday;
}

double EasyTrade::iSMI(int ratesArrayIndex, int period_Q, int period_R, int period_S, int signal, int shift){

	double* HQ_Buffer;
	double* SM_Buffer;
	double* HQ_EMA_1, *SM_EMA_1;
	double* HQ_EMA_2, *SM_EMA_2;
	double* SMI_Buffer;
	int shift0Index = pParams->ratesBuffers->rates[ratesArrayIndex].info.arraySize - 1 ;
	int arraySize = pParams->ratesBuffers->rates[ratesArrayIndex].info.arraySize;
	int i, n;
	int highestHighIndex, lowestLowIndex;
	double SMI_Signal;
	Rates*     rates;
	TA_RetCode taRetCode;
    int        outBegIdx, outNBElement;
	double highestHigh, lowestLow;
  

	rates = &pParams->ratesBuffers->rates[ratesArrayIndex]; 

	HQ_Buffer = (double*)malloc((pParams->ratesBuffers->rates[ratesArrayIndex].info.arraySize)  * sizeof(double));
	SM_Buffer = (double*)malloc((pParams->ratesBuffers->rates[ratesArrayIndex].info.arraySize)  * sizeof(double));
	HQ_EMA_1 = (double*)malloc((pParams->ratesBuffers->rates[ratesArrayIndex].info.arraySize)  * sizeof(double));
	SM_EMA_1 = (double*)malloc((pParams->ratesBuffers->rates[ratesArrayIndex].info.arraySize)  * sizeof(double));
	HQ_EMA_2 = (double*)malloc((pParams->ratesBuffers->rates[ratesArrayIndex].info.arraySize)  * sizeof(double));
	SM_EMA_2 = (double*)malloc((pParams->ratesBuffers->rates[ratesArrayIndex].info.arraySize)  * sizeof(double));
	SMI_Buffer = (double*)malloc((pParams->ratesBuffers->rates[ratesArrayIndex].info.arraySize)  * sizeof(double));

	for (i = 0; i < arraySize ; i++){

		if (i > period_Q){

			highestHigh = pParams->ratesBuffers->rates[ratesArrayIndex].high[i];
			lowestLow = pParams->ratesBuffers->rates[ratesArrayIndex].low[i];

			for(n = 0; n < period_Q; n++){ 

				if(pParams->ratesBuffers->rates[ratesArrayIndex].high[i-n] > highestHigh){
					highestHighIndex = i-n;
				}

				if(pParams->ratesBuffers->rates[ratesArrayIndex].low[i-n] < lowestLow){
					lowestLowIndex = i-n;
				}
			}

			HQ_Buffer[i] = pParams->ratesBuffers->rates[ratesArrayIndex].high[highestHighIndex]-pParams->ratesBuffers->rates[ratesArrayIndex].low[lowestLowIndex] ;
			SM_Buffer[i] = pParams->ratesBuffers->rates[ratesArrayIndex].close[i] - (pParams->ratesBuffers->rates[ratesArrayIndex].high[highestHighIndex]+pParams->ratesBuffers->rates[ratesArrayIndex].low[lowestLowIndex])/2 ;
		
		} else {

			HQ_Buffer[i] = 0 ;
			SM_Buffer[i] = 0 ;
		}

	}

	for (i = 0; i < arraySize ; i++){

		if (i > period_R+period_Q){

taRetCode = TA_MA(i, i, HQ_Buffer, period_R, TA_MAType_EMA, &outBegIdx, &outNBElement, &HQ_EMA_1[i]);
taRetCode = TA_MA(i, i, SM_Buffer, period_R, TA_MAType_EMA, &outBegIdx, &outNBElement, &SM_EMA_1[i]);

		}
 else {

	 HQ_EMA_1[i] = 0;
	 SM_EMA_1[i] = 0;
 }

	}

	for (i = 0; i < arraySize; i++){

		if (i > period_Q + period_S + period_R){

			taRetCode = TA_MA(i, i, HQ_EMA_1, period_S, TA_MAType_EMA, &outBegIdx, &outNBElement, &HQ_EMA_2[i]);
			taRetCode = TA_MA(i, i, SM_EMA_1, period_S, TA_MAType_EMA, &outBegIdx, &outNBElement, &SM_EMA_2[i]);
		}
		else {

			HQ_EMA_2[i] = 0;
			SM_EMA_2[i] = 0;
		}

	}

	for (i = 0; i < arraySize; i++){

		if (i > period_S + period_R + period_Q){
			SMI_Buffer[i] = 100 * SM_EMA_2[i] / 0.5 / HQ_EMA_2[i];
		}
		else {
			SMI_Buffer[i] = 0;
		}
	}

	taRetCode = TA_MA(arraySize - shift, arraySize - shift, SMI_Buffer, signal + 1, TA_MAType_EMA, &outBegIdx, &outNBElement, &SMI_Signal);

	free(HQ_Buffer);
	free(SM_Buffer);
	free(HQ_EMA_1);
	free(HQ_EMA_2);
	free(SM_EMA_1);
	free(SM_EMA_2);
	free(SMI_Buffer);

	return(SMI_Signal);

}

double EasyTrade::iSTO(int ratesArrayIndex, int period, int k, int d, int signal, int shift)
{
	TA_RetCode retCode;
	int        outBegIdx, outNBElement;
	double	   stoch1, stoch2;
	int shift0Index = pParams->ratesBuffers->rates[ratesArrayIndex].info.arraySize - 1;

	retCode = TA_STOCH(shift0Index - shift, shift0Index - shift, pParams->ratesBuffers->rates[ratesArrayIndex].high, pParams->ratesBuffers->rates[ratesArrayIndex].low, pParams->ratesBuffers->rates[ratesArrayIndex].close, period, k, TA_MAType_SMA, d, TA_MAType_SMA, &outBegIdx, &outNBElement, &stoch1, &stoch2);
	if (retCode != TA_SUCCESS)
	{
		return INDICATOR_CALCULATION_ERROR;
	}

	if (signal == 0)
	{
		return stoch1;
	}

	if (signal == 1)
	{
		return stoch2;
	}

	return 0;
}

/*
Look back 100 days, try to find the last two tops or downs in the MACD fast trend. 
*/
int EasyTrade::iMACDTrendBeiLi(int ratesArrayIndex, int fastPeriod, int slowPeriod, int signalPeriod, int startShift, double macdLimit, OrderType orderType, int *pTruningPointIndex, double *pTurningPoint, int * pMinPointIndex, double *pMinPoint)
{	
	double	  fast[300] = {}, slow[300] = {}, preHist[300] = {};	
	//int start = 1;
	int macdTrend = 0;
	int priceTrend = 0;
	int trend = 0;

	//double  turningPoint;
	//double  minPoint;
	
	//int     truningPointIndex = -1;	
	*pTruningPointIndex = -1;

	//startShift = 1;
		
	// Load 100 MACD signals
	for (int i = startShift; i < 299; i++)
	{
		iMACDAll(ratesArrayIndex, fastPeriod, slowPeriod, signalPeriod, i, &fast[i], &slow[i], &preHist[i]);
	}

	// ��������ϣ� �Ϳ�����
	// ��������£� �Ϳ��ײ�

	if (fast[startShift] > 0)
		trend = 1;
	else if (fast[startShift] < 0)
		trend = -1;
	else
		return 0;
	
	for (int i = startShift + 1; i < 299; i++)
	{	
		if (trend > 0 && fast[i] <= 0)
			break;
		if (trend < 0 && fast[i] >= 0)
			break;
				
		if( 
			(trend > 0 &&  fast[i] < fast[i - 1] && fast[i] < fast[i + 1])
			||
			(trend < 0 && fast[i] > fast[i - 1] && fast[i] > fast[i + 1])
			)
		{
			*pMinPoint = fast[i];
			*pMinPointIndex = i;			
		}
		
		if (
			(trend > 0 && fast[i] > fast[i - 1] && fast[i] > fast[i + 1])
			||
			(trend < 0 && fast[i] < fast[i - 1] && fast[i] < fast[i + 1])
			)
		{
			*pTurningPoint = fast[i];
			*pTruningPointIndex = i;
			break;
		}
	}

	if (*pTruningPointIndex >= 0)
	{

		if (fast[startShift] - *pTurningPoint > macdLimit)
			macdTrend = 1;
		else if (*pTurningPoint - fast[startShift] > macdLimit)
			macdTrend = -1;
		else
			macdTrend = 0;

		if (iClose(ratesArrayIndex, startShift) > iClose(ratesArrayIndex, *pTruningPointIndex))
			priceTrend = 1;
		else
			priceTrend = -1;
				
		if ((macdTrend == 1 && priceTrend == -1) || (macdTrend == -1 && priceTrend == 1))
			return 1;

		if (orderType == BUY && priceTrend == -1 && macdTrend == -1)
			return 1;

		if (orderType == SELL && priceTrend == 1 && macdTrend == 1)
			return 1;
	}
	
	return 0;
}

double EasyTrade::iBBandStop(int ratesArrayIndex, int bb_period, double bb_deviation, int * trend, double * bbStopPrice,int *index)
{
	TA_RetCode retCode;
	int        outBegIdx, outNBElement;
	double	  upperBand[2000] = {}, middleBand[2000] = {}, lowerBand[2000] = {};
	int shift1Index = pParams->ratesBuffers->rates[ratesArrayIndex].info.arraySize - 2;
	int start = 0;

	double upLimit = 10000, downLimit = 0;
	*trend = 0;
	*bbStopPrice = 0;

	retCode = TA_BBANDS(start, shift1Index, pParams->ratesBuffers->rates[ratesArrayIndex].close, bb_period, bb_deviation, bb_deviation, TA_MAType_SMA, &outBegIdx, &outNBElement, upperBand, middleBand, lowerBand);
	if (retCode != TA_SUCCESS)
	{
		return INDICATOR_CALCULATION_ERROR;
	}

	//int bbSize = 0;	


	// Loop through and chceck out if BBS exists
	for (int i = start; i < outNBElement; i++)
	{
		if (upperBand[i] == 0)
			break;
		if (*trend == 0)
		{
			if (upperBand[i] < upLimit)
				upLimit = upperBand[i];
			if (lowerBand[i] > downLimit)
				downLimit = lowerBand[i];
		}

		if (*trend == 1)
		{
			if (lowerBand[i] > *bbStopPrice)
				*bbStopPrice = lowerBand[i];

			downLimit = *bbStopPrice;
		}

		if (*trend == -1)
		{
			if (upperBand[i]<*bbStopPrice)
				*bbStopPrice = upperBand[i];

			upLimit = *bbStopPrice;
		}

		// Found a new up trend
		if (pParams->ratesBuffers->rates[ratesArrayIndex].close[outBegIdx+i] > upLimit && *trend != 1)
		{
			*trend = 1;
			*bbStopPrice = lowerBand[i];	
			*index = outBegIdx + i;
		}
		//Found a new down trend
		else if (pParams->ratesBuffers->rates[ratesArrayIndex].close[outBegIdx+i] < downLimit && *trend != -1)
		{
			*trend = -1;
			*bbStopPrice = upperBand[i];	
			*index = outBegIdx + i;
		}	

		
	}

	return 0;
}

double EasyTrade::iBBands(int ratesArrayIndex, int bb_period, double bb_deviation, int signal, int shift)
{
  TA_RetCode retCode;
  int        outBegIdx, outNBElement;
  double	  upperBand, middleBand, lowerBand;
  int shift0Index = pParams->ratesBuffers->rates[ratesArrayIndex].info.arraySize - 1 ;
    
  retCode = TA_BBANDS(shift0Index-shift, shift0Index-shift, pParams->ratesBuffers->rates[ratesArrayIndex].open, bb_period, bb_deviation, bb_deviation, TA_MAType_SMA, &outBegIdx, &outNBElement, &upperBand, &middleBand, &lowerBand);
  if(retCode != TA_SUCCESS)
  {
    return INDICATOR_CALCULATION_ERROR ;
  }

  switch(signal)
  {
  case 0: return upperBand; break;
  case 1: return middleBand; break;
  case 2: return lowerBand; break;
  default: return 0; break;	
  }
    
}

double EasyTrade::iASI(int ratesArrayIndex, int mode, int length, int smooth, double * outBull,double *outBear)
{
	double price1, price2;

	double	  bulls[20] = {}, avgBulls[10] = {}, smoothBulls[5] = {};
	double	  bears[20] = {}, avgBears[10] = {}, smoothBears[5] = {};
	int outBegIdx, outNBElement;
	double ma;
	double high, low;


	for (int i = 19; i > 0; i--)
	{
		price1 = iClose(ratesArrayIndex, 20-i);
		price2 = iClose(ratesArrayIndex, 20-i+1);

		if (mode == 0)
		{
			bulls[i] = 0.5 * (fabs(price1 - price2) + (price1 - price2));
			bears[i] = 0.5 * (fabs(price1 - price2) - (price1 - price2));
		}
		else
		{
			getHighLow(ratesArrayIndex, pParams->ratesBuffers->rates[B_PRIMARY_RATES].info.arraySize - (20-i) - 1, length, &high, &low);
			bulls[i] = price1 - low;
			bears[i] = high - price1;
		}
	}

	for (int i = 9; i >0; i--)
	{
		TA_MA(20 - (10 - i), 20 - (10 - i), bulls, length, TA_MAType_SMA, &outBegIdx, &outNBElement, &ma);
		avgBulls[i] = ma;

		TA_MA(20 - (10 - i), 20 - (10 - i), bears, length, TA_MAType_SMA, &outBegIdx, &outNBElement, &ma);
		avgBears[i] = ma;
	}

	for (int i = 4; i > 0; i--)
	{
		TA_MA(10 - (5 - i), 10 - (5 - i), avgBulls, smooth, TA_MAType_SMA, &outBegIdx, &outNBElement, &ma);
		smoothBulls[i] = ma;

		TA_MA(10 - (5 - i), 10 - (5 - i), avgBears, smooth, TA_MAType_SMA, &outBegIdx, &outNBElement, &ma);
		smoothBears[i] = ma;
	}

	*outBull = smoothBulls[4];
	*outBear = smoothBears[4];
	return 0;
}

double EasyTrade::iMACD(int ratesArrayIndex, int fastPeriod, int slowPeriod, int signalPeriod, int signal, int shift)
{
  TA_RetCode retCode;
  int        outBegIdx, outNBElement;
  double	   macd1, macd2, macd3;
  int shift0Index = pParams->ratesBuffers->rates[ratesArrayIndex].info.arraySize - 1 ;

  TA_SetUnstablePeriod(TA_FUNC_UNST_EMA, 35);

  //retCode = TA_MACD(shift0Index - shift, shift0Index - shift, pParams->ratesBuffers->rates[ratesArrayIndex].close, fastPeriod, slowPeriod, signalPeriod, &outBegIdx, &outNBElement, &macd1, &macd2, &macd3);
  retCode = TA_MACDEXT(shift0Index - shift, shift0Index - shift, pParams->ratesBuffers->rates[ratesArrayIndex].close, fastPeriod, TA_MAType_EMA, slowPeriod, TA_MAType_EMA, signalPeriod, TA_MAType_EMA, &outBegIdx, &outNBElement, &macd1, &macd2, &macd3);

  TA_SetUnstablePeriod(TA_FUNC_UNST_EMA, 0);
  if(retCode != TA_SUCCESS)
  {
    return INDICATOR_CALCULATION_ERROR ;
  }

  switch(signal)
  {
  case 0: return macd1; break;
  case 1: return macd2; break;
  case 2: return macd3; break;
  default: return 0; break;	
  }
}

double EasyTrade::iMACDAll(int ratesArrayIndex, int fastPeriod, int slowPeriod, int signalPeriod, int shift,double *pMacd, double *pMmacdSignal,double *pMacdHist)
{
	TA_RetCode retCode;
	int        outBegIdx, outNBElement;	
	int shift0Index = pParams->ratesBuffers->rates[ratesArrayIndex].info.arraySize - 1;

	TA_SetUnstablePeriod(TA_FUNC_UNST_EMA, 35);
	retCode = TA_MACDEXT(shift0Index - shift, shift0Index - shift, pParams->ratesBuffers->rates[ratesArrayIndex].close, fastPeriod, TA_MAType_EMA, slowPeriod, TA_MAType_EMA, signalPeriod, TA_MAType_EMA, &outBegIdx, &outNBElement, pMacd, pMmacdSignal, pMacdHist);

	TA_SetUnstablePeriod(TA_FUNC_UNST_EMA, 0);
	if (retCode != TA_SUCCESS)
	{
		return INDICATOR_CALCULATION_ERROR;
	}
	return 0;

}

double EasyTrade::iStdev(int ratesArrayIndex, int type, int period, int shift)
{
  TA_RetCode taRetCode;
  int        outBegIdx, outNBElement;
  double	   stdev;
  int shift0Index = pParams->ratesBuffers->rates[ratesArrayIndex].info.arraySize - 1 ;

  switch(type)
  {
  case 0: {
	  taRetCode = TA_STDDEV(shift0Index-shift, shift0Index-shift, pParams->ratesBuffers->rates[ratesArrayIndex].open, period, 1, &outBegIdx, &outNBElement, &stdev);
  if(taRetCode != TA_SUCCESS)
  {
    return INDICATOR_CALCULATION_ERROR ;
  }
		  }
  break;
  case 1: {
	  taRetCode = TA_STDDEV(shift0Index-shift, shift0Index-shift, pParams->ratesBuffers->rates[ratesArrayIndex].high, period, 1, &outBegIdx, &outNBElement, &stdev);
  if(taRetCode != TA_SUCCESS)
  {
    return INDICATOR_CALCULATION_ERROR ;
  }
		  }
  break;
  case 2:{
	  taRetCode = TA_STDDEV(shift0Index-shift, shift0Index-shift, pParams->ratesBuffers->rates[ratesArrayIndex].low, period, 1, &outBegIdx, &outNBElement, &stdev);
  if(taRetCode != TA_SUCCESS)
  {
    return INDICATOR_CALCULATION_ERROR ;
  }
		  }
  break;
  case 3:{
	  taRetCode = TA_STDDEV(shift0Index-shift, shift0Index-shift, pParams->ratesBuffers->rates[ratesArrayIndex].close, period, 1, &outBegIdx, &outNBElement, &stdev);
  if(taRetCode != TA_SUCCESS)
  {
    return INDICATOR_CALCULATION_ERROR ;
  }
		  }
  break;
  }

  

  return stdev;
}

double EasyTrade::iCCI(int ratesArrayIndex, int period, int shift)
{
  TA_RetCode taRetCode;
  int        outBegIdx, outNBElement;
  double	   cci;
  int shift0Index = pParams->ratesBuffers->rates[ratesArrayIndex].info.arraySize - 1 ;

  taRetCode = TA_CCI(shift0Index-shift, shift0Index-shift, pParams->ratesBuffers->rates[ratesArrayIndex].high, pParams->ratesBuffers->rates[ratesArrayIndex].low, pParams->ratesBuffers->rates[ratesArrayIndex].close, period, &outBegIdx, &outNBElement, &cci);
  if(taRetCode != TA_SUCCESS)
  {
    return INDICATOR_CALCULATION_ERROR ;
  }

  return cci;
}

double EasyTrade::iMA(int type, int ratesArrayIndex, int period, int shift)
{
  TA_RetCode taRetCode;
  int        outBegIdx, outNBElement;
  double	   ma;
  int shift0Index = pParams->ratesBuffers->rates[ratesArrayIndex].info.arraySize - 1 ;
  
  //TA_SetUnstablePeriod(TA_FUNC_UNST_EMA, 100);

  switch(type)
  {
  case 0:
    taRetCode = TA_MA(shift0Index-shift, shift0Index-shift, pParams->ratesBuffers->rates[ratesArrayIndex].open, period, TA_MAType_EMA, &outBegIdx, &outNBElement, &ma);
    break;
  case 1:
    taRetCode = TA_MA(shift0Index-shift, shift0Index-shift, pParams->ratesBuffers->rates[ratesArrayIndex].high, period, TA_MAType_EMA, &outBegIdx, &outNBElement, &ma);
    break;
  case 2:
    taRetCode = TA_MA(shift0Index-shift, shift0Index-shift, pParams->ratesBuffers->rates[ratesArrayIndex].low, period, TA_MAType_EMA, &outBegIdx, &outNBElement, &ma);
    break;
  case 3:
    taRetCode = TA_MA(shift0Index-shift, shift0Index-shift, pParams->ratesBuffers->rates[ratesArrayIndex].close, period, TA_MAType_EMA, &outBegIdx, &outNBElement, &ma);
    break;
  case 4:
	taRetCode = TA_MA(shift0Index - shift, shift0Index - shift, pParams->ratesBuffers->rates[ratesArrayIndex].volume, period, TA_MAType_EMA, &outBegIdx, &outNBElement, &ma);
	break;
  }

  //TA_SetUnstablePeriod(TA_FUNC_UNST_EMA, 0);

  if(taRetCode != TA_SUCCESS)
  {
    return INDICATOR_CALCULATION_ERROR ;
  }

  return ma;
}

double EasyTrade::iRSI(int ratesArrayIndex, int period, int shift)
{
  int i;
  double rsi = 0,  rs = 0, averageGain = 0, averageLoss = 0, candleBody;
  int shiftIndex = pParams->ratesBuffers->rates[ratesArrayIndex].info.arraySize - 1 - shift ;

  double atr = iAtrSafeShiftZero((int)parameter(ATR_AVERAGING_PERIOD));

  for (i=0; i<period; i++)
  {
    candleBody = pParams->ratesBuffers->rates[ratesArrayIndex].close[shiftIndex-i]-pParams->ratesBuffers->rates[ratesArrayIndex].close[shiftIndex-i-1];

    if(candleBody > 0 )
    {
      averageGain += candleBody/period;
    }

    if(candleBody <= 0)
    {
      averageLoss -= candleBody/period;
    }
  }

  rsi=100.0-100.0/(1+averageGain/averageLoss);

  return rsi;
}

double EasyTrade::iRangeAverage(int ratesArrayIndex, int period, int shift)
{
  int i;
  double average = 0;
  int shiftIndex = pParams->ratesBuffers->rates[ratesArrayIndex].info.arraySize - 1 - shift ;

  for (i=0; i<period; i++)
  {
    average += (pParams->ratesBuffers->rates[ratesArrayIndex].high[shiftIndex-i] - pParams->ratesBuffers->rates[ratesArrayIndex].low[shiftIndex-i])/period;  
  }

  return average;
}

double EasyTrade::iRangeSafeShiftZero(int period)
{
  double average = 0;
  int i,j;
  int dailyShift0Index = pParams->ratesBuffers->rates[DAILY_RATES].info.arraySize - 1 ;
  double* highDaily = (double*)malloc(period * sizeof(double));
  double* lowDaily  = (double*)malloc(period * sizeof(double));

  for (i=0; i<period; i++)
  {
    highDaily[i]  =  pParams->ratesBuffers->rates[DAILY_RATES].high[dailyShift0Index-i];
    lowDaily[i]   =  pParams->ratesBuffers->rates[DAILY_RATES].low[dailyShift0Index-i];

    if (i == 0)
    {
      highDaily[i]  =  pParams->ratesBuffers->rates[DAILY_RATES].open[dailyShift0Index];
      lowDaily[i]   =  pParams->ratesBuffers->rates[DAILY_RATES].open[dailyShift0Index];

      j = 1;

      while  (openTime(j) >= pParams->ratesBuffers->rates[DAILY_RATES].time[dailyShift0Index])
      {
        if (high(j) > highDaily[i])
          highDaily[i] = high(j) ;

        if (low(j) < lowDaily[i])
          lowDaily[i] = low(j) ;

        j++;
      }
    }		
  }

  for (i=0; i<period-1; i++)
  {
    average += (highDaily[i]-lowDaily[i])/period ;
  }

  free(highDaily);
  free(lowDaily);

  return average;
}


double EasyTrade::iAtrSafeShiftZero(int period)
{
  double trueRange;
  double average;
  int i,j, currentDay, lastCandleCloseDay;
  int dailyShift0Index = pParams->ratesBuffers->rates[DAILY_RATES].info.arraySize - 1 ;
  struct tm  currentTime, lastCandleTime;
  double* openDaily  = (double*)malloc(period * sizeof(double));
  double* highDaily  = (double*)malloc(period * sizeof(double));
  double* lowDaily   = (double*)malloc(period * sizeof(double));
  double* closeDaily = (double*)malloc(period * sizeof(double));

  safe_gmtime(&currentTime, pParams->currentBrokerTime);
  safe_gmtime(&lastCandleTime, openTime(1));

  currentDay = currentTime.tm_wday;
  lastCandleCloseDay = lastCandleTime.tm_wday;

  i = 0;

  while (i<period)
  {
    if ((i > 0) || lastCandleCloseDay != currentDay )
    {
      openDaily[i]  =  pParams->ratesBuffers->rates[DAILY_RATES].open[dailyShift0Index-i];
      highDaily[i]  =  pParams->ratesBuffers->rates[DAILY_RATES].high[dailyShift0Index-i];
      lowDaily[i]   =  pParams->ratesBuffers->rates[DAILY_RATES].low[dailyShift0Index-i];
      closeDaily[i] =  pParams->ratesBuffers->rates[DAILY_RATES].close[dailyShift0Index-i];
      i++;
    }

    if ((i == 0) && (lastCandleCloseDay == currentDay))
    {
      closeDaily[i] = close(1);
      highDaily [i] = high(1);
      lowDaily[i] = low(1);

      j = 1;

      while  (lastCandleCloseDay == currentDay)
      {
        if (high(j) > highDaily[i])
        {
          highDaily[i] = high(j);
        }

        if (low(j) < lowDaily[i])
        {
          lowDaily[i] = low(j);
        }

        openDaily[i] = open(j); 

        j++;

        safe_gmtime(&lastCandleTime, openTime(j));
        lastCandleCloseDay = lastCandleTime.tm_wday;
      }

      i++;
    }		
  }

  average = (highDaily[period-1]-lowDaily[period-1])/period ;

  for (i=0; i<period-1; i++)
  {
    trueRange = max(highDaily[i]-lowDaily[i], max(fabs(highDaily[i]-closeDaily[i+1]), fabs(lowDaily[i]-closeDaily[i+1]))) ;

    average += trueRange/period;
  }

  free(openDaily);
  free(highDaily);
  free(lowDaily);
  free(closeDaily);

  return average;
}

int EasyTrade::Period()
{
  return((int)pParams->settings[TIMEFRAME]);
}

double EasyTrade::iAtrWholeDaysSimple(int rateIndex, int periodATR)
{
  double average;
  int i;
  double highValue, lowValue;
  int shift0Index = pParams->ratesBuffers->rates[rateIndex].info.arraySize - 1;

  average = 0;

  for (i=0; i<periodATR; i++)
  {
    highValue   =  pParams->ratesBuffers->rates[rateIndex].high[shift0Index-(int)(i*((1440/2)/Period())+(1440/Period()))];
	lowValue    =  pParams->ratesBuffers->rates[rateIndex].low[shift0Index-(int)(i*((1440/2)/Period())+1)];
    average +=  fabs(highValue-lowValue)/periodATR ; 
  }

  return average;
}

int EasyTrade::Digits()
{
  return(pParams->ratesBuffers->rates[PRIMARY_RATES_INDEX].info.digits);
}

double EasyTrade::iAtrDailyByHourInterval(int period, int firstHour, int lastHour)
{
  double trueRange;
  double average;
  int i,j,k;
  int dailyShift0Index = pParams->ratesBuffers->rates[DAILY_RATES].info.arraySize - 1 ;
  double* openDaily  = (double*)malloc(period * sizeof(double));
  double* highDaily  = (double*)malloc(period * sizeof(double));
  double* lowDaily   = (double*)malloc(period * sizeof(double));
  double* closeDaily = (double*)malloc(period * sizeof(double));
  struct tm  timeInfo;
  int hourDifferential = lastHour-firstHour;
  
  i = 0;
  k = 1;

  if ((lastHour < firstHour) || (firstHour < 0) || (lastHour > 23))
	  return -1;

  while (i<period)
  {

	safe_gmtime(&timeInfo, openTime(k));

	if (timeInfo.tm_hour == lastHour){

		openDaily[i]  =  open(k+hourDifferential);
		highDaily[i]  =  open(k+hourDifferential);
		lowDaily[i]   =  open(k+hourDifferential);
		closeDaily[i] =  close(k);

		j = 1;

		while  (j <= hourDifferential){

			if (high(k+j) > highDaily[i])
			highDaily[i] = high(k+j) ;

			if (low(k+j) < lowDaily[i])
			lowDaily[i] = low(k+j) ;

			j++;
		}	

		i++;
	}

	k++;

  }

  average = (highDaily[period-1]-lowDaily[period-1])/period ;

  for (i=0; i<period-1; i++)
  {
    trueRange = max(highDaily[i]-lowDaily[i], max(fabs(highDaily[i]-closeDaily[i+1]), fabs(lowDaily[i]-closeDaily[i+1]))) ;

    average += trueRange/period;
  }

  free(openDaily);
  free(highDaily);
  free(lowDaily);
  free(closeDaily);

  return average;
}

double EasyTrade::iAtrSafeShiftZeroWholeDays(int period)
{
  double trueRange;
  double average;
  int i,j;
  int dailyShift0Index = pParams->ratesBuffers->rates[DAILY_RATES].info.arraySize - 1 ;
  double* openDaily  = (double*)malloc(period * sizeof(double));
  double* highDaily  = (double*)malloc(period * sizeof(double));
  double* lowDaily   = (double*)malloc(period * sizeof(double));
  double* closeDaily = (double*)malloc(period * sizeof(double));

  for (i=0; i<period; i++)
  {

    openDaily[i]  =  open(24*(i+1));
    highDaily[i]  =  open(24*(i+1));
    lowDaily[i]   =  open(24*(i+1));
    closeDaily[i] =  close(i*24+1);

    j = 1;

    while  (j < 25)
    {
      if (high(i*24+j) > highDaily[i])
        highDaily[i] = high(i*24+j) ;

      if (low(i*24+j) < lowDaily[i])
        lowDaily[i] = low(i*24+j) ;

      j++;
    }		
  }

  average = (highDaily[period-1]-lowDaily[period-1])/period ;

  for (i=0; i<period-1; i++)
  {
    trueRange = max(highDaily[i]-lowDaily[i], max(fabs(highDaily[i]-closeDaily[i+1]), fabs(lowDaily[i]-closeDaily[i+1]))) ;

    average += trueRange/period;
  }

  free(openDaily);
  free(highDaily);
  free(lowDaily);
  free(closeDaily);

  return average;
}

double EasyTrade::iAtr(int ratesArrayIndex, int period, int shift)
{
  TA_RetCode taRetCode;
  int        outBegIdx, outNBElement;
  double	   atr;
  int shift0Index = pParams->ratesBuffers->rates[ratesArrayIndex].info.arraySize - 1 ;

  taRetCode = TA_ATR(shift0Index-shift, shift0Index-shift, pParams->ratesBuffers->rates[ratesArrayIndex].high, pParams->ratesBuffers->rates[ratesArrayIndex].low, pParams->ratesBuffers->rates[ratesArrayIndex].close, period, &outBegIdx, &outNBElement, &atr);

  if(taRetCode != TA_SUCCESS)
  {
    return INDICATOR_CALCULATION_ERROR ;
  }

  return atr;
}

void EasyTrade::print(double valueToPrint)
{
  logCritical("Print = %lf", valueToPrint);
}

AsirikuyReturnCode EasyTrade::checkOrders(double takeProfit, double stopLoss)
{
  AsirikuyReturnCode returnCode = SUCCESS;

  // check internal TP
  checkInternalTP(pParams, PRIMARY_RATES_INDEX, 0, takeProfit);
  if(returnCode != SUCCESS)
  {
    return logAsirikuyError("checkOrders()", returnCode);
  }

  // check timed exit
  returnCode = checkTimedExit(pParams, PRIMARY_RATES_INDEX, 0, FALSE, TRUE);
  if(returnCode != SUCCESS)
  {
    return logAsirikuyError("checkOrders()", returnCode);
  }

  // check that open orders are properly set
  if(!areOrdersCorrect(pParams, pParams->settings[USE_SL], pParams->settings[USE_TP]))
  {
    setStops(pParams, PRIMARY_RATES_INDEX, 0, stopLoss, takeProfit, FALSE, TRUE);
  }

  return SUCCESS;

}

AsirikuyReturnCode EasyTrade::openOrUpdateLongEasy(double takeProfit, double stopLoss,double risk)
{
	//Defualt rate and result index is 0, it means it used the execution time frame.
  return openOrUpdateLongTrade(pParams, 0, 0, stopLoss, takeProfit,risk, FALSE, FALSE);
}

AsirikuyReturnCode EasyTrade::openOrUpdateShortEasy(double takeProfit, double stopLoss,double risk)
{
  return openOrUpdateShortTrade(pParams, 0, 0, stopLoss, takeProfit, risk,FALSE, FALSE);
}

AsirikuyReturnCode EasyTrade::closeAllLimitAndStopOrders(time_t currentTime)
{
	int i;
	struct tm timeInfo1, timeInfo2;
	safe_gmtime(&timeInfo1, currentTime);
	for (i = 0; i < pParams->settings[ORDERINFO_ARRAY_SIZE]; i++)
	{
		safe_gmtime(&timeInfo2, pParams->orderInfo[i].openTime);
		if (timeInfo1.tm_mday == timeInfo2.tm_mday && pParams->orderInfo[i].ticket != 0 && pParams->orderInfo[i].isOpen)
		{
			if (pParams->orderInfo[i].type == BUYSTOP)
			{
				logInfo("Close buy stop orders type = %d, ticket = %d", (int)pParams->orderInfo[i].type, (int)pParams->orderInfo[i].ticket);
				closeBuyStopEasy(pParams->orderInfo[i].ticket);
			}

			if (pParams->orderInfo[i].type == SELLSTOP)
			{
				logInfo("Close sell stop orders type = %d, ticket = %d", (int)pParams->orderInfo[i].type, (int)pParams->orderInfo[i].ticket);
				closeSellStopEasy(pParams->orderInfo[i].ticket);
			}

			if (pParams->orderInfo[i].type == BUYLIMIT)
			{
				logInfo("Close buy Limit orders type = %d, ticket = %d", (int)pParams->orderInfo[i].type, (int)pParams->orderInfo[i].ticket);
				closeBuyLimitEasy(pParams->orderInfo[i].ticket);
			}

			if (pParams->orderInfo[i].type == SELLLIMIT)
			{
				logInfo("Close sell Limit orders type = %d, ticket = %d", (int)pParams->orderInfo[i].type, (int)pParams->orderInfo[i].ticket);
				closeSellLimitEasy(pParams->orderInfo[i].ticket);
			}
		}
	}
	
	return SUCCESS;
}

AsirikuyReturnCode EasyTrade::closeAllBuyStopOrders(time_t currentTime)
{
	int i;

	struct tm timeInfo1, timeInfo2;
	safe_gmtime(&timeInfo1, currentTime);

	for (i = 0; i < pParams->settings[ORDERINFO_ARRAY_SIZE]; i++)
	{

		if (pParams->orderInfo[i].type == BUYSTOP && pParams->orderInfo[i].ticket != 0 && pParams->orderInfo[i].isOpen)
		{
			safe_gmtime(&timeInfo2, pParams->orderInfo[i].openTime);
			if (timeInfo1.tm_mday != timeInfo2.tm_mday)
			{
				logInfo("Close all Limit orders type = %d, ticket = %d", (int)pParams->orderInfo[i].type, (int)pParams->orderInfo[i].ticket);
				closeBuyStopEasy(pParams->orderInfo[i].ticket);
			}
		}
	}
	return SUCCESS;
}

AsirikuyReturnCode EasyTrade::closeAllSellStopOrders(time_t currentTime)
{
	int i;
	struct tm timeInfo1, timeInfo2;
	safe_gmtime(&timeInfo1, currentTime);
	for (i = 0; i < pParams->settings[ORDERINFO_ARRAY_SIZE]; i++)
	{

		if (pParams->orderInfo[i].type == SELLSTOP && pParams->orderInfo[i].ticket != 0 && pParams->orderInfo[i].isOpen)
		{
			safe_gmtime(&timeInfo2, pParams->orderInfo[i].openTime);
			if (timeInfo1.tm_mday != timeInfo2.tm_mday)
			{
				logInfo("Close all Limit orders type = %d, ticket = %d", (int)pParams->orderInfo[i].type, (int)pParams->orderInfo[i].ticket);
				closeSellStopEasy(pParams->orderInfo[i].ticket);
			}
		}
	}
	return SUCCESS;
}


AsirikuyReturnCode EasyTrade::closeAllBuyLimitOrders(time_t currentTime)
{
	int i;

	struct tm timeInfo1, timeInfo2;
	safe_gmtime(&timeInfo1, currentTime);

	for (i = 0; i < pParams->settings[ORDERINFO_ARRAY_SIZE]; i++)
	{

		if (pParams->orderInfo[i].type == BUYLIMIT && pParams->orderInfo[i].ticket != 0 && pParams->orderInfo[i].isOpen)
		{
			safe_gmtime(&timeInfo2, pParams->orderInfo[i].openTime);
			if (timeInfo1.tm_mday != timeInfo2.tm_mday)
			{		
				logInfo("Close all Limit orders type = %d, ticket = %d", (int)pParams->orderInfo[i].type, (int)pParams->orderInfo[i].ticket);
				closeBuyLimitEasy(pParams->orderInfo[i].ticket);
			}
		}
	}
	return SUCCESS;
}

AsirikuyReturnCode EasyTrade::closeAllSellLimitOrders(time_t currentTime)
{
	int i;
	struct tm timeInfo1, timeInfo2;
	safe_gmtime(&timeInfo1, currentTime);
	for (i = 0; i < pParams->settings[ORDERINFO_ARRAY_SIZE]; i++)
	{

		if (pParams->orderInfo[i].type == SELLLIMIT && pParams->orderInfo[i].ticket != 0 && pParams->orderInfo[i].isOpen)
		{
			safe_gmtime(&timeInfo2, pParams->orderInfo[i].openTime);
			if (timeInfo1.tm_mday != timeInfo2.tm_mday)
			{
				logInfo("Close all Limit orders type = %d, ticket = %d", (int)pParams->orderInfo[i].type, (int)pParams->orderInfo[i].ticket);
				closeSellLimitEasy(pParams->orderInfo[i].ticket);
			}
		}
	}
	return SUCCESS;
}

AsirikuyReturnCode EasyTrade::closeAllLongTermLongs()
{
	int i;

	for (i = 0; i < pParams->settings[ORDERINFO_ARRAY_SIZE]; i++)
	{

		if (pParams->orderInfo[i].type == BUY && pParams->orderInfo[i].ticket != 0 && pParams->orderInfo[i].isOpen && pParams->orderInfo[i].takeProfit == 0)
		{
			logWarning("closeAllLongTermLongs type = %d, ticket = %d", (int)pParams->orderInfo[i].type, (int)pParams->orderInfo[i].ticket);
			closeLongEasy(pParams->orderInfo[i].ticket);
		}
	}

	return SUCCESS;
}

AsirikuyReturnCode EasyTrade::closeAllLongs()
{
  int i;

  for (i=0; i < pParams->settings[ORDERINFO_ARRAY_SIZE]; i++)
  {

	if (pParams->orderInfo[i].type == BUY && pParams->orderInfo[i].ticket != 0 && pParams->orderInfo[i].isOpen)
	{
		logInfo("closeAllLongs type = %d, ticket = %d", (int)pParams->orderInfo[i].type, (int)pParams->orderInfo[i].ticket);
		closeLongEasy(pParams->orderInfo[i].ticket);
	}
  }

  return SUCCESS;
}

AsirikuyReturnCode EasyTrade::modifyAllShorts_DayTrading(double stopLoss1, double stopLossPrice2, double takePrice, int tpMode, time_t currentTime, double adjust, BOOL stopMovingbackSL)
{
	int i;
	struct tm timeInfo1, timeInfo2;

	safe_gmtime(&timeInfo1, currentTime);

	double newTP = takePrice;
	double newSL = stopLoss1;

	for (i = 0; i < pParams->settings[ORDERINFO_ARRAY_SIZE]; i++)
	{
		newTP = takePrice;
		newSL = stopLoss1;

		if (pParams->orderInfo[i].type == SELL && pParams->orderInfo[i].ticket != 0 && pParams->orderInfo[i].isOpen)
		{
			safe_gmtime(&timeInfo2, pParams->orderInfo[i].openTime);
			if (timeInfo1.tm_mday != timeInfo2.tm_mday) // previous day, 
			{
				if (stopLossPrice2 - pParams->bidAsk.ask[0] < 0)
					newSL = adjust;
				else 
					newSL = stopLossPrice2 - pParams->bidAsk.ask[0] + adjust;
			}

			logInfo("ModifyAllShorts type = %d, ticket = %d", (int)pParams->orderInfo[i].type, (int)pParams->orderInfo[i].ticket);

			if (tpMode == 1 && pParams->orderInfo[i].openPrice - pParams->bidAsk.ask[0] >= 0) // New day and break event
				newTP = 0;
			else
			{
				if (newTP == -1) // No change
				{
					if (pParams->orderInfo[i].takeProfit == 0)
						newTP = 0;
					else
						newTP = fabs(pParams->orderInfo[i].takeProfit - pParams->bidAsk.bid[0]);
				}
				if (newSL == -1)
				{
					if (pParams->orderInfo[i].stopLoss == 0)
						newSL = 0;
					else
						newSL = fabs(pParams->bidAsk.bid[0] - pParams->orderInfo[i].stopLoss);
				}
			}

			//������ֹ�������ƶ�
			if (stopMovingbackSL == TRUE && newSL > fabs(pParams->bidAsk.bid[0] - pParams->orderInfo[i].stopLoss))
				newSL = fabs(pParams->bidAsk.bid[0] - pParams->orderInfo[i].stopLoss);

			modifyTradeEasy(SELL, pParams->orderInfo[i].ticket, newSL, newTP);
		}
	}

	return SUCCESS;
}


AsirikuyReturnCode EasyTrade::modifyAllShorts(double stopLoss,double takePrice,int tpMode,BOOL stopMovingbackSL)
{
	int i;
	double newTP = takePrice;
	double newSL = stopLoss;
	for (i = 0; i < pParams->settings[ORDERINFO_ARRAY_SIZE]; i++)
	{	
		newTP = takePrice;
		newSL = stopLoss;

		if (pParams->orderInfo[i].type == SELL && pParams->orderInfo[i].ticket != 0 && pParams->orderInfo[i].isOpen)
		{
			logInfo("ModifyAllShorts type = %d, ticket = %d", (int)pParams->orderInfo[i].type, (int)pParams->orderInfo[i].ticket);

			if (tpMode == 1 && pParams->orderInfo[i].openPrice - pParams->bidAsk.ask[0] >= 0) // New day and break event
				newTP = 0;
			else
			{
				if (takePrice == -1) // No change
				{
					if (pParams->orderInfo[i].takeProfit == 0)
						newTP = 0;
					else
						newTP = fabs(pParams->orderInfo[i].takeProfit - pParams->bidAsk.bid[0]);
				}
				if (stopLoss == -1)
				{
					if (pParams->orderInfo[i].stopLoss == 0)
						newSL = 0;
					else
						newSL = fabs(pParams->bidAsk.bid[0] - pParams->orderInfo[i].stopLoss);
				}
			}

			//������ֹ�������ƶ�
			if (stopMovingbackSL == TRUE && newSL > fabs(pParams->bidAsk.bid[0] - pParams->orderInfo[i].stopLoss))
				newSL = fabs(pParams->bidAsk.bid[0] - pParams->orderInfo[i].stopLoss);

		//if (newTP > fabs(pParams->bidAsk.bid[0] - pParams->orderInfo[i].takeProfit))
		//	newTP = fabs(pParams->bidAsk.bid[0] - pParams->orderInfo[i].takeProfit);

			modifyTradeEasy(SELL, pParams->orderInfo[i].ticket, newSL, newTP);
		}
	}

	return SUCCESS;
}

double EasyTrade::isSamePriceBuyStopOrder(double entryPrice, time_t currentTime,double gap)
{
	int i;

	for (i = 0; i < pParams->settings[ORDERINFO_ARRAY_SIZE]; i++)
	{
		if (pParams->orderInfo[i].ticket != 0 && pParams->orderInfo[i].type == BUYSTOP && pParams->orderInfo[i].isOpen)
		{			
			if (fabs(entryPrice - pParams->orderInfo[i].openPrice) <= gap)
			{
				return TRUE;
			}
		}
	}

	return FALSE;
}

double EasyTrade::isSamePriceSellStopOrder(double entryPrice, time_t currentTime, double gap)
{
	int i;
	
	for (i = 0; i < pParams->settings[ORDERINFO_ARRAY_SIZE]; i++)
	{
		if (pParams->orderInfo[i].ticket != 0 && pParams->orderInfo[i].type == SELLSTOP && pParams->orderInfo[i].isOpen)
		{			
			if (fabs(entryPrice - pParams->orderInfo[i].openPrice) <= gap)
			{
				return TRUE;
			}
		}
	}

	return FALSE;
}

double EasyTrade::isSamePriceBuyLimitOrder(double entryPrice, time_t currentTime, double gap)
{
	int i;
	
	for (i = 0; i < pParams->settings[ORDERINFO_ARRAY_SIZE]; i++)
	{
		if (pParams->orderInfo[i].ticket != 0 && pParams->orderInfo[i].type == BUYLIMIT && pParams->orderInfo[i].isOpen)
		{			
			if (fabs(entryPrice - pParams->orderInfo[i].openPrice) <= gap)
			{
				return TRUE;
			}
		}
	}

	return FALSE;
}

double EasyTrade::isSamePriceSellLimitOrder(double entryPrice, time_t currentTime,double gap)
{
	int i;
	for (i = 0; i < pParams->settings[ORDERINFO_ARRAY_SIZE]; i++)
	{
		if (pParams->orderInfo[i].ticket != 0 && pParams->orderInfo[i].type == SELLLIMIT && pParams->orderInfo[i].isOpen)
		{			
			if (fabs(entryPrice - pParams->orderInfo[i].openPrice) <= gap)
			{
				return TRUE;
			}
		}
	}

	return FALSE;
}

double EasyTrade::adjustTakePrice_Weekly_Swing(int ratesArrayIndex,double init_takePrice)
{
	TA_RetCode retCode;
	int i, hours;
	time_t openTime;
	int shift0Index = pParams->ratesBuffers->rates[ratesArrayIndex].info.arraySize - 1;
	int shift1Index = pParams->ratesBuffers->rates[ratesArrayIndex].info.arraySize - 2;
	struct tm timeInfo;	
	time_t currentTime = pParams->ratesBuffers->rates[ratesArrayIndex].time[shift0Index];
	int outBegIdx, outNBElement;
	double low = -1,high = -1;
	double takePrice = -1;

	safe_gmtime(&timeInfo, currentTime);
	
	for (i = 0; i < pParams->settings[ORDERINFO_ARRAY_SIZE]; i++)
	{
		if (pParams->orderInfo[i].ticket != 0 && pParams->orderInfo[i].isOpen)
		{

			openTime = pParams->orderInfo[i].openTime;
			hours = (int)difftime(currentTime,openTime) / 3600;
			if (pParams->orderInfo[i].type == BUY)
			{
				retCode = TA_MIN(shift1Index, shift1Index, pParams->ratesBuffers->rates[ratesArrayIndex].low, hours, &outBegIdx, &outNBElement, &low);
				if (retCode == TA_SUCCESS)
					takePrice = init_takePrice + low - pParams->bidAsk.ask[0];
			}

			if (pParams->orderInfo[i].type == SELL)
			{
				retCode = TA_MAX(shift1Index, shift1Index, pParams->ratesBuffers->rates[ratesArrayIndex].high, hours, &outBegIdx, &outNBElement, &high);
				if (retCode == TA_SUCCESS)
					takePrice = pParams->bidAsk.bid[0] - (high - init_takePrice);
			}
			break;
		}
	}
	return takePrice;
}

AsirikuyReturnCode EasyTrade::getHighLow( int ratesArrayIndex, int shfitIndex, int shift, double *pHigh, double *pLow)
{
	TA_RetCode retCode;
	int outBegIdx, outNBElement;

	retCode = TA_MIN(shfitIndex, shfitIndex, pParams->ratesBuffers->rates[ratesArrayIndex].low, shift, &outBegIdx, &outNBElement, pLow);
	if (retCode != TA_SUCCESS)
	{
		return logTALibError("TA_MIN()", retCode);
	}

	retCode = TA_MAX(shfitIndex, shfitIndex, pParams->ratesBuffers->rates[ratesArrayIndex].high, shift, &outBegIdx, &outNBElement, pHigh);

	if (retCode != TA_SUCCESS)
	{
		return logTALibError("TA_MAX()", retCode);
	}
	return SUCCESS;
}

int EasyTrade::getOldestOpenOrderIndex(int rateIndex)
{
	int i;
	int    shift0Index = pParams->ratesBuffers->rates[rateIndex].info.arraySize - 1, shift1Index = pParams->ratesBuffers->rates[rateIndex].info.arraySize - 2;
	time_t openTime, maxTime = LLONG_MAX;
	struct tm timeInfo1;
	int  execution_tf = (int)pParams->settings[TIMEFRAME];
	int index = -1;

	time_t currentTime = pParams->ratesBuffers->rates[rateIndex].time[shift0Index];
	safe_gmtime(&timeInfo1, currentTime);

	for (i = 0; i < pParams->settings[ORDERINFO_ARRAY_SIZE]; i++)
	{
		if (pParams->orderInfo[i].ticket != 0 && pParams->orderInfo[i].isOpen == TRUE)
		{
			openTime = pParams->orderInfo[i].openTime;

			if (openTime < maxTime)
			{
				maxTime = openTime;
				index = i;
			}
		}
	}

	return index;
}

int EasyTrade::getLastestOrderIndexExceptLimitAndStopOrders(int rateIndex,BOOL isClosedOnly)
{
	int i;
	int    shift0Index = pParams->ratesBuffers->rates[rateIndex].info.arraySize - 1, shift1Index = pParams->ratesBuffers->rates[rateIndex].info.arraySize - 2;
	time_t openTime, maxTime = 0;
	struct tm timeInfo1;
	int  execution_tf = (int)pParams->settings[TIMEFRAME];
	int index = -1;

	time_t currentTime = pParams->ratesBuffers->rates[rateIndex].time[shift0Index];
	safe_gmtime(&timeInfo1, currentTime);

	for (i = 0; i < pParams->settings[ORDERINFO_ARRAY_SIZE]; i++)
	{
		if (pParams->orderInfo[i].ticket != 0 
			&& (pParams->orderInfo[i].type == BUY || pParams->orderInfo[i].type == SELL)
			&& (isClosedOnly == FALSE || pParams->orderInfo[i].isOpen == FALSE)
			)
		{
			openTime = pParams->orderInfo[i].openTime;

			if (openTime > maxTime)
			{
				maxTime = openTime;
				index = i;
			}
		}
	}

	return index;
}

int EasyTrade::getLastestOrderIndex(int rateIndex)
{
	int i;
	int    shift0Index = pParams->ratesBuffers->rates[rateIndex].info.arraySize - 1, shift1Index = pParams->ratesBuffers->rates[rateIndex].info.arraySize - 2;
	time_t openTime, maxTime = 0;
	struct tm timeInfo1;
	int  execution_tf = (int)pParams->settings[TIMEFRAME];
	int index = -1;
	
	time_t currentTime = pParams->ratesBuffers->rates[rateIndex].time[shift0Index];
	safe_gmtime(&timeInfo1, currentTime);

	for (i = 0; i < pParams->settings[ORDERINFO_ARRAY_SIZE]; i++)
	{
		if (pParams->orderInfo[i].ticket != 0)
		{
			openTime = pParams->orderInfo[i].openTime;

			if (openTime > maxTime)
			{
				maxTime = openTime;
				index = i;
			}
		}
	}

	return index;
}

double EasyTrade::getLastestOrderPrice(int rateIndex, BOOL * pIsOpen)
{
	int i;
	int    shift0Index = pParams->ratesBuffers->rates[rateIndex].info.arraySize - 1, shift1Index = pParams->ratesBuffers->rates[rateIndex].info.arraySize - 2;
	time_t openTime, maxTime = 0;
	struct tm timeInfo1;
	int  execution_tf = (int)pParams->settings[TIMEFRAME];
	int index = -1;
	double openPrice = 0;

	time_t currentTime = pParams->ratesBuffers->rates[rateIndex].time[shift0Index];
	safe_gmtime(&timeInfo1, currentTime);

	for (i = 0; i < pParams->settings[ORDERINFO_ARRAY_SIZE]; i++)
	{
		if (pParams->orderInfo[i].ticket != 0)
		{
			openTime = pParams->orderInfo[i].openTime;

			if (openTime > maxTime)
			{
				maxTime = openTime;
				index = i;
			}
		}
	}

	*pIsOpen = FALSE;

	if (index >= 0)
	{
		*pIsOpen = pParams->orderInfo[index].isOpen;
		openPrice = pParams->orderInfo[index].openPrice;
	}

	return openPrice;
}

OrderType EasyTrade::getLastestOrderType(int rateIndex, double *pHigh, double *pLow, BOOL * pIsOpen)
{
	int i;
	int    shift0Index = pParams->ratesBuffers->rates[rateIndex].info.arraySize - 1, shift1Index = pParams->ratesBuffers->rates[rateIndex].info.arraySize - 2;
	time_t openTime, maxTime = 0;
	struct tm timeInfo1, timeInfo2;
	int  execution_tf = (int)pParams->settings[TIMEFRAME];
	int minGap, hourGap, openIndex, count, index = -1;
	OrderType orderType = NONE;

	time_t currentTime = pParams->ratesBuffers->rates[rateIndex].time[shift0Index];
	safe_gmtime(&timeInfo1, currentTime);

	for (i = 0; i < pParams->settings[ORDERINFO_ARRAY_SIZE]; i++)
	{
		if (pParams->orderInfo[i].ticket != 0)
		{
			openTime = pParams->orderInfo[i].openTime;

			if (openTime > maxTime)
			{
				maxTime = openTime;
				index = i;
			}
		}
	}

	*pIsOpen = FALSE;

	if (index >= 0)
	{
		*pIsOpen = pParams->orderInfo[index].isOpen;

		openTime = pParams->orderInfo[index].openTime;
		safe_gmtime(&timeInfo2, openTime);
		minGap = (timeInfo1.tm_min - timeInfo2.tm_min) / execution_tf;
		hourGap = (timeInfo1.tm_hour - timeInfo2.tm_hour) * (60 / execution_tf);

		openIndex = shift1Index;
		count = hourGap + minGap-1;
		if (count > 1)
			getHighLow(rateIndex, openIndex, count, pHigh, pLow);
		else
		{
			*pHigh = -100000;
			*pLow = 100000;
		}
		orderType = pParams->orderInfo[index].type;
	}

	return orderType;
}

OrderType EasyTrade::getLastestOrderType_XAUUSD(int rateIndex, double *pHigh, double *pLow,BOOL * pIsOpen)
{
	int i;
	int    shift0Index = pParams->ratesBuffers->rates[rateIndex].info.arraySize - 1, shift1Index = pParams->ratesBuffers->rates[rateIndex].info.arraySize - 2;
	time_t openTime, maxTime = 0;
	struct tm timeInfo1, timeInfo2;
	int  execution_tf = (int)pParams->settings[TIMEFRAME];
	int minGap, hourGap, openIndex, count,index = -1;
	OrderType orderType = NONE;

	time_t currentTime = pParams->ratesBuffers->rates[rateIndex].time[shift0Index];
	safe_gmtime(&timeInfo1, currentTime);

	for (i = 0; i < pParams->settings[ORDERINFO_ARRAY_SIZE]; i++)
	{
		if (pParams->orderInfo[i].ticket != 0)
		{
			openTime = pParams->orderInfo[i].openTime;
			
			if (openTime > maxTime)
			{
				maxTime = openTime;
				index = i;
			}			
		}
	}

	*pIsOpen = FALSE;

	if (index >= 0)
	{
		*pIsOpen = pParams->orderInfo[index].isOpen;

		openTime = pParams->orderInfo[index].openTime;
		safe_gmtime(&timeInfo2, openTime);
		minGap = int((timeInfo1.tm_min - timeInfo2.tm_min) / execution_tf + 0.5);
		hourGap = (timeInfo1.tm_hour - timeInfo2.tm_hour) * (60 / execution_tf);

		openIndex = shift1Index - (hourGap + minGap);
		count = (timeInfo2.tm_hour - 1) * (60 / execution_tf) + (int)(timeInfo2.tm_min / execution_tf) -1;

		if (count > 1)
			getHighLow(rateIndex, openIndex, count, pHigh, pLow);
		else
		{
			*pHigh = pParams->orderInfo[index].openPrice;
			*pLow = pParams->orderInfo[index].openPrice;
		}
		orderType = pParams->orderInfo[index].type;
	}

	return orderType;
}

//OrderType EasyTrade::getLastestOpenOrderType_XAUUSD(int rateIndex, double *pHigh, double *pLow)
//{
//	int i;
//	int    shift0Index = pParams->ratesBuffers->rates[rateIndex].info.arraySize - 1, shift1Index = pParams->ratesBuffers->rates[rateIndex].info.arraySize - 2;
//	time_t openTime;
//	struct tm timeInfo1, timeInfo2;
//	int  execution_tf = (int)pParams->settings[TIMEFRAME];
//	int minGap, hourGap, openIndex, count;
//
//	time_t currentTime = pParams->ratesBuffers->rates[rateIndex].time[shift0Index];
//	safe_gmtime(&timeInfo1, currentTime);
//
//	for (i = 0; i < pParams->settings[ORDERINFO_ARRAY_SIZE]; i++)
//	{
//		if (pParams->orderInfo[i].ticket != 0 && pParams->orderInfo[i].isOpen)
//		{
//			openTime = pParams->orderInfo[i].openTime;
//			safe_gmtime(&timeInfo2, pParams->orderInfo[i].openTime);
//			minGap = (timeInfo1.tm_min - timeInfo2.tm_min) / execution_tf;
//			hourGap = (timeInfo1.tm_hour - timeInfo2.tm_hour) * (60 / execution_tf);
//
//			openIndex = shift1Index;
//			count = hourGap + minGap - 1;
//			if (count > 1)
//				getHighLow(rateIndex, openIndex, count, pHigh, pLow);
//			else
//			{
//				*pHigh = -1000;
//				*pLow = 1000;
//			}
//
//			return pParams->orderInfo[i].type;
//
//		}
//	}
//
//	return NONE;
//}

OrderType EasyTrade::getLastestOpenOrderType_GBPJPY(int rateIndex, double *pHigh, double *pLow)
{
	int i;
	int    shift0Index = pParams->ratesBuffers->rates[rateIndex].info.arraySize - 1, shift1Index = pParams->ratesBuffers->rates[rateIndex].info.arraySize - 2;
	time_t openTime;
	struct tm timeInfo1, timeInfo2;
	int  execution_tf = (int)pParams->settings[TIMEFRAME];
	int minGap, hourGap, openIndex, count;

	time_t currentTime = pParams->ratesBuffers->rates[rateIndex].time[shift0Index];
	safe_gmtime(&timeInfo1, currentTime);

	for (i = 0; i < pParams->settings[ORDERINFO_ARRAY_SIZE]; i++)
	{
		if (pParams->orderInfo[i].ticket != 0 && pParams->orderInfo[i].isOpen)
		{
			openTime = pParams->orderInfo[i].openTime;
			safe_gmtime(&timeInfo2, pParams->orderInfo[i].openTime);
			minGap = (timeInfo1.tm_min - timeInfo2.tm_min) / execution_tf;
			hourGap = (timeInfo1.tm_hour - timeInfo2.tm_hour) * (60 / execution_tf);

			openIndex = shift1Index;
			count = hourGap + minGap;
			if (count > 1)
				getHighLow(rateIndex, openIndex, count, pHigh, pLow);
			else
			{
				*pHigh = -100000;
				*pLow = 100000;
			}

			return pParams->orderInfo[i].type;

		}
	}

	return NONE;
}

//XAUUSD only
OrderType EasyTrade::getLastestOpenOrderType(int rateIndex,double *pHigh, double *pLow)
{
	int i;	
	int    shift0Index = pParams->ratesBuffers->rates[rateIndex].info.arraySize - 1, shift1Index = pParams->ratesBuffers->rates[rateIndex].info.arraySize - 2;
	time_t openTime;
	struct tm timeInfo1, timeInfo2;
	int  execution_tf = (int)pParams->settings[TIMEFRAME];
	int minGap, hourGap, openIndex, count;

	time_t currentTime = pParams->ratesBuffers->rates[rateIndex].time[shift0Index];
	safe_gmtime(&timeInfo1, currentTime);

	for (i = 0; i < pParams->settings[ORDERINFO_ARRAY_SIZE]; i++)
	{
		if (pParams->orderInfo[i].ticket != 0 && pParams->orderInfo[i].isOpen)
		{
			openTime = pParams->orderInfo[i].openTime;
			safe_gmtime(&timeInfo2, pParams->orderInfo[i].openTime);
			minGap = int( (timeInfo1.tm_min - timeInfo2.tm_min) / execution_tf + 0.5);
			hourGap = (timeInfo1.tm_hour - timeInfo2.tm_hour) * (60 / execution_tf);

			openIndex = shift1Index - (hourGap + minGap);
			count = (timeInfo2.tm_hour - 1) * (60 / execution_tf) + (int)(timeInfo2.tm_min / execution_tf) ;

			if (count > 1)
				getHighLow(rateIndex, openIndex, count, pHigh, pLow);
			else
			{
				*pHigh = -100000;
				*pLow = 100000;
			}
			return pParams->orderInfo[i].type;
			
		}
	}

	return NONE;
}

double EasyTrade::hasSameDayDayTradingOrder(int rateIndex, OrderInfo * pOrder, double * pHigh, double * pLow)
{	
	int i;
	int    shift0Index = pParams->ratesBuffers->rates[rateIndex].info.arraySize - 1, shift1Index = pParams->ratesBuffers->rates[rateIndex].info.arraySize - 2;
	time_t openTime, maxTime = 0;
	struct tm timeInfo1, timeInfo2;
	int  execution_tf = (int)pParams->settings[TIMEFRAME];
	int minGap, hourGap, openIndex, count, index = -1;
	OrderType orderType = NONE;

	time_t currentTime = pParams->ratesBuffers->rates[rateIndex].time[shift0Index];
	safe_gmtime(&timeInfo1, currentTime);

	for (i = 0; i < pParams->settings[ORDERINFO_ARRAY_SIZE]; i++)
	{
		if (pParams->orderInfo[i].ticket != 0 && pParams->orderInfo[i].isOpen && pParams->orderInfo[i].takeProfit != 0)
		{
			openTime = pParams->orderInfo[i].openTime;
			safe_gmtime(&timeInfo2, openTime);

			if (timeInfo1.tm_year == timeInfo2.tm_year && timeInfo1.tm_yday == timeInfo2.tm_yday)
			{
				pOrder = &(pParams->orderInfo[i]);
				index = i;
				break;
			}
		}
	}

	if (index >= 0)
	{
		openTime = pParams->orderInfo[index].openTime;
		safe_gmtime(&timeInfo2, openTime);
		minGap = int((timeInfo1.tm_min - timeInfo2.tm_min) / execution_tf + 0.5);
		hourGap = (timeInfo1.tm_hour - timeInfo2.tm_hour) * (60 / execution_tf);

		openIndex = shift1Index - (hourGap + minGap);
		count = (timeInfo2.tm_hour - 1) * (60 / execution_tf) + (int)(timeInfo2.tm_min / execution_tf) - 1;
		if(count > 1)
			getHighLow(rateIndex, openIndex, count, pHigh, pLow);
		else
		{
			*pHigh = -100000;
			*pLow = 100000;
		}
		orderType = pParams->orderInfo[index].type;
	}

	return FALSE;
}

double EasyTrade::isSameDaySamePricePendingOrder_with_TP(double entryPrice, double limit, time_t currentTime)
{
	int i;
	struct tm timeInfo1, timeInfo2;
	safe_gmtime(&timeInfo1, currentTime);


	for (i = 0; i < pParams->settings[ORDERINFO_ARRAY_SIZE]; i++)
	{
		if (pParams->orderInfo[i].ticket != 0 && pParams->orderInfo[i].isOpen && pParams->orderInfo[i].takeProfit != 0)
		{
			safe_gmtime(&timeInfo2, pParams->orderInfo[i].openTime);
			if (fabs(entryPrice - pParams->orderInfo[i].openPrice) < limit && timeInfo1.tm_year == timeInfo2.tm_year && timeInfo1.tm_yday == timeInfo2.tm_yday)
			{
				return TRUE;
			}
		}
	}

	return FALSE;
}

int EasyTrade::hasSameWeekOrder(time_t currentTime, BOOL *pIsOpen)
{
	int i;
	int monday, friday, current;
	struct tm timeInfo1, timeInfo2;
	BOOL isCrossNewYear = FALSE;

	safe_gmtime(&timeInfo1, currentTime);

	monday = timeInfo1.tm_yday - timeInfo1.tm_wday + 1;
	friday = timeInfo1.tm_yday - timeInfo1.tm_wday + 5;
	if (monday < 0)
	{
		isCrossNewYear = TRUE;
		monday += 365;
		friday += 365;
	}

	for (i = 0; i < pParams->settings[ORDERINFO_ARRAY_SIZE]; i++)
	{
		if (pParams->orderInfo[i].ticket != 0)
		{
			safe_gmtime(&timeInfo2, pParams->orderInfo[i].openTime);
			current = timeInfo2.tm_yday;
			
			if (isCrossNewYear == FALSE && timeInfo2.tm_year != timeInfo1.tm_year)
				continue;

			if (isCrossNewYear && timeInfo2.tm_yday < 7)
				current += 365;

			if (current >= monday && current <= friday)
			{
				*pIsOpen = pParams->orderInfo[i].isOpen;
				return TRUE;				
			}
		}
	}

	return FALSE;
}

int EasyTrade::hasSameDayOrderExcludeBreakeventOrders(time_t currentTime, BOOL *pIsOpen,double points)
{
	int i;
	struct tm timeInfo1, timeInfo2;
	safe_gmtime(&timeInfo1, currentTime);

	*pIsOpen = FALSE;
	for (i = 0; i < pParams->settings[ORDERINFO_ARRAY_SIZE]; i++)
	{
		if (pParams->orderInfo[i].ticket != 0)
		{
			safe_gmtime(&timeInfo2, pParams->orderInfo[i].openTime);
			if (timeInfo1.tm_year == timeInfo2.tm_year &&  timeInfo1.tm_mon == timeInfo2.tm_mon && timeInfo1.tm_mday == timeInfo2.tm_mday)
			{
				if (pParams->orderInfo[i].isOpen == FALSE && pParams->orderInfo[i].profit< 0 && fabs(pParams->orderInfo[i].openPrice - pParams->orderInfo[i].closePrice) <= points)
					continue;
				*pIsOpen = pParams->orderInfo[i].isOpen;
				return TRUE;
			}
		}
	}

	return FALSE;
}

int EasyTrade::hasSameDayOrder( time_t currentTime,BOOL *pIsOpen)
{
	int i;
	struct tm timeInfo1, timeInfo2;
	safe_gmtime(&timeInfo1, currentTime);

	*pIsOpen = FALSE;
	for (i = 0; i < pParams->settings[ORDERINFO_ARRAY_SIZE]; i++)
	{
		if (pParams->orderInfo[i].ticket != 0)
		{
			safe_gmtime(&timeInfo2, pParams->orderInfo[i].openTime);
			if (timeInfo1.tm_year == timeInfo2.tm_year &&  timeInfo1.tm_mon == timeInfo2.tm_mon && timeInfo1.tm_mday == timeInfo2.tm_mday)
			{
				*pIsOpen = pParams->orderInfo[i].isOpen;
				return TRUE;
			}
		}
	}

	return FALSE;
}

double EasyTrade::isSameDaySamePricePendingOrder(double entryPrice, double limit, time_t currentTime)
{
	int i;
	struct tm timeInfo1, timeInfo2;	
	safe_gmtime(&timeInfo1, currentTime);
	

	for (i = 0; i < pParams->settings[ORDERINFO_ARRAY_SIZE]; i++)
	{
		if (pParams->orderInfo[i].ticket != 0 && pParams->orderInfo[i].isOpen)
		{
			safe_gmtime(&timeInfo2, pParams->orderInfo[i].openTime);
			if (fabs(entryPrice - pParams->orderInfo[i].openPrice) < limit && timeInfo1.tm_mday == timeInfo2.tm_mday)
			{				
				return TRUE;
			}
		}
	}

	return FALSE;
}

int EasyTrade::getLossTimesInWeek(time_t currentTime, double * total_lost_pips)
{
	int i;
	int lossTimes = 0;
	int monday, friday, current;
	struct tm timeInfo1, timeInfo2;
	BOOL isCrossNewYear = FALSE;
	*total_lost_pips = 0;

	safe_gmtime(&timeInfo1, currentTime);

	monday = timeInfo1.tm_yday - timeInfo1.tm_wday + 1;
	friday = timeInfo1.tm_yday - timeInfo1.tm_wday + 5;
	if (monday < 0)
	{
		isCrossNewYear = TRUE;
		monday += 365;
		friday += 365;
	}

	for (i = 0; i < pParams->settings[ORDERINFO_ARRAY_SIZE]; i++)
	{		
		if (pParams->orderInfo[i].ticket != 0)
		{
			safe_gmtime(&timeInfo2, pParams->orderInfo[i].openTime);
			current = timeInfo2.tm_yday;

			if (isCrossNewYear == FALSE && timeInfo2.tm_year != timeInfo1.tm_year)
				continue;

			if (isCrossNewYear && timeInfo2.tm_yday < 7)
				current += 365;

			if (current >= monday && current <= friday)
			{
				if (pParams->orderInfo[i].isOpen == FALSE && pParams->orderInfo[i].profit < 0)
				{
					*total_lost_pips += fabs(pParams->orderInfo[i].closePrice - pParams->orderInfo[i].openPrice) * pParams->orderInfo[i].lots;
					lossTimes++;
				}

				if (pParams->orderInfo[i].isOpen == TRUE)
				{
					if (pParams->orderInfo[i].type == BUY && pParams->bidAsk.ask[0] < pParams->orderInfo[i].openPrice)
					{
						*total_lost_pips += fabs(pParams->bidAsk.bid[0] - pParams->orderInfo[i].openPrice) * pParams->orderInfo[i].lots;
						lossTimes++;
					}
					if (pParams->orderInfo[i].type == SELL && pParams->bidAsk.bid[0] > pParams->orderInfo[i].openPrice)
					{
						*total_lost_pips += fabs(pParams->bidAsk.bid[0] - pParams->orderInfo[i].openPrice) * pParams->orderInfo[i].lots;
						lossTimes++;
					}
				}

			}

			
		}
		
	}

	return lossTimes;
}

int EasyTrade::getWinTimesInWeek(time_t currentTime)
{
	int i;
	int winningTimes = 0;
	int monday, friday, current;
	struct tm timeInfo1, timeInfo2;
	BOOL isCrossNewYear = FALSE;

	safe_gmtime(&timeInfo1, currentTime);

	monday = timeInfo1.tm_yday - timeInfo1.tm_wday + 1;
	friday = timeInfo1.tm_yday - timeInfo1.tm_wday + 5;
	if (monday < 0)
	{
		isCrossNewYear = TRUE;
		monday += 365;
		friday += 365;
	}
	
	for (i = 0; i < pParams->settings[ORDERINFO_ARRAY_SIZE]; i++)
	{
		// Close winning trades.
		if (pParams->orderInfo[i].ticket != 0)
		{
			safe_gmtime(&timeInfo2, pParams->orderInfo[i].openTime);
			current = timeInfo2.tm_yday;

			if (isCrossNewYear == FALSE && timeInfo2.tm_year != timeInfo1.tm_year)
				continue;

			if (isCrossNewYear && timeInfo2.tm_yday < 7)
				current += 365;

			if (current >= monday && current <= friday)
			{

				if (pParams->orderInfo[i].isOpen == FALSE && pParams->orderInfo[i].profit > 0)
					winningTimes++;					

				if (pParams->orderInfo[i].isOpen == TRUE)
				{
					if (pParams->orderInfo[i].type == BUY && pParams->bidAsk.ask[0] > pParams->orderInfo[i].takeProfit)
						winningTimes++;

					if (pParams->orderInfo[i].type == SELL && pParams->bidAsk.bid[0] < pParams->orderInfo[i].takeProfit)
						winningTimes++;
				}
			}
		}
		
	}

	return winningTimes;
}


int EasyTrade::getLossTimesInPreviousDays(time_t currentTime,double *pTotal_Lost_Pips)
{
	int i;
	time_t time = currentTime;	
	int winTimes = 0,lostTimes = 0;
	double lost_pips = 0;
	int lostDays = 0;
	struct tm timeInfo1;

	*pTotal_Lost_Pips = 0;

	for (i = 0; i < 200; i++)
	{
		currentTime -= 60 * 60 * 24;
		safe_gmtime(&timeInfo1, currentTime);

		winTimes = getWinTimesInDay(currentTime);
		lostTimes = getLossTimesInDay(currentTime, &lost_pips);
		if (winTimes == 0 && lostTimes > 0)
		{
			lostDays++;
			*pTotal_Lost_Pips += lost_pips;

		}

		if (winTimes > 0)
			break;	
	}

	return lostDays;
}

int EasyTrade::getLastestOrderIndexExcept(int rateIndex, int exceptIndexs[])
{
	int i,j;
	int    shift0Index = pParams->ratesBuffers->rates[rateIndex].info.arraySize - 1, shift1Index = pParams->ratesBuffers->rates[rateIndex].info.arraySize - 2;
	time_t openTime, maxTime = 0;
	struct tm timeInfo1;
	int  execution_tf = (int)pParams->settings[TIMEFRAME];
	int index = -1;
	BOOL isFound = FALSE;

	time_t currentTime = pParams->ratesBuffers->rates[rateIndex].time[shift0Index];
	safe_gmtime(&timeInfo1, currentTime);

	for (i = 0; i < pParams->settings[ORDERINFO_ARRAY_SIZE]; i++)
	{
		isFound = FALSE;
		for (j = 0; j < 7; j++)
		{
			if (i == exceptIndexs[j])
			{
				isFound = TRUE;
				break;
			}
		}

		if (isFound == TRUE)
			continue;
		

		if (pParams->orderInfo[i].ticket != 0)
		{
			openTime = pParams->orderInfo[i].openTime;

			if (openTime > maxTime)
			{
				maxTime = openTime;
				index = i;
			}
		}
	}

	return index;
}

int EasyTrade::hasBigWinInCurrentTrend(int rateIndex, OrderType type,double points)
{	
	struct tm timeInfo1;
	int index1, index2, index3, index4;
	int	exceptIndexs[3] = {};

	index1 = getLastestOrderIndex(rateIndex);
	if (index1 >= 0)
	{
		//big win
		if (pParams->orderInfo[index1].type == type
			&& pParams->orderInfo[index1].profit > 0 && abs(pParams->orderInfo[index1].closePrice - pParams->orderInfo[index1].openPrice) > points)
			return TRUE;
		else
			return FALSE;
	}
	else
		return FALSE;

	exceptIndexs[0] = index1;

	index2 = getLastestOrderIndexExcept(rateIndex, exceptIndexs);
	if (index2 >= 0)
	{
		if (pParams->orderInfo[index2].type == type
			&& pParams->orderInfo[index2].profit > 0 && abs(pParams->orderInfo[index2].closePrice - pParams->orderInfo[index2].openPrice) > points)
			return TRUE;
		else
			return FALSE;
	}
	else
		return FALSE;

	exceptIndexs[1] = index2;

	index3 = getLastestOrderIndexExcept(rateIndex, exceptIndexs);
	if (index3 >= 0)
	{
		if (pParams->orderInfo[index3].type == type
			&& pParams->orderInfo[index3].profit > 0 && abs(pParams->orderInfo[index3].closePrice - pParams->orderInfo[index3].openPrice) > points)
			return TRUE;
		else
			return FALSE;
	}
	else
		return FALSE;

	exceptIndexs[2] = index3;

	index4 = getLastestOrderIndexExcept(rateIndex, exceptIndexs);
	if (index4 >= 0)
	{
		if (pParams->orderInfo[index4].type == type
			&& pParams->orderInfo[index4].profit > 0 && abs(pParams->orderInfo[index4].closePrice - pParams->orderInfo[index4].openPrice) > points)
			return TRUE;
		else
			return FALSE;
	}
	else
		return FALSE;

	return TRUE;
}

int EasyTrade::getSameSideWonTradesInCurrentTrend(int rateIndex, OrderType type)
{
	int sameSideTimes = 0;
	struct tm timeInfo1;
	int index1, index2, index3, index4, index5, index6, index7, index8;
	int	exceptIndexs[7] = {};
		
	index1 = getLastestOrderIndex(rateIndex);
	if (index1 >= 0)
	{
		if (pParams->orderInfo[index1].type == type)
		{
			if (pParams->orderInfo[index1].profit > 0)
				sameSideTimes++;
		}
		else
			return sameSideTimes;
	}
	else
		return sameSideTimes;

	exceptIndexs[0] = index1;

	index2 = getLastestOrderIndexExcept(rateIndex, exceptIndexs);
	if (index2 >= 0)
	{
		if (pParams->orderInfo[index2].type == type)
		{
			if (pParams->orderInfo[index2].profit > 0)
				sameSideTimes++;
		}			
		else
			return sameSideTimes;
	}
	else
		return sameSideTimes;

	exceptIndexs[1] = index2;

	index3 = getLastestOrderIndexExcept(rateIndex, exceptIndexs);
	if (index3 >= 0)
	{
		if (pParams->orderInfo[index3].type == type)
		{
			if (pParams->orderInfo[index3].profit > 0)
				sameSideTimes++;
		}			
		else
			return sameSideTimes;
	}
	else
		return sameSideTimes;

	exceptIndexs[2] = index3;

	index4 = getLastestOrderIndexExcept(rateIndex, exceptIndexs);
	if (index4 >= 0)
	{
		if (pParams->orderInfo[index4].type == type)
		{
			if (pParams->orderInfo[index4].profit > 0)
				sameSideTimes++;
		}		
		else
			return sameSideTimes;
	}
	else
		return sameSideTimes;

	exceptIndexs[3] = index4;
	index5 = getLastestOrderIndexExcept(rateIndex, exceptIndexs);
	if (index5 >= 0)
	{
		if (pParams->orderInfo[index5].type == type)
		{
			if (pParams->orderInfo[index5].profit > 0)
				sameSideTimes++;
		}
		else
			return sameSideTimes;
	}
	else
		return sameSideTimes;

	exceptIndexs[4] = index5;
	index6 = getLastestOrderIndexExcept(rateIndex, exceptIndexs);
	if (index6 >= 0)
	{
		if (pParams->orderInfo[index6].type == type)
		{
			if (pParams->orderInfo[index6].profit > 0)
				sameSideTimes++;
		}
		else
			return sameSideTimes;
	}
	else
		return sameSideTimes;

	exceptIndexs[5] = index6;
	index7 = getLastestOrderIndexExcept(rateIndex, exceptIndexs);
	if (index7 >= 0)
	{
		if (pParams->orderInfo[index7].type == type)
		{
			if (pParams->orderInfo[index7].profit > 0)
				sameSideTimes++;
		}
		else
			return sameSideTimes;
	}
	else
		return sameSideTimes;

	exceptIndexs[6] = index7;
	index8 = getLastestOrderIndexExcept(rateIndex, exceptIndexs);
	if (index8 >= 0)
	{
		if (pParams->orderInfo[index8].type == type)
		{
			if (pParams->orderInfo[index8].profit > 0)
				sameSideTimes++;
		}
		else
			return sameSideTimes;
	}
	else
		return sameSideTimes;

	return sameSideTimes;
}

int EasyTrade::getSameSideTradesInCurrentTrend(int rateIndex, OrderType type)
{		
	int sameSideTimes = 0;	
	struct tm timeInfo1;
	int index1,index2, index3,index4;		
	int	exceptIndexs[3] = {};

	index1 = getLastestOrderIndex(rateIndex);
	if (index1 >= 0)
	{
		if (pParams->orderInfo[index1].type == type)
			sameSideTimes++;
		else
			return sameSideTimes;
	}
	else
		return sameSideTimes;

	exceptIndexs[0] = index1;

	index2 = getLastestOrderIndexExcept(rateIndex, exceptIndexs);
	if (index2 >= 0)
	{
		if (pParams->orderInfo[index2].type == type)
			sameSideTimes++;
		else
			return sameSideTimes;
	}
	else
		return sameSideTimes;

	exceptIndexs[1] = index2;

	index3 = getLastestOrderIndexExcept(rateIndex, exceptIndexs);
	if (index3 >= 0)
	{
		if (pParams->orderInfo[index3].type == type)
			sameSideTimes++;
		else
			return sameSideTimes;
	}
	else
		return sameSideTimes;

	exceptIndexs[2] = index3;

	index4 = getLastestOrderIndexExcept(rateIndex, exceptIndexs);
	if (index4 >= 0)
	{
		if (pParams->orderInfo[index4].type == type)
			sameSideTimes++;
		else
			return sameSideTimes;
	}
	else
		return sameSideTimes;

	return sameSideTimes;
}

int EasyTrade::getLossTimesFromNow(time_t currentTime, double * pTotal_Lost_Pips)
{
	int i;
	time_t time = currentTime;
	int winTimes = 0, lostTimes = 0;
	double lost_pips = 0;
	int lostDays = 0;
	struct tm timeInfo1;

	*pTotal_Lost_Pips = 0;

	//loop through 300 days
	for (i = 0; i < 300; i++) 
	{		
		safe_gmtime(&timeInfo1, currentTime);

		winTimes = getWinTimesInDay(currentTime);
		lostTimes = getLossTimesInDay(currentTime, &lost_pips);
		if (winTimes == 0 && lostTimes > 0)
		{
			lostDays++;
			*pTotal_Lost_Pips += lost_pips;

		}

		if (winTimes > 0)
			break;

		currentTime -= 60 * 60 * 24;
	}

	return lostDays;
}

int EasyTrade::getOrderCount()
{
	int i;
	int count = 0;
	
	for (i = 0; i < pParams->settings[ORDERINFO_ARRAY_SIZE]; i++)
	{
		if (pParams->orderInfo[i].ticket != 0 && pParams->orderInfo[i].isOpen == TRUE)
			count++;
	}

	return count;
}

double EasyTrade::caculateFreeMargin()
{
	int i;
	double cost = 0;
	
	for (i = 0; i < pParams->settings[ORDERINFO_ARRAY_SIZE]; i++)
	{
		if (pParams->orderInfo[i].ticket != 0 && pParams->orderInfo[i].isOpen == TRUE)
		{
			cost += pParams->bidAsk.ask[0] * pParams->orderInfo[i].lots;
		}
	}

	return pParams->accountInfo.equity - cost;
	
}

int EasyTrade::getOrderCountTodayExcludeBreakeventOrders(time_t currentTime, double points)
{
	int i;
	int count = 0;
	struct tm timeInfo1, timeInfo2;


	safe_gmtime(&timeInfo1, currentTime);

	for (i = 0; i < pParams->settings[ORDERINFO_ARRAY_SIZE]; i++)
	{
		if (pParams->orderInfo[i].ticket != 0)
		{
			safe_gmtime(&timeInfo2, pParams->orderInfo[i].openTime);

			if (timeInfo1.tm_year == timeInfo2.tm_year && timeInfo1.tm_yday == timeInfo2.tm_yday)
			{
				if (pParams->orderInfo[i].isOpen == FALSE && pParams->orderInfo[i].profit< 0 && fabs(pParams->orderInfo[i].openPrice - pParams->orderInfo[i].closePrice) <= points)
					continue;
				count++;
			}
		}

	}

	return count;
}

int EasyTrade::getOrderCountToday(time_t currentTime)
{
	int i;
	int count = 0;
	struct tm timeInfo1, timeInfo2;
	

	safe_gmtime(&timeInfo1, currentTime);

	for (i = 0; i < pParams->settings[ORDERINFO_ARRAY_SIZE]; i++)
	{
		if (pParams->orderInfo[i].ticket != 0 && (pParams->orderInfo[i].type == BUY || pParams->orderInfo[i].type == SELL))
		{
			safe_gmtime(&timeInfo2, pParams->orderInfo[i].openTime);

			if (timeInfo1.tm_year == timeInfo2.tm_year && timeInfo1.tm_yday == timeInfo2.tm_yday)
			{
				count++;
			}
		}

	}

	return count;
}

int EasyTrade::getOrderCountForCurrentWeek(time_t currentTime)
{
	int i;
	int count = 0;
	struct tm timeInfo1, timeInfo2;
	int days = 0;

	safe_gmtime(&timeInfo1, currentTime);

	for (i = 0; i < pParams->settings[ORDERINFO_ARRAY_SIZE]; i++)
	{
		if (pParams->orderInfo[i].ticket != 0 && pParams->orderInfo[i].isOpen == TRUE && (pParams->orderInfo[i].type == BUY || pParams->orderInfo[i].type == SELL))
		{
			safe_gmtime(&timeInfo2, pParams->orderInfo[i].openTime);

			days = difftime(currentTime, pParams->orderInfo[i].openTime) / 60 / 60 / 24;

			if (days <= timeInfo1.tm_wday)
			{
				count++;
			}
		}

	}

	return count;
}

int EasyTrade::getLossTimesInDayExcludeBreakeventOrders(time_t currentTime, double * total_lost_pips,double points)
{
	int i;
	int lossTimes = 0;
	struct tm timeInfo1, timeInfo2;
	*total_lost_pips = 0;

	safe_gmtime(&timeInfo1, currentTime);

	for (i = 0; i < pParams->settings[ORDERINFO_ARRAY_SIZE]; i++)
	{
		if (pParams->orderInfo[i].ticket != 0)
		{
			safe_gmtime(&timeInfo2, pParams->orderInfo[i].openTime);

			if (timeInfo1.tm_year == timeInfo2.tm_year && timeInfo1.tm_yday == timeInfo2.tm_yday)
			{
				if (pParams->orderInfo[i].isOpen == FALSE && pParams->orderInfo[i].profit < 0 && fabs(pParams->orderInfo[i].closePrice - pParams->orderInfo[i].openPrice) >= points)
				{
					lossTimes++;
					*total_lost_pips += fabs(pParams->orderInfo[i].closePrice - pParams->orderInfo[i].openPrice) * pParams->orderInfo[i].lots;
				}

				if (pParams->orderInfo[i].isOpen == TRUE)
				{
					if (pParams->orderInfo[i].type == BUY && pParams->bidAsk.ask[0] < pParams->orderInfo[i].openPrice
						//&& fabs(pParams->bidAsk.ask[0] - pParams->orderInfo[i].openPrice) > 0.2
						)
					{
						lossTimes++;
						*total_lost_pips += fabs(pParams->bidAsk.ask[0] - pParams->orderInfo[i].openPrice)* pParams->orderInfo[i].lots;
					}

					if (pParams->orderInfo[i].type == SELL && pParams->bidAsk.bid[0] > pParams->orderInfo[i].openPrice
						//&& fabs(pParams->bidAsk.ask[0] - pParams->orderInfo[i].openPrice) > 0.2
						)
					{
						lossTimes++;
						*total_lost_pips += fabs(pParams->bidAsk.bid[0] - pParams->orderInfo[i].openPrice)* pParams->orderInfo[i].lots;
					}
				}

			}


		}

	}

	return lossTimes;
}

int EasyTrade::getLossTimesInDayCloseOrder(time_t currentTime, double * total_lost_pips)
{
	int i;
	int lossTimes = 0;
	struct tm timeInfo1, timeInfo2;
	*total_lost_pips = 0;

	safe_gmtime(&timeInfo1, currentTime);

	for (i = 0; i < pParams->settings[ORDERINFO_ARRAY_SIZE]; i++)
	{
		if (pParams->orderInfo[i].ticket != 0)
		{
			safe_gmtime(&timeInfo2, pParams->orderInfo[i].openTime);

			if (timeInfo1.tm_year == timeInfo2.tm_year && timeInfo1.tm_yday == timeInfo2.tm_yday)
			{
				if (pParams->orderInfo[i].isOpen == FALSE && pParams->orderInfo[i].profit < 0)
				{
					lossTimes++;
					*total_lost_pips += fabs(pParams->orderInfo[i].closePrice - pParams->orderInfo[i].openPrice) * pParams->orderInfo[i].lots;
				}

			}


		}

	}

	return lossTimes;
}

int EasyTrade::getLossTimesInDay(time_t currentTime,double * total_lost_pips)
{
	int i;
	int lossTimes = 0;	
	struct tm timeInfo1, timeInfo2;	
	*total_lost_pips = 0;

	safe_gmtime(&timeInfo1, currentTime);

	for (i = 0; i < pParams->settings[ORDERINFO_ARRAY_SIZE]; i++)
	{
		if (pParams->orderInfo[i].ticket != 0)
		{
			safe_gmtime(&timeInfo2, pParams->orderInfo[i].openTime);			

			if (timeInfo1.tm_year == timeInfo2.tm_year && timeInfo1.tm_yday == timeInfo2.tm_yday)
			{
				if (pParams->orderInfo[i].isOpen == FALSE && pParams->orderInfo[i].profit < 0) 
				{
					lossTimes++;
					*total_lost_pips += fabs(pParams->orderInfo[i].closePrice - pParams->orderInfo[i].openPrice) * pParams->orderInfo[i].lots;
				}

				if (pParams->orderInfo[i].isOpen == TRUE)
				{
					if (pParams->orderInfo[i].type == BUY && pParams->bidAsk.ask[0] < pParams->orderInfo[i].openPrice 
						//&& fabs(pParams->bidAsk.ask[0] - pParams->orderInfo[i].openPrice) > 0.2
						)
					{
						lossTimes++;
						*total_lost_pips += fabs(pParams->bidAsk.ask[0] - pParams->orderInfo[i].openPrice)* pParams->orderInfo[i].lots;
					}

					if (pParams->orderInfo[i].type == SELL && pParams->bidAsk.bid[0] > pParams->orderInfo[i].openPrice  
						//&& fabs(pParams->bidAsk.ask[0] - pParams->orderInfo[i].openPrice) > 0.2
						)
					{
						lossTimes++;
						*total_lost_pips += fabs(pParams->bidAsk.bid[0] - pParams->orderInfo[i].openPrice)* pParams->orderInfo[i].lots;
					}
				}

			}


		}

	}

	return lossTimes;
}

int EasyTrade::getLossTimesInDaywithSamePrice(time_t currentTime, double openPrice, double limit)
{
	int i;
	int lossTimes = 0;
	struct tm timeInfo1, timeInfo2;

	safe_gmtime(&timeInfo1, currentTime);

	for (i = 0; i < pParams->settings[ORDERINFO_ARRAY_SIZE]; i++)
	{
		// Close winning trades.
		if (pParams->orderInfo[i].ticket != 0)
		{
			safe_gmtime(&timeInfo2, pParams->orderInfo[i].openTime);
			if (timeInfo1.tm_year == timeInfo2.tm_year && timeInfo1.tm_yday == timeInfo2.tm_yday && fabs(pParams->orderInfo[i].openPrice - openPrice) < limit)
			{

				if (pParams->orderInfo[i].isOpen == FALSE && pParams->orderInfo[i].profit < 0)
					lossTimes++;

				if (pParams->orderInfo[i].isOpen == TRUE)
				{
					if (pParams->orderInfo[i].type == BUY && pParams->bidAsk.ask[0] < pParams->orderInfo[i].openPrice						
						)
					{
						lossTimes++;						
					}

					if (pParams->orderInfo[i].type == SELL && pParams->bidAsk.bid[0] > pParams->orderInfo[i].openPrice)
					{
						lossTimes++;						
					}
				}
			}
		}

	}

	return lossTimes;
}

int EasyTrade::getWinTimesInDaywithSamePrice(time_t currentTime,double openPrice,double limit)
{
	int i;
	int winningTimes = 0;
	struct tm timeInfo1, timeInfo2;

	safe_gmtime(&timeInfo1, currentTime);

	for (i = 0; i < pParams->settings[ORDERINFO_ARRAY_SIZE]; i++)
	{
		// Close winning trades.
		if (pParams->orderInfo[i].ticket != 0)
		{
			safe_gmtime(&timeInfo2, pParams->orderInfo[i].openTime);
			if (timeInfo1.tm_year == timeInfo2.tm_year && timeInfo1.tm_yday == timeInfo2.tm_yday && fabs(pParams->orderInfo[i].openPrice- openPrice) < limit )
			{

				if (pParams->orderInfo[i].isOpen == FALSE && pParams->orderInfo[i].profit > 0)
					winningTimes++;

				if (pParams->orderInfo[i].isOpen == TRUE && pParams->orderInfo[i].takeProfit > 0)
				{
					if (pParams->orderInfo[i].type == BUY && pParams->bidAsk.ask[0] > pParams->orderInfo[i].takeProfit)
						winningTimes++;

					if (pParams->orderInfo[i].type == SELL && pParams->bidAsk.bid[0] < pParams->orderInfo[i].takeProfit)
						winningTimes++;
				}
			}
		}

	}

	return winningTimes;
}

int EasyTrade::getWinTimesInDay(time_t currentTime)
{
	int i;
	int winningTimes = 0;	
	struct tm timeInfo1, timeInfo2;
	
	safe_gmtime(&timeInfo1, currentTime);

	for (i = 0; i < pParams->settings[ORDERINFO_ARRAY_SIZE]; i++)
	{
		// Close winning trades.
		if (pParams->orderInfo[i].ticket != 0)
		{
			safe_gmtime(&timeInfo2, pParams->orderInfo[i].openTime);
			if (timeInfo1.tm_year == timeInfo2.tm_year && timeInfo1.tm_yday == timeInfo2.tm_yday)
			{

				if (pParams->orderInfo[i].isOpen == FALSE && pParams->orderInfo[i].profit > 0)
					winningTimes++;

				if (pParams->orderInfo[i].isOpen == TRUE && pParams->orderInfo[i].takeProfit > 0)
				{
					if (pParams->orderInfo[i].type == BUY && pParams->bidAsk.ask[0] > pParams->orderInfo[i].takeProfit)
						winningTimes++;

					if (pParams->orderInfo[i].type == SELL && pParams->bidAsk.bid[0] < pParams->orderInfo[i].takeProfit)
						winningTimes++;
				}
			}
		}

	}

	return winningTimes;
}

double EasyTrade::isSamePricePendingOrder(double entryPrice, double limit)
{
	int i;

	for (i = 0; i < pParams->settings[ORDERINFO_ARRAY_SIZE]; i++)
	{
		if (pParams->orderInfo[i].ticket != 0 && pParams->orderInfo[i].isOpen)
		{	
			if (fabs(entryPrice - pParams->orderInfo[i].openPrice) < limit )
			{
				return TRUE;
			}
		}
	}

	return FALSE;
}

int EasyTrade::hasOpenOrder()
{
	int i;

	for (i = 0; i < pParams->settings[ORDERINFO_ARRAY_SIZE]; i++)
	{
		if (pParams->orderInfo[i].ticket != 0 && pParams->orderInfo[i].isOpen)
		{			
			return TRUE;		
		}
	}

	return FALSE;
}

double EasyTrade::isSameWeekSamePricePendingOrder(double entryPrice, double limit, time_t currentTime)
{
	int i;
	int monday, friday, current;
	struct tm timeInfo1, timeInfo2;
	BOOL isCrossNewYear = FALSE;

	safe_gmtime(&timeInfo1, currentTime);

	monday = timeInfo1.tm_yday - timeInfo1.tm_wday + 1;
	friday = timeInfo1.tm_yday - timeInfo1.tm_wday + 5;
	if (monday < 0)
	{
		isCrossNewYear = TRUE;
		monday += 365;
		friday += 365;
	}

	for (i = 0; i < pParams->settings[ORDERINFO_ARRAY_SIZE]; i++)
	{
		if (pParams->orderInfo[i].ticket != 0 && pParams->orderInfo[i].isOpen)
		{
			safe_gmtime(&timeInfo2, pParams->orderInfo[i].openTime);
			current = timeInfo2.tm_yday;
			if (isCrossNewYear)
				current += 365;

			if (fabs(entryPrice - pParams->orderInfo[i].openPrice) < limit 
				&& current >= monday && current <= friday)
			{
				return TRUE;
			}
		}
	}

	return FALSE;
}

double EasyTrade::caculateStrategyPNLCloseOrder(OrderType type, double openPrice, double closePrice, double lots)
{
	double mLP, equity = pParams->accountInfo.equity;
	int adjust = 0;
	double stopLoss, risk = 0;

	if ((int)pParams->settings[DISABLE_COMPOUNDING] == TRUE)
	{
		equity = pParams->settings[ORIGINAL_EQUITY];
	}


	if (type == BUY)
	{		
		if (closePrice - openPrice > 0)
			adjust = 1;
		else 
			adjust = -1;
	}
	if (type == SELL)
	{		
		if (closePrice - openPrice < 0)
			adjust = 1;
		else
			adjust = -1;
	}

	stopLoss = fabs(closePrice - openPrice);

	mLP = maxLossPerLot(pParams, type, openPrice, stopLoss);

	risk = lots * mLP * adjust / (0.01 * equity);
	logDebug("PNL = %lf, Equity = %lf, maxLossPerLot =%lf,OrderSize = %lf", risk, equity, mLP, lots);


	return risk;
}


double EasyTrade::caculateStrategyPNLOrder(OrderType type, double openPrice, double lots, BOOL isIgnoredLockedProfit)
{
	double mLP, equity = pParams->accountInfo.equity;
	int adjust = 0;
	double currentPrice, stopLoss, risk = 0;

	if ((int)pParams->settings[DISABLE_COMPOUNDING] == TRUE)
	{
		equity = pParams->settings[ORIGINAL_EQUITY];
	}

	
	if (type == BUY || type == BUYLIMIT || type == BUYSTOP)
	{
		currentPrice = pParams->bidAsk.ask[0];
		if (openPrice - currentPrice > 0)
			adjust = -1;
		else if (!isIgnoredLockedProfit)
			adjust = 1;
	}
	if (type == SELL || type == SELLLIMIT || type == SELLSTOP)
	{
		currentPrice = pParams->bidAsk.bid[0];
		if (openPrice - currentPrice < 0)
			adjust = -1;
		else if (!isIgnoredLockedProfit)
			adjust = 1;
	}

	stopLoss = fabs(currentPrice - openPrice);

	mLP = maxLossPerLot(pParams, type, openPrice, stopLoss);

	risk = lots * mLP * adjust / (0.01 * equity);
	logDebug("PNL = %lf, Equity = %lf, maxLossPerLot =%lf,OrderSize = %lf", risk, equity, mLP, lots);


	return risk;
}

double EasyTrade::caculateStrategyPNL(BOOL isIgnoredLockedProfit)
{		
	int i, adjust = 1;
	double risk = 0;

	for (i = 0; i < pParams->settings[ORDERINFO_ARRAY_SIZE]; i++)
	{
		if (pParams->orderInfo[i].ticket != 0 && pParams->orderInfo[i].isOpen)		
			risk = risk + caculateStrategyPNLOrder(pParams->orderInfo[i].type, pParams->orderInfo[i].openPrice, pParams->orderInfo[i].lots, isIgnoredLockedProfit);
	}

	return risk;
}

double EasyTrade::caculateStrategyWeeklyPNL(time_t currentTime)
{
	
	double risk = 0;	
	int i;	
	int monday, friday, current;
	struct tm timeInfo1, timeInfo2;
	BOOL isCrossNewYear = FALSE;	

	safe_gmtime(&timeInfo1, currentTime);

	monday = timeInfo1.tm_yday - timeInfo1.tm_wday + 1;
	friday = timeInfo1.tm_yday - timeInfo1.tm_wday + 5;
	if (monday < 0)
	{
		isCrossNewYear = TRUE;
		monday += 365;
		friday += 365;
	}

	for (i = 0; i < pParams->settings[ORDERINFO_ARRAY_SIZE]; i++)
	{
		//Only looking for closed trades
		if (pParams->orderInfo[i].ticket != 0 && pParams->orderInfo[i].isOpen == FALSE)
		{
			safe_gmtime(&timeInfo2, pParams->orderInfo[i].openTime);
			current = timeInfo2.tm_yday;

			if (isCrossNewYear == FALSE && timeInfo2.tm_year != timeInfo1.tm_year)
				continue;

			if (isCrossNewYear && timeInfo2.tm_yday < 7)
				current += 365;

			if (current >= monday && current <= friday)
			{
				risk = risk + caculateStrategyPNLCloseOrder(pParams->orderInfo[i].type, pParams->orderInfo[i].openPrice, pParams->orderInfo[i].closePrice, pParams->orderInfo[i].lots);
			}
			
		}
	}	

	return risk;
}

int EasyTrade::getSamePricePendingNoTPOrders(double entryPrice, double limit)
{
	int i, count = 0;

	for (i = 0; i < pParams->settings[ORDERINFO_ARRAY_SIZE]; i++)
	{
		if (pParams->orderInfo[i].ticket != 0 && pParams->orderInfo[i].isOpen && pParams->orderInfo[i].takeProfit == 0)
		{
			if (fabs(entryPrice - pParams->orderInfo[i].openPrice) < limit)
			{
				count++;
			}
		}
	}

	return count;
}

double EasyTrade::caculateStrategyVolRiskForNoTPOrders(double dailyATR)
{
	double mLP;
	double equity = pParams->accountInfo.equity;
	int i, adjust = 0;
	double totalLoss = 0, risk = 0;

	if ((int)pParams->settings[DISABLE_COMPOUNDING] == TRUE)
	{
		equity = pParams->settings[ORIGINAL_EQUITY];
	}

	for (i = 0; i < pParams->settings[ORDERINFO_ARRAY_SIZE]; i++)
	{
		if (pParams->orderInfo[i].ticket != 0 && pParams->orderInfo[i].isOpen && pParams->orderInfo[i].takeProfit == 0)
		{
			if (pParams->orderInfo[i].type == BUY)
			{
				if (pParams->orderInfo[i].openPrice - pParams->orderInfo[i].stopLoss > 0)
					adjust = -1;
			}
			if (pParams->orderInfo[i].type == SELL)
			{
				if (pParams->orderInfo[i].openPrice - pParams->orderInfo[i].stopLoss < 0)
					adjust = -1;
			}

			mLP = maxLossPerLot(pParams, pParams->orderInfo[i].type, pParams->orderInfo[i].openPrice, dailyATR);

			risk = risk + pParams->orderInfo[i].lots * mLP * adjust / (0.01 * pParams->accountInfo.equity);
			logDebug("VolRisk = %lf, Equity = %lf, maxLossPerLot =%lf,OrderSize = %lf", risk, equity, mLP, pParams->orderInfo[i].lots);

		}
	}

	return risk;
}

double EasyTrade::caculateStrategyVolRisk(double dailyATR)
{	
	double mLP;
	double equity = pParams->accountInfo.equity;
	int i, adjust = 0;
	double totalLoss = 0, risk = 0;

	if ((int)pParams->settings[DISABLE_COMPOUNDING] == TRUE)
	{
		equity = pParams->settings[ORIGINAL_EQUITY];
	}

	for (i = 0; i < pParams->settings[ORDERINFO_ARRAY_SIZE]; i++)
	{
		if (pParams->orderInfo[i].ticket != 0 && pParams->orderInfo[i].isOpen)
		{
			if (pParams->orderInfo[i].type == BUY)
			{
				if (pParams->orderInfo[i].openPrice - pParams->orderInfo[i].stopLoss > 0)
					adjust = -1;				
			}
			if (pParams->orderInfo[i].type == SELL)
			{
				if (pParams->orderInfo[i].openPrice - pParams->orderInfo[i].stopLoss < 0)
					adjust = -1;				
			}

			mLP = maxLossPerLot(pParams, pParams->orderInfo[i].type, pParams->orderInfo[i].openPrice, dailyATR);

			risk = risk + pParams->orderInfo[i].lots * mLP * adjust / (0.01 * pParams->accountInfo.equity);
			logDebug("VolRisk = %lf, Equity = %lf, maxLossPerLot =%lf,OrderSize = %lf", risk, equity, mLP, pParams->orderInfo[i].lots);

		}
	}

	return risk;
}

double EasyTrade::caculateStrategyRisk(BOOL isIgnoredLockedProfit)
{
	double mLP;
	double equity = pParams->accountInfo.equity;
	int i, adjust = 0;
	double stopLoss, totalLoss = 0, risk = 0;
	
	if ((int)pParams->settings[DISABLE_COMPOUNDING] == TRUE)
	{
		equity = pParams->settings[ORIGINAL_EQUITY];
	}
	
	for (i = 0; i < pParams->settings[ORDERINFO_ARRAY_SIZE]; i++)
	{
		if (pParams->orderInfo[i].ticket != 0 && pParams->orderInfo[i].isOpen)
		{
			adjust = 0;

 			if (pParams->orderInfo[i].type == BUY)
			{
				if (pParams->orderInfo[i].openPrice - pParams->orderInfo[i].stopLoss > 0)
					adjust = -1;
				else if (!isIgnoredLockedProfit)
					adjust = 1;
			}
			if (pParams->orderInfo[i].type == SELL)
			{
				if (pParams->orderInfo[i].openPrice - pParams->orderInfo[i].stopLoss < 0)
					adjust = -1;
				else if (!isIgnoredLockedProfit)
					adjust = 1;
			}

			if (adjust == 0)
				continue;

			stopLoss = fabs(pParams->orderInfo[i].openPrice - pParams->orderInfo[i].stopLoss);			
			if (stopLoss == 0)
				mLP = 0;
			else			
				mLP = maxLossPerLot(pParams, pParams->orderInfo[i].type, pParams->orderInfo[i].openPrice, stopLoss);

			risk = risk + pParams->orderInfo[i].lots * mLP * adjust / (0.01 * pParams->accountInfo.equity);
			logDebug("Risk = %lf, Equity = %lf, maxLossPerLot =%lf,OrderSize = %lf", risk, equity, mLP, pParams->orderInfo[i].lots);

		}
	}

	return risk;
}

AsirikuyReturnCode EasyTrade::modifyAllOrdersOnSameDate(int orderIndex,double stopLoss, double takePrice, BOOL stopMovingbackSL)
{
	int i;
	struct tm timeInfo1, timeInfo2;
	double entryPrice;

	double newTP = takePrice;
	double newSL = stopLoss;
	for (i = 0; i < pParams->settings[ORDERINFO_ARRAY_SIZE]; i++)
	{
		newTP = takePrice;
		newSL = stopLoss;

		safe_gmtime(&timeInfo1, pParams->orderInfo[orderIndex].openTime);
		safe_gmtime(&timeInfo2, pParams->orderInfo[i].openTime);

		if (pParams->orderInfo[i].type == pParams->orderInfo[orderIndex].type 
			&& pParams->orderInfo[i].ticket != 0 
			&& pParams->orderInfo[i].isOpen == TRUE
			&& timeInfo1.tm_mday == timeInfo2.tm_mday)
		{
			if (pParams->orderInfo[i].type == BUY)
				entryPrice = pParams->bidAsk.ask[0];
			else
				entryPrice = pParams->bidAsk.bid[0];

			logInfo("ModifyAllLongs type = %d, ticket = %d,stopLoss=%lf, takePrice=%lf", 
				(int)pParams->orderInfo[i].type, (int)pParams->orderInfo[i].ticket,newSL,newTP );

			if (newTP == -1) // No change
			{
				if (pParams->orderInfo[i].takeProfit == 0)
					newTP = 0;
				else
					newTP = fabs(pParams->orderInfo[i].takeProfit - entryPrice);
			}

			if (newSL == -1)
			{
				if (pParams->orderInfo[i].stopLoss == 0)
					newSL = 0;
				else
					newSL = fabs(entryPrice - pParams->orderInfo[i].stopLoss);
			}


			//������ֹ�������ƶ�
			if (stopMovingbackSL == TRUE && newSL > fabs(entryPrice - pParams->orderInfo[i].stopLoss))
				newSL = fabs(entryPrice - pParams->orderInfo[i].stopLoss);

			modifyTradeEasy(pParams->orderInfo[i].type, pParams->orderInfo[i].ticket, newSL, newTP);
		}
	}

	return SUCCESS;
}

AsirikuyReturnCode EasyTrade::modifyAllLongs_DayTrading(double stopLoss1, double stopLossPrice2, double takePrice, int tpMode, time_t currentTime, double adjust, BOOL stopMovingbackSL)
{
	int i;	
	struct tm timeInfo1, timeInfo2;
	
	safe_gmtime(&timeInfo1, currentTime);

	double newTP = takePrice;
	double newSL = stopLoss1;
	for (i = 0; i < pParams->settings[ORDERINFO_ARRAY_SIZE]; i++)
	{
		newTP = takePrice;
		newSL = stopLoss1;
		if (pParams->orderInfo[i].type == BUY && pParams->orderInfo[i].ticket != 0 && pParams->orderInfo[i].isOpen)
		{
			safe_gmtime(&timeInfo2, pParams->orderInfo[i].openTime);
			if (timeInfo1.tm_mday != timeInfo2.tm_mday) // previous day, 
			{
				if (pParams->bidAsk.bid[0] - stopLossPrice2 < 0)
					newSL = adjust;
				else
					newSL = pParams->bidAsk.bid[0] - stopLossPrice2 + adjust;
			}

			logInfo("ModifyAllLongs type = %d, ticket = %d,tpMode=%d", (int)pParams->orderInfo[i].type, (int)pParams->orderInfo[i].ticket, tpMode);
			if (tpMode == 1 && pParams->orderInfo[i].openPrice - pParams->bidAsk.bid[0] <= 0) // New day and break event			
				newTP = 0;
			else
			{
				if (newTP == -1) // No change
				{
					if (pParams->orderInfo[i].takeProfit == 0)
						newTP = 0;
					else
						newTP = fabs(pParams->orderInfo[i].takeProfit - pParams->bidAsk.ask[0]);
				}

				if (newSL == -1)
				{
					if (pParams->orderInfo[i].stopLoss == 0)
						newSL = 0;
					else
						newSL = fabs(pParams->bidAsk.ask[0] - pParams->orderInfo[i].stopLoss);
				}
			}

			//������ֹ�������ƶ�
			if (stopMovingbackSL == TRUE && newSL > fabs(pParams->bidAsk.ask[0] - pParams->orderInfo[i].stopLoss))
				newSL = fabs(pParams->bidAsk.ask[0] - pParams->orderInfo[i].stopLoss);

			modifyTradeEasy(BUY, pParams->orderInfo[i].ticket, newSL, newTP);
		}
	}

	return SUCCESS;
}

/*
tpMode = 1: ���µ�һ�죬ȥ��TP=0
tpMode = 2: ����ֹӯ 20 ��
*/
AsirikuyReturnCode EasyTrade::modifyAllLongs(double stopLoss, double takePrice, int tpMode, BOOL stopMovingbackSL)
{
	int i;
	double newTP = takePrice;
	double newSL = stopLoss;
	for (i = 0; i < pParams->settings[ORDERINFO_ARRAY_SIZE]; i++)
	{	
		newTP = takePrice;
		newSL = stopLoss;
		if (pParams->orderInfo[i].type == BUY && pParams->orderInfo[i].ticket != 0 && pParams->orderInfo[i].isOpen)
		{			
			logInfo("ModifyAllLongs type = %d, ticket = %d,tpMode=%d", (int)pParams->orderInfo[i].type, (int)pParams->orderInfo[i].ticket,tpMode);
			if (tpMode == 1 && pParams->orderInfo[i].openPrice - pParams->bidAsk.bid[0] <= 0) // New day and break event			
				newTP = 0;		
			else if (tpMode == 2)
			{

			}
			else
			{
				if (takePrice == -1) // No change
				{
					if (pParams->orderInfo[i].takeProfit == 0)
						newTP = 0;
					else
						newTP = fabs(pParams->orderInfo[i].takeProfit - pParams->bidAsk.ask[0]);
				}

				if (stopLoss == -1)
				{
					if (pParams->orderInfo[i].stopLoss == 0)
						newSL = 0;
					else
						newSL = fabs(pParams->bidAsk.ask[0] - pParams->orderInfo[i].stopLoss);
				}
			}

			//������ֹ�������ƶ�
			if (stopMovingbackSL == TRUE && newSL > fabs(pParams->bidAsk.ask[0] - pParams->orderInfo[i].stopLoss))
				newSL = fabs(pParams->bidAsk.ask[0] - pParams->orderInfo[i].stopLoss);

			//if (newTP > fabs(pParams->orderInfo[i].takeProfit - pParams->bidAsk.ask[0]))
			//	newTP = fabs(pParams->orderInfo[i].takeProfit - pParams->bidAsk.ask[0]);

			modifyTradeEasy(BUY, pParams->orderInfo[i].ticket, newSL, newTP);
		}
	}

	return SUCCESS;
}

AsirikuyReturnCode EasyTrade::closeAllLongsWithNegative(int tradeMode, time_t currentTime,int days)
{
	int i, monday, friday, current;
	struct tm timeInfo1, timeInfo2;
	double diffDays = 0;
	BOOL isCrossNewYear = FALSE;

	safe_gmtime(&timeInfo1, currentTime);

	// ����������
	monday = timeInfo1.tm_yday - timeInfo1.tm_wday + 1;
	friday = timeInfo1.tm_yday - timeInfo1.tm_wday + 5;
	 
	if (monday < 0)
	{
		isCrossNewYear = TRUE;
		monday += 365;
		friday += 365;
	}

	for (i = 0; i < pParams->settings[ORDERINFO_ARRAY_SIZE]; i++)
	{

		if (pParams->orderInfo[i].type == BUY && pParams->orderInfo[i].ticket != 0 && pParams->orderInfo[i].isOpen && pParams->orderInfo[i].openPrice - pParams->bidAsk.bid[0] > 0)
		{
			if (tradeMode == 1)
			{
				if (pParams->orderInfo[i].takeProfit == 0)
					continue;
				else
				{
					safe_gmtime(&timeInfo2, pParams->orderInfo[i].openTime);
					if (timeInfo1.tm_mday == timeInfo2.tm_mday)
					{
						if (timeInfo1.tm_hour < 23 || (timeInfo1.tm_hour == 23 && timeInfo1.tm_min < 25))
							continue;
					}
				}
			}
			else if (tradeMode == 2)
			{
				if (pParams->orderInfo[i].takeProfit == 0)
					continue;
				else
				{
					safe_gmtime(&timeInfo2, pParams->orderInfo[i].openTime);
					current = timeInfo2.tm_yday;

					if (isCrossNewYear)
						current += 365;

					if (current >= monday && current <= friday)
					{
						if (timeInfo1.tm_wday == 5 && timeInfo1.tm_hour == 23 && timeInfo1.tm_min >= 25)
							logInfo("Hit End of week");
						else
							continue;
					}
				}
			}
			else if (tradeMode == 3) // Only close on EOD if days> 3
			{
				safe_gmtime(&timeInfo2, pParams->orderInfo[i].openTime);
				if (timeInfo2.tm_wday + days >= 6) //cross weekend
					days += 2;
				diffDays = difftime(currentTime,pParams->orderInfo[i].openTime) / (3600 * 24);
				if (diffDays < days)
					continue;				
				else if (timeInfo1.tm_hour < 23 || (timeInfo1.tm_hour == 23 && timeInfo1.tm_min < 25))									
						continue;				
				logWarning("closing neative trades more than %lf days type = %d, ticket = %d", diffDays,(int)pParams->orderInfo[i].type, (int)pParams->orderInfo[i].ticket);
			}
			else if (tradeMode == 4)
			{
				
				safe_gmtime(&timeInfo2, pParams->orderInfo[i].openTime);
				current = timeInfo2.tm_yday;

				if (isCrossNewYear)
					current += 365;

				if (current >= monday && current <= friday)									
						continue;
			}
			else if (tradeMode == 5) //Close all negative trades intraday
			{
				if (pParams->orderInfo[i].takeProfit == 0)
					continue;
				//else
				//{
				//	if (fabs(pParams->orderInfo[i].openPrice - pParams->bidAsk.bid[0]) < fabs(pParams->orderInfo[i].takeProfit - pParams->orderInfo[i].openPrice) /2)
				//		continue;					
				//}
			}

			logInfo("closeAllLongsWithNegative type = %d, ticket = %d", (int)pParams->orderInfo[i].type, (int)pParams->orderInfo[i].ticket);
			closeLongEasy(pParams->orderInfo[i].ticket);
		}
	}

	return SUCCESS;
}

AsirikuyReturnCode EasyTrade::closeAllCurrentDayShortTermOrders(int tradeMode,time_t currentTime)
{
	int i, monday, friday;
	struct tm timeInfo1, timeInfo2;
	safe_gmtime(&timeInfo1, currentTime);

	monday = timeInfo1.tm_yday - timeInfo1.tm_wday + 1;
	friday = timeInfo1.tm_yday - timeInfo1.tm_wday + 5;

	for (i = 0; i < pParams->settings[ORDERINFO_ARRAY_SIZE]; i++)
	{
		if (pParams->orderInfo[i].ticket != 0 && pParams->orderInfo[i].isOpen && pParams->orderInfo[i].takeProfit !=0)
		{
			safe_gmtime(&timeInfo2, pParams->orderInfo[i].openTime);

			if (tradeMode == 1)
			{								
				if (timeInfo1.tm_mday != timeInfo2.tm_mday)
					continue;
			}
			if (tradeMode == 2)
			{	
				if (timeInfo2.tm_yday < monday || timeInfo2.tm_yday > friday)
					continue;
			}

			logWarning("closeAllCurrentDayShortTermOrders type = %d, ticket = %d", (int)pParams->orderInfo[i].type, (int)pParams->orderInfo[i].ticket);
			if (pParams->orderInfo[i].type == SELL)
				closeShortEasy(pParams->orderInfo[i].ticket);
			if (pParams->orderInfo[i].type == BUY)
				closeLongEasy(pParams->orderInfo[i].ticket);
		}
	}

	return SUCCESS;
}

AsirikuyReturnCode EasyTrade::closeWinningPositions(double total, double target)
{	
	double adjust = total;	
	int riskIndex;
	double closeRisk[ORDERINFO_ARRAY_SIZE] = {0};	
	while (adjust >= target)
	{
		if (closeBiggestWinningPosition(closeRisk, &riskIndex))
			adjust -= closeRisk[riskIndex];
		else
			break;
	}
	
	return SUCCESS;
}

BOOL EasyTrade::closeBiggestWinningPosition(double closeRisk[], int * riskIndex)
{
	int i;		
	double risk = 0, maxRisk= 0,adjust = 0;

	*riskIndex = -1;
	
	for (i = 0; i < pParams->settings[ORDERINFO_ARRAY_SIZE]; i++)
	{
		if (pParams->orderInfo[i].ticket != 0 && pParams->orderInfo[i].isOpen && closeRisk[i] == 0)
		{		

			risk = caculateStrategyPNLOrder(pParams->orderInfo[i].type, pParams->orderInfo[i].openPrice, pParams->orderInfo[i].lots,FALSE);
			
			if (risk > maxRisk)
			{
				maxRisk = risk;
				*riskIndex = i;
			}			
		}
	}

	if (*riskIndex >= 0)
	{
		logWarning("closeBiggestWinningPosition type = %d, ticket = %d", (int)pParams->orderInfo[*riskIndex].type, (int)pParams->orderInfo[*riskIndex].ticket);

		if ((int)pParams->orderInfo[*riskIndex].type == SELL)
			closeShortEasy(pParams->orderInfo[*riskIndex].ticket);
		if ((int)pParams->orderInfo[*riskIndex].type == BUY)
			closeLongEasy(pParams->orderInfo[*riskIndex].ticket);	

		closeRisk[*riskIndex] = maxRisk;
		return TRUE;
	}

	return FALSE;
}
AsirikuyReturnCode EasyTrade::closeAllShortsWithNegative(int tradeMode, time_t currentTime,int days)
{
	int i, monday, friday, current;
	struct tm timeInfo1, timeInfo2;
	double diffDays = 0;
	BOOL isCrossNewYear = FALSE;

	safe_gmtime(&timeInfo1, currentTime);

	monday = timeInfo1.tm_yday - timeInfo1.tm_wday + 1;
	friday = timeInfo1.tm_yday - timeInfo1.tm_wday + 5;
	if (monday < 0)
	{
		isCrossNewYear = TRUE;
		monday += 365;
		friday += 365;
	}

	for (i = 0; i < pParams->settings[ORDERINFO_ARRAY_SIZE]; i++)
	{
		if (pParams->orderInfo[i].type == SELL && pParams->orderInfo[i].ticket != 0 && pParams->orderInfo[i].isOpen && pParams->orderInfo[i].openPrice - pParams->bidAsk.ask[0] < 0)
		{
			if (tradeMode == 1)
			{
				if (pParams->orderInfo[i].takeProfit == 0)
					continue;
				else
				{
					safe_gmtime(&timeInfo2, pParams->orderInfo[i].openTime);
					if (timeInfo1.tm_mday == timeInfo2.tm_mday) //���ֵĵ�һ�죬��Ҫclose.���������һ��bar,����close,��ֹ�ڶ��������ȱ�ڡ�
					{
						if (timeInfo1.tm_hour < 23 || (timeInfo1.tm_hour == 23 && timeInfo1.tm_min <25))
							continue;
					}
				}
			}
			if (tradeMode == 2)
			{
				if (pParams->orderInfo[i].takeProfit == 0)
					continue;
				else
				{
					safe_gmtime(&timeInfo2, pParams->orderInfo[i].openTime);
					current = timeInfo2.tm_yday;

					if (isCrossNewYear)
						current += 365;

					if (current >= monday && current <= friday)					
					{
						if (timeInfo1.tm_wday == 5 && timeInfo1.tm_hour == 23 && timeInfo1.tm_min >= 25)
							logInfo("Hit End of week");
						else
							continue;
						
					}
				}
			}
			else if (tradeMode == 3) // Only close on EOD if days> 3
			{
				safe_gmtime(&timeInfo2, pParams->orderInfo[i].openTime);
				if (timeInfo2.tm_wday + days >= 6) //cross weekend
					days += 2;

				diffDays = difftime(currentTime,pParams->orderInfo[i].openTime) / (3600 * 24);
				if (diffDays < days)
					continue;
				else if (timeInfo1.tm_hour < 23 || (timeInfo1.tm_hour == 23 && timeInfo1.tm_min < 25))
					continue;

				logWarning("closing neative trades more than %lf days type = %d, ticket = %d", diffDays, (int)pParams->orderInfo[i].type, (int)pParams->orderInfo[i].ticket);

			}
			else if (tradeMode == 5) //Close all negative trades intraday
			{
				if (pParams->orderInfo[i].takeProfit == 0)
					continue;
				//else
				//{
				//	if (fabs(pParams->orderInfo[i].openPrice - pParams->bidAsk.ask[0]) < fabs(pParams->orderInfo[i].takeProfit - pParams->orderInfo[i].openPrice) /2)
				//		continue;
				//}
			}

			logInfo("closeAllShortsWithNegative type = %d, ticket = %d", (int)pParams->orderInfo[i].type, (int)pParams->orderInfo[i].ticket);
			closeShortEasy(pParams->orderInfo[i].ticket);
		}
	}

	return SUCCESS;
}

AsirikuyReturnCode EasyTrade::closeAllShorts()
{
  int i;

  for (i=0; i < pParams->settings[ORDERINFO_ARRAY_SIZE]; i++)
  {
    logInfo("closeAllShorts type = %d, ticket = %d", (int)pParams->orderInfo[i].type,  (int)pParams->orderInfo[i].ticket );

    if(pParams->orderInfo[i].type == SELL && pParams->orderInfo[i].ticket != 0 && pParams->orderInfo[i].isOpen )
      closeShortEasy(pParams->orderInfo[i].ticket);
  }

  return SUCCESS;
}

AsirikuyReturnCode EasyTrade::closeAllLongTermShorts()
{
	int i;

	for (i = 0; i < pParams->settings[ORDERINFO_ARRAY_SIZE]; i++)
	{		

		if (pParams->orderInfo[i].type == SELL && pParams->orderInfo[i].ticket != 0 && pParams->orderInfo[i].isOpen && pParams->orderInfo[i].takeProfit == 0)
		{
			logWarning("closeAllLongTermShorts type = %d, ticket = %d", (int)pParams->orderInfo[i].type, (int)pParams->orderInfo[i].ticket);
			closeShortEasy(pParams->orderInfo[i].ticket);
		}
	}

	return SUCCESS;
}

AsirikuyReturnCode EasyTrade::closeSellLimitEasy(int orderTicket)
{
	int tradingSignals, i, resultIndex;

	resultIndex = -1;

	for (i = 0; i < (int)pParams->settings[MAX_OPEN_ORDERS]; i++)
	{
		if (pParams->results[i].tradingSignals == SIGNAL_NONE)
		{
			resultIndex = i;
			break;
		}
	}

	if (resultIndex == -1)
	{
		logInfo("Results array assignations already full");
		return SUCCESS;
	}

	tradingSignals = (int)pParams->results[resultIndex].tradingSignals;

	logInfo("TradeSignal(Exit criteria) : Close BUY. InstanceID = %d, Ticket = %d", (int)pParams->settings[STRATEGY_INSTANCE_ID], orderTicket);
	addTradingSignal(SIGNAL_CLOSE_SELLLIMIT, &tradingSignals);

	pParams->results[resultIndex].ticketNumber = orderTicket;
	pParams->results[resultIndex].tradingSignals = tradingSignals;

	return SUCCESS;
}

AsirikuyReturnCode EasyTrade::closeBuyStopEasy(int orderTicket)
{
	int tradingSignals, i, resultIndex;

	resultIndex = -1;

	for (i = 0; i < (int)pParams->settings[MAX_OPEN_ORDERS]; i++)
	{
		if (pParams->results[i].tradingSignals == SIGNAL_NONE)
		{
			resultIndex = i;
			break;
		}
	}

	if (resultIndex == -1)
	{
		logInfo("Results array assignations already full");
		return SUCCESS;
	}

	tradingSignals = (int)pParams->results[resultIndex].tradingSignals;

	logInfo("TradeSignal(Exit criteria) : Close BUY. InstanceID = %d, Ticket = %d", (int)pParams->settings[STRATEGY_INSTANCE_ID], orderTicket);
	addTradingSignal(SIGNAL_CLOSE_BUYSTOP, &tradingSignals);

	pParams->results[resultIndex].ticketNumber = orderTicket;
	pParams->results[resultIndex].tradingSignals = tradingSignals;

	return SUCCESS;
}

AsirikuyReturnCode EasyTrade::closeSellStopEasy(int orderTicket)
{
	int tradingSignals, i, resultIndex;

	resultIndex = -1;

	for (i = 0; i < (int)pParams->settings[MAX_OPEN_ORDERS]; i++)
	{
		if (pParams->results[i].tradingSignals == SIGNAL_NONE)
		{
			resultIndex = i;
			break;
		}
	}

	if (resultIndex == -1)
	{
		logInfo("Results array assignations already full");
		return SUCCESS;
	}

	tradingSignals = (int)pParams->results[resultIndex].tradingSignals;

	logInfo("TradeSignal(Exit criteria) : Close BUY. InstanceID = %d, Ticket = %d", (int)pParams->settings[STRATEGY_INSTANCE_ID], orderTicket);
	addTradingSignal(SIGNAL_CLOSE_SELLSTOP, &tradingSignals);

	pParams->results[resultIndex].ticketNumber = orderTicket;
	pParams->results[resultIndex].tradingSignals = tradingSignals;

	return SUCCESS;
}

AsirikuyReturnCode EasyTrade::closeBuyLimitEasy(int orderTicket)
{
	int tradingSignals, i, resultIndex;

	resultIndex = -1;

	for (i = 0; i < (int)pParams->settings[MAX_OPEN_ORDERS]; i++)
	{
		if (pParams->results[i].tradingSignals == SIGNAL_NONE)
		{
			resultIndex = i;
			break;
		}
	}

	if (resultIndex == -1)
	{
		logInfo("Results array assignations already full");
		return SUCCESS;
	}

	tradingSignals = (int)pParams->results[resultIndex].tradingSignals;

	logInfo("TradeSignal(Exit criteria) : Close BUY Limit. InstanceID = %d, Ticket = %d", (int)pParams->settings[STRATEGY_INSTANCE_ID], orderTicket);
	addTradingSignal(SIGNAL_CLOSE_BUYLIMIT, &tradingSignals);

	pParams->results[resultIndex].ticketNumber = orderTicket;
	pParams->results[resultIndex].tradingSignals = tradingSignals;

	return SUCCESS;
}

AsirikuyReturnCode EasyTrade::closeLongEasy(int orderTicket)
{
  int tradingSignals, i, resultIndex;

  resultIndex = -1;

  for (i=0;i < (int)pParams->settings[MAX_OPEN_ORDERS]; i++)
  {
    if (pParams->results[i].tradingSignals == SIGNAL_NONE)
    {
      resultIndex = i;
      break;
    }
  }

  if (resultIndex == -1)
  {
    logInfo("Results array assignations already full");
    return SUCCESS;
  }

  tradingSignals = (int)pParams->results[resultIndex].tradingSignals;

  logInfo("TradeSignal(Exit criteria) : Close BUY. InstanceID = %d, Ticket = %d", (int)pParams->settings[STRATEGY_INSTANCE_ID], orderTicket);
  addTradingSignal(SIGNAL_CLOSE_BUY, &tradingSignals);

  pParams->results[resultIndex].ticketNumber = orderTicket;
  pParams->results[resultIndex].tradingSignals = tradingSignals;

  return SUCCESS;
}

AsirikuyReturnCode EasyTrade::closeShortEasy(int orderTicket)
{
  int tradingSignals, i, resultIndex;

  resultIndex = -1;

  for (i=0;i < (int)pParams->settings[MAX_OPEN_ORDERS]; i++)
  {
    if (pParams->results[i].tradingSignals == SIGNAL_NONE)
    {
      resultIndex = i;
      break;
    }
  }

  if (resultIndex == -1)
  {
    logInfo("Results array assignations already full");
    return SUCCESS;
  }

  tradingSignals = (int)pParams->results[resultIndex].tradingSignals;

  logInfo("TradeSignal(Exit criteria) : Close SELL. InstanceID = %d, Ticket = %d", (int)pParams->settings[STRATEGY_INSTANCE_ID], orderTicket);
  addTradingSignal(SIGNAL_CLOSE_SELL, &tradingSignals);

  pParams->results[resultIndex].ticketNumber = orderTicket;
  pParams->results[resultIndex].tradingSignals = tradingSignals;

  return SUCCESS;
}

AsirikuyReturnCode EasyTrade::modifyTradeEasy(int orderType, int orderTicket, double stopLoss, double takeProfit)
{
  int tradingSignals, i, resultIndex;
  AsirikuyReturnCode returnCode = SUCCESS;

  resultIndex = -1;

  for (i=0;i < (int)pParams->settings[MAX_OPEN_ORDERS]; i++)
  {
    if (pParams->results[i].tradingSignals == SIGNAL_NONE)
    {
      resultIndex = i;
      break;
    }
  }

  if (resultIndex == -1)
  {
    logInfo("Results array assignations already full");
    return SUCCESS;
  }

  tradingSignals = (int)pParams->results[resultIndex].tradingSignals;

  logInfo("TradeSignal(Update criteria) : Easy Update. InstanceID = %d, Ticket = %d, New SL = %lf, New TP = %lf", (int)pParams->settings[STRATEGY_INSTANCE_ID], orderTicket, stopLoss, takeProfit);

  returnCode = setStops(pParams, 0, resultIndex, stopLoss, takeProfit, FALSE, FALSE);
  if(returnCode != SUCCESS)
  {
    return logAsirikuyError("modifyShortEasy()", returnCode);
  }

  if(orderType == BUY)
  {
    addTradingSignal(SIGNAL_UPDATE_BUY, &tradingSignals);
  }

  if(orderType == SELL)
  {
    addTradingSignal(SIGNAL_UPDATE_SELL, &tradingSignals);
  }

  pParams->results[resultIndex].ticketNumber = orderTicket;
  pParams->results[resultIndex].tradingSignals = tradingSignals;
  
  return SUCCESS;
}

AsirikuyReturnCode EasyTrade::iPivot(int ratesArrayIndex, int shift, double *pPivot, double *pS1, double *pR1, double *pS2, double * pR2, double *pS3, double *pR3)
{
	double preHigh, preLow, preClose;

	preHigh = iHigh(ratesArrayIndex, shift);
	preLow = iLow(ratesArrayIndex, shift);
	preClose = iClose(ratesArrayIndex, shift);

	*pPivot = (preHigh + preLow + preClose) / 3;
	*pS1 = 2 * (*pPivot) - preHigh;
	*pR1 = 2 * (*pPivot) - preLow;
	*pS2 = *pPivot - (preHigh - preLow);
	*pR2 = *pPivot + (preHigh - preLow);
	*pS3 = preLow - 2 * (preHigh - *pPivot);
	*pR3 = preHigh + 2 * (*pPivot - preLow);
	return SUCCESS;
}

AsirikuyReturnCode EasyTrade::validateCurrentTime(StrategyParams* pParams, int primary_rate)
{
	time_t currentTime;
	struct tm timeInfo,adjustedLocalTimeInfo;
	char  timeString[MAX_TIME_STRING_SIZE] = "";	
	char  adjustedLocaltimeString[MAX_TIME_STRING_SIZE] = "";
	int   shift0Index = pParams->ratesBuffers->rates[primary_rate].info.arraySize - 1;
	int   primary_tf = (int)pParams->settings[TIMEFRAME];

	TimezoneInfo *referenceTZ;
	time_t localTimeUTC = time(NULL);
	time_t adjustedLocalTime;
	TZOffsets tzOffsets;
	int diff = 0;
		
	getTimezoneInfo(pParams->accountInfo.referenceName, &referenceTZ);
	calculateOffsets(localTimeUTC, tzOffsets.referenceTZOffsets, referenceTZ);

	adjustedLocalTime = getAdjustedLocalTime(localTimeUTC, &tzOffsets);
	safe_gmtime(&adjustedLocalTimeInfo, adjustedLocalTime);
	safe_timeString(adjustedLocaltimeString, adjustedLocalTime);

	currentTime = pParams->ratesBuffers->rates[primary_rate].time[shift0Index];
	safe_gmtime(&timeInfo, currentTime);
	safe_timeString(timeString, currentTime);

	logDebug("checking missing bars: current time = %s, adjusted local current time =%s", timeString, adjustedLocaltimeString);
	diff = (int)(difftime(adjustedLocalTime, currentTime) / 60);

	if (diff < primary_tf)
	{
		return SUCCESS;
	}
	else
	{
		logError("Potential missing bars: Current day not matached: current time = %s, adjusted local current time =%s", timeString, adjustedLocaltimeString);

		return ERROR_IN_RATES_RETRIEVAL;
	}
}

//Print and valid the rate bars information to detect any missing rates?
void EasyTrade::printBarInfo(StrategyParams* pParams, int rate, char * currentTimeString)
{	
	time_t currentBarTime;
	struct tm barTimeInfo;
	char  currentBarTimeString[MAX_TIME_STRING_SIZE] = "";

	int   shift0Index = pParams->ratesBuffers->rates[rate].info.arraySize - 1;
	int   shift1Index = pParams->ratesBuffers->rates[rate].info.arraySize - 2;

	currentBarTime = pParams->ratesBuffers->rates[rate].time[shift0Index];
	safe_gmtime(&barTimeInfo, currentBarTime);
	safe_timeString(currentBarTimeString, currentBarTime);

	logDebug("current time=%s checking rate %d current bar 0: time =%s high=%lf,low=%lf,open=%lf,close=%lf", currentTimeString, rate, currentBarTimeString,
		pParams->ratesBuffers->rates[rate].high[shift0Index],
		pParams->ratesBuffers->rates[rate].low[shift0Index],
		pParams->ratesBuffers->rates[rate].open[shift0Index],
		pParams->ratesBuffers->rates[rate].close[shift0Index]);

	//currentBarTime = pParams->ratesBuffers->rates[rate].time[shift1Index];
	//safe_gmtime(&barTimeInfo, currentBarTime);
	//safe_timeString(currentBarTimeString, currentBarTime);

	//fprintf(stderr, "[WARNING] current time=%s checking rate %d current bar 1 time =%s high=%lf,low=%lf,open=%lf,close=%lf", currentTimeString,rate, currentBarTimeString,
	//	pParams->ratesBuffers->rates[rate].high[shift1Index],
	//	pParams->ratesBuffers->rates[rate].low[shift1Index],
	//	pParams->ratesBuffers->rates[rate].open[shift1Index],
	//	pParams->ratesBuffers->rates[rate].close[shift1Index]);

}
AsirikuyReturnCode EasyTrade::validateDailyBars(StrategyParams* pParams, int primary_rate, int daily_rate)
{
	time_t currentTime, currentDailyTime;
	struct tm timeInfo, dailyTimeInfo;
	char  timeString[MAX_TIME_STRING_SIZE] = "";
	char  dailyTimeString[MAX_TIME_STRING_SIZE] = "";
	int   shift0Index = pParams->ratesBuffers->rates[primary_rate].info.arraySize - 1;
	int   shiftDaily0Index = pParams->ratesBuffers->rates[daily_rate].info.arraySize - 1;
	int   startHour = 0;

	//Validate daily bars first
	currentDailyTime = pParams->ratesBuffers->rates[daily_rate].time[shiftDaily0Index];
	safe_gmtime(&dailyTimeInfo, currentDailyTime);
	safe_timeString(dailyTimeString, currentDailyTime);

	currentTime = pParams->ratesBuffers->rates[primary_rate].time[shift0Index];
	safe_gmtime(&timeInfo, currentTime);
	safe_timeString(timeString, currentTime);

	//if (strstr(pParams->tradeSymbol, "XAU") != NULL
	//	|| strstr(pParams->tradeSymbol, "XAG") != NULL
	//	|| strstr(pParams->tradeSymbol, "XPD") != NULL
	//	|| strstr(pParams->tradeSymbol, "XTI") != NULL
	//	//|| (strstr(pParams->tradeSymbol, "BTC") != NULL && timeInfo.tm_wday == 6)
	//	//|| (strstr(pParams->tradeSymbol, "ETH") != NULL && timeInfo.tm_wday == 6)
	//	|| strstr(pParams->tradeSymbol, "US500USD") != NULL
	//	|| strstr(pParams->tradeSymbol, "USTECUSD") != NULL
	//	)
	//	startHour = 1;

	logDebug("checking missing bars: Current daily bar matached: current time = %s, current daily time =%s", timeString, dailyTimeString);
	printBarInfo(pParams, daily_rate, timeString);

	if (dailyTimeInfo.tm_yday == timeInfo.tm_yday  //&& dailyTimeInfo.tm_hour == startHour
		)
	{	
		return SUCCESS;
	}
	else
	{
		logError("Potential missing bars: Current day not matached: current time = %s, current daily time =%s", timeString, dailyTimeString);

		return ERROR_IN_RATES_RETRIEVAL;
	}
}

AsirikuyReturnCode EasyTrade::validateHourlyBars(StrategyParams* pParams, int primary_rate, int hourly_rate)
{
	time_t currentTime, currentHourlyTime;
	struct tm timeInfo, hourlyTimeInfo;
	char  timeString[MAX_TIME_STRING_SIZE] = "";
	char  hourlyTimeString[MAX_TIME_STRING_SIZE] = "";
	int   shift0Index = pParams->ratesBuffers->rates[primary_rate].info.arraySize - 1;
	int   shiftDaily0Index = pParams->ratesBuffers->rates[hourly_rate].info.arraySize - 1;
	int   offset_min = 3;
	int   offset_hour = 0;
	int   start_min = 5;


	//Validate daily bars first
	currentHourlyTime = pParams->ratesBuffers->rates[hourly_rate].time[shiftDaily0Index];
	safe_gmtime(&hourlyTimeInfo, currentHourlyTime);
	safe_timeString(hourlyTimeString, currentHourlyTime);

	currentTime = pParams->ratesBuffers->rates[primary_rate].time[shift0Index];
	safe_gmtime(&timeInfo, currentTime);
	safe_timeString(timeString, currentTime);

	if (strstr(pParams->tradeSymbol, "BTCUSD") != NULL || strstr(pParams->tradeSymbol, "ETHUSD") != NULL)
	{
		//offset_min = 7;
		//if (timeInfo.tm_min == 0)
		//	offset_hour = 1;
		//else
		//	offset_hour = 0;

		if (timeInfo.tm_wday == 6)
		{
			if (timeInfo.tm_min < 45)
				offset_hour = 1;
			start_min = 45;
			offset_min = 45;
		}
	}

	logDebug("checking missing bars: Current hourly bar matached: current time = %s, current hourly time =%s", timeString, hourlyTimeString);
	printBarInfo(pParams, hourly_rate, timeString);
	if (strstr(pParams->tradeSymbol, "BTCUSD") != NULL || strstr(pParams->tradeSymbol, "ETHUSD") != NULL)
	{
		if (timeInfo.tm_hour == 0 && timeInfo.tm_min == 0
			&& hourlyTimeInfo.tm_yday == timeInfo.tm_yday - 1 && hourlyTimeInfo.tm_hour == 23 && hourlyTimeInfo.tm_min <= offset_min)
		{			
			return SUCCESS;
		}
		else if (hourlyTimeInfo.tm_yday == timeInfo.tm_yday && hourlyTimeInfo.tm_hour == timeInfo.tm_hour - offset_hour && hourlyTimeInfo.tm_min <= offset_min)
		{
			return SUCCESS;
		}


		logError("Potential missing bars: Current hourly bar not matached: current time = %s, current hourly time =%s", timeString, hourlyTimeString);
		return ERROR_IN_RATES_RETRIEVAL;
	}
	else 
		if (hourlyTimeInfo.tm_yday == timeInfo.tm_yday && hourlyTimeInfo.tm_hour == timeInfo.tm_hour && hourlyTimeInfo.tm_min <=offset_min)
	{		
		return SUCCESS;
	}
	else
	{
		logError("Potential missing bars: Current hourly bar not matached: current time = %s, current hourly time =%s", timeString, hourlyTimeString);
		return ERROR_IN_RATES_RETRIEVAL;
	}

}
BOOL EasyTrade::validateSecondaryBarsGap(StrategyParams* pParams, time_t currentTime, time_t currentSeondaryTime, int secondary_tf, int primary_tf, bool isWithPrimary, int startHour, int rateErrorTimes)
{	
	struct tm timeInfo, secondaryTimeInfo;
	char  timeString[MAX_TIME_STRING_SIZE] = "";
	char  secondaryTimeString[MAX_TIME_STRING_SIZE] = "";	
	int diff = 0;
	int closeMin = 60 - secondary_tf;
	int startMin = 0;
	int   offset_min = 3;
	int closeHour = 23;
	int specialCloseHour = 19;
	int specialCloseMin = 45;

	BOOL isCheckHistoricalBars = TRUE;
	//if (rateErrorTimes > 2)
	//	isCheckHistoricalBars = FALSE;


	safe_gmtime(&timeInfo, currentTime);
	safe_timeString(timeString, currentTime);

	safe_gmtime(&secondaryTimeInfo, currentSeondaryTime);
	safe_timeString(secondaryTimeString, currentSeondaryTime);

	diff = (int)(difftime(currentTime,currentSeondaryTime) / 60);

	if (strstr(pParams->tradeSymbol, "BTCUSD") != NULL || strstr(pParams->tradeSymbol, "ETHUSD") != NULL)
	{
		offset_min = 7;
		//if (timeInfo.tm_wday == 0)
		//	closeHour = 16;
		specialCloseHour = 16;

		startMin = 5;
		if (timeInfo.tm_wday == 6)
		{
			startMin = 45;		
		}

		if (secondary_tf >= 60)
		{
			if (timeInfo.tm_wday == 6)
				offset_min = 47;

			if (timeInfo.tm_wday == 6)
				closeMin = 45;
			else
				closeMin = 5;
		}
	}

	logDebug("validateSecondaryBarsGap:current time = %s, current secondary time =%s weekend=%d,startHour=%d,diff=%d,secondary_tf=%d",
		timeString, secondaryTimeString, isWeekend(currentTime), startHour, diff, secondary_tf);

	
	if (isWithPrimary)
	{
		if (secondaryTimeInfo.tm_yday == timeInfo.tm_yday && secondaryTimeInfo.tm_hour == timeInfo.tm_hour)
		{
			if (secondaryTimeInfo.tm_min % secondary_tf >= offset_min)
			{
				logError("Current seondary bar not matached: current time = %s, current secondary time =%s System InstanceID = %d",
					timeString, secondaryTimeString, (int)pParams->settings[STRATEGY_INSTANCE_ID]);
				return FALSE;
			}

			if (diff > (secondary_tf - primary_tf) && diff >= primary_tf)
			{
				logError("Current seondary bar not matached: current time = %s, current secondary time =%s System InstanceID = %d",
					timeString, secondaryTimeString, (int)pParams->settings[STRATEGY_INSTANCE_ID]);
				return FALSE;
			}
		}
		else
		{
			logError("Current seondary bar not matached: current time = %s, current secondary time =%s System InstanceID = %d",
				timeString, secondaryTimeString, (int)pParams->settings[STRATEGY_INSTANCE_ID]);
			return FALSE;
		}
		
	}
	else if (isCheckHistoricalBars == TRUE)
	{
		//if (secondaryTimeInfo.tm_wday == 5 && timeInfo.tm_wday == 1 )
		//{
		//	if (diff != 2 * MINUTES_PER_DAY + secondary_tf)
		//	{
		//		fprintf(stderr, "[ERROR] Potential missing bars: Current seondary bar not matached: current time = %s, current secondary time =%s", timeString, secondaryTimeString);
		//		return FALSE;
		//	}
		//}
		//���ʱ����Ҫcheck weekend? public holiday? ����public holiday���ü��㡣���ʱ�򣬽�����
		//23��55 ------ 0��00 or 1:00(XAU)
		//TradeMAX broker, �����������С�ÿ���壬����ֻ��23��44,����20�������̡�

		
		if (diff > secondary_tf) 
		{						
			if (strcmp(pParams->accountInfo.referenceName, "TradeMax Group Pty Ltd") == 0)
			{
				if (secondary_tf == 15)
					closeMin = 30;
				if (secondary_tf == 5)
					closeMin = 40;
			}
			
			//if (startHour == 1 && diff < 2 * MINUTES_PER_HOUR) // XAU for a noremal weekday.
			//{
			//	if (diff == MINUTES_PER_HOUR + secondary_tf &&
			//		( (secondaryTimeInfo.tm_year == timeInfo.tm_year && timeInfo.tm_yday - secondaryTimeInfo.tm_yday == 1) ||
			//			(secondaryTimeInfo.tm_year < timeInfo.tm_year && timeInfo.tm_yday == 0)
			//		) )
			//		return TRUE;
			//	else
			//	{
			//		fprintf(stderr, "[ERROR] validateSecondaryBarsGap: Current seondary bar not matached: current time = %s, current secondary time =%s System InstanceID = %d",
			//			timeString, secondaryTimeString, (int)pParams->settings[STRATEGY_INSTANCE_ID]);
			//		saveRateFile((int)pParams->settings[STRATEGY_INSTANCE_ID], rateErrorTimes+1, (BOOL)pParams->settings[IS_BACKTESTING]);
			//		return FALSE;
			//	}
			//}

			//TODO: 
			//For example XAU close eariler on US holiday, it means close on 20:00. Close hour is 20, min is 0
			// XTI: 19:55, US500: 19:55
			
			//Only BTCUSD will miss 00:00 on 5m chart, in the weekend,it can be more than 5 mins....
			if ((strstr(pParams->tradeSymbol, "BTCUSD") != NULL || strstr(pParams->tradeSymbol, "ETHUSD") != NULL) && secondary_tf == 5
				&& secondaryTimeInfo.tm_hour == closeHour
				&& ((!isWeekend(currentTime) && secondaryTimeInfo.tm_min == closeMin)
					|| (isWeekend(currentTime) && secondaryTimeInfo.tm_min >= closeMin - 30)
					)
				&& timeInfo.tm_hour == startHour && timeInfo.tm_min == startMin) {
				
				//saveRateFile((int)pParams->settings[STRATEGY_INSTANCE_ID], 0, (BOOL)pParams->settings[IS_BACKTESTING]);
				return TRUE;
			}
			if ((strstr(pParams->tradeSymbol, "BTCUSD") != NULL || strstr(pParams->tradeSymbol, "ETHUSD") != NULL) && secondary_tf == 5
				&& secondaryTimeInfo.tm_hour == closeHour
				&& timeInfo.tm_wday == 6
				&& timeInfo.tm_hour == startHour
				&& timeInfo.tm_min == secondaryTimeInfo.tm_min) {
				//saveRateFile((int)pParams->settings[STRATEGY_INSTANCE_ID], 0, (BOOL)pParams->settings[IS_BACKTESTING]);
				return TRUE;
			}
			else if ((strstr(pParams->tradeSymbol, "BTCUSD") != NULL || strstr(pParams->tradeSymbol, "ETHUSD") != NULL) && secondary_tf == 60
				&& timeInfo.tm_wday == 0 //Sunday BTC Saturday is 00:45, Sunday 00:05, but sometimes it keeps changing.
				&& timeInfo.tm_hour == startHour && timeInfo.tm_min == startMin
				&& secondaryTimeInfo.tm_hour == closeHour
				&& secondaryTimeInfo.tm_min == specialCloseMin
				) {
				//saveRateFile((int)pParams->settings[STRATEGY_INSTANCE_ID], 0, (BOOL)pParams->settings[IS_BACKTESTING]);
				return TRUE;
			}
			else if (secondaryTimeInfo.tm_hour == closeHour && secondaryTimeInfo.tm_min == closeMin &&
				timeInfo.tm_hour == startHour && timeInfo.tm_min == startMin) {
				saveRateFile((int)pParams->settings[STRATEGY_INSTANCE_ID], 0, (BOOL)pParams->settings[IS_BACKTESTING]);
				return TRUE;
			}
			else if (secondaryTimeInfo.tm_hour == specialCloseHour
				&& secondaryTimeInfo.tm_min == closeMin
				&& timeInfo.tm_hour == startHour && timeInfo.tm_min == startMin) {
				saveRateFile((int)pParams->settings[STRATEGY_INSTANCE_ID], 0, (BOOL)pParams->settings[IS_BACKTESTING]);
				return TRUE;
			}
			else
			{
				//if (rateErrorTimes > 2) {
				//	fprintf(stderr, "[WARNING] Skip rate error: current time = %s, current secondary time =%s System InstanceID = %d",
				//		timeString, secondaryTimeString, (int)pParams->settings[STRATEGY_INSTANCE_ID]);
				//	return TRUE;
				//}
				if (rateErrorTimes <= 2) {
					logError("validateSecondaryBarsGap: Current seondary bar not matached: current time = %s, current secondary time =%s System InstanceID = %d",
						timeString, secondaryTimeString, (int)pParams->settings[STRATEGY_INSTANCE_ID]);

					saveRateFile((int)pParams->settings[STRATEGY_INSTANCE_ID], rateErrorTimes + 1, (BOOL)pParams->settings[IS_BACKTESTING]);
				}
				return FALSE;
			}
		}
		else if (diff > secondary_tf + 1 || diff < secondary_tf - 1)
		{
			if ((strstr(pParams->tradeSymbol, "BTCUSD") != NULL || strstr(pParams->tradeSymbol, "ETHUSD") != NULL) && secondary_tf == 60
				&& timeInfo.tm_wday == 0 //Sunday BTC Saturday is 00:45, Sunday 00:05, but sometimes it keeps changing.
				&& timeInfo.tm_hour == startHour && timeInfo.tm_min == startMin
				&& secondaryTimeInfo.tm_hour == closeHour
				&& secondaryTimeInfo.tm_min == specialCloseMin
				) {
				//saveRateFile((int)pParams->settings[STRATEGY_INSTANCE_ID], 0, (BOOL)pParams->settings[IS_BACKTESTING]);
				return TRUE;
			}
			//if (rateErrorTimes > 2) {
			//	fprintf(stderr, "[WARNING] Skip rate error: current time = %s, current secondary time =%s System InstanceID = %d",
			//		timeString, secondaryTimeString, (int)pParams->settings[STRATEGY_INSTANCE_ID]);
			//	return TRUE;
			//}
			if (rateErrorTimes <= 2) {
				logError("validateSecondaryBarsGap: Current seondary bar not matached: current time = %s, current secondary time =%s System InstanceID = %d",
					timeString, secondaryTimeString, (int)pParams->settings[STRATEGY_INSTANCE_ID]);
				saveRateFile((int)pParams->settings[STRATEGY_INSTANCE_ID], rateErrorTimes + 1, (BOOL)pParams->settings[IS_BACKTESTING]);
			}
			return FALSE;
		}

		//if (rateErrorTimes > 2) {
		//	fprintf(stderr, "[INFO] reset rate to 0: current time = %s, current secondary time =%s System InstanceID = %d",
		//		timeString, secondaryTimeString, (int)pParams->settings[STRATEGY_INSTANCE_ID]);
		//	saveRateFile((int)pParams->settings[STRATEGY_INSTANCE_ID], 0, (BOOL)pParams->settings[IS_BACKTESTING]);
		//}
	}

	
	return TRUE;
}

AsirikuyReturnCode EasyTrade::validateSecondaryBars(StrategyParams* pParams, int primary_rate, int secondary_rate, int secondary_tf, int rateErrorTimes)
{
	time_t currentTime, currentSeondaryTime;	
	struct tm timInfo,secondaryTimeInfo;
	char  timeString[MAX_TIME_STRING_SIZE] = "";
	char  secondaryTimeString[MAX_TIME_STRING_SIZE] = "";
	int   shift0Index = pParams->ratesBuffers->rates[primary_rate].info.arraySize - 1;
	int   shiftSecondary0Index = pParams->ratesBuffers->rates[secondary_rate].info.arraySize - 1;
	int   primary_tf = (int)pParams->settings[TIMEFRAME];

	int   startHour = 0;
	int   checkedBarNum = pParams->ratesBuffers->rates[secondary_rate].info.arraySize -1;

	// Check out the first bar
	currentSeondaryTime = pParams->ratesBuffers->rates[secondary_rate].time[shiftSecondary0Index];
	currentTime = pParams->ratesBuffers->rates[primary_rate].time[shift0Index];

	safe_timeString(timeString, currentTime);
	safe_timeString(secondaryTimeString, currentSeondaryTime);

	safe_gmtime(&timInfo, currentTime);
	safe_gmtime(&secondaryTimeInfo, currentSeondaryTime);

	if (strstr(pParams->tradeSymbol, "XAU") != NULL
		|| strstr(pParams->tradeSymbol, "XAG") != NULL
		|| strstr(pParams->tradeSymbol, "XPD") != NULL
		|| strstr(pParams->tradeSymbol, "XTI") != NULL
		|| strstr(pParams->tradeSymbol, "SpotCrude") != NULL
		//|| (strstr(pParams->tradeSymbol, "BTC") != NULL && timInfo.tm_wday == 6)
		//|| (strstr(pParams->tradeSymbol, "ETH") != NULL && timInfo.tm_wday == 6)
		|| strstr(pParams->tradeSymbol, "US500USD") != NULL 
		|| strstr(pParams->tradeSymbol, "NAS100USD") != NULL
		|| strstr(pParams->tradeSymbol, "USTECUSD") != NULL
		)
		startHour = 1;
		
	//if (primary_tf != secondary_tf)
	{

		logDebug("Checking missing bars:current time = %s, current secondary time =%s", timeString, secondaryTimeString);
		logDebug("current time=%s checking rate %d current bar 0: time =%s high=%lf,low=%lf,open=%lf,close=%lf", timeString, secondary_rate, secondaryTimeString,
			pParams->ratesBuffers->rates[secondary_rate].high[shiftSecondary0Index],
			pParams->ratesBuffers->rates[secondary_rate].low[shiftSecondary0Index],
			pParams->ratesBuffers->rates[secondary_rate].open[shiftSecondary0Index],
			pParams->ratesBuffers->rates[secondary_rate].close[shiftSecondary0Index]);

		if (!validateSecondaryBarsGap(pParams, currentTime, currentSeondaryTime, secondary_tf, primary_tf, true, startHour, rateErrorTimes))
			return ERROR_IN_RATES_RETRIEVAL;
		

		if (secondary_rate < MINUTES_PER_HOUR) //ֻ�yԇ����
		{
			checkedBarNum = (int)((secondaryTimeInfo.tm_hour-startHour) * MINUTES_PER_HOUR + secondaryTimeInfo.tm_min) / secondary_tf;
		}
		else
			checkedBarNum = (int)(MINUTES_PER_DAY + (secondaryTimeInfo.tm_hour-startHour) * MINUTES_PER_HOUR + secondaryTimeInfo.tm_min) / secondary_tf;
		
		logWarning("Checking %d history secondary bars: current time = %s, current secondary time =%s", checkedBarNum, timeString, secondaryTimeString);

		while (shiftSecondary0Index >= pParams->ratesBuffers->rates[secondary_rate].info.arraySize - 1 - checkedBarNum)
		{
			currentSeondaryTime = pParams->ratesBuffers->rates[secondary_rate].time[shiftSecondary0Index - 1];
			currentTime = pParams->ratesBuffers->rates[secondary_rate].time[shiftSecondary0Index];
			safe_timeString(timeString, currentTime);
			safe_timeString(secondaryTimeString, currentSeondaryTime);

			logDebug("Checking missing bars:current time = %s, current secondary time =%s", timeString, secondaryTimeString);
			logDebug("current time=%s checking rate %d current bar 0: time =%s high=%lf,low=%lf,open=%lf,close=%lf", timeString, secondary_rate, secondaryTimeString,
				pParams->ratesBuffers->rates[secondary_rate].high[shiftSecondary0Index - 1],
				pParams->ratesBuffers->rates[secondary_rate].low[shiftSecondary0Index - 1],
				pParams->ratesBuffers->rates[secondary_rate].open[shiftSecondary0Index - 1],
				pParams->ratesBuffers->rates[secondary_rate].close[shiftSecondary0Index - 1]);

			if (!validateSecondaryBarsGap(pParams, currentTime, currentSeondaryTime, secondary_tf, primary_tf, false, startHour, rateErrorTimes)) {

				if (rateErrorTimes > 2) {
					logWarning("Skip rate error: current time = %s, current secondary time =%s System InstanceID = %d",
						timeString, secondaryTimeString, (int)pParams->settings[STRATEGY_INSTANCE_ID]);	
					return SUCCESS;
				}
				else
					return ERROR_IN_RATES_RETRIEVAL;
			}

			shiftSecondary0Index--;
		}

		if (rateErrorTimes > 0) {
			logInfo("reset rate to 0: current time = %s, current secondary time =%s System InstanceID = %d",
				timeString, secondaryTimeString, (int)pParams->settings[STRATEGY_INSTANCE_ID]);
			saveRateFile((int)pParams->settings[STRATEGY_INSTANCE_ID], 0, (BOOL)pParams->settings[IS_BACKTESTING]);
		}
	}
	
	
	return SUCCESS;

}








