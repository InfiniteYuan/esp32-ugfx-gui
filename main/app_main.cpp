/*
 * ESPRESSIF MIT License
 *
 * Copyright (c) 2017 <ESPRESSIF SYSTEMS (SHANGHAI) PTE LTD>
 *
 * Permission is hereby granted for use on ESPRESSIF SYSTEMS products only, in which case,
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
#include "iot_xpt2046.h"
#include "lwip/api.h"
#include "bitmap.h"
#include "iot_lcd.h"
#include "iot_wifi_conn.h"
#include "nvs_flash.h"
#include "esp_event_loop.h"
#include "freertos/event_groups.h"
#include "board_ST7735.h"
//#include "include/app_touch.h"
#include "gfx.h"

static const char* TAG = "ESP-CAM";
CXpt2046 *xpt;
CEspLcd* tft = NULL;

void app_lcd_init()
{
    lcd_conf_t lcd_pins = {
        .lcd_model = LCD_MOD_ST7789, //LCD_MOD_ILI9341,//LCD_MOD_ST7789,
        .pin_num_miso = GPIO_NUM_25,
        .pin_num_mosi = GPIO_NUM_23,
        .pin_num_clk = GPIO_NUM_19,
        .pin_num_cs = GPIO_NUM_22,
        .pin_num_dc = GPIO_NUM_21,
        .pin_num_rst = GPIO_NUM_18,
        .pin_num_bckl = GPIO_NUM_5,
        .clk_freq = 30 * 1000 * 1000,
        .rst_active_level = 0,
        .bckl_active_level = 1,
        .spi_host = HSPI_HOST,
    };

    /*Initialize SPI Handler*/
    if (tft == NULL) {
        tft = new CEspLcd(&lcd_pins);
//        camera_queue = xQueueCreate(CAMERA_CACHE_NUM - 2, sizeof(camera_evt_t));
    }

    /*screen initialize*/
    tft->invertDisplay(false);
    tft->setRotation(1);
//    tft->fillScreen(COLOR_GREEN);
//    tft->drawBitmap(0, 0, esp_logo, 137, 26);
    tft->drawString("Status: Initialize touch ...", 5, 50);
}

/******** pixmap *********/
#define PIXMAP_WIDTH    40
#define PIXMAP_HEIGHT   10
/******** pixmap *********/

/*********** GWIN button ************
static GListener gl;
static GHandle   ghButton1;

static void createWidgets(void) {
    GWidgetInit wi;

    // Apply some default values for GWIN
    gwinWidgetClearInit(&wi);
    wi.g.show = TRUE;

    // Apply the button parameters
    wi.g.width = 100;
    wi.g.height = 30;
    wi.g.y = 10;
    wi.g.x = 10;
    wi.text = "Push Button";

    // Create the actual button
    ghButton1 = gwinButtonCreate(0, &wi);
}
*/

/*********** GWIN frame ********************
static GListener    gl;
static GHandle      ghFrame1;
static GHandle      ghSliderR, ghSliderG, ghSliderB;
static GHandle      ghButton1, ghButton2, ghButton3;
static GHandle      ghWindow1;

static void _updateColor(void) {
    uint32_t color;

    color  = (unsigned)gwinSliderGetPosition(ghSliderR) << 16;
    color |= (unsigned)gwinSliderGetPosition(ghSliderG) <<  8;
    color |= (unsigned)gwinSliderGetPosition(ghSliderB) <<  0;

    gwinSetBgColor(ghWindow1, HTML2COLOR(color));
    gwinClear(ghWindow1);
}

static void _createWidgets(void) {
    GWidgetInit wi;

    // Apply some default values for GWIN
    gwinWidgetClearInit(&wi);
    wi.g.show = TRUE;

    // Create a surprise label behind the frame window
    wi.g.width = 100;
    wi.g.height = 20;
    wi.g.y = 100;
    wi.g.x = 150;
    wi.text = "Surprise!";
    gwinLabelCreate(0, &wi);

    // Apply the frame parameters
    wi.g.width = 300;
    wi.g.height = 200;
    wi.g.y = 10;
    wi.g.x = 10;
    wi.text = "Colorpicker";
    ghFrame1 = gwinFrameCreate(0, &wi, GWIN_FRAME_BORDER | GWIN_FRAME_CLOSE_BTN | GWIN_FRAME_MINMAX_BTN);

    // Apply the button parameters
    wi.g.width = 60;
    wi.g.height = 20;
    wi.g.x = 10;
    wi.g.y = 10;
    wi.text = "Random";
    wi.g.parent = ghFrame1;
    ghButton1 = gwinButtonCreate(0, &wi);

    // Apply the slider parameters
    wi.g.width = 200;
    wi.g.height = 20;
    wi.g.x = 80;
    wi.g.y += 0;
    wi.text = "Red";
    wi.g.parent = ghFrame1;
    ghSliderR = gwinSliderCreate(0, &wi);
    gwinSliderSetRange(ghSliderR, 0, 255);
    gwinSliderSetPosition(ghSliderR, 180);

    // Apply the button parameters
    wi.g.width = 60;
    wi.g.height = 20;
    wi.g.x = 10;
    wi.g.y += 25;
    wi.text = "Random";
    wi.g.parent = ghFrame1;
    ghButton2 = gwinButtonCreate(0, &wi);

    // Apply the slider parameters
    wi.g.width = 200;
    wi.g.height = 20;
    wi.g.x = 80;
    wi.g.y += 0;
    wi.text = "Green";
    wi.g.parent = ghFrame1;
    ghSliderG = gwinSliderCreate(0, &wi);
    gwinSliderSetRange(ghSliderG, 0, 255);
    gwinSliderSetPosition(ghSliderG, 60);

    // Apply the button parameters
    wi.g.width = 60;
    wi.g.height = 20;
    wi.g.x = 10;
    wi.g.y += 25;
    wi.text = "Random";
    wi.g.parent = ghFrame1;
    ghButton3 = gwinButtonCreate(0, &wi);

    // Apply the slider parameters
    wi.g.width = 200;
    wi.g.height = 20;
    wi.g.x = 80;
    wi.g.y += 0;
    wi.text = "Blue";
    wi.g.parent = ghFrame1;
    ghSliderB = gwinSliderCreate(0, &wi);
    gwinSliderSetRange(ghSliderB, 0, 255);
    gwinSliderSetPosition(ghSliderB, 235);

    // Color Preview
    wi.g.width = 270;
    wi.g.height = 65;
    wi.g.x = 10;
    wi.g.y = 90;
    ghWindow1 = gwinWindowCreate(0, &wi.g);

    _updateColor();
}
*/

/************ GWIN radio *********
#define GROUP1      0   // this will be the first radio button group
#define GROUP2      1   // this will be the second radio button group

static GListener    gl;
static GHandle      ghRadio11, ghRadio12, ghRadio13;    // the first group
static GHandle      ghRadio21, ghRadio22, ghRadio23;    // the second group

static void createWidgets(void) {
    GWidgetInit wi;

    // Apply some default values for GWIN
    gwinWidgetClearInit(&wi);
    wi.g.show = TRUE;

    // create Radio11
    wi.g.y = 10; wi.g.x = 10; wi.g.width = 80; wi.g.height = 20; wi.text = "Radio 1";
    ghRadio11 = gwinRadioCreate(0, &wi, GROUP1);

    // create Radio12
    wi.g.y = 50; wi.g.x = 10; wi.g.width = 80; wi.g.height = 20; wi.text = "Radio 2";
    ghRadio12 = gwinRadioCreate(0, &wi, GROUP1);

    // create Radio13
    wi.g.y = 90; wi.g.x = 10; wi.g.width = 80; wi.g.height = 20; wi.text = "Radio 3";
    ghRadio13 = gwinRadioCreate(0, &wi, GROUP1);

    // create Radio21
    wi.g.y = 10; wi.g.x = 120; wi.g.width = 80; wi.g.height = 20; wi.text = "Radio 1";
    ghRadio21 = gwinRadioCreate(0, &wi, GROUP2);

    // create Radio22
    wi.g.y = 50; wi.g.x = 120; wi.g.width = 80; wi.g.height = 20; wi.text = "Radio 2";
    ghRadio22 = gwinRadioCreate(0, &wi, GROUP2);

    // create Radio23
    wi.g.y = 90; wi.g.x = 120; wi.g.width = 80; wi.g.height = 20; wi.text = "Radio 3";
    ghRadio23 = gwinRadioCreate(0, &wi, GROUP2);

    // Check the first radio buttons
    gwinRadioPress(ghRadio11);
    gwinRadioPress(ghRadio21);
}
*/

/********** GWIN keyboard **************
// The variables we need
static font_t       font;
static GListener    gl;
static GHandle      ghConsole;
static GHandle      ghKeyboard;


// Create the widgets.
static void createWidgets(void) {
    GWidgetInit     wi;

    gwinWidgetClearInit(&wi);

    // Create the console - set colors before making it visible
    wi.g.show = FALSE;
    wi.g.x = 0; wi.g.y = 0;
    wi.g.width = gdispGetWidth(); wi.g.height = gdispGetHeight()/2;
    ghConsole = gwinConsoleCreate(0, &wi.g);
    gwinSetColor(ghConsole, Black);
    gwinSetBgColor(ghConsole, HTML2COLOR(0xF0F0F0));
    gwinShow(ghConsole);
    gwinClear(ghConsole);

    // Create the keyboard
    wi.g.show = TRUE;
    wi.g.x = 0; wi.g.y = gdispGetHeight()/2;
    wi.g.width = gdispGetWidth(); wi.g.height = gdispGetHeight()/2;
    ghKeyboard = gwinKeyboardCreate(0, &wi);
}
*/

