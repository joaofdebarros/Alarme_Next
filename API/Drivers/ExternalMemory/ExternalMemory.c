/*
 * ExternalMemory.c
 *
 *  Created on: 26 de mar. de 2025
 *      Author: diego.marinho
 */

#include "ExternalMemory.h"

static void SPI_Memory_Init(void);

// Commands
#define AT25SF_FAST_READ_COMMAND		0x0B
#define AT25SF_DEEP_POWER_DOWN			0xB9
#define AT25SF_RELEASE_POWER_DOWN		0xAB
#define AT25SF_PAGE_PROGRAM				0x02
#define AT25SF_CHIP_ERASE				0xC7
#define AT25SF_WRITE_ENABLE				0x06
#define AT25SF_WRITE_DISABLE			0x04
#define AT25SF_ENABLE_RESET				0x66
#define AT25SF_RESET_DEVICE				0x99
#define AT25SF_4K_BLOCK_ERASE			0x20
#define AT25SF_32K_BLOCK_ERASE			0x52
#define AT25SF_64K_BLOCK_ERASE			0xD8
#define AT25SF_READ_UNIQUE_ID			0x4B
#define AT25SF_READ_STATUS_1			0x05
#define AT25SF_READ_STATUS_2			0x35
#define AT25SF_READ_STATUS_3			0x15
#define AT25SF_PROGRAM_SUSPEND			0x75
#define AT25SF_PROGRAM_RESUME			0x7A
#define AT25SF_WRITE_STATUS_REG			0x01
#define AT25SF_GET_JEDEC_ID             0x9F

// Bits
#define AT25SF_STATUS_BUSY				0x01
#define AT25SF_STATUS_WEL				0x02

// Internal Prototypes
at25sf_error_t at25sf_set_write_status(uint8_t status);
at25sf_error_t at25sf_operation_wait(void);
at25sf_error_t at25sf_send_byte(uint8_t data, uint8_t deselect);
at25sf_error_t at25sf_send_command_with_address(uint8_t command, uint32_t address, uint8_t with_dummy, uint8_t deselect);
at25sf_error_t at25sf_write_status_reg(uint8_t value);
void at25sf_update_status(uint8_t status);

// State variables
uint8_t at25sf_line = 0;
uint8_t at25sf_command_buf[5];
uint8_t at25sf_unique_id[8];
uint8_t at25sf_status[3];
at25sf_error_t at25sf_error = AT25SF_OK;
uint8_t at25sf_deselect_on_end = 0;
uint16_t cnt = 0;
uint32_t adrrt = 0;
uint32_t tick = 0, ant_tick, wait_line;
at25_cfg_t at25_cfg = {0};

cy_stc_scb_spi_context_t spiContext;  // Contexto da SPI

at25sf_error_t at25sf_init(uint16_t Mtxhold, uint32_t CSgpio, uint32_t CSpin)
{
    at25_cfg.gpio = CSgpio;
    at25_cfg.pin = CSpin;
    at25_cfg.mtxI = Mtxhold;

    //SPI_Memory_Init();
    initMaster();

	// Powering Up the memory.
	return 0;
}

void at25sf_set_index(uint16_t index){
    at25_cfg.mtxI = index;
}

at25sf_error_t at25sf_chip_erase(void)
{
	Cy_SCB_SPI_Enable(SPI_MEMORY_HW);
	if (at25sf_set_write_status(1) == AT25SF_OK)
	{
		while (Cy_SCB_SPI_IsBusBusy(SPI_MEMORY_HW));
		Cy_SCB_SPI_Disable(SPI_MEMORY_HW, &spiContext);

		Cy_SCB_SPI_Enable(SPI_MEMORY_HW);
		at25sf_update_status(1);
		Cy_SCB_SPI_Disable(SPI_MEMORY_HW, &spiContext);

		if (at25sf_status[0] | AT25SF_STATUS_WEL)
		{
			Cy_SCB_SPI_Enable(SPI_MEMORY_HW);
			at25sf_send_byte(AT25SF_CHIP_ERASE, 1);
			while (Cy_SCB_SPI_IsBusBusy(SPI_MEMORY_HW));
			Cy_SCB_SPI_Disable(SPI_MEMORY_HW, &spiContext);

			at25sf_busy_wait(42000);
		}
	}

	return at25sf_error;
}

