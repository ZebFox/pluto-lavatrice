#ifndef VIEW_TYPES_H_INCLUDED
#define VIEW_TYPES_H_INCLUDED

#include <stdint.h>
#include "gel/keypad/keypad.h"
#include "model/model.h"


#define PMAN_NAVIGATION_DEPTH 4
#define PMAN_VIEW_NULL        0
#define PMAN_DATA_NULL        NULL

#define VIEW_EMPTY_MSG                                                                                                 \
    ((view_message_t){.vmsg = {.code = VIEW_PAGE_COMMAND_CODE_NOTHING},                                                \
                      .cmsg = {.code = VIEW_CONTROLLER_COMMAND_CODE_NOTHING}})


typedef enum {
    VIEW_PAGE_COMMAND_CODE_NOTHING = 0,
    VIEW_PAGE_COMMAND_CODE_REBASE,
    VIEW_PAGE_COMMAND_CODE_SWAP_PAGE,
    VIEW_PAGE_COMMAND_CODE_SWAP_PAGE_EXTRA,
    VIEW_PAGE_COMMAND_CODE_CHANGE_PAGE,
    VIEW_PAGE_COMMAND_CODE_CHANGE_PAGE_EXTRA,
    VIEW_PAGE_COMMAND_CODE_BACK,
    VIEW_PAGE_COMMAND_CODE_RESET_PAGE,
} view_page_command_code_t;

typedef struct {
    view_page_command_code_t code;

    union {
        struct {
            const void *page;
            void       *extra;
        };
    };
} view_page_command_t;


typedef enum {
    VIEW_CONTROLLER_COMMAND_CODE_NOTHING = 0,
    VIEW_CONTROLLER_COMMAND_CODE_UPDATE_PWM,
    VIEW_CONTROLLER_COMMAND_CODE_START_PROGRAM,
    VIEW_CONTROLLER_COMMAND_CODE_SAVE_PARMAC,
    VIEW_CONTROLLER_COMMAND_CODE_SEND_PARMAC,
    VIEW_CONTROLLER_COMMAND_CODE_SEND_PARMAC_AND_TURNOFF,
    VIEW_CONTROLLER_COMMAND_CODE_REMOVE_PROGRAM,
    VIEW_CONTROLLER_COMMAND_CODE_PAUSE,
    VIEW_CONTROLLER_COMMAND_CODE_STOP,
    VIEW_CONTROLLER_COMMAND_CODE_FORCE_DRAIN,
    VIEW_CONTROLLER_COMMAND_CODE_AZZERA_LITRI,
    VIEW_CONTROLLER_COMMAND_CODE_OFFSET_PRESSIONE,
    VIEW_CONTROLLER_COMMAND_CODE_TEST,
    VIEW_CONTROLLER_COMMAND_CODE_FORZA_APERTURA_OBLO,
    VIEW_CONTROLLER_COMMAND_CODE_AZZERA_ALLARMI,
    VIEW_CONTROLLER_COMMAND_CODE_TEST_DIGOUT,
    VIEW_CONTROLLER_COMMAND_CODE_TEST_DIGOUT_MULTI,
    VIEW_CONTROLLER_COMMAND_CODE_TEST_LEVEL_CONTROL,
    VIEW_CONTROLLER_COMMAND_CODE_TEST_REFRESH,
    VIEW_CONTROLLER_COMMAND_CODE_TEST_DAC_CONTROL,
    VIEW_CONTROLLER_COMMAND_CODE_TEST_LED_CONTROl,
    VIEW_CONTROLLER_COMMAND_CODE_TEST_CLEAR_CREDIT,
    VIEW_CONTROLLER_COMMAND_CODE_DIGOUT_TURNOFF,
    VIEW_CONTROLLER_COMMAND_CODE_CLEAR_LITERS,
    VIEW_CONTROLLER_COMMAND_CODE_PARAMETERS_SAVE,
    VIEW_CONTROLLER_COMMAND_CODE_RESET_RAM,
    VIEW_CONTROLLER_COMMAND_CODE_UPDATE_LED,
    VIEW_CONTROLLER_COMMAND_CODE_UPDATE_CONTRAST,
    VIEW_CONTROLLER_COMMAND_CODE_SAVE_CONTRAST,
    VIEW_CONTROLLER_COMMAND_CODE_PRIVATE_PARAMETERS_SAVE,
    VIEW_CONTROLLER_COMMAND_CODE_CREATE_PROGRAM,
    VIEW_CONTROLLER_COMMAND_CODE_CLONE_PROGRAM,
    VIEW_CONTROLLER_COMMAND_CODE_LOAD_PROGRAM,
    VIEW_CONTROLLER_COMMAND_CODE_UPDATE_PROGRAM,
    VIEW_CONTROLLER_COMMAND_CODE_RETRY_COMMUNICATION,
    VIEW_CONTROLLER_COMMAND_CODE_CHANGE_AB_COMMUNICATION,
    VIEW_CONTROLLER_COMMAND_CODE_SEND_DEBUG_CODE,
    VIEW_CONTROLLER_COMMAND_CODE_UPDATE_STATISTICS,
    VIEW_CONTROLLER_COMMAND_CODE_RELOAD_PREVIEWS,
    VIEW_CONTROLLER_COMMAND_CODE_APRI_OBLO,
    VIEW_CONTROLLER_COMMAND_CODE_CHIUDI_OBLO,
    VIEW_CONTROLLER_COMMAND_CODE_COLPO_SAPONE,
    VIEW_CONTROLLER_COMMAND_CODE_CONTROLLO_SAPONE,
    VIEW_CONTROLLER_COMMAND_CODE_AGGIORNA_ORA_DATA,
    VIEW_CONTROLLER_COMMAND_CODE_MODIFICA_PARAMETRI_IN_LAVAGGIO,
    VIEW_CONTROLLER_COMMAND_CODE_RESET,
    VIEW_CONTROLLER_COMMAND_CODE_LOAD_ARCHIVE,
} view_controller_command_code_t;