/********** GWIN textedit_virtual_keyboard ********
static GListener    gl;
static GHandle      ghTextedit1;
static GHandle      ghTextedit2;
static GHandle      ghTextedit3;
static GHandle      ghKeyboard;

static void guiCreate(void)
{
    GWidgetInit wi;
    gwinWidgetClearInit(&wi);

    // TextEdit1
    wi.g.show = TRUE;
    wi.g.x = 10;
    wi.g.y = 20;
    wi.g.width = 200;
    wi.g.height = 35;
    wi.text = "Use the TAB key";
    ghTextedit1 = gwinTexteditCreate(0, &wi, 100);

    // TextEdit2
    wi.g.show = TRUE;
    wi.g.x = 10;
    wi.g.y = 100;
    wi.g.width = 200;
    wi.g.height = 35;
    wi.text = "to switch between";
    ghTextedit2 = gwinTexteditCreate(0, &wi, 20);

    // TextEdit3
    wi.g.show = TRUE;
    wi.g.x = 10;
    wi.g.y = 180;
    wi.g.width = 200;
    wi.g.height = 35;
    wi.text = "the different widgets";
    ghTextedit3 = gwinTexteditCreate(0, &wi, 100);

    // Virtual keyboard
    wi.g.show = TRUE;
    wi.g.x = 0;
    wi.g.y = (gdispGetHeight()/2);
    wi.g.width = gdispGetWidth();
    wi.g.height = gdispGetHeight()/2;
    ghKeyboard = gwinKeyboardCreate(0, &wi);
}
*/

/********** GWIN container_nested**********
static GListener    gl;
static GHandle      ghContainer1, ghContainer2, ghContainer3;
static GHandle      ghButton1, ghButton2, ghButton3, ghButton4;
static GHandle      ghCheckbox1, ghCheckbox2, ghCheckbox3;

static void createWidgets(void) {
    GWidgetInit wi;

    // Apply some default values for GWIN
    gwinWidgetClearInit(&wi);
    wi.g.show = FALSE;

    // Container 1
    wi.g.width = 300;
    wi.g.height = 220;
    wi.g.y = 40;
    wi.g.x = 10;
    wi.text = "Container 1";
    ghContainer1 = gwinContainerCreate(0, &wi, GWIN_CONTAINER_BORDER);

    wi.g.show = TRUE;

    // Container 2
    wi.g.width = 100;
    wi.g.height = 150;
    wi.g.y = 55;
    wi.g.x = 10;
    wi.g.parent = ghContainer1;
    wi.text = "Container 2";
    ghContainer2 = gwinContainerCreate(0, &wi, GWIN_CONTAINER_BORDER);

    // Container 3
    wi.g.width = 100;
    wi.g.height = 150;
    wi.g.y = 55;
    wi.g.x = 185;
    wi.g.parent = ghContainer1;
    wi.text = "Container 3";
    ghContainer3 = gwinContainerCreate(0, &wi, GWIN_CONTAINER_BORDER);

    // Button 1
    wi.g.width = 80;
    wi.g.height = 20;
    wi.g.y = 10;
    wi.g.x = 10;
    wi.text = "Button 1";
    wi.g.parent = ghContainer2;
    ghButton1 = gwinButtonCreate(0, &wi);

    // Button 2
    wi.g.width = 80;
    wi.g.height = 20;
    wi.g.y = 40;
    wi.g.x = 10;
    wi.text = "Button 2";
    wi.g.parent = ghContainer2;
    ghButton2 = gwinButtonCreate(0, &wi);

    // Button 3
    wi.g.width = 80;
    wi.g.height = 20;
    wi.g.y = 10;
    wi.g.x = 10;
    wi.text = "Button 3";
    wi.g.parent = ghContainer3;
    ghButton3 = gwinButtonCreate(0, &wi);

    // Button 4
    wi.g.width = 80;
    wi.g.height = 20;
    wi.g.y = 40;
    wi.g.x = 10;
    wi.text = "Button 4";
    wi.g.parent = ghContainer3;
    ghButton4 = gwinButtonCreate(0, &wi);

    // Checkbox 1
    wi.g.width = 80;
    wi.g.height = 20;
    wi.g.y = 25;
    wi.g.x = 10;
    wi.text = " Visible C2";
    wi.g.parent = ghContainer1;
    ghCheckbox1 = gwinCheckboxCreate(0, &wi);

    // Checkbox 2
    wi.g.width = 80;
    wi.g.height = 20;
    wi.g.y = 25;
    wi.g.x = 185;
    wi.text = " Visible C3";
    wi.g.parent = ghContainer1;
    ghCheckbox2 = gwinCheckboxCreate(0, &wi);

    // Checkbox 3
    wi.g.width = 80;
    wi.g.height = 20;
    wi.g.y = 10;
    wi.g.x = 10;
    wi.text = " Visible C1";
    wi.g.parent = 0;
    ghCheckbox3 = gwinCheckboxCreate(0, &wi);


    // Make the container become visible - therefore all its children
    // become visible as well
    gwinShow(ghContainer1);
}
*/

/********** DEMO notepad ********************
#define COLOR_SIZE  20
#define PEN_SIZE    20
#define OFFSET      3

#define COLOR_BOX(a)        (ev.x >= a && ev.x <= a + COLOR_SIZE)
#define PEN_BOX(a)          (ev.y >= a && ev.y <= a + COLOR_SIZE)
#define GET_COLOR(a)        (COLOR_BOX(a * COLOR_SIZE + OFFSET))
#define GET_PEN(a)          (PEN_BOX(a * 2 * PEN_SIZE + OFFSET))
#define DRAW_COLOR(a)       (a * COLOR_SIZE + OFFSET)
#define DRAW_PEN(a)         (a * 2 * PEN_SIZE + OFFSET)
#define DRAW_AREA(x, y)     (x >= PEN_SIZE + OFFSET + 3 && x <= gdispGetWidth() && \
                             y >= COLOR_SIZE + OFFSET + 3 && y <= gdispGetHeight())

void drawScreen(void) {
    char *msg = "uGFX";
    font_t      font1, font2;

    font1 = gdispOpenFont("DejaVuSans24*");
    font2 = gdispOpenFont("DejaVuSans12*");

    gdispClear(White);
    gdispDrawString(gdispGetWidth()-gdispGetStringWidth(msg, font1)-3, 3, msg, font1, Black);

    gdispFillArea(0 * COLOR_SIZE + 3, 3, COLOR_SIZE, COLOR_SIZE, Black);
    gdispFillArea(1 * COLOR_SIZE + 3, 3, COLOR_SIZE, COLOR_SIZE, Red);
    gdispFillArea(2 * COLOR_SIZE + 3, 3, COLOR_SIZE, COLOR_SIZE, Yellow);
    gdispFillArea(3 * COLOR_SIZE + 3, 3, COLOR_SIZE, COLOR_SIZE, Green);
    gdispFillArea(4 * COLOR_SIZE + 3, 3, COLOR_SIZE, COLOR_SIZE, Blue);
    gdispDrawBox (5 * COLOR_SIZE + 3, 3, COLOR_SIZE, COLOR_SIZE, Black);

    gdispFillStringBox(OFFSET * 2, DRAW_PEN(1), PEN_SIZE, PEN_SIZE, "1", font2, White, Black, justifyCenter);
    gdispFillStringBox(OFFSET * 2, DRAW_PEN(2), PEN_SIZE, PEN_SIZE, "2", font2, White, Black, justifyCenter);
    gdispFillStringBox(OFFSET * 2, DRAW_PEN(3), PEN_SIZE, PEN_SIZE, "3", font2, White, Black, justifyCenter);
    gdispFillStringBox(OFFSET * 2, DRAW_PEN(4), PEN_SIZE, PEN_SIZE, "4", font2, White, Black, justifyCenter);
    gdispFillStringBox(OFFSET * 2, DRAW_PEN(5), PEN_SIZE, PEN_SIZE, "5", font2, White, Black, justifyCenter);

    gdispCloseFont(font1);
    gdispCloseFont(font2);
}
static GEvent* pe;
static GListener    gl;
static GSourceHandle            gs;
static GEventMouse              *pem;
static GHandle                  ghc;
static GEventType               deviceType;
*/

/**********DEmO mandelbrot***************
void mandelbrot(float x1, float y1, float x2, float y2) {
    unsigned int i,j, width, height;
    uint16_t iter;
    color_t color;
    float fwidth, fheight;

    float sy = y2 - y1;
    float sx = x2 - x1;
    const int MAX = 512;

    width = (unsigned int)gdispGetWidth();
    height = (unsigned int)gdispGetHeight();
    fwidth = width;
    fheight = height;

    for(i = 0; i < width; i++) {
        for(j = 0; j < height; j++) {
            float cy = j * sy / fheight + y1;
            float cx = i * sx / fwidth + x1;
            float x=0.0f, y=0.0f, xx=0.0f, yy=0.0f;
            for(iter=0; iter <= MAX && xx+yy<4.0f; iter++) {
                xx = x*x;
                yy = y*y;
                y = 2.0f*x*y + cy;
                x = xx - yy + cx;
            }
            //color = ((iter << 8) | (iter&0xFF));
            color = RGB2COLOR(iter<<7, iter<<4, iter);
            gdispDrawPixel(i, j, color);
        }
    }
}
*/

