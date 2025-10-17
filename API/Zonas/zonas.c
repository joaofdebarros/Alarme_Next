/*
 * zonas.c
 *
 *  Created on: 27 de fev. de 2025
 *      Author: diego.marinho
 */


#include "zonas.h"

//zonas_t zonas;

zonas_t zonas = {
    .buffer_models = {
        [MODEL_PPA] = {200, 1100, 1500, 670, 800},
        [1] = {0}, // pode preencher outros modelos
        [2] = {0}
    }
};

/* The TCPWM interrupt configuration structure */
cy_stc_sysint_t tcpwm_intr_config =
{
    .intrSrc = ADC_PWM1_IRQ,
    .intrPriority = 7U,
};

/* Assign CONNECTIVITY_TASK */
#define ZONAS_TASK_NAME            ("ZONAS")
#define ZONAS_TASK_STACK_SIZE      (configMINIMAL_STACK_SIZE)
#define ZONAS_TASK_PRIORITY        (tskIDLE_PRIORITY + 1)

static void ADC_ZONAS_Init(void);

/* TCPWM interrupt handler */
void user_tcpwm_intr_handler(void);

void zonas_init(hEvents_t eventHandle, uint32_t eventIndex){
	zonas.event.handle = eventHandle;
	zonas.event.index = eventIndex;
	zonas.status_rtos_connectivity = cy_rtos_thread_create(&zonas.thread_zona, zonas_task,ZONAS_TASK_NAME,NULL,2*ZONAS_TASK_STACK_SIZE,ZONAS_TASK_PRIORITY,NULL);
	ADC_ZONAS_Init();

	zonas.setup.Type_Zona = Type_ZonaDupla_Paralelo;

	zonas.setup.Model_resistor = MODEL_PPA;
}


/*******************************************************************************
 * Function Name: zonas_task
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
void zonas_task(void *pvParameters)
{
	state_zona_t new_state;

    (void) pvParameters;
    for(;;)
    {
    	cy_rtos_thread_wait_notification(portMAX_DELAY);

    	for(uint8_t i = 0; i < NUM_ZONAS; i++){
    		new_state = analyze_zona(zonas.setup, zonas.adc_result_buf[i]);

    		if (new_state != zonas.config_zonas[i].state) {
    			zonas.config_zonas[i].state = new_state;
    			zonas.config_zonas[i].ADC = zonas.adc_result_buf[i];
    			//chamar uma função de callbak pra passar as informações
    			//ou por ponteiro -> acho que a melhor solução vai ser queue
    			//setar um evento
    			zonas.config_zonas[i].i_chanel = i+1;
    			zonas.config_zonas[i].status_channel = true;
    			cy_rtos_event_setbits(zonas.event.handle,zonas.event.index);
    		}
    	}

    }
}

static void ADC_ZONAS_Init(void){
    if(CY_HPPASS_SUCCESS != Cy_HPPASS_AC_Start(0U, 1000U))
    {
        CY_ASSERT(0);
    }

    /* Initialize TCPWM using the config structure generated using device configurator*/
    if (CY_TCPWM_SUCCESS != Cy_TCPWM_PWM_Init(ADC_PWM1_HW,ADC_PWM1_NUM, &ADC_PWM1_config))
    {
        CY_ASSERT(0);
    }
    /* Enable the initialized TCPWM */
    Cy_TCPWM_PWM_Enable(ADC_PWM1_HW, ADC_PWM1_NUM);

    /* Configure user TCPWM interrupt */
    Cy_SysInt_Init(&tcpwm_intr_config, user_tcpwm_intr_handler);
    NVIC_EnableIRQ(tcpwm_intr_config.intrSrc);

    //__enable_irq();

    /*Start the timer*/
    Cy_TCPWM_TriggerStart_Single(ADC_PWM1_HW, ADC_PWM1_NUM);
}

