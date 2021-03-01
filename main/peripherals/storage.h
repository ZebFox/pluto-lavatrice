#ifndef STORAGE_H_INCLUDED
#define STORAGE_H_INCLUDED

#include <stdint.h>
#include <stdlib.h>

void storage_init(void);

int  load_uint8_option(uint8_t *value, char *key);
void save_uint8_option(uint8_t *value, char *key);
int  load_uint16_option(uint16_t *value, char *key);
void save_uint16_option(uint16_t *value, char *key);
int  load_uint32_option(uint32_t *value, char *key);
void save_uint32_option(uint32_t *value, char *key);
int  load_uint64_option(uint64_t *value, char *key);
void save_uint64_option(uint64_t *value, char *key);
int  load_blob_option(void *value, size_t len, char *key);
void save_blob_option(void *value, size_t len, char *key);

#endif