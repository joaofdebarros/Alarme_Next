/*
 * memory.c
 *
 *  Created on: 03-Apr-2025
 *      Author: pradeepnaidu
 */

/*******************************************************************************
 Copyright (c) 2015, Infineon Technologies AG                                 **
 All rights reserved.                                                         **
                                                                              **
 Redistribution and use in source and binary forms, with or without           **
 modification,are permitted provided that the following conditions are met:   **
                                                                              **
 *Redistributions of source code must retain the above copyright notice,      **
 this list of conditions and the following disclaimer.                        **
 *Redistributions in binary form must reproduce the above copyright notice,   **
 this list of conditions and the following disclaimer in the documentation    **
 and/or other materials provided with the distribution.                       **
 *Neither the name of the copyright holders nor the names of its contributors **
 may be used to endorse or promote products derived from this software without**
 specific prior written permission.                                           **
                                                                              **
 THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"  **
 AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE    **
 IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE   **
 ARE  DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE   **
 LIABLE  FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR         **
 CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF         **
 SUBSTITUTE GOODS OR  SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS    **
 INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN      **
 CONTRACT, STRICT LIABILITY,OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)       **
 ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE   **
 POSSIBILITY OF SUCH DAMAGE.                                                  **
                                                                              **
 To improve the quality of the software, users are encouraged to share        **
 modifications, enhancements or bug fixes with Infineon Technologies AG       **
 dave@infineon.com).                                                          **
                                                                              **
********************************************************************************
**                                                                            **
**                                                                            **
** PLATFORM : Infineon Psoc COntrol                                        **
**                                                                            **
** AUTHOR : Pradeep                                     **
**                                                                            **
                                         **
**                                                                            **
*******************************************************************************/


#include "memory.h"

/* S25FL flash chip command definitions */
#define S25FL_CM_READ   		0x03
#define S25FL_CM_READ_FAST 	    0x0B
#define S25FL_CM_QUAL_READ  	0x3B
#define S25FL_CM_QUAD_READ  	0x6B
#define S25FL_CM_ID_READ    	0x9F

#define S25FL_CM_WRITE_EN  	0x06
#define S25FL_CM_WRITE_DIS 	0x04

#define S25FL_CM_ERASE_P4E 	0x20 // 4KB sector
#define S25FL_CM_ERASE_P8E 	0x40 // 8KB sector
#define S25FL_CM_ERASE_SE   0xD8 // 64KB sector

#define S25FL_CM_PROG_PAGE 	    0x02 // page programming
#define S25FL_CM_PROG_QPAGE 	0x32 // quad page programming

#define S25FL_CM_REG_WRITE 	    0x01 // write status and configure register
#define S25FL_CM_REG_READ_ST  	0x05 // read status register
#define S25FL_CM_REG_READ_CFG	0x35 // read configure register
#define S25FL_CM_REG_RESET 	0x30 // reset the erase and program fall flag

#define mSPI_INTR_PRIORITY  (0)

cy_stc_scb_spi_context_t mSPI_context;

uint32_t spi_event = 0;

/*******************************************************************************
 * Function Name: mSPI_Interrupt
 *******************************************************************************
 *
 * Invokes the Cy_SCB_SPI_Interrupt() PDL driver function.
 *
 ******************************************************************************/
void mSPI_Interrupt(void)
{
    Cy_SCB_SPI_Interrupt(SPI_MEMORY_HW, &mSPI_context);
}

static void callback(uint32_t event)
{
	spi_event = event;

}

/*******************************************************************************
 * Function Name: initMaster
 *******************************************************************************
 *
 * Summary:
 * This function initializes the SPI master based on the configuration done in
 * design.modus file.
 *
 * Parameters:
 * None
 *
 * Return:
 * uint32_t - Returns INIT_SUCCESS if the initialization is successful.
 * Otherwise it returns INIT_FAILURE
 *
 ******************************************************************************/
uint32_t initMaster(void)
{
    cy_en_scb_spi_status_t result;

    cy_en_sysint_status_t sysSpistatus;

    /* Configure the SPI block */

    result = Cy_SCB_SPI_Init(SPI_MEMORY_HW, &SPI_MEMORY_config, &mSPI_context);

    if( result != CY_SCB_SPI_SUCCESS)
    {
        return(INIT_FAILURE);
    }

    /* Set active slave select to line 0 */
    Cy_SCB_SPI_SetActiveSlaveSelect(SPI_MEMORY_HW, CY_SCB_SPI_SLAVE_SELECT0);

    /* Populate configuration structure */

    Cy_SCB_SPI_RegisterCallback	(SPI_MEMORY_HW,callback,&mSPI_context );

    const cy_stc_sysint_t mSPI_SCB_IRQ_cfg =
    {
            .intrSrc      = SPI_MEMORY_IRQ,
            .intrPriority = mSPI_INTR_PRIORITY
    };

    /* Hook interrupt service routine and enable interrupt */
    sysSpistatus = Cy_SysInt_Init(&mSPI_SCB_IRQ_cfg, &mSPI_Interrupt);

    if(sysSpistatus != CY_SYSINT_SUCCESS)
    {
        return(INIT_FAILURE);
    }
    /* Enable interrupt in NVIC */
    NVIC_EnableIRQ(SPI_MEMORY_IRQ);

    /* Enable the SPI Master block */
    Cy_SCB_SPI_Enable(SPI_MEMORY_HW);

    /* Initialization completed */
    return(INIT_SUCCESS);
}

/**
 *This function sends read status register command to SPI flash chip
 *and returns Status register value
 *
 *
 * @return	SPI Flash status register value
 *
 */
