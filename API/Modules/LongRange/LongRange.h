/*
 * LongRange.h
 *
 *  Created on: 3 de abr. de 2025
 *      Author: diego.marinho
 */

#ifndef API_MODULES_LONGRANGE_LONGRANGE_H_
#define API_MODULES_LONGRANGE_LONGRANGE_H_

#include "cy_pdl.h"
#include "cybsp.h"
#include "cyabs_rtos.h"
#include "alarm.h"
#include "packet.h"
#include "callback.h"

typedef enum{
	HeaderLR = 0,
	LENLR,
	DADOLR
}stateLR_t;

typedef struct{
    struct{
        hEvents_t handle;
        uint32_t index;
    }event;

    cy_thread_t thread_LR;



	cy_stc_scb_uart_context_t uartContext; //contexto da usart

	stateLR_t state;
	uint8_t count_header;
	uint8_t lenght_data_cryp;

	uint8_t byte_receive;
	uint8_t data[128];
}long_range_t;

extern long_range_t long_range;

void LongRange_init(hEvents_t eventHandle, uint32_t eventIndex);
void longRange_task(void *pvParameters);
void packet_transmit_longRange(uint8_t *byte_transmit,uint8_t len);


#endif /* API_MODULES_LONGRANGE_LONGRANGE_H_ */
