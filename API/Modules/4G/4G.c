/*
 * 4G.c
 *
 *  Created on: 14 de mar. de 2025
 *      Author: diego.marinho
 */

#include "4G.h"

lte_t lte;


#define LTE_TASK_NAME            ("LTE")
#define LTE_TASK_STACK_SIZE      1024
#define LTE_TASK_PRIORITY        (tskIDLE_PRIORITY + 1)


#define UART_4G_INTR_NUM        ((IRQn_Type) scb_1_interrupt_IRQn)
#define UART_4G_INTR_PRIORITY   (7U)


static void Serial_LTE_Init(void);
static void packet_receive_4G(uint8_t *byte_receive);

void UART_4G_Isr(void);

void LTE_init(hEvents_t eventHandle, uint32_t eventIndex){
	lte.event.handle = eventHandle;
	lte.event.index = eventIndex;
	Serial_LTE_Init();
	lte.status_rtos_lte = cy_rtos_thread_create(&lte.thread_lte, LTE_task,LTE_TASK_NAME,NULL,LTE_TASK_STACK_SIZE,LTE_TASK_PRIORITY,NULL);
	PowerOnA7672();
}

/*******************************************************************************
 * Function Name: LTE_task
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
void LTE_task(void *pvParameters)
{
	//uint8_t i = 0;
    (void) pvParameters;
    for(;;)
    {
    	cy_rtos_thread_wait_notification(portMAX_DELAY);
    	packet_receive_4G(&lte.byte_receive);
    }
}



void Serial_LTE_Init(void){
    cy_en_scb_uart_status_t init_status;

    /* Start UART operation */
    init_status = Cy_SCB_UART_Init(SERIAL_4G_HW, &SERIAL_4G_config, &lte.uartContext);
    if (init_status!=CY_SCB_UART_SUCCESS)
    {
        __disable_irq();

        CY_ASSERT(0);
    }

    cy_stc_sysint_t uartIntrConfig =
    {
        .intrSrc      = UART_4G_INTR_NUM,
        .intrPriority = UART_4G_INTR_PRIORITY,
    };

    /* Hook interrupt service routine and enable interrupt */
    (void) Cy_SysInt_Init(&uartIntrConfig, &UART_4G_Isr);
    NVIC_EnableIRQ(UART_4G_INTR_NUM);

    Cy_SCB_UART_Enable(SERIAL_4G_HW);

    /* Start receive operation (do not check status) */
    (void) Cy_SCB_UART_Receive(SERIAL_4G_HW, &lte.byte_receive, 1, &lte.uartContext);
}

