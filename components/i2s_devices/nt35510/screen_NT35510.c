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
#include "stdio.h"
#include "string.h"
#include "esp_system.h"
#include "lcd_com.h"

#define LCD_BUF_ADDR     (0x3f8f0000)

static  lcd_dev_t * lcd_dev = NULL;

static void lcd_set_pox(uint16_t x, uint16_t y)
{
    WriteReg(0x2a00, x >> 8);
    WriteReg(0x2a01, x & 0xff);
    WriteReg(0x2b00, y >> 8);
    WriteReg(0x2b01, y & 0xff);
    WriteCmd(0x2C00);       
}

static void lcd_set_window(uint8_t win)
{
    if(win & 0x20) {
        // LCD_LOG("lcd_dev_t 0x20\n");
        lcd_dev->xset_cmd = 0x2b00;
        lcd_dev->yset_cmd = 0x2a00;
        // uint16_t x_size = lcd_dev->x_size;
        // lcd_dev->x_size =  lcd_dev->y_size;
        // lcd_dev->y_size =  x_size;
    } else {
        // LCD_LOG("lcd_dev_t ohter\n");
        lcd_dev->xset_cmd = 0x2a00;
        lcd_dev->yset_cmd = 0x2b00;
    }
    WriteReg(0x3600, win);
}

static void lcd_set_box(uint16_t x, uint16_t y, uint16_t x_size, uint16_t y_size)
{
    uint16_t x_end = x + (x_size - 1);
    uint16_t y_end = y + (y_size - 1);
    WriteReg(lcd_dev->xset_cmd, x >> 8);
    WriteReg(lcd_dev->xset_cmd + 1, x & 0xff);
    WriteReg(lcd_dev->xset_cmd + 2, x_end >> 8);
    WriteReg(lcd_dev->xset_cmd + 3, x_end & 0xff);
    WriteReg(lcd_dev->yset_cmd, y >> 8); 
    WriteReg(lcd_dev->yset_cmd + 1, y & 0xff);
    WriteReg(lcd_dev->yset_cmd + 2, y_end >> 8);
    WriteReg(lcd_dev->yset_cmd + 3, y_end & 0xff);
    WriteCmd(0x2C00);
}

static void lcd_fresh(void)
{
    LcdWrite(lcd_dev->lcd_buf, lcd_dev->x_size * lcd_dev->y_size * 2);       
}

static void lcd_fill_screen(uint16_t color)
{
    uint16_t *p = lcd_dev->lcd_buf;
    lcd_set_box(0, 0, lcd_dev->x_size,lcd_dev->y_size);
    for(int i = 0; i < lcd_dev->x_size; i++) {
        p[i] = color;
    }
    for(int i = 0; i < lcd_dev->y_size; i++) {
        LcdWrite(lcd_dev->lcd_buf, lcd_dev->x_size * lcd_dev->pix);
    }
}
static void lcd_fill_area(uint16_t color, uint16_t x, uint16_t y)
{
    // LCD_LOG("LCD dev lcd_fill_areal\n");
    // printf("%d.%d,%d\n", color, x, y);
    uint16_t *p = lcd_dev->lcd_buf;
    for(int i = 0; i < x; i++) {
        p[i] = color;
    }
    // LCD_LOG("LCD dev p\n");
    for(int i = 0; i < y; i++) {
        LcdWrite(lcd_dev->lcd_buf, x * lcd_dev->pix);
    }
    // LCD_LOG("LCD dev LcdWrite\n");
}

static void lcd_draw_bmp(uint16_t*bmp, uint16_t x, uint16_t y, uint16_t x_size, uint16_t y_size)
{
    lcd_set_box(x, y, x_size, y_size);
    LcdWrite(bmp, x_size * y_size * lcd_dev->pix);
}

static void lcd_put_char(uint8_t *str, uint16_t x, uint16_t y, uint16_t x_size, uint16_t y_size, uint16_t wcolor, uint16_t bcolor)
{
    uint8_t *pdata = str;
    uint16_t *pbuf;
    for(int i = 0; i < y_size; i++) {
        pbuf = lcd_dev->lcd_buf+(x+(i+y)*lcd_dev->x_size);
        for(int j = 0; j < x_size / 8; j++) {
            for(int k = 0; k < 8; k++) {
                if(*pdata & (0x80 >> k)) {
                    *pbuf = wcolor;   
                } else {
                    *pbuf = bcolor;    
                }
                pbuf++;
            }
            pdata++;
        }
    }
    lcd_fresh();
}

