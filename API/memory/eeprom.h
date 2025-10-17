/*
 * eeprom.h
 *
 *  Created on: 7 de mar. de 2025
 *      Author: diego.marinho
 */

#ifndef MEMORY_EEPROM_H_
#define MEMORY_EEPROM_H_

#include "cybsp.h"
#include "cy_em_eeprom.h"

typedef union {
    uint64_t valor;
    struct {
        uint32_t w0;
        uint32_t w1;
    };
    struct {
        uint16_t hw0;
        uint16_t hw1;
        uint16_t hw2;
        uint16_t hw3;
    };
    struct {
        uint8_t b0;
        uint8_t b1;
        uint8_t b2;
        uint8_t b3;
        uint8_t b4;
        uint8_t b5;
        uint8_t b6;
        uint8_t b7;
    };
    uint8_t barray[8];
} conv_double_t;


cy_en_em_eeprom_status_t eeprom_init(void);
cy_en_em_eeprom_status_t eeprom_esc_raw_data(uint32_t endereco, uint32_t* data, uint16_t n);
cy_en_em_eeprom_status_t eeprom_le_raw_data(uint32_t endereco, uint32_t* data, uint16_t n);
void eeprom_erase(void);


#endif /* MEMORY_EEPROM_H_ */
