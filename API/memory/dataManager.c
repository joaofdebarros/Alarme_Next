/*
 * dataManager.c
 *
 *  Created on: 10 de mar. de 2025
 *      Author: diego.marinho
 */



#include "dataManager.h"

uint32_t __val_endereco(mem_end_e endereco)
{
	if (endereco == MEM_END_BLE_ROOT) {
		return 8;
	}

//	while (endereco < MEM_LIST_CODE || endereco >= MEM_END_BIOM_E) {
//		// TODO erro de acesso
//
//		;
//	}
	return MEM_END_INIC_DATA + (endereco-MEM_END_SENHA_S);
}


void mem_le_access_file(uint16_t endereco, uint32_t data[MEM_DATA_SIZE], uint8_t size){

    endereco = MEM_END_SENHA_S + endereco*4;
    eeprom_le_raw_data(endereco, data, size);
}

uint8_t mem_esc_access_file(uint16_t endereco, uint32_t data[MEM_DATA_SIZE], uint8_t size){
    endereco = MEM_END_SENHA_S + endereco*4;
    return eeprom_esc_raw_data(endereco, data, size);
}

void mem_esc_root_ble(uint32_t* data)
{
	eeprom_esc_raw_data(0,data, MEM_BLE_ROOT_SIZE);
}

void mem_le_root_ble(uint32_t* data)
{
	eeprom_le_raw_data(0,data, MEM_BLE_ROOT_SIZE);
}

void mem_esc_owner_ble(uint32_t* data)
{
	//40 bytes = 10 palavras de 4 bytes (tamanho do root)
	eeprom_esc_raw_data(40,data, MEM_BLE_OWNER_SIZE);
}

void mem_le_owner_ble(uint32_t* data)
{
	eeprom_le_raw_data(40,data, MEM_BLE_OWNER_SIZE);
}

//uint8_t mem_esc_root_senha(uint64_t senha)
//{
//	if (eeprom_esc_double(__val_endereco(MEM_END_SENHA_ROOT), senha)) {
//		return 1;
//	}
//	if (eeprom_le_double(__val_endereco(MEM_END_SENHA_ROOT)) != senha) {
//		return 1;
//	}
//	return 0;
//}
