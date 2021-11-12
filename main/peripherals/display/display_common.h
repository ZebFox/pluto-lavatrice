#ifndef __DISPLAY_MONO_H__
#define __DISPLAY_MONO_H__

#include <stdint.h>

#define DISP_MONO_BUF_SIZE  (128 * 64)
#define DISP_COLOR_BUF_SIZE (320 * 240 / 8)

#define MAX_TRANSFER_SIZE 4000

void set_data_command(uint8_t dc);
void set_reset(uint8_t res);
void init_display_spi();
void disp_spi_send_data(uint8_t *data, unsigned int length);
void set_backlight(int percentage);

#endif