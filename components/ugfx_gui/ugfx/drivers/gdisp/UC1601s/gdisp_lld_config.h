/*
 * This file is subject to the terms of the GFX License. If a copy of
 * the license was not distributed with this file, you can obtain one at:
 *
 *              http://ugfx.org/license.html
 */

#ifndef _GDISP_LLD_CONFIG_H
#define _GDISP_LLD_CONFIG_H

#if GFX_USE_GDISP

/*===========================================================================*/
/* Driver hardware support.                                                  */
/*===========================================================================*/

#define GDISP_HARDWARE_FLUSH			TRUE		// This controller requires flushing
#define GDISP_HARDWARE_DRAWPIXEL		TRUE
#define GDISP_HARDWARE_PIXELREAD		TRUE
#define GDISP_HARDWARE_CONTROL      	TRUE
#define GDISP_HARDWARE_FILLS        	FALSE

#define GDISP_LLD_PIXELFORMAT			GDISP_PIXELFORMAT_MONO

// This controller supports a special gdispControl() to inverse the display.
// Pass a parameter of 1 for inverse and 0 for normal.
#define GDISP_CONTROL_INVERSE			(GDISP_CONTROL_LLD+0)

#endif	/* GFX_USE_GDISP */

#endif	/* _GDISP_LLD_CONFIG_H */
