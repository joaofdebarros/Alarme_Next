/*
 * teclado.c
 *
 *  Created on: 5 de set. de 2025
 *      Author: diego.marinho
 */


#include "teclado.h"

teclado_t teclado;

#define TECLADO_TASK_NAME            ("teclado")
#define TECLADO_TASK_STACK_SIZE      (1536)
#define TECLADO_TASK_PRIORITY        (tskIDLE_PRIORITY + 1)


void teclado_init(hEvents_t eventHandle, uint32_t eventIndex,uint8_t *key) {

	teclado.saved_key = key;

	teclado.event.handle = eventHandle;
	teclado.event.index = eventIndex;

	teclado.status_rtos_teclado = cy_rtos_thread_create(&teclado.thread_teclado, teclado_task, TECLADO_TASK_NAME, NULL,TECLADO_TASK_STACK_SIZE, TECLADO_TASK_PRIORITY, NULL);
}

void teclado_task(void *pvParameters)
{
	packet_teclado_void_t SendTeclado;

    (void) pvParameters;
    for(;;)
    {
    	cy_rtos_thread_wait_notification(portMAX_DELAY);
    	if(teclado.function_TECLADO == TECLADO_REGISTER){
    		SendTeclado.cmd = TECLADOCMD_REGISTER;
    		//SendTeclado.timestamp = rtc_get_timestamp();
    		SendTeclado.timestamp = 35;
    		SendTeclado.data.raw[0] = 0;
    		SendTeclado.data.len = 1;

    		Send_Packet_Teclado(&SendTeclado,teclado.saved_key,TECLADO_ID,0,TECLADO_REGISTER);
    	}
    }
}

void teclado_start_register(void){
	teclado.function_TECLADO = TECLADO_REGISTER;
	cy_rtos_thread_set_notification(&teclado.thread_teclado);
}



void Send_Packet_Teclado(packet_teclado_void_t *p, uint8_t *key, uint8_t id, uint8_t addr, uint8_t func){
    static uint8_t sendReponse[128]; // using this global to reduce stack size fo the task
    uint8_t len;
    packet_teclado_t t;
    memset(t.data, 0, 256);  // Zerando 128 bytes manualmente
    t.id = 0;

    packet_get_and_encrypt_teclado(p, key, id, addr, func, &t, sendReponse, &len);

    packet_transmit_bus(sendReponse,len);
}

void registrar_teclado(uint8_t crc, uint8_t *count) {
	bool cadastrar = false;
	bool recalcular_crc = false;
	packet_teclado_void_t SendTecladoCommit;


	for (int i = 0; i < 3; i++) {
		if (crc != 0) {
			cadastrar = true;

			if (crc == teclado.teclado_registrado[i].crc) {
				cadastrar = false;

				break;
			}
		}
	}

	if (cadastrar) {
		teclado.teclado_registrado[teclado.teclado_count].crc = crc;
		teclado.teclado_registrado[teclado.teclado_count].num = teclado.teclado_count;
		*count = teclado.teclado_count;
		vTaskDelay(5);
		for(int i = 0; i < 2; i++){
//			SendTecladoCommit.cmd = TECLADOCMD_ADD_COMMIT;
//    		//SendTeclado.timestamp = rtc_get_timestamp();
//			SendTecladoCommit.timestamp = 35;
//			SendTecladoCommit.data.raw[0] = 0;
//			SendTecladoCommit.data.len = 1;
//
//    		Send_Packet_Teclado(&SendTecladoCommit,teclado.saved_key,TECLADO_ID,0,TECLADO_WORK);
		}
		teclado.teclado_count++;
	}
}

void teclado_get_status(uint8_t address_byte, uint8_t *teclado_number) {
	packet_teclado_void_t SendTeclado;

	SendTeclado.cmd = TECLADOCMD_REGISTER;
	SendTeclado.timestamp = 1;
	SendTeclado.data.raw[0] = 0;
	SendTeclado.data.len = 1;

	Send_Packet_Teclado(&SendTeclado,teclado.saved_key,TECLADO_ID,address_byte,TECLADO_REGISTER);

}


