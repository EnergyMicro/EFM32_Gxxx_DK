/**************************************************************************//**
 * @file
 * @brief Ambient light sensor example for EFM32_Gxxx_DK
 * @details
 *   Control user LEDs with ambient light sensor on DK.
 *
 * @note
 *   This example requires BSP version 1.0.6 or later.
 *
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
#include <string.h>
#include <stdlib.h>
#include "em_device.h"
#include "em_chip.h"
#include "em_emu.h"
#include "em_cmu.h"
#include "em_adc.h"
#include "bsp.h"
#include "bsp_trace.h"
#include "rtcdrv.h"

/**************************************************************************//**
 * @brief  Main function
 *****************************************************************************/
int main(void)
{
  ADC_Init_TypeDef init = ADC_INIT_DEFAULT;
  ADC_InitSingle_TypeDef singleInit = ADC_INITSINGLE_DEFAULT;
  uint32_t sample;

  /* Chip revision alignment and errata fixes */
  CHIP_Init();

  /* Initialize DK board register access */
  BSP_Init(BSP_INIT_DEFAULT);

  /* If first word of user data page is non-zero, enable eA Profiler trace */
  BSP_TraceProfilerSetup();

  /* Connect ambient light sensor to EFM32 (and ensure potmeter */
  /* sharing same ADC channel is not enabled). */
  BSP_PeripheralAccess(BSP_POTMETER, false);
  BSP_PeripheralAccess(BSP_AMBIENT, true);

  /* Enable clocks required */
  CMU_ClockEnable(cmuClock_HFPER, true);
  CMU_ClockEnable(cmuClock_ADC0, true);

  /* Init common issues for both single conversion and scan mode */
  init.timebase = ADC_TimebaseCalc(0);
  /* Might as well finish conversion as quickly as possibly since polling */
  /* for completion. */
  init.prescale = ADC_PrescaleCalc(7000000, 0);
  /* WARMUPMODE must be set to Normal according to ref manual before */
  /* entering EM2. In this example, the warmup time is not a big problem */
  /* due to relatively infrequent polling. Leave at default NORMAL, */
  ADC_Init(ADC0, &init);

  /* Init for single conversion use. */
  singleInit.reference = adcRef2V5;
  singleInit.input = adcSingleInpCh5; /* According to DK HW design */
  singleInit.resolution = adcRes6Bit; /* 6 bit sufficient for 16 LEDs */
  ADC_InitSingle(ADC0, &singleInit);

  /* Ensure internal reference settled */
  RTCDRV_Trigger(1, NULL);
  EMU_EnterEM2(true);

  /* Main loop - just check potmeter and update LEDs */
  while (1)
  {
    ADC_IntClear(ADC0, ADC_IF_SINGLE);
    ADC_Start(ADC0, adcStartSingle);

    /* Wait for completion */
    while (!(ADC_IntGet(ADC0) & ADC_IF_SINGLE));
    sample = ADC_DataSingleGet(ADC0);

    /* Light sensor provides voltage in range 0.1-2V, whereas reference */
    /* used is 2.5V. So, scale up sample value somewhat. */
    sample *= 5;
    sample /= 4;
    BSP_LedsSet(((uint16_t)1 << ((sample * 16) / 0x3f)) - 1);

    /* Wait some time before polling again */
    RTCDRV_Trigger(200, NULL);
    EMU_EnterEM2(true);
  }
}
