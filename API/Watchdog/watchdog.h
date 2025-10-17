/*
 * watchdog.h
 *
 *  Created on: 6 de mar. de 2025
 *      Author: diego.marinho
 */

#ifndef WATCHDOG_WATCHDOG_H_
#define WATCHDOG_WATCHDOG_H_

#include "cy_pdl.h"

/* WDT time out for reset mode, in milliseconds. Max limit is given by
 * CYHAL_WDT_MAX_TIMEOUT_MS */
#define WDT_TIME_OUT_MS                     12000
#define ENABLE_BLOCKING_FUNCTION            0

/* Match count =  Desired interrupt interval in seconds x ILO Frequency in Hz */
#define WDT_MATCH_COUNT                     (WDT_TIME_OUT_MS*32000)/1000

void initialize_wdt();



#endif /* WATCHDOG_WATCHDOG_H_ */
