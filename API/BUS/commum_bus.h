/*
 * commum_bus.h
 *
 *  Created on: 4 de set. de 2025
 *      Author: diego.marinho
 */



#ifndef API_BUS_PGM_COMMUM_BUS_H_
#define API_BUS_PGM_COMMUM_BUS_H_

#include "cy_pdl.h"
#include "cybsp.h"
#include "alarm.h"
#include "PGM.h"
#include "teclado.h"
#include "cyabs_rtos.h"

typedef enum{
	PGM_NODE = 0,
	TECLADO_NODE
}type_node_t;

typedef enum {
  START_ST = 0,
  SIZE_ST,
  IDT_ST,
  ADDRS_ST,
  FUNCTION_ST,
  DATA_ST,
} state_receive_t;

typedef struct{
    struct{
        hEvents_t waitInt;
        hEvents_t handle;
        uint32_t index;
    }event;
	uint8_t data[200];

	cy_rslt_t status_rtos_bus;
	cy_thread_t thread_bus;
	cy_stc_scb_uart_context_t uartContext; //contexto da usart

	state_receive_t state;
	uint8_t byte_receive;
	uint8_t count_header;
	uint8_t lenght_data_cryp;
	//state_receive_t state;
	type_node_t type_node;
}bus_t;

/* ======================== VARIÁVEIS EXTERNAS ======================== */
extern bus_t bus;

void commum_bus_init(void);
void BUS_task(void *pvParameters);
void packet_transmit_bus(uint8_t *buffer, uint8_t len);
//
void switch_to_gpio();
void switch_to_uart();
uint8_t calculate_checksum(uint8_t *buffer, uint8_t payload_size);


#endif /* API_BUS_PGM_COMMUM_BUS_H_ */


