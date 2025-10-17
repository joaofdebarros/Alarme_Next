/*
 * alarm.h
 *
 *  Created on: 19 de fev. de 2025
 *      Author: diego.marinho
 */

#ifndef ALARM_ALARM_H_
#define ALARM_ALARM_H_

#include "stdint.h"
#include "cyabs_rtos.h"

typedef void* hEvents_t;

/*
 * Macros
 */


/*
 * Enumerates
 */

typedef enum{
    ALARM_MODE_ONESHOT,
    ALARM_MODE_CONTINUOUS
}alarm_mode_t;

/*
 * Structs and Unions
 */

typedef struct{
    hEvents_t       event;
    uint_fast32_t   index;
    uint_fast64_t   timestamp;
    uint_fast64_t   offset;
    alarm_mode_t    mode;
}alarm_table_t;

typedef struct{
	cy_semaphore_t  alarm_semaphore;
	cy_thread_t     alarm_handle;
	cy_timer_t      app_timer_handle;

    uint_fast8_t    tRunning;
    uint_fast64_t   counter;
    uint_fast16_t   ticksMs;
    uint_fast8_t    isManual;
}alarm_cfg_t;


/*
 * Prototypes
 */

void alarm_init(uint16_t period);

//uint8_t alarm_setAlarm_ts(uint32_t index, uint32_t timestamp);

uint8_t alarm_setAlarm_os(hEvents_t eventHandle, uint32_t index, uint64_t ms, alarm_mode_t mode);

void alarm_cancelAlarm(uint32_t index);

uint64_t alarm_getCounter();

void alarm_interrupt_handler();
void app_timer_callback(cy_timer_callback_arg_t arg);



#endif /* ALARM_ALARM_H_ */
