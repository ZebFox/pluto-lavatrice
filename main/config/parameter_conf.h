#ifndef GEL_CONF_H_INCLUDED
#define GEL_CONF_H_INCLUDED

#include <stdint.h>
#include <stdlib.h>


typedef struct __attribute__((__packed__)) {
    const char **descrizione;
    void (*format)(char *string, uint16_t, const void *);
    const char   *fmt;
    const char ***valori;
} parameter_user_data_t;

#define GEL_PARAMETER_USER_DATA         parameter_user_data_t
#define GEL_PARAMETER_MAX_SIZE          2
#define GEL_PARAMETER_HANDLE_ATTRIBUTES __attribute__((__packed__))

#endif
