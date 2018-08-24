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
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "soc/soc.h"
#include "soc/dport_reg.h"
#include "driver/gpio.h"
#include "lcd_com.h"
#include "i2s_lcd.h"

#define I2S_PORT_NUM (0)

#define LCD_D0_PIN  (19)
#define LCD_D1_PIN  (21)
#define LCD_D2_PIN  (0)
#define LCD_D3_PIN  (22)
#define LCD_D4_PIN  (23)
#define LCD_D5_PIN  (33)
#define LCD_D6_PIN  (32)
#define LCD_D7_PIN  (27)
#define LCD_D8_PIN  (25)
#define LCD_D9_PIN  (26)
#define LCD_D10_PIN (12)
#define LCD_D11_PIN (13)
#define LCD_D12_PIN (14)
#define LCD_D13_PIN (15)
#define LCD_D14_PIN (2)
#define LCD_D15_PIN (4)
#define LCD_WR_PIN  (18)
#define LCD_RS_PIN        (5)
#define LCD_RS_PIN_MUX    (PERIPHS_IO_MUX_MTCK_U)

// #if 1

// void WriteCmd8(unsigned char cmd)
// {
//     gpio_set_level(LCD_RS_PIN, 0);
//     i2s_lcd_write_data(I2S_PORT_NUM, (char *)&cmd, 1, 100);
//     gpio_set_level(LCD_RS_PIN, 1);
// }

// void WriteData8(unsigned char data)
// {
//     i2s_lcd_write_data(I2S_PORT_NUM, (char *)&data, 1, 100);
// }

// void WriteCmdData(unsigned char cmd, uint32_t data)
// {
//     WriteCmd8(cmd);
//     i2s_lcd_write_data(I2S_PORT_NUM, (char *)&data, 4, 100);
// }

// void WriteData(uint16_t data)
// {
//     uint16_t d = (data >> 8) | (data << 8);
//     i2s_lcd_write_data(I2S_PORT_NUM, (char *)&d, 2, 100);
// }

// void WriteCmd(uint16_t cmd)
// {
//     uint16_t c = (cmd >> 8) | (cmd << 8);
//     gpio_set_level(LCD_RS_PIN, 0);
//     i2s_lcd_write_data(I2S_PORT_NUM, (char *)&c, 2, 100);
//     gpio_set_level(LCD_RS_PIN, 1);
// }

// void WriteReg(uint16_t cmd, uint16_t data)
// {
//     uint16_t d = (data >> 8) | (data << 8); 
//     uint16_t c = (cmd >> 8) | (cmd << 8); 
//     gpio_set_level(LCD_RS_PIN, 0);
//     i2s_lcd_write_data(I2S_PORT_NUM, (char *)&c, 2, 100);
//     gpio_set_level(LCD_RS_PIN, 1);
//     i2s_lcd_write_data(I2S_PORT_NUM, (char *)&d, 2, 100);
// }

// void LCD_delay(uint32_t delay_time)
// {
//     vTaskDelay(10/portTICK_RATE_MS);
// }

// void LcdWrite(uint16_t *data, uint32_t len)
// {
//     i2s_lcd_write_data(I2S_PORT_NUM, (char *)data, len, 100);   
// }

// #else

void WriteData(uint16_t data)
{
    // printf("WriteData\n");
    // i2s_lcd_write_data(I2S_PORT_NUM, (char *)&data, 2, 100);
    REG_WRITE(0x6000f000, data);
    I2S0.conf.tx_start = 1;
    while(!(I2S0.state.tx_idle));
    I2S0.conf.tx_start = 0;
    I2S0.conf.tx_reset = 1;
    I2S0.conf.tx_reset = 0;
    I2S0.conf.tx_fifo_reset = 1;
    I2S0.conf.tx_fifo_reset = 0;
}

