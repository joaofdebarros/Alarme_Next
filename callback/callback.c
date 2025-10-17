/*
 * callback.c
 *
 *  Created on: 24 de fev. de 2025
 *      Author: diego.marinho
 */

#include "callback.h"



void callback_connectivity_receive(uint8_t *data, uint8_t len, uint8_t mode){
	packet_error_e packetErr;
	packetErr = packet_data_demount(data, len,&app.app_connectivity.packet);
	app.Wireless_mode = app.app_connectivity.packet.id;
	app.Remote_Mode = mode;
	alarm_setAlarm_os(&app.app_event, APP_EVENT_CONNEC, 10, ALARM_MODE_ONESHOT);
}

void callback_longRange_receive(uint8_t *data, uint8_t len){
	packet_error_e packetErr;
	packetErr = packet_data_demount_LORA(data, len,&app.app_connectivity.packet);
	alarm_setAlarm_os(&app.app_event, APP_EVENT_LONGRANGE, 10, ALARM_MODE_ONESHOT);
}

void callback_teclado_receive(uint8_t *data, uint8_t len){
	packet_error_e packetErr;
	packetErr = packet_data_demount_teclado(data, len,&app.app_teclado.packet);
	alarm_setAlarm_os(&app.app_event, APP_EVENT_TECLADO, 10, ALARM_MODE_ONESHOT);
}