#if 0
extern "C" void app_main()
{
    app_lcd_init();
//    init_xpt2046();
//    lcd_init_wifi();
//
//    CWiFi *my_wifi = CWiFi::GetInstance(WIFI_MODE_STA);
//    printf("connect wifi\n");
//    my_wifi->Connect(WIFI_SSID, WIFI_PASSWORD, portMAX_DELAY);
//    ip4_addr_t s_ip_addr = my_wifi->IP();
//
//    lcd_wifi_connect_complete();
//    // VERY UNSTABLE without this delay after init'ing wifi... however, much more stable with a new Power Supply
//    vTaskDelay(500 / portTICK_RATE_MS);
//    ESP_LOGI(TAG, "Free heap: %u", xPortGetFreeHeapSize());
//
//    sample();
    coord_t     width, height;
    coord_t     i, j;
    coord_t     r1, r2, cx, cy;
    uint8_t     sectors;

    set_semaphore(tft->getSemphor());
    set_spidevice(tft->getSpidevice());
    set_dc(tft->getlcd_dc());

    // Initialize and clear the display
//    gfxInit();

    // Get the screen size
//    width = gdispGetWidth();
//    height = gdispGetHeight();

/*********** basic *****************
 *
    // Code Here
    gdispDrawBox(10, 10, width/2, height/2, Yellow);
    gdispFillArea(width/2, height/2, width/2-10, height/2-10, Blue);
    gdispDrawLine(5, 30, width-50, height-40, Red);

    for(i = 5, j = 0; i < width && j < height; i += 7, j += i/20)
        gdispDrawPixel(i, j, White);
*/

/*********** circle *****************
    // Code Here
//    gdispFillArc(width/2, height/2, width/4, -10, -45, White);
//    gdispDrawCircle(width/2+width/8, height/2-height/8, 13, Green);
//    gdispFillCircle (width/2+width/8, height/2-height/8, 10, Red);
//    gdispDrawArc(width/2+width/8, height/2-height/8, 20, 25, 115, Gray);
//    gdispFillEllipse (width-width/6, height-height/6, width/8, height/16, Blue);
//    gdispDrawEllipse (width-width/6, height-height/6, width/16, height/8, Yellow);
*/

/*********** arcsectors **************
    // Initialize some variables
    r1 = width > height ? height/3 : width/3;
    r2 = r1*3/4;
    cx = width/2;
    cy = height/2;
    sectors = 1;

    while(1) {
        // Draw the arc sectors
        gdispClear(White);
        gdispDrawArcSectors(cx, cy, r1, sectors, Blue);
        gdispFillArcSectors(cx, cy, r2, sectors, Red);

        // Increase the sectors counter
        sectors++;

        // Waste some time
        gfxSleepMilliseconds(250);
    }
*/

/*********** multiple_displays **************
    coord_t     display, cnt;
    font_t      f;
    char        buf[16];

    // Initialize and clear the display
    gfxInit();

    // Get a font to write with
    f = gdispOpenFont("*");

    // Cycle through each display
    cnt = gdispGetDisplayCount();
    for(display = 0; display < cnt; display++) {

        // Set the default display to the specified display
        gdispSetDisplay(gdispGetDisplay(display));

        // Get the screen size
        width = gdispGetWidth();
        height = gdispGetHeight();

        // Draw draw draw
        sprintg(buf, "Display %u", display);
        if (width < 128) {
            gdispDrawBox(0, 0, width/2, height/2, Yellow);
            gdispFillStringBox(0, height/2, width, height/2, buf, f, Black, Blue, justifyCenter);
        } else {
            gdispDrawBox(10, 10, width/2, height/2, Yellow);
            gdispFillStringBox(width/2, height/2, width/2-10, height/2-10, buf, f, White, Blue, justifyCenter);
        }
        gdispDrawLine(5, 30, width-50, height-40, Red);

        for(i = 5, j = 0; i < width && j < height; i += 7, j += i/20)
            gdispDrawPixel(i, j, White);
    }
*/

/*********** pixmap **************
    static GDisplay* pixmap;
    static pixel_t* surface;
    // Initialize and clear the display
    gfxInit();

    // Get the screen size
    width = gdispGetWidth();
    height = gdispGetHeight();

    // Create a pixmap and get a pointer to the bits
    pixmap = gdispPixmapCreate(PIXMAP_WIDTH, PIXMAP_HEIGHT);
    surface = gdispPixmapGetBits(pixmap);

    // A pixmap can be treated either as a virtual display or as a memory framebuffer surface.
    // We demonstrate writing to it using both methods.

    // First demo drawing onto the surface directly
    for(j = 0; j < PIXMAP_HEIGHT; j++)
        for(i = 0; i < PIXMAP_WIDTH; i++)
            surface[j*PIXMAP_WIDTH + i] = RGB2COLOR(0, 255-i*(256/PIXMAP_WIDTH), j*(256/PIXMAP_HEIGHT));

    // Secondly, show drawing a line on it like a virtual display
    gdispGDrawLine(pixmap, 0, 0, gdispGGetWidth(pixmap)-1, gdispGGetHeight(pixmap)-1, White);

    i = j = 0;
    while(TRUE) {
        // Clear the old position
        gdispFillArea(i, j, PIXMAP_WIDTH, PIXMAP_HEIGHT, Black);

        // Change the position
        i += PIXMAP_WIDTH/2;
        if (i >= width - PIXMAP_WIDTH/2) {
            i %= width - PIXMAP_WIDTH/2;
            j = (j + PIXMAP_HEIGHT/2) % (height - PIXMAP_HEIGHT/2);
        }

        // Blit the pixmap to the real display at the new position
        gdispBlitArea(i, j, PIXMAP_WIDTH, PIXMAP_HEIGHT, surface);

        // Wait
        gfxSleepMilliseconds(100);
    }
*/

/************* GWIN basic ****************
    GHandle GW1, GW2;
    gfxInit();
    gdispClear(White);

    // Create two windows
    {
        GWindowInit wi;

        gwinClearInit(&wi);
        wi.show = TRUE; wi.x = 20; wi.y = 10; wi.width = 200; wi.height = 150;
        GW1 = gwinWindowCreate(0, &wi);
        wi.show = TRUE; wi.x = 50; wi.y = 190; wi.width = 150; wi.height = 100;
        GW2 = gwinWindowCreate(0, &wi);
    }

    // Set fore- and background colors for both windows
    gwinSetColor(GW1, Black);
    gwinSetBgColor(GW1, White);
    gwinSetColor(GW2, White);
    gwinSetBgColor(GW2, Blue);

    // Clear both windows - to set background color
    gwinClear(GW1);
    gwinClear(GW2);

    gwinDrawLine(GW1, 5, 30, 150, 110);
    for(i = 5, j = 0; i < 200 && j < 150; i += 3, j += i/20)
            gwinDrawPixel(GW1, i, j);

    //Draw two filled circles at the same coordinate
    //of each window to demonstrate the relative coordinates
    //of windows

    gwinFillCircle(GW1, 20, 20, 15);
    gwinFillCircle(GW2, 20, 20, 15);
*/

/************* GWin button **************
    GEvent* pe;
    static const orientation_t  orients[] = { GDISP_ROTATE_0, GDISP_ROTATE_90, GDISP_ROTATE_180, GDISP_ROTATE_270 };
    unsigned which;

    // Initialize the display
    gfxInit();

    // We are currently at GDISP_ROTATE_0
    which = 1;
    gdispSetOrientation(orients[which]);

    // Set the widget defaults
    gwinSetDefaultFont(gdispOpenFont("UI2"));
    gwinSetDefaultStyle(&WhiteWidgetStyle, FALSE);
    gdispClear(White);

    // create the widget
    ESP_LOGI("XPT2046", "gevencreateWidgetscreateWidgetscreateWidgetscreateWidgetscreateWidgetscreateWidgetstEventWait");
    createWidgets();

    // We want to listen for widget events
    geventListenerInit(&gl);
    gwinAttachListener(&gl);

    while(1) {
        // Get an Event
        ESP_LOGI("XPT2046", "geventEventWait");
        pe = geventEventWait(&gl, TIME_INFINITE);
        ESP_LOGI("XPT2046", "geventEventWait stop");

        switch(pe->type) {
            case GEVENT_GWIN_BUTTON:
                if (((GEventGWinButton*)pe)->gwin == ghButton1) {
                    ESP_LOGI("XPT2046", "GEventGWinButton ghButton1");
                    // Our button has been pressed
//                    if (++which >= sizeof(orients)/sizeof(orients[0]))
//                        which = 0;

                    // Setting the orientation during run-time is a bit naughty particularly with
                    // GWIN windows. In this case however we know that the button is in the top-left
                    // corner which should translate safely into any orientation.
                    gdispSetOrientation(orients[which]);
                    gdispClear(White);
                    gwinRedrawDisplay(GDISP, FALSE);
                }
                break;
            default:
                break;
        }
    }
*/

/************* GWIN frame ************
    GEvent* pe;

    // Initialize the display
    gfxInit();

    // Set the widget defaults
    gwinSetDefaultFont(gdispOpenFont("*"));
    gwinSetDefaultStyle(&WhiteWidgetStyle, FALSE);
    gdispClear(White);

    // create the widget
    _createWidgets();

    // We want to listen for widget events
    geventListenerInit(&gl);
    gwinAttachListener(&gl);

    while(1) {
        // Get an Event
        pe = geventEventWait(&gl, TIME_INFINITE);

        switch(pe->type) {
            case GEVENT_GWIN_SLIDER:
                if (((GEventGWinSlider *)pe)->gwin == ghSliderR || \
                                                      ghSliderG || \
                                                      ghSliderB ) {
                    _updateColor();
                }
                break;

            case GEVENT_GWIN_BUTTON:
                if (((GEventGWinButton *)pe)->gwin == ghButton1) {
                    gwinSliderSetPosition(ghSliderR, rand() % 256);
                } else if (((GEventGWinButton *)pe)->gwin == ghButton2) {
                    gwinSliderSetPosition(ghSliderG, rand() % 256);
                } else if (((GEventGWinButton *)pe)->gwin == ghButton3) {
                    gwinSliderSetPosition(ghSliderB, rand() % 256);
                }

                _updateColor();

            default:
                break;
        }
    }
*/

/************ GWIN RAdio ******
    GEvent* pe;

    // Initialize the display
    gfxInit();

    // Set the widget defaults
    gwinSetDefaultFont(gdispOpenFont("UI2"));
    gwinSetDefaultStyle(&WhiteWidgetStyle, FALSE);
    gdispClear(White);

    // create the widget
    createWidgets();

    // We want to listen for widget events
    geventListenerInit(&gl);
    gwinAttachListener(&gl);

    while(1) {
        // Get an Event
        pe = geventEventWait(&gl, TIME_INFINITE);

        switch(pe->type) {
            case GEVENT_GWIN_RADIO:
                //printf("group: %u   radio: %s\n", ((GEventGWinRadio *)pe)->group, gwinGetText(((GEventGWinRadio *)pe)->gwin));
                break;

            default:
                break;
        }
    }
*/

/************ GWIN Console ************
    GHandle GW1, GW2, GW3;
    font_t  font1, font2;

    // initialize and clear the display
    gfxInit();

    // Set some fonts
    font1 = gdispOpenFont("UI2");
    font2 = gdispOpenFont("DejaVu Sans 12");
    gwinSetDefaultFont(font1);

    // create the three console windows
    {
        GWindowInit     wi;

        gwinClearInit(&wi);
        wi.show = TRUE;
        wi.x = 0; wi.y = 0; wi.width = gdispGetWidth(); wi.height = gdispGetHeight()/2;
        GW1 = gwinConsoleCreate(0, &wi);
        wi.y = gdispGetHeight()/2; wi.width = gdispGetWidth()/2; wi.height = gdispGetHeight();
        GW2 = gwinConsoleCreate(0, &wi);
        wi.x = gdispGetWidth()/2; wi.height = gdispGetHeight();
        GW3 = gwinConsoleCreate(0, &wi);
    }

    // Use a special font for GW1
    gwinSetFont(GW1, font2);

    // Set the fore- and background colors for each console
    gwinSetColor(GW1, Green);
    gwinSetBgColor(GW1, Black);
    gwinSetColor(GW2, White);
    gwinSetBgColor(GW2, Blue);
    gwinSetColor(GW3, Black);
    gwinSetBgColor(GW3, Red);

    // clear all console windows - to set background
    gwinClear(GW1);
    gwinClear(GW2);
    gwinClear(GW3);

    // Output some data on the first console
    for(i = 0; i < 10; i++) {
        gwinPrintf(GW1, "Hello \033buGFX\033B!\n");
    }

    // Output some data on the second console - Fast
    for(i = 0; i < 32; i++) {
        gwinPrintf(GW2, "Message Nr.: \0331\033b%d\033B\033C\n", i+1);
    }

    // Output some data on the third console - Slowly
    for(i = 0; i < 32; i++) {
        gwinPrintf(GW3, "Message Nr.: \033u%d\033U\n", i+1);
        gfxSleepMilliseconds(500);
    }

    // Make console 3 invisible and then visible again to demonstrate the history buffer
    gwinPrintf(GW2, "Making red window \033uinvisible\033U\n");
    gwinSetVisible(GW3, FALSE);
    gfxSleepMilliseconds(1000);
    gwinPrintf(GW2, "Making red window \033uvisible\033U\n");
    gwinSetVisible(GW3, TRUE);
    gwinPrintf(GW3, "\033bI'm back!!!\033B\n", i+1);
**/

/************ GWIN keyboard ************
    GEvent *            pe;
    GEventKeyboard *    pk;

    // Initialize the display
    gfxInit();

    // Set the widget defaults
    font = gdispOpenFont("*");          // Get the first defined font.
    gwinSetDefaultFont(font);
    gwinSetDefaultStyle(&WhiteWidgetStyle, FALSE);
    gdispClear(White);

    // Create the gwin windows/widgets
    createWidgets();

    // We want to listen for widget events
    geventListenerInit(&gl);
    gwinAttachListener(&gl);

    // We also want to listen to keyboard events from the virtual keyboard
    geventAttachSource(&gl, gwinKeyboardGetEventSource(ghKeyboard), GLISTEN_KEYTRANSITIONS|GLISTEN_KEYUP);

    while(1) {
        // Get an Event
        pe = geventEventWait(&gl, TIME_INFINITE);

        switch(pe->type) {
        case GEVENT_GWIN_KEYBOARD:
            // This is a widget event generated on the standard gwin event source
            gwinPrintf(ghConsole, "Keyboard visibility has changed\n");
            break;

        case GEVENT_KEYBOARD:
            // This is a keyboard event from a keyboard source which must be separately listened to.
            // It is not sent on the gwin event source even though in this case it was generated by a gwin widget.
            pk = (GEventKeyboard *)pe;

            gwinPrintf(ghConsole, "KEYSTATE: 0x%04X [ %s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s]",
                pk->keystate,
                (!pk->keystate ? "NONE " : ""),
                ((pk->keystate & GKEYSTATE_KEYUP) ? "KEYUP " : ""),
                ((pk->keystate & GKEYSTATE_REPEAT) ? "REPEAT " : ""),
                ((pk->keystate & GKEYSTATE_SPECIAL) ? "SPECIAL " : ""),
                ((pk->keystate & GKEYSTATE_RAW) ? "RAW " : ""),
                ((pk->keystate & GKEYSTATE_SHIFT_L) ? "LSHIFT " : ""),
                ((pk->keystate & GKEYSTATE_SHIFT_R) ? "RSHIFT " : ""),
                ((pk->keystate & GKEYSTATE_CTRL_L) ? "LCTRL " : ""),
                ((pk->keystate & GKEYSTATE_CTRL_R) ? "RCTRL " : ""),
                ((pk->keystate & GKEYSTATE_ALT_L) ? "LALT " : ""),
                ((pk->keystate & GKEYSTATE_ALT_R) ? "RALT " : ""),
                ((pk->keystate & GKEYSTATE_FN) ? "FN " : ""),
                ((pk->keystate & GKEYSTATE_COMPOSE) ? "COMPOSE " : ""),
                ((pk->keystate & GKEYSTATE_WINKEY) ? "WINKEY " : ""),
                ((pk->keystate & GKEYSTATE_CAPSLOCK) ? "CAPSLOCK " : ""),
                ((pk->keystate & GKEYSTATE_NUMLOCK) ? "NUMLOCK " : ""),
                ((pk->keystate & GKEYSTATE_SCROLLLOCK) ? "SCROLLLOCK " : "")
                );
            if (pk->bytecount) {
                gwinPrintf(ghConsole, " Keys:");
                for (i = 0; i < pk->bytecount; i++)
                    gwinPrintf(ghConsole, " 0x%02X", (uint8_t)pk->c[i]);
                gwinPrintf(ghConsole, " [");
                for (i = 0; i < pk->bytecount; i++)
                    gwinPrintf(ghConsole, "%c", pk->c[i] >= ' ' && pk->c[i] <= '~' ? pk->c[i] : ' ');
                gwinPrintf(ghConsole, "]");
            }
            gwinPrintf(ghConsole, "\n");
            break;

        default:
            gwinPrintf(ghConsole, "Unknown %d\n", pe->type);
            break;
        }
    }
*/

/************ GWIN textedit_virtual_keyboard ********
    gfxInit();

    gdispClear(Silver);
    gwinSetDefaultFont(gdispOpenFont("DejaVuSans16"));
    gwinSetDefaultStyle(&WhiteWidgetStyle, FALSE);
    gwinSetDefaultColor(Black);
    gwinSetDefaultBgColor(White);

    geventListenerInit(&gl);
    geventAttachSource(&gl, ginputGetKeyboard(0), 0);

    guiCreate();
*/

/********** GWIN container_nested**********
    GEvent* pe;

    // Initialize the display
    gfxInit();

    // Set the widget defaults
    gwinSetDefaultFont(gdispOpenFont("*"));
    gwinSetDefaultStyle(&WhiteWidgetStyle, FALSE);
    gdispClear(White);

    // Create the widget
    createWidgets();

    // We want to listen for widget events
    geventListenerInit(&gl);
    gwinAttachListener(&gl);

    // Set the initial state of the checkboxes
    gwinCheckboxCheck(ghCheckbox1, TRUE);
    gwinCheckboxCheck(ghCheckbox2, TRUE);
    gwinCheckboxCheck(ghCheckbox3, TRUE);

    while(1) {
        // Get an Event
        pe = geventEventWait(&gl, TIME_INFINITE);

        switch(pe->type) {
            case GEVENT_GWIN_CHECKBOX:
                if (((GEventGWinCheckbox*)pe)->gwin == ghCheckbox1) {
                    gwinSetVisible(ghContainer2, gwinCheckboxIsChecked(ghCheckbox1));
                }
                else if (((GEventGWinCheckbox*)pe)->gwin == ghCheckbox2) {
                    gwinSetVisible(ghContainer3, gwinCheckboxIsChecked(ghCheckbox2));
                }
                else if (((GEventGWinCheckbox*)pe)->gwin == ghCheckbox3) {
                    gwinSetVisible(ghContainer1, gwinCheckboxIsChecked(ghCheckbox3));
                }
                break;
        }
    }
*/

/********** DEMO notepad ********************
    color_t color = Black;
    uint16_t pen = 0;

    gfxInit();

//    gs = ginputGetMouse(9999);
//    geventAttachSource(&gl, gs, GLISTEN_MOUSEDOWNMOVES|GLISTEN_MOUSEMETA);
//    pem = (GEventMouse *)&gl.event;
//    ginputGetMouseStatus(0, pem);
//    deviceType = pem->type;

    drawScreen();
    geventListenerInit(&gl);
    gwinAttachListener(&gl);

    while (TRUE) {
        pe = geventEventWait(&gl, TIME_INFINITE);
        ESP_LOGI("XPT2046", "geventEventWait");
    }
*/

/**********DEmO mandelbrot***************
    float cx1, cy1;
    float zoom = 1.0f;

    gfxInit();

    cx1 = -0.086f;
    cy1 = 0.85f;

    while(TRUE) {
        mandelbrot(-2.0f*zoom+cx1, -1.5f*zoom+cy1, 2.0f*zoom+cx1, 1.5f*zoom+cy1);

        zoom *= 0.7f;
        if(zoom <= 0.00001f)
            zoom = 1.0f;
    }
*/

/********************************************/




    while(TRUE) {
        gfxSleepMilliseconds(500);
    }
}

