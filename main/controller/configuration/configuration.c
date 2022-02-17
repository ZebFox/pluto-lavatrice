#include <dirent.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include "peripherals/fs_storage.h"
#include "configuration.h"
#include "esp_log.h"
#include "model/model.h"
#include "model/parmac.h"
#include "model/programs.h"
//#include "model/parametri_macchina.h"
//#include "model/parametri_lavaggio.h"
#include "utils/utils.h"
#include "config/app_config.h"

#define DIR_PROGRAMMI "programmi"
#define DIR_PARAMETRI "parametri"
#define BASENAME(x)   (strrchr(x, '/') + 1)

#define BASE_PATH              LITTLEFS_PARTITION_PATH
#define PROGRAMS_PATH          (BASE_PATH "/data/programmi")
#define PARAMS_PATH            (BASE_PATH "/data/parametri")
#define PATH_FILE_INDICE       (BASE_PATH "/data/programmi/index.txt")
#define PATH_FILE_PARMAC       (BASE_PATH "/data/parametri/parmac.bin")
#define PATH_FILE_DATA_VERSION (BASE_PATH "/data/version.txt")

#define DIR_CHECK(x)                                                                                                   \
    {                                                                                                                  \
        int res = x;                                                                                                   \
        if (res < 0)                                                                                                   \
            ESP_LOGE(TAG, "Errore nel maneggiare una cartella: %s", strerror(errno));                                  \
    }

static int load_parmac(parmac_t *parmac);

static const char *TAG = "Configuration";

/*
 * Funzioni di utilita'
 */

static int count_occurrences(const char *str, char c) {
    int i = 0;
    for (i = 0; str[i]; str[i] == c ? i++ : *str++)
        ;
    return i;
}


static char *nth_strrchr(const char *str, char c, int n) {
    const char *s = &str[strlen(str) - 1];

    while ((n -= (*s == c)) && (s != str))
        s--;

    return (char *)s;
}

static int is_dir(const char *path) {
    struct stat path_stat;
    if (stat(path, &path_stat) < 0)
        return 0;
    return S_ISDIR(path_stat.st_mode);
}

static int is_file(const char *path) {
    struct stat path_stat;
    if (stat(path, &path_stat) < 0)
        return 0;
    return S_ISREG(path_stat.st_mode);
}

static int is_drive(const char *path) {
    struct stat path_stat;
    if (stat(path, &path_stat) < 0)
        return 0;
    return S_ISBLK(path_stat.st_mode);
}

static int dir_exists(char *name) {
    DIR *dir = opendir(name);
    if (dir) {
        closedir(dir);
        return 1;
    }
    return 0;
}

static void create_dir(char *name) {
    DIR_CHECK(mkdir(name, 0766));
}

static int cp(const char *to, const char *from) {
    int     fd_to, fd_from;
    char    buf[4096];
    ssize_t nread;

    fd_from = open(from, O_RDONLY);
    if (fd_from < 0) {
        ESP_LOGE(TAG, "Non sono riuscito ad aprire %s: %s", from, strerror(errno));
        return -1;
    }

    fd_to = open(to, O_WRONLY | O_CREAT | O_TRUNC, 0666);
    if (fd_to < 0) {
        ESP_LOGE(TAG, "Non sono riuscito ad aprire %s: %s", to, strerror(errno));
        close(fd_from);
        return -1;
    }

    while ((nread = read(fd_from, buf, sizeof buf)) > 0) {
        char   *out_ptr = buf;
        ssize_t nwritten;

        do {
            nwritten = write(fd_to, out_ptr, nread);

            if (nwritten >= 0) {
                nread -= nwritten;
                out_ptr += nwritten;
            } else {
                close(fd_from);
                close(fd_to);
                return -1;
            }
        } while (nread > 0);
    }

    close(fd_to);
    close(fd_from);

    return 0;
}


/*
 *  Inizializzazione
 */

