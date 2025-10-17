/*
 * timestamp.h
 *
 *  Created on: 5 de mar. de 2025
 *      Author: diego.marinho
 */

#ifndef TIMESTAMP_TIMESTAMP_H_
#define TIMESTAMP_TIMESTAMP_H_

#include "stdint.h"

#include "cy_pdl.h"
#include "time.h"

#include "cybsp.h"

typedef union {
	struct { // 1
		// dias da semana que o acesso eh valido
		uint8_t d_dom : 1;
		uint8_t d_seg : 1;
		uint8_t d_ter : 1;
		uint8_t d_qua : 1;
		uint8_t d_qui : 1;
		uint8_t d_sex : 1;
		uint8_t d_sab : 1;
		uint8_t d_unico : 1; // se o acesso pode ser feito apenas uma vez
	};
	uint8_t u8;
} dias_t;


cy_en_rtc_status_t rtc_init(void);
void rtc_set_timestamp(uint32_t uts);
uint32_t rtc_get_timestamp(void);
uint8_t rtc_dia_valida(dias_t semana, uint16_t inic, uint16_t delta);


#endif /* TIMESTAMP_TIMESTAMP_H_ */
