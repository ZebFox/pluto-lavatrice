#include <assert.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <dirent.h>
#include <stdio.h>
#include "esp_log.h"
#include "microtar.h"
#include "zlib.h"
#include "gzguts.h"
#include "model/model.h"
#include "utils/utils.h"
#include "archive_management.h"
#include "peripherals/storage.h"
#include "configuration.h"


#define DATA_VERSION_FILE "version.txt"

#define TAR_PROGRAMS_PATH "programmi"
#define TAR_PARMAC_PATH   "parametri/parmac.bin"


static int  gzip_mtar_open(mtar_t *tar, const char *zip, const char *mode);
static int  is_archive(struct dirent *dir);
static int  deflate_file(FILE *source, FILE *dest);
static void copy_file_to_tar(mtar_t *tar, const char *dest_path, const char *source_path);


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
    res = 0;

    /* Print all file names and sizes */
    while ((mtar_read_header(&tar, &h)) != MTAR_ENULLRECORD) {
        if ((strcmp(h.name, TAR_PARMAC_PATH) == 0) ||
            (strncmp(h.name, TAR_PROGRAMS_PATH, strlen(TAR_PROGRAMS_PATH)) == 0 &&
             strlen(h.name) > strlen(TAR_PROGRAMS_PATH))) {
            ESP_LOGI(TAG, "Found %s", h.name);

            if (mtar_find(&tar, h.name, &h)) {
                ESP_LOGE(TAG, "Fatal error!");
                mtar_close(&tar);
                return -1;
            }

            if (configuration_copy_from_tar(&tar, h.name, h.size)) {
                ESP_LOGW(TAG, "Unable to copy %s", h.name);
            }
        }
        mtar_next(&tar);
    }

    /* Close archive */
    mtar_close(&tar);
    return res;
}


int archive_management_save_configuration(const char *path, const char *name) {
    mtar_t tar;

    char archive[64] = {0};
    snprintf(archive, sizeof(archive), "%s/%s.WS2020.tar", path, name);

    /* Open archive for reading */
    // int res = gzip_mtar_open(&tar, archive, "w");
    int res = mtar_open(&tar, archive, "w");
    if (res != MTAR_ESUCCESS) {
        ESP_LOGW(TAG, "Could not open archive %s: %s (%i)", archive, mtar_strerror(res), res);
        return -1;
    }

    ESP_LOGI(TAG, "Opened %s", archive);

    // Data version
    char version_string[8] = "";
    snprintf(version_string, sizeof(version_string), "%i", COMPATIBILITY_VERSION);
    mtar_write_file_header(&tar, DATA_VERSION_FILE, strlen(version_string));
    mtar_write_data(&tar, version_string, strlen(version_string));

    // Parmac
    copy_file_to_tar(&tar, TAR_PARMAC_PATH, PATH_FILE_PARMAC);

    // Programs
    mtar_write_dir_header(&tar, "programmi");

    struct dirent *entry;
    DIR           *dir = opendir(PROGRAMS_PATH);
    if (dir != NULL) {
        while ((entry = readdir(dir)) != NULL) {
            char dest[128] = {0};
            if (snprintf(dest, sizeof(dest), "programmi/%s", entry->d_name) > sizeof(dest)) {
                continue;
            }
            char source[128] = {0};
            if (snprintf(source, sizeof(source), "%s/%s", PROGRAMS_PATH, entry->d_name) > sizeof(source)) {
                continue;
            }

            copy_file_to_tar(&tar, dest, source);
        }

        (void)closedir(dir);
    } else {
        ESP_LOGW(TAG, "Couldn't open the directory: %s", strerror(errno));
    }

    /* Finalize -- this needs to be the last thing done before closing */
    mtar_finalize(&tar);

    /* Close archive */
    mtar_close(&tar);

    ESP_LOGI(TAG, "writing %s successful!", archive);

    char gzip_archive[64] = {0};
    snprintf(gzip_archive, sizeof(gzip_archive), "%s/%s.WS2020.tar.gz", path, name);

    // Compression
    FILE *source = fopen(archive, "rb");
    // Appending .gz to the file name makes it extractable on host platforms
    FILE *comp = fopen(gzip_archive, "wb");

    if (comp == NULL || source == NULL) {
        ESP_LOGE(TAG, "Error opening file before compressing");
    }

    res = deflate_file(source, comp);

    fclose(source);
    fclose(comp);

    remove(archive);

    ESP_LOGI(TAG, "writing %s successful (%i)!", gzip_archive, res);

    return 0;
}


size_t archive_management_copy_archive_names(char **strings, name_t **archives, size_t len) {
    free(*archives);

    if (len > 0) {
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
    }

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
            ESP_LOGI(TAG, "Found archive %s", dir->d_name);
            count++;
        }
    }

    if (count > 0) {
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

        *strings = strings_array;
    } else {
        *strings = NULL;
    }

    closedir(d);

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
    if (res != MTAR_ESUCCESS) {
        ESP_LOGW(TAG, "Could not read from archive: %s (%i)", mtar_strerror(res), res);
    }
    return res;
}

static int gzip_mtar_write(mtar_t *tar, const void *data, unsigned size) {
    int written = gzwrite(tar->stream, data, size);

    int res = written == size ? MTAR_ESUCCESS : MTAR_EWRITEFAIL;

    if (res != MTAR_ESUCCESS) {
        ESP_LOGW(TAG, "Could not write to archive: %s (%i - %i)", mtar_strerror(res), written, res);
    }
    return res;
}

