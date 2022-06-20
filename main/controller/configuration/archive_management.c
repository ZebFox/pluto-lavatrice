#include <assert.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <dirent.h>
#include <stdio.h>
#include "esp_log.h"
#include "microtar.h"
#include "zlib.h"
#include "model/model.h"
#include "utils/utils.h"
#include "archive_management.h"
#include "configuration.h"


#define DATA_VERSION_FILE "version.txt"

#define PROGRAMS_PATH "programmi"
#define PARMAC_PATH   "parametri/parmac.bin"


static int gzip_mtar_open(mtar_t *tar, const char *zip, const char *mode);
static int is_archive(struct dirent *dir);


static const char *TAG = "Archive management";


int archive_management_extract_configuration(const char *zipped_archive) {
    mtar_t        tar;
    mtar_header_t h;

    char archive[128] = {0};
    strcpy(archive, zipped_archive);
    char *dot = strrchr(archive, '.');
    if (dot == NULL) {
        ESP_LOGW(TAG, "invalid zip name: %s", zipped_archive);
        return -1;
    }
    *dot = '\0';

    /* Open archive for reading */
    int res = gzip_mtar_open(&tar, zipped_archive, "r");
    if (res != MTAR_ESUCCESS) {
        ESP_LOGW(TAG, "Could not open archive %s: %s (%i)", zipped_archive, mtar_strerror(res), res);
        return -1;
    }

    configuration_delete_all();

    /* Print all file names and sizes */
    while ((mtar_read_header(&tar, &h)) != MTAR_ENULLRECORD) {
        if ((strcmp(h.name, PARMAC_PATH) == 0) || (strncmp(h.name, PROGRAMS_PATH, strlen(PROGRAMS_PATH)) == 0)) {
            ESP_LOGI(TAG, "Found %s", h.name);

            if (mtar_find(&tar, h.name, &h)) {
                ESP_LOGE(TAG, "Fatal error!");
                mtar_close(&tar);
                return -1;
            }

            if (configuration_copy_from_tar(&tar, h.name, h.size)) {
                ESP_LOGW(TAG, "Unable to copy %s", h.name);
                mtar_close(&tar);
                return -1;
            }
        }
        mtar_next(&tar);
    }

    /* Close archive */
    mtar_close(&tar);
    return 0;
}


size_t archive_management_copy_archive_names(char **strings, name_t **archives, size_t len) {
    free(*archives);
    name_t *archives_array = malloc(sizeof(name_t) * len);
    assert(archives_array != NULL);


    char tmp[64] = {0};

    for (size_t i = 0; i < len; i++) {
        strcpy(tmp, strings[i]);
        char *suffix = strstr(tmp, ARCHIVE_SUFFIX);
        assert(suffix != NULL);
        *suffix = '\0';
        strcpy(archives_array[i], tmp);
    }

    *archives = archives_array;

    return len;
}


int archive_management_list_archives(const char *path, char ***strings) {
    size_t         count = 0;
    DIR           *d;
    struct dirent *dir;
    d = opendir(path);
    if (d == NULL) {
        ESP_LOGW(TAG, "Unable to open folder %s: %s", path, strerror(errno));
        return -1;
    }

    while ((dir = readdir(d)) != NULL) {
        if (is_archive(dir)) {
            count++;
        }
    }
    rewinddir(d);

    char **strings_array = malloc(sizeof(char *) * count);
    assert(strings_array != NULL);
    size_t i = 0;
    while ((dir = readdir(d)) != NULL && i < count) {
        if (is_archive(dir)) {
            size_t len       = strlen(dir->d_name) + 1;
            strings_array[i] = malloc(len);
            assert(strings_array[i] != NULL);
            memset(strings_array[i], 0, len - 1);
            strcpy(strings_array[i], dir->d_name);
            i++;
        }
    }

    closedir(d);

    *strings = strings_array;
    return (int)count;
}


static int is_archive(struct dirent *dir) {
    if (dir->d_type == DT_REG) {
        char *found = strstr(dir->d_name, ARCHIVE_SUFFIX);
        if (found != NULL && found + strlen(ARCHIVE_SUFFIX) == dir->d_name + strlen(dir->d_name)) {
            return 1;
        }
    }
    return 0;
}


static int gzip_mtar_read(mtar_t *tar, void *data, unsigned size) {
    int res = gzread(tar->stream, data, size) == size ? MTAR_ESUCCESS : MTAR_EREADFAIL;
    return res;
}

static int gzip_mtar_write(mtar_t *tar, const void *data, unsigned size) {
    int res = gzwrite(tar->stream, data, size) == size ? MTAR_ESUCCESS : MTAR_EWRITEFAIL;
    return res;
}

static int gzip_mtar_seek(mtar_t *tar, unsigned pos) {
    int res = gzseek(tar->stream, pos, SEEK_SET) >= 0 ? MTAR_ESUCCESS : MTAR_ESEEKFAIL;
    return res;
}

static int gzip_mtar_close(mtar_t *tar) {
    int res = gzclose(tar->stream) == 0 ? MTAR_ESUCCESS : MTAR_EFAILURE;
    return res;
}


static int gzip_mtar_open(mtar_t *tar, const char *zip, const char *mode) {
    int           err;
    mtar_header_t h;

    /* Init tar struct and functions */
    memset(tar, 0, sizeof(*tar));
    tar->write = gzip_mtar_write;
    tar->read  = gzip_mtar_read;
    tar->seek  = gzip_mtar_seek;
    tar->close = gzip_mtar_close;

    /* Assure mode is always binary */
    if (strchr(mode, 'r')) {
        mode = "rb";
    } else if (strchr(mode, 'w')) {
        mode = "wb";
    } else if (strchr(mode, 'a')) {
        mode = "ab";
    }
    /* Open file */
    tar->stream = gzopen(zip, mode);
    if (tar->stream == NULL) {
        ESP_LOGW(TAG, "Unable to open zip archive %s", zip);
        return MTAR_EOPENFAIL;
    }
    /* Read first header to check it is valid if mode is `r` */
    if (*mode == 'r') {
        err = mtar_read_header(tar, &h);
        if (err != MTAR_ESUCCESS) {
            mtar_close(tar);
            return err;
        }
    }

    /* Return ok */
    return MTAR_ESUCCESS;
}
