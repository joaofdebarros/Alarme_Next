/*
 * Battery.c
 *
 *  Created on: 1 de set. de 2025
 *      Author: joao.victor
 */

#include "power.h"
#include "NotifySystem.h"
#include "cy_gpio.h"
#include "cy_syslib.h"
#include "cyabs_rtos.h"
#include "cycfg_pins.h"
#include <stdint.h>

power_t power;

uint16_t duty_cycle;
//bool battery_short_circuited = false;

ac_status_e last_ac_status = AC_OK;
battery_status_e last_battery_status = BATTERY_OK;
uint16_t last_battery_level;
auxiliar_status_e last_auxiliar_status = AUXILIAR_OK;
sirene_status_e last_sirene_status = SIRENE_OK;

uint16_t siren_timeout_time = 0;

notify_type_e notify_type;
notify_method_e notify_method;
	
/* The status mask for the potentiometer SAR channel */
#define CYBSP_BAT_CHAN_MSK          (1UL << 9)
#define CYBSP_AC_CHAN_MSK          (1UL << 9)

/* HPPASS block ready status*/
volatile bool hppass_is_ready = false;

/* ADC conversion starting flag */
volatile bool start_adc_conversion = false;
bool boot_completed = false;

/* Assign CONNECTIVITY_TASK */
#define POWER_TASK_NAME            ("POWER")
#define POWER_TASK_STACK_SIZE      (configMINIMAL_STACK_SIZE)
#define POWER_TASK_PRIORITY        (tskIDLE_PRIORITY + 1)

/* HPPASS interrupt handler */
void hppass_intr_handler(void);

static void ADC_POWER_Init(void);

/* TCPWM interrupt handler */
void adc_pwm2_intr_handler(void);

void power_init(hEvents_t eventHandle, uint32_t eventIndex){
	power.event.handle = eventHandle;
	power.event.index = eventIndex;
	power.status_rtos_connectivity = cy_rtos_thread_create(&power.thread_power, power_task,POWER_TASK_NAME,NULL,2*POWER_TASK_STACK_SIZE,POWER_TASK_PRIORITY,NULL);
	ADC_POWER_Init();
}


/*******************************************************************************
 * Function Name: power_task
 ********************************************************************************
 * Summary:
 *  This RTOS task toggles the User LED each time the semaphore is obtained.
 *
 * Parameters:
 *  void *pvParameters : Task parameter defined during task creation (unused)
 *
 * Return:
 *  The RTOS task never returns.
 *
 *******************************************************************************/
void power_task(void *pvParameters)
{
    (void) pvParameters;
    for(;;)
    {
    	vTaskDelay(1000);
    	
		get_ad_read();
		
		power.ac_status = check_ac();
		power.battery_status = check_bat();
		power.auxiliar_status = check_aux();
		
		if(boot_completed){
			power.sirene_status = check_sirene();
			notify_change();
		}
		
		control_power();
		
		SIRENE_timeout();

    }
}

static void ADC_POWER_Init(void){
    if(CY_HPPASS_SUCCESS != Cy_HPPASS_AC_Start(0U, 1000U))
    {
        CY_ASSERT(0);
    }

	/* The HPPASS interrupt configuration structure */
    cy_stc_sysint_t hppass_intr_config =
    {
        .intrSrc = pass_interrupt_mcpass_IRQn,
        .intrPriority = 0U,
    };
    
    /* Configure HPPASS interrupt */
    Cy_HPPASS_SetInterruptMask(CY_HPPASS_INTR_AC_INT);
    Cy_SysInt_Init(&hppass_intr_config, hppass_intr_handler);
    NVIC_EnableIRQ(hppass_intr_config.intrSrc);
	
	/* Start the HPPASS autonomous controller (AC) from state 0, didn't wait for HPPASS block ready */
    hppass_is_ready = false;
    if(CY_HPPASS_SUCCESS != Cy_HPPASS_AC_Start(0U, 0U))
    {
        CY_ASSERT(0);
    }

    /* CLear ADC conversion starting flag */
    start_adc_conversion = false;
}

void hppass_intr_handler(void)
{
    uint32_t intrStatus = Cy_HPPASS_GetInterruptStatusMasked();
    /* Clear interrupt */
    Cy_HPPASS_ClearInterrupt(intrStatus);

    /* Check AC interrupt */
    if(CY_HPPASS_INTR_AC_INT == (intrStatus & CY_HPPASS_INTR_AC_INT))
    {
        hppass_is_ready = true;
    }
}

void get_Power_notification(notify_type_e *Type, notify_method_e *Method){
	memcpy(Type,&notify_type,sizeof(notify_type_e));
	memcpy(Method,&notify_method,sizeof(notify_method_e));
}

