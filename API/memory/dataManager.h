/*
 * dataManager.h
 *
 *  Created on: 10 de mar. de 2025
 *      Author: diego.marinho
 */

#ifndef API_MEMORY_DATAMANAGER_H_
#define API_MEMORY_DATAMANAGER_H_

#include "stdint.h"
#include "eeprom.h"

#define SEG_ACCESS_PER_FILE     10
#define MEM_END_INIC_DATA	    28
#define MEM_BLE_ROOT_SIZE	    9  // -> 36 bytes
#define MEM_BLE_OWNER_SIZE	    11  // -> 36 bytes
#define MEM_DATA_SIZE		    20 // -> 56 bytes
#define MEM_FILE_ACC_SIZE       (MEM_DATA_SIZE*SEG_ACCESS_PER_FILE) // -> 140 words = 560 bytes

#define SEG_QTDE_SENHAS         50
#define SEG_QTDE_RFID           50
#define SEG_QTDE_BLE            50
#define SEG_QTDE_BIOM           100


#define SEG_FILES_SENHA         (SEG_QTDE_SENHAS/SEG_ACCESS_PER_FILE)
#define SEG_FILES_RFID          (SEG_QTDE_RFID/SEG_ACCESS_PER_FILE)
#define SEG_FILES_BLE           (SEG_QTDE_BLE/SEG_ACCESS_PER_FILE)
#define SEG_FILES_BIOM          (SEG_QTDE_BIOM/SEG_ACCESS_PER_FILE)

#define MAX_CODE 10
#define MAX_SETOR_FIXO 16
#define MAX_SETOR 10
#define MAX_PARTITION 10
#define MAX_SENSOR 10
#define MAX_PGM 20
#define MAX_TECLADO 3
#define MAX_GATE 2
#define MAX_CENARIO 5
#define MEM_DATA_SETOR_SIZE		    7
#define MEM_DATA_PARTITION_SIZE		    10
#define MEM_DATA_SENSOR_SIZE		    3
#define MEM_DATA_PGM_SIZE		    2
#define MEM_DATA_TECLADO_SIZE		    1
#define MEM_DATA_GATE_SIZE		    2
#define MEM_DATA_CENARIO_SIZE		    12
#define TAMANHO_CODE 42

#define MAX_ACTIONS_PER_SCENARIO   4


typedef enum{
    MEM_CLASS_CODE,
    MEM_CLASS_REMOTE,
    MEM_CLASS_TECL,
	MEM_CLASS_ROLE,

    MEM_CLASS_QTD
} mem_access_class_e;

typedef enum{
    MEM_USER_VIRTUAL,
    MEM_USER_TECL,
	MEM_USER_ROLE,
	MEM_USER_CONTROL,
} mem_access_class_ident_e;

typedef enum {
    MEM_NONE = 0,
    MEM_FULL = 1,

    MEM_LIST_CODE = 3,
    MEM_LIST_RFID = 4,

    MEM_END_CFG, // stores configurations of Digital Lock, like autolock, volume, etc

    MEM_END_SECRET_1,
    MEM_END_SECRET_2,

	MEM_END_SENHA_ROOT 	= 0x10, // reserve the 10 first address,
	MEM_END_BLE_ROOT, // 8; size = 20 define em memoria.h:15

	MEM_END_SENHA_S = 0,
	MEM_END_SENHA_E = (SEG_FILES_SENHA-1) + MEM_END_SENHA_S, // each file stores 10 codes access

	MEM_END_RFID_S,
	MEM_END_RFID_E = (SEG_FILES_RFID-1) + MEM_END_RFID_S, // each file stores 10 tags access

	MEM_END_BLE_S,
	MEM_END_BLE_E = (SEG_FILES_BLE-1) + MEM_END_BLE_S, // each file stores 10 ekeys access

	MEM_END_BIOM_INFO,
    MEM_END_BIOM_S,
    MEM_END_BIOM_E = (SEG_FILES_BIOM-1) + MEM_END_BIOM_S, // each file stores 10 fingerprints access

	// end of this type of configs
	MEM_END

} mem_end_e;


void mem_le_access_file(uint16_t endereco, uint32_t data[MEM_DATA_SIZE],uint8_t size);
uint8_t mem_esc_access_file(uint16_t endereco, uint32_t data[MEM_DATA_SIZE], uint8_t size);
void mem_esc_root_ble(uint32_t* data);

#endif /* API_MEMORY_DATAMANAGER_H_ */