#elif 1

static const GWidgetStyle YellowWidgetStyle = {
    Yellow,                         // window background
    HTML2COLOR(0x800000),           // focus

    // enabled color set
    {
        HTML2COLOR(0x0000FF),       // text
        HTML2COLOR(0x404040),       // edge
        HTML2COLOR(0xE0E0E0),       // fill
        HTML2COLOR(0x00E000)        // progress - active area
    },

    // disabled color set
    {
        HTML2COLOR(0xC0C0C0),       // text
        HTML2COLOR(0x808080),       // edge
        HTML2COLOR(0xE0E0E0),       // fill
        HTML2COLOR(0xC0E0C0)        // progress - active area
    },

    // pressed color set
    {
        HTML2COLOR(0xFF00FF),       // text
        HTML2COLOR(0x404040),       // edge
        HTML2COLOR(0x808080),       // fill
        HTML2COLOR(0x00E000),       // progress - active area
    }
};

/* The variables we need */
static font_t       font;
static GListener    gl;
static GHandle      ghConsole;
static GTimer       FlashTimer;

#if GWIN_NEED_TABSET
    static GHandle      ghTabset;
#else
    static GHandle      ghTabButtons, ghTabSliders, ghTabCheckboxes, ghTabLabels, ghTabRadios, ghTabLists, ghTabImages, ghTabProgressbar;
