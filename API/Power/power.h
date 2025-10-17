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

#define AC_MIN 2100
#define AC_MAX 3500


#define BAT_MIN 10
#define BAT_MAX 1000
#define BATTERY_LVL_1 2293
#define BATTERY_LVL_2 2512
#define BATTERY_LVL_3 2620
#define BATTERY_LVL_4 2800
#define BATTERY_LVL_5 3030

#define AUX_MIN 2000
#define AUX_MAX 3000

typedef enum {
  AC_OK,
  AC_VERY_HIGH,
  AC_VERY_LOW,
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
	uint8_t channelB1;
	uint8_t channeB3;
	
	ac_status_e ac_status;
	battery_status_e battery_status;
	auxiliar_status_e auxiliar_status;
}power_t;

extern power_t power;

void power_init(hEvents_t eventHandle, uint32_t eventIndex);
void power_task(void *pvParameters);

ac_status_e check_ac();
battery_status_e check_bat();
auxiliar_status_e check_aux();
void control_power();
void get_ad_read();
#endif /* API_POWER_POWER_H_ */
