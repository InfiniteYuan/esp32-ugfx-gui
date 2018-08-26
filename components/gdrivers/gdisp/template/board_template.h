/*
 * This file is subject to the terms of the GFX License. If a copy of
 * the license was not distributed with this file, you can obtain one at:
 *
 *              http://ugfx.org/license.html
 */

#ifndef _GDISP_LLD_BOARD_H
#define _GDISP_LLD_BOARD_H

#define ST7565_LCD_BIAS         ST7565_LCD_BIAS_7
#define ST7565_ADC              ST7565_ADC_NORMAL
#define ST7565_COM_SCAN         ST7565_COM_SCAN_INC
#define ST7565_PAGE_ORDER       0,1,2,3,4,5,6,7
/*
 * Custom page order for several LCD boards, e.g. HEM12864-99
 * #define ST7565_PAGE_ORDER       4,5,6,7,0,1,2,3
 */

static GFXINLINE void init_board(GDisplay *g)
{
    (void) g;
}

static GFXINLINE void post_init_board(GDisplay *g)
{
    (void) g;
}

static GFXINLINE void setpin_reset(GDisplay *g, bool_t state)
{
    (void) g;
    (void) state;
}

static GFXINLINE void acquire_bus(GDisplay *g)
{
    (void) g;
}

static GFXINLINE void release_bus(GDisplay *g)
{
    (void) g;
}

static GFXINLINE void write_cmd(GDisplay *g, uint8_t cmd)
{
    (void) g;
    (void) cmd;
}

static GFXINLINE void write_data(GDisplay *g, uint8_t *data, uint16_t length)
{
    (void) g;
    (void) data;
    (void) length;
}

#endif /* _GDISP_LLD_BOARD_H */
