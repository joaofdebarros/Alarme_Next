/*
 * ExternalMemory.h
 *
 *  Created on: 26 de mar. de 2025
 *      Author: diego.marinho
 */

#ifndef API_DRIVERS_EXTERNALMEMORY_EXTERNALMEMORY_H_
#define API_DRIVERS_EXTERNALMEMORY_EXTERNALMEMORY_H_

#include <stdio.h>
#include <string.h>
#include "cybsp.h"
#include "cy_pdl.h"
#include "memory.h"

// Macros & Defines

#define AT25SF_CS_ON()      Cy_GPIO_Write((GPIO_PRT_Type*)at25_cfg.gpio, at25_cfg.pin, 0)

#define AT25SF_CS_OFF()     Cy_GPIO_Write((GPIO_PRT_Type*)at25_cfg.gpio, at25_cfg.pin, 1)
#define AT25SF_SPI			0
#define AT25SF_PAGE_SIZE	256
#define AT25SF_MAX_ADDR		0x7FFFFF

// Types
typedef enum _at25sf_error_t
{
	AT25SF_OK,
	AT25SF_ERROR,
} at25sf_error_t;

typedef enum _at25sf_block_size_t
{
	AT25SF_4K_BLOCK,
	AT25SF_32K_BLOCK,
	AT25SF_64K_BLOCK,
} at25sf_block_size_t;

typedef enum{
    AT25SF_POWER_OFF,
    AT25SF_POWER_ON
}at25sf_power_st_e;

typedef union {
    uint32_t valor;
    uint8_t barray[4];
} conv_32_t;

extern at25sf_error_t at25sf_error;
extern uint8_t at25sf_deselect_on_end;

typedef struct{
    uint16_t mtxI;
    uint32_t gpio;
    uint32_t pin;
    at25sf_power_st_e powerSt;
}at25_cfg_t;

// Module Interface
at25sf_error_t at25sf_init(uint16_t Mtxhold, uint32_t CSgpio, uint32_t CSpin);
at25sf_error_t at25sf_chip_erase(void);
at25sf_error_t at25sf_block_erase(uint32_t address, at25sf_block_size_t block_size);
at25sf_error_t at25sf_power_down(void);
at25sf_error_t at25sf_power_up(void);
at25sf_error_t at25sf_reset(void);
at25sf_error_t at25sf_send_byte(uint8_t data, uint8_t deselect);
at25sf_error_t at25sf_read(uint32_t address, uint8_t *data, uint16_t count);
at25sf_error_t at25sf_write(uint32_t address, uint8_t *data, uint16_t count);
void at25sf_set_index(uint16_t index);
void at25sf_busy_wait(uint32_t retries);


at25sf_error_t at25sf_set_write_status(uint8_t status);
void at25sf_update_status(uint8_t status);
uint32_t at25sf_read_id(void);

at25sf_error_t at25sf_send_bytes(uint8_t data, uint8_t deselect,uint8_t nBytes);

void mSPI_Interrupt(void);

#endif /* API_DRIVERS_EXTERNALMEMORY_EXTERNALMEMORY_H_ */
