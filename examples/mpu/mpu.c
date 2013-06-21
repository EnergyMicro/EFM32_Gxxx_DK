/**************************************************************************//**
 * @file
 * @brief MPU example for EFM32_G2xx_DK and EFM32_G8xx_DK.
 *        Connect a terminal application with baudrate 9600-8-N-1
 *        on serial port B of the kit to run the demo.
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
#include <stdio.h>
#include <inttypes.h>
#include "em_device.h"
#include "em_chip.h"
#include "em_emu.h"
#include "em_mpu.h"
#include "bsp.h"
#include "bsp_trace.h"
#include "retargetserial.h"

/***************************************************************************//**
 *
 *  This example project demonstrate use of the memory protection unit (MPU).
 *  The cpu will normally run in Priviledged state, but upon reception
 *  of a lowercase 'x' on the UART it will switch into User state and
 *  intentionally generate two access violation exceptions.
 *
 *  The first violation is by accessing the UART, the second by accessing
 *  internal SRAM.
 *  To be able to execute in User state at all, User state access must be
 *  granted to flash and SRAM memory. We use two MPU regions for this.
 *  Additionally one MPU section is used to deny User state access to a small
 *  part of SRAM. This section overlaps the section used to grant general
 *  general SRAM access, but it will take precedence because its section number
 *  is higher.
 *  A fourth MPU section is used to deny User state access to UART registers.
 *
 *  Entry into User state is performed by writing to the CONTROL cpu core
 *  register. Before exiting the memory protection fault handler the cpu
 *  is switched back to Priviledged state. (It is impossible to switch from
 *  User state to Priviledged state in Thread mode).
 *
 ******************************************************************************/


/**************************************************************************//**
 * @brief  Hard fault exception handler.
 *****************************************************************************/
void HardFault_Handler( void )    /* We should never end up here !           */
{
  static uint32_t shcsr, hfsr, bfar;

  bfar  = SCB->BFAR;            /* Bus fault address register                */
  hfsr  = SCB->HFSR;            /* Hard fault status register                */
  shcsr = SCB->SHCSR;           /* System Handler Control and State Register */

  printf( "\nHard fault !\n"
            "  System Control Block (SCB) registers: \n"
            "    SCB->SHCSR = 0x%"PRIX32"\n"
            "    SCB->HFSR  = 0x%"PRIX32"\n"
            "    SCB->BFAR  = 0x%"PRIX32"\n",
          shcsr, hfsr, bfar );

  for(;;);
}


/**************************************************************************//**
 * @brief  Memory protection fault first level exception handler.
 *****************************************************************************/
#ifdef __CC_ARM  /* MDK-ARM compiler */

__asm void MemManage_Handler( void )
{
  EXTERN MemManage_HandlerC
  TST   LR, #4
  ITE   EQ
  MRSEQ R0, MSP
  MRSNE R0, PSP
  B MemManage_HandlerC
}
#else

#if defined(__GNUC__)
void MemManage_Handler( void ) __attribute__ ((naked));
#endif
void MemManage_Handler( void )
{
  /*
   * Get the appropriate stack pointer, depending on our mode,
   * and use it as a parameter to a C handler.
   */
  __asm("TST   LR, #4");
  __asm("ITE   EQ");
  __asm("MRSEQ R0, MSP");
  __asm("MRSNE R0, PSP");
  __asm("B MemManage_HandlerC");
}
#endif

/**************************************************************************//**
 * @brief  Memory protection fault second level exception handler.
 *****************************************************************************/
void MemManage_HandlerC( uint32_t *stack )
{
  static uint32_t mmfar, pc, shcsr, cfsr;

  mmfar = SCB->MMFAR;           /* Memory Management Fault Address Register  */
  cfsr  = SCB->CFSR;            /* Configurable Fault Status Register        */
  shcsr = SCB->SHCSR;           /* System Handler Control and State Register */
  pc = stack[6];                /* Get stacked return address                */

  printf( "\nMPU fault !\n"
            "  Violation memory address  : 0x%"PRIX32"\n"
            "  Violation program counter : 0x%"PRIX32"\n"
            "  System Control Block (SCB) registers: \n"
            "    SCB->SHCSR = 0x%"PRIX32"\n"
            "    SCB->CFSR  = 0x%"PRIX32"\n"
            "    SCB->MMFAR = 0x%"PRIX32"\n",
          mmfar, pc, shcsr, cfsr, mmfar );

  SCB->CFSR |= 0xFF;              /* Clear all status bits in the            */
                                  /* MMFSR part of CFSR                      */
  __set_CONTROL( 0 );             /* Enter Priviledged state before exit     */
}


/**************************************************************************//**
 * @brief  Main function
 *****************************************************************************/
int main(void)
{
  int c;
  MPU_RegionInit_TypeDef flashInit       = MPU_INIT_FLASH_DEFAULT;
  MPU_RegionInit_TypeDef sramInit        = MPU_INIT_SRAM_DEFAULT;
  MPU_RegionInit_TypeDef peripheralInit  = MPU_INIT_PERIPHERAL_DEFAULT;

  /* Chip revision alignment and errata fixes */
  CHIP_Init();

  /* Initialize DVK board register access */
  BSP_Init(BSP_INIT_DEFAULT);

  /* If first word of user data page is non-zero, enable eA Profiler trace */
  BSP_TraceProfilerSetup();

  /* Enable printf on RS232 port - this example only supports LEUART */
  RETARGET_SerialInit();
  RETARGET_SerialCrLf(1);

  printf("\nEFM32 MPU access violation example.\n"
           "Hit lowercase 'x' to force access violations.\n");

  MPU_Disable();

  /* Flash memory */
  MPU_ConfigureRegion( &flashInit );

  /* SRAM */
  MPU_ConfigureRegion( &sramInit );

  /* SRAM, a 4k part with priviledged only access, this regions settings  */
  /* will override those of the previous region                           */
  sramInit.regionNo         = 2;
  sramInit.baseAddress      = RAM_MEM_BASE + 0x2000;
  sramInit.size             = mpuRegionSize4Kb;
  sramInit.accessPermission = mpuRegionApPRw;
  MPU_ConfigureRegion( &sramInit );

  /* LEUART, priviledged only access */
  peripheralInit.regionNo         = 3;
  peripheralInit.baseAddress      = LEUART1_BASE;
  peripheralInit.size             = mpuRegionSize128b;
  peripheralInit.accessPermission = mpuRegionApPRw;
  MPU_ConfigureRegion( &peripheralInit );

  MPU_Enable( MPU_CTRL_PRIVDEFENA ); /* Full access to default memory map */
                                     /* in priviledged state              */

  while (1)
  {
    EMU_EnterEM2(true);

    /* Retrieve new character */
    c = getchar();
    if (c > 0)
    {
      if ( c == 'x' )
      {
        /* Generate an access violation in LEUART1 peripheral     */
        __set_CONTROL( 1 );   /* Enter User (unpriviledged) state */
        putchar( c );

        /* MemManage_Handler() will set back to priviledged state */

        /* Generate an access violation in internal SRAM          */
        __set_CONTROL( 1 );   /* Enter User (unpriviledged) state */
        *(volatile uint32_t *)(RAM_MEM_BASE + 0x2000) = 1;

        /* MemManage_Handler() will set back to priviledged state */
      }
    else
      {
      /* Echo character */
      putchar(c);
      }
    }

    /* Most terminals issue CR when pressing enter, add LF */
    if (c == '\r')
    {
      putchar('\n');
    }
  }
}
