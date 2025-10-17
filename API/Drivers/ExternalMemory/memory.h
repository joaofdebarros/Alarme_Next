/*
 * memory.h
 *
 *  Created on: 03-Apr-2025
 *      Author: pradeepnaidu
 */

#ifndef MEMORY_H_
#define MEMORY_H_


#include "cybsp.h"
#include <stdio.h>
#include <string.h>

#define INIT_SUCCESS            (0)
#define INIT_FAILURE            (1)

void S25FL_ReadPage(uint32_t address,  uint8_t  *pSPIReceiveData, uint32_t len);
uint16_t S25FL_StatusRead();
void S25FL_ProgrammPage(uint32_t address,  uint8_t  *pSPISendData, uint32_t lentgh);
void S25FL_SectorErase(uint32_t address);
void S25FL_WriteEnable();
uint32_t initMaster(void);

uint8_t EraseSector();
uint8_t ProgramPage(uint32_t address,  uint8_t  *pSPISendData, uint32_t len);


#endif /* MEMORY_H_ */
