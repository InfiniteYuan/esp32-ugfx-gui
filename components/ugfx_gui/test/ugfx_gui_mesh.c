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
#include "unity.h"

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

static const char *TAG = "espnow_contrl_example";

font_t          font16;
font_t          font12;

static GListener    gl;
static GHandle      ghSliderR, ghSliderG, ghSliderB;
static GHandle      ghButton1, ghButton2, ghButton3;
static GHandle      ghWindow1;
static gdispImage   imgLight;

enum low_power_data_type {
    ESPNOW_CONTROL_PAIR,
    ESPNOW_CONTROL_GET,
    ESPNOW_CONTROL_SET,
    ESPNOW_CONTROL_REQUEST,
    ESPNOW_CONTROL_RESPONSE,
};

typedef int sockfd_t;
typedef union {
    struct {
        in_port_t port; /**< port */
        in_addr_t ip;   /**< IP address */
    } __attribute__((packed));
    uint8_t mac[6];
} sockaddr_t;

typedef struct {
    uint8_t type;
    sockaddr_t dest_addr;
    uint8_t size;
    uint8_t data[0];
} low_power_data_t;

enum light_status_cid {
    STATUS_CID            = 0,
    HUE_CID               = 1,
    SATURATION_CID        = 2,
    VALUE_CID             = 3,
    COLOR_TEMPERATURE_CID = 4,
    BRIGHTNESS_CID        = 5,
};

typedef enum light_control {
    LIGHT_CONTROL_ON,
    LIGHT_CONTROL_OFF,
    LIGHT_CONTROL_COLOR,
} light_control_t;

typedef struct {
    int type;
} light_evt_t;

// system
QueueHandle_t light_queue = NULL;

esp_err_t mdf_info_init()
{
    static bool init_flag = false;

    if (!init_flag) {
        esp_err_t ret = nvs_flash_init();

        if (ret == ESP_ERR_NVS_NO_FREE_PAGES) {
            ESP_ERROR_CHECK(nvs_flash_erase());
            ret = nvs_flash_init();
        }

        ESP_ERROR_CHECK(ret);

        init_flag = true;
    }

    return ESP_OK;
}

static esp_err_t mdf_wifi_init()
{
    static bool mdf_wifi_init_flag = false;

    if (!mdf_wifi_init_flag) {

        wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();

        tcpip_adapter_init();
        tcpip_adapter_dhcpc_stop(TCPIP_ADAPTER_IF_STA);
        tcpip_adapter_dhcps_stop(TCPIP_ADAPTER_IF_AP);

        ESP_ERROR_CHECK(esp_event_loop_init(NULL, NULL));
        ESP_ERROR_CHECK(esp_wifi_init(&cfg));
        ESP_ERROR_CHECK(esp_wifi_set_storage(WIFI_STORAGE_RAM));
        ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));

        ESP_ERROR_CHECK(esp_wifi_set_ps(WIFI_PS_NONE));
        ESP_ERROR_CHECK(esp_wifi_start());

        mdf_wifi_init_flag = true;
    }
    return ESP_OK;
}

void rgb2hsv(uint16_t *hue, uint8_t *saturation, uint8_t *value,
                               uint8_t red, uint8_t green, uint8_t blue)
{
    double temp, max, min, R, G, B;
    R = red / 255.0;
    G = green / 255.0;
    B = blue / 255.0;

    temp = B > G ? B : G;
    max = temp > R ? temp : R;//get max

    temp = B > G ? G : B;
    min = temp > R ? R : temp;//get min

    if(fabs(max-min) < 0.000001)
        *hue = 0;
    else if(fabs(R-max) < 0.000001)
        *hue = (int)((G-B)/(max-min)) % 6;
    else if(fabs(G-max) < 0.000001)
        *hue = 2 + (B-R)/(max-min);
    else if(fabs(B-max) < 0.000001)
        *hue = 4 + (R-G)/(max-min);

    *hue = *hue * 60;
    if(*hue < 0)
        *hue = *hue + 360;

    if(fabs(0-max) < 0.000001)
        *saturation = 0;
    else
        *saturation = ((max-min)/max) * 100;

    *value = max * 100;

    // MDF_LOGD("light HSV:%d, %d, %d,",*hue, *saturation, *value);
}

void set_light_color(uint8_t red, uint8_t green, uint8_t blue)
{
    uint16_t hue;
    uint8_t saturation;
    uint8_t value;

    rgb2hsv(&hue, &saturation, &value, red, green, blue);
    char light_color[100];
    sprintf(light_color, "{\"request\":\"set_status\",\"characteristics\":[{\"cid\":%d,\"value\":%d}]}", HUE_CID, hue);
    sprintf(light_color, "{\"request\":\"set_status\",\"characteristics\":[{\"cid\":%d,\"value\":%d}]}", SATURATION_CID, saturation);
    sprintf(light_color, "{\"request\":\"set_status\",\"characteristics\":[{\"cid\":%d,\"value\":%d}]}", VALUE_CID, value);
}

