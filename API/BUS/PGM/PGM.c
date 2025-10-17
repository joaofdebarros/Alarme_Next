/*
 * PGM.c
 *
 *  Created on: 29 de jul. de 2025
 *      Author: joao.victor
 */

#include "PGM.h"
#include <stdint.h>
#include <string.h>

pgm_t pgm;
pgm_cmd_t pgm_received_cmd;
pgm_cmd_t received_cmd;

bool received_ack;

extern bus_t bus;


#define PGM_PACKET_HEADER_LEN 1
#define PGM_PACKET_LENGTH_LEN 1
#define PGM_PACKET_ID_LEN 1
#define PGM_PACKET_ADDRESS_LEN 1
#define PGM_PACKET_FUNCTION_LEN 1
#define PGM_PACKET_TAIL_LEN 1
#define PGM_PACKET_CHECKSUM_LEN 1

/* ======================== DEFINES INTERNOS ======================== */
/* Assign CONNECTIVITY_TASK */
#define PGM_TASK_NAME ("PGM")
#define PGM_TASK_STACK_SIZE (configMINIMAL_STACK_SIZE)
#define PGM_TASK_PRIORITY (tskIDLE_PRIORITY + 3)

/* Assign UART interrupt number and priority */
#define UART_PGM_INTR_NUM ((IRQn_Type)scb_3_interrupt_IRQn)
#define UART_PGM_INTR_PRIORITY (7U)

/* ========================= Varialvel usart ========================== */
cy_stc_scb_uart_context_t uart_pgm_Context;

/* ======================== PROTÓTIPOS DE FUNÇÕES ESTÁTICAS
 * ======================== */

uint8_t tx_pgm_buffer[12];
uint8_t data_pgm_toggle[] = {0x1F};
uint8_t DATA_NULL = 0;
volatile bool transmiting_pgm_toggle = 0;

void pgm_init(hEvents_t eventHandle, uint32_t eventIndex) {
  const char *msg = "Hello RTOS!";
  pgm.event.handle = eventHandle;
  pgm.event.index = eventIndex;
  
  pgm.status_rtos_pgm =
      cy_rtos_thread_create(&pgm.thread_pgm, pgm_task, PGM_TASK_NAME, NULL,
                            CY_RTOS_MIN_STACK_SIZE, PGM_TASK_PRIORITY, NULL);
  cy_rtos_timer_init(&pgm.pgm_timer_handle, CY_TIMER_TYPE_PERIODIC,
                     pgm_timer_callback, (cy_timer_callback_arg_t)msg);
  cy_rtos_semaphore_init(&pgm.pgm_semaphore, 1, 0);


}

/* ======================== TASK PGM ======================== */

void pgm_task(void *pvParameters) {
  (void)pvParameters;
  for (;;) {
    cy_rtos_thread_wait_notification(portMAX_DELAY);
    pgm_start(&received_cmd);
  }
}

/* ======================== INICIALIZAÇÃO ======================== */



/* ======================== FUNÇÕES PRINCIPAIS DO PGM ========================
 */

void get_pgm_info(pgm_registrado_t *packet, uint8_t *number_of_pgm){

	memcpy(packet, pgm.pgm_registrado, 5 * sizeof(pgm_registrado_t));
	memcpy(number_of_pgm, &pgm.pgm_count, sizeof(pgm.pgm_count));
	
}

