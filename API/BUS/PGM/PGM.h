/*
 * PGM.h
 *
 *  Created on: 29 de jul. de 2025
 *      Author: joao.victor
 */

#ifndef API_PGM_PGM_H_
#define API_PGM_PGM_H_

#include "alarm.h"
#include "cyabs_rtos.h"
#include "timestamp.h"
#include <stdbool.h> // Tipo booleano
#include <stdint.h>  // Tipos padrão
#include <string.h>
#include "commum_bus.h"

#define PGM_HEADER 5
#define PGM_TAIL 2
#define START_BYTE 0x7E
#define STOP_BYTE 0x81
#define SCB0_UART_RX_CTRL_ADDR (0x42820048)
#define SCB0_UART_RX_CTRL (*(volatile uint32_t *)0x42820048UL)

typedef union {
  uint8_t Byte;
  struct {
    bool rele_1 : 1;
    bool rele_2 : 1;
    bool rele_3 : 1;
    bool rele_4 : 1;
    bool rele_5 : 1;
    bool bitflag5 : 1;
    bool bitflag6 : 1;
    bool bitflag7 : 1;
  } Bits;
} PGM_flagbits_t;


typedef enum {
  PGM_ID = 1,
  PGM_BROADCAST_ID,
} ID_PGM_t;

typedef enum {
  PGM_REGISTER = 0x00,
  PGM_TOGGLE,
  PGM_DELAYED_TOGGLE,
  PGM_PULSED,
  PGM_RETENTION,
  PGM_STATUS,
  PGM_DELETE,
  PGM_RETRY_CRC,
  PGM_GATE_STATUS,
  PGM_GATE_CMD,
} function_PGM_t;

typedef struct{
  uint8_t state;
  uint16_t time;
} pgm_timebase_t;

// Estrutura que deve ser manipulada para
typedef struct {
  uint8_t ID;
  uint8_t address;
  uint8_t function;
  uint8_t rele_number;
  pgm_timebase_t timebase;
//  PGM_flagbits_t estado_relays;
  //	bool ack_register_received;
  //	bool ack_toggle_received;
} pgm_cmd_t;

typedef enum {
  PGM_PACKET_OK,
  PGM_PACKET_FAIL_HEADER,
  PGM_PACKET_FAIL_TAIL,
  PGM_PACKET_FAIL_LENGHT,
  PGM_PACKET_FAIL_CHECKSUM,
  PGM_PACKET_FAIL_UNKNOWN = 0xFF
} pgm_packet_error_e;

typedef struct {
  uint8_t len;
  uint8_t id;
  uint8_t address;
  uint8_t function;
  uint8_t data[5];
  uint8_t checksum;
  uint8_t tail;
} pgm_packet_t;

typedef struct {
  uint8_t uid0;
  uint8_t uid1;
  uint8_t uid2;
  uint8_t uid3;
  uint8_t crc;
  uint8_t num;
} pgm_info_t;

typedef struct {
  uint8_t uid0;
  uint8_t uid1;
  uint8_t uid2;
  uint8_t uid3;
  uint8_t crc;
  uint8_t num;
} pgm_registrado_t;

typedef struct {
  struct {
    hEvents_t waitInt;
    hEvents_t handle;
    uint32_t index;
  } event;

  cy_rslt_t status_rtos_pgm;
  cy_thread_t thread_pgm;
  cy_thread_t thread_pgm_receive;
  cy_semaphore_t pgm_semaphore;
  cy_timer_t pgm_timer_handle;

  PGM_flagbits_t status_relays;
  pgm_packet_t pgm_packet;
  pgm_packet_error_e packError_pgm;
  uint8_t data[12];
  pgm_registrado_t pgm_registrado[5];
  uint8_t pgm_count;

  //state_receive_t state;
  uint8_t byte_receive;
  uint8_t count_header;
  uint8_t lenght_data;
} pgm_t;

extern pgm_t pgm;

void pgm_init(hEvents_t eventHandle, uint32_t eventIndex);
void pgm_task(void *pvParameters);
void pgm_task_receive(void *pvParameters);
void get_pgm_info(pgm_registrado_t *packet, uint8_t *number_of_pgm);
void pgm_start(pgm_cmd_t *pgm_cmd);
void registrar_pgm(uint8_t uid0, uint8_t uid1, uint8_t uid2, uint8_t uid3,
                   uint8_t crc);
void pgm_register(uint8_t *module_count);
void pgm_toggle(uint8_t function, uint8_t address_byte, uint8_t rele_index, uint16_t time, uint8_t state);
void pgm_get_status(uint8_t address_byte, uint8_t *pgm_number);
void pgm_delete(uint8_t ID, uint8_t address_byte);
void pgm_retry_crc(uint8_t IDT, uint8_t uid0, uint8_t uid1, uint8_t uid2,
                   uint8_t uid3, uint8_t crc);
uint8_t packet_build_pgm(uint8_t *tx, uint8_t size, uint8_t id, uint8_t adrs,
                         uint8_t fnct, uint8_t *data, uint8_t payload_size);
pgm_packet_error_e pgm_packet_demount(uint8_t *datain, uint16_t len,
                                      pgm_packet_t *packet);
void pgm_timer_callback(cy_timer_callback_arg_t arg);


#endif /* API_PGM_PGM_H_ */
