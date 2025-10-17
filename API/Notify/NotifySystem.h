/*
 * NotifySystem.h
 *
 *  Created on: 5 de mar. de 2025
 *      Author: diego.marinho
 */

#ifndef NOTIFY_NOTIFYSYSTEM_H_
#define NOTIFY_NOTIFYSYSTEM_H_


#include <stdint.h>   // Tipos padrão
#include <stdbool.h>  // Tipo booleano
#include "string.h"
#include "cyabs_rtos.h"
#include "alarm.h"
#include "timestamp.h"

#define NOTIFICATION_MAX_QUEUE 10
#define ITEM_SIZE sizeof(notify_struct_u)
#define NYSY_STRUCT_VERSION     2

#define SIZEOF_LONG_NOTIFY		30+12
#define NUM_NOTIFY 10

typedef enum{
	NOTIFY_ERR_OK,
	NOTIFY_ERR_FAIL,
	NOTIFY_ERR_BAD,
	NOTIFY_ERR_FULL,
	NOTIFY_ERR_EMPTY,
	NOTIFY_ERR_UNKNOWN = 0xFF
}notify_error_e;

typedef enum{
	METHOD_SENSOR_WIRE,
	METHOD_SENSOR_WIRELESS,
	METHOD_USER_CONTROL,
	METHOD_USER_APP,
	METHOD_USER_TECLADO,
	METHOD_USER_CENARIO
}notify_method_e;

typedef enum{
	TYPE_SETOR_VIOLATED = 1,
	TYPE_SETOR_FRAUDE,
	TYPE_PARTITION_ARMED,
	TYPE_PARTITION_DISARMED,
}notify_type_e;

typedef union{
	struct __attribute__ ((__packed__)){
	    uint32_t id;
		uint32_t timestamp;
		notify_type_e Type;
		notify_method_e Method;
		uint8_t uuid[16];
		uint8_t uuid2[16];
	};
	uint8_t raw[SIZEOF_LONG_NOTIFY];
}notify_struct_u;

typedef struct{
    uint32_t ID;
	uint8_t payload[64];
	uint8_t len;
}notify_enc_struct_t;

typedef struct{
    struct{
        hEvents_t waitInt;
        hEvents_t handle;
        uint32_t index;
    }event;
	cy_queue_t notify_queue;
}notify_t;

extern notify_t notify;

void initNotificationSystem();
void notify_task(void *pvParameters);
notify_struct_u nysy_generateStruct(notify_method_e Method, notify_type_e Type, uint8_t *name_uuid, uint8_t len_id, uint8_t *uuid_secondary);


#endif /* NOTIFY_NOTIFYSYSTEM_H_ */
