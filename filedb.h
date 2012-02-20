#ifndef PACFIND_FILEDB_H
#define PACFIND_FILEDB_H

#include <sys/stat.h>
#include <stdio.h>

typedef enum filedb_status {
    FD_OK,
    FD_Skip,
    FD_Stop,
} filedb_status_t;

typedef filedb_status_t (*filedb_visit_t) (const char *filename, const struct stat *stat, void *ctx);
typedef filedb_status_t (*filedb_process_t) (FILE *stream, void *ctx);


filedb_status_t filedb_walk(const char *dbname,
                            filedb_visit_t visit,
                            filedb_process_t process,
                            void *ctx);

#endif
