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
#include "ugfx_test.h"

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

#include "../resource/include/ppc.h"
static const char *TAG = "espnow_contrl_example";

font_t font32;

static GListener gl;
static GHandle ghFrame1, ghFrame2;
static GHandle ghSliderR, ghSliderG, ghSliderB;
static GHandle ghButton1, ghButtonColor, ghCheckbox1, ghRadio1;
static GHandle ghButtonAir, ghButtonFan, ghButtonRef, ghButtonScreen, ghButtonLight;
static GHandle ghWindow1, ghContainer;
static gdispImage img_red_slider, img_green_slider, img_blue_slider, img_slider_mouse, imgLightOnOff, img_color, img_background;
static gdispImage img_air, img_fan, img_ref, img_screen, img_screen_light;

#define GWIN_FLG_SYSENABLED 0x00000800 /**< The window is enabled after parents are tested */
#define GROUP_YESNO 1

typedef struct slider_image {
    gdispImage *slider_image;
    gdispImage *mouse_image;
} slider_image_t;

// system
QueueHandle_t light_queue = NULL;

void create_widgets();

void _updateColor(void)
{
    uint32_t color;

    color = (unsigned)gwinSliderGetPosition(ghSliderR) << 16;
    color |= (unsigned)gwinSliderGetPosition(ghSliderG) << 8;
    color |= (unsigned)gwinSliderGetPosition(ghSliderB) << 0;

    gwinSetBgColor(ghWindow1, HTML2COLOR(color));
    gwinClear(ghWindow1);
}

void gwinButtonDraw_Toggle_Image(GWidgetObject *gw, void *param)
{
    coord_t sy;
    static bool pressed = TRUE;

    if (!(gw->g.flags & GWIN_FLG_SYSENABLED)) {
        sy = 2 * gw->g.height;
    } else if ((gw->g.flags & GBUTTON_FLG_PRESSED)) {
        sy = gw->g.height;
        pressed = !pressed;
    } else {
        sy = 0;
    }

    if (pressed) {
        sy = 0;
    } else {
        sy = gw->g.height;
    }

    gdispGImageDraw(gw->g.display, (gdispImage *)param, gw->g.x, gw->g.y, gw->g.width, gw->g.height, 0, sy);
}

void gwinSliderDraw_Mouse_Image(GWidgetObject *gw, void *param)
{
#define gsw ((GSliderObject *)gw)
#define gi ((gdispImage *)param)
    const GColorSet *pcol;
    coord_t z, v;

    if ((gw->g.flags & GWIN_FLG_SYSENABLED)) {
        pcol = &gw->pstyle->enabled;
    } else {
        pcol = &gw->pstyle->disabled;
    }

    if (gw->g.width < gw->g.height) {
        // Vertical slider
    } else {
        // Horizontal slider
        gdispGImageDraw(gw->g.display, gi, gw->g.x, gw->g.y, gw->g.width, gw->g.height, 0, 0);
        gdispGDrawStringBox(gw->g.display, gw->g.x + 1, gw->g.y + 1, gw->g.width - 2, gw->g.height - 2, gw->text, gw->g.font, pcol->text, justifyCenter);
        gdispGImageDraw(gw->g.display, &img_slider_mouse, gw->g.x + gsw->dpos - 8, gw->g.y, (&img_slider_mouse)->width, (&img_slider_mouse)->width, 0, 0);
    }

#undef gsw
#undef gi
}