void get_ad_read(){
	uint32_t result_status = 0;
	
	Cy_TCPWM_TriggerStopOrKill_Single(BAT_CHARGE_PWM_HW, BAT_CHARGE_PWM_NUM);
	vTaskDelay(1);
	
	 /* Trigger SAR ADC group 0 conversion */
    Cy_HPPASS_SetFwTrigger(CY_HPPASS_TRIG_1_MSK);
    
    /* Wait for channel conversion done */
    do
    {
        result_status = Cy_HPPASS_SAR_Result_GetStatus();
    } while(!(result_status & CYBSP_BAT_CHAN_MSK));
    
    power.auxiliar_result = ((Cy_HPPASS_SAR_Result_ChannelRead(8)) * 4.58);
	power.battery_result = ((Cy_HPPASS_SAR_Result_ChannelRead(9)) * 4.58);
	power.sirene_result = Cy_HPPASS_SAR_Result_ChannelRead(10);
	power.ac_result = ((Cy_HPPASS_SAR_Result_ChannelRead(11)) * 4.58);
	
	/* Clear result status */
    Cy_HPPASS_SAR_Result_ClearStatus(CYBSP_BAT_CHAN_MSK);
	Cy_GPIO_Inv(LED2_PORT, LED2_PIN);
}

void notify_change(){
	
	if(power.battery_status != last_battery_status){
		notify_type = TYPE_BATTERY;
		switch(power.battery_status){
			case BATTERY_OK:
				last_battery_status = BATTERY_OK;
				notify_method = METHOD_BATTERY_CONNECTED;
				break;
			case BATTERY_SHORT_CIRCUIT:
				last_battery_status = BATTERY_SHORT_CIRCUIT;
				notify_method = METHOD_BATTERY_SHORT_CIRCUITED;
				break;
			case BATTERY_DISCONNECTED:
				last_battery_status = BATTERY_DISCONNECTED;
				notify_method = METHOD_BATTERY_DISCONNECTED;
				break;
		}
		cy_rtos_event_setbits(power.event.handle,power.event.index);
		cy_rtos_delay_milliseconds(1);
	}
	
	if(power.auxiliar_status != last_auxiliar_status){
		notify_type = TYPE_AUX;
		switch(power.auxiliar_status){
			case AUXILIAR_OK:
				last_auxiliar_status = AUXILIAR_OK;
				notify_method = METHOD_AUX_REGULAR;
				break;
			case AUXILIAR_SHORT_CIRCUIT:
				last_auxiliar_status = AUXILIAR_SHORT_CIRCUIT;
				notify_method = METHOD_AUX_SHORTCIRCUITED;
				break;
			case AUXILIAR_OVERLOAD:
				last_auxiliar_status = AUXILIAR_OVERLOAD;
				notify_method = METHOD_AUX_OVERLOADED;
				break;
		}
		cy_rtos_event_setbits(power.event.handle,power.event.index);
		cy_rtos_delay_milliseconds(1);
	}
	
	if(power.ac_status != last_ac_status){
		notify_type = TYPE_AC;
		switch(power.ac_status){
			case AC_OK:
				last_ac_status = AC_OK;
				notify_method = METHOD_AC_CONNECTED;
				break;
			case AC_VERY_HIGH:
				last_ac_status = AC_VERY_HIGH;
				notify_method = METHOD_AC_IRREGULAR;
				break;
			case AC_DISCONNECTED:
				last_ac_status = AC_DISCONNECTED;
				notify_method = METHOD_AC_DISCONNECTED;
				break;
		}
		cy_rtos_event_setbits(power.event.handle,power.event.index);
		cy_rtos_delay_milliseconds(1);
	}
	
	if(power.sirene_status != last_sirene_status){
		notify_type = TYPE_SIRENE;
		switch(power.sirene_status){
			case SIRENE_OK:
				last_sirene_status = SIRENE_OK;
				notify_method = METHOD_SIRENE_CONNECTED;
				break;
			case SIRENE_SHORT_CIRCUIT:
				last_sirene_status = SIRENE_SHORT_CIRCUIT;
				notify_method = METHOD_SIRENE_SHORTCIRCUITED;
				break;
			case SIRENE_DISCONNECTED:
				last_sirene_status = SIRENE_DISCONNECTED;
				notify_method = METHOD_SIRENE_DISCONNECTED;
				break;
			case SIRENE_UNAVAILABLE:
				last_sirene_status = SIRENE_UNAVAILABLE;
				notify_method = METHOD_SIRENE_UNAVAILABLE;
				break;
		}
		cy_rtos_event_setbits(power.event.handle,power.event.index);
		cy_rtos_delay_milliseconds(1);
	}
	
}