static int gzip_mtar_seek(mtar_t *tar, unsigned pos) {
    int res = gzseek(tar->stream, pos, SEEK_SET) >= 0 ? MTAR_ESUCCESS : MTAR_ESEEKFAIL;
    if (res != MTAR_ESUCCESS) {
        ESP_LOGW(TAG, "Could not seek %i in archive: %s (%i)", pos, mtar_strerror(res), res);
    }
    return res;
}

static int gzip_mtar_close(mtar_t *tar) {
    int res = gzclose(tar->stream) == 0 ? MTAR_ESUCCESS : MTAR_EFAILURE;

    if (res != MTAR_ESUCCESS) {
        ESP_LOGW(TAG, "Could not close archive: %s (%i)", mtar_strerror(res), res);
    }

    return res;
}


static int gzip_mtar_open(mtar_t *tar, const char *zip, const char *mode) {
    int           err;
    mtar_header_t h;

    ESP_LOGI(TAG, "Opening %s in mode %s", zip, mode);

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

    gz_statep state = tar->stream;
    ESP_LOGI(TAG, "Opening %s in mode %s %i %i", zip, mode, state->mode, state->err);

    /* Read first header to check it is valid if mode is `r` */
    if (*mode == 'r') {
        err = mtar_read_header(tar, &h);
        if (err != MTAR_ESUCCESS) {
            ESP_LOGW(TAG, "Unable to read tar header");
            mtar_close(tar);
            return err;
        }
    }

    /* Return ok */
    return MTAR_ESUCCESS;
}

void zerr(int ret) {
    switch (ret) {
        case Z_ERRNO:
            ESP_LOGE(TAG, "File I/O Error");
            break;
        case Z_STREAM_ERROR:
            ESP_LOGE(TAG, "Invalid Compression level");
            break;
        case Z_DATA_ERROR:
            ESP_LOGE(TAG, "Data error");
            break;
        case Z_MEM_ERROR:
            ESP_LOGE(TAG, "Memory error");
            break;
        case Z_VERSION_ERROR:
            ESP_LOGE(TAG, "Version error");
            break;
    }
}


static int deflate_file(FILE *source, FILE *dest) {
#define CHUNK_SIZE 8

    int      ret, flush;
    unsigned have;
    z_stream strm;

    /*  Allocating on the stack only works for very small chunk sizes.
        unsigned char in[CHUNK_SIZE];
        unsigned char out[CHUNK_SIZE];
    */
    unsigned char *in  = (unsigned char *)malloc(CHUNK_SIZE * sizeof(char));
    unsigned char *out = (unsigned char *)malloc(CHUNK_SIZE * sizeof(char));

    strm.zalloc = Z_NULL;
    strm.zfree  = Z_NULL;
    strm.opaque = Z_NULL;

    ESP_LOGI(TAG, "Initiated Compression");

    ret = deflateInit2(&strm, Z_NO_COMPRESSION, Z_DEFLATED, 9 | 16, 1, Z_DEFAULT_STRATEGY);
    if (ret != Z_OK) {
        return ret;
    }

    do {
        strm.avail_in = fread(in, 1, CHUNK_SIZE, source);
        if (ferror(source)) {
            zerr(deflateEnd(&strm));
            return Z_ERRNO;
        }
        flush        = feof(source) ? Z_FINISH : Z_NO_FLUSH;
        strm.next_in = in;

        do {
            strm.avail_out = CHUNK_SIZE;
            strm.next_out  = out;
            ret            = deflate(&strm, flush);
            assert(ret != Z_STREAM_ERROR);
            zerr(ret);
            have = CHUNK_SIZE - strm.avail_out;

            if (fwrite(out, sizeof(char), have, dest) != have || ferror(dest)) {
                zerr(deflateEnd(&strm));
                return Z_ERRNO;
            }
        } while (strm.avail_out == 0);
        assert(strm.avail_in == 0);

    } while (flush != Z_FINISH);
    assert(ret == Z_STREAM_END);

    zerr(deflateEnd(&strm));
    free(in);
    free(out);
    return Z_OK;

#undef CHUNK_SIZE
}


static void copy_file_to_tar(mtar_t *tar, const char *dest_path, const char *source_path) {
#define BUFFER_SIZE 2048
    ESP_LOGI(TAG, "Tarring %s to %s", source_path, dest_path);

    FILE *f = fopen(source_path, "r");
    if (f == NULL) {
        ESP_LOGW(TAG, "Cannot open file %s: %s", source_path, strerror(errno));
        return;
    }

    char *buffer = malloc(BUFFER_SIZE);
    if (buffer == NULL) {
        ESP_LOGW(TAG, "Unable to allocate memory");
        fclose(f);
        return;
    }


    fseek(f, 0L, SEEK_END);
    size_t size = ftell(f);
    rewind(f);

    mtar_write_file_header(tar, dest_path, size);

    size_t count = 0;
    while (count < size) {
        int read = fread(buffer, 1, BUFFER_SIZE, f);

        if (read <= 0) {
            break;
        } else {
            count += read;
            mtar_write_data(tar, buffer, read);
        }
    }

    fclose(f);
    free(buffer);
#undef BUFFER_SIZE
}
