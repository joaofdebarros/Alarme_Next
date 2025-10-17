/*
 * 4G.h
 *
 *  Created on: 14 de mar. de 2025
 *      Author: diego.marinho
 */

#ifndef API_MODULES_4G_4G_H_
#define API_MODULES_4G_4G_H_

#include "stdint.h"
#include "alarm.h"
#include "cy_pdl.h"
#include "cybsp.h"
#include "stdio.h"
#include "Connectivity.h"
#include "callback.h"

typedef enum{
	HeaderLTE = 0,
	LEN_LTE,
	ID_LTE,
	DADO_LTE
}state_serialLTE_t;

typedef enum{
	MQTT_DISCONNECTED = 0,
	MQTT_CONNECTING,
	MQTT_CONNECT_FAIL,
	MQTT_CONNECTED
}Status_Mqtt_Connect_t;

typedef struct{
	char* broker_address;
	char* username;
	char* password;
}mqtt_config_t;

typedef struct{
	char* topic;
	char* data;
	uint8_t length;
}mqtt_pub_t;

typedef struct{
	char* topic;
	char* data;
	uint8_t length;
}mqtt_sub_t;

typedef struct{
    struct{
        hEvents_t waitInt;
        hEvents_t handle;
        uint32_t index;
    }event;

    cy_rslt_t status_rtos_lte; //variável de status RTOS
    cy_thread_t thread_lte; //variável contexto da task RTOS

    uint8_t byte_receive; //armazena os dados de recepção da serial
    cy_stc_scb_uart_context_t uartContext; //contexto da usart
    mqtt_config_t mqtt_config; //configuração do MQTT
    mqtt_pub_t mqtt_pub;
    mqtt_sub_t mqtt_sub;

    Status_Mqtt_Connect_t Status_Mqtt_Connect;


    uint8_t payload_receive[128];
    uint8_t length_payload;
    uint8_t lenght_data_cryp;
    state_serialLTE_t state;

    uint8_t count_header;
    uint8_t data[200];
}lte_t;

extern lte_t lte;

void LTE_init(hEvents_t eventHandle, uint32_t eventIndex);
void LTE_task(void *pvParameters);


Status_Mqtt_Connect_t Get_MQTTStatus_Connect(void);

void PowerOnA7672(void);
void PowerOffA7672(void);
void ResetA7672(void);

void packet_transmit_4G(uint8_t *buffer, uint8_t len);


#endif /* API_MODULES_4G_4G_H_ */
