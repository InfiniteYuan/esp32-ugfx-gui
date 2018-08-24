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
#include <string.h>
#include "unity.h"
#include "esp_log.h"
#include "driver/i2c.h"
#include "iot_ft5x06.h"
#include "iot_i2c_bus.h"
#include "lcd_com.h"
#include "ppc.h"

#define I2C_NUM    (0)
#define I2C_SCL    (3)
#define I2C_SDA    (1)

lcd_dev_t *LcdNt35510Create(uint16_t x_size, uint16_t y_size);

//Store the touch information 
touch_info_t touch_info;

//Touch panel interface init.
static void touch_task(void *param)
{
    i2c_bus_handle_t i2c_bus = NULL;
    ft5x06_handle_t dev = NULL;
    i2c_config_t conf = {
        .mode = I2C_MODE_MASTER,
        .sda_io_num = I2C_SDA,
        .sda_pullup_en = GPIO_PULLUP_ENABLE,
        .scl_io_num = I2C_SCL,
        .scl_pullup_en = GPIO_PULLUP_ENABLE,
        .master.clk_speed = 200000,
    };
    i2c_bus = iot_i2c_bus_create(I2C_NUM, &conf);
    dev = iot_ft5x06_create(i2c_bus, FT5X06_ADDR_DEF);
    while(1) {
        if(iot_ft5x06_touch_report(dev, &touch_info) == ESP_OK) {
            if( touch_info.touch_point ) {
                vTaskDelay(50/portTICK_PERIOD_MS); 
            } else {
                vTaskDelay(10/portTICK_PERIOD_MS);    
            }
        }
    }
}

//
static void pic_slide_tset(void)
{
    lcd_dev_t *nt35510 = LcdNt35510Create(800, 480);
    if(nt35510 == NULL) {
        while(1);
    }
    uint16_t color = 0xaefc;
    uint16_t *raw = NULL;
    uint16_t *buf = NULL;
    uint16_t pox, poy;
    nt35510->lcd.LcdFillScreen(color);
    float stepx = 1.0;
    float stepy = 1.0;
    float k = 1.0;
    float m = 1.0;
    uint16_t lenx = 0;
    uint16_t leny = 0;
    uint16_t len = 0;
    uint16_t offsetx = 0;
    uint16_t offsety = 0;
    uint16_t i,j;
    uint16_t idx = 0;
    uint16_t idy = 0;
    int cnt = 0;
    xTaskCreate(touch_task, "touch_task", 2048, NULL, 10, NULL);
    nt35510->lcd.LcdDrawBmp(gImage_pic, 0, 0, 480, 800);
    int refresh = 1;
    while(1) {
        if(touch_info.touch_point == 1) {
            pox = 479-touch_info.curx[0];
            stepy = 1.0 - 1.0*pox/480;
            refresh = 1;
            if(stepy < 0.2) {
                stepy = 0.2;
            }
        } else {
            if(refresh == 1) {
                nt35510->lcd.LcdDrawBmp(gImage_pic, 0, 0, 480, 800);
                refresh = 0;
                continue;
            }
            stepy += 0.4;
            if(stepy >= 1.0) {
                refresh = 1;
                stepy = 1.0;
            }
        }

        leny = stepy * 480;
        len =  stepy * 800;
        k = 1.0/stepy;
        offsety = (480-leny)/2;
        stepx = 1.0*(800-len)/leny;
        i = 0;
        while(1) {
            j = 0;
            idy = k * i;
            if(idy > 480) {
                break;
            }
            lenx = stepx*i+len;
            m = 1.0 * 800/lenx;
            offsetx = (800-lenx)/2;
            raw = &(gImage_pic[idy*800*2]);
            buf = &(nt35510->lcd_buf[offsetx]);
            for(int i = 0; i < 500; i++) {
                nt35510->lcd_buf[i] = 0xaefb;  
            }
            for(int i = 500; i < 800; i++) {
                nt35510->lcd_buf[i] = 0xaefe;  
            }
            while(1) {
                idx = m * j;
                if(j > lenx || idx > 800) {
                    break;
                }
                buf[j] = raw[idx];
                j++;
            }
            LcdWrite(nt35510->lcd_buf, 800*2);
            cnt++;
            i++;
        }
        for(int i = 0; i < 500; i++) {
            nt35510->lcd_buf[i] = 0xaefb;  
        }
        for(int i = 500; i < 800; i++) {
            nt35510->lcd_buf[i] = 0xaefe;  
        }
        for(int c = 0; c < 480-cnt; c++) {
            LcdWrite(nt35510->lcd_buf, 800*2);
        }
        cnt = 0;
    }
}

