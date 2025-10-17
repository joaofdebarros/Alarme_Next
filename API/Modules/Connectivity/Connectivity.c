/*
 * Connectivity.c
 *
 *  Created on: 24 de fev. de 2025
 *      Author: diego.marinho
 */

/* ======================== INCLUDES ======================== */
#include "Connectivity.h"


/* ======================== DEFINES INTERNOS ======================== */
/* Assign CONNECTIVITY_TASK */
#define CONNECTIVITY_TASK_NAME            ("Connectivity")
#define CONNECTIVITY_TASK_STACK_SIZE      (configMINIMAL_STACK_SIZE)
#define CONNECTIVITY_TASK_PRIORITY        (tskIDLE_PRIORITY + 1)

/* Assign UART interrupt number and priority */
#define UART_CONNECTIVIDADE_INTR_NUM        ((IRQn_Type) scb_5_interrupt_IRQn)
#define UART_CONNECTIVIDADE_INTR_PRIORITY   (7U)

/* ======================== VARIÁVEIS GLOBAIS ======================== */
connectivity_t connectivity;

/* ========================= Varialvel usart ========================== */
cy_stc_scb_uart_context_t uartContext;

/* ======================== VARIÁVEIS ESTÁTICAS ======================== */

/* ======================== PROTÓTIPOS DE FUNÇÕES ESTÁTICAS ======================== */
static void Serial_Connectivity_Init(void);
static void packet_receive_connectivity(uint8_t *byte_receive);
void UART_Isr(void);

/* ======================== IMPLEMENTAÇÃO DAS FUNÇÕES ======================== */

void connectivity_init(uint32_t eventIndex){
	connectivity.event.index = eventIndex;
	Serial_Connectivity_Init();
	connectivity.status_rtos_connectivity = cy_rtos_thread_create(&connectivity.thread_connectivity, connectivity_task,CONNECTIVITY_TASK_NAME,NULL,CY_RTOS_MIN_STACK_SIZE,CONNECTIVITY_TASK_PRIORITY,NULL);
}


/*******************************************************************************
 * Function Name: connectivity_task
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
void connectivity_task(void *pvParameters)
{

    (void) pvParameters;
    for(;;)
    {
    	cy_rtos_thread_wait_notification(portMAX_DELAY);
    	packet_receive_connectivity(&connectivity.byte_receive);
    }
}



/* ======================== IMPLEMENTAÇÃO DAS FUNÇÕES INTERNAS ======================== */


/* =============== init serial usart ====================== */
static void Serial_Connectivity_Init(void){
    cy_en_scb_uart_status_t init_status;

    /* Start UART operation */
    init_status = Cy_SCB_UART_Init(SERIAL_CONNECTIVITY_HW, &SERIAL_CONNECTIVITY_config, &uartContext);
    if (init_status!=CY_SCB_UART_SUCCESS)
    {
        __disable_irq();

        CY_ASSERT(0);
    }

    cy_stc_sysint_t uartIntrConfig =
    {
        .intrSrc      = UART_CONNECTIVIDADE_INTR_NUM,
        .intrPriority = UART_CONNECTIVIDADE_INTR_PRIORITY,
    };

    /* Hook interrupt service routine and enable interrupt */
    (void) Cy_SysInt_Init(&uartIntrConfig, &UART_Isr);
    NVIC_EnableIRQ(UART_CONNECTIVIDADE_INTR_NUM);

    Cy_SCB_UART_Enable(SERIAL_CONNECTIVITY_HW);

    /* Start receive operation (do not check status) */
    (void) Cy_SCB_UART_Receive(SERIAL_CONNECTIVITY_HW, &connectivity.byte_receive, 1, &uartContext);
}


static void packet_receive_connectivity(uint8_t *byte_receive){
	uint8_t lenght_connectivity;
	uint8_t id;

	switch(connectivity.state){
	case Header:
		if(*byte_receive == 0xAA){
			connectivity.count_header++;
			if(connectivity.count_header == LENGHT_HEADER){
				connectivity.count_header = 0;
				connectivity.state = LEN;
			}
		}
		(void) Cy_SCB_UART_Receive(SERIAL_CONNECTIVITY_HW, &connectivity.byte_receive, 1, &uartContext);

		break;
	case LEN:
		lenght_connectivity = *byte_receive;
		connectivity.data[0] = lenght_connectivity;
		connectivity.state = ID;
		connectivity.lenght_data_cryp =
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
		(void) Cy_SCB_UART_Receive(SERIAL_CONNECTIVITY_HW, &connectivity.byte_receive, 1, &uartContext);

		break;

	case ID:
		id = *byte_receive;
		connectivity.data[1] = id;
		connectivity.state = DADO;

		if(id == 1){
			(void) Cy_SCB_UART_Receive(SERIAL_CONNECTIVITY_HW, &connectivity.data[2], (connectivity.lenght_data_cryp + LENGHT_IV + LENGHT_IV + LENGHT_HEADER), &uartContext);
		}
		else{
			(void) Cy_SCB_UART_Receive(SERIAL_CONNECTIVITY_HW, &connectivity.data[2], (connectivity.lenght_data_cryp + LENGHT_IV + LENGHT_HEADER), &uartContext);
		}


		break;

	case DADO:
		connectivity.state = Header;
		(void) Cy_SCB_UART_Receive(SERIAL_CONNECTIVITY_HW, &connectivity.byte_receive, 1, &uartContext);
		CyDelay(10);
		callback_connectivity_receive(connectivity.data,connectivity.lenght_data_cryp,0);
		connectivity.state = Header;
		break;

	default:
	}
}


// UART interrupt handler
void UART_Isr(void)
{
	Cy_SCB_UART_Interrupt(SERIAL_CONNECTIVITY_HW, &uartContext);
	cy_rtos_thread_set_notification(&connectivity.thread_connectivity);

}

void packet_transmit_connectivity(uint8_t *buffer, uint8_t len){
	(void) Cy_SCB_UART_Transmit(SERIAL_CONNECTIVITY_HW, buffer, len, &uartContext);
}



