// Copyright 2015-2016 Espressif Systems (Shanghai) PTE LTD
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at

//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

/* C Includes */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* RTOS Includes */
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "freertos/semphr.h"

/* SPI Includes */
#include "iot_lcd.h"
#include "lcd_adapter.h"

/* ESP Includes */
#include "sdkconfig.h"
#include "esp_log.h"

/* uGFX Include */
#include "ugfx_driver_config.h"

class CEspLcdAdapter: public CEspLcd
{
public:
    const uint16_t* pFrameBuffer = NULL;
    CEspLcdAdapter(lcd_conf_t* lcd_conf, int height = LCD_TFTHEIGHT, int width = LCD_TFTWIDTH, bool dma_en = true, int dma_word_size = 1024, int dma_chan = 1):
        CEspLcd(lcd_conf, height, width, dma_en, dma_word_size, dma_chan)
    {
        /* Code here*/
    }
    void inline writeCmd(uint8_t cmd)
    {
        transmitCmd(cmd);
    }
    void inline writeData(uint16_t data)
    {
        transmitData(data);
    }
    void inline writeData(uint8_t data)
    {
        transmitData(data);
    }
    void inline writeData(uint16_t data, int point_num)
    {
        transmitData(data, point_num);
    }
    void inline writeCmdData(uint8_t cmd, uint32_t data)
    {
        transmitCmdData(cmd, data);
    }
    void inline writeData(uint8_t* data, uint16_t length)
    {
        transmitData(data, length);
    }
};

static CEspLcdAdapter* lcd_obj = NULL;

#if UGFX_DRIVER_AUTO_FLUSH_ENABLE
SemaphoreHandle_t flush_sem = NULL;

void board_lcd_flush_task(void* arg)
{
    portBASE_TYPE res;
    while(1) {
        res = xSemaphoreTake(flush_sem, portMAX_DELAY);
        if (res == pdTRUE) {
            lcd_obj->drawBitmap(0, 0, (const uint16_t*)lcd_obj->pFrameBuffer, UGFX_DRIVER_SCREEN_WIDTH, UGFX_DRIVER_SCREEN_HEIGHT);
            vTaskDelay(UGFX_DRIVER_AUTO_FLUSH_INTERVAL / portTICK_RATE_MS);
        }
    }
}
#endif

void board_lcd_init()
{
	/*Initialize LCD*/
	lcd_conf_t lcd_pins = {
        .lcd_model    = LCD_MOD_AUTO_DET,
        .pin_num_miso = UGFX_LCD_MISO_GPIO,
        .pin_num_mosi = UGFX_LCD_MOSI_GPIO,
        .pin_num_clk  = UGFX_LCD_CLK_GPIO,
        .pin_num_cs   = UGFX_LCD_CS_GPIO,
        .pin_num_dc   = UGFX_LCD_DC_GPIO,
        .pin_num_rst  = UGFX_LCD_RESET_GPIO,
        .pin_num_bckl = UGFX_LCD_BL_GPIO,
        .clk_freq     = 32 * 1000 * 1000,
        .rst_active_level = 0,
        .bckl_active_level = 1,
        .spi_host = HSPI_HOST,
        .init_spi_bus = true,
    };

	if(lcd_obj == NULL) {
	    lcd_obj = new CEspLcdAdapter(&lcd_pins);
	}
    lcd_obj->writeCmdData(0x36, 0x80|0x08);     // as default rotate

#if UGFX_DRIVER_AUTO_FLUSH_ENABLE
	// For framebuffer mode and flush
	if (flush_sem == NULL) {
	    flush_sem = xSemaphoreCreateBinary();
	}
	xTaskCreate(board_lcd_flush_task, "flush_task", 1500, NULL, 5, NULL);
#endif
}

void board_lcd_flush(int16_t x, int16_t y, const uint16_t* bitmap, int16_t w, int16_t h)
{
#if UGFX_DRIVER_AUTO_FLUSH_ENABLE
    lcd_obj->pFrameBuffer = bitmap;
    xSemaphoreGive(flush_sem);
#else
    lcd_obj->drawBitmap(x, y, bitmap, w, h);
#endif
}

void board_lcd_write_cmd(uint8_t cmd)
{
    lcd_obj->writeCmd(cmd);
}

void board_lcd_write_data(uint16_t data)
{
    lcd_obj->writeData(data);
}

void board_lcd_write_data_byte(uint8_t data)
{
    lcd_obj->writeData(data);
}

void board_lcd_write_data_byte_repeat(uint16_t data, int point_num)
{
    lcd_obj->writeData(data, point_num-1);
}

void board_lcd_write_cmddata(uint8_t cmd, uint32_t data)
{
    lcd_obj->writeCmdData(cmd, data);
}

void board_lcd_write_datas(uint8_t* data, uint16_t length)
{
    /* Code here*/
}

void board_lcd_set_backlight(uint16_t data)
{
    /* Code here*/
}
