/*
 * Modulo di inizializzazione del sistema.
 * Contiene le funzioni di configurazione (specifica) di I2C ed SPI
 */
#include <assert.h>
#include "driver/spi_master.h"
#include "driver/gpio.h"
#include "driver/i2c.h"
#include "hardwareprofile.h"


void system_i2c_init(void) {
    int       port     = I2C_NUM_0;
    int       sda_pin  = HAL_I2C_SDA;
    int       scl_pin  = HAL_I2C_SCL;
    int       speed_hz = 400000;

    i2c_config_t conf = {
        .mode             = I2C_MODE_MASTER,
        .sda_io_num       = sda_pin,
        .sda_pullup_en    = GPIO_PULLUP_ENABLE,
        .scl_io_num       = scl_pin,
        .scl_pullup_en    = GPIO_PULLUP_ENABLE,
        .master.clk_speed = speed_hz,
    };

    ESP_ERROR_CHECK(i2c_param_config(port, &conf));

    ESP_ERROR_CHECK(i2c_driver_install(port, I2C_MODE_MASTER, 0,
                                       0 /*I2C_MASTER_RX_BUF_DISABLE, I2C_MASTER_TX_BUF_DISABLE */,
                                       0 /* intr_alloc_flags */));
}