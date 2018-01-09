/*
 * This file is subject to the terms of the GFX License. If a copy of
 * the license was not distributed with this file, you can obtain one at:
 *
 *              http://ugfx.org/license.html
 */

#ifndef _GDISP_LLD_BOARD_H
#define _GDISP_LLD_BOARD_H

#ifdef __cplusplus
extern "C"
{
#endif

#include "gfx.h"
#include "freertos/semphr.h"
#include "driver/spi_master.h"
#include "iot_lcd.h"

#define ST7565_LCD_BIAS         ST7565_LCD_BIAS_7
#define ST7565_ADC              ST7565_ADC_NORMAL
#define ST7565_COM_SCAN         ST7565_COM_SCAN_INC
#define ST7565_PAGE_ORDER       0,1,2,3,4,5,6,7
/*
 * Custom page order for several LCD boards, e.g. HEM12864-99
 * #define ST7565_PAGE_ORDER       4,5,6,7,0,1,2,3
 */

void set_spidevice(spi_device_handle_t mspi_wr);

void set_semaphore(SemaphoreHandle_t mspi_mux);

void set_dc(lcd_dc_t mdc);

void post_init_board(GDisplay *g);

void setpin_reset(GDisplay *g, bool_t state);

void acquire_bus(GDisplay *g);

void release_bus(GDisplay *g);

void acquire_sem();

void release_sem();

void write_cmd(GDisplay *g, uint8_t cmd);

void write_data(GDisplay *g, uint16_t data);

void write_data_byte(GDisplay *g, uint8_t data);

void write_data_byte_repeat(GDisplay *g, uint16_t data, int point_num);

void write_cmddata(GDisplay *g, uint8_t cmd, uint32_t data);

void write_datas(GDisplay *g, uint8_t* data, uint16_t length);

void set_backlight(GDisplay *g, uint16_t data);

#ifdef __cplusplus
}
#endif

#endif /* _GDISP_LLD_BOARD_H */