void creat_light_widgets(void)
{
    GWidgetInit wi;

    // Apply some default values for GWIN
    gwinWidgetClearInit(&wi);
    wi.g.show = FALSE;
    // wi.g.show = TRUE;

    // Apply the frame parameters
    wi.g.width = gdispGetWidth();
    wi.g.height = gdispGetHeight();
    wi.g.y = 0;
    wi.g.x = 0;
    wi.text = "Mesh App based on uGFX";
    ghFrame2 = gwinFrameCreate(0, &wi, GWIN_FRAME_CLOSE_BTN | GWIN_FRAME_KEEPONCLOSE);
    // ghFrame2 = gwinFrameCreate(0, &wi, NULL);

    // Apply the button parameters
    gdispImageError err = gdispImageOpenFile(&imgLightOnOff, "light_on_off_128.png");
    printf("err: %x\n", err);
    wi.customDraw = gwinButtonDraw_Toggle_Image;
    wi.customParam = &imgLightOnOff;
    wi.g.parent = ghFrame2;
    wi.g.show = TRUE;

    wi.g.width = 128;
    wi.g.height = 128;
    wi.g.x = (gdispGetWidth() - wi.g.width * 2) / 3;
    wi.g.y = (gdispGetHeight() - wi.g.height) / 2;
    wi.text = "";
    ghButton1 = gwinButtonCreate(0, &wi);

    // Apply the button parameters
    gdispImageOpenFile(&img_color, "color.png");
    wi.customDraw = gwinButtonDraw_Image;
    wi.customParam = &img_color;
    wi.g.width = 128;
    wi.g.height = 128;
    wi.g.x = wi.g.width + 2 * (gdispGetWidth() - wi.g.width * 2) / 3;
    wi.g.y += 0;
    wi.text = "";
    ghButtonColor = gwinButtonCreate(0, &wi);

    ESP_LOGI("widgets", "imgLightOnOff:%p", &imgLightOnOff);
}

void create_color_frame()
{
    GWidgetInit wi;

    // Apply some default values for GWIN
    gwinWidgetClearInit(&wi);
    wi.g.show = FALSE;
    // wi.g.show = TRUE;

    // Apply the frame parameters
    wi.g.width = gdispGetWidth();
    wi.g.height = gdispGetHeight();
    wi.g.y = 0;
    wi.g.x = 0;
    wi.text = "Colorpicker for light";
    ghFrame1 = gwinFrameCreate(0, &wi, GWIN_FRAME_CLOSE_BTN | GWIN_FRAME_KEEPONCLOSE);

    // Apply the slider parameters
    gdispImageOpenFile(&img_red_slider, "red_slider.png");
    gdispImageOpenFile(&img_slider_mouse, "slider_mouse.png");
    slider_image_t slider_image;
    slider_image.slider_image = &img_red_slider;
    slider_image.mouse_image = &img_slider_mouse;
    wi.customDraw = gwinSliderDraw_Mouse_Image;
    wi.customParam = &img_red_slider;
    wi.g.parent = ghFrame1;
    wi.g.show = TRUE;

    wi.g.width = 200;
    wi.g.height = 20;
    wi.g.x = (gdispGetWidth() - wi.g.width) / 2;
    wi.g.y = 20;
    wi.text = "Red";
    ghSliderR = gwinSliderCreate(0, &wi);
    gwinSliderSetRange(ghSliderR, 0, 255);
    gwinSliderSetPosition(ghSliderR, 180);

    // Apply the slider parameters
    gdispImageOpenFile(&img_green_slider, "green_slider.png");
    wi.customDraw = gwinSliderDraw_Mouse_Image;
    wi.customParam = &img_green_slider;

    wi.g.width = 200;
    wi.g.height = 20;
    wi.g.x = (gdispGetWidth() - wi.g.width) / 2;
    wi.g.y += 30;
    wi.text = "Green";
    ghSliderG = gwinSliderCreate(0, &wi);
    gwinSliderSetRange(ghSliderG, 0, 255);
    gwinSliderSetPosition(ghSliderG, 60);

    // Apply the slider parameters
    gdispImageOpenFile(&img_blue_slider, "blue_slider.png");
    wi.customDraw = gwinSliderDraw_Mouse_Image;
    wi.customParam = &img_blue_slider;

    wi.g.width = 200;
    wi.g.height = 20;
    wi.g.x = (gdispGetWidth() - wi.g.width) / 2;
    wi.g.y += 30;
    wi.text = "Blue";
    ghSliderB = gwinSliderCreate(0, &wi);
    gwinSliderSetRange(ghSliderB, 0, 255);
    gwinSliderSetPosition(ghSliderB, 235);

    // Set slider to return extended events
    gwinSliderSendExtendedEvents(ghSliderR, TRUE);
    gwinSliderSendExtendedEvents(ghSliderG, TRUE);
    gwinSliderSendExtendedEvents(ghSliderB, TRUE);

    // Color Preview
    wi.g.width = 200;
    wi.g.height = 60;
    wi.g.x = (gdispGetWidth() - wi.g.width) / 2;
    wi.g.y += 30;
    ghWindow1 = gwinWindowCreate(0, &wi.g);

    _updateColor();
}