static void packet_receive_4G(uint8_t *byte_receive){
	uint8_t lenght_connectivity;
	uint8_t id;

	switch(lte.state){
	case HeaderLTE:
		if(*byte_receive == 0xAA){
			lte.count_header++;
			if(lte.count_header == LENGHT_HEADER){
				lte.count_header = 0;
				lte.state = LEN_LTE;
			}
		}
		(void) Cy_SCB_UART_Receive(SERIAL_4G_HW, &lte.byte_receive, 1, &lte.uartContext);

		break;
	case LEN_LTE:
		lenght_connectivity = *byte_receive;
		lte.data[0] = lenght_connectivity;
		lte.state = ID_LTE;
		lte.lenght_data_cryp =
			    ((lenght_connectivity + 5) <= 16) ? 16 :
			    ((lenght_connectivity + 5) <= 32) ? 32 :
			    ((lenght_connectivity + 5) <= 48) ? 48 :
			    ((lenght_connectivity + 5) <= 64) ? 64 :
			    ((lenght_connectivity + 5) <= 80) ? 80 :
			    ((lenght_connectivity + 5) <= 96) ? 96 :
			    ((lenght_connectivity + 5) <= 112) ? 112 :
			    ((lenght_connectivity + 5) <= 128) ? 128 :
			    ((lenght_connectivity + 5) <= 144) ? 144 :
			    ((lenght_connectivity + 5) <= 160) ? 160 :
			    ((lenght_connectivity + 5) <= 176) ? 176 :
			    ((lenght_connectivity + 5) <= 192) ? 192 :
			    ((lenght_connectivity + 5) <= 208) ? 208 :
			    ((lenght_connectivity + 5) <= 224) ? 224 :
			    ((lenght_connectivity + 5) <= 240) ? 240 :
			    ((lenght_connectivity + 5) <= 256) ? 256 :
			    ((lenght_connectivity + 5) <= 272) ? 272 : 0;

		//(void) Cy_SCB_UART_Receive(SERIAL_CONNECTIVITY_HW, &connectivity.data[1], (connectivity.lenght_data_cryp + LENGHT_IV + LENGHT_HEADER + 1), &uartContext);
		(void) Cy_SCB_UART_Receive(SERIAL_4G_HW, &lte.byte_receive, 1, &lte.uartContext);

		break;

	case ID_LTE:
		id = *byte_receive;
		lte.data[1] = id;
		lte.state = DADO_LTE;

		if(id == 1){
			(void) Cy_SCB_UART_Receive(SERIAL_4G_HW, &lte.data[2], (lte.lenght_data_cryp + LENGHT_IV + LENGHT_IV + LENGHT_HEADER), &lte.uartContext);
		}
		else if(id == 2){
			(void) Cy_SCB_UART_Receive(SERIAL_4G_HW, &lte.data[2], ((lte.data[0]+2) + LENGHT_HEADER), &lte.uartContext);
		}
		else{
			(void) Cy_SCB_UART_Receive(SERIAL_4G_HW, &lte.data[2], (lte.lenght_data_cryp + LENGHT_IV + LENGHT_HEADER), &lte.uartContext);
		}


		break;

	case DADO_LTE:
		lte.state = HeaderLTE;
		(void) Cy_SCB_UART_Receive(SERIAL_4G_HW, &lte.byte_receive, 1, &lte.uartContext);
		CyDelay(10);
		if(lte.data[1] == 1){
			callback_connectivity_receive(lte.data,lte.lenght_data_cryp,1);
		}
		else{
			packet_data_demount_LORA(lte.data, (lte.data[0]+2),&app.app_connectivity.packet);
			alarm_setAlarm_os(&app.app_event, APP_EVENT_LTE, 10, ALARM_MODE_ONESHOT);
		}

		lte.state = HeaderLTE;
		break;

	default:
	}
}

void UART_4G_Isr(void)
{
	Cy_SCB_UART_Interrupt(SERIAL_4G_HW, &lte.uartContext);
	cy_rtos_thread_set_notification(&lte.thread_lte);

}

void packet_transmit_4G(uint8_t *buffer, uint8_t len){
	(void) Cy_SCB_UART_Transmit(SERIAL_4G_HW, buffer, len, &lte.uartContext);
}

void PowerOnA7672(void){
	Cy_GPIO_Write(SIM_PWRKEY_PORT, SIM_PWRKEY_PIN, 1);
	Cy_SysLib_Delay(250);
	Cy_GPIO_Write(SIM_PWRKEY_PORT, SIM_PWRKEY_PIN, 0);
}

void PowerOffA7672(void){
	Cy_GPIO_Write(SIM_PWRKEY_PORT, SIM_PWRKEY_PIN, 1);
	Cy_SysLib_Delay(3000);
	Cy_GPIO_Write(SIM_PWRKEY_PORT, SIM_PWRKEY_PIN, 0);
}

void ResetA7672(void){
	Cy_GPIO_Write(SIM_RESET_PORT, SIM_RESET_PIN, 1);
	Cy_SysLib_Delay(500);
	Cy_GPIO_Write(SIM_RESET_PORT, SIM_RESET_PIN, 0);
}




Status_Mqtt_Connect_t Get_MQTTStatus_Connect(void){
	return lte.Status_Mqtt_Connect;
}