void at25sf_busy_wait(uint32_t retries)
{
    retries *= 5;
	do
	{
		Cy_SCB_SPI_Enable(SPI_MEMORY_HW);
		at25sf_update_status(1);
		Cy_SCB_SPI_Disable(SPI_MEMORY_HW, &spiContext);
		// willl be good if implement a us delay function, agree?
		Cy_SysLib_DelayUs(200);
		retries--;

	} while ((at25sf_status[0] & AT25SF_STATUS_BUSY) && retries);
}

void at25sf_update_status(uint8_t status)
{
	uint8_t readfifo[6];
	if (status < 1 || status > 2)
		status = 1;

	at25sf_status[status - 1] = 0xFF;

	switch (status)
	{
	case 1:
		at25sf_command_buf[0] = AT25SF_READ_STATUS_1;
		at25sf_send_bytes(at25sf_command_buf[0], 0,2);
		break;

	case 2:
		at25sf_command_buf[0] = AT25SF_READ_STATUS_2;
		at25sf_send_bytes(at25sf_command_buf[0], 0,2);
		break;

	case 3:
		at25sf_command_buf[0] = AT25SF_READ_STATUS_3;
		at25sf_send_bytes(at25sf_command_buf[0], 0,2);
		break;
	}

	//hSpi_receive(AT25SF_SPI, &at25sf_status[status - 1], 1);
	while (Cy_SCB_SPI_IsBusBusy(SPI_MEMORY_HW));
	Cy_SCB_SPI_ReadArray(SPI_MEMORY_HW, readfifo, 3);
	at25sf_status[status - 1] = readfifo[1];
	at25sf_operation_wait();
	AT25SF_CS_OFF();
}

void at25sf_mtx_lock(){
    //hSpi_mtx_lock(at25_cfg.SPIHandle, 0x02);
}

void at25sf_mtx_unlock(){
    //hSpi_mtx_unlock(at25_cfg.SPIHandle);
}

uint32_t at25sf_read_id(void)
{
	Cy_SCB_SPI_Enable(SPI_MEMORY_HW);
	uint8_t teste_tx[4], teste_tx2;
	teste_tx[0] = 0x9F;
	teste_tx[1] = 0xff;
	teste_tx[2] = 0xff;
	teste_tx[3] = 0xff;
	uint32 masterStatus;
	at25sf_command_buf[0] = AT25SF_GET_JEDEC_ID;

	teste_tx2 = 0x9F;
    conv_32_t t;
    int i;
    // Send Command to Read Unique ID.
    //at25sf_send_byte(AT25SF_GET_JEDEC_ID, 0);
    AT25SF_CS_ON();
    //Cy_SCB_SPI_Transfer_Buffer(SPI_MEMORY_HW, teste_tx, at25sf_unique_id, 4, 4, 0xFF, &spiContext);
    //Cy_SCB_SPI_Transfer(SPI_MEMORY_HW,teste_tx,at25sf_unique_id,4,&spiContext);
    Cy_SCB_SPI_WriteArray(SPI_MEMORY_HW, at25sf_command_buf, 4);
    // Clear ID buffer.
    for (i = 0; i < 8; i++)
        at25sf_unique_id[i] = 0x00;

    //while (Cy_SCB_SPI_IsBusBusy(SPI_MEMORY_HW));
    while (Cy_SCB_SPI_IsBusBusy(SPI_MEMORY_HW));

    // Receive ID bytes (64 bit).
    at25sf_deselect_on_end = 1;
    //hSpi_receive(AT25SF_SPI, at25sf_unique_id, 3);
    Cy_SCB_SPI_ReadArray(SPI_MEMORY_HW, at25sf_unique_id, 4);
    at25sf_operation_wait();
    //masterStatus  = Cy_SCB_SPI_GetTransferStatus(SPI_MEMORY_HW, &spiContext);CY_SCB_SPI_TRANSFER_ACTIVE
    AT25SF_CS_OFF();

    memcpy(t.barray, at25sf_unique_id+1, 4);
    Cy_SCB_SPI_Disable(SPI_MEMORY_HW, &spiContext);

    return t.valor;
}

