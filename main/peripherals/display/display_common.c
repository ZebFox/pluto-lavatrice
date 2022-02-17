#include <stdint.h>
#include "display_common.h"
#include "driver/spi_master.h"
#include "driver/gpio.h"
#include "driver/ledc.h"
#include "peripherals/hardwareprofile.h"

static spi_device_handle_t display_spi;

// data = 1, command = 0
void set_data_command(uint8_t dc) {
    gpio_set_level(HAL_DISPLAY_D, dc);
}

void set_reset(uint8_t res) {
    gpio_set_level(HAL_DISPLAY_RESET, res);
}

static void init_backlight() {
    ledc_timer_config_t ledc_timer = {
        .duty_resolution = LEDC_TIMER_8_BIT,        // resolution of PWM duty
        .freq_hz         = 1000,                    // frequency of PWM signal
        .speed_mode      = LEDC_LOW_SPEED_MODE,     // timer mode
        .timer_num       = LEDC_TIMER_2,            // timer index
        .clk_cfg         = LEDC_AUTO_CLK,           // Auto select the source clock
    };
    // Set configuration of timer0 for high speed channels
    ledc_timer_config(&ledc_timer);

    ledc_channel_config_t ledc_channel = {.channel    = LEDC_CHANNEL_1,
                                          .duty       = 0xFF,
                                          .gpio_num   = HAL_DISPLAY_RETRO,
                                          .speed_mode = LEDC_LOW_SPEED_MODE,
                                          .hpoint     = 0,
                                          .timer_sel  = LEDC_TIMER_2};
    ledc_channel_config(&ledc_channel);
    ledc_stop(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_1, 0);
    set_backlight(100);
}

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

void init_display_spi(void) {
    gpio_config_t io_conf;
    io_conf.intr_type    = GPIO_INTR_DISABLE;
    io_conf.mode         = GPIO_MODE_OUTPUT;
    io_conf.pin_bit_mask = ((1ULL << HAL_DISPLAY_RESET) | (1ULL << HAL_DISPLAY_D));
    io_conf.pull_down_en = 0;
    io_conf.pull_up_en   = 0;
    gpio_config(&io_conf);

    spi_bus_config_t buscfg = {
        .miso_io_num     = -1,
        .mosi_io_num     = HAL_DISPLAY_DOUT,
        .sclk_io_num     = HAL_DISPLAY_CLK,
        .quadwp_io_num   = -1,
        .quadhd_io_num   = -1,
        .max_transfer_sz = MAX_TRANSFER_SIZE,
    };

    spi_device_interface_config_t devcfg = {
        .clock_speed_hz = 15 * 1000 * 1000,     // Clock out at 15 MHz
        .mode           = 0,                   // SPI mode 0
        .spics_io_num   = HAL_DISPLAY_CS,      // CS pin
        .queue_size     = 10,
        .pre_cb         = NULL,
        .post_cb        = NULL,
        .flags          = SPI_DEVICE_HALFDUPLEX,
    };

    // Initialize the SPI bus
    ESP_ERROR_CHECK(spi_bus_initialize(HSPI_HOST, &buscfg, SPI_DMA_CH_AUTO));

    // Attach the LCD to the SPI bus
    ESP_ERROR_CHECK(spi_bus_add_device(HSPI_HOST, &devcfg, &display_spi));

    init_backlight();
}

void disp_spi_send_data(uint8_t *data, unsigned int length) {
    esp_err_t ret;
    if (length == 0)
        return;     // no need to send anything

    spi_transaction_t t = {.length    = length * 8,     // transaction length is in bits
                           .tx_buffer = data,
                           .rx_buffer = NULL};

    ret = spi_device_transmit(display_spi, &t);     // Transmit!
    ESP_ERROR_CHECK(ret);
}

void set_backlight(int percentage) {
    int duty = (percentage * 0xFF) / 100;
    // gpio_set_level(RETRO, percentage > 0);
    if (duty != ledc_get_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_1)) {
        ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_1, duty);
        ledc_update_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_1);
    }
}