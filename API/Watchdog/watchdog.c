/*
 * watchdog.c
 *
 *  Created on: 6 de mar. de 2025
 *      Author: diego.marinho
 */


#include "watchdog.h"

void initialize_wdt(){
	   /* Step 1- Unlock WDT */
	   Cy_WDT_Unlock();

	   /* Step 2- Write the ignore bits - operate with only 14 bits */
	   Cy_WDT_SetIgnoreBits(16);

	   /* Step 3- Write match value */
	   Cy_WDT_SetMatch(WDT_MATCH_COUNT);

	   /* Step 4- Clear match event interrupt, if any */
	   Cy_WDT_ClearInterrupt();

	   /* Step 5- Enable WDT */
	   Cy_WDT_Enable();

	   /* Step 6- Lock WDT configuration */
	   Cy_WDT_Lock();
}


