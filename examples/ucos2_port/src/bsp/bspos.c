/***************************************************************************//**
 * @file
 * @brief uC/OS-II example - Board Support Package (BSP)
 * @author Energy Micro AS
 * @version 3.20.0
 *******************************************************************************
 * @section License
 * <b>(C) Copyright 2010 Energy Micro AS, http://www.energymicro.com</b>
 *******************************************************************************
 *
 * This source code is the property of Energy Micro AS. The source and compiled
 * code may only be used on Energy Micro "EFM32" microcontrollers.
 *
 * This copyright notice may not be removed from the source code nor changed.
 *
 * DISCLAIMER OF WARRANTY/LIMITATION OF REMEDIES: Energy Micro AS has no
 * obligation to support this Software. Energy Micro AS is providing the
 * Software "AS IS", with no express or implied warranties of any kind,
 * including, but not limited to, any implied warranties of merchantability
 * or fitness for any particular purpose or warranties against infringement
 * of any proprietary rights of a third party.
 *
 * Energy Micro AS will not be liable for any consequential, incidental, or
 * special damages, or any other relief, or for any claim by any third party,
 * arising from your use of this Software.
 *
 ******************************************************************************/
#include <includes.h>


/***************************************************************************//**
 *                                BSPOS_Init()
 * @brief      Board Support Package Initialization.
 *
 * @param[in]  none
 * @exception  none
 * @return     none
 *
 ******************************************************************************/
void  BSPOS_Init (void)
{
  BSP_Init(BSP_INIT_DEFAULT);
  BSP_LedsSet( 0 );

  /* Set external crystal oscillator */
  CMU_ClockSelectSet(cmuClock_HF, cmuSelect_HFXO);

  /* Enable module clocks */
  CMU_ClockEnable(cmuClock_GPIO, true);
  CMU_ClockEnable(cmuClock_HFPER, true);
}
