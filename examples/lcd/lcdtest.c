/**************************************************************************//**
 * @file
 * @brief LCD test/demo routines
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
#include <string.h>
#include <stdlib.h>
#include "em_device.h"
#include "em_lcd.h"
#include "segmentlcd.h"
#include "lcdtest.h"

/** SysTick polled Delay routine */
extern void Delay(uint32_t dlyTicks);

/** Demo scroll text */
static char *stext = "Welcome to Energy Micro EFM32        ";

/**************************************************************************//**
 * @brief LCD scrolls a text over the display, sort of "polled printf"
 *****************************************************************************/
void ScrollText(char *scrolltext)
{
  int  i, len;
  char buffer[8];

  buffer[7] = 0x00;
  len       = strlen(scrolltext);
  if (len < 7) return;
  for (i = 0; i < (len - 7); i++)
  {
    memcpy(buffer, scrolltext + i, 7);
    SegmentLCD_Write(buffer);
    Delay(125);
  }
}


/**************************************************************************//**
 * @brief LCD Blink Test
 *****************************************************************************/
void BlinkTest(void)
{
  SegmentLCD_EnergyMode(0, 1);
  SegmentLCD_EnergyMode(1, 1);
  SegmentLCD_EnergyMode(2, 1);
  SegmentLCD_EnergyMode(3, 1);
  SegmentLCD_EnergyMode(4, 1);
  SegmentLCD_Number(2359);
  SegmentLCD_Symbol(LCD_SYMBOL_COL10, 1);
  SegmentLCD_Symbol(LCD_SYMBOL_GECKO, 1);
  SegmentLCD_Symbol(LCD_SYMBOL_EFM32, 1);
  SegmentLCD_Write(" EFM32 ");

  LCD_BlinkEnable(true);

  Delay(2000);
  SegmentLCD_EnergyMode(4, 0);
  Delay(62);
  SegmentLCD_EnergyMode(3, 0);
  Delay(62);
  SegmentLCD_EnergyMode(2, 0);
  Delay(62);
  SegmentLCD_EnergyMode(1, 0);
  Delay(62);
  SegmentLCD_EnergyMode(0, 0);

  LCD_BlinkEnable(false);
}

/**************************************************************************//**
 * @brief LCD Test Routine, shows various text and patterns
 *****************************************************************************/
void Test(void)
{
  int i, numberOfIterations = 1000;

  /* Exercise for the reader: Go to EM2 here and on all delays... */
  while (--numberOfIterations)
  {
    SegmentLCD_AllOff();
    for (i = 100; i > 0; i--)
    {
      SegmentLCD_Number(i);
      Delay(10);
    }
    SegmentLCD_NumberOff();

    SegmentLCD_Symbol(LCD_SYMBOL_GECKO, 1);
    SegmentLCD_Symbol(LCD_SYMBOL_EFM32, 1);
    SegmentLCD_Write(" Gecko ");
    Delay(1000);

    SegmentLCD_AllOn();
    Delay(1000);

    SegmentLCD_AllOff();
    SegmentLCD_Write("OOOOOOO");
    Delay(62);
    SegmentLCD_Write("XXXXXXX");
    Delay(62);
    SegmentLCD_Write("+++++++");
    Delay(62);
    SegmentLCD_Write("@@@@@@@");
    Delay(62);
    SegmentLCD_Write("ENERGY ");
    Delay(250);
    SegmentLCD_Write("@@ERGY ");
    Delay(62);
    SegmentLCD_Write(" @@RGY ");
    Delay(62);
    SegmentLCD_Write(" M@@GY ");
    Delay(62);
    SegmentLCD_Write(" MI@@Y ");
    Delay(62);
    SegmentLCD_Write(" MIC@@ ");
    Delay(62);
    SegmentLCD_Write(" MICR@@");
    Delay(62);
    SegmentLCD_Write(" MICRO@");
    Delay(62);
    SegmentLCD_Write(" MICRO ");
    Delay(250);
    SegmentLCD_Write("-EFM32-");
    Delay(250);

    /* Various eye candy */
    SegmentLCD_AllOff();
    for (i = 0; i < 8; i++)
    {
      SegmentLCD_Number(numberOfIterations + i);
      SegmentLCD_ARing(i, 1);
      Delay(20);
    }
    for (i = 0; i < 8; i++)
    {
      SegmentLCD_Number(numberOfIterations + i);
      SegmentLCD_ARing(i, 0);
      Delay(100);
    }

    for (i = 0; i < 5; i++)
    {
      SegmentLCD_Number(numberOfIterations + i);
      SegmentLCD_Battery(i);
      SegmentLCD_EnergyMode(i, 1);
      Delay(100);
      SegmentLCD_EnergyMode(i, 0);
      Delay(100);
    }
    SegmentLCD_Symbol(LCD_SYMBOL_ANT, 1);
    for (i = 0; i < 4; i++)
    {
      SegmentLCD_EnergyMode(i, 1);
      Delay(100);
    }
    /* Scroll text */
    ScrollText(stext);

    /* Blink test*/
    BlinkTest(); \
  }
  /* Segment bits */
  SegmentLCD_AllOff();
  SegmentLCD_Disable();
}

