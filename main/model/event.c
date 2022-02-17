#include <assert.h>
#include "event.h"
#include "gel/serializer/serializer.h"



size_t event_serialize(uint8_t *buffer, event_t event) {
    size_t i = 0;
    i += serialize_uint64_be(&buffer[i], event.istante);
    i += serialize_uint8(&buffer[i], event.lavaggio);
    i += serialize_uint8(&buffer[i], event.numero_step);
    i += serialize_uint8(&buffer[i], event.codice_step);
    i += serialize_uint8(&buffer[i], event.codice_evento);
    assert(i == EVENT_SERIALIZED_SIZE);
    return i;
}


size_t event_deserialize(event_t *event, uint8_t *buffer) {
    size_t i = 0;
    i += deserialize_uint64_be(&event->istante, &buffer[i]);
    i += deserialize_uint8(&event->lavaggio, &buffer[i]);
    i += deserialize_uint8(&event->numero_step, &buffer[i]);
    i += deserialize_uint8(&event->codice_step, &buffer[i]);
    i += deserialize_uint8(&event->codice_evento, &buffer[i]);
    assert(i == EVENT_SERIALIZED_SIZE);
    return i;
}