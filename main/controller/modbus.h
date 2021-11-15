#ifndef MODBUS_H_INCLUDED
#define MODBUS_H_INCLUDED

#include <stdint.h>
#include <stdlib.h>

typedef enum {
    MODBUS_RESPONSE_ERROR,
} modbus_response_code_t;

typedef struct {
    modbus_response_code_t code;
    uint8_t                address;
    int                    error;
    int                    scanning;
    int                    devices_number;
    union {
        struct {
            uint16_t class;
            uint16_t serial_number;
        };
    };
} modbus_response_t;

void modbus_init(void);
void modbus_read_input_register(void);
void modbus_set_device_output(uint8_t index, uint8_t value );
void modbus_stop_current_operation(void);
void modbus_set_device_output(uint8_t address, uint8_t value);

#endif