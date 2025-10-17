/*
 * firmwareUpdate.c
 *
 *  Created on: 8 de mai. de 2025
 *      Author: diego.marinho
 */


#include "firmwareUpdate.h"

#define METADATA_SIZE		sizeof(fota_metadata_u)

#define FOTA_EVENT_POST()   if (fota.hEvent != NULL) \
                                cy_rtos_event_setbits(fota.hEvent, fota.eventIdx);

static fota_data_t fota;
//static hTask_t fotaT;
static fota_pending_e fotaQ;

/*
 * auxiliary function to write the block on the flash memory
 * The function writes and reads the block to check if everything is Ok
 */
static uint8_t data[256];
fota_state_e __fota_write(){
	uint8_t retries = 4, ret=0;
	fota_state_e e;
	uint8_t read_verify[128];

	memcpy(data, fota.data, fota.blockSize);
	do{
		//at25sf_mtx_lock();
		//at25sf_power_up();
		//taskENTER_CRITICAL();
		ProgramPage(fota.actualAddr,fota.data,(uint32_t)fota.blockSize);
		//taskEXIT_CRITICAL();
		//at25sf_write(fota.actualAddr, fota.data, fota.blockSize);
		//at25sf_read(fota.actualAddr, data, fota.blockSize+5);
		//at25sf_power_up();
		S25FL_ReadPage(fota.actualAddr,data,fota.blockSize);
		memcpy(read_verify, data+4, fota.blockSize);
		//at25sf_power_down();
		ret = memcmp(fota.data, read_verify, fota.blockSize);
		retries++;

	}while (retries < 4 && ret != 0);
	if (ret != 0){
		e = FOTA_ERROR;
		fota.lastN--;
	}
	else{
		//fota.crc32Calc = crc32(fota.crc32Calc, data, fota.blockSize);
		fota.crc32Calc = crc32(fota.crc32Calc, fota.data, fota.blockSize);

		fota.actualAddr += (fota.blockSize)*2;
		fota.writedSize += fota.blockSize;
		if (fota.writedSize < fota.totalSizeCrip){
			e = FOTA_WRITING;
		}
		else{
			e = FOTA_END_WRITE;
		}
	}
	return e;
}

/*
 * function to check if the writed data on flash in OK
 */
fota_state_e __fota_verify(){
	if (fota.crc32Calc == fota.crc32){
		fota_metadata_u metadata = {0};

		metadata.CRC32 = fota.crc32;
		metadata.len = fota.totalSize;
		metadata.version = 1;

		//at25sf_mtx_lock();
//		at25sf_power_up();
//		ProgramPage(fota.beginAddr, metadata.raw, METADATA_SIZE);
//		at25sf_power_down();
		ProgramPage(fota.beginAddr,metadata.raw,128);
		//at25sf_mtx_unlock();

		fota.state = FOTA_DONE;
	}
	else{
		fota.state = FOTA_CRC32_ERROR;
	}

	return fota.state;
}

/*
 * auxiliary function to erase the flash section
 */
uint8_t erased[256];
void __fota_erase_flash(uint32_t size){
	uint32_t it, i;

	it = ceil((float)size/0x1000)+1; // aways remember to cast to float on ceil or floor operations

	//at25sf_mtx_lock();
    //at25sf_power_up();
    cy_rtos_delay_milliseconds(50);
    //at25sf_mtx_unlock();

    //for (i=0; i<it ; i++){
        //at25sf_mtx_lock();
        //at25sf_block_erase(0, AT25SF_64K_BLOCK);
        EraseSector();
        cy_rtos_delay_milliseconds(50);
        //at25sf_mtx_unlock();
    //}

    //at25sf_mtx_lock();
    //at25sf_power_down();
    //at25sf_mtx_unlock();
}

/*
 * task of the Fota
 */
