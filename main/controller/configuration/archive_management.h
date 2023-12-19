#ifndef ARCHIVE_MANAGEMENT_H_INCLUDED
#define ARCHIVE_MANAGEMENT_H_INCLUDED


#include "model/model.h"


#define ARCHIVE_SUFFIX ".WS2020.tar.gz"


int    archive_management_list_archives(const char *path, char ***strings);
size_t archive_management_copy_archive_names(char **strings, name_t **archives, size_t len);
int    archive_management_extract_configuration(const char *zipped_archive);
int    archive_management_save_configuration(const char *path, const char *name);


#endif
