/**************************************************************************//**
 * @file
 * @brief LCD controller and Energy Mode/RTC demo for  EFM32_G8xx_DK
 *  NOTE! This uses onbard segment LCD controller, and will not work on
 *        EFM32_G2xx_DK
 * @author Energy Micro AS
 * @version 3.20.0
 ******************************************************************************
 * @section License
 * <b>(C) Copyright 2012 Energy Micro AS, http://www.energymicro.com</b>
 *******************************************************************************
 *
 * Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and to alter it and redistribute it
 * freely, subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you must not
 *    claim that you wrote the original software.
 * 2. Altered source versions must be plainly marked as such, and must not be
 *    misrepresented as being the original software.
 * 3. This notice may not be removed or altered from any source distribution.
 * 4. The source and compiled code may only be used on Energy Micro "EFM32"
 *    microcontrollers and "EFR4" radios.
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
 *****************************************************************************/
#include <stdint.h>
#include <stdbool.h>
#include "em_device.h"
#include "em_cmu.h"
#include "em_chip.h"
#include "bsp.h"
#include "bsp_trace.h"
#include "rtcdrv.h"
#include "vddcheck.h"
#include "segmentlcd.h"
#include "lcdtest.h"

/**************************************************************************//**
 * @brief  Main function
 *****************************************************************************/
int main(void)
{
  bool vboost = false;

  /* Chip revision alignment and errata fixes */
  CHIP_Init();

  /* Initialize DVK board register access */
  BSP_Init(BSP_INIT_DEFAULT);

  /* If first word of user data page is non-zero, enable eA Profiler trace */
  BSP_TraceProfilerSetup();

  /* Initialize board specific registers */
  VDDCHECK_Init();

  /* Check if voltage is below 3V, if so use voltage boost */
  if (VDDCHECK_LowVoltage(2.9))
    vboost = true;

  /* Disable Voltage Comparator */
  VDDCHECK_Disable();

  /* Run Energy Mode with LCD demo, see lcdtest.c */
  SegmentLCD_Init(vboost);
  /* Display a message if vboost is enabled */
  if ( vboost )
  {
    SegmentLCD_Write("vboost");
    RTCDRV_Delay(5000, false);
  }
  Test();

  return 0;
}
