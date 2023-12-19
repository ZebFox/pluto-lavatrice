#ifndef MSC_H_INCLUDED
#define MSC_H_INCLUDED


#include "model/model.h"


typedef enum {
    MSC_RESPONSE_CODE_ARCHIVE_EXTRACTION_COMPLETE,
    MSC_RESPONSE_CODE_ARCHIVE_SAVING_COMPLETE,
} msc_response_code_t;


typedef struct {
    msc_response_code_t code;
    union {
        int error;
    };
} msc_response_t;


void                    msc_init(void);
size_t                  msc_read_archives(model_t *pmodel);
removable_drive_state_t msc_is_device_mounted(void);
void                    msc_extract_archive(name_t archive);
int                     msc_get_response(msc_response_t *response);
void                    msc_save_archive(name_t archive);

#endif
