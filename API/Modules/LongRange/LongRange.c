/*
 * LongRange.c
 *
 *  Created on: 3 de abr. de 2025
 *      Author: diego.marinho
 */

#include "LongRange.h"

long_range_t long_range;

#define LR_TASK_NAME            ("LongRange")
#define LR_TASK_STACK_SIZE      configMINIMAL_STACK_SIZE
#define LR_TASK_PRIORITY        (tskIDLE_PRIORITY + 1)

#define UART_LONGRANGE_INTR_NUM        ((IRQn_Type) scb_2_interrupt_IRQn)
#define UART_LONGRANGE_INTR_PRIORITY   (7U)

static void LR_Connectivity_Init(void);
void UART_LR_Isr(void);
static void packet_receive_longRange(uint8_t *byte_receive);

void LongRange_init(hEvents_t eventHandle, uint32_t eventIndex){
	cy_rslt_t test_status;
	long_range.event.handle = eventHandle;
	long_range.event.index = eventIndex;

	LR_Connectivity_Init();

	test_status = cy_rtos_thread_create(&long_range.thread_LR, longRange_task,LR_TASK_NAME,NULL,CY_RTOS_MIN_STACK_SIZE,LR_TASK_PRIORITY,NULL);
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
void longRange_task(void *pvParameters)
{

    (void) pvParameters;
    for(;;)
    {
    	cy_rtos_thread_wait_notification(portMAX_DELAY);
    	packet_receive_longRange(&long_range.byte_receive);
    }
}

static void LR_Connectivity_Init(void){
    cy_en_scb_uart_status_t init_status;

    /* Start UART operation */
    init_status = Cy_SCB_UART_Init(SERIAL_LR_HW, &SERIAL_LR_config, &long_range.uartContext);
    if (init_status!=CY_SCB_UART_SUCCESS)
    {
        __disable_irq();

        CY_ASSERT(0);
    }

    cy_stc_sysint_t uartIntrConfig =
    {
        .intrSrc      = UART_LONGRANGE_INTR_NUM,
        .intrPriority = UART_LONGRANGE_INTR_PRIORITY,
    };

    /* Hook interrupt service routine and enable interrupt */
    (void) Cy_SysInt_Init(&uartIntrConfig, &UART_LR_Isr);
    NVIC_EnableIRQ(UART_LONGRANGE_INTR_NUM);

    Cy_SCB_UART_Enable(SERIAL_LR_HW);

    /* Start receive operation (do not check status) */
    (void) Cy_SCB_UART_Receive(SERIAL_LR_HW, &long_range.byte_receive, 1, &long_range.uartContext);
}

static void packet_receive_longRange(uint8_t *byte_receive){
	uint8_t lenght_connectivity;

	switch(long_range.state){
	case HeaderLR:
		if(*byte_receive == 0xAA){
			long_range.count_header++;
			if(long_range.count_header == LENGHT_HEADER){
				long_range.count_header = 0;
				long_range.state = LENLR;
			}
		}
		(void) Cy_SCB_UART_Receive(SERIAL_LR_HW, &long_range.byte_receive, 1, &long_range.uartContext);

		break;
	case LENLR:
		lenght_connectivity = *byte_receive;
		long_range.data[0] = lenght_connectivity;
		long_range.state = DADOLR;
		long_range.lenght_data_cryp = lenght_connectivity + 2;
//		long_range.lenght_data_cryp =
//		    ((lenght_connectivity + 5) <= 16) ? 16 :
//		    ((lenght_connectivity + 5) <= 32) ? 32 :
//		    ((lenght_connectivity + 5) <= 48) ? 48 :
//		    ((lenght_connectivity + 5) <= 64) ? 64 :
//		    ((lenght_connectivity + 5) <= 80) ? 80 :
//		    ((lenght_connectivity + 5) <= 96) ? 96 :
//		    ((lenght_connectivity + 5) <= 112) ? 112 :
//		    ((lenght_connectivity + 5) <= 128) ? 128 :
//		    ((lenght_connectivity + 5) <= 144) ? 144 : 0;

		(void) Cy_SCB_UART_Receive(SERIAL_LR_HW, &long_range.data[1], (long_range.lenght_data_cryp + LENGHT_HEADER + 1), &long_range.uartContext);

		break;
	case DADOLR:
		long_range.state = HeaderLR;
		packet_receive_longRange(&long_range.byte_receive);
		CyDelay(10);
		callback_longRange_receive(long_range.data,long_range.lenght_data_cryp);
		long_range.state = HeaderLR;
		break;


	default:
	}
	//(void) Cy_SCB_UART_Receive(SERIAL_LR_HW, &long_range.byte_receive, 1, &long_range.uartContext);
}

void packet_transmit_longRange(uint8_t *byte_transmit,uint8_t len){
	Cy_SCB_UART_Transmit(SERIAL_LR_HW, (uint8_t*)byte_transmit, len, &long_range.uartContext);
}

void UART_LR_Isr(void){
	Cy_SCB_UART_Interrupt(SERIAL_LR_HW, &long_range.uartContext);
	cy_rtos_thread_set_notification(&long_range.thread_LR);
}
