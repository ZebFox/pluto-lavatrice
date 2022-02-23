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
    VIEW_CONTROLLER_COMMAND_CODE_TEST,
    VIEW_CONTROLLER_COMMAND_CODE_TEST_DIGOUT,
    VIEW_CONTROLLER_COMMAND_CODE_TEST_REFRESH,
    VIEW_CONTROLLER_COMMAND_CODE_DIGOUT_TURNOFF,
    VIEW_CONTROLLER_COMMAND_CODE_PARAMETERS_SAVE,
    VIEW_CONTROLLER_COMMAND_CODE_RESET_RAM,
    VIEW_CONTROLLER_COMMAND_CODE_UPDATE_LED,
    VIEW_CONTROLLER_COMMAND_CODE_UPDATE_CONTRAST,
    VIEW_CONTROLLER_COMMAND_CODE_PRIVATE_PARAMETERS_SAVE,
    VIEW_CONTROLLER_COMMAND_CODE_CREATE_PROGRAM,
    VIEW_CONTROLLER_COMMAND_CODE_LOAD_PROGRAM,
    VIEW_CONTROLLER_COMMAND_CODE_UPDATE_PROGRAM,
} view_controller_command_code_t;


typedef struct {
    view_controller_command_code_t code;
    union {
        struct {
            int output;
            int value;
        };
        int      test;
        uint16_t num;
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
    VIEW_EVENT_CODE_COIN,
    VIEW_EVENT_CODE_STATO_UPDATE,
    VIEW_EVENT_CODE_ALARM,
    VIEW_EVENT_CODE_DATA_REFRESH,
    VIEW_EVENT_CODE_PROGRAM_SAVED,
    VIEW_EVENT_CODE_PROGRAM_LOADED,
    VIEW_EVENT_CODE_TIMER,
    VIEW_EVENT_CODE_OPEN,
} view_event_code_t;


typedef struct {
    view_event_code_t code;
    union {
        keypad_update_t key_event;
        int             timer_id;
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
