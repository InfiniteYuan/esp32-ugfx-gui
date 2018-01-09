#include "board_ST7735.h"
#include "iot_lcd.h"
#include "spi_lcd.h"
#include "driver/spi_master.h"
#include "freertos/semphr.h"
#include "gfx.h"

spi_device_handle_t spi_wr;
SemaphoreHandle_t spi_mux;
lcd_dc_t dc;

void set_spidevice(spi_device_handle_t mspi_wr)
{
    spi_wr = mspi_wr;
}

void set_semaphore(SemaphoreHandle_t mspi_mux)
{
    spi_mux = mspi_mux;
}

void set_dc(lcd_dc_t mdc)
{
    dc = mdc;
}

void post_init_board(GDisplay *g)
{
    (void) g;
}

void setpin_reset(GDisplay *g, bool_t state)
{
    (void) g;
    (void) state;
}

void acquire_bus(GDisplay *g)
{
    xSemaphoreTakeRecursive(spi_mux, portMAX_DELAY);
}

void release_bus(GDisplay *g)
{
    xSemaphoreGiveRecursive(spi_mux);
}

void acquire_sem()
{
    xSemaphoreTakeRecursive(spi_mux, portMAX_DELAY);
}

void release_sem()
{
    xSemaphoreGiveRecursive(spi_mux);
}

void write_cmd(GDisplay *g, uint8_t cmd)
{
    xSemaphoreTakeRecursive(spi_mux, portMAX_DELAY);
    lcd_cmd(spi_wr, cmd, &dc);
    xSemaphoreGiveRecursive(spi_mux);
}

void write_data(GDisplay *g, uint16_t data)
{
    xSemaphoreTakeRecursive(spi_mux, portMAX_DELAY);
    lcd_data(spi_wr, (uint8_t *)&data, 2, &dc);
    xSemaphoreGiveRecursive(spi_mux);
}

void write_data_byte(GDisplay *g, uint8_t data)
{
    xSemaphoreTakeRecursive(spi_mux, portMAX_DELAY);
    lcd_data(spi_wr, (uint8_t *)&data, 1, &dc);
    xSemaphoreGiveRecursive(spi_mux);
}

void transmitData(uint8_t* data, int length)
{
    xSemaphoreTakeRecursive(spi_mux, portMAX_DELAY);
    lcd_data(spi_wr, (uint8_t *)data, length, &dc);
    xSemaphoreGiveRecursive(spi_mux);
}

void write_data_byte_repeat(GDisplay *g, uint16_t data, int point_num)
{
    if ((point_num * sizeof(uint16_t)) <= (16 * sizeof(uint32_t))) {
        uint16_t* data_buf = (uint16_t*) malloc(point_num * sizeof(uint16_t));
        for (int i = 0; i < point_num; i++) {
            data_buf[i] = data;
        }
        transmitData((uint8_t*) (data_buf), point_num * sizeof(uint16_t));
    } else {
        int gap_point = 1024;
        uint16_t* data_buf = (uint16_t*) malloc(gap_point * sizeof(uint16_t));
        int offset = 0;
        while (point_num > 0) {
            int trans_points = point_num > gap_point ? gap_point : point_num;

            for (int i = 0; i < trans_points; i++) {
                data_buf[i] = data;
            }
            transmitData((uint8_t*) (data_buf), trans_points * sizeof(uint16_t));
            offset += trans_points;
            point_num -= trans_points;
        }
        free(data_buf);
        data_buf = NULL;
    }
}

void write_cmddata(GDisplay *g, uint8_t cmd, uint32_t data)
{
    xSemaphoreTakeRecursive(spi_mux, portMAX_DELAY);
    lcd_cmd(spi_wr, cmd, &dc);
    lcd_data(spi_wr, (uint8_t *)&data, 4, &dc);
    xSemaphoreGiveRecursive(spi_mux);
}

void write_datas(GDisplay *g, uint8_t* data, uint16_t length)
{
    (void) g;
    (void) data;
    (void) length;
}

void set_backlight(GDisplay *g, uint16_t data)
{
    (void) g;
    (void) data;
}