#endif
static GHandle      ghPgButtons, ghPgSliders, ghPgCheckboxes, ghPgLabels, ghPgRadios, ghPgLists, ghPgImages, ghPgProgressbars;
static GHandle      ghButton1, ghButton2, ghButton3, ghButton4;
static GHandle      ghSlider1, ghSlider2, ghSlider3, ghSlider4;
static GHandle      ghCheckbox1, ghCheckbox2, ghCheckbox3, ghCheckDisableAll;
static GHandle      ghLabelSlider1, ghLabelSlider2, ghLabelSlider3, ghLabelSlider4, ghLabelRadio1;
static GHandle      ghRadio1, ghRadio2;
static GHandle      ghRadioBlack, ghRadioWhite, ghRadioYellow;
static GHandle      ghList1, ghList2, ghList3, ghList4;
static GHandle      ghImage1;
static GHandle      ghProgressbar1;
static gdispImage   imgYesNo;

/* Some useful macros */
#define ScrWidth            gdispGetWidth()
#define ScrHeight           gdispGetHeight()

#define BUTTON_PADDING      20
#define TAB_HEIGHT          30
#define LABEL_HEIGHT        15
#define BUTTON_WIDTH        50
#define BUTTON_HEIGHT       30
#define LIST_WIDTH          75
#define LIST_HEIGHT         80
#define SLIDER_WIDTH        20
#define CHECKBOX_WIDTH      80
#define CHECKBOX_HEIGHT     20
#define RADIO_WIDTH         50
#define RADIO_HEIGHT        20
#define COLOR_WIDTH         80
#define DISABLEALL_WIDTH    100
#define GROUP_TABS          0
#define GROUP_YESNO         1
#define GROUP_COLORS        2

#if !GWIN_NEED_TABSET
    // Wrap tabs onto the next line if they don't fit.
    static void settabtext(GWidgetInit *pwi, char *txt) {
        if (pwi->g.x >= ScrWidth) {
            pwi->g.x = 0;
            pwi->g.y += pwi->g.height;
        }
        pwi->text = txt;
        pwi->g.width = gdispGetStringWidth(pwi->text, font) + BUTTON_PADDING;
        if (pwi->g.x + pwi->g.width > ScrWidth) {
            pwi->g.x = 0;
            pwi->g.y += pwi->g.height;
        }
    }

    /**
     * Set the visibility of widgets based on which tab is selected.
     */
    static void setTab(GHandle tab) {
        /* Make sure everything is invisible first */
        gwinHide(ghPgButtons);
        gwinHide(ghPgSliders);
        gwinHide(ghPgCheckboxes);
        gwinHide(ghPgLabels);
        gwinHide(ghPgRadios);
        gwinHide(ghPgLists);
        gwinHide(ghPgImages);
        gwinHide(ghPgProgressbars);

        /* Turn on widgets depending on the tab selected */
        if (tab == ghTabButtons)
            gwinShow(ghPgButtons);
        else if (tab == ghTabSliders)
            gwinShow(ghPgSliders);
        else if (tab == ghTabCheckboxes)
            gwinShow(ghPgCheckboxes);
        else if (tab == ghTabLabels)
            gwinShow(ghPgLabels);
        else if (tab == ghTabRadios)
            gwinShow(ghPgRadios);
        else if (tab == ghTabLists)
            gwinShow(ghPgLists);
        else if (tab == ghTabImages)
            gwinShow(ghPgImages);
        else if (tab == ghTabProgressbar)
            gwinShow(ghPgProgressbars);
    }
#endif

// Wrap buttons onto the next line if they don't fit.
static void setbtntext(GWidgetInit *pwi, coord_t maxwidth, char *txt) {
    if (pwi->g.x >= maxwidth) {
        pwi->g.x = 5;
        pwi->g.y += pwi->g.height+1;
    }
    pwi->text = txt;
    pwi->g.width = gdispGetStringWidth(pwi->text, font) + BUTTON_PADDING;
    if (pwi->g.x + pwi->g.width > maxwidth) {
        pwi->g.x = 5;
        pwi->g.y += pwi->g.height+1;
    }
}

/**
 * Create all the widgets.
 * With the exception of the Pages they are all initially visible.
 *
 * This routine is complicated by the fact that we want a dynamic
 * layout so it looks good on small and large displays.
 * It is tested to work on 320x272 as a minimum LCD size.
 */
