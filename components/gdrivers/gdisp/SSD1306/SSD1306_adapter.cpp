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

/* I2C Includes */
#include "iot_i2c_bus.h"
#include "iot_ssd1306.h"
#include "ssd1306_fonts.h"
#include "driver/gpio.h"
#include "lcd_adapter.h"

/* ESP Includes */
#include "sdkconfig.h"
#include "ugfx_driver_config.h"
#include "esp_log.h"

#define POWER_CNTL_IO               19
//I2C
#define OLED_IIC_SCL_NUM            (gpio_num_t)CONFIG_UGFX_LCD_SCL_GPIO       /*!< gpio number for I2C master clock IO4*/
#define OLED_IIC_SDA_NUM            (gpio_num_t)CONFIG_UGFX_LCD_SDA_GPIO       /*!< gpio number for I2C master data IO17*/
#define OLED_IIC_NUM                I2C_NUM_1                                  /*!< I2C number >*/

class CLcdAdapter: public CSsd1306
{
public:
    const uint8_t *pFrameBuffer = NULL;
    CLcdAdapter(CI2CBus *p_i2c_bus): CSsd1306(p_i2c_bus)
    {
        gpio_config_t conf;
        conf.pin_bit_mask = (1 << POWER_CNTL_IO);
        conf.mode = GPIO_MODE_OUTPUT;
        conf.pull_up_en = (gpio_pullup_t) 0;
        conf.pull_down_en = (gpio_pulldown_t) 0;
        conf.intr_type = (gpio_int_type_t) 0;
        gpio_config(&conf);
        gpio_set_level((gpio_num_t) POWER_CNTL_IO, 0);
    }
    void inline writeCmd(uint8_t cmd)
    {
        iot_ssd1306_write_byte(get_dev_handle(), cmd, SSD1306_CMD);
    }
    void inline writeData(uint8_t data)
    {
        iot_ssd1306_write_byte(get_dev_handle(), data, SSD1306_DAT);
    }
    void inline writeData(uint8_t *data, uint16_t length)
    {
        for (uint16_t i = 0; i < length; i++) {
            iot_ssd1306_write_byte(get_dev_handle(), data[i], SSD1306_DAT);
        }
    }
};

static CLcdAdapter *lcd_obj = NULL;
static CI2CBus *i2c_bus = NULL;

void board_lcd_init()
{
    nvs_flash_init();

    /*Initialize LCD*/
    i2c_bus = new CI2CBus(OLED_IIC_NUM, OLED_IIC_SCL_NUM, OLED_IIC_SDA_NUM);

    if (lcd_obj == NULL) {
        lcd_obj = new CLcdAdapter(i2c_bus);
    }
}

void board_lcd_flush(int16_t x, int16_t y, const uint8_t *bitmap, int16_t w, int16_t h)
{
    lcd_obj->draw_bitmap(x, y, bitmap, UGFX_DRIVER_SCREEN_WIDTH, UGFX_DRIVER_SCREEN_HEIGHT);
}

void board_lcd_write_cmd(uint8_t cmd)
{
    lcd_obj->writeCmd(cmd);
}

void board_lcd_write_data(uint8_t data)
{
    lcd_obj->writeData(data);
}

void board_lcd_write_datas(uint8_t *data, uint16_t length)
{
    lcd_obj->writeData(data, length);
}

void board_lcd_set_backlight(uint16_t data)
{
    /* Code here*/
}