void pgm_start(pgm_cmd_t *pgm_cmd) {
  static uint8_t attempts = 0;
  received_cmd = *pgm_cmd;
  switch (pgm_cmd->function) {
  case PGM_REGISTER:
    if (attempts < 10) {
      cy_rtos_timer_start(&pgm.pgm_timer_handle, 1500);
      pgm_register(&pgm.pgm_count);
      if (pgm.pgm_count == 4) {
        attempts = 10;
      } else {
        attempts++;
      }

    } else {
      for (uint8_t i = 0; i < pgm.pgm_count; i++) {
        pgm_get_status(pgm.pgm_registrado[i].crc, &i);
        vTaskDelay(100);
      }
      attempts = 0;
      received_ack = false;
      cy_rtos_timer_stop(&pgm.pgm_timer_handle);
      alarm_setAlarm_os(pgm.event.handle,pgm.event.index , 10,
            ALARM_MODE_ONESHOT);
    }
    break;
  case PGM_TOGGLE:
    if (!received_ack && attempts < 5) {
      cy_rtos_timer_start(&pgm.pgm_timer_handle, 100);
      pgm_toggle(pgm_cmd->function, pgm_cmd->address, pgm_cmd->rele_number, pgm_cmd->timebase.time, pgm_cmd->timebase.state);
      attempts++;
    } else {
      received_ack = false;
      attempts = 0;
      cy_rtos_timer_stop(&pgm.pgm_timer_handle);
    }
    break;
  case PGM_PULSED:
    if (!received_ack && attempts < 5) {
      cy_rtos_timer_start(&pgm.pgm_timer_handle, 100);
      pgm_toggle(pgm_cmd->function, pgm_cmd->address, pgm_cmd->rele_number, pgm_cmd->timebase.time, pgm_cmd->timebase.state);
      attempts++;
    } else {
      received_ack = false;
      attempts = 0;
      cy_rtos_timer_stop(&pgm.pgm_timer_handle);
    }
    break;
  case PGM_RETENTION:
    if (!received_ack && attempts < 5) {
      cy_rtos_timer_start(&pgm.pgm_timer_handle, 100);
      pgm_toggle(pgm_cmd->function, pgm_cmd->address, pgm_cmd->rele_number, pgm_cmd->timebase.time, pgm_cmd->timebase.state);
      attempts++;
    } else {
      received_ack = false;
      attempts = 0;
      cy_rtos_timer_stop(&pgm.pgm_timer_handle);
    }
    break;
  case PGM_STATUS:
    if (!received_ack && attempts < 5) {
      cy_rtos_timer_start(&pgm.pgm_timer_handle, 100);
      pgm_get_status(pgm_cmd->address, &pgm_cmd->rele_number);
      attempts++;
    } else {
      received_ack = false;
      attempts = 0;
      cy_rtos_timer_stop(&pgm.pgm_timer_handle);
    }
    break;
  case PGM_DELETE:

    //        alarm_setAlarm_os(&app.app_event, APP_EVENT_TECLADO, 200,
    //        ALARM_MODE_CONTINUOUS);

    for (int i = 0; i < 10; i++) {
      pgm_delete(pgm_cmd->ID, pgm_cmd->address);
      pgm.pgm_registrado[i].uid0 = 0;
      pgm.pgm_registrado[i].uid1 = 0;
      pgm.pgm_registrado[i].uid2 = 0;
      pgm.pgm_registrado[i].uid3 = 0;
      pgm.pgm_registrado[i].crc = 0;
      pgm.pgm_registrado[i].num = 0;
      pgm.pgm_count = 0;
    }

    //        alarm_cancelAlarm(APP_EVENT_TECLADO);

    break;
  case PGM_RETRY_CRC:

    break;
  }
}

void registrar_pgm(uint8_t uid0, uint8_t uid1, uint8_t uid2, uint8_t uid3,
                   uint8_t crc) {
  bool cadastrar = false;
  bool recalcular_crc = false;
  for (int i = 0; i < 5; i++) {
    if (crc != 0) {
      cadastrar = true;

      if (crc == pgm.pgm_registrado[i].crc) {
        cadastrar = false;
        if (uid0 != pgm.pgm_registrado[i].uid0 ||
            uid1 != pgm.pgm_registrado[i].uid1 ||
            uid2 != pgm.pgm_registrado[i].uid2 ||
            uid3 != pgm.pgm_registrado[i].uid3) {
          for (int i = 0; i < 3; i++) {
            pgm_retry_crc(PGM_ID, uid0, uid1, uid2, uid3, crc);
          }
        }
        break;
      }
    }
  }

  if (cadastrar) {
    pgm.pgm_registrado[pgm.pgm_count].uid0 = uid0;
    pgm.pgm_registrado[pgm.pgm_count].uid1 = uid1;
    pgm.pgm_registrado[pgm.pgm_count].uid2 = uid2;
    pgm.pgm_registrado[pgm.pgm_count].uid3 = uid3;
    pgm.pgm_registrado[pgm.pgm_count].crc = crc;
    pgm.pgm_registrado[pgm.pgm_count].num = pgm.pgm_count;
	vTaskDelay(5);
	for(int i = 0; i < 2; i++){
	cy_rtos_delay_milliseconds(100);
		pgm_get_status(pgm.pgm_registrado[pgm.pgm_count].crc, &pgm.pgm_count);
	}
    

    pgm.pgm_count++;
  }
}

void pgm_register(uint8_t *module_count) {
  uint8_t tamanho = packet_build_pgm(tx_pgm_buffer, 8, PGM_ID, 0, PGM_REGISTER,
                                     module_count, 1);

  packet_transmit_bus(tx_pgm_buffer, tamanho);
}


