/*
 * teclado.h
 *
 *  Created on: 5 de set. de 2025
 *      Author: diego.marinho
 */

#ifndef API_BUS_TECLADO_TECLADO_H_
#define API_BUS_TECLADO_TECLADO_H_

#include "cy_pdl.h"
#include "cybsp.h"
#include "alarm.h"
#include "cyabs_rtos.h"
#include "commum_bus.h"
#include "packet.h"
#include "packet_bus.h"

typedef enum {
  TECLADO_ID = 4,
  TECLADO_BROADCAST_ID,
  TECLADO_RESPONSE_ID
} ID_TECLADO_t;

typedef struct {
  uint8_t crc;
  uint8_t num;
} teclado_registrado_t;


typedef struct{
    struct{
        hEvents_t waitInt;
        hEvents_t handle;
        uint32_t index;
    }event;

	cy_rslt_t status_rtos_teclado;
	cy_thread_t thread_teclado;
	packet_teclado_t packet;
	packet_teclado_void_t TecladoPacket;
	uint8_t *saved_key;

	function_TECLADO_t function_TECLADO;

	teclado_registrado_t teclado_registrado[3];
	uint8_t teclado_count;

}teclado_t;

extern teclado_t teclado;

void teclado_init(hEvents_t eventHandle, uint32_t eventIndex,uint8_t *key);
void teclado_task(void *pvParameters);


void Send_Packet_Teclado(packet_teclado_void_t *p, uint8_t *key, uint8_t id, uint8_t addr, uint8_t func);
void teclado_start_register(void);

void registrar_teclado(uint8_t crc, uint8_t *count);
void teclado_get_status(uint8_t address_byte, uint8_t *teclado_number);


#endif /* API_BUS_TECLADO_TECLADO_H_ */