#include "asc8x16.h"

static void inline lcd_asc8x16_to_men(char str, uint16_t x, uint16_t y,uint16_t wcolor, uint16_t bcolor)
{
    uint8_t *pdata = font_asc8x16 + (str - ' ') * 16;
    uint16_t *pbuf;
    for(int i = 0; i < 16; i++) {
        pbuf = lcd_dev->lcd_buf+(x+(i+y)*lcd_dev->x_size);
            for(int k = 0; k < 8; k++) {
                if(*pdata & (0x80 >> k)) {
                    *pbuf = wcolor;   
                } else {
                    *pbuf = bcolor;    
                }
                pbuf++;
            }
            pdata++;
    }   
}

static void lcd_put_asc8x16(char str, uint16_t x, uint16_t y,uint16_t wcolor, uint16_t bcolor)
{
    
    lcd_asc8x16_to_men(str, x, y,wcolor, bcolor);
    lcd_fresh();          
}

static void lcd_put_string8x16(char *str, uint16_t x, uint16_t y, uint16_t wcolor, uint16_t bcolor)
{
    uint32_t x_ofsset = 0;
    uint32_t y_offset = 0;
    while(*str != '\0') {
        lcd_asc8x16_to_men(*str, x+x_ofsset, y+y_offset, wcolor, bcolor);
        x_ofsset = x_ofsset + 8;
        if(x_ofsset > lcd_dev->x_size - 8) {
            y_offset += 16;
            x_ofsset = 0;
            if(y_offset > lcd_dev->y_size - 16) {
                break;
            }
        }
        str++;
    }
    lcd_fresh();
}