void create_widgets()
{
    GWidgetInit wi;

    // Apply some default values for GWIN
    gwinWidgetClearInit(&wi);
    // wi.g.show = TRUE;
    wi.g.show = FALSE;

    // Apply the container parameters
    wi.g.width = gdispGetWidth();
    wi.g.height = gdispGetHeight();
    wi.g.y = 0;
    wi.g.x = 0;
    wi.text = "Container";
    ghContainer = gwinContainerCreate(0, &wi, 0);
    wi.g.parent = ghContainer;
    wi.g.show = TRUE;

    // Apply the button parameters
    gdispImageOpenFile(&img_air, "air_conditioning_w.bmp");
    gdispImageCache(&img_air);
    wi.customDraw = gwinButtonDraw_Image;
    wi.customParam = &img_air;
    wi.g.width = 128;
    wi.g.height = 128;
    wi.g.x = 26;
    wi.g.y = (gdispGetHeight() - wi.g.height) / 2;
    wi.text = "";
    ghButtonAir = gwinButtonCreate(0, &wi);

    // Apply the button parameters
    gdispImageOpenFile(&img_fan, "fan_w.bmp");
    gdispImageCache(&img_fan);
    wi.customDraw = gwinButtonDraw_Image;
    wi.customParam = &img_fan;
    wi.g.width = 128;
    wi.g.height = 128;
    wi.g.x += 154;
    wi.g.y += 0;
    wi.text = "";
    ghButtonFan = gwinButtonCreate(0, &wi);

    // Apply the button parameters
    gdispImageOpenFile(&img_ref, "refrigerator_w.bmp");
    gdispImageCache(&img_ref);
    wi.customDraw = gwinButtonDraw_Image;
    wi.customParam = &img_ref;
    wi.g.width = 128;
    wi.g.height = 128;
    wi.g.x += 154;
    wi.g.y += 0;
    wi.text = "";
    ghButtonRef = gwinButtonCreate(0, &wi);

    // Apply the button parameters
    gdispImageOpenFile(&img_screen, "screen_w.bmp");
    gdispImageCache(&img_screen);
    wi.customDraw = gwinButtonDraw_Image;
    wi.customParam = &img_screen;
    wi.g.width = 128;
    wi.g.height = 128;
    wi.g.x += 154;
    wi.g.y += 0;
    wi.text = "";
    ghButtonScreen = gwinButtonCreate(0, &wi);

    // Apply the button parameters
    gdispImageOpenFile(&img_screen_light, "screen_light_w.bmp");
    gdispImageCache(&img_screen_light);
    wi.customDraw = gwinButtonDraw_Image;
    wi.customParam = &img_screen_light;
    wi.g.width = 128;
    wi.g.height = 128;
    wi.g.x += 154;
    wi.g.y += 0;
    wi.text = "";
    ghButtonLight = gwinButtonCreate(0, &wi);

    gwinShow(ghContainer);
}

