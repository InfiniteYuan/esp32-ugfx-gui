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
#ifndef   __LCD_COM_H__
#define   __LCD_COM_H__

#include "stdio.h"
#include "esp_system.h"

#define LCD_LOG(s)    printf("[%s   %d] : %s\n", __FUNCTION__, __LINE__, s);

#ifdef __cplusplus
extern "C"
{
#endif

typedef struct Lcd_APIs{
    void (*LcdInit)(void);
    void (*LcdSetPos)(uint16_t x, uint16_t y);
    void (*LcdSetBox)(uint16_t x, uint16_t y, uint16_t x_size, uint16_t y_size);
    void (*LcdWrite)(uint16_t *data, uint16_t len);
    void (*LcdDrawRet)(uint16_t sx, uint16_t sy, uint16_t ex, uint16_t ey, uint16_t rgb);
    void (*LcdDrawHline)(uint16_t sx, uint16_t sy, uint16_t ey, uint16_t rgb);
    void (*LcdDrawVline)(uint16_t sy, uint16_t sx, uint16_t ex, uint16_t rgb);
    void (*LcdFresh)(uint16_t sx, uint16_t sy, uint16_t ex, uint16_t ey);
    void (*LcdDrawBmp)(uint16_t*bmp, uint16_t x, uint16_t y, uint16_t x_size, uint16_t y_size);
    void (*LcdFillScreen)(uint16_t rgb);
    void (*LcdFillArea)(uint16_t color, uint16_t x, uint16_t y);
    void (*LcdSetWin)(uint16_t win);
    void (*LcdPutsAsc8x16)(char *str, uint16_t x, uint16_t y, uint16_t wcolor, uint16_t bcolor);
} lcd_api_t;

typedef struct tuoch_dev{
    int (*touch_report)(uint16_t *pos);
} touch_dev_t;

typedef struct screen_dev { 
    lcd_api_t   lcd;
    touch_dev_t touch;
    uint16_t x_size;
    uint16_t y_size;
    uint8_t wine_ori;
    uint16_t *lcd_buf;
    uint16_t xset_cmd;
    uint16_t yset_cmd;
    uint16_t pix;
} lcd_dev_t;

void WriteCmdData(uint16_t cmd, uint32_t data);

void WriteData(uint16_t data);
void WriteCmd(uint16_t cmd);
void WriteReg(uint16_t cmd, uint16_t data);
void LCD_delay(uint32_t delay_time);
void LcdWrite(uint16_t *data, uint32_t len);
void lcd_interface_cfg(void);

#ifdef __cplusplus
}
#endif

#endif