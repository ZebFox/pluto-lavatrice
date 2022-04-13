#include "rom/miniz.h"
#include "microtar.h"
// Use microtar as well


void archive_management_extract_configuration(void) {
    mtar_t        tar;
    mtar_header_t h;
    char         *p;

    /* Open archive for reading */
    mtar_open(&tar, "test.tar", "r");

    /* Print all file names and sizes */
    while ((mtar_read_header(&tar, &h)) != MTAR_ENULLRECORD) {
        printf("%s (%d bytes)\n", h.name, h.size);
        mtar_next(&tar);
    }

    /* Load and print contents of file "test.txt" */
    mtar_find(&tar, "test.txt", &h);
    p = calloc(1, h.size + 1);
    mtar_read_data(&tar, p, h.size);
    printf("%s", p);
    free(p);

    /* Close archive */
    mtar_close(&tar);
}