#include "lcd_adapter.h"
int ugfx_testcase_pixel_second(void)
{
    coord_t width, height;
    uint16_t i, j;
    uint64_t x = 0;
    bool second = true;
    char string_display[50];

    font32 = gdispOpenFont("DejaVuSans32");

    // Initialize and clear the display
    gfxInit();
    gdispClear(Red);
#if (GDISP_NEED_AUTOFLUSH == GFXOFF)
    gdispFlush();
#endif
    gfxSleepMilliseconds(500);
    gdispClear(Green);
#if (GDISP_NEED_AUTOFLUSH == GFXOFF)
    gdispFlush();
#endif
    gfxSleepMilliseconds(500);
    gdispClear(Blue);
#if (GDISP_NEED_AUTOFLUSH == GFXOFF)
    gdispFlush();
#endif
    gfxSleepMilliseconds(500);

    // Get the screen size
    width = gdispGetWidth();
    height = gdispGetHeight();

    uint32_t time = 0;
    time = xTaskGetTickCount();
    while (second) {
        i = rand() % (width - 1);
        j = rand() % (height - 1);
        x += i * j;
        gdispFillArea(rand() % (width - 1), rand() % (height - 1), i, j, rand() % (0xffff));
#if (GDISP_NEED_AUTOFLUSH == GFXOFF)
        gdispFlush();
#endif
        if ((xTaskGetTickCount() - time) > 3000 / portTICK_RATE_MS) {
            gdispClear(HTML2COLOR(0x88ff88));
            sprintf(string_display, "Pixel/Second: %lld", x / 3);
            printf("Pixel/Second: %lld\n", x / 3);
            gdispDrawString(width / 2 - 50, height / 2 - 5, string_display, font32, White);
#if (GDISP_NEED_AUTOFLUSH == GFXOFF)
            gdispFlush();
#endif
            gfxSleepMilliseconds(1500);
            time = xTaskGetTickCount();
            x = 0;
            second = false;
        }
    }
    return 1;
}

int ugfx_bmp_img_test(void)
{
    static gdispImage myImage;
    static gdispImage myImage1;

    coord_t width, height;
    uint16_t i, j;
    uint64_t x = 0;
    bool second = true;
    char string_display[50];

    font32 = gdispOpenFont("DejaVuSans32");

    // Initialize and clear the display
    gfxInit();

    // Get the screen size
    width = gdispGetWidth();
    height = gdispGetHeight();

    gdispImageOpenFile(&myImage, "bmpback800.bmp");
    gdispImageCache(&myImage);
    gdispImageOpenFile(&myImage1, "bmp1800.bmp");
    gdispImageCache(&myImage1);

    while (1) {

        gdispClear(Red);
#if (GDISP_NEED_AUTOFLUSH == GFXOFF)
        gdispFlush();
#endif
        gfxSleepMilliseconds(500);
        gdispClear(Green);
#if (GDISP_NEED_AUTOFLUSH == GFXOFF)
        gdispFlush();
#endif
        gfxSleepMilliseconds(500);
        gdispClear(Blue);
#if (GDISP_NEED_AUTOFLUSH == GFXOFF)
        gdispFlush();
#endif
        gfxSleepMilliseconds(500);

        uint32_t time = 0;
        bool starig = false;
        time = xTaskGetTickCount();
        second = true;
        while (second) {
            // Set up IO for our image
            starig ? gdispImageDraw(&myImage, 0, 0, width, height, 0, 0) : gdispImageDraw(&myImage1, 0, 0, width, height, 0, 0);
            starig = !starig;
            // gdispImageDraw(&myImage1, 0, 0, width, height, 0, 0);
            ++x;
#if (GDISP_NEED_AUTOFLUSH == GFXOFF)
            gdispFlush();
#endif
            if ((xTaskGetTickCount() - time) > 3000 / portTICK_RATE_MS) {
                gdispClear(HTML2COLOR(0x88ff88));
                sprintf(string_display, "Image refresh test frame/Second: %lld", x / 3);
                gdispDrawString(width / 2 - 250, height / 2 - 5, string_display, font32, White);
#if (GDISP_NEED_AUTOFLUSH == GFXOFF)
                gdispFlush();
#endif
                gfxSleepMilliseconds(1500);
                time = xTaskGetTickCount();
                x = 0;
                second = false;
            }
        }
    }
    return 1;
}

// TEST_CASE("uGFX testcase 12: Mesh", "[ugfx][gui][hmi][mesh]")
#define BALLCOLOR1 Red
#define BALLCOLOR2 Yellow
#define WALLCOLOR HTML2COLOR(0x303030)
#define BACKCOLOR HTML2COLOR(0xC0C0C0)
#define FLOORCOLOR HTML2COLOR(0x606060)
#define SHADOWALPHA (255 - 255 * 0.2)