void __FOTA_task(){
	fota_state_e e, f;

	do{
		switch (fotaQ){
		case FOTA_PEND_NONE:
			// if nothing is going on
			cy_rtos_semaphore_get(&fota.pendSemaphore,portMAX_DELAY);
			break;
		case FOTA_PEND_ERASE:
			// first step is to erase the flash
			__fota_erase_flash(fota.totalSize+METADATA_SIZE);
			fotaQ = FOTA_PEND_RESPONSE_OK;
			FOTA_EVENT_POST();
			break;
		case FOTA_PEND_BLOCK:
			// when a block is pending to write,
			// if the block is the last one, CRC32 will be checked, if OK, reboot
			e = __fota_write();
			if (e == FOTA_WRITING){
				fotaQ = FOTA_PEND_RESPONSE_OK;
				//FOTA_EVENT_POST();
				alarm_setAlarm_os(fota.hEvent, fota.eventIdx, 100, ALARM_MODE_ONESHOT);
			}
			else if(e == FOTA_END_WRITE){
				f = __fota_verify();
				if (f == FOTA_DONE){
					fotaQ = FOTA_PEND_RESPONSE_DONE;
					FOTA_EVENT_POST();
				}
				else{
					fotaQ = FOTA_PEND_RESPONSE_FAIL;
					FOTA_EVENT_POST();
				}
			}
			else{
				fotaQ = FOTA_PEND_RESPONSE_FAIL;
			}
			break;
		default:
			cy_rtos_semaphore_get(&fota.pendSemaphore,portMAX_DELAY);
			break;
		}
	} while (fotaQ != FOTA_PEND_STOP);
}

void FirmwareUpdate_init(hEvents_t handler, uint32_t index){
	memset(&fota, 0, sizeof(fota_data_t));
	fota.state = FOTA_IDLE;

	fotaQ = FOTA_PEND_NONE;
	cy_rtos_semaphore_init(&fota.pendSemaphore,1,0);
	fota.fotaT = cy_rtos_thread_create(&fota.thread_Fota, __FOTA_task,"fota",NULL,1024,3,NULL);
	if (handler != NULL && index != 0){
	    fota.eventIdx = index;
	    fota.hEvent = handler;
	}
}

bool FirmwareUpdate_startWrite(uint32_t beginAddr, uint32_t Size, uint32_t crc32){

//	if (fotaRegion == FOTA_REGION0)
//	    fs_file_open(FOTA_REGION0_FILE);
//	else
//	    fs_file_open(FOTA_REGION1_FILE);
//	beginAddr = fs_file_address(PSFFSYS_ADDRESS_ABSOLUTE);
//	fs_file_close();

	//fota.actualAddr = beginAddr+METADATA_SIZE;
	fota.actualAddr = 0x100;
	fota.beginAddr = beginAddr;
	fota.writedSize = 0;
	fota.lastN = 0xFFFF;
	fota.totalSize = Size;

	fota.totalSizeCrip = Size + (16 - (Size % 16));


	fota.crc32 = crc32;
	fota.crc32Calc = 0;

	fota.state = FOTA_START;

	fotaQ = FOTA_PEND_ERASE;
	cy_rtos_semaphore_set(&fota.pendSemaphore);

	return true;
}

fota_state_e FirmwareUpdate_nextWrite(uint8_t *data, uint16_t blockN, uint16_t block_size){
	fota_state_e e;

	if (blockN != fota.lastN){
		fota.blockSize = block_size;
		fota.lastN = blockN;
		memcpy(fota.data, data, block_size);
		e = FOTA_WRITING;
		fotaQ = FOTA_PEND_BLOCK;
	}
	else{
		e = FOTA_WRITING;
		fotaQ = FOTA_PEND_RESPONSE_OK;
	}
	cy_rtos_semaphore_set(&fota.pendSemaphore);

	return e;
}

fota_pending_e FirmwareUpdate_getState(){
	fota_pending_e b;
	b = fotaQ;

	switch (fotaQ){
	case FOTA_PEND_RESPONSE_DONE:
	case FOTA_PEND_RESPONSE_FAIL:
	case FOTA_PEND_RESPONSE_OK:
		// if a response is pending, return the response and put the Machine State on IDLE (FOTA_PEND_NONE)
		b = fotaQ;
		fotaQ = FOTA_PEND_NONE;
		break;
	default:
		b = FOTA_PEND_NO_RESPONSE;
		break;
	}

	return b;
}