void control_power(){
	
	switch (power.ac_status) {
		case AC_OK:
			last_ac_status = AC_OK;
			switch (power.battery_status) {
				case BATTERY_OK:
					last_battery_status = BATTERY_OK;
					/* Then start the PWM */
	  				bat_pwm(ON);
					Cy_GPIO_Write(LED1_PORT, LED1_PIN, 0);
			
					if(power.battery_result < BATTERY_LVL_1){
						pwm_duty_cycle(10);
						last_battery_level = BATTERY_VERY_LOW;
				
					}else if(power.battery_result < BATTERY_LVL_2){
						pwm_duty_cycle(15);
						last_battery_level = BATTERY_VERY_LOW;
				
					}else if(power.battery_result < BATTERY_LVL_3){
						pwm_duty_cycle(25);
						last_battery_level = BATTERY_LOW;
				
					}else if(power.battery_result < BATTERY_LVL_4){
						pwm_duty_cycle(50);
						last_battery_level = BATTERY_MEDIUM;
							
					}else if(power.battery_result < BATTERY_LVL_5){
						pwm_duty_cycle(90);
						last_battery_level = BATTERY_HIGH;
							
					}else if(power.battery_result > BATTERY_LVL_5){
						pwm_duty_cycle(2);
						last_battery_level = BATTERY_FULL;	
					}
					break;	
		
				case BATTERY_SHORT_CIRCUIT:
					last_battery_status = BATTERY_SHORT_CIRCUIT;
					bat_gnd(OFF);
					break;
						
				case BATTERY_DISCONNECTED:
					last_battery_status = BATTERY_DISCONNECTED;
					bat_gnd(ON);
					boot_completed = true;
					bat_pwm(ON);
					pwm_duty_cycle(5);
					Cy_GPIO_Write(LED1_PORT, LED1_PIN, 1);
					break;	
			}	
			
			switch (power.auxiliar_status) {
				case AUXILIAR_OK:
					last_auxiliar_status = AUXILIAR_OK;
					break;
				case AUXILIAR_OVERLOAD:
					last_auxiliar_status = AUXILIAR_OVERLOAD;
					break;
				case AUXILIAR_SHORT_CIRCUIT:
					last_auxiliar_status = AUXILIAR_SHORT_CIRCUIT;
					break;
			}
			
			switch (power.sirene_status) {
				case SIRENE_OK:
					last_sirene_status = SIRENE_OK;
					break;
				case SIRENE_SHORT_CIRCUIT:
					last_sirene_status = SIRENE_SHORT_CIRCUIT;
					break;
				case SIRENE_DISCONNECTED:
					last_sirene_status = SIRENE_DISCONNECTED;
					break;
				case SIRENE_UNAVAILABLE:
					last_sirene_status = SIRENE_UNAVAILABLE;
					break;
			}
			
			break; // break do AC_OK
			
		case AC_DISCONNECTED:
			
			bat_pwm(OFF);
			
			switch (power.battery_status) {
				case BATTERY_OK:
					last_battery_status = BATTERY_OK;
					break;	
		
				case BATTERY_SHORT_CIRCUIT:
					bat_gnd(OFF);
					last_battery_status = BATTERY_SHORT_CIRCUIT;
					break;
						
				case BATTERY_DISCONNECTED:
					last_battery_status = BATTERY_DISCONNECTED;
					break;	
			}
			
			switch (power.auxiliar_status) {
				case AUXILIAR_OK:
					last_auxiliar_status = AUXILIAR_OK;
					break;
				case AUXILIAR_OVERLOAD:
					last_auxiliar_status = AUXILIAR_OVERLOAD;
					break;
				case AUXILIAR_SHORT_CIRCUIT:
					last_auxiliar_status = AUXILIAR_SHORT_CIRCUIT;
					break;
			}
			
			switch (power.sirene_status) {
				case SIRENE_OK:
					last_sirene_status = SIRENE_OK;
					break;
				case SIRENE_SHORT_CIRCUIT:
					last_sirene_status = SIRENE_SHORT_CIRCUIT;
					break;
				case SIRENE_DISCONNECTED:
					last_sirene_status = SIRENE_DISCONNECTED;
					break;
				case SIRENE_UNAVAILABLE:
					last_sirene_status = SIRENE_UNAVAILABLE;
					break;
			}
			
			break; // break do AC_DISCONNECTED
		case AC_VERY_HIGH:
		
			switch (power.battery_status) {
				case BATTERY_OK:
					
					break;	
		
				case BATTERY_SHORT_CIRCUIT:
					
					break;
						
				case BATTERY_DISCONNECTED:
					
					break;	
			}
			
			switch (power.auxiliar_status) {
				case AUXILIAR_OK:
					last_auxiliar_status = AUXILIAR_OK;
					break;
				case AUXILIAR_OVERLOAD:
					last_auxiliar_status = AUXILIAR_OVERLOAD;
					break;
				case AUXILIAR_SHORT_CIRCUIT:
					last_auxiliar_status = AUXILIAR_SHORT_CIRCUIT;
					break;
			}
			
			switch (power.sirene_status) {
				case SIRENE_OK:
					last_sirene_status = SIRENE_OK;
					break;
				case SIRENE_SHORT_CIRCUIT:
					last_sirene_status = SIRENE_SHORT_CIRCUIT;
					break;
				case SIRENE_DISCONNECTED:
					last_sirene_status = SIRENE_DISCONNECTED;
					break;
				case SIRENE_UNAVAILABLE:
					last_sirene_status = SIRENE_UNAVAILABLE;
					break;
			}
			
			break; // break do AC_VERY_HIGH
	}
	
	last_sirene_status = power.sirene_status;
	
}

