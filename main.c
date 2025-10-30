/*******************************************************************************
 * File Name        : main.c
 *
 * Description      : This source file contains the main routine for non-secure
 *                    application running on CM33 CPU.
 *
 * Related Document : See README.md
 *
 ********************************************************************************
 * Copyright 2024-2025, Cypress Semiconductor Corporation (an Infineon company) or
 * an affiliate of Cypress Semiconductor Corporation.  All rights reserved.
 *
 * This software, including source code, documentation and related
 * materials ("Software") is owned by Cypress Semiconductor Corporation
 * or one of its affiliates ("Cypress") and is protected by and subject to
 * worldwide patent protection (United States and foreign),
 * United States copyright laws and international treaty provisions.
 * Therefore, you may use this Software only as provided in the license
 * agreement accompanying the software package from which you
 * obtained this Software ("EULA").
 * If no EULA applies, Cypress hereby grants you a personal, non-exclusive,
 * non-transferable license to copy, modify, and compile the Software
 * source code solely for use in connection with Cypress's
 * integrated circuit products.  Any reproduction, modification, translation,
 * compilation, or representation of this Software except as specified
 * above is prohibited without the express written permission of Cypress.
 *
 * Disclaimer: THIS SOFTWARE IS PROVIDED AS-IS, WITH NO WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING, BUT NOT LIMITED TO, NONINFRINGEMENT, IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE. Cypress
 * reserves the right to make changes to the Software without notice. Cypress
 * does not assume any liability arising out of the application or use of the
 * Software or any product or circuit described in the Software. Cypress does
 * not authorize its products for use in any products where a malfunction or
 * failure of the Cypress product may reasonably be expected to result in
 * significant property damage, injury or death ("High Risk Product"). By
 * including Cypress's product in a High Risk Product, the manufacturer
 * of such system or application assumes all risk of such use and in doing
 * so agrees to indemnify Cypress against all liability.
 *******************************************************************************/

/*******************************************************************************
 * Header Files
 *******************************************************************************/
#include "cy_gpio.h"
#include "cy_pdl.h"
#include "cybsp.h"

#include "FreeRTOS.h"
#include "cycfg_pins.h"
#include "task.h"
#include "cyabs_rtos.h"
#include "cyabs_rtos_impl.h"
#include "cy_tcpwm_pwm.h"

#include "application/app.h"
#include "4G.h"
#include "power.h"
#include "cy_syslib.h"
#include <stdint.h>

/*******************************************************************************
 * Macros
 ******************************************************************************/
/*******************************************************************************
 * Macros
 *******************************************************************************/
#define GPIO_INTERRUPT_PRIORITY 7

/*******************************************************************************
* Global Variables
*******************************************************************************/
cy_stc_sysint_t intrCfgBTN =
{
    .intrSrc = ioss_interrupts_sec_gpio_5_IRQn, /* Interrupt source is GPIO port 5 interrupt */
    .intrPriority = GPIO_INTERRUPT_PRIORITY /* Interrupt priority is 7 */
};

/*******************************************************************************
 * Global Variables
 *******************************************************************************/
extern lte_t lte;
uint8_t state_button = 0;


/*******************************************************************************
 * Function Prototypes
 *******************************************************************************/
static void BTN_interrupt_handler_PDL();


/*******************************************************************************
 * Function Name: main
 ********************************************************************************
 * This is the main function. It creates two tasks, initializes the semaphore
 *  for synchronization between tasks, and starts the FreeRTOS scheduler.
 *
 *
 * Parameters:
 *  void
 *
 * Return:
 *  int
 *
 *******************************************************************************/

int main(void)
{
    cy_rslt_t result;

    /* Initialize the device and board peripherals */
    result = cybsp_init();
    /* Board init failed. Stop program execution */
    if (result != CY_RSLT_SUCCESS)
    {
        CY_ASSERT(0);
    }

    Cy_GPIO_Write(LED1_PORT, LED1_PIN, 0);
    Cy_GPIO_Write(CTRL_AUXILIAR_PORT, CTRL_AUXILIAR_PIN,1);
    /* Enable global interrupts */
    __enable_irq();

	/*Init and start PWM: 2HZ and 50% duty-cycle*/
    if (CY_TCPWM_SUCCESS != Cy_TCPWM_PWM_Init(BAT_CHARGE_PWM_HW, BAT_CHARGE_PWM_NUM, &BAT_CHARGE_PWM_config))
    {
        CY_ASSERT(0);
    }

	/* Enable the initialized PWM */
 	Cy_TCPWM_PWM_Enable(BAT_CHARGE_PWM_HW, BAT_CHARGE_PWM_NUM);

    Cy_GPIO_Pin_SecFastInit(BUTTON_PORT, BUTTON_PIN, CY_GPIO_DM_PULLUP, 1UL, HSIOM_SEL_GPIO);

    Cy_SysInt_Init(&intrCfgBTN, BTN_interrupt_handler_PDL);
    NVIC_EnableIRQ(ioss_interrupts_sec_gpio_5_IRQn);

    opPDL_init();


    /* Start the scheduler */
    vTaskStartScheduler();
    for(;;)
    {
        /* vTaskStartScheduler never returns */
    }

}

/*******************************************************************************
* Function Name: GPIO_Interrupt_handler_PDL
********************************************************************************
*
*  Summary:
*  GPIO interrupt handler for the PDL example.
*
*  Parameters:
*  None
*
*  Return:
*  None
*
**********************************************************************************/
static void BTN_interrupt_handler_PDL()
{
	volatile uint32_t test_time;
    /* Clear pin interrupt logic. Required to detect next interrupt */
    Cy_GPIO_ClearInterrupt(BUTTON_PORT, BUTTON_PIN);
//    Cy_GPIO_Inv(SIRENE_ON_PORT, SIRENE_ON_PIN);
    //alarm_setAlarm_os(&app.app_event, APP_EVENT_LTE, 1000, ALARM_MODE_ONESHOT);
    //PowerOffA7672();
    //test_time = rtc_get_timestamp();
    alarm_setAlarm_os(&app.app_event, APP_EVENT_BUTTON, 1, ALARM_MODE_ONESHOT);
//	cy_rtos_event_setbits(&app.app_event,APP_EVENT_BUTTON);
}

/* [] END OF FILE */

