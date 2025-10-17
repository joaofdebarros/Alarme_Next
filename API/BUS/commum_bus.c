/*
 * commum_bus.c
 *
 *  Created on: 4 de set. de 2025
 *      Author: diego.marinho
 */



#include "commum_bus.h"

#include "callback.h"

bus_t bus;
extern bool received_ack;

#define BUS_TASK_NAME            ("Bus")
#define BUS_TASK_STACK_SIZE      512
#define BUS_TASK_PRIORITY        (tskIDLE_PRIORITY + 3)

#define UART_BUS_INTR_NUM ((IRQn_Type)scb_3_interrupt_IRQn)
#define UART_BUS_INTR_PRIORITY (7U)

static void Serial_BUS_Init(void);
static void packet_receive_bus(uint8_t *byte_receive);
void UART_BUS_Isr(void);

void commum_bus_init(void){
	//bus.status_rtos_bus = cy_rtos_thread_create(&bus.thread_bus, BUS_task,BUS_TASK_NAME,NULL,BUS_TASK_STACK_SIZE,BUS_TASK_PRIORITY,NULL);
	bus.status_rtos_bus = cy_rtos_thread_create(&bus.thread_bus, BUS_task, BUS_TASK_NAME, NULL,
			BUS_TASK_STACK_SIZE, BUS_TASK_PRIORITY, NULL);
	Serial_BUS_Init();
}

void BUS_task(void *pvParameters)
{

    (void) pvParameters;
    for(;;)
    {
    	cy_rtos_thread_wait_notification(portMAX_DELAY);
    	packet_receive_bus(&bus.byte_receive);
    }
}

static void packet_receive_bus(uint8_t *byte_receive){
	  uint8_t lenght_pgm;
	  uint8_t id;
	  uint8_t address;

	  switch (bus.state) {
	  case START_ST:
		  if (*byte_receive == 0x7E) {

			  bus.state = SIZE_ST;
		  }
		  (void)Cy_SCB_UART_Receive(SERIAL_PGM_HW, &bus.byte_receive, 1,&bus.uartContext);
		  break;
	  case SIZE_ST:
		  lenght_pgm = *byte_receive;
		  bus.data[0] = lenght_pgm;
		  bus.state = IDT_ST;
		  (void)Cy_SCB_UART_Receive(SERIAL_PGM_HW, &bus.byte_receive, 1,&bus.uartContext);

		  break;
	  case IDT_ST:
		  id = *byte_receive;
		  if (id == 0x03) {
			  bus.data[1] = id;
			  bus.state = ADDRS_ST;
			  bus.type_node = PGM_NODE;
		  }
		  else if(id == 0x06){
			  bus.data[1] = id;
			  bus.state = ADDRS_ST;
			  bus.type_node = TECLADO_NODE;

			  bus.lenght_data_cryp =
					    ((bus.data[0] + 2) <= 16) ? 16 :
					    ((bus.data[0] + 2) <= 32) ? 32 :
					    ((bus.data[0] + 2) <= 48) ? 48 :
					    ((bus.data[0] + 2) <= 64) ? 64 :
					    ((bus.data[0] + 2) <= 80) ? 80 :
					    ((bus.data[0] + 2) <= 96) ? 96 :
					    ((bus.data[0] + 2) <= 112) ? 112 :
					    ((bus.data[0] + 2) <= 128) ? 128 :
					    ((bus.data[0] + 2) <= 144) ? 144 :
					    ((bus.data[0] + 2) <= 160) ? 160 :
					    ((bus.data[0] + 2) <= 176) ? 176 :
					    ((bus.data[0] + 2) <= 192) ? 192 :
					    ((bus.data[0] + 2) <= 208) ? 208 :
					    ((bus.data[0] + 2) <= 224) ? 224 :
					    ((bus.data[0] + 2) <= 240) ? 240 :
					    ((bus.data[0] + 2) <= 256) ? 256 :
					    ((bus.data[0] + 2) <= 272) ? 272 : 0;
		  }
		  else {
			  bus.state = START_ST;
		  }
		  (void)Cy_SCB_UART_Receive(SERIAL_PGM_HW, &bus.byte_receive, 1,&bus.uartContext);

		  break;
	  case ADDRS_ST:
		  address = *byte_receive;
		  bus.data[2] = address;
		  bus.state = DATA_ST;

		  if(bus.type_node == PGM_NODE)
			  (void)Cy_SCB_UART_Receive(SERIAL_PGM_HW, &bus.data[3],(bus.data[0] - (PGM_HEADER - 1)),&bus.uartContext);

		  if(bus.type_node == TECLADO_NODE)
			  (void)Cy_SCB_UART_Receive(SERIAL_PGM_HW, &bus.data[3],(bus.lenght_data_cryp+16+3),&bus.uartContext);

		  break;
	  case DATA_ST:
		  bus.state = START_ST;

		  if(bus.type_node == PGM_NODE){
			  pgm.packError_pgm = pgm_packet_demount(bus.data, bus.data[0], &pgm.pgm_packet);

			  if (pgm.packError_pgm == PGM_PACKET_OK) {
				  switch (pgm.pgm_packet.function) {
				  case PGM_REGISTER:
					  registrar_pgm(pgm.pgm_packet.data[0], pgm.pgm_packet.data[1],pgm.pgm_packet.data[2], pgm.pgm_packet.data[3],pgm.pgm_packet.data[4]);
					  break;
				  case PGM_TOGGLE:
					  received_ack = true;
					  break;
				  case PGM_STATUS:
					  for(int i = 0; i < pgm.pgm_count; i++){
						  if(pgm.pgm_packet.address == pgm.pgm_registrado[i].crc){
							  cy_rtos_delay_milliseconds(100);
							  pgm_get_status(pgm.pgm_registrado[i].crc, &pgm.pgm_registrado[i].num);
						  }
					  }
					  received_ack = true;
					  break;
				  case PGM_DELETE:
					  received_ack = true;
					  break;
				  case PGM_RETRY_CRC:
					  break;
				  }
			  }
			    /* Start receive operation (do not check status) */
			    (void)Cy_SCB_UART_Receive(SERIAL_PGM_HW, &bus.byte_receive, 1,&bus.uartContext);
		  }
		  else if(bus.type_node == TECLADO_NODE){
			  callback_teclado_receive(bus.data,bus.lenght_data_cryp);
			  //packet_data_demount_teclado(bus.data, bus.lenght_data_cryp,&teclado.packet);
			  //packet_decrypt_and_get_teclado(&teclado.packet, teclado.saved_key,&teclado.TecladoPacket);
			  //cy_rtos_thread_set_notification(&teclado.thread_teclado);
			  (void)Cy_SCB_UART_Receive(SERIAL_PGM_HW, &bus.byte_receive, 1,&bus.uartContext);
		  }
		  break;
	  }


}
//


