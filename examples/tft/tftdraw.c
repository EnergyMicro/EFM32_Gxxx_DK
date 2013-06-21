/**************************************************************************//**
 * @file
 * @brief TFT address mapped example application
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
#include <stdint.h>
#include "em_device.h"
#include "bsp.h"

#include "glib/glib.h"
#include "glib/glib_color.h"
#include "glib/glib_font.h"
#include "dmd/ssd2119/dmd_ssd2119.h"

#include "tftdraw.h"

/** Graphics context */
GLIB_Context gc;

/**************************************************************************//**
 * @brief Really simple and silly random number generator
 * @param limit Upper limit of return value
 *****************************************************************************/
static int randomGenerator(int limit)
{
  static uint32_t rnum = 0xabcd7381;

  rnum = ((rnum * 27) << 8) | (((rnum / 13) & 0xa5a7f196) + (rnum >> 13));

  return(rnum % limit);
}


/**************************************************************************//**
 * @brief Clears/updates entire background ready to be drawn
 *****************************************************************************/
void TFT_displayUpdate(void)
{
  int            i, tmp;
  GLIB_Rectangle rect = {
    .xMin =   0,
    .yMin =   0,
    .xMax = 319,
    .yMax = 239,
  };
  /* Set clipping region to entire image */
  GLIB_setClippingRegion(&gc, &rect);
  GLIB_resetDisplayClippingArea(&gc);

  /* Generate "wild" rectangle pattern  */
  for (i = 0; i < 20; i++)
  {
    rect.xMin = randomGenerator(320);
    rect.xMax = randomGenerator(320);
    rect.yMin = randomGenerator(230) + 10;
    rect.yMax = randomGenerator(230) + 10;
    if (rect.xMin > rect.xMax)
    {
      tmp       = rect.xMin;
      rect.xMin = rect.xMax;
      rect.xMax = tmp;
    }
    if (rect.yMin > rect.yMax)
    {
      tmp       = rect.yMin;
      rect.yMin = rect.yMax;
      rect.yMax = tmp;
    }
    gc.foregroundColor = GLIB_rgbColor(128 + randomGenerator(127),
                                       randomGenerator(200),
                                       randomGenerator(255));
    GLIB_drawRectFilled(&gc, &rect);
  }
}

/**************************************************************************//**
 * @brief Initialize viewer
 *****************************************************************************/
void TFT_init(void)
{
  static char    *efm32_hello = "EFM32 @ 32MHz / SSD2119 TFT demo\n";
  EMSTATUS       status;
  GLIB_Rectangle rect = {
    .xMin =   0,
    .yMin =   0,
    .xMax = 319,
    .yMax = 239,
  };

  /* Initialize graphics - abort on failure */
  status = DMD_init(BC_SSD2119_BASE, BC_SSD2119_BASE + 2);
  if ((status != DMD_OK) && (status != DMD_ERROR_DRIVER_ALREADY_INITIALIZED)) while (1) ;

  /* Make sure display is configured with correct rotation */
  if ((status == DMD_OK)) DMD_flipDisplay(1,1);

  /* Init graphics context - abort on failure */
  status = GLIB_contextInit(&gc);
  if (status != GLIB_OK) while (1) ;

  /* Clear framebuffer */
  gc.foregroundColor = GLIB_rgbColor(20, 40, 20);
  GLIB_drawRectFilled(&gc, &rect);

  /* Update drawing regions of picture  */
  gc.foregroundColor = GLIB_rgbColor(200, 200, 200);
  GLIB_drawString(&gc, efm32_hello, strlen(efm32_hello), 0, 0, 1);

  TFT_displayUpdate();
}

