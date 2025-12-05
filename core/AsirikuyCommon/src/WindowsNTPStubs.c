/**
 * @file WindowsNTPStubs.c
 * @brief Windows stub implementations for NTP and Timezone functions
 * 
 * These functions are not available on Windows due to missing Boost ASIO dependencies.
 * This stub implementation provides safe return values to allow linking on Windows.
 */

#include <time.h>
#include "AsirikuyDefines.h"

/* NTP Client stubs - from NTPCWrapper.hpp */

/**
 * Stub: Requests the current time from a random NTP server.
 * Returns current system time instead.
 */
time_t queryRandomNTPServer()
{
    return time(NULL);
}

/**
 * Stub: Requests the current time from a specified NTP server.
 * Returns current system time instead.
 */
time_t queryNTPServer(const char* ntpServer)
{
    (void)ntpServer; /* Unused parameter */
    return time(NULL);
}

/**
 * Stub: Sets the update interval for polling NTP servers.
 */
void setNtpUpdateInterval(int seconds)
{
    (void)seconds; /* Unused parameter */
}

/**
 * Stub: Sets the NTP timeout.
 */
void setNtpTimeout(int milliseconds)
{
    (void)milliseconds; /* Unused parameter */
}

/**
 * Stub: Sets the number of NTP servers to use for validating times.
 */
void setTotalNtpReferenceTimes(int total)
{
    (void)total; /* Unused parameter */
}

/* Timezone stubs - from Timezones.hpp */

/**
 * Stub: Gets the days of the year when daylight savings time starts and ends.
 * On Windows, returns a safe default (US Eastern Time DST dates).
 */
AsirikuyReturnCode getDSTStartEndDays(time_t currentTime, TimezoneInfo* pTzInfo, 
                                     int* pDstStartDay, int* pDstEndDay)
{
    (void)currentTime;  /* Unused parameter */
    (void)pTzInfo;      /* Unused parameter */
    
    if (!pDstStartDay || !pDstEndDay)
        return NULL_POINTER;
    
    /* US Eastern Time DST: second Sunday in March to first Sunday in November */
    /* Approximate days: 73 (March 13, 2023) to 305 (November 5, 2023) */
    *pDstStartDay = 73;
    *pDstEndDay = 305;
    
    return SUCCESS;
}

/**
 * Stub: Determines if the current hour is in daylight savings time or not.
 */
int isDST(int dstStartDay, int dstEndDay, int dstStartHour, int dstEndHour, 
          int currentDay, int currentHour)
{
    (void)dstStartHour;  /* Unused parameter */
    (void)dstEndHour;    /* Unused parameter */
    (void)currentHour;   /* Unused parameter */
    
    /* Simple check: is current day between DST start and end days? */
    if (currentDay >= dstStartDay && currentDay < dstEndDay)
        return 1;  /* In DST */
    
    return 0;  /* Not in DST */
}

/**
 * Stub: Converts UTC time to a specified timezone.
 * On Windows, returns the UTC time unchanged.
 */
time_t utcToTimezone(time_t utcTime, TimezoneInfo tzInfo)
{
    (void)tzInfo;  /* Unused parameter */
    
    /* Return UTC time unchanged - timezone conversion not available */
    return utcTime;
}