static void createWidgets(void) {
    GWidgetInit     wi;
    coord_t         border, pagewidth;

    gwinWidgetClearInit(&wi);

    // Calculate page borders based on screen size
    border = ScrWidth < 450 ? 1 : 5;

    // Create the Tabs
    #if GWIN_NEED_TABSET
        wi.g.show = TRUE;
        wi.g.x = border; wi.g.y = 0;
        wi.g.width = ScrWidth - 2*border; wi.g.height = ScrHeight-wi.g.y-border;
        ghTabset            = gwinTabsetCreate(0, &wi, GWIN_TABSET_BORDER);
        ghPgButtons         = gwinTabsetAddTab(ghTabset, "Buttons", FALSE);
        ghPgSliders         = gwinTabsetAddTab(ghTabset, "Sliders", FALSE);
        ghPgCheckboxes      = gwinTabsetAddTab(ghTabset, "Checkbox", FALSE);
        ghPgRadios          = gwinTabsetAddTab(ghTabset, "Radios", FALSE);
        ghPgLists           = gwinTabsetAddTab(ghTabset, "Lists", FALSE);
        ghPgLabels          = gwinTabsetAddTab(ghTabset, "Labels", FALSE);
        ghPgImages          = gwinTabsetAddTab(ghTabset, "Images", FALSE);
        ghPgProgressbars    = gwinTabsetAddTab(ghTabset, "Progressbar", FALSE);

        pagewidth = gwinGetInnerWidth(ghTabset)/2;

        // Console - we apply some special colors before making it visible
        //  We put the console on the tabset itself rather than a tab-page.
        //  This makes it appear on every page :)
        wi.g.parent = ghTabset;
        wi.g.x = pagewidth;
        wi.g.width = pagewidth;
        ghConsole = gwinConsoleCreate(0, &wi.g);
        gwinSetColor(ghConsole, Black);
        gwinSetBgColor(ghConsole, HTML2COLOR(0xF0F0F0));

    #else
        wi.g.show = TRUE; wi.customDraw = gwinRadioDraw_Tab;
        wi.g.height = TAB_HEIGHT; wi.g.y = 0;
        wi.g.x = 0; setbtntext(&wi, ScrWidth, "Buttons");
        ghTabButtons     = gwinRadioCreate(0, &wi, GROUP_TABS);
        wi.g.x += wi.g.width; settabtext(&wi, "Sliders");
        ghTabSliders     = gwinRadioCreate(0, &wi, GROUP_TABS);
        wi.g.x += wi.g.width; settabtext(&wi, "Checkbox");
        ghTabCheckboxes  = gwinRadioCreate(0, &wi, GROUP_TABS);
        wi.g.x += wi.g.width; settabtext(&wi, "Radios");
        ghTabRadios      = gwinRadioCreate(0, &wi, GROUP_TABS);
        wi.g.x += wi.g.width; settabtext(&wi, "Lists");
        ghTabLists       = gwinRadioCreate(0, &wi, GROUP_TABS);
        wi.g.x += wi.g.width; settabtext(&wi, "Labels");
        ghTabLabels      = gwinRadioCreate(0, &wi, GROUP_TABS);
        wi.g.x += wi.g.width; settabtext(&wi, "Images");
        ghTabImages      = gwinRadioCreate(0, &wi, GROUP_TABS);
        wi.g.x += wi.g.width; settabtext(&wi, "Progressbar");
        ghTabProgressbar = gwinRadioCreate(0, &wi, GROUP_TABS);
        wi.g.y += wi.g.height;
        wi.customDraw = 0;

        // Create the Pages
        wi.g.show = FALSE;
        wi.g.x = border; wi.g.y += border;
        wi.g.width = ScrWidth/2 - border; wi.g.height = ScrHeight-wi.g.y-border;
        ghPgButtons         = gwinContainerCreate(0, &wi, GWIN_CONTAINER_BORDER);
        ghPgSliders         = gwinContainerCreate(0, &wi, GWIN_CONTAINER_BORDER);
        ghPgCheckboxes      = gwinContainerCreate(0, &wi, GWIN_CONTAINER_BORDER);
        ghPgRadios          = gwinContainerCreate(0, &wi, GWIN_CONTAINER_BORDER);
        ghPgLists           = gwinContainerCreate(0, &wi, GWIN_CONTAINER_BORDER);
        ghPgLabels          = gwinContainerCreate(0, &wi, GWIN_CONTAINER_BORDER);
        ghPgImages          = gwinContainerCreate(0, &wi, GWIN_CONTAINER_BORDER);
        ghPgProgressbars    = gwinContainerCreate(0, &wi, GWIN_CONTAINER_BORDER);
        wi.g.show = TRUE;

        // Console - we apply some special colors before making it visible
        wi.g.x = ScrWidth/2+border;
        wi.g.width = ScrWidth/2 - 2*border;
        ghConsole = gwinConsoleCreate(0, &wi.g);
        gwinSetColor(ghConsole, Black);
        gwinSetBgColor(ghConsole, HTML2COLOR(0xF0F0F0));

        pagewidth = gwinGetInnerWidth(ghPgButtons);
    #endif

    // Buttons
    wi.g.parent = ghPgButtons;
    wi.g.width = BUTTON_WIDTH; wi.g.height = BUTTON_HEIGHT; wi.g.y = 5;
    wi.g.x = 5; setbtntext(&wi, pagewidth, "Button 1");
    ghButton1 = gwinButtonCreate(0, &wi);
    wi.g.x += wi.g.width+3; setbtntext(&wi, pagewidth, "Button 2");
    ghButton2 = gwinButtonCreate(0, &wi);
    wi.g.x += wi.g.width+3; setbtntext(&wi, pagewidth, "Button 3");
    ghButton3 = gwinButtonCreate(0, &wi);
    wi.g.x += wi.g.width+3; setbtntext(&wi, pagewidth, "Button 4");
    ghButton4 = gwinButtonCreate(0, &wi);

    // Horizontal Sliders
    wi.g.parent = ghPgSliders;
    wi.g.width = pagewidth - 10; wi.g.height = SLIDER_WIDTH;
    wi.g.x = 5; wi.g.y = 5; wi.text = "S1";
    ghSlider1 = gwinSliderCreate(0, &wi);
    gwinSliderSetPosition(ghSlider1, 33);
    wi.g.y += wi.g.height + 1; wi.text = "S2";
    ghSlider2 = gwinSliderCreate(0, &wi);
    gwinSliderSetPosition(ghSlider2, 86);

    // Vertical Sliders
    wi.g.y += wi.g.height + 5;
    wi.g.width = SLIDER_WIDTH; wi.g.height = gwinGetInnerHeight(ghPgSliders) - 5 - wi.g.y;
    wi.g.x = 5; wi.text = "S3";
    ghSlider3 = gwinSliderCreate(0, &wi);
    gwinSliderSetPosition(ghSlider3, 13);
    wi.g.x += wi.g.width+1; wi.text = "S4";
    ghSlider4 = gwinSliderCreate(0, &wi);
    gwinSliderSetPosition(ghSlider4, 76);

    // Checkboxes - for the 2nd and 3rd checkbox we apply special drawing before making it visible
    wi.g.parent = ghPgCheckboxes;
    wi.g.width = CHECKBOX_WIDTH; wi.g.height = CHECKBOX_HEIGHT; wi.g.x = 5;
    wi.g.y = 5; wi.text = "C1";
    ghCheckbox1 = gwinCheckboxCreate(0, &wi);
    wi.customDraw = gwinCheckboxDraw_CheckOnRight;
    wi.g.y += wi.g.height+1; wi.text = "C2";
    ghCheckbox2 = gwinCheckboxCreate(0, &wi);
    wi.customDraw = gwinCheckboxDraw_Button;
    wi.g.y += wi.g.height+1; wi.text = "C3"; wi.g.width = BUTTON_WIDTH; wi.g.height = BUTTON_HEIGHT;
    ghCheckbox3 = gwinCheckboxCreate(0, &wi);
    wi.g.y += wi.g.height+1; wi.text = "Disable All";
    wi.customDraw = 0; wi.g.width = DISABLEALL_WIDTH; wi.g.height = CHECKBOX_HEIGHT;
    ghCheckDisableAll = gwinCheckboxCreate(0, &wi);

    // Labels
    wi.g.parent = ghPgLabels;
    wi.g.width = pagewidth-10;  wi.g.height = LABEL_HEIGHT;
    wi.g.x = wi.g.y = 5; wi.text = "N/A";
    ghLabelSlider1 = gwinLabelCreate(0, &wi);
    gwinLabelSetAttribute(ghLabelSlider1, 100, "Slider 1:");
    wi.g.y += LABEL_HEIGHT + 2;
    ghLabelSlider2 = gwinLabelCreate(0, &wi);
    gwinLabelSetAttribute(ghLabelSlider2, 100, "Slider 2:");
    wi.g.y += LABEL_HEIGHT + 2;
    ghLabelSlider3 = gwinLabelCreate(0, &wi);
    gwinLabelSetAttribute(ghLabelSlider3, 100, "Slider 3:");
    wi.g.y += LABEL_HEIGHT + 2;
    ghLabelSlider4 = gwinLabelCreate(0, &wi);
    gwinLabelSetAttribute(ghLabelSlider4, 100, "Slider 4:");
    wi.g.y += LABEL_HEIGHT + 2;
    ghLabelRadio1 = gwinLabelCreate(0, &wi);
    gwinLabelSetAttribute(ghLabelRadio1, 100, "RadioButton 1:");


    // Radio Buttons
    wi.g.parent = ghPgRadios;
    wi.g.width = RADIO_WIDTH; wi.g.height = RADIO_HEIGHT; wi.g.y = 5;
    wi.g.x = 5; wi.text = "Yes";
    ghRadio1 = gwinRadioCreate(0, &wi, GROUP_YESNO);
    wi.g.x += wi.g.width; wi.text = "No"; if (wi.g.x + wi.g.width > pagewidth) { wi.g.x = 5; wi.g.y += RADIO_HEIGHT; }
    ghRadio2 = gwinRadioCreate(0, &wi, GROUP_YESNO);
    gwinRadioPress(ghRadio1);
    wi.g.width = COLOR_WIDTH; wi.g.y += RADIO_HEIGHT+5;
    wi.g.x = 5; wi.text = "Black";
    ghRadioBlack = gwinRadioCreate(0, &wi, GROUP_COLORS);
    wi.g.x += wi.g.width; wi.text = "White"; if (wi.g.x + wi.g.width > pagewidth) { wi.g.x = 5; wi.g.y += RADIO_HEIGHT; }
    ghRadioWhite = gwinRadioCreate(0, &wi, GROUP_COLORS);
    wi.g.x += wi.g.width; wi.text = "Yellow"; if (wi.g.x + wi.g.width > pagewidth) { wi.g.x = 5; wi.g.y += RADIO_HEIGHT; }
    ghRadioYellow = gwinRadioCreate(0, &wi, GROUP_COLORS);
    gwinRadioPress(ghRadioWhite);

    // Lists
    border = pagewidth < 10+2*LIST_WIDTH ? 2 : 5;
    wi.g.parent = ghPgLists;
    wi.g.width = LIST_WIDTH; wi.g.height = LIST_HEIGHT; wi.g.y = border;
    wi.g.x = border; wi.text = "L1";
    ghList1 = gwinListCreate(0, &wi, FALSE);
    gwinListAddItem(ghList1, "Item 0", FALSE);
    gwinListAddItem(ghList1, "Item 1", FALSE);
    gwinListAddItem(ghList1, "Item 2", FALSE);
    gwinListAddItem(ghList1, "Item 3", FALSE);
    gwinListAddItem(ghList1, "Item 4", FALSE);
    gwinListAddItem(ghList1, "Item 5", FALSE);
    gwinListAddItem(ghList1, "Item 6", FALSE);
    gwinListAddItem(ghList1, "Item 7", FALSE);
    gwinListAddItem(ghList1, "Item 8", FALSE);
    gwinListAddItem(ghList1, "Item 9", FALSE);
    gwinListAddItem(ghList1, "Item 10", FALSE);
    gwinListAddItem(ghList1, "Item 11", FALSE);
    gwinListAddItem(ghList1, "Item 12", FALSE);
    gwinListAddItem(ghList1, "Item 13", FALSE);
    wi.text = "L2"; wi.g.x += LIST_WIDTH+border; if (wi.g.x + LIST_WIDTH > pagewidth) { wi.g.x = border; wi.g.y += LIST_HEIGHT+border; }
    ghList2 = gwinListCreate(0, &wi, TRUE);
    gwinListAddItem(ghList2, "Item 0", FALSE);
    gwinListAddItem(ghList2, "Item 1", FALSE);
    gwinListAddItem(ghList2, "Item 2", FALSE);
    gwinListAddItem(ghList2, "Item 3", FALSE);
    gwinListAddItem(ghList2, "Item 4", FALSE);
    gwinListAddItem(ghList2, "Item 5", FALSE);
    gwinListAddItem(ghList2, "Item 6", FALSE);
    gwinListAddItem(ghList2, "Item 7", FALSE);
    gwinListAddItem(ghList2, "Item 8", FALSE);
    gwinListAddItem(ghList2, "Item 9", FALSE);
    gwinListAddItem(ghList2, "Item 10", FALSE);
    gwinListAddItem(ghList2, "Item 11", FALSE);
    gwinListAddItem(ghList2, "Item 12", FALSE);
    gwinListAddItem(ghList2, "Item 13", FALSE);
    wi.text = "L3"; wi.g.x += LIST_WIDTH+border; if (wi.g.x + LIST_WIDTH > pagewidth) { wi.g.x = border; wi.g.y += LIST_HEIGHT+border; }
    ghList3 = gwinListCreate(0, &wi, TRUE);
    gwinListAddItem(ghList3, "Item 0", FALSE);
    gwinListAddItem(ghList3, "Item 1", FALSE);
    gwinListAddItem(ghList3, "Item 2", FALSE);
    gwinListAddItem(ghList3, "Item 3", FALSE);
    gdispImageOpenFile(&imgYesNo, "image_yesno.gif");
    gwinListItemSetImage(ghList3, 1, &imgYesNo);
    gwinListItemSetImage(ghList3, 3, &imgYesNo);
    wi.text = "L4"; wi.g.x += LIST_WIDTH+border; if (wi.g.x + LIST_WIDTH > pagewidth) { wi.g.x = border; wi.g.y += LIST_HEIGHT+border; }
    ghList4 = gwinListCreate(0, &wi, TRUE);
    gwinListAddItem(ghList4, "Item 0", FALSE);
    gwinListAddItem(ghList4, "Item 1", FALSE);
    gwinListAddItem(ghList4, "Item 2", FALSE);
    gwinListAddItem(ghList4, "Item 3", FALSE);
    gwinListAddItem(ghList4, "Item 4", FALSE);
    gwinListAddItem(ghList4, "Item 5", FALSE);
    gwinListAddItem(ghList4, "Item 6", FALSE);
    gwinListAddItem(ghList4, "Item 7", FALSE);
    gwinListAddItem(ghList4, "Item 8", FALSE);
    gwinListAddItem(ghList4, "Item 9", FALSE);
    gwinListAddItem(ghList4, "Item 10", FALSE);
    gwinListAddItem(ghList4, "Item 11", FALSE);
    gwinListAddItem(ghList4, "Item 12", FALSE);
    gwinListAddItem(ghList4, "Item 13", FALSE);
    gwinListSetScroll(ghList4, scrollSmooth);

    // Image
    wi.g.parent = ghPgImages;
    wi.g.x = wi.g.y = 0; wi.g.width = pagewidth; wi.g.height = gwinGetInnerHeight(ghPgImages);
    ghImage1 = gwinImageCreate(0, &wi.g);
    gwinImageOpenFile(ghImage1, "romfs_img_ugfx.gif");

    // Progressbar
    wi.g.parent = ghPgProgressbars;
    wi.g.width = pagewidth-10; wi.g.height = SLIDER_WIDTH; wi.g.y = 5;
    wi.g.x = 5; wi.text = "Progressbar 1";
    ghProgressbar1 = gwinProgressbarCreate(0, &wi);
    gwinProgressbarSetResolution(ghProgressbar1, 10);
}