// self_mac: 30:ae:a4:08:9c:28
// light_mac: 0x30, 0xae, 0xa4, 0x57, 0xe0, 0x5c
// coreboard_mac: 30:ae:a4:80:0b:90
void light_task(void *pvParameter)
{
    esp_err_t ret = ESP_OK;
    uint8_t channel = 13;
    uint8_t light_mac[6] = {0x30, 0xae, 0xa4, 0x80, 0x0b, 0x90};
    size_t espnow_size  = 0;
    
    uint16_t hue;
    uint8_t saturation;
    uint8_t value;
    uint8_t red, green, blue;

    low_power_data_t *espnow_data = mdf_calloc(1, 256);
    espnow_data->type = ESPNOW_CONTROL_REQUEST;

    const char *light_off = "{\"request\":\"set_status\",\"characteristics\":[{\"cid\":0,\"value\":0}]}";
    const char *light_on = "{\"request\":\"set_status\",\"characteristics\":[{\"cid\":0,\"value\":1}]}";

    if (light_queue == NULL) {
        light_queue = xQueueCreate(10, sizeof(light_evt_t));
    }

    ret = mdf_info_init();
    MDF_ERROR_CHECK(ret < 0, ;, "mdf_info_init: %d", ret);
    ret = mdf_wifi_init();
    MDF_ERROR_CHECK(ret < 0, ;, "mdf_wifi_init: %d", ret);

    //WIFI_SECOND_CHAN_NONE  
    //WIFI_SECOND_CHAN_ABOVE  
    //WIFI_SECOND_CHAN_BELOW

    ret = esp_wifi_set_channel(channel, WIFI_SECOND_CHAN_BELOW);
    MDF_ERROR_CHECK(ret < 0, ;, "esp_wifi_set_channel: %d", ret);
    ret = mdf_espnow_init();
    MDF_ERROR_CHECK(ret < 0, ;, "esp_wifi_set_channel: %d", ret);

    ESP_ERROR_CHECK(mdf_espnow_enable(MDF_ESPNOW_CONTROL));
    ESP_ERROR_CHECK(mdf_espnow_add_peer_default_encrypt(light_mac));

    portBASE_TYPE retpot;
    light_evt_t evt;
    for(;;) {
        retpot = xQueueReceive(light_queue, &evt, 500 / portTICK_PERIOD_MS);
        if(retpot == pdTRUE){
            if(evt.type == LIGHT_CONTROL_ON) {

                MDF_LOGI("light on");
                espnow_data->size = strlen(light_on);
                espnow_size = espnow_data->size + sizeof(low_power_data_t);
                memcpy(espnow_data->data, light_on, strlen(light_on));

                ret = mdf_espnow_write(MDF_ESPNOW_CONTROL, light_mac,
                                   espnow_data, espnow_size, portMAX_DELAY);
                MDF_ERROR_CONTINUE(ret < 0, "mdf_espnow_write ret: %d", ret);

                vTaskDelay(500 / portTICK_RATE_MS);
            } else if(evt.type == LIGHT_CONTROL_OFF) {

                MDF_LOGI("light off");
                espnow_data->size = strlen(light_off);
                espnow_size = espnow_data->size + sizeof(low_power_data_t);
                memcpy(espnow_data->data, light_off, strlen(light_off));

                ret = mdf_espnow_write(MDF_ESPNOW_CONTROL, light_mac,
                                   espnow_data, espnow_size, portMAX_DELAY);
                MDF_ERROR_CONTINUE(ret < 0, "mdf_espnow_write ret: %d", ret);

                vTaskDelay(500 / portTICK_RATE_MS);
            } else if(evt.type == LIGHT_CONTROL_COLOR) {

                red = (unsigned)gwinSliderGetPosition(ghSliderR);
                green = (unsigned)gwinSliderGetPosition(ghSliderG);
                blue = (unsigned)gwinSliderGetPosition(ghSliderB);
                
                // MDF_LOGD("light RGB:%d, %d, %d,",red, green, blue);

                rgb2hsv(&hue, &saturation, &value, red, green, blue);
                char light_color[100];
                
                MDF_LOGI("light color");
                espnow_data->size = strlen(light_color);
                sprintf(light_color, "{\"request\":\"set_status\",\"characteristics\":[{\"cid\":%d,\"value\":%d},{\"cid\":%d,\"value\":%d},{\"cid\":%d,\"value\":%d}]}%c", HUE_CID, hue, SATURATION_CID, saturation, VALUE_CID, value,'\0');

                espnow_size = espnow_data->size + sizeof(low_power_data_t);
                memcpy(espnow_data->data, light_color, strlen(light_color));
                ret = mdf_espnow_write(MDF_ESPNOW_CONTROL, light_mac, espnow_data, espnow_size, portMAX_DELAY);
                MDF_ERROR_CONTINUE(ret < 0, "mdf_espnow_write ret: %d", ret);
                vTaskDelay(500 / portTICK_RATE_MS);
            }
        }
    }
}

