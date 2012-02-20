#include <archive.h>
#include <archive_entry.h>
#include "util.h"

#include "filedb.h"


static cookie_io_functions_t archive_stream_funcs = {
    .read = (cookie_read_function_t *) archive_read_data,
};

FILE *open_archive_stream(struct archive* a)
{
    return fopencookie(a, "r", archive_stream_funcs);
}

filedb_status_t
filedb_walk(const char *dbname,
            filedb_visit_t visit,
            filedb_process_t process,
            void *ctx)
{
    filedb_status_t s = FD_OK;
    struct archive *a;
    struct archive_entry *ent;

    a = archive_read_new();
    archive_read_support_compression_all(a);
    archive_read_support_format_all(a);
    archive_read_open_filename(a, dbname, 10240);

    while (archive_read_next_header(a, &ent) == ARCHIVE_OK) {
        const char *pathname = archive_entry_pathname(ent);
        const struct stat *st = archive_entry_stat(ent);

        s = visit(pathname, st, ctx);

        if (s == FD_OK) {
            /* do nothing */
        } else if (s == FD_Skip) {
            archive_read_data_skip(a);
            s = FD_OK;
            continue;
        } else if (s == FD_Stop) {
            break;
        } else {
        }

        FILE *stream = open_archive_stream(a);
        if (!stream) {
            goto error;
        }

        s = process(stream, ctx);

        fclose(stream);
        
        if (s == FD_OK) {
            /* do nothing */
        } else if (s == FD_Skip) {
            s = FD_OK;
            continue;
        } else if (s == FD_Stop) {
            break;
        }
    }
    
error:
    archive_read_finish(a);
    return s;
}


#ifdef FILEDB_TEST

filedb_status_t my_visit(const char *filename, const struct stat *stat, void *ctx)
{
    printf("%s\n", filename);
    
    return FD_Skip;
}

filedb_status_t my_process(FILE *stream, void *ctx)
{
    return FD_OK;
}



int main()
{
    filedb_walk("core.files.tar.gz", my_visit, my_process, NULL);
    return 1;
}

#endif  /* FILEDB_TEST */
