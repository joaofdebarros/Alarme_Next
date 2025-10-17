/*
 * Connectivity.h
 *
 *  Created on: 24 de fev. de 2025
 *      Author: diego.marinho
 */

#ifndef MODULES_CONNECTIVITY_CONNECTIVITY_H_
#define MODULES_CONNECTIVITY_CONNECTIVITY_H_


/* ======================== INCLUDES ======================== */
#include <stdint.h>   // Tipos padrão
#include <stdbool.h>  // Tipo booleano
#include "cyabs_rtos.h"
#include "cy_pdl.h"
#include "cybsp.h"
#include "packet.h"
#include "callback.h"

/* ======================== DEFINES ======================== */


/* ======================== MACROS ======================== */
#define MULTIPLICAR(a, b) ((a) * (b))  // Exemplo de macro

/* ======================== ENUMS ======================== */
typedef enum{
	Header = 0,
	LEN,
	ID,
	DADO
}state_t;

/* ======================== STRUCTS ======================== */


typedef struct{
    struct{
        hEvents_t waitInt;
        hEvents_t handle;
        uint32_t index;
    }event;
	uint8_t data[200];

	cy_rslt_t status_rtos_connectivity;
	cy_thread_t thread_connectivity;

	state_t state;
	uint8_t byte_receive;
	uint8_t count_header;
	uint8_t lenght_data_cryp;
}connectivity_t;

/* ======================== VARIÁVEIS EXTERNAS ======================== */
extern connectivity_t connectivity;

/* ======================== PROTÓTIPOS DE FUNÇÕES ======================== */
void connectivity_init(uint32_t eventIndex);
void connectivity_task(void *pvParameters);
void packet_transmit_connectivity(uint8_t *buffer, uint8_t len);



#endif /* MODULES_CONNECTIVITY_CONNECTIVITY_H_ */
