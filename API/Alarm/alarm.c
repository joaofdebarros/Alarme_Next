/*
 * alarm.c
 *
 *  Created on: 19 de fev. de 2025
 *      Author: diego.marinho
 */

#include "alarm.h"
#include "string.h"


#define ALARM_TABLE_QUANTITY      15

static alarm_table_t table[ALARM_TABLE_QUANTITY];
static alarm_cfg_t cfg;


#define ALARM_TASK_NAME            ("ALARM")
#define ALARM_TASK_STACK_SIZE      (configMINIMAL_STACK_SIZE)
#define ALARM_TASK_PRIORITY        (tskIDLE_PRIORITY + 1)


void alarm_task(void *pvParameters)
{
	(void) pvParameters;
    uint32_t sum;
    uint8_t i;
    cy_rslt_t semRet;

	for(;;)
	{
		semRet = cy_rtos_semaphore_get(&cfg.alarm_semaphore,0xFFFFFFFF);
        sum = 0;
        if (semRet == CY_RSLT_SUCCESS){
          if (cfg.tRunning == 1){
              for (i=0; i<ALARM_TABLE_QUANTITY; i++){
                  sum += table[i].index;
                  if (table[i].index != 0){
                      if (table[i].timestamp <= cfg.counter){
                          if (table[i].event != NULL)
                              cy_rtos_event_setbits(table[i].event,table[i].index);
                          if (table[i].mode == ALARM_MODE_ONESHOT){
                              table[i].index = 0;
                          }
                          else{
                              table[i].timestamp = cfg.counter + table[i].offset;
                          }
                      }
                  }
              }
              if (sum == 0){
                  cfg.tRunning = 0;
              }
          }
        }
	}


}

uint8_t alarm_setAlarm_os(hEvents_t eventHandle, uint32_t index, uint64_t ms, alarm_mode_t mode){
    uint8_t i, freeIdx;

    freeIdx = 0xFF;
    if (ms == 0){
        return 0;
    }
    for (i=0; i<ALARM_TABLE_QUANTITY; i++){
        // check if alarm is free or is the same index
        if (table[i].index == 0 || table[i].index == index){
            freeIdx = i;
            if (table[i].index == index){
                break;
            }
        }
    }
    if (freeIdx<ALARM_TABLE_QUANTITY){
        table[freeIdx].event = eventHandle;
        table[freeIdx].index = index;
        table[freeIdx].mode = mode;
        table[freeIdx].offset = ms;
        table[freeIdx].timestamp = cfg.counter + ms;
        if (!cfg.tRunning){
            cfg.tRunning = 1;
        }

        return 1;
    }
    else{
        return 0;
    }
}

void alarm_cancelAlarm(uint32_t index){
    uint8_t i;

    for (i=0; i<ALARM_TABLE_QUANTITY; i++){
        if (table[i].index == index){
            break;
        }
    }
    if (i<ALARM_TABLE_QUANTITY){
        table[i].index = 0;
    }
}

uint64_t alarm_getCounter(){
    return cfg.counter;
}

void alarm_interrupt_handler(){
    cfg.counter += cfg.ticksMs;
    cy_rtos_semaphore_set(&cfg.alarm_semaphore);
}


void alarm_init(uint16_t period){
	const char *msg = "Hello RTOS!";
    memset(table, 0, sizeof(alarm_table_t)*ALARM_TABLE_QUANTITY);
    cy_rtos_semaphore_init(&cfg.alarm_semaphore,1,0);
    cy_rtos_thread_create(&cfg.alarm_handle, alarm_task,ALARM_TASK_NAME,NULL,CY_RTOS_MIN_STACK_SIZE,ALARM_TASK_PRIORITY,NULL);

    cy_rtos_timer_init(&cfg.app_timer_handle,CY_TIMER_TYPE_PERIODIC,app_timer_callback,(cy_timer_callback_arg_t)msg);
    cy_rtos_timer_start(&cfg.app_timer_handle, 200);

    cfg.tRunning = 0;
    cfg.counter = 0;
    cfg.ticksMs = period;
    cfg.isManual = 0;
}

/*******************************************************************************
 * Function Name: timer_callback
 ********************************************************************************
 * Summary:
 *  This is the callback function for the RTOS timer. It is triggered when the
 *  timer expires and executes the defined action.
 *
 * Parameters:
 *  void *arg : Argument passed during timer initialization.
 *
 * Return:
 *  None
 */
void app_timer_callback(cy_timer_callback_arg_t arg){
	alarm_interrupt_handler();
}
