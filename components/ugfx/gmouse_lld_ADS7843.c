/*
 * This file is subject to the terms of the GFX License. If a copy of
 * the license was not distributed with this file, you can obtain one at:
 *
 *              http://ugfx.org/license.html
 */

#include "gfx.h"

#if (GFX_USE_GINPUT && GINPUT_NEED_MOUSE)

#define GMOUSE_DRIVER_VMT		GMOUSEVMT_ADS7843
#include "src/ginput/ginput_driver_mouse.h"

// Get the hardware interface
#include "gmouse_lld_ADS7843_board.h"

#include "iot_xpt2046.h"
#include "stdio.h"
#include "driver/gpio.h"
#include "driver/spi_master.h"
#include "esp_partition.h"
#include "freertos/semphr.h"
#include "esp_log.h"
#include "driver/gpio.h"

#define CMD_X				0xD0
#define CMD_Y				0x90
#define CMD_ENABLE_IRQ		0x80

spi_device_handle_t m_spi = NULL;
SemaphoreHandle_t spi_mux;
position m_pos;

static bool_t MouseXYZ(GMouse* m, GMouseReading* pdr)
{
//    ESP_LOGI("XPT2046", "MouseXYZ");
	// No buttons
	pdr->buttons = 0;
	pdr->z = 0;

	if (getpin_pressed(m)) {
		pdr->z = 1;						// Set to Z_MAX as we are pressed

	    acquire_sem();
		pdr->x = read_value(m, TOUCH_CMD_X);

		pdr->y = read_value(m, TOUCH_CMD_Y);

	    release_sem();
	}
	return TRUE;
}

const GMouseVMT const GMOUSE_DRIVER_VMT[1] = {{
	{
		GDRIVER_TYPE_TOUCH,
		GMOUSE_VFLG_TOUCH | GMOUSE_VFLG_CALIBRATE | GMOUSE_VFLG_CAL_TEST |
			GMOUSE_VFLG_ONLY_DOWN | GMOUSE_VFLG_POORUPDOWN,
		sizeof(GMouse)+GMOUSE_ADS7843_BOARD_DATA_SIZE,
		_gmouseInitDriver,
		_gmousePostInitDriver,
		_gmouseDeInitDriver
	},
	1,				// z_max - (currently?) not supported
	0,				// z_min - (currently?) not supported
	1,				// z_touchon
	0,				// z_touchoff
	{				// pen_jitter
		GMOUSE_ADS7843_PEN_CALIBRATE_ERROR,			// calibrate
		GMOUSE_ADS7843_PEN_CLICK_ERROR,				// click
		GMOUSE_ADS7843_PEN_MOVE_ERROR				// move
	},
	{				// finger_jitter
		GMOUSE_ADS7843_FINGER_CALIBRATE_ERROR,		// calibrate
		GMOUSE_ADS7843_FINGER_CLICK_ERROR,			// click
		GMOUSE_ADS7843_FINGER_MOVE_ERROR			// move
	},
	init_board, 	// init
	0,				// deinit
	MouseXYZ,		// get
	0,				// calsave
	0				// calload
}};

bool_t init_board(GMouse* m, unsigned driverinstance) {

    ESP_LOGI("XPT2046", "Init xpt2046");
    xpt_conf_t xpt_conf = {
            .pin_num_cs = GPIO_NUM_26,
            .pin_num_irq = GPIO_NUM_4,
            .clk_freq = 1 * 1000 * 1000,
            .spi_host = HSPI_HOST,
    };

    gpio_config_t gpio_conf = {
            .pin_bit_mask = GPIO_SEL_4,
            .intr_type = GPIO_PIN_INTR_DISABLE,
            .mode = GPIO_MODE_INPUT,
            .pull_down_en = 0,
            .pull_up_en = 0,
    };
    gpio_config(&gpio_conf);

    spi_device_interface_config_t devcfg = {
        // Use low speed to read ID.
        .clock_speed_hz = xpt_conf.clk_freq,     //Clock out frequency
        .mode = 0,                                //SPI mode 0
        .spics_io_num = xpt_conf.pin_num_cs,     //CS pin
        .queue_size = 7,                          //We want to be able to queue 7 transactions at a time
    };

    spi_bus_add_device((spi_host_device_t) xpt_conf.spi_host, &devcfg, &m_spi);
    return TRUE;
}

bool_t getpin_pressed(GMouse* m) {
//    ESP_LOGI("XPT2046", "get_pin:%d", gpio_get_level(GPIO_NUM_4));
    return (gpio_get_level(GPIO_NUM_4) == 0);
}

uint16_t read_value(GMouse* m, uint16_t port)
{
//    ESP_LOGI("XPT2046", "read_value");
    esp_err_t ret;
    uint8_t datas[3] = {0};
    datas[0] = port;

    spi_transaction_t t = {
        .length = 1 * 8 * 3,              // Len is in bytes, transaction length is in bits.
        .tx_buffer = &datas,                // Data
        .flags = SPI_TRANS_USE_RXDATA,
    };
    ret = spi_device_transmit(m_spi, &t); //Transmit!
    assert(ret == ESP_OK);              // Should have had no issues.

    return (t.rx_data[1] << 8 | t.rx_data[2]) >> 3;
}

static const uint8_t calibration_data[] = { 0x3A, 0x3B, 0x05, 0xBE, 0x84, 0xC2, 0x22, 0xBB,
                                            0x3E, 0xE7, 0x81, 0x43, 0x8A, 0xE6, 0x40, 0xBB,
                                            0x93, 0xAF, 0x2D, 0x3E, 0x73, 0xEE, 0x27, 0xC1 };

bool_t LoadMouseCalibration(unsigned instance, void *data, size_t sz)
{
    ESP_LOGI("XPT2046", "LoadMouseCalibration");
    assert(sizeof(calibration_data) == sz);
    memcpy(data, (void*)calibration_data, sz);
    return TRUE;
}

/**
* @brief   Save a set of mouse calibration data
* @return  TRUE if the save operation was successful.
*
* @param[in] instance       The mouse input instance number
* @param[in] data          The data to save
* @param[in] sz         The size in bytes of the data to retrieve.
*
* @note    This routine is provided by the user application. It is only
*          called if GINPUT_TOUCH_USER_CALIBRATION_SAVE has been set to TRUE in the
*          users gfxconf.h file.
*/
#if GINPUT_TOUCH_USER_CALIBRATION_SAVE
bool_t SaveMouseCalibration(unsigned instance, const void *data, size_t sz)
{
    ESP_LOGI("XPT2046", "SaveMouseCalibration");
//    Serial.printf("\nCalibration data:\n");
    for (size_t i = 0; i < sz; ++i)
    {
//        Serial.printf("0x%02X, ", ((uint8_t*)data)[i]);
        ;
    }
//    Serial.printf("\n\n");

    return TRUE;
}
#endif

#endif /* GFX_USE_GINPUT && GINPUT_NEED_MOUSE */