void ugfx_testcase_04(void)
{
    coord_t width, height, x, y, radius, ballx, bally, dx, floor;
    coord_t minx, miny, maxx, maxy;
    coord_t ballcx, ballcy;
    color_t colour;
    float ii, spin, dy, spinspeed, h, f, g;

    // gfxInit();

    width = gdispGetWidth();
    height = gdispGetHeight();

    radius = height / 5 + height % 2 + 1; // The ball radius
    ii = 1.0 / radius;                    // radius as easy math
    floor = height / 5 - 1;               // floor position
    spin = 0.0;                           // current spin angle on the ball
    spinspeed = 0.1;                      // current spin speed of the ball
    ballx = width / 2;                    // ball x position (relative to the ball center)
    bally = height / 4;                   // ball y position (relative to the ball center)
    dx = .01 * width;                     // motion in the x axis
    dy = 0.0;                             // motion in the y axis
    ballcx = 12 * radius / 5;             // ball x diameter including the shadow
    ballcy = 21 * radius / 10;            // ball y diameter including the shadow

    minx = miny = 0;
    maxx = width;
    maxy = height; // The clipping window for this frame.

    uint32_t time = 0;
    bool testcase = true;
    time = xTaskGetTickCount();
    while (testcase) {
        // Draw one frame
        gdispStreamStart(minx, miny, maxx - minx, maxy - miny);
        for (y = miny; h = (bally - y) * ii, y < maxy; y++) {
            for (x = minx; x < maxx; x++) {
                g = (ballx - x) * ii;
                f = -.3 * g + .954 * h;
                if (g * g < 1 - h * h) {
                    /* The inside of the ball */
                    if ((((int)(9 - spin + (.954 * g + .3 * h) * invsqrt(1 - f * f)) + (int)(2 + f * 2)) & 1)) {
                        colour = BALLCOLOR1;
                    } else {
                        colour = BALLCOLOR2;
                    }
                } else {
                    // The background (walls and floor)
                    if (y > height - floor) {
                        if (x < height - y || height - y > width - x) {
                            colour = WALLCOLOR;
                        } else {
                            colour = FLOORCOLOR;
                        }
                    } else if (x < floor || x > width - floor) {
                        colour = WALLCOLOR;
                    } else {
                        colour = BACKCOLOR;
                    }

                    // The ball shadow is darker
                    if (g * (g + .4) + h * (h + .1) < 1) {
                        colour = gdispBlendColor(colour, Black, SHADOWALPHA);
                    }
                }
                gdispStreamColor(colour); /* pixel to the LCD */
            }
        }
        gdispStreamStop();

        // Force a display update if the controller supports it
        gdispFlush();

        // Calculate the new frame size (note this is a drawing optimisation only)
        minx = ballx - radius;
        miny = bally - radius;
        maxx = minx + ballcx;
        maxy = miny + ballcy;
        if (dx > 0) {
            maxx += dx;
        } else {
            minx += dx;
        }
        if (dy > 0) {
            maxy += dy;
        } else {
            miny += dy;
        }
        if (minx < 0) {
            minx = 0;
        }
        if (maxx > width) {
            maxx = width;
        }
        if (miny < 0) {
            miny = 0;
        }
        if (maxy > height) {
            maxy = height;
        }

        // Motion
        spin += spinspeed;
        ballx += dx;
        bally += dy;
        dx = ballx < radius || ballx > width - radius ? spinspeed = -spinspeed, -dx : dx;
        dy = bally > height - 1.75 * floor ? -.04 * height : dy + .002 * height;

        if ((xTaskGetTickCount() - time) > 20000 / portTICK_RATE_MS) {
            printf("ugfx_testcase_4\n");
            time = xTaskGetTickCount();
            testcase = false;
        }
    }
    gdispClear(White);
    create_widgets();
}

