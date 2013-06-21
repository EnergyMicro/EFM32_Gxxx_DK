/**************************************************************************//**
 * @file main.c
 * @brief main code for applications for IEC60335 verification on EFM32 devices.
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
#include "em_device.h"
#include "iec60335_class_b.h"
#include "main.h"
#include "em_cmu.h"

/*! @addtogroup IEC60335_EXAMPLE
* @{
*/

IRQTestData_t IRQtest;
FlashCRC_t bistTest;

extern ClockTest_t ClockTest;               /*!< RTC/Timer-test structure */
extern IRQTestData_t IRQtest;               /*!< IRQ-test structure */

/*!
 * @brief SysTick_Handler
 * Interrupt Service Routine for system tick counter.
 */
void SysTick_Handler(void)
{
  IEC60335_ClassB_Clocktest_TimerHandler();
}

/*!
 * @brief TIMER0_IRQHandler
 * Interrupt Service Routine for timer0.
 */
void TIMER0_IRQHandler(void)
{
  /* Clear Interrupt */
  TIMER0->IFC = TIMER_IFC_OF;
  IEC60335_ClassB_Clocktest_TimerHandler();
}

void initSystick(uint32_t mode)
{
  if (mode == 1) // enable
  {
    /* Setup SysTick Timer for 10 msec interrupts  */
    /* set reload register */
    SysTick->LOAD = ((14000000 / 100) & SysTick_LOAD_RELOAD_Msk) - 1;
    /* set Priority for Cortex-M3 System Interrupts */
    SCB->SHP[((uint32_t)(SysTick_IRQn) & 0xF) - 4] = ((((1 << __NVIC_PRIO_BITS) - 1) << (8 - __NVIC_PRIO_BITS)) & 0xff);
    /* Load the SysTick Counter Value */
    SysTick->VAL = 0;
    /* Enable SysTick IRQ and SysTick Timer */
    SysTick->CTRL = SysTick_CTRL_CLKSOURCE_Msk | SysTick_CTRL_TICKINT_Msk | SysTick_CTRL_ENABLE_Msk;
  }
  if (mode == 0) // disable
  {
    /* set reload register */
    SysTick->LOAD = 0;
    /* Load the SysTick Counter Value */
    SysTick->VAL = 0;
    /* Enable SysTick IRQ and SysTick Timer */
    SysTick->CTRL = 0;
  }
}

/*!
 * @brief short basic initialization to enable generic controller functions
 * @note user initialization can vary here
 */
void initController(void)
{
  /* Enable required clock domains */
  CMU_ClockEnable(cmuClock_HFPER, true);
  /* TODO : user initialization here */
}

/*!
 * @brief main routine testing the library content
 * @note POST tests are located in the startup files
 */
int main (void)
{
  testResult_t	result = IEC60335_testFailed;                 /* test results */
/* critical data */
  critical_uint32_t Crit_test = IEC60335_ClassB_CriticalDataInit(12345678);
/* hardware setup */
  initController();

/* critical data */
  IEC60335_ClassB_CriticalDataPush(Crit_test, 0x87654321);    /* regular usage */
  result = IEC60335_ClassB_CriticalDataValidate(Crit_test);

/* not allowed by compiler */
// Crit_test = 34343434;
  result = IEC60335_ClassB_CriticalDataValidate(Crit_test);

  Crit_test.data = 89898989;                                  /* not allowed in runtime */
  result = IEC60335_ClassB_CriticalDataValidate(Crit_test);

  result = IEC60335_ClassB_FLASHtest_POST();

/* clock/rtc test */
  IEC60335_ClassB_initClockTest(100, 5);                      /* init clock test */
  result = IEC60335_ClassB_Clocktest_PollHandler();           /* check evidence */

/* IRQ test */
  IRQtest.MaxThres = 1000;                                    /* IRQ test parameter */
  IRQtest.MinThres = 10;
  initSystick(1);
  IEC60335_ClassB_InitInterruptTest(SysTick_IRQn, CallIRQHandler, &IRQtest);
  while (IEC60335_ClassB_InterruptCheck(SysTick_IRQn) != IEC60335_testPassed)
  {}
/* IRQ test off */
  IEC60335_ClassB_InitInterruptTest(SVCall_IRQn, 0, 0);
  initSystick(0);

/* Flash test */
  result = IEC60335_ClassB_FLASHtest_BIST (FLASH_CRC_Restart);

  while (1)
  {
    result = IEC60335_ClassB_CPUregTest_BIST();

    result = IEC60335_ClassB_FLASHtest_BIST (0);

    result = IEC60335_ClassB_PCTest_BIST();

    result = IEC60335_ClassB_RAMtest_BIST (0x20000100, 0x10);

    if (result != IEC60335_testPassed)
    {
      /* Break point here to test with debugger */
      __NOP();
    }
  }
}
/*!
 * @}
*/
/************************************** EOF *********************************/
