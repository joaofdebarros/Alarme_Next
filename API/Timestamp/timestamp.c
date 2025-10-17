/*
 * timestamp.c
 *
 *  Created on: 5 de mar. de 2025
 *      Author: diego.marinho
 */

#include "timestamp.h"

#define MAX_ATTEMPTS             (500u)  /* Maximum number of attempts for RTC operation */
#define INIT_DELAY_MS             (5u)    /* delay 5 milliseconds before trying again */

//const cy_stc_rtc_config_t USER_RTC_config =
//{
//    .sec = 0U,
//    .min = 0U,
//    .hour = 12U,
//    .amPm = CY_RTC_AM,
//    .hrFormat = CY_RTC_24_HOURS,
//    .dayOfWeek = CY_RTC_TUESDAY,
//    .date = 3U,
//    .month = CY_RTC_SEPTEMBER,
//    .year = 24U,
//};

cy_en_rtc_status_t rtc_init(void){
    uint32_t attempts = MAX_ATTEMPTS;
    cy_en_rtc_status_t rtc_result;

    /* Setting the time and date can fail. For example the RTC might be busy.
       Check the result and try again, if necessary.  */
    do
    {
        rtc_result = Cy_RTC_Init(&USER_RTC_config);
        attempts--;

        Cy_SysLib_Delay(INIT_DELAY_MS);
    } while(( rtc_result != CY_RTC_SUCCESS) && (attempts != 0u));

    return (rtc_result);
}


void rtc_set_timestamp(uint32_t uts)
{
	cy_stc_rtc_config_t dateTime;
	time_t now;

	if (uts == 0) {
		return;
	}

	if(rtc_get_timestamp() < uts){

		now = (time_t)uts;

		struct tm time_tm;
		time_tm = *(localtime(&now));

		dateTime.hour = (uint8_t)time_tm.tm_hour - 3;
		dateTime.min = (uint8_t)time_tm.tm_min;
		dateTime.sec = (uint8_t)time_tm.tm_sec;

		if (time_tm.tm_wday == 0)
		{
			time_tm.tm_wday = 7;
		}

		dateTime.dayOfWeek = (uint8_t)time_tm.tm_wday+1;
		dateTime.month = (uint8_t)time_tm.tm_mon+1;
		dateTime.date = (uint8_t)time_tm.tm_mday;
		dateTime.year = (uint16_t)(time_tm.tm_year+1900-2000);


		dateTime.amPm = CY_RTC_AM;      // Ignorado em formato 24h
		dateTime.hrFormat = CY_RTC_24_HOURS; // Formato de 24 horas


		Cy_RTC_SetDateAndTime(&dateTime);
	}
}


// le o valor interno do rtc e converte para um time_t
uint32_t rtc_get_timestamp(void)
{
	cy_stc_rtc_config_t dateTime;
	Cy_RTC_GetDateAndTime(&dateTime);

	struct tm tim = {0};

	uint8_t hh = dateTime.hour;
	uint8_t mm = dateTime.min;
	uint8_t ss = dateTime.sec;

	tim.tm_hour = hh;
	tim.tm_min = mm;
	tim.tm_sec = ss;

	uint8_t d = dateTime.date;
	uint8_t m = dateTime.month;
	uint16_t y = dateTime.year;
	uint16_t yr = (uint16_t)(y+2000-1900);
	time_t currentTime = {0};

	tim.tm_year = yr;
	tim.tm_mon = m - 1;
	tim.tm_mday = d;

	currentTime = mktime(&tim);

	return currentTime;
}

uint8_t rtc_dia_valida(dias_t semana, uint16_t inic, uint16_t delta)
{
	cy_stc_rtc_config_t dateTime;
	Cy_RTC_GetDateAndTime(&dateTime);


	struct tm *t; // estrutura para datahora na time.h
	uint16_t atual;
	time_t seconds;

	if ((semana.u8 | inic | delta) == 0) {
		return 1;
	}

	// le data e hora do rtc
	//seconds = time(NULL);
	seconds = dateTime.sec;

	// Setup a tm structure based on the RTC
	//t = localtime(&seconds);
//	t->tm_hour = &rtcTime.Hours;
//	t->tm_min = &rtcTime.Minutes;
//	t->tm_wday = &rtcDate.WeekDay;

	atual = (dateTime.hour) * 60 + dateTime.min;

	// se o dia da semana eh valido
	if (semana.u8 & (1 << (dateTime.dayOfWeek-1))) {
		if ((inic == 0 && delta == 0) || (atual >= inic && atual <= (inic+delta))) {
			return 1;
		}
		return 0;
	}
	// se nao, verifica o dia anterior (por causa de overlap)
	if (dateTime.dayOfWeek > 0) { // se nao for domingo
		dateTime.dayOfWeek--; // dia anterior
	}
	else {
		dateTime.dayOfWeek = 6; // sabado
	}

	// novamente, se o dia de ontem for valido
	if (semana.u8 & (1 << dateTime.dayOfWeek)) {
		// considerando o dia anterior, ja se passaram 24h
		atual += 24 * 60;
		if (atual > inic && atual < inic+delta) {
			return 1;
		}
	}

	return 0;
}