void ugfx_testcase_12(void)
{
    GEvent *pe;
    bool on_off = true;

    // Initialize the display
    gfxInit();
    // ugfx_gui_gwin();

    // Set the widget defaults
    gwinSetDefaultFont(gdispOpenFont("*"));
    gwinSetDefaultStyle(&WhiteWidgetStyle, FALSE);
    gdispClear(White);
#if (GDISP_NEED_AUTOFLUSH == GFXOFF)
    gdispFlush();
#endif

    // create the widget
    create_widgets();
    create_color_frame();
    creat_light_widgets();

    // We want to listen for widget events
    geventListenerInit(&gl);
    gwinAttachListener(&gl);

    // ugfx_gui_gwin();
    // #if (GDISP_NEED_AUTOFLUSH == GFXOFF)
    // gdispFlush();
    // #endif

    while (1) {
#if (GDISP_NEED_AUTOFLUSH == GFXOFF)
        gdispFlush();
#endif
        // Get an Event
        pe = geventEventWait(&gl, TIME_INFINITE);

        switch (pe->type) {
        case GEVENT_GWIN_SLIDER:
            if (((GEventGWinSlider *)pe)->gwin == ghSliderR || ghSliderG || ghSliderB) {
                _updateColor();
                // on_off ? light_color() : NULL;
            }
            break;
        case GEVENT_GWIN_BUTTON:
            if (((GEventGWinButton *)pe)->gwin == ghButton1) {
                if (!on_off) {
                    // light_on();
                    on_off = true;
                } else if (on_off) {
                    // light_off();
                    on_off = false;
                }
            } else if (((GEventGWinButton *)pe)->gwin == ghButtonAir) {
                gwinHide(ghContainer);
                // gwinDisable(ghContainer);
                ugfx_gui_gwin();
                gwinShow(ghContainer);
                // gwinEnable(ghContainer);
            } else if (((GEventGWinButton *)pe)->gwin == ghButtonFan) {
                gwinHide(ghContainer);
                // gwinDisable(ghContainer);
                ugfx_gui_polygons();
                gwinShow(ghContainer);
                // gwinEnable(ghContainer);
            } else if (((GEventGWinButton *)pe)->gwin == ghButtonRef) {
                gwinHide(ghContainer);
                // gwinDisable(ghContainer);
                ugfx_gui_tetris();
                gwinShow(ghContainer);
                // gwinEnable(ghContainer);
            } else if (((GEventGWinButton *)pe)->gwin == ghButtonScreen) {
                gwinHide(ghContainer);
                // gwinDisable(ghContainer);
                ugfx_testcase_pixel_second();
                // ugfx_bmp_img_test();
                // create_color_frame();
                // gwinShow(ghFrame2);
                gwinShow(ghContainer);
                // gwinEnable(ghContainer);
            } else if (((GEventGWinButton *)pe)->gwin == ghButtonLight) {
                gwinHide(ghContainer);
                // gwinDisable(ghContainer);
                // ugfx_testcase_04();
                gwinShow(ghFrame1);
                gwinEnable(ghFrame1);
            } else if (((GEventGWinButton *)pe)->gwin == ghButtonColor) {
                gwinHide(ghContainer);
                // gwinDisable(ghContainer);
                // create_color_frame();
                gwinShow(ghContainer);
                // gwinEnable(ghContainer);
            }
            break;
        case GEVENT_GWIN_CLOSE:
            if (((GEventGWin *)pe)->gwin == ghFrame1) {
                gwinHide(ghFrame1);
                // gwinDisable(ghFrame1);
                gwinShow(ghContainer);
                // gwinEnable(ghContainer);
            } else if (((GEventGWin *)pe)->gwin == ghFrame2) {
                gwinHide(ghFrame2);
                // gwinEnable(ghContainer);
            }
            break;
        default:
            // Oops - not a event.
            break;
        }
    }
}

void app_main()
{
    // main();
    // ugfx_testcase_04();
    ugfx_testcase_12();
    // ugfx_testcase_pixel_second();
    // ugfx_bmp_img_test();
    // ugfx_gui_gwin();
}
