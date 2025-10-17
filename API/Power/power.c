/*
 * Battery.c
 *
 *  Created on: 1 de set. de 2025
 *      Author: joao.victor
 */

#include "power.h"
#include "cy_gpio.h"
#include "cycfg_pins.h"

power_t power;

uint16_t duty_cycle;
bool short_circuited = false;
/* The status mask for the potentiometer SAR channel */
#define CYBSP_BAT_CHAN_MSK          (1UL << 9)
#define CYBSP_AC_CHAN_MSK          (1UL << 9)

/* HPPASS block ready status*/
volatile bool hppass_is_ready = false;

/* ADC conversion starting flag */
volatile bool start_adc_conversion = false;

/* The TCPWM interrupt configuration structure */
//cy_stc_sysint_t adc_pwm2_intr_config =
//{
//    .intrSrc = ADC_PWM2_IRQ,
//    .intrPriority = 0U,
//};

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
		
		control_power();

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
    
	power.battery_result = Cy_HPPASS_SAR_Result_ChannelRead(9);
	power.ac_result = Cy_HPPASS_SAR_Result_ChannelRead(11);
	power.auxiliar_result = Cy_HPPASS_SAR_Result_ChannelRead(8);
	
	/* Clear result status */
    Cy_HPPASS_SAR_Result_ClearStatus(CYBSP_BAT_CHAN_MSK);
	Cy_GPIO_Inv(LED2_PORT, LED2_PIN);
}

void pwm_duty_cycle(uint8_t percentage){
	uint16_t duty_cycle;
	
	duty_cycle = 150 * percentage;
	
	Cy_TCPWM_PWM_SetCompare0Val(BAT_CHARGE_PWM_HW, BAT_CHARGE_PWM_NUM, duty_cycle);
	
}

void control_power(){
	
	switch (power.auxiliar_status) {
	case AUXILIAR_OK:
		
		break;
	case AUXILIAR_OVERLOAD:
		
		break;
	case AUXILIAR_SHORT_CIRCUIT:
		
		break;
	}
	
	switch (power.battery_status) {
	case BATTERY_OK:
		if(power.ac_status == AC_OK){
			Cy_GPIO_Write(LED1_PORT, LED1_PIN, 0);
			
			/* Then start the PWM */
	  		Cy_TCPWM_TriggerStart_Single(BAT_CHARGE_PWM_HW, BAT_CHARGE_PWM_NUM);
			if(power.battery_result < BATTERY_LVL_1){
				pwm_duty_cycle(10);
				
			}else if(power.battery_result < BATTERY_LVL_2){
				pwm_duty_cycle(15);
				
			}else if(power.battery_result < BATTERY_LVL_3){
				pwm_duty_cycle(25);
				
			}else if(power.battery_result < BATTERY_LVL_4){
				pwm_duty_cycle(50);
				
			}else if(power.battery_result < BATTERY_LVL_5){
				pwm_duty_cycle(90);
				
			}else if(power.battery_result > BATTERY_LVL_5){
				pwm_duty_cycle(2);
				
			}
		}else if(power.ac_status == AC_DISCONNECTED){
			/* Then stop the PWM */
	  		Cy_TCPWM_TriggerStopOrKill_Single(BAT_CHARGE_PWM_HW, BAT_CHARGE_PWM_NUM);
			if(power.battery_result < 1170){ 
				/* Desliga o sistema quando chega a 10,8V na bateria*/
				Cy_GPIO_Write(PROTECT_GND_BAT_PORT, PROTECT_GND_BAT_PIN, 0);
			
			}else if(power.battery_result < BATTERY_LVL_2){
				
				
			}else if(power.battery_result < BATTERY_LVL_3){
				
				
			}else if(power.battery_result < BATTERY_LVL_4){
				
				
			}else if(power.battery_result < BATTERY_LVL_5){
				
				
			}else if(power.battery_result > BATTERY_LVL_5){
				
			}
		}
		
		break;	
	case BATTERY_SHORT_CIRCUIT:
	short_circuited = true;
		Cy_GPIO_Write(PROTECT_GND_BAT_PORT, PROTECT_GND_BAT_PIN, 0);
		break;
	case BATTERY_DISCONNECTED:
		if(power.ac_status == AC_OK && !short_circuited){
			Cy_GPIO_Write(PROTECT_GND_BAT_PORT, PROTECT_GND_BAT_PIN, 1);
			Cy_GPIO_Write(LED1_PORT, LED1_PIN, 1);
		}
		break;
	}	
	
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
	if(power.battery_result > BAT_MIN && power.battery_result < BAT_MAX){
		return BATTERY_DISCONNECTED;
	}else if(power.battery_result < BAT_MIN){
		return BATTERY_SHORT_CIRCUIT;
	}else{
		return BATTERY_OK;
	}
}

auxiliar_status_e check_aux(){
	if(power.auxiliar_result > AUX_MIN && power.auxiliar_result < AUX_MAX){
		return AUXILIAR_OVERLOAD;
	}else if(power.auxiliar_result <= AUX_MIN){
		return AUXILIAR_SHORT_CIRCUIT;
	}else{
		return AUXILIAR_OK;
	}
}


