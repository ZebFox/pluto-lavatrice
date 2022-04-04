#ifndef MACHINE_SERIAL_H_INCLUDED
#define MACHINE_SERIAL_H_INCLUDED


#include <stdint.h>
#include <stdlib.h>


void machine_serial_init(void);
int  machine_serial_read(uint8_t *buffer, size_t len);
int  machine_serial_write(uint8_t *buffer, size_t len);
void machine_serial_flush(void);


#endif