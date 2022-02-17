#ifndef EVENT_H_INCLUDED
#define EVENT_H_INCLUDED

#include <stdlib.h>
#include <stdint.h>


#define EVENT_SERIALIZED_SIZE 12


typedef enum {
    EVENT_CODICE_PARTENZA = 0,
} event_codice_t;


typedef struct {
    uint64_t istante;
    uint8_t lavaggio;
    uint8_t numero_step;
    uint8_t codice_step;
    uint8_t codice_evento;
} event_t;

size_t event_serialize(uint8_t *buffer, event_t event);
size_t event_deserialize(event_t *event, uint8_t *buffer);


#endif