/**
 * Set the value of the labels
 */
static void setLabels(void) {
    char tmp[20];

    // The sliders
    snprintg(tmp, sizeof(tmp), "%d%%", gwinSliderGetPosition(ghSlider1));
    gwinSetText(ghLabelSlider1, tmp, TRUE);
    snprintg(tmp, sizeof(tmp), "%d%%", gwinSliderGetPosition(ghSlider2));
    gwinSetText(ghLabelSlider2, tmp, TRUE);
    snprintg(tmp, sizeof(tmp), "%d%%", gwinSliderGetPosition(ghSlider3));
    gwinSetText(ghLabelSlider3, tmp, TRUE);
    snprintg(tmp, sizeof(tmp), "%d%%", gwinSliderGetPosition(ghSlider4));
    gwinSetText(ghLabelSlider4, tmp, TRUE);

    // The radio buttons
    if (gwinRadioIsPressed(ghRadio1))
        gwinSetText(ghLabelRadio1, "Yes", TRUE);
    else if (gwinRadioIsPressed(ghRadio2))
        gwinSetText(ghLabelRadio1, "No", TRUE);
}

/**
 * Control the progress bar auto-increment
 */
static void setProgressbar(bool_t onoff) {
    if (onoff)
        gwinProgressbarStart(ghProgressbar1, 500);
    else {
        gwinProgressbarStop(ghProgressbar1);        // Stop the progress bar
        gwinProgressbarReset(ghProgressbar1);
    }
}

/**
 * Set the enabled state of every widget (except the tabs etc)
 */
static void setEnabled(bool_t ena) {
    gwinSetEnabled(ghPgButtons, ena);
    gwinSetEnabled(ghPgSliders, ena);
    gwinSetEnabled(ghPgLabels, ena);
    gwinSetEnabled(ghPgRadios, ena);
    gwinSetEnabled(ghPgLists, ena);
    gwinSetEnabled(ghPgImages, ena);
    gwinSetEnabled(ghPgProgressbars, ena);
    // Checkboxes we need to do individually so we don't disable the checkbox to re-enable everything
    gwinSetEnabled(ghCheckbox1, ena);
    gwinSetEnabled(ghCheckbox2, ena);
    gwinSetEnabled(ghCheckbox3, ena);
    //gwinSetEnabled(ghCheckDisableAll, TRUE);
}

static void FlashOffFn(void *param) {
    (void)  param;

    gwinNoFlash(ghCheckbox3);
}

void lcd_task(void * pvParameter)
{
    tft->drawBitmap(0, 0, esp_logo, 137, 26);
    vTaskDelete(NULL);
}