at25sf_error_t at25sf_block_erase(uint32_t address, at25sf_block_size_t block_size)
{
	uint8_t command = AT25SF_4K_BLOCK_ERASE;
	uint16_t wait = 20;

	switch (block_size)
	{
	case AT25SF_4K_BLOCK:
		command = AT25SF_4K_BLOCK_ERASE;
		wait = 70;
		break;

	case AT25SF_32K_BLOCK:
		command = AT25SF_32K_BLOCK_ERASE;
		wait = 170;
		break;

	case AT25SF_64K_BLOCK:
		command = AT25SF_64K_BLOCK_ERASE;
		wait = 350;
		break;
	}
//	//Cy_SysLib_DelayUs(200);
//	Cy_SCB_SPI_Enable(SPI_MEMORY_HW);
//	at25sf_set_write_status(1);
//	while (Cy_SCB_SPI_IsBusBusy(SPI_MEMORY_HW));
//	Cy_SCB_SPI_Disable(SPI_MEMORY_HW, &spiContext);
//	//Cy_SysLib_DelayUs(200);
//
//	Cy_SCB_SPI_Enable(SPI_MEMORY_HW);
//	//Cy_SysLib_DelayUs(200);
//
//	at25sf_send_command_with_address(command, address, 0, 1);
//	Cy_SysLib_DelayUs(2000);
//
//	Cy_SCB_SPI_Disable(SPI_MEMORY_HW, &spiContext);

	Cy_SCB_SPI_Enable(SPI_MEMORY_HW);
	if (at25sf_set_write_status(1) == AT25SF_OK)
	{
		while (Cy_SCB_SPI_IsBusBusy(SPI_MEMORY_HW));
		Cy_SCB_SPI_Disable(SPI_MEMORY_HW, &spiContext);
		//Cy_SysLib_DelayUs(200);

		Cy_SCB_SPI_Enable(SPI_MEMORY_HW);
		at25sf_update_status(1);
		Cy_SCB_SPI_Disable(SPI_MEMORY_HW, &spiContext);

		if (at25sf_status[0] | AT25SF_STATUS_WEL)
		{
			Cy_SCB_SPI_Enable(SPI_MEMORY_HW);
			if (at25sf_send_command_with_address(command, address, 0, 1) == AT25SF_OK)
				while (Cy_SCB_SPI_IsBusBusy(SPI_MEMORY_HW));
			    Cy_SCB_SPI_Disable(SPI_MEMORY_HW, &spiContext);

				//Cy_SysLib_DelayUs(200);



				at25sf_busy_wait(wait);
		}
	}


	return at25sf_error;
}


at25sf_error_t at25sf_set_write_status(uint8_t status)
{
	if (status)
		return at25sf_send_byte(AT25SF_WRITE_ENABLE, 1);

	return at25sf_send_byte(AT25SF_WRITE_DISABLE, 1);
}

at25sf_error_t at25sf_write_status_reg(uint8_t value)
{
	if (at25sf_send_byte(AT25SF_WRITE_STATUS_REG, 0) == AT25SF_OK)
		return at25sf_send_byte(value, 1);

	return AT25SF_ERROR;
}

at25sf_error_t at25sf_power_down(void)
{
    at25sf_error_t err = AT25SF_OK;

    if (at25_cfg.powerSt == AT25SF_POWER_ON){
        err = at25sf_send_byte(AT25SF_DEEP_POWER_DOWN, 1);
        at25_cfg.powerSt = AT25SF_POWER_OFF;
    }

	return err;
}

at25sf_error_t at25sf_power_up(void)
{
    at25sf_error_t err;

    if (at25_cfg.powerSt == AT25SF_POWER_OFF){
        err = at25sf_send_byte(AT25SF_RELEASE_POWER_DOWN, 1);
        err = at25sf_send_byte(AT25SF_RELEASE_POWER_DOWN, 1);
        at25_cfg.powerSt = AT25SF_POWER_ON;
    }

    return err;
}

at25sf_error_t at25sf_reset(void)
{
	return at25sf_send_byte(AT25SF_RESET_DEVICE, 1);
}