void configuration_init(void) {
    if (!dir_exists(PROGRAMS_PATH))
        create_dir(PROGRAMS_PATH);
    if (!dir_exists(PARAMS_PATH))
        create_dir(PARAMS_PATH);

    if (!is_file(PATH_FILE_DATA_VERSION)) {
        configuration_save_data_version();
    }
}

/*
 *  Programmi
 */

void configuration_remove_program_file(char *name) {
    char filename[128];
    snprintf(filename, sizeof(filename), "%s/%s", PROGRAMS_PATH, name);

    if (remove(filename)) {
        ESP_LOGE(TAG, "Non sono riuscito a cancellare il file %s: %s", filename, strerror(errno));
    }
}


int configuration_update_program(programma_lavatrice_t *p) {
    char    filename[128];
    uint8_t buffer[MAX_PROGRAM_SIZE];
    int     res = 0;

    size_t size = serialize_program(buffer, p);

    snprintf(filename, sizeof(filename), "%s/%s", PROGRAMS_PATH, p->filename);
    FILE *fp = fopen(filename, "w");
    if (fwrite(buffer, 1, size, fp) == 0) {
        res = 1;
        ESP_LOGE(TAG, "Non sono riuscito a scrivere il file %s : %s", filename, strerror(errno));
    }

    fclose(fp);
    return res;
}


int configuration_update_program_index(programma_lavatrice_t *p, int len) {
    int res = 0;

    FILE *findex = fopen(PATH_FILE_INDICE, "w");

    if (!findex) {
        ESP_LOGE(TAG, "Operazione di scrittura dell'indice fallita: %s", strerror(errno));
        res = 1;
    } else {
        char buffer[64];

        for (int i = 0; i < len; i++) {
            if (strlen(p[i].filename) == 0)
                continue;

            snprintf(buffer, STRING_NAME_SIZE + 1, "%s\n", p[i].filename);
            if (fwrite(buffer, 1, strlen(buffer), findex) == 0) {
                ESP_LOGE(TAG, "Errore durante la scrittura dell'indice dei programmi: %s", strerror(errno));
                res = 1;
            }
        }
        fclose(findex);
    }

    return res;
}


void configuration_clear_orphan_programs(programma_lavatrice_t *ps, int num) {
    struct dirent *dir;
    char           string[300];

    DIR *d = opendir(PROGRAMS_PATH);

    if (d) {
        while ((dir = readdir(d)) != NULL) {
            int orphan = 1;

            if (strcmp(dir->d_name, "index.txt") == 0)
                continue;

            for (int i = 0; i < num; i++) {
                if (strcmp(ps[i].filename, dir->d_name) == 0) {
                    orphan = 0;
                    break;
                }
            }

            if (orphan) {
                sprintf(string, "%s/%s", PROGRAMS_PATH, dir->d_name);
                remove(string);
            }
        }
        closedir(d);
    }
}


int list_saved_programs(char *names[]) {
    int count = 0;

    FILE *findex = fopen(PATH_FILE_INDICE, "r");
    if (!findex) {
        ESP_LOGI(TAG, "Indice non trovato: %s", strerror(errno));
        return -1;
    } else {
        char filename[STRING_NAME_SIZE + 1];

        while (fgets(filename, STRING_NAME_SIZE + 1, findex)) {
            int len = strlen(filename);

            if (filename[len - 1] == '\n')     // Rimuovo il newline
                filename[len - 1] = 0;

            names[count] = malloc(strlen(filename) + 1);
            strcpy(names[count++], filename);
        }

        fclose(findex);
    }

    return count;
}


