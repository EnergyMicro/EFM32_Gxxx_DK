/***************************************************************************//**
 * @file
 * @brief Accelerometer example for EFM32_Gxxx_DK
 * @details
 *   Use ADC/DMA in order to read accelerometer. Only one (X) axis used to
 *   indicate tilt on board, using leds.
 *
 * @par Usage
 *   Hold the DK in front, with display in normal viewing position. Tilt
 *   DK left/right to move user LEDs.
 *
 * @author Energy Micro AS
 * @version 3.20.0
 *******************************************************************************
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
#include "bsp.h"
#include "bsp_trace.h"
#include "em_chip.h"
#include "em_emu.h"
#include "em_cmu.h"
#include "em_adc.h"
#include "em_dma.h"
#include "rtcdrv.h"
#include "dmactrl.h"

/*******************************************************************************
 *******************************   DEFINES   ***********************************
 ******************************************************************************/

/** DMA channel used for scan sequence sampling accelerometer. */
#define ACCEL_DMA_CHANNEL           0


/*******************************************************************************
 ***************************   LOCAL FUNCTIONS   *******************************
 ******************************************************************************/

/***************************************************************************//**
 * @brief
 *   Configure ADC usage for this application.
 *******************************************************************************/
static void accelADCConfig(void)
{
  ADC_Init_TypeDef init = ADC_INIT_DEFAULT;
  ADC_InitScan_TypeDef scanInit = ADC_INITSCAN_DEFAULT;

  /* Init common issues for both single conversion and scan mode */
  init.timebase = ADC_TimebaseCalc(0);
  init.prescale = ADC_PrescaleCalc(4000000, 0);
  ADC_Init(ADC0, &init);

  /* Init for scan sequence use (accelerometer X, Y and Z axis). */
  scanInit.reference = adcRefVDD;
  scanInit.input = ADC_SCANCTRL_INPUTMASK_CH2 |
                   ADC_SCANCTRL_INPUTMASK_CH3 |
                   ADC_SCANCTRL_INPUTMASK_CH4;
  ADC_InitScan(ADC0, &scanInit);
}


/***************************************************************************//**
 * @brief
 *   Configure DMA usage for this application.
 *******************************************************************************/
static void accelDMAConfig(void)
{
  DMA_Init_TypeDef dmaInit;
  DMA_CfgDescr_TypeDef descrCfg;
  DMA_CfgChannel_TypeDef chnlCfg;

  /* Configure general DMA issues */
  dmaInit.hprot = 0;
  dmaInit.controlBlock = dmaControlBlock;
  DMA_Init(&dmaInit);

  /* Configure DMA channel used */
  chnlCfg.highPri = false;
  chnlCfg.enableInt = false;
  chnlCfg.select = DMAREQ_ADC0_SCAN;
  chnlCfg.cb = NULL;
  DMA_CfgChannel(ACCEL_DMA_CHANNEL, &chnlCfg);

  descrCfg.dstInc = dmaDataInc4;
  descrCfg.srcInc = dmaDataIncNone;
  descrCfg.size = dmaDataSize4;
  descrCfg.arbRate = dmaArbitrate1;
  descrCfg.hprot = 0;
  DMA_CfgDescr(ACCEL_DMA_CHANNEL, true, &descrCfg);
}


/*******************************************************************************
 **************************   GLOBAL FUNCTIONS   *******************************
 ******************************************************************************/

int main(void)
{
  /** Number of samples/channels taken from accelerometer. */
  #define ACCEL_SAMPLES               3

  /** X axis sample index. */
  #define ACCEL_X                     0
  /** Y axis sample index. */
  #define ACCEL_Y                     1
  /** Z axis sample index. */
  #define ACCEL_Z                     2

  /*
   * Tilt levels: Midpoint is theoretically half value of max sampling value
   * (ie 0x800 for 12 bit sampling). In real world, some sort of calibration
   * is required if more accurate sensing is required. We just use set some
   * fixed limit, that should be sufficient for this basic example.
   */

  /** Tilt left limit */
  #define TILT_LEFT                   0x750
  /** Tilt right limit */
  #define TILT_RIGHT                  0x8b0

  SYSTEM_ChipRevision_TypeDef chipRev;
  uint32_t leds;
  uint32_t samples[ACCEL_SAMPLES];
  int errataShift = 0;
  int i;

  /* Chip revision alignment and errata fixes */
  CHIP_Init();

  /* ADC errata for rev B when using VDD as reference, need to multiply */
  /* result by 2 */
  SYSTEM_ChipRevisionGet(&chipRev);
  if ((chipRev.major == 1) && (chipRev.minor == 1))
  {
    errataShift = 1;
  }

  /* Initialize DK board register access */
  BSP_Init(BSP_INIT_DEFAULT);

  /* If first word of user data page is non-zero, enable eA Profiler trace */
  BSP_TraceProfilerSetup();

  /* Connect accelerometer to EFM32. */
  BSP_PeripheralAccess(BSP_ACCEL, true);

  /* Enable clocks required */
  CMU_ClockEnable(cmuClock_HFPER, true);
  CMU_ClockEnable(cmuClock_ADC0, true);
  CMU_ClockEnable(cmuClock_DMA, true);

  /* Configure ADC and DMA used for scanning accelerometer */
  accelADCConfig();
  accelDMAConfig();

  /* Main loop, keep polling accelerometer */
  leds = 0x0180;
  while (1)
  {
    DMA_ActivateBasic(ACCEL_DMA_CHANNEL,
                      true,
                      false,
                      samples,
                      (void *)((uint32_t)&(ADC0->SCANDATA)),
                      ACCEL_SAMPLES - 1);

    /* Scan all axis', even though this app only use the X axis */
    ADC_IntClear(ADC0, ADC_IF_SCAN);
    ADC_Start(ADC0, adcStartScan);

    /* Poll for completion, entering EM2 when waiting for next poll */
    while (!(ADC_IntGet(ADC0) & ADC_IF_SCAN))
    {
      RTCDRV_Trigger(5, NULL);
      EMU_EnterEM2(true);
    }

    if (errataShift)
    {
      for (i = 0; i < ACCEL_SAMPLES; i++)
      {
        samples[i] <<= errataShift;
      }
    }

    if (samples[ACCEL_X] < TILT_LEFT)
    {
      if (leds < 0xc000)
      {
        leds <<= 1;
      }
    }
    else if (samples[ACCEL_X] > TILT_RIGHT)
    {
      if (leds > 0x0003)
      {
        leds >>= 1;
      }
    }
    else
    {
      if (leds > 0x0180)
      {
        leds >>= 1;
      }
      else if (leds < 0x0180)
      {
        leds <<= 1;
      }
    }

    BSP_LedsSet((uint16_t)leds);
    /* Delay to slow down led movement */
    RTCDRV_Trigger(70, NULL);
    EMU_EnterEM2(true);
  }
}
