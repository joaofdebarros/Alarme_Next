/*
 * Battery.h
 *
 *  Created on: 1 de set. de 2025
 *      Author: joao.victor
 */

#ifndef API_POWER_POWER_H_
#define API_POWER_POWER_H_

#include "stdint.h"
#include "cyabs_rtos.h"
#include "alarm.h"
#include "NotifySystem.h"
#include "cy_pdl.h"
#include "cybsp.h"
#include "cy_gpio.h"
#include "cy_hppass_sar.h"
#include "cy_tcpwm.h"
#include "cy_tcpwm_pwm.h"
#include "cycfg_peripherals.h"
#include "cycfg_pins.h"
#include <stdint.h>
#include "mtb_hal.h"

#define AC_MIN 9175
#define AC_MAX 15000

#define BATTERY_SHORT_CIRCUIT_THRESHOLD 200
#define BATTERY_CONNECTED_THRESHOLD 10000
#define BATTERY_LVL_1 10518
#define BATTERY_LVL_2 11520
#define BATTERY_LVL_3 12010
#define BATTERY_LVL_4 12840
#define BATTERY_LVL_5 13900

#define AUX_MIN 9175
#define AUX_MAX 14000
#define AUX_SHORT_CIRCUIT_THRESHOLD 3000

#define SIRENE_SHORT_CIRCUIT_THRESHOLD 100
#define SIRENE_NORMAL 1000
#define SIRENE_DISCONNECTED_THRESHOLD 1500 

#define ON 1
#define OFF 0 

typedef enum {
  AC_OK,
  AC_VERY_HIGH,
  AC_DISCONNECTED
} ac_status_e;

typedef enum {
  BATTERY_OK,
  BATTERY_SHORT_CIRCUIT,
  BATTERY_DISCONNECTED,
} battery_status_e;

typedef enum {
  AUXILIAR_OK,
  AUXILIAR_SHORT_CIRCUIT,
  AUXILIAR_OVERLOAD,
} auxiliar_status_e;

typedef enum {
  SIRENE_OK,
  SIRENE_SHORT_CIRCUIT,
  SIRENE_DISCONNECTED,
  SIRENE_UNAVAILABLE
} sirene_status_e;

typedef enum{
  BATTERY_VERY_LOW,
  BATTERY_LOW,
  BATTERY_MEDIUM,
  BATTERY_HIGH,
  BATTERY_FULL,
} battery_level_t;

typedef struct{
    struct{
        hEvents_t waitInt;
        hEvents_t handle;
        uint32_t index;
    }event;

	cy_rslt_t status_rtos_connectivity;
	cy_thread_t thread_power;
	cy_queue_t power_queue;

	uint16_t battery_result;
	uint16_t ac_result;
	uint16_t auxiliar_result;
	uint16_t sirene_result;
	uint8_t channelB1;
	uint8_t channeB3;
	
	ac_status_e ac_status;
	battery_status_e battery_status;
	auxiliar_status_e auxiliar_status;
	sirene_status_e sirene_status;
}power_t;

extern power_t power;

void power_init(hEvents_t eventHandle, uint32_t eventIndex);
void power_task(void *pvParameters);

ac_status_e check_ac();
battery_status_e check_bat();
auxiliar_status_e check_aux();
sirene_status_e check_sirene();
void notify_change();
void get_Power_notification(notify_type_e *Type, notify_method_e *Method);
void control_power();
void pwm_duty_cycle(uint8_t percentage);
void get_ad_read();
void bat_gnd(bool state);
void bat_pwm(bool state);
sirene_status_e control_SIRENE(bool state, uint16_t timeout);
void SIRENE_timeout(void);
#endif /* API_POWER_POWER_H_ */