at25sf_error_t at25sf_read(uint32_t address, uint8_t *data, uint16_t count)
{
	uint8_t test[128];
	memset(test,0xFF,128);
	at25sf_error_t err = AT25SF_OK;

	if ((address + count > AT25SF_MAX_ADDR) || count == 0)
		return AT25SF_ERROR;

	Cy_SCB_SPI_Enable(SPI_MEMORY_HW);
	if (at25sf_send_command_with_address(AT25SF_FAST_READ_COMMAND, address, 1, 0) == AT25SF_OK)
	{
		//at25sf_command_buf[0] = 0;
		//while (Cy_SCB_SPI_IsBusBusy(SPI_MEMORY_HW));
		Cy_SCB_SPI_WriteArray(SPI_MEMORY_HW, test, count);

//		for(int i = 0; i<count; i++){
//			Cy_SCB_SPI_WriteArray(SPI_MEMORY_HW, at25sf_command_buf, 1);
//			while (Cy_SCB_SPI_IsBusBusy(SPI_MEMORY_HW));
//		}
		while (Cy_SCB_SPI_IsBusBusy(SPI_MEMORY_HW));
		//hSpi_receive(AT25SF_SPI, data, count);
		Cy_SCB_SPI_ReadArray(SPI_MEMORY_HW, data, count+5);
		// TODO : if we have any problem comment this line
//		err = at25sf_operation_wait();
		Cy_SCB_SPI_Disable(SPI_MEMORY_HW, &spiContext);
		AT25SF_CS_OFF();

		return err;
	}

	Cy_SCB_SPI_Disable(SPI_MEMORY_HW, &spiContext);

	return at25sf_error;
}

at25sf_error_t at25sf_write(uint32_t address, uint8_t *data, uint16_t count)
{
	uint16_t curr_len = count, max = 0, pagelen = 0;
	Cy_SCB_SPI_Enable(SPI_MEMORY_HW);

	do
	{
		max = AT25SF_PAGE_SIZE - (address & 0xFF);
		pagelen = (curr_len <= max) ? curr_len : max;

		// every page program instruction requires to set the WEL bit
		if (at25sf_set_write_status(1) == AT25SF_OK)
		{
			while (Cy_SCB_SPI_IsBusBusy(SPI_MEMORY_HW));
			Cy_SCB_SPI_Disable(SPI_MEMORY_HW, &spiContext);

			Cy_SCB_SPI_Enable(SPI_MEMORY_HW);
			at25sf_update_status(1);
			Cy_SCB_SPI_Disable(SPI_MEMORY_HW, &spiContext);

			if (at25sf_status[0] & AT25SF_STATUS_WEL)
			{
				Cy_SCB_SPI_Enable(SPI_MEMORY_HW);
				if (at25sf_send_command_with_address(AT25SF_PAGE_PROGRAM, address, 0, 0) == AT25SF_OK)
				{
					//while (Cy_SCB_SPI_IsBusBusy(SPI_MEMORY_HW));
					Cy_SCB_SPI_WriteArray(SPI_MEMORY_HW, data, pagelen);
					while (Cy_SCB_SPI_IsBusBusy(SPI_MEMORY_HW));
					AT25SF_CS_OFF();
					Cy_SCB_SPI_Disable(SPI_MEMORY_HW, &spiContext);

					at25sf_busy_wait(20);

					data += pagelen;
					address += pagelen;
					curr_len -= pagelen;
				}
			}
		}
	} while (curr_len > 0);

	Cy_SCB_SPI_Disable(SPI_MEMORY_HW, &spiContext);


	return at25sf_error;
}

at25sf_error_t at25sf_send_byte(uint8_t data, uint8_t deselect)
{
	at25sf_command_buf[0] = data;
	at25sf_command_buf[1] = 0xff;

	at25sf_deselect_on_end = deselect;
	AT25SF_CS_ON();
	//hSpi_transmit(AT25SF_SPI, at25sf_command_buf, 1);
	Cy_SCB_SPI_WriteArray(SPI_MEMORY_HW, at25sf_command_buf, 1);
	//while (Cy_SCB_SPI_GetNumInTxFifo(SPI_MEMORY_HW) > 0);  // Aguarda FIFO esvaziar
	//while (Cy_SCB_SPI_IsBusBusy(SPI_MEMORY_HW));
	// Limpa qualquer erro no FIFO de recepção
	//Cy_SCB_SPI_ClearRxFifoStatus(SPI_MEMORY_HW, CY_SCB_SPI_RX_NO_DATA | CY_SCB_SPI_RX_OVERFLOW);

	// Limpa qualquer erro no FIFO de transmissão
	//Cy_SCB_SPI_ClearTxFifoStatus(SPI_MEMORY_HW, CY_SCB_SPI_TX_OVERFLOW);
	if (deselect){
		AT25SF_CS_OFF();
	}



	return at25sf_operation_wait();
}

