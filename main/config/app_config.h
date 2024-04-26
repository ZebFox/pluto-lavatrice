#ifndef APP_CONFIG_H_INCLUDED
#define APP_CONFIG_H_INCLUDED

/*
 *  Macro riguardanti la configurazione dell'applicazione
 */

#define FIRMWARE_VERSION_MAJOR 1
#define FIRMWARE_VERSION_MINOR 'A'
#define FIRMWARE_VERSION_PATCH 12

#define VER_DATI 3

#define APP_CONFIG_BASE_TASK_STACK_SIZE 512

#define LANGUAGE_TIMEOUT 20UL * 1000UL
#define ALARM_TIMEOUT    10UL * 1000UL
#define PAGE_TIMEOUT     60UL * 1000UL

#define LONG_PRESS_INCREASE 4000

#endif