void pgm_toggle(uint8_t function, uint8_t address_byte, uint8_t rele_index, uint16_t time, uint8_t state) {

  uint8_t MSB = (time >> 8) & 0xFF;
  uint8_t LSB = time & 0xFF;
  uint8_t data[4] = {rele_index, state, MSB, LSB};
  uint8_t tamanho = packet_build_pgm(tx_pgm_buffer, 11, PGM_ID, address_byte,
                                     function, data, 4);

  packet_transmit_bus(tx_pgm_buffer, tamanho);
}

void pgm_get_status(uint8_t address_byte, uint8_t *pgm_number) {

  uint8_t tamanho = packet_build_pgm(tx_pgm_buffer, 8, PGM_ID, address_byte,
                                     PGM_STATUS, pgm_number, 1);
  packet_transmit_bus(tx_pgm_buffer, tamanho);
}

void pgm_delete(uint8_t IDT, uint8_t address_byte) {

  uint8_t tamanho = packet_build_pgm(tx_pgm_buffer, 8, IDT, address_byte,
                                     PGM_DELETE, &DATA_NULL, 1);
  packet_transmit_bus(tx_pgm_buffer, tamanho);
}

void pgm_retry_crc(uint8_t IDT, uint8_t uid0, uint8_t uid1, uint8_t uid2,
                   uint8_t uid3, uint8_t crc) {
  uint8_t data[5];
  data[0] = uid0;
  data[1] = uid1;
  data[2] = uid2;
  data[3] = uid3;
  data[4] = 0x15;
  ;
  uint8_t tamanho =
      packet_build_pgm(tx_pgm_buffer, 12, IDT, crc, PGM_RETRY_CRC, &data, 5);
  packet_transmit_bus(tx_pgm_buffer, tamanho);
}

/* ======================== FUNÇÕES PARA RECONFIGURAÇÃO DOS PINOS
 * ======================== */


/* ======================== FUNÇÕES RELACIONADAS AO PACOTE
 * ======================== */

uint8_t packet_build_pgm(uint8_t *tx, uint8_t size, uint8_t id, uint8_t adrs,
                         uint8_t fnct, uint8_t *data, uint8_t payload_size) {
  uint8_t total_size = PGM_HEADER + payload_size + PGM_TAIL;

  tx[0] = START_BYTE;
  tx[1] = size;
  tx[2] = id;
  tx[3] = adrs;
  tx[4] = fnct;

  for (int i = 0; i < payload_size; i++) {
    tx[5 + i] = data[i];
  }

  tx[5 + payload_size] = calculate_checksum(tx, payload_size);
  tx[6 + payload_size] = STOP_BYTE;

  return total_size;
}


pgm_packet_error_e pgm_packet_demount(uint8_t *datain, uint16_t len,
                                      pgm_packet_t *packet) {
  uint8_t checksum_validate;
  uint16_t i, size, lHold;

  if (datain == NULL || packet == NULL) {
    return PGM_PACKET_FAIL_UNKNOWN;
  }

  checksum_validate = 0x7E;
  // Transport all bytes to the struct
  size = 0;
  memset(packet, 0, sizeof(pgm_packet_t));

  for (i = 0; i < PGM_PACKET_LENGTH_LEN; i++) {
    packet->len += (datain[size++]);
  }
  lHold = (len - 7);
  checksum_validate ^= packet->len;

  for (i = 0; i < PGM_PACKET_ID_LEN; i++) {
    packet->id += (datain[size++]);
  }
  checksum_validate ^= packet->id;

  for (i = 0; i < PGM_PACKET_ADDRESS_LEN; i++) {
    packet->address = datain[size++];
  }
  checksum_validate ^= packet->address;

  for (i = 0; i < PGM_PACKET_FUNCTION_LEN; i++) {
    packet->function = datain[size++];
  }
  checksum_validate ^= packet->function;

  memcpy(packet->data, &datain[size], lHold);
  size += lHold;

  for (i = 0; i < lHold; i++) {
    checksum_validate ^= packet->data[i];
  }

  for (i = 0; i < PGM_PACKET_CHECKSUM_LEN; i++) {
    packet->checksum += (datain[size++]);
  }
  checksum_validate = ~checksum_validate;

  for (i = 0; i < PGM_PACKET_TAIL_LEN; i++) {
    packet->tail += (datain[size++]);
  }

  if (packet->tail != 0x81) {
    return PGM_PACKET_FAIL_TAIL;
  }

  if (checksum_validate != packet->checksum) {
    return PGM_PACKET_FAIL_CHECKSUM;
  }

  return PGM_PACKET_OK;
}


void pgm_timer_callback(cy_timer_callback_arg_t arg) {
  cy_rtos_thread_set_notification(&pgm.thread_pgm);
}
