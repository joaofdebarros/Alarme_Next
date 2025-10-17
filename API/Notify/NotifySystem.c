/*
 * NotifySystem.c
 *
 *  Created on: 5 de mar. de 2025
 *      Author: diego.marinho
 */

#include "NotifySystem.h"

notify_t notify;

static uint32_t notifyCounter;
cy_rslt_t sttaus_test;

/* Assign CONNECTIVITY_TASK */
#define NOTIFY_TASK_NAME            ("NOTIFY")
#define NOTIFY_TASK_STACK_SIZE      (configMINIMAL_STACK_SIZE)
#define NOTIFY_TASK_PRIORITY        (tskIDLE_PRIORITY + 1)


/*
 * Private functions
 */

static uint32_t __id_get_and_add(){
    uint32_t a;

    a = notifyCounter;
    notifyCounter++;

    return a;
}

void initNotificationSystem(hEvents_t eventHandle, uint32_t eventIndex){
	notify.event.handle = eventHandle;
	notify.event.index = eventIndex;
}

notify_struct_u nysy_generateStruct(notify_method_e Method, notify_type_e Type, uint8_t *name_uuid, uint8_t len_id, uint8_t *uuid_secondary){
	notify_struct_u n = {0};


    n.timestamp = rtc_get_timestamp();
    n.id = __id_get_and_add();
	n.Method = Method;
	n.Type = Type;
	if (name_uuid != NULL)
		memcpy(n.uuid, name_uuid, len_id);
	else
		memset(n.uuid, 0, len_id);

	if (uuid_secondary != NULL)
		memcpy(n.uuid2, uuid_secondary, 16);
	else
		memset(n.uuid2, 0, 16);

	return n;
}
