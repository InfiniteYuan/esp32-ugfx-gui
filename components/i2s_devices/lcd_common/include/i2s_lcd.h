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
#ifndef __I2S_LCD_H__
#define __I2S_LCD_H__

#include "driver/i2s.h"
#include "esp_err.h"
#include <esp_types.h>

typedef struct {
    int     data_width;          /*!< Parallel data width, 16bit or 8bit available */
    int     ws_io_num;           /*!< write clk io*/    
    uint8_t data_io_num[16];     /*!< Parallel data output IO*/  
} i2s_paral_pin_config_t;

esp_err_t i2s_lcd_pin_config(i2s_port_t i2s_num, i2s_paral_pin_config_t* pin_conf);
esp_err_t i2s_lcd_driver_install(i2s_port_t i2s_num);
int i2s_lcd_write_data(i2s_port_t i2s_num, const char *src, size_t size, TickType_t ticks_to_wait);

#endif