static void nt35510_init(void)
{
    WriteCmd(0x0100);
    LCD_delay(10);
    WriteCmd(0x1200);
    WriteReg(0xf000, 0x0055);
    WriteReg(0xf001, 0x00aa);
    WriteReg(0xf002, 0x0052);
    WriteReg(0xf003, 0x0008);
    WriteReg(0xf004, 0x0001);

    WriteReg(0xbc01, 0x0086);
    WriteReg(0xbc02, 0x006a);
    WriteReg(0xbd01, 0x0086);
    WriteReg(0xbd02, 0x006a);
    WriteReg(0xbe01, 0x0067);

    WriteReg(0xd100, 0x0000);
    WriteReg(0xd101, 0x005d);
    WriteReg(0xd102, 0x0000);
    WriteReg(0xd103, 0x006b);
    WriteReg(0xd104, 0x0000);
    WriteReg(0xd105, 0x0084);
    WriteReg(0xd106, 0x0000);
    WriteReg(0xd107, 0x009c);
    WriteReg(0xd108, 0x0000);
    WriteReg(0xd109, 0x00b1);
    WriteReg(0xd10a, 0x0000);
    WriteReg(0xd10b, 0x00d9);
    WriteReg(0xd10c, 0x0000);
    WriteReg(0xd10d, 0x00fd);
    WriteReg(0xd10e, 0x0001);
    WriteReg(0xd10f, 0x0038);
    WriteReg(0xd110, 0x0001);
    WriteReg(0xd111, 0x0068);
    WriteReg(0xd112, 0x0001);
    WriteReg(0xd113, 0x00b9);
    WriteReg(0xd114, 0x0001);
    WriteReg(0xd115, 0x00fb);
    WriteReg(0xd116, 0x0002);
    WriteReg(0xd117, 0x0063);
    WriteReg(0xd118, 0x0002);
    WriteReg(0xd119, 0x00b9);
    WriteReg(0xd11a, 0x0002);
    WriteReg(0xd11b, 0x00bb);
    WriteReg(0xd11c, 0x0003);
    WriteReg(0xd11d, 0x0003);
    WriteReg(0xd11e, 0x0003);
    WriteReg(0xd11f, 0x0046);
    WriteReg(0xd120, 0x0003);
    WriteReg(0xd121, 0x0069);
    WriteReg(0xd122, 0x0003);
    WriteReg(0xd123, 0x008f);
    WriteReg(0xd124, 0x0003);
    WriteReg(0xd125, 0x00a4);
    WriteReg(0xd126, 0x0003);
    WriteReg(0xd127, 0x00b9);
    WriteReg(0xd128, 0x0003);
    WriteReg(0xd129, 0x00c7);
    WriteReg(0xd12a, 0x0003);
    WriteReg(0xd12b, 0x00c9);
    WriteReg(0xd12c, 0x0003);
    WriteReg(0xd12d, 0x00cb);
    WriteReg(0xd12e, 0x0003);
    WriteReg(0xd12f, 0x00cb);
    WriteReg(0xd130, 0x0003);
    WriteReg(0xd131, 0x00cb);
    WriteReg(0xd132, 0x0003);
    WriteReg(0xd133, 0x00cc);

    WriteReg(0xd200, 0x0000);
    WriteReg(0xd201, 0x005d);
    WriteReg(0xd202, 0x0000);
    WriteReg(0xd203, 0x006b);
    WriteReg(0xd204, 0x0000);
    WriteReg(0xd205, 0x0084);
    WriteReg(0xd206, 0x0000);
    WriteReg(0xd207, 0x009c);
    WriteReg(0xd208, 0x0000);
    WriteReg(0xd209, 0x00b1);
    WriteReg(0xd20a, 0x0000);
    WriteReg(0xd20b, 0x00d9);
    WriteReg(0xd20c, 0x0000);
    WriteReg(0xd20d, 0x00fd);
    WriteReg(0xd20e, 0x0001);
    WriteReg(0xd20f, 0x0038);
    WriteReg(0xd210, 0x0001);
    WriteReg(0xd211, 0x0068);
    WriteReg(0xd212, 0x0001);
    WriteReg(0xd213, 0x00b9);
    WriteReg(0xd214, 0x0001);
    WriteReg(0xd215, 0x00fb);
    WriteReg(0xd216, 0x0002);
    WriteReg(0xd217, 0x0063);
    WriteReg(0xd218, 0x0002);
    WriteReg(0xd219, 0x00b9);
    WriteReg(0xd21a, 0x0002);
    WriteReg(0xd21b, 0x00bb);
    WriteReg(0xd21c, 0x0003);
    WriteReg(0xd21d, 0x0003);
    WriteReg(0xd21e, 0x0003);
    WriteReg(0xd21f, 0x0046);
    WriteReg(0xd220, 0x0003);
    WriteReg(0xd221, 0x0069);
    WriteReg(0xd222, 0x0003);
    WriteReg(0xd223, 0x008f);
    WriteReg(0xd224, 0x0003);
    WriteReg(0xd225, 0x00a4);
    WriteReg(0xd226, 0x0003);
    WriteReg(0xd227, 0x00b9);
    WriteReg(0xd228, 0x0003);
    WriteReg(0xd229, 0x00c7);
    WriteReg(0xd22a, 0x0003);
    WriteReg(0xd22b, 0x00c9);
    WriteReg(0xd22c, 0x0003);
    WriteReg(0xd22d, 0x00cb);
    WriteReg(0xd22e, 0x0003);
    WriteReg(0xd22f, 0x00cb);
    WriteReg(0xd230, 0x0003);
    WriteReg(0xd231, 0x00cb);
    WriteReg(0xd232, 0x0003);
    WriteReg(0xd233, 0x00cc);

    WriteReg(0xd300, 0x0000);
    WriteReg(0xd301, 0x005d);
    WriteReg(0xd302, 0x0000);
    WriteReg(0xd303, 0x006b);
    WriteReg(0xd304, 0x0000);
    WriteReg(0xd305, 0x0084);
    WriteReg(0xd306, 0x0000);
    WriteReg(0xd307, 0x009c);
    WriteReg(0xd308, 0x0000);
    WriteReg(0xd309, 0x00b1);
    WriteReg(0xd30a, 0x0000);
    WriteReg(0xd30b, 0x00d9);
    WriteReg(0xd30c, 0x0000);
    WriteReg(0xd30d, 0x00fd);
    WriteReg(0xd30e, 0x0001);
    WriteReg(0xd30f, 0x0038);
    WriteReg(0xd310, 0x0001);
    WriteReg(0xd311, 0x0068);
    WriteReg(0xd312, 0x0001);
    WriteReg(0xd313, 0x00b9);
    WriteReg(0xd314, 0x0001);
    WriteReg(0xd315, 0x00fb);
    WriteReg(0xd316, 0x0002);
    WriteReg(0xd317, 0x0063);
    WriteReg(0xd318, 0x0002);
    WriteReg(0xd319, 0x00b9);
    WriteReg(0xd31a, 0x0002);
    WriteReg(0xd31b, 0x00bb);
    WriteReg(0xd31c, 0x0003);
    WriteReg(0xd31d, 0x0003);
    WriteReg(0xd31e, 0x0003);
    WriteReg(0xd31f, 0x0046);
    WriteReg(0xd320, 0x0003);
    WriteReg(0xd321, 0x0069);
    WriteReg(0xd322, 0x0003);
    WriteReg(0xd323, 0x008f);
    WriteReg(0xd324, 0x0003);
    WriteReg(0xd325, 0x00a4);
    WriteReg(0xd326, 0x0003);
    WriteReg(0xd327, 0x00b9);
    WriteReg(0xd328, 0x0003);
    WriteReg(0xd329, 0x00c7);
    WriteReg(0xd32a, 0x0003);
    WriteReg(0xd32b, 0x00c9);
    WriteReg(0xd32c, 0x0003);
    WriteReg(0xd32d, 0x00cb);
    WriteReg(0xd32e, 0x0003);
    WriteReg(0xd32f, 0x00cb);
    WriteReg(0xd330, 0x0003);
    WriteReg(0xd331, 0x00cb);
    WriteReg(0xd332, 0x0003);
    WriteReg(0xd333, 0x00cc);

    WriteReg(0xd400, 0x0000);
    WriteReg(0xd401, 0x005d);
    WriteReg(0xd402, 0x0000);
    WriteReg(0xd403, 0x006b);
    WriteReg(0xd404, 0x0000);
    WriteReg(0xd405, 0x0084);
    WriteReg(0xd406, 0x0000);
    WriteReg(0xd407, 0x009c);
    WriteReg(0xd408, 0x0000);
    WriteReg(0xd409, 0x00b1);
    WriteReg(0xd40a, 0x0000);
    WriteReg(0xd40b, 0x00d9);
    WriteReg(0xd40c, 0x0000);
    WriteReg(0xd40d, 0x00fd);
    WriteReg(0xd40e, 0x0001);
    WriteReg(0xd40f, 0x0038);
    WriteReg(0xd410, 0x0001);
    WriteReg(0xd411, 0x0068);
    WriteReg(0xd412, 0x0001);
    WriteReg(0xd413, 0x00b9);
    WriteReg(0xd414, 0x0001);
    WriteReg(0xd415, 0x00fb);
    WriteReg(0xd416, 0x0002);
    WriteReg(0xd417, 0x0063);
    WriteReg(0xd418, 0x0002);
    WriteReg(0xd419, 0x00b9);
    WriteReg(0xd41a, 0x0002);
    WriteReg(0xd41b, 0x00bb);
    WriteReg(0xd41c, 0x0003);
    WriteReg(0xd41d, 0x0003);
    WriteReg(0xd41e, 0x0003);
    WriteReg(0xd41f, 0x0046);
    WriteReg(0xd420, 0x0003);
    WriteReg(0xd421, 0x0069);
    WriteReg(0xd422, 0x0003);
    WriteReg(0xd423, 0x008f);
    WriteReg(0xd424, 0x0003);
    WriteReg(0xd425, 0x00a4);
    WriteReg(0xd426, 0x0003);
    WriteReg(0xd427, 0x00b9);
    WriteReg(0xd428, 0x0003);
    WriteReg(0xd429, 0x00c7);
    WriteReg(0xd42a, 0x0003);
    WriteReg(0xd42b, 0x00c9);
    WriteReg(0xd42c, 0x0003);
    WriteReg(0xd42d, 0x00cb);
    WriteReg(0xd42e, 0x0003);
    WriteReg(0xd42f, 0x00cb);
    WriteReg(0xd430, 0x0003);
    WriteReg(0xd431, 0x00cb);
    WriteReg(0xd432, 0x0003);
    WriteReg(0xd433, 0x00cc);

    WriteReg(0xd500, 0x0000);
    WriteReg(0xd501, 0x005d);
    WriteReg(0xd502, 0x0000);
    WriteReg(0xd503, 0x006b);
    WriteReg(0xd504, 0x0000);
    WriteReg(0xd505, 0x0084);
    WriteReg(0xd506, 0x0000);
    WriteReg(0xd507, 0x009c);
    WriteReg(0xd508, 0x0000);
    WriteReg(0xd509, 0x00b1);
    WriteReg(0xd50a, 0x0000);
    WriteReg(0xd50b, 0x00D9);
    WriteReg(0xd50c, 0x0000);
    WriteReg(0xd50d, 0x00fd);
    WriteReg(0xd50e, 0x0001);
    WriteReg(0xd50f, 0x0038);
    WriteReg(0xd510, 0x0001);
    WriteReg(0xd511, 0x0068);
    WriteReg(0xd512, 0x0001);
    WriteReg(0xd513, 0x00b9);
    WriteReg(0xd514, 0x0001);
    WriteReg(0xd515, 0x00fb);
    WriteReg(0xd516, 0x0002);
    WriteReg(0xd517, 0x0063);
    WriteReg(0xd518, 0x0002);
    WriteReg(0xd519, 0x00b9);
    WriteReg(0xd51a, 0x0002);
    WriteReg(0xd51b, 0x00bb);
    WriteReg(0xd51c, 0x0003);
    WriteReg(0xd51d, 0x0003);
    WriteReg(0xd51e, 0x0003);
    WriteReg(0xd51f, 0x0046);
    WriteReg(0xd520, 0x0003);
    WriteReg(0xd521, 0x0069);
    WriteReg(0xd522, 0x0003);
    WriteReg(0xd523, 0x008f);
    WriteReg(0xd524, 0x0003);
    WriteReg(0xd525, 0x00a4);
    WriteReg(0xd526, 0x0003);
    WriteReg(0xd527, 0x00b9);
    WriteReg(0xd528, 0x0003);
    WriteReg(0xd529, 0x00c7);
    WriteReg(0xd52a, 0x0003);
    WriteReg(0xd52b, 0x00c9);
    WriteReg(0xd52c, 0x0003);
    WriteReg(0xd52d, 0x00cb);
    WriteReg(0xd52e, 0x0003);
    WriteReg(0xd52f, 0x00cb);
    WriteReg(0xd530, 0x0003);
    WriteReg(0xd531, 0x00cb);
    WriteReg(0xd532, 0x0003);
    WriteReg(0xd533, 0x00cc);

    WriteReg(0xd600, 0x0000);
    WriteReg(0xd601, 0x005d);
    WriteReg(0xd602, 0x0000);
    WriteReg(0xd603, 0x006b);
    WriteReg(0xd604, 0x0000);
    WriteReg(0xd605, 0x0084);
    WriteReg(0xd606, 0x0000);
    WriteReg(0xd607, 0x009c);
    WriteReg(0xd608, 0x0000);
    WriteReg(0xd609, 0x00b1);
    WriteReg(0xd60a, 0x0000);
    WriteReg(0xd60b, 0x00d9);
    WriteReg(0xd60c, 0x0000);
    WriteReg(0xd60d, 0x00fd);
    WriteReg(0xd60e, 0x0001);
    WriteReg(0xd60f, 0x0038);
    WriteReg(0xd610, 0x0001);
    WriteReg(0xd611, 0x0068);
    WriteReg(0xd612, 0x0001);
    WriteReg(0xd613, 0x00b9);
    WriteReg(0xd614, 0x0001);
    WriteReg(0xd615, 0x00fb);
    WriteReg(0xd616, 0x0002);
    WriteReg(0xd617, 0x0063);
    WriteReg(0xd618, 0x0002);
    WriteReg(0xd619, 0x00b9);
    WriteReg(0xd61a, 0x0002);
    WriteReg(0xd61b, 0x00bb);
    WriteReg(0xd61c, 0x0003);
    WriteReg(0xd61d, 0x0003);
    WriteReg(0xd61e, 0x0003);
    WriteReg(0xd61f, 0x0046);
    WriteReg(0xd620, 0x0003);
    WriteReg(0xd621, 0x0069);
    WriteReg(0xd622, 0x0003);
    WriteReg(0xd623, 0x008f);
    WriteReg(0xd624, 0x0003);
    WriteReg(0xd625, 0x00a4);
    WriteReg(0xd626, 0x0003);
    WriteReg(0xd627, 0x00b9);
    WriteReg(0xd628, 0x0003);
    WriteReg(0xd629, 0x00c7);
    WriteReg(0xd62a, 0x0003);
    WriteReg(0xd62b, 0x00c9);
    WriteReg(0xd62c, 0x0003);
    WriteReg(0xd62d, 0x00cb);
    WriteReg(0xd62e, 0x0003);
    WriteReg(0xd62f, 0x00cb);
    WriteReg(0xd630, 0x0003);
    WriteReg(0xd631, 0x00cb);
    WriteReg(0xd632, 0x0003);
    WriteReg(0xd633, 0x00cc);

    WriteReg(0xba00, 0x0024);
    WriteReg(0xba01, 0x0024);
    WriteReg(0xba02, 0x0024);

    WriteReg(0xb900, 0x0024);
    WriteReg(0xb901, 0x0024);
    WriteReg(0xb902, 0x0024);

    WriteReg(0xf000, 0x0055);
    WriteReg(0xf001, 0x00aa);
    WriteReg(0xf002, 0x0052);
    WriteReg(0xf003, 0x0008);
    WriteReg(0xf004, 0x0000);

    WriteReg(0xb100, 0x00cc);
    WriteReg(0xB500, 0x0050);

    WriteReg(0xbc00, 0x0005);
    WriteReg(0xbc01, 0x0005);
    WriteReg(0xbc02, 0x0005);

    WriteReg(0xb800, 0x0001);
    WriteReg(0xb801, 0x0003);
    WriteReg(0xb802, 0x0003);
    WriteReg(0xb803, 0x0003);

    WriteReg(0xbd02, 0x0007);
    WriteReg(0xbd03, 0x0031);
    WriteReg(0xbe02, 0x0007);
    WriteReg(0xbe03, 0x0031);
    WriteReg(0xbf02, 0x0007);
    WriteReg(0xbf03, 0x0031);

    WriteReg(0xff00, 0x00aa);
    WriteReg(0xff01, 0x0055);
    WriteReg(0xff02, 0x0025);
    WriteReg(0xff03, 0x0001);

    WriteReg(0xf304, 0x0011);
    WriteReg(0xf306, 0x0010);
    WriteReg(0xf308, 0x0000);

    WriteReg(0x3500, 0x0000);
    WriteReg(0x3A00, 0x0005);
    //Display On
    WriteCmd(0x2900);
    // Out sleep
    WriteCmd(0x1100);
    // Write continue
    WriteCmd(0x2C00);
}

