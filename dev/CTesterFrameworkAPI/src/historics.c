//
//  historics.c
//  asr
//
//  Created by Jorge Ferrando on 10/04/13.
//  Copyright (c) 2013 Jorge Ferrando. All rights reserved.
//

#include "CTesterFrameworkDefines.h"
#include "historics.h"

time_t parseDate(const char * str)
{
	time_t result;
    struct tm ti={0};
    		sscanf(str, "%d.%d.%d,%d:%d", &ti.tm_year, &ti.tm_mon, &ti.tm_mday, &ti.tm_hour, &ti.tm_min);
    {
        /* ... error parsing ... */
    }

    ti.tm_year-=1900;
    ti.tm_mon-=1;
    
    result = mktime(&ti);
	return result;
}

int __stdcall  readHistoricFile(char *historicPath, Rates **result, char **error){ 
    FILE *file;
    char error_t[MAX_ERROR_LENGTH];
	Rates *rates;
	int ch, numLines = 0, maxLineLenght = 0, maxLineLenghtTemp = 0;
	char line[128]; /* or other suitable maximum line size */
    int i=0;
	time_t *dateT;
	double *open, *low, *high, *close, *volume;
    
    //Open the file
    file = fopen(historicPath, "r");
    if (file == NULL){
        sprintf(error_t, "Error opening the historic file %s\n", historicPath);
        *error = malloc(strlen(error_t) + 1);
        strcpy(*error, error_t);
        return false;
    }
    
    //Get number of lines in the file
    do
    {
        ch = fgetc(file);
        if(ch == '\n') {
            numLines++;
            maxLineLenghtTemp = 0;
        }
        else {
            maxLineLenghtTemp++;
            if (maxLineLenghtTemp > maxLineLenght) maxLineLenght = maxLineLenghtTemp;
        }
    } while (ch != EOF);
    if(ch != '\n' && numLines != 0) numLines++;
    rewind(file);
    
    
    //allocate space to save the rates
    rates = malloc(numLines * sizeof(struct rates_t));
    while (fgets(line, sizeof(line), file) != NULL) /* read a line */
    {
        char date[17], time[6], dateS[30];
		open   = malloc(sizeof(double));
		high   = malloc(sizeof(double));
		low    = malloc(sizeof(double));
		close  = malloc(sizeof(double));
		volume = malloc(sizeof(double));
		dateT  = malloc(sizeof(time_t));

        sscanf(line, "%[^,],%[^,],%lf,%lf,%lf,%lf,%d", date, &time, open, high, low, close, volume);
        sprintf(dateS, "%s,%s", date, time);
		*dateT = parseDate(date);
		rates[i].open    = open;
		rates[i].high    = high;
		rates[i].low    = low;
		rates[i].close  = close;
		rates[i].volume = volume;
        rates[i].time =  dateT;
        i++;
    }

    fclose (file);
	*result = rates;
    return i;
}