typedef struct {
    view_controller_command_code_t code;
    union {
        struct {
            uint8_t output;
            uint8_t value;
        };
        int      test;
        uint16_t num;
        struct {
            uint16_t duration;
            uint16_t speed;
            uint16_t level;
            uint16_t temperature;
        };
        struct {
            uint16_t source;
            uint16_t destination;
        };
        name_t archive_name;
    };
} view_controller_command_t;


typedef struct {
    view_page_command_t       vmsg;
    view_controller_command_t cmsg;
} view_message_t;

typedef uint8_t view_t;


typedef enum {
    VIEW_EVENT_CODE_KEYPAD,
    VIEW_EVENT_CODE_MODEL_UPDATE,
    VIEW_EVENT_CODE_DEVICE_UPDATE,
    VIEW_EVENT_CODE_CONFIGURATION_LOADED,
    VIEW_EVENT_CODE_COIN,
    VIEW_EVENT_CODE_STATO_UPDATE,
    VIEW_EVENT_CODE_ALARM,
    VIEW_EVENT_CODE_DATA_REFRESH,
    VIEW_EVENT_CODE_PROGRAM_SAVED,
    VIEW_EVENT_CODE_PROGRAM_LOADED,
    VIEW_EVENT_CODE_PROGRAM_CHANGE_DONE,
    VIEW_EVENT_CODE_PREVIEWS_LOADED,
    VIEW_EVENT_CODE_TIMER,
    VIEW_EVENT_CODE_OPEN,
} view_event_code_t;


typedef struct {
    view_event_code_t code;
    union {
        int             error;
        keypad_update_t key_event;
        uint32_t        timer_id;
        unsigned int    coins;
        uint16_t        num;
    };
} view_event_t;


typedef view_message_t pman_message_t;

typedef view_event_t pman_event_t;

typedef void *pman_page_data_t;

typedef model_t *pman_model_t;

typedef view_t pman_view_t;

#endif