void bat_gnd(bool state){
	Cy_GPIO_Write(PROTECT_GND_BAT_PORT, PROTECT_GND_BAT_PIN, state);
}

void bat_pwm(bool state){
	if(state == ON){
		Cy_TCPWM_TriggerStart_Single(BAT_CHARGE_PWM_HW, BAT_CHARGE_PWM_NUM);
	}else{
		Cy_TCPWM_TriggerStopOrKill_Single(BAT_CHARGE_PWM_HW, BAT_CHARGE_PWM_NUM);
	}
}

void pwm_duty_cycle(uint8_t percentage){
	uint16_t duty_cycle;
	
	duty_cycle = 150 * percentage;
	
	Cy_TCPWM_PWM_SetCompare0Val(BAT_CHARGE_PWM_HW, BAT_CHARGE_PWM_NUM, duty_cycle);
	
}

ac_status_e check_ac(){
	if(power.ac_result < AC_MIN){
		return AC_DISCONNECTED;
	}else if(power.ac_result > AC_MAX){
		return AC_VERY_HIGH;
	}else{
		return AC_OK;
	}
}

battery_status_e check_bat(){
	if(power.battery_result >= BATTERY_SHORT_CIRCUIT_THRESHOLD && power.battery_result < BATTERY_CONNECTED_THRESHOLD){
		return BATTERY_DISCONNECTED;
	}else if(power.battery_result < BATTERY_SHORT_CIRCUIT_THRESHOLD){
		return BATTERY_SHORT_CIRCUIT;
	}else{
		return BATTERY_OK;
	}
}

auxiliar_status_e check_aux(){
	if(power.auxiliar_result > AUX_MIN && power.auxiliar_result < AUX_MAX){
		return AUXILIAR_OK;
	}else if(power.auxiliar_result <= AUX_SHORT_CIRCUIT_THRESHOLD){
		return AUXILIAR_SHORT_CIRCUIT;
	}else{
		return AUXILIAR_OVERLOAD;
	}
	
}

sirene_status_e check_sirene(){
//	if(power.battery_status == BATTERY_DISCONNECTED){
//		return SIRENE_UNAVAILABLE;
//	}else{
		if(power.sirene_result >= SIRENE_NORMAL && power.sirene_result < SIRENE_DISCONNECTED_THRESHOLD){
			return SIRENE_OK;
		}else if(power.sirene_result <= SIRENE_SHORT_CIRCUIT_THRESHOLD){
			return SIRENE_SHORT_CIRCUIT;
		}else{
			return SIRENE_DISCONNECTED;
		}
//	}
		
}

sirene_status_e control_SIRENE(bool state, uint16_t timeout){
	if(state == ON){
		switch (power.sirene_status) {
			case SIRENE_OK:
				siren_timeout_time = timeout;
				Cy_GPIO_Write(SIRENE_ON_PORT, SIRENE_ON_PIN, 1);
				return SIRENE_OK;
				break;
			case SIRENE_DISCONNECTED:
				Cy_GPIO_Write(SIRENE_ON_PORT, SIRENE_ON_PIN, 0);
				return SIRENE_DISCONNECTED;
				break;
			case SIRENE_SHORT_CIRCUIT:
				Cy_GPIO_Write(SIRENE_ON_PORT, SIRENE_ON_PIN, 0);
				return SIRENE_SHORT_CIRCUIT;
				break;	
			case SIRENE_UNAVAILABLE:
				Cy_GPIO_Write(SIRENE_ON_PORT, SIRENE_ON_PIN, 0);
				return SIRENE_UNAVAILABLE;
				break;	
		}
	}else{
		Cy_GPIO_Write(SIRENE_ON_PORT, SIRENE_ON_PIN, 0);
	}
}

void SIRENE_timeout(void){
	if(--siren_timeout_time == 0){
		Cy_GPIO_Write(SIRENE_ON_PORT, SIRENE_ON_PIN, 0);
	}
}

