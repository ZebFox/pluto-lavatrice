#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "freertos/semphr.h"
#include "freertos/event_groups.h"
#include "esp_err.h"
#include "esp_log.h"
#include "usb/usb_host.h"
#include "msc_host.h"
#include "msc_host_vfs.h"
#include "ffconf.h"
#include "ff.h"
#include "esp_vfs.h"
#include "errno.h"
#include "hal/usb_hal.h"
#include "archive_management.h"
#include "config/app_config.h"
#include "utils/utils.h"
#include "msc.h"


#define EVENT_DRIVE_DETECTED         0x01
#define EVENT_REAEDY_TO_UNINSTALL    0x02
#define EVENT_USB_HOST_LIB_INSTALLED 0x04
#define EVENT_DRIVE_MOUNTED          0x08
#define EVENT_USB_HOST_LIB_DONE      0x10

#define MOUNTPOINT "/usb"


typedef enum {
    TASK_MESSAGE_CODE_LOAD_ARCHIVE,
    TASK_MESSAGE_CODE_SAVE_ARCHIVE,
} task_message_code_t;


typedef struct {
    task_message_code_t code;
    union {
        name_t archive_name;
    };
} task_message_t;


static void msc_event_cb(const msc_host_event_t *event, void *arg);
static void msc_task(void *args);
static void msc_host_lib_task(void *args);
static void msc_install_host_lib(void);
static void msc_install_device(uint8_t device_address);
static void msc_uninstall(msc_host_device_handle_t handle);


static const char *TAG = "MSC";


static QueueHandle_t            event_queue;
static QueueHandle_t            message_queue;
static QueueHandle_t            response_queue;
static SemaphoreHandle_t        sem;
static EventGroupHandle_t       event_group;
static msc_host_vfs_handle_t    vfs_handle;
static msc_host_device_handle_t msc_device;
static size_t                   listed_archives_num = 0;
static char                   **listed_archives     = NULL;



void msc_init(void) {
    static StaticEventGroup_t static_event_group;
    event_group = xEventGroupCreateStatic(&static_event_group);

    static StaticSemaphore_t static_semaphore;
    sem = xSemaphoreCreateMutexStatic(&static_semaphore);

    static uint8_t       event_queue_buffer[sizeof(msc_host_event_t) * 4] = {0};
    static StaticQueue_t static_event_queue;
    event_queue = xQueueCreateStatic(sizeof(event_queue_buffer) / sizeof(msc_host_event_t), sizeof(msc_host_event_t),
                                     event_queue_buffer, &static_event_queue);

    static uint8_t       message_queue_buffer[sizeof(task_message_t) * 4] = {0};
    static StaticQueue_t static_message_queue;
    message_queue = xQueueCreateStatic(sizeof(message_queue_buffer) / sizeof(task_message_t), sizeof(task_message_t),
                                       message_queue_buffer, &static_message_queue);

    static uint8_t       response_queue_buffer[sizeof(msc_response_t) * 4] = {0};
    static StaticQueue_t static_response_queue;
    response_queue = xQueueCreateStatic(sizeof(response_queue_buffer) / sizeof(msc_response_t), sizeof(msc_response_t),
                                        response_queue_buffer, &static_response_queue);

    msc_install_host_lib();

    static StaticTask_t static_task;
    static StackType_t  task_stack[APP_CONFIG_BASE_TASK_STACK_SIZE * 10] = {0};
    xTaskCreateStatic(msc_task, TAG, sizeof(task_stack) / sizeof(task_stack[0]), NULL, 2, task_stack, &static_task);

    static StaticTask_t static_host_lib_task;
    static StackType_t  host_lib_task_stack[APP_CONFIG_BASE_TASK_STACK_SIZE * 4] = {0};
    xTaskCreateStatic(msc_host_lib_task, "msc host lib task",
                      sizeof(host_lib_task_stack) / sizeof(host_lib_task_stack[0]), NULL, 2, host_lib_task_stack,
                      &static_host_lib_task);
}