void light_on()
{
    light_evt_t evt;
    evt.type = LIGHT_CONTROL_ON;
    xQueueSend(light_queue, &evt, portMAX_DELAY);
}

void light_off()
{
    light_evt_t evt;
    evt.type = LIGHT_CONTROL_OFF;
    xQueueSend(light_queue, &evt, portMAX_DELAY);
}

void light_color()
{
    light_evt_t evt;
    evt.type = LIGHT_CONTROL_COLOR;
    xQueueSend(light_queue, &evt, portMAX_DELAY);
}

void _updateColor(void) 
{
    uint32_t color;

    color  = (unsigned)gwinSliderGetPosition(ghSliderR) << 16;
    color |= (unsigned)gwinSliderGetPosition(ghSliderG) <<  8;
    color |= (unsigned)gwinSliderGetPosition(ghSliderB) <<  0;

    gwinSetBgColor(ghWindow1, HTML2COLOR(color));
    gwinClear(ghWindow1);
}

void creat_widgets(void)
{
    GWidgetInit wi;

    // Apply some default values for GWIN
    gwinWidgetClearInit(&wi);
    wi.g.show = TRUE;

    // Apply StringBox
	gdispDrawStringBox(0, 0, gdispGetWidth(), gdispGetFontMetric(font16, fontHeight), "Mesh APP based on uGFX", font16, Black, justifyCenter);

    // Apply the slider parameters
    wi.g.width = 200;
    wi.g.height = 20;
    wi.g.x = (gdispGetWidth()-wi.g.width)/2;
    wi.g.y = 30;
    wi.text = "Red";
    ghSliderR = gwinSliderCreate(0, &wi);
    gwinSliderSetRange(ghSliderR, 0, 255);
    gwinSliderSetPosition(ghSliderR, 180);

    // Apply the slider parameters
    wi.g.width = 200;
    wi.g.height = 20;
    wi.g.x = (gdispGetWidth()-wi.g.width)/2;
    wi.g.y += 25;
    wi.text = "Green";
    ghSliderG = gwinSliderCreate(0, &wi);
    gwinSliderSetRange(ghSliderG, 0, 255);
    gwinSliderSetPosition(ghSliderG, 60);

    // Apply the slider parameters
    wi.g.width = 200;
    wi.g.height = 20;
    wi.g.x = (gdispGetWidth()-wi.g.width)/2;
    wi.g.y += 25;
    wi.text = "Blue";
    ghSliderB = gwinSliderCreate(0, &wi);
    gwinSliderSetRange(ghSliderB, 0, 255);
    gwinSliderSetPosition(ghSliderB, 235);

	// Set slider to return extended events
	gwinSliderSendExtendedEvents(ghSliderR, TRUE);
	gwinSliderSendExtendedEvents(ghSliderG, TRUE);
	gwinSliderSendExtendedEvents(ghSliderB, TRUE);

    // Color Preview
    wi.g.width = 270;
    wi.g.height = 65;
    wi.g.x = (gdispGetWidth()-wi.g.width)/2;
    wi.g.y += 35;
    ghWindow1 = gwinWindowCreate(0, &wi.g);

    // Apply the button parameters
    wi.g.width = 48;
    wi.g.height = 48;
    wi.g.x = (gdispGetWidth()-wi.g.width)/2;
    wi.g.y += 75;
    wi.text = "ON/OFF";
    ghButton1 = gwinButtonCreate(0, &wi);

    ESP_LOGI("widgets", "imgLight:%p", &imgLight);

	_updateColor();
}

void ugfx_testcase_12(void)
{
    GEvent* pe;
    bool on_off = true;

    // xTaskCreatePinnedToCore(light_task, "light_task", 2048*2, NULL, 4, NULL, 1);

    // Initialize the display
	gfxInit();

	font16 = gdispOpenFont("DejaVuSans16");
	font12 = gdispOpenFont("DejaVuSans12");

    // Set the widget defaults
    gwinSetDefaultFont(gdispOpenFont("*"));
    gwinSetDefaultStyle(&WhiteWidgetStyle, FALSE);
	gdispClear(White);
    
    // create the widget
	creat_widgets();

    // We want to listen for widget events
    geventListenerInit(&gl);
    gwinAttachListener(&gl);

    while(1) {
        // Get an Event
        pe = geventEventWait(&gl, TIME_INFINITE);

        switch(pe->type) {
            case GEVENT_GWIN_SLIDER:
                if (((GEventGWinSlider *)pe)->gwin == ghSliderR || ghSliderG || ghSliderB ) {
                    _updateColor();
                    light_color();
                }
                break;
            case GEVENT_GWIN_BUTTON:
                if(((GEventGWinButton *)pe)->gwin == ghButton1 ) {
                    if(!on_off) {
                        light_on();
                        on_off = true;
                    } else if(on_off) {
                        light_off();
                        on_off = false;
                    }
                }
                break;
            default:
				// Oops - not a event.
                break;
        }
    }
}

TEST_CASE("uGFX testcase 12: Mesh", "[ugfx][gui][hmi][mesh]")
{
    ugfx_testcase_12();
}