void listener(void * pvParameter)
{
    GEvent *            pe;
    while(1) {
        // Get an Event
        pe = geventEventWait(&gl, TIME_INFINITE);

        switch(pe->type) {
        case GEVENT_GWIN_BUTTON:
            gwinPrintf(ghConsole, "Button %s\n", gwinGetText(((GEventGWinButton *)pe)->gwin));
            break;

        case GEVENT_GWIN_SLIDER:
            gwinPrintf(ghConsole, "Slider %s=%d\n", gwinGetText(((GEventGWinSlider *)pe)->gwin), ((GEventGWinSlider *)pe)->position);
            break;

        case GEVENT_GWIN_CHECKBOX:
            gwinPrintf(ghConsole, "Checkbox %s=%s\n", gwinGetText(((GEventGWinCheckbox *)pe)->gwin), ((GEventGWinCheckbox *)pe)->isChecked ? "Checked" : "UnChecked");

            // If it is the Disable All checkbox then do that.
            if (((GEventGWinCheckbox *)pe)->gwin == ghCheckDisableAll) {
                gwinPrintf(ghConsole, "%s All\n", ((GEventGWinCheckbox *)pe)->isChecked ? "Disable" : "Enable");
                setEnabled(!((GEventGWinCheckbox *)pe)->isChecked);

            // If it is the toggle button checkbox start the flash.
            } else if (((GEventGWinCheckbox *)pe)->gwin == ghCheckbox3) {
                gwinFlash(ghCheckbox3);
                gtimerStart(&FlashTimer, FlashOffFn, 0, FALSE, 3000);
            }
            break;

        case GEVENT_GWIN_LIST:
            gwinPrintf(ghConsole, "List %s Item %d %s\n", gwinGetText(((GEventGWinList *)pe)->gwin), ((GEventGWinList *)pe)->item,
                    gwinListItemIsSelected(((GEventGWinList *)pe)->gwin, ((GEventGWinList *)pe)->item) ? "Selected" : "Unselected");
            break;

        case GEVENT_GWIN_RADIO:
            gwinPrintf(ghConsole, "Radio Group %u=%s\n", ((GEventGWinRadio *)pe)->group, gwinGetText(((GEventGWinRadio *)pe)->gwin));

            switch(((GEventGWinRadio *)pe)->group) {
            #if !GWIN_NEED_TABSET
                case GROUP_TABS:

                    // Set control visibility depending on the tab selected
                    setTab(((GEventGWinRadio *)pe)->gwin);

                    // We show the state of some of the GUI elements here
                    setProgressbar(((GEventGWinRadio *)pe)->gwin == ghTabProgressbar);
                    if (((GEventGWinRadio *)pe)->gwin == ghTabLabels)
                        setLabels();
                    break;
            #endif

            case GROUP_COLORS:
                {
                    const GWidgetStyle  *pstyle;

                    gwinPrintf(ghConsole, "Change Color Scheme\n");

                    if (((GEventGWinRadio *)pe)->gwin == ghRadioYellow)
                        pstyle = &YellowWidgetStyle;
                    else if (((GEventGWinRadio *)pe)->gwin == ghRadioBlack)
                        pstyle = &BlackWidgetStyle;
                    else
                        pstyle = &WhiteWidgetStyle;

                    // Clear the screen to the new color
                    gdispClear(pstyle->background);

                    // Update the style on all controls
                    gwinSetDefaultStyle(pstyle, TRUE);
                }
                break;
            }
            break;

        #if GWIN_NEED_TABSET
            case GEVENT_GWIN_TABSET:
                gwinPrintf(ghConsole, "TabPage %u (%s)\n", ((GEventGWinTabset *)pe)->nPage, gwinTabsetGetTitle(((GEventGWinTabset *)pe)->ghPage));

                // We show the state of some of the GUI elements here
                setProgressbar(((GEventGWinTabset *)pe)->ghPage == ghPgProgressbars);
                if (((GEventGWinTabset *)pe)->ghPage == ghPgLabels)
                    setLabels();
                break;
        #endif

        default:
            gwinPrintf(ghConsole, "Unknown %d\n", pe->type);
            break;
        }
    }
}

extern "C" void app_main() {

    app_lcd_init();

    set_semaphore(tft->getSemphor());
    set_spidevice(tft->getSpidevice());
    set_dc(tft->getlcd_dc());

    GEvent *            pe;

    // Initialize the display
    gfxInit();

    // Set the widget defaults
    font = gdispOpenFont("*");          // Get the first defined font.
    gwinSetDefaultFont(font);
    gwinSetDefaultStyle(&WhiteWidgetStyle, FALSE);
    gdispClear(White);

    // Create the gwin windows/widgets
    createWidgets();

    // Assign toggles and dials to specific buttons & sliders etc.
    #if GINPUT_NEED_TOGGLE
        gwinAttachToggle(ghButton1, 0, 0);
        gwinAttachToggle(ghButton2, 0, 1);
    #endif
    #if GINPUT_NEED_DIAL
        gwinAttachDial(ghSlider1, 0, 0);
        gwinAttachDial(ghSlider3, 0, 1);
    #endif

    // Make the console visible
    gwinShow(ghConsole);
    gwinClear(ghConsole);

    // We want to listen for widget events
    geventListenerInit(&gl);
    gwinAttachListener(&gl);
    gtimerInit(&FlashTimer);

    #if !GWIN_NEED_TABSET
        // Press the Tab we want visible
        gwinRadioPress(ghTabButtons);
    #endif

    xTaskCreate(&lcd_task, "lcd_task", 2048*2, NULL, 10, NULL);
//    xTaskCreate(&listener, "listener", 2048*2, NULL, 10, NULL);
    while(1) {
        // Get an Event
        pe = geventEventWait(&gl, TIME_INFINITE);

        switch(pe->type) {
        case GEVENT_GWIN_BUTTON:
            gwinPrintf(ghConsole, "Button %s\n", gwinGetText(((GEventGWinButton *)pe)->gwin));
            break;

        case GEVENT_GWIN_SLIDER:
            gwinPrintf(ghConsole, "Slider %s=%d\n", gwinGetText(((GEventGWinSlider *)pe)->gwin), ((GEventGWinSlider *)pe)->position);
            break;

        case GEVENT_GWIN_CHECKBOX:
            gwinPrintf(ghConsole, "Checkbox %s=%s\n", gwinGetText(((GEventGWinCheckbox *)pe)->gwin), ((GEventGWinCheckbox *)pe)->isChecked ? "Checked" : "UnChecked");

            // If it is the Disable All checkbox then do that.
            if (((GEventGWinCheckbox *)pe)->gwin == ghCheckDisableAll) {
                gwinPrintf(ghConsole, "%s All\n", ((GEventGWinCheckbox *)pe)->isChecked ? "Disable" : "Enable");
                setEnabled(!((GEventGWinCheckbox *)pe)->isChecked);

            // If it is the toggle button checkbox start the flash.
            } else if (((GEventGWinCheckbox *)pe)->gwin == ghCheckbox3) {
                gwinFlash(ghCheckbox3);
                gtimerStart(&FlashTimer, FlashOffFn, 0, FALSE, 3000);
            }
            break;

        case GEVENT_GWIN_LIST:
            gwinPrintf(ghConsole, "List %s Item %d %s\n", gwinGetText(((GEventGWinList *)pe)->gwin), ((GEventGWinList *)pe)->item,
                    gwinListItemIsSelected(((GEventGWinList *)pe)->gwin, ((GEventGWinList *)pe)->item) ? "Selected" : "Unselected");
            break;

        case GEVENT_GWIN_RADIO:
            gwinPrintf(ghConsole, "Radio Group %u=%s\n", ((GEventGWinRadio *)pe)->group, gwinGetText(((GEventGWinRadio *)pe)->gwin));

            switch(((GEventGWinRadio *)pe)->group) {
            #if !GWIN_NEED_TABSET
                case GROUP_TABS:

                    // Set control visibility depending on the tab selected
                    setTab(((GEventGWinRadio *)pe)->gwin);

                    // We show the state of some of the GUI elements here
                    setProgressbar(((GEventGWinRadio *)pe)->gwin == ghTabProgressbar);
                    if (((GEventGWinRadio *)pe)->gwin == ghTabLabels)
                        setLabels();
                    break;
            #endif

            case GROUP_COLORS:
                {
                    const GWidgetStyle  *pstyle;

                    gwinPrintf(ghConsole, "Change Color Scheme\n");

                    if (((GEventGWinRadio *)pe)->gwin == ghRadioYellow)
                        pstyle = &YellowWidgetStyle;
                    else if (((GEventGWinRadio *)pe)->gwin == ghRadioBlack)
                        pstyle = &BlackWidgetStyle;
                    else
                        pstyle = &WhiteWidgetStyle;

                    // Clear the screen to the new color
                    gdispClear(pstyle->background);

                    // Update the style on all controls
                    gwinSetDefaultStyle(pstyle, TRUE);
                }
                break;
            }
            break;

        #if GWIN_NEED_TABSET
            case GEVENT_GWIN_TABSET:
                gwinPrintf(ghConsole, "TabPage %u (%s)\n", ((GEventGWinTabset *)pe)->nPage, gwinTabsetGetTitle(((GEventGWinTabset *)pe)->ghPage));

                // We show the state of some of the GUI elements here
                setProgressbar(((GEventGWinTabset *)pe)->ghPage == ghPgProgressbars);
                if (((GEventGWinTabset *)pe)->ghPage == ghPgLabels)
                    setLabels();
                break;
        #endif

        default:
            gwinPrintf(ghConsole, "Unknown %d\n", pe->type);
            break;
        }
    }
}
#endif