size_t msc_read_archives(model_t *pmodel) {
    if (msc_is_device_mounted() == REMOVABLE_DRIVE_STATE_MOUNTED) {
        xSemaphoreTake(sem, portMAX_DELAY);
        pmodel->system.num_archivi =
            archive_management_copy_archive_names(listed_archives, &pmodel->system.archivi, listed_archives_num);
        xSemaphoreGive(sem);
        return pmodel->system.num_archivi;
    } else {
        pmodel->system.num_archivi = 0;
        free(pmodel->system.archivi);
        pmodel->system.archivi = NULL;
        return 0;
    }
}


removable_drive_state_t msc_is_device_mounted(void) {
    uint32_t res = xEventGroupWaitBits(event_group, EVENT_DRIVE_MOUNTED | EVENT_USB_HOST_LIB_DONE, pdFALSE, pdFALSE, 0);
    if ((res & EVENT_DRIVE_MOUNTED) > 0) {
        return REMOVABLE_DRIVE_STATE_MOUNTED;
    } else if ((res & EVENT_USB_HOST_LIB_DONE) > 0) {
        return REMOVABLE_DRIVE_STATE_INVALID;
    }
    return REMOVABLE_DRIVE_STATE_MISSING;
}


void msc_extract_archive(name_t archive) {
    task_message_t message = {.code = TASK_MESSAGE_CODE_LOAD_ARCHIVE};
    strcpy(message.archive_name, archive);
    xQueueSend(message_queue, &message, portMAX_DELAY);
}


void msc_save_archive(name_t archive) {
    task_message_t message = {.code = TASK_MESSAGE_CODE_SAVE_ARCHIVE};
    strcpy(message.archive_name, archive);
    xQueueSend(message_queue, &message, portMAX_DELAY);
}


int msc_get_response(msc_response_t *response) {
    return xQueueReceive(response_queue, response, 0) == pdTRUE;
}


static void msc_install_host_lib(void) {
    const usb_host_config_t host_config = {
        .skip_phy_setup = false,
        .intr_flags     = ESP_INTR_FLAG_LEVEL1,
    };
    ESP_ERROR_CHECK(usb_host_install(&host_config));

    const msc_host_driver_config_t msc_config = {
        .create_backround_task = true,
        .task_priority         = 5,
        .stack_size            = APP_CONFIG_BASE_TASK_STACK_SIZE * 4,
        .callback              = msc_event_cb,
    };
    ESP_ERROR_CHECK(msc_host_install(&msc_config));

    xEventGroupSetBits(event_group, EVENT_USB_HOST_LIB_INSTALLED);
    xEventGroupClearBits(event_group, EVENT_REAEDY_TO_UNINSTALL);
}


static void msc_install_device(uint8_t device_address) {
    ESP_ERROR_CHECK(msc_host_install_device(device_address, &msc_device));

    //msc_host_print_descriptors(msc_device);

    msc_host_device_info_t info;
    ESP_ERROR_CHECK(msc_host_get_device_info(msc_device, &info));

    const esp_vfs_fat_mount_config_t mount_config = {
        .format_if_mount_failed = false,
        .max_files              = 3,
        .allocation_unit_size   = 1024,
    };

    ESP_ERROR_CHECK(msc_host_vfs_register(msc_device, MOUNTPOINT, &mount_config, &vfs_handle));
}


static void msc_uninstall(msc_host_device_handle_t handle) {
    ESP_ERROR_CHECK(msc_host_vfs_unregister(vfs_handle));
    ESP_ERROR_CHECK(msc_host_uninstall_device(handle));
    ESP_ERROR_CHECK(msc_host_uninstall());

    xEventGroupWaitBits(event_group, EVENT_REAEDY_TO_UNINSTALL, pdFALSE, pdTRUE, portMAX_DELAY);
    // ESP_ERROR_CHECK(usb_host_uninstall());
    usb_host_uninstall();

    xEventGroupClearBits(event_group, EVENT_USB_HOST_LIB_INSTALLED);
    ESP_LOGI(TAG, "MSC device uninstalled");
}


