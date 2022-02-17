#include <stdint.h>
#include "peripherals/machine_serial.h"


void machine_serial_init(void) {}

int machine_serial_write(uint8_t *buffer, size_t len) {
    return len;
}

int machine_serial_read(uint8_t *buffer, size_t len) {
    return len;
}