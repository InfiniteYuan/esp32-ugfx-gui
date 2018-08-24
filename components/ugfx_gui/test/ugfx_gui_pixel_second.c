/*
* ESPRESSIF MIT License
*
* Copyright (c) 2018 <ESPRESSIF SYSTEMS (SHANGHAI) PTE LTD>
*
* Permission is hereby granted for use on all ESPRESSIF SYSTEMS products, in which case,
* it is free of charge, to any person obtaining a copy of this software and associated
* documentation files (the "Software"), to deal in the Software without restriction, including
* without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense,
* and/or sell copies of the Software, and to permit persons to whom the Software is furnished
* to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in all copies or
* substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
* FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
* COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
* IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
* CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*
*/

#include "iot_ugfx.h"

#include "mdf_common.h"
#include "mdf_espnow.h"

#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <assert.h>
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "freertos/timers.h"
#include "nvs_flash.h"
#include "esp_event_loop.h"
#include "tcpip_adapter.h"
#include "esp_wifi.h"
#include "esp_log.h"
#include "esp_system.h"
#include "esp_now.h"
#include "rom/ets_sys.h"
#include "rom/crc.h"
#include "unity.h"

int ugfx_testcase_pixel_second(void) 
{
    coord_t     width, height;
    uint16_t     i, j;
    uint32_t    x = 0;
    bool second = true;
    char string_display[50];
    font_t      font1;
    uint32_t time = 0;
    font1 = gdispOpenFont("DejaVu*");

    // Initialize and clear the display
    gfxInit();
    gdispClear(Red);
    gfxSleepMilliseconds(1000);
    gdispClear(Green);
    gfxSleepMilliseconds(1000);
    gdispClear(Blue);
    gfxSleepMilliseconds(1000);

    // Get the screen size
    width = gdispGetWidth();
    height = gdispGetHeight();

    time = xTaskGetTickCount();
    while (second) {
        i = rand()%(width-1);
        j = rand()%(height-1);
        x += i * j;
        gdispFillArea(rand()%(width-1), rand()%(height-1), i, j, rand()%(0xffff));
        // gfxSleepMilliseconds(50);
        if((xTaskGetTickCount() - time) > 3000 / portTICK_RATE_MS ) {
            gdispClear(HTML2COLOR(0x88ff88));
            sprintf(string_display, "Pixel/Second: %d", x/3);
            gdispDrawString(width/2-150, height/2-20, string_display, font1, Red);
            gfxSleepMilliseconds(500);
            second = false;
        }
    }
    return 1;
}


TEST_CASE("uGFX testcase 13: pixel_second", "[ugfx][gui][hmi]")
{
    ugfx_testcase_pixel_second();
}