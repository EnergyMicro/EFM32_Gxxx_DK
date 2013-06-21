/**************************************************************************//**
 * @file
 * @brief Potentiometer example for EFM32_Gxxx_DK
 * @details
 *   Control user LEDs with potentiometer on DVK.
 *
 * @par Usage
 * @li Use potentiometer to to see level indicated by LEDs
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
#include "em_system.h"
#include "bsp.h"
#include "bsp_trace.h"
#include "rtcdrv.h"

/* Defines according to DVK design */
#define POTENTIOMETER_PULLUP_OHM 10000
#define POTENTIOMETER_MAX_OHM    100000
#define POTENTIOMETER_VDD_mV     3300

/**************************************************************************//**
 * @brief  Main function
 *****************************************************************************/
int main(void)
{
  ADC_Init_TypeDef init = ADC_INIT_DEFAULT;
  ADC_InitSingle_TypeDef singleInit = ADC_INITSINGLE_DEFAULT;
  uint32_t sample;
  uint32_t vpot;
  uint32_t rpot;
  SYSTEM_ChipRevision_TypeDef chipRev;
  int errataShift = 0;

  /* Chip revision alignment and errata fixes */
  CHIP_Init();

  /* ADC errata for rev B when using VDD as reference, need to multiply */
  /* result by 2 */
  SYSTEM_ChipRevisionGet(&chipRev);
  if ((chipRev.major == 1) && (chipRev.minor == 1))
  {
    errataShift = 1;
  }

  /* Initialize DVK board register access */
  BSP_Init(BSP_INIT_DEFAULT);

  /* If first word of user data page is non-zero, enable eA Profiler trace */
  BSP_TraceProfilerSetup();

  /* Connect potentiometer to EFM32 (and ensure ambient light sensor */
  /* sharing same ADC channel is not enabled). */
  BSP_PeripheralAccess(BSP_AMBIENT, false);
  BSP_PeripheralAccess(BSP_POTMETER, true);

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
  singleInit.reference = adcRefVDD;
  singleInit.input = adcSingleInpCh5; /* According to DVK HW design */
  singleInit.resolution = adcRes8Bit; /* Use at least 8 bit since unlinear voltage */
  ADC_InitSingle(ADC0, &singleInit);

  /* Ensure internal reference settled */
  RTCDRV_Trigger(1, NULL);
  EMU_EnterEM2(true);

  /* Main loop - just check potentiometer and update LEDs */
  while (1)
  {
    ADC_IntClear(ADC0, ADC_IF_SINGLE);
    ADC_Start(ADC0, adcStartSingle);

    /* Wait for completion */
    while (!(ADC_IntGet(ADC0) & ADC_IF_SINGLE));
    sample = ADC_DataSingleGet(ADC0) << errataShift;

    /*
      DVK potentiometer design:

               | Vdd = 3.3V
              +-+
              | | Rpullup = 10kOhm
              +-+
               |
               +------> Vpot (to ADC)
               |
              +-+
              | | Rpot = 0-100kOhm
              +-+
               | Gnd

       Vpot = Rpot * Vdd / (Rpullup + Rpot)

       This gives a non-linear voltage level measured by ADC with respect to
       pot meter position. In order to determine the actual Rpot setting, which
       is linear with respect to position, rewrite the above formula to:

       Rpot = Rpullup * Vpot / (Vdd - Vpot)
    */

    /* Vpot sampled with 8 bit, divide by max value */
    vpot = (POTENTIOMETER_VDD_mV * sample) / 0xff;

    /* Calculate Rpot determining volume */
    rpot = (POTENTIOMETER_PULLUP_OHM * vpot) / (POTENTIOMETER_VDD_mV - vpot);
    /* The potentiometer may not be exact in upper range, make sure we don't use a higher */
    /* value than assumed by defines. */
    if (rpot > POTENTIOMETER_MAX_OHM)
    {
      rpot = POTENTIOMETER_MAX_OHM;
    }

    /* We have 16 LEDs, add half interval for improving rounding effects. */
    rpot += (POTENTIOMETER_MAX_OHM / (16 * 2));

    BSP_LedsSet((uint16_t)((1 << ((16 * rpot) / POTENTIOMETER_MAX_OHM)) - 1));

    /* Wait some time before polling again */
    RTCDRV_Trigger(100, NULL);
    EMU_EnterEM2(true);
  }
}
