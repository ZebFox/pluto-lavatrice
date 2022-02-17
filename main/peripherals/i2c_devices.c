#include "i2c_common/i2c_common.h"
#include "i2c_devices/rtc/M41T81/m41t81.h"
#include "i2c_ports/esp-idf/esp_idf_i2c_port.h"

i2c_driver_t rtc_driver = {
    .device_address = M41T11_DEFAULT_ADDRESS,
    .i2c_transfer   = esp_idf_i2c_port_transfer,
    .ack_polling    = esp_idf_i2c_ack_polling,
};