int load_saved_programs(model_t *model) {
    uint8_t buffer[MAX_PROGRAM_SIZE];
    char   *names[100];
    name_t  filename;
    int     num = list_saved_programs(names);
    char    path[PATH_MAX];
    int     count = 0;

    if (num < 0) {
        configuration_update_program_index(NULL, 0);
        num = 0;
    }

    for (int i = 0; i < num; i++) {
        sprintf(path, "%s/%s", PROGRAMS_PATH, names[i]);
        memset(filename, 0, sizeof(name_t));
        strcpy(filename, names[i]);
        free(names[i]);

        if (is_file(path)) {
            ESP_LOGD(TAG, "Trovato lavaggio %s", path);
            FILE *fp = fopen(path, "r");

            if (!fp) {
                ESP_LOGE(TAG, "Non sono riuscito ad aprire il file %s: %s", path, strerror(errno));
                continue;
            }

            size_t read = fread(buffer, 1, MAX_PROGRAM_SIZE, fp);

            if (read == 0) {
                ESP_LOGE(TAG, "Non sono riuscito a leggere il file %s: %s", path, strerror(errno));
            } else {
                //TODO: caricare solo i programmi necessari
#if 0
                deserialize_program(&model->prog.programmi[count], buffer);
                memcpy(&model->prog.programmi[count].filename, filename, STRING_NAME_SIZE);
                model->prog.programmi[count].modificato = 0;

                // Controllo dei limiti
                for (size_t i = 0; i < model->prog.programmi[count].num_steps; i++) {
                    programma_lavatrice_t *prog = &model->prog.programmi[count];
                    prog->modificato            = init_parameter_lav_list(&model->prog.parmac, &prog->steps[i]);
                }
#endif

                count++;
            }

            fclose(fp);
        }
    }

    model->prog.num_programmi = count;
    return count;
}


/*
 *  Parametri macchina
 */

static int load_parmac(parmac_t *parmac) {
    uint8_t buffer[PARMAC_SIZE] = {0};
    int     res                 = 0;
    FILE   *f                   = fopen(PATH_FILE_PARMAC, "r");

    if (!f) {
        ESP_LOGI(TAG, "Parametri macchina non trovati");
        return -1;
    } else {
        if ((res = fread(buffer, 1, PARMAC_SIZE, f)) == 0) {
            ESP_LOGE(TAG, "Errore nel caricamento dei parametri macchina: %s", strerror(errno));
        }
        fclose(f);

        if (res == 0) {
            return -1;
        } else {
            return 0;
        }
    }
}


int save_parmac(parmac_t *parmac) {
    uint8_t buffer[PARMAC_SIZE] = {0};
    int     res                 = 0;

    size_t size = model_serialize_parmac(buffer, parmac);

    FILE *f = fopen(PATH_FILE_PARMAC, "w");
    if (!f) {
        ESP_LOGE(TAG, "Non riesco ad aprire %s in scrittura: %s", PATH_FILE_PARMAC, strerror(errno));
        res = 1;
    } else {
        if (fwrite(buffer, 1, size, f) == 0) {
            ESP_LOGE(TAG, "Non riesco a scrivere i parametri macchina: %s", strerror(errno));
            res = 1;
        }
        fclose(f);
    }

    return res;
}


int configuration_save_data_version(void) {
    int res = 0;

    FILE *findex = fopen(PATH_FILE_DATA_VERSION, "w");
    if (!findex) {
        ESP_LOGE(TAG, "Operazione di scrittura della versione fallita: %s", strerror(errno));
        res = -1;
    } else {
        char string[64];
        snprintf(string, 64, "%i", VER_DATI);
        fwrite(string, 1, strlen(string), findex);
        fclose(findex);
    }

    return res;
}


int load_all_data(model_t *model) {
    int err = load_parmac(&model->prog.parmac);
    parmac_init(model, 0);

    if (err) {
        //change_machine_name(model, NOME_MACCHINA_NUOVA);
        parmac_init(model, 1);
        save_parmac(&model->prog.parmac);
    }

    load_saved_programs(model);
    //configuration_clear_orphan_programs(model->prog.programmi, model->prog.num_programmi);
    //model_select_program(model, 0);

    return configuration_read_local_data_version();
}


int configuration_read_local_data_version(void) {
    FILE *f          = fopen(PATH_FILE_DATA_VERSION, "r");
    char  buffer[17] = {0};

    if (f) {
        if (fread(buffer, 1, 16, f) <= 0) {
            fclose(f);
            return -1;
        }
        fclose(f);

        int res = atoi(buffer);
        return res;
    } else {
        return -1;
    }
}