void WriteCmd(uint16_t cmd)
{
    // gpio_set_level(LCD_RS_PIN, 0);
    // i2s_lcd_write_data(I2S_PORT_NUM, (char *)&cmd, 2, 100);
    // gpio_set_level(LCD_RS_PIN, 1);
    GPIO.out_w1tc = (1 << LCD_RS_PIN);
    REG_WRITE(0x6000f000, cmd);
    I2S0.conf.tx_start = 1;
    while(!(I2S0.state.tx_idle));
    I2S0.conf.tx_start = 0;
    I2S0.conf.tx_reset = 1;
    I2S0.conf.tx_reset = 0;
    I2S0.conf.tx_fifo_reset = 1;
    I2S0.conf.tx_fifo_reset = 0;
    GPIO.out_w1ts = (1 << LCD_RS_PIN);
}

void WriteCmdData(uint16_t cmd, uint32_t data)
{
    gpio_set_level(LCD_RS_PIN, 0);
    i2s_lcd_write_data(I2S_PORT_NUM, (char *)&cmd, 2, 100);
    gpio_set_level(LCD_RS_PIN, 1);
    i2s_lcd_write_data(I2S_PORT_NUM, (char *)&data, 4, 100);
}

void WriteReg(uint16_t cmd, uint16_t data)
{
    // gpio_set_level(LCD_RS_PIN, 0);
    // i2s_lcd_write_data(I2S_PORT_NUM, (char *)&cmd, 2, 100);
    // gpio_set_level(LCD_RS_PIN, 1);
    // i2s_lcd_write_data(I2S_PORT_NUM, (char *)&data, 2, 100);
    GPIO.out_w1tc = (1 << LCD_RS_PIN);
    REG_WRITE(0x6000f000, cmd);
    I2S0.conf.tx_start = 1;
    while(!(I2S0.state.tx_idle));
    I2S0.conf.tx_start = 0;
    I2S0.conf.tx_reset = 1;
    I2S0.conf.tx_reset = 0;
    I2S0.conf.tx_fifo_reset = 1;
    I2S0.conf.tx_fifo_reset = 0;
    GPIO.out_w1ts = (1 << LCD_RS_PIN);

    REG_WRITE(0x6000f000, data);
    I2S0.conf.tx_start = 1;
    while(!(I2S0.state.tx_idle));
    I2S0.conf.tx_start = 0;
    I2S0.conf.tx_reset = 1;
    I2S0.conf.tx_reset = 0;
    I2S0.conf.tx_fifo_reset = 1;
    I2S0.conf.tx_fifo_reset = 0;
}

void LCD_delay(uint32_t delay_time)
{
    vTaskDelay(10/portTICK_RATE_MS);
}

void LcdWrite(uint16_t *data, uint32_t len)
{
    i2s_lcd_write_data(I2S_PORT_NUM, (char *)data, len, 100);   
}

// #endif


void lcd_interface_cfg(void)
{
    i2s_paral_pin_config_t pin_conf = {
        .data_width = 16,
        .data_io_num = {
            LCD_D0_PIN,  LCD_D1_PIN,  LCD_D2_PIN,  LCD_D3_PIN,
            LCD_D4_PIN,  LCD_D5_PIN,  LCD_D6_PIN,  LCD_D7_PIN,
            LCD_D8_PIN,  LCD_D9_PIN,  LCD_D10_PIN, LCD_D11_PIN,
            LCD_D12_PIN, LCD_D13_PIN, LCD_D14_PIN, LCD_D15_PIN,
    },
        .ws_io_num = LCD_WR_PIN,
    };
    // printf("lcd_interface_cfg\n");
    i2s_lcd_driver_install(I2S_PORT_NUM);
    i2s_lcd_pin_config(I2S_PORT_NUM, &pin_conf);
    gpio_set_direction(LCD_RS_PIN, GPIO_MODE_OUTPUT);
    gpio_set_pull_mode(LCD_RS_PIN, GPIO_PULLUP_ONLY);
    PIN_FUNC_SELECT(LCD_RS_PIN_MUX, 2);//??????????????
}