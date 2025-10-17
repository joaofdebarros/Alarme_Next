/*
 * eeprom.c
 *
 *  Created on: 7 de mar. de 2025
 *      Author: diego.marinho
 */


#include "eeprom.h"

/* The size of data to store in EEPROM for devices that have flash memory
 *  */
#define DATA_SIZE                       (CY_FLASH_SIZEOF_ROW)
/* The Simple Mode is turned off */
#define SIMPLE_MODE                     (0u)
/* Increases the flash endurance twice */
#define WEAR_LEVELING                   (2u)
/* The Redundant Copy is turned off */
#define REDUNDANT_COPY                  (0u)
/* The Blocking Write is turned on */
#define BLOCKING_WRITE                  (1u)

CY_ALIGN(DATA_SIZE)
const uint8_t emEepromStorage[CY_EM_EEPROM_GET_PHYSICAL_SIZE(DATA_SIZE*6, SIMPLE_MODE, WEAR_LEVELING,
                                                             REDUNDANT_COPY)] = { 0u };

cy_stc_eeprom_context_t em_eeprom_context;

/* Emulated EEPROM configuration and context structure. */
cy_stc_eeprom_config_t em_eeprom_config =
{
    .eepromSize         = DATA_SIZE*6,                /* 512 bytes */
    .blockingWrite      = 1,        /* Blocking writes enabled */
    .redundantCopy      = REDUNDANT_COPY,      /* Redundant copy enabled */
    .wearLevelingFactor = WEAR_LEVELING, /* Wear levelling factor of 2 */
    .simpleMode         = SIMPLE_MODE,          /* Simple mode disabled */
	.userFlashStartAddr   = (uint32_t)&(emEepromStorage[0u]),
};

cy_en_em_eeprom_status_t eeprom_init(void){
	cy_en_em_eeprom_status_t em_eeprom_status;
	em_eeprom_config.userFlashStartAddr = (uint32_t) emEepromStorage;
	em_eeprom_status = Cy_Em_EEPROM_Init(&em_eeprom_config, &em_eeprom_context);
	return em_eeprom_status;

}

cy_en_em_eeprom_status_t eeprom_esc_raw_data(uint32_t endereco, uint32_t* data, uint16_t n)
{
	cy_en_em_eeprom_status_t em_eeprom_status_write;
    /* Write initial data to Emulated EEPROM. */
    em_eeprom_status_write = Cy_Em_EEPROM_Write(endereco,(uint8_t*)data,n*4,&em_eeprom_context);
    return em_eeprom_status_write;
}

cy_en_em_eeprom_status_t eeprom_le_raw_data(uint32_t endereco, uint32_t* data, uint16_t n)
{
	cy_en_em_eeprom_status_t em_eeprom_status_read;
    /* Read contents of Emulated EEPROM after write */
	em_eeprom_status_read = Cy_Em_EEPROM_Read(endereco,(uint8_t*)data,n*4,&em_eeprom_context);
	return em_eeprom_status_read;
}

void eeprom_erase(void){
	Cy_Em_EEPROM_Erase(&em_eeprom_context);
}

