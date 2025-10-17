/*
 * firmwareUpdate.h
 *
 *  Created on: 8 de mai. de 2025
 *      Author: diego.marinho
 */

#ifndef API_UPDATE_FIRMWAREUPDATE_H_
#define API_UPDATE_FIRMWAREUPDATE_H_


#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <math.h>

#include "ExternalMemory.h"
#include "crc.h"
#include "cy_pdl.h"
#include "cybsp.h"
#include "alarm.h"

#include "memory.h"


#define MAX_BLOCK_SIZE		224

/*
 * Enumerates
 */
typedef enum {
	FOTA_IDLE,
	FOTA_START,
	FOTA_WRITING,
	FOTA_END_WRITE,
	FORA_FLASH_WRITE_ERROR,
	FOTA_DONE,
	FOTA_CRC32_ERROR,
	FOTA_ERROR
} fota_state_e;

typedef enum{
	FOTA_PEND_NONE,
	FOTA_PEND_ERASE,
	FOTA_PEND_BLOCK,
	FOTA_PEND_NO_RESPONSE,
	FOTA_PEND_RESPONSE_OK,
	FOTA_PEND_RESPONSE_DONE,
	FOTA_PEND_RESPONSE_FAIL,
	FOTA_PEND_STOP
} fota_pending_e;

typedef enum{
    FOTA_NONE    = 0,
    FOTA_REGION0 = 1,
    FOTA_REGION1 = 2
} fota_region_e;

/*
 * Typedefs, Structs and Unions
 */
typedef struct{
	cy_rslt_t fotaT; //variável de status RTOS
	cy_thread_t thread_Fota; //variável contexto da task RTOS
	cy_semaphore_t  pendSemaphore;
    hEvents_t hEvent;
    uint32_t eventIdx;
	fota_state_e state;
	uint32_t beginAddr;
	uint32_t actualAddr;
	uint32_t writedSize;
	uint32_t totalSize;
	uint32_t totalSizeCrip;
	uint16_t blockSize;
	uint16_t lastN;
	uint8_t data[MAX_BLOCK_SIZE];
	uint32_t crc32;
	uint32_t crc32Calc;
}fota_data_t;

typedef union{
	struct{
		uint32_t len;
		uint32_t CRC32;
		uint32_t version;
		uint8_t _reserved[116];
	};
	uint8_t raw[128];
}fota_metadata_u;

typedef union{
    struct{
        fota_region_e news     : 4;
        fota_region_e active  : 4;
    };
    uint8_t raw;
}fota_info_t;

/*
 * Forwarded functions
 */

/**
 * @brief			Initialize the variables and start the FOTA task
 *
 * 	@return 		nothing
 */
//void FirmwareUpdate_init(hEvents_t handler, uint32_t index);
void FirmwareUpdate_init(hEvents_t handler, uint32_t index);

/**
 * @brief			Start the FOTA Update, the first step is to erase the sectors where the new firmware will be stored
 *
 * 	@param[in]		beginAddr: Initial addr for FOTA data, including metadata and firmware file
 * 	@param[in]		Size: Size of the firmware in bytes
 * 	@param[in]		md5Data: array containing the 16 bytes of the MD5
 *
 * 	@return 		true if MD5 ins't NULL
 */
bool FirmwareUpdate_startWrite(uint32_t beginAddr, uint32_t Size, uint32_t crc32);

/**
 * @brief			Put the machine state in the write mode, this operation will write the desired block
 * 					When check the _getState(), will return FOTA_WRITING while doing the process, when finish,
 * 					return FOTA_PEND_RESPONSE_OK when block was writed succefully,
 * 					return FOTA_PEND_RESPONSE_DONE when last block was sucefully stored and all data is OK,
 * 					otherwise, returns FOTA_PEND_RESPONSE_FAIL
 *
 * 	@param[in]		beginAddr: Initial addr for FOTA data, including metadata and firmware file
 * 	@param[in]		Size: Size of the firmware in bytes
 * 	@param[in]		md5Data: array containing the 16 bytes of the MD5
 *
 * 	@return 		true if MD5 ins't NULL
 */
fota_state_e FirmwareUpdate_nextWrite(uint8_t *data, uint16_t blockN, uint16_t block_size);

/**
 * @brief			Get the response of the machine state, only returns the Response Pend, the busy state always return
 * 					FOTA_STATE_NONE.
 *
 * 	@return 		if the last command was flash_nextWrite():
 * 					FOTA_PEND_RESPONSE_OK
 * 						When the block was succefully writed on flash memory
 *
 *					FOTA_PEND_RESPONSE_DONE
 *						When the last block was succefully writed on flash memory
 *						and the entire firmware is OK, using the MD5 hash
 *
 *					FOTA_PEND_RESPONSE_FAIL
 *						When the firmware check or the write block was corrupted
 *
 */
fota_pending_e FirmwareUpdate_getState();

/**
 * @brief			reset the microcontroller and go to bootloader
 *
 * 	@return 		don't return (really!!)
 */
void JumpToBootloader();


#endif /* API_UPDATE_FIRMWAREUPDATE_H_ */