/*******************************************************************************
* Function Name: user_tcpwm_intr_handler
********************************************************************************
* Summary:
* This is the TCPWM interrupt handler. This ISR read the ADC channel results
* and toggle the user LED.
*
* Parameters:
*  void
*
* Return:
*  void
*
*******************************************************************************/
void user_tcpwm_intr_handler(void)
{
    uint32_t intrStatus = Cy_TCPWM_GetInterruptStatusMasked(ADC_PWM1_HW, ADC_PWM1_NUM);

    Cy_TCPWM_ClearInterrupt(ADC_PWM1_HW, ADC_PWM1_NUM, intrStatus);

    /* Read all data from FIFO 0 */
    zonas.adc_result_buf[zonas.channel_z1_11] = Cy_HPPASS_FIFO_Read(0U, &zonas.channel_z1_11);
    zonas.adc_result_buf[zonas.channel_z2_12] = Cy_HPPASS_FIFO_Read(0U, &zonas.channel_z2_12);
    zonas.adc_result_buf[zonas.channel_z3_13] = Cy_HPPASS_FIFO_Read(0U, &zonas.channel_z3_13);
    zonas.adc_result_buf[zonas.channel_z4_14] = Cy_HPPASS_FIFO_Read(0U, &zonas.channel_z4_14);
    zonas.adc_result_buf[zonas.channel_z5_15] = Cy_HPPASS_FIFO_Read(0U, &zonas.channel_z5_15);
    zonas.adc_result_buf[zonas.channel_z6_16] = Cy_HPPASS_FIFO_Read(0U, &zonas.channel_z6_16);
    zonas.adc_result_buf[zonas.channel_z7_17] = Cy_HPPASS_FIFO_Read(0U, &zonas.channel_z7_17);
    zonas.adc_result_buf[zonas.channel_z8_18] = Cy_HPPASS_FIFO_Read(0U, &zonas.channel_z8_18);

    //adicionar um semaforo ou uma task notify
    cy_rtos_thread_set_notification(&zonas.thread_zona);

}

state_zona_t analyze_zona(setup_t setup,uint16_t read_ADC){
    switch (setup.Type_Zona) {
        case Type_ZonaDupla_Serie: // Zona dupla, 1K - 2K2 - EOL (série)
            if (read_ADC < (RESISTOR_1K / 2)) return TENTATIVA_FRAUDE;
            else if (read_ADC < RESISTOR_1K) return ZONA_INFERIOR_VIOLADA;
            else if (read_ADC > RESISTOR_2K2) return ZONA_SUPERIOR_VIOLADA;
            return ZONA_INVIOLADA;

        case Type_ZonaDupla_Paralelo: // Zona dupla, 1K - 2K2
            if (read_ADC < zonas.buffer_models[setup.Model_resistor][0]) return TENTATIVA_FRAUDE;
            else if (read_ADC > zonas.buffer_models[setup.Model_resistor][1] && read_ADC < zonas.buffer_models[setup.Model_resistor][2]) return ZONA_INFERIOR_VIOLADA;
            else if (read_ADC > zonas.buffer_models[setup.Model_resistor][3] && read_ADC < zonas.buffer_models[setup.Model_resistor][4]) return ZONA_SUPERIOR_VIOLADA;
            return ZONA_INVIOLADA;

        case Type_ZonaSimples: // Zona simples, EOL
            if (read_ADC < (RESISTOR1_PPA / 2)) return TENTATIVA_FRAUDE;
            if (read_ADC > RESISTOR1_PPA) return ZONA_VIOLADA;
            return ZONA_INVIOLADA;

        	break;

        case Type_ZonaSimplesSR: // Zona simples, sem resistor
            if (read_ADC < 500){
            	return ZONA_INVIOLADA;
            }
            else{
            	return ZONA_VIOLADA;
            }

        default:
            return ZONA_INVIOLADA;
    }
}

void Set_Configuration_setor(setup_t *setup_zona){
	zonas.setup.Type_Zona = setup_zona->Type_Zona;
	zonas.setup.Model_resistor = setup_zona->Model_resistor;
}

void Get_SetorViolation(uint8_t *ID_Setor, state_zona_t *state){
	for(uint8_t i = 0; i<8; i++){
		if(zonas.config_zonas[i].status_channel == true){
			zonas.config_zonas[i].status_channel = false;

			if(zonas.config_zonas[i].state == TENTATIVA_FRAUDE || zonas.config_zonas[i].state == ZONA_INVIOLADA){
				*ID_Setor = (i+1);
			}
			else{
				*ID_Setor = (i+1) + (zonas.config_zonas[i].state-1)*8;
			}

			*state = zonas.config_zonas[i].state;

			//memcpy(setup_chanel, &zonas.config_zonas[i],sizeof(setup_zonas_t));
			break;
		}
	}
}