static void msc_event_cb(const msc_host_event_t *event, void *arg) {
    xQueueSend(event_queue, event, 4);
}


static void msc_task(void *args) {
    (void)args;

    for (;;) {
        msc_host_event_t event;
        if (xQueueReceive(event_queue, &event, pdMS_TO_TICKS(100))) {
            switch (event.event) {
                case MSC_DEVICE_CONNECTED:
                    ESP_LOGI(TAG, "MSC device connected");
                    xEventGroupSetBits(event_group, EVENT_DRIVE_DETECTED);
                    msc_install_device(event.device.address);

                    char **strings = NULL;
                    int    res     = archive_management_list_archives(MOUNTPOINT, &strings);

                    xSemaphoreTake(sem, portMAX_DELAY);
                    utils_free_string_list(listed_archives, listed_archives_num);
                    listed_archives = strings;
                    if (res > 0) {
                        listed_archives_num = res;
                    } else {
                        listed_archives_num = 0;
                    }

                    for (size_t i = 0; i < listed_archives_num; i++) {
                        ESP_LOGI(TAG, "Archive %zu: %s", i, strings[i]);
                    }
                    xSemaphoreGive(sem);

                    xEventGroupSetBits(event_group, EVENT_DRIVE_MOUNTED);
#if 0
                    msc_uninstall(msc_device);
                    ESP_LOGI(TAG, "Done");
                    msc_install_host_lib();
#endif
                    break;

                case MSC_DEVICE_DISCONNECTED:
                    ESP_LOGI(TAG, "MSC device disconnected");
                    msc_uninstall(msc_device);
                    xEventGroupClearBits(event_group, EVENT_DRIVE_MOUNTED);
                    xEventGroupSetBits(event_group, EVENT_USB_HOST_LIB_DONE);
                    break;

                default:
                    ESP_LOGI(TAG, "MSC event %i", event.event);
                    break;
            }
        }

        task_message_t message;
        if (xQueueReceive(message_queue, &message, pdMS_TO_TICKS(100))) {
            switch (message.code) {
                case TASK_MESSAGE_CODE_LOAD_ARCHIVE: {
                    char string[128] = {0};
                    snprintf(string, sizeof(string), "%s/%s%s", MOUNTPOINT, message.archive_name, ARCHIVE_SUFFIX);
                    msc_response_t response = {.code = MSC_RESPONSE_CODE_ARCHIVE_EXTRACTION_COMPLETE};
                    response.error          = archive_management_extract_configuration(string);
                    xQueueSend(response_queue, &response, portMAX_DELAY);
                    break;
                }

                case TASK_MESSAGE_CODE_SAVE_ARCHIVE: {
                    msc_response_t response = {.code = MSC_RESPONSE_CODE_ARCHIVE_SAVING_COMPLETE};
                    response.error          = archive_management_save_configuration(MOUNTPOINT, message.archive_name);
                    xQueueSend(response_queue, &response, portMAX_DELAY);
                    break;
                }
            }
        }
    }

    vTaskDelete(NULL);
}


// Handles common USB host library events
static void msc_host_lib_task(void *args) {
    (void)args;

    for (;;) {
        xEventGroupWaitBits(event_group, EVENT_USB_HOST_LIB_INSTALLED, pdFALSE, pdTRUE, portMAX_DELAY);

        uint32_t event_flags;
        usb_host_lib_handle_events(portMAX_DELAY, &event_flags);
        // Release devices once all clients has deregistered
        if (event_flags & USB_HOST_LIB_EVENT_FLAGS_NO_CLIENTS) {
            ESP_LOGI(TAG, "Releasing clients");
            usb_host_device_free_all();
        }
        // Give ready_to_uninstall_usb semaphore to indicate that USB Host library
        // can be deinitialized, and terminate this task.
        if (event_flags & USB_HOST_LIB_EVENT_FLAGS_ALL_FREE) {
            ESP_LOGI(TAG, "Usb host ready to uninstall");
            xEventGroupSetBits(event_group, EVENT_REAEDY_TO_UNINSTALL);
            break;
        }
    }

    vTaskDelete(NULL);
}