//Multi-touch test

void multi_touch_test(void)
{
    
    lcd_dev_t *nt35510 = LcdNt35510Create(800, 480);
    if(nt35510 == NULL) {
        while(1);
    }
    uint16_t color = 0xaefc;
    touch_info_t touch_tmp;
    nt35510->lcd.LcdFillScreen(color);
    xTaskCreate(touch_task, "touch_task", 2048, NULL, 10, NULL);

    uint16_t col1 = (0xf800 & (0x0000 << 8))| (0x03e0 & (0x0000 << 4))| (0x001f & 0x0005);
    uint16_t col2 = (0xf800 & (0x00ff << 8))| (0x03e0 & (0x00a5 << 4))| (0x001f & 0x0000);
    uint16_t col3 = (0xf800 & (0x00ff << 8))| (0x03e0 & (0x0069 << 4))| (0x001f & 0x00b4);
    uint16_t col4 = (0xf800 & (0x0099 << 8))| (0x03e0 & (0x0032 << 4))| (0x001f & 0x00cc);
    uint16_t col5 = (0xf800 & (0x00ff << 8))| (0x03e0 & (0x00ff << 4))| (0x001f & 0x0000);
    uint16_t col6 = (0xf800 & (0x0000 << 8))| (0x03e0 & (0x00ff << 4))| (0x001f & 0x0000);
    uint16_t *data = nt35510->lcd_buf;
    while(1) {
        if(touch_info.touch_point) {
            memcpy(&touch_tmp, &touch_info, sizeof(touch_info_t));
            for(int j = 0; j < 480; j++) {
                if(j == touch_tmp.curx[0]) {
                    for(int i = 0; i < 800; i++) {
                        data[i] = col2;
                    }
                } else if(j == touch_tmp.curx[1]) {
                    for(int i = 0; i < 800; i++) {
                        data[i] = col3;
                    }
                } else if(j == touch_tmp.curx[2]) {
                    for(int i = 0; i < 800; i++) {
                        data[i] = col4;
                    }
                } else if(j == touch_tmp.curx[3]) {
                    for(int i = 0; i < 800; i++) {
                        data[i] = col5;
                    }
                }else if( j == touch_tmp.curx[4]) {
                    for(int i = 0; i < 800; i++) {
                        data[i] = col6;
                    }
                } else {
                    for(int i = 0; i < 800; i++) {
                        if(i == touch_tmp.cury[0]) {
                            data[i] = col2;
                        } else if(i == touch_tmp.cury[1]) {
                            data[i] = col3;
                        } else if(i == touch_tmp.cury[2]) {
                            data[i] = col4;
                        } else if(i == touch_tmp.cury[3]) {
                            data[i] = col5;
                        } else if(i == touch_tmp.cury[4]) {
                            data[i] = col6;  
                        } else {
                            data[i] = col1;
                        }
                    }
                }
                LcdWrite(nt35510->lcd_buf, sizeof(uint16_t) * 800);
            }
        } else  {
            for(int i = 0; i < 800; i++) {
                data[i] = col1;   
            }
            for(int j = 0; j < 480; j++) {
                LcdWrite(nt35510->lcd_buf, sizeof(uint16_t) * 800);
            }
        }
    }
}

//Resolution  480*800

TEST_CASE("Cap picture slide test", "[Cap TFT test][iot][device]")
{
    pic_slide_tset();
}

TEST_CASE("Cap multi touch test", "[Cap TFT test][iot][device]")
{
    multi_touch_test();
}