static void Serial_BUS_Init(void) {
  cy_en_scb_uart_status_t init_status;

  /* Start UART operation */
  init_status =
		  Cy_SCB_UART_Init(SERIAL_PGM_HW, &SERIAL_PGM_config, &bus.uartContext);
  if (init_status != CY_SCB_UART_SUCCESS) {
    __disable_irq();

    CY_ASSERT(0);
  }

  cy_stc_sysint_t uartIntrConfig = {
      .intrSrc = UART_BUS_INTR_NUM,
      .intrPriority = UART_BUS_INTR_PRIORITY,
  };

  /* Hook interrupt service routine and enable interrupt */
  (void)Cy_SysInt_Init(&uartIntrConfig, &UART_BUS_Isr);
  NVIC_EnableIRQ(UART_BUS_INTR_NUM);

  Cy_SCB_UART_Enable(SERIAL_PGM_HW);

  /*Inverter o RX da UART: foi feita a alteração no arquivo cycfg_peripherals.c
   * de irdaInvertRx*/
  //    SCB0_UART_RX_CTRL |= (1UL << 6);

  //  SERIAL_PGM_config.irdaInvertRx = true;

  switch_to_gpio();

  /* Start receive operation (do not check status) */
  (void)Cy_SCB_UART_Receive(SERIAL_PGM_HW, &bus.byte_receive, 1,
                            &bus.uartContext);
}

void UART_BUS_Isr(void) {
  Cy_SCB_UART_Interrupt(SERIAL_PGM_HW, &bus.uartContext);
  cy_rtos_thread_set_notification(&bus.thread_bus);
}
//
void switch_to_gpio() {
  Cy_GPIO_SetHSIOM(Bus_TX_PORT, Bus_TX_PIN, P5_3_GPIO);
  Cy_GPIO_SetDrivemode(Bus_TX_PORT, Bus_TX_PIN, CY_GPIO_DM_STRONG_IN_OFF);
  Cy_GPIO_Clr(Bus_TX_PORT, Bus_TX_PIN);
}

void switch_to_uart() {
  Cy_GPIO_SetHSIOM(Bus_TX_PORT, Bus_TX_PIN, P5_3_SCB3_UART_TX);
  Cy_GPIO_SetDrivemode(Bus_TX_PORT, Bus_TX_PIN, CY_GPIO_DM_STRONG_IN_OFF);
}

uint8_t calculate_checksum(uint8_t *buffer, uint8_t payload_size) {
  uint8_t sum = 0;
  for (int i = 0; i < (PGM_HEADER + payload_size); i++) {
    sum ^= buffer[i];
  }

  return ~sum;
}
//
void packet_transmit_bus(uint8_t *buffer, uint8_t len) {
  Cy_GPIO_Clr(Bus_Ctrl_PORT, Bus_Ctrl_PIN);
  switch_to_uart();
  cy_rtos_delay_milliseconds(1);
  (void)Cy_SCB_UART_Transmit(SERIAL_PGM_HW, buffer, len, &bus.uartContext);
  cy_rtos_delay_milliseconds(50);
  switch_to_gpio();
  Cy_GPIO_Write(Bus_Ctrl_PORT, Bus_Ctrl_PIN, 1);
}

