/***************************************************************************//**
 * @file
 * @brief uC/OS-III example - Board Support Package (BSP) header
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
#ifndef  __BSPOS_PRESENT_H
#define  __BSPOS_PRESENT_H

#ifdef __cplusplus
extern "C" {
#endif

/*******************************************************************************
 *****************************   INCLUDE FILES   *******************************
 ******************************************************************************/


/*******************************************************************************
 **************************   FUNCTION PROTOTYPES   ****************************
 ******************************************************************************/
void BSPOS_Init(void);
CPU_INT32U OS_CPU_SysTickClkFreq (void);

#ifdef __cplusplus
}
#endif

#endif  /* end of __BSPOS_PRESENT_H */