uint16_t S25FL_StatusRead(void)
{
  uint8_t rxBuffer[2];

  uint8_t data[2];


  /*Load the command*/
  data[0] = S25FL_CM_REG_READ_ST;


  /* Receive the status command from the  SPI flash chip */
  spi_event = 0;

  Cy_SCB_SPI_Transfer(SPI_MEMORY_HW, data, rxBuffer, 2, &mSPI_context);

  while(spi_event != CY_SCB_SPI_TRANSFER_CMPLT_EVENT);

  return rxBuffer[1];
}


/**
 *This function sends sector erase command
 *
 */

void  S25FL_SectorErase(uint32_t address)
{
  uint8_t data[4];

  /*Load the command*/
  data[0] = S25FL_CM_ERASE_SE;



  /* Send 24 bit address of sector starting from MSB
   *
   * Send 3rd byte of 24bit address
   *
   * */
  data[1] = ((address & 0x00FF0000)>>16);

  /*Send 24 bit address of sector starting from MSB
   *
   * Send 2nd byte of 24bit address
   *
   * */
  data[2] = ((address & 0x0000FF00)>>8);


  /*Send 24 bit address of sector starting from MSB
   *
   * Send 1st byte of 24bit address
   *
   * */
  data [3]=((address & 0x000000FF)>>0);

  spi_event = 0;

  Cy_SCB_SPI_Transfer(SPI_MEMORY_HW, data, NULL, 4, &mSPI_context);


  while(spi_event != CY_SCB_SPI_TRANSFER_CMPLT_EVENT);


}

/**
 * This function sends the write enable command to SPI flash chip
 *
 */
void S25FL_WriteEnable(void)
{
  uint8_t data = 0;


  /* Write Enable command*/
  data = S25FL_CM_WRITE_EN;

  spi_event = 0;

  Cy_SCB_SPI_Transfer(SPI_MEMORY_HW, &data, NULL, 1, &mSPI_context);

  while(spi_event != CY_SCB_SPI_TRANSFER_CMPLT_EVENT);


}


/**
 *This function sends read page command and reads out 256 bytes
 *of data from sector specified to the given buffer
 *
 */
void S25FL_ReadPage(uint32_t address,  uint8_t  *pSPIReceiveData, uint32_t len)
{
    uint8_t data[260];

  /* read page command */
    data[0] = S25FL_CM_READ;

  /* Send 24 bit address of sector starting from MSB
     *
     * Send 3rd byte of 24bit address
     *
     * */
    data[1] = ((address & 0x00FF0000)>>16);

    /*Send 24 bit address of sector starting from MSB
     *
     * Send 2nd byte of 24bit address
     *
     * */
    data[2] = ((address & 0x0000FF00)>>8);


    /*Send 24 bit address of sector starting from MSB
     *
     * Send 1st byte of 24bit address
     *
     * */
    data [3]=((address & 0x000000FF)>>0);

    Cy_SCB_SPI_ClearRxFifo(SPI_MEMORY_HW);

    spi_event = 0;

    /* Receive the 256 bytes */

    Cy_SCB_SPI_Transfer_Buffer(SPI_MEMORY_HW, data, pSPIReceiveData, 4,(len+4),0x00,&mSPI_context)  ;


    while(spi_event != CY_SCB_SPI_TRANSFER_CMPLT_EVENT);

}



/**
 * This function programs 256 bytes of Data to the
 * sector specified.
 *
 */
void  S25FL_ProgrammPage( uint32_t address,  uint8_t  *pSPISendData, uint32_t lentgh)
{

  uint8_t data[260];

  /* Write program page data */
  data[0] = S25FL_CM_PROG_PAGE;

  /* Send 24 bit address of sector starting from MSB
    *
    * Send 3rd byte of 24bit address
    *
    * */
   data[1] = ((address & 0x00FF0000)>>16);

   /*Send 24 bit address of sector starting from MSB
    *
    * Send 2nd byte of 24bit address
    *
    * */
   data[2] = ((address & 0x0000FF00)>>8);


   /*Send 24 bit address of sector starting from MSB
    *
    * Send 1st byte of 24bit address
    *
    * */
   data [3]=((address & 0x000000FF)>>0);

   memcpy(data+4,pSPISendData,256);

   spi_event = 0;

   Cy_SCB_SPI_Transfer(SPI_MEMORY_HW, data, NULL,lentgh, &mSPI_context);

   while(spi_event != CY_SCB_SPI_TRANSFER_CMPLT_EVENT);

}

uint8_t ProgramPage(uint32_t address,  uint8_t  *pSPISendData, uint32_t len)
{
  uint32_t tmp;
  /* Send write enable command for programming page */
  S25FL_WriteEnable();

  /* Read the status register value of SPI Flash chip */
  tmp = S25FL_StatusRead();

  /* Check the value of status register */
  if ((tmp != 0x0002)) /* not enabled */
    return 2; 	   /* not enabled */

  /* Program page */
  S25FL_ProgrammPage(address, pSPISendData, (len+4));

  /* wait till prog page is finished */
  do
  {
    tmp = S25FL_StatusRead();
  } while ((tmp & 0x0001)); 	  /* wait until busy=0 */

  return 1;
}

/**
 * This function erases the sector with start address 0x00000000
 */

uint8_t EraseSector()
{
  uint8_t tmp;

  /* Send write Enable data */
  S25FL_WriteEnable();

  /* Read Status registers */
  tmp = S25FL_StatusRead();
  if (tmp != 0x0002)  /* not enabled */
    return 2;

  /* Send sector erase command */
  S25FL_SectorErase( 0x00000000);

  /* Wait till sector erase is completed */
  do
  {
    tmp = S25FL_StatusRead();
  } while (tmp & 0x0001); 	  /* wait until busy=0 */

  return 1;
}