static void lcd_api_register(void)
{
    lcd_dev->lcd.LcdInit = nt35510_init;
    lcd_dev->lcd.LcdSetPos = lcd_set_pox;
    lcd_dev->lcd.LcdFillScreen = lcd_fill_screen;
    lcd_dev->lcd.LcdDrawBmp = lcd_draw_bmp;
    lcd_dev->lcd.LcdPutsAsc8x16 = lcd_put_string8x16;
    lcd_dev->lcd.LcdSetBox = lcd_set_box;
    lcd_dev->lcd.LcdFillArea = lcd_fill_area;
}

lcd_dev_t *LcdNt35510Create(uint16_t x_size, uint16_t y_size)
{
    lcd_dev = (lcd_dev_t *)malloc(sizeof(lcd_dev_t));
    // LCD_LOG("lcd_dev_t LcdNt35510Create\n");
    if(lcd_dev == NULL) {
        LCD_LOG("LCD dev malloc fail\n");
        goto error;
    }
    memset(lcd_dev, 0, sizeof(lcd_dev_t));
    lcd_dev->x_size = x_size;
    lcd_dev->y_size = y_size;
    lcd_dev->xset_cmd = 0x2a00;
    lcd_dev->yset_cmd = 0x2b00;
    lcd_dev->pix = sizeof(uint16_t);
    uint16_t *p = (uint16_t *)LCD_BUF_ADDR;//malloc(sizeof(uint16_t) * 800);
    if(p == NULL) {
        LCD_LOG("malloc fail\n");
        goto error;
    }
    lcd_dev->lcd_buf = p;
    lcd_interface_cfg();
    lcd_api_register();
    nt35510_init();
    // lcd_set_window(0x60);
    // lcd_set_window(0x10);
    // lcd_fill_screen(0xFFFFFF);
    // lcd_fill_screen(0xFF00FF);
    return lcd_dev;
error:
    if(lcd_dev) {
        free(lcd_dev);
    }
    return NULL;
}