at25sf_error_t at25sf_send_bytes(uint8_t data, uint8_t deselect,uint8_t nBytes)
{
	at25sf_command_buf[0] = data;
	at25sf_command_buf[1] = 0xFF;
	at25sf_command_buf[2] = 0xFF;

	at25sf_deselect_on_end = deselect;
	AT25SF_CS_ON();
	//hSpi_transmit(AT25SF_SPI, at25sf_command_buf, 1);
	Cy_SCB_SPI_WriteArray(SPI_MEMORY_HW, at25sf_command_buf, nBytes+1);
	while (Cy_SCB_SPI_IsBusBusy(SPI_MEMORY_HW));
	if (deselect){
		AT25SF_CS_OFF();
		Cy_SCB_SPI_ClearRxFifo(SPI_MEMORY_HW);
		Cy_SCB_SPI_ClearTxFifo(SPI_MEMORY_HW);
	}

	return at25sf_operation_wait();
}

at25sf_error_t at25sf_send_command_with_address(uint8_t command, uint32_t address, uint8_t with_dummy, uint8_t deselect)
{
	at25sf_command_buf[0] = command;
	at25sf_command_buf[1] = address >> 16;
	at25sf_command_buf[2] = address >> 8;
	at25sf_command_buf[3] = address >> 0;

	if (with_dummy)
		at25sf_command_buf[4] = 0xFF;

	at25sf_deselect_on_end = deselect;

	AT25SF_CS_ON();
	Cy_SCB_SPI_WriteArray(SPI_MEMORY_HW, at25sf_command_buf, with_dummy ? 5 : 4);
	//hSpi_transmit(AT25SF_SPI, at25sf_command_buf, with_dummy ? 5 : 4);
	//while (Cy_SCB_SPI_IsBusBusy(SPI_MEMORY_HW));
	if (deselect){
		AT25SF_CS_OFF();
	}




	return at25sf_operation_wait();
}

at25sf_error_t at25sf_operation_wait(void)
{
//	wait_line = 1;
//	ant_tick = hGet_Tick();
//	wait_line = 2;
//	tick = hGet_Tick() - ant_tick;
	//asm(" nop");
//    System_NOP();
//    System_NOP();
//    System_NOP();
//    System_NOP();
//    System_NOP();
//    System_NOP();

	return at25sf_error;
}

//void mSPI_Interrupt(void)
//{
//    Cy_SCB_SPI_Interrupt(SPI_MEMORY_HW, &spiContext);
//}

#define SPI_INTR_NUM        ((IRQn_Type) scb_4_interrupt_IRQn)
#define SPI_INTR_PRIORITY   (7U)

static void SPI_Memory_Init(void){

    const cy_stc_sysint_t mSPI_SCB_IRQ_cfg =
    {
        .intrSrc      = SPI_MEMORY_IRQ,
        .intrPriority = 7u
    };

	/* Configure SPI block */
	Cy_SCB_SPI_Init(SPI_MEMORY_HW, &SPI_MEMORY_config, &spiContext);

    /* Set active slave select to line 0 */
    //Cy_SCB_SPI_SetActiveSlaveSelect(SPI_MEMORY_HW, CY_SCB_SPI_SLAVE_SELECT0);

    /* Enable SPI master block. */
    Cy_SCB_SPI_Enable(SPI_MEMORY_HW);

    //Cy_SysInt_Init(&mSPI_SCB_IRQ_cfg, &mSPI_Interrupt);

    //NVIC_EnableIRQ((IRQn_Type) mSPI_SCB_IRQ_cfg.intrSrc);

}


