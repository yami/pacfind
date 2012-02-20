#include <string.h>             /* we use gnu version of basename() */
#include <libgen.h>
#include <cdb.h>
#include <pcre.h>

#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#include <errno.h>

#include "logging.h"
#include "util.h"
#include "filedb.h"

#include "bindb.h"
#include "slist.h"

struct bindb_context {
    char *b_bindb_file;
    char *b_bindb_tmpfile;
    int   b_bindb_fd;
    struct cdb_make b_cdbm;
    
    pcre       *b_re;
    pcre_extra *b_ex;

    char *b_repository;
    char *b_package;
};

typedef enum match_status {
    M_Match,
    M_NoMatch,
    M_Error,
} match_status_t;

static void
bindb_context_set_package(struct bindb_context *bctx, const char *package)
{
    xfree(bctx->b_package);
    bctx->b_package = xstrdup(package);
}

static void
bindb_context_set_repository(struct bindb_context *bctx, const char *repo)
{
    xfree(bctx->b_repository);
    bctx->b_repository = xstrdup(repo);
}

void
bindb_context_delete(struct bindb_context *bctx)
{
    pcre_free_study(bctx->b_ex);
    pcre_free(bctx->b_re);

    if (bctx->b_bindb_fd != -1) {
        close(bctx->b_bindb_fd);
    }

    xfree(bctx->b_repository);
    xfree(bctx->b_package);

    xfree(bctx->b_bindb_tmpfile);
    xfree(bctx->b_bindb_file);
    xfree(bctx);
}

struct bindb_context *
bindb_context_new(const char *bindb_dir, const char *bindb_filename)
{
    char *template;
    char *file;
    
    if (asprintf(&template, "%s/%s.XXXXXX", bindb_dir, bindb_filename) == -1) {
        return NULL;
    }

    if (asprintf(&file, "%s/%s", bindb_dir, bindb_filename) == -1) {
        xfree(template);
        return NULL;
    }
    
    const char *pcre_error;
    int pcre_erroff;

    struct bindb_context *bctx = zmalloc(sizeof(struct bindb_context));
    
    /* initialize cdb */
    bctx->b_bindb_fd = -1;
    bctx->b_bindb_tmpfile = mktemp(template);
    bctx->b_bindb_file = file;
    
    if ((bctx->b_bindb_fd = open(bctx->b_bindb_tmpfile, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH)) == -1) {
        goto error;
    }

    cdb_make_start(&bctx->b_cdbm, bctx->b_bindb_fd);

    /* initialize pcre */
    bctx->b_re = pcre_compile("(?:^|/)s?bin/.+", 0, &pcre_error, &pcre_erroff, NULL);
    if (!bctx->b_re) {
        goto error;
    }

    bctx->b_ex = pcre_study(bctx->b_re, PCRE_STUDY_JIT_COMPILE, &pcre_error);

    return bctx;
    
error:
    bindb_context_delete(bctx);
    return NULL;
}

void
bindb_finalize(struct bindb_context *bctx)
{
    cdb_make_finish(&bctx->b_cdbm);
    
    if (bctx->b_bindb_fd != -1) {
        close(bctx->b_bindb_fd);
    }
    
    if (rename(bctx->b_bindb_tmpfile, bctx->b_bindb_file) == -1) {
        fprintf(stderr, "failed to rename %s to %s: %s\n",
                bctx->b_bindb_tmpfile, bctx->b_bindb_file, strerror(errno));
    }
}


static void
bindb_add(struct bindb_context *bctx, const char *bin_pathname)
{
    char *pname = xstrdup(bin_pathname);

    char *bname = xstrdup(basename(pname));

    size_t repo_len = strlen(bctx->b_repository) + 1;
    
    size_t value_len = repo_len + strlen(bctx->b_package) + 1;

    char *key = string_strip(bname);
    size_t key_len = strlen(key);

    char *value = xmalloc(value_len);

    memcpy(value, bctx->b_repository, repo_len);
    memcpy(value + repo_len, bctx->b_package, value_len - repo_len);
    
    cdb_make_add(&bctx->b_cdbm, key, key_len, value, value_len);

    /*
    DLOG(DEBUG, "bindb_add: key=%s repo=%s package=%s\n",
         key, value, value + strlen(value) + 1);
    */
    xfree(bname);
    xfree(pname);
    xfree(value);
}

static match_status_t
bindb_match(struct bindb_context *bctx, const char *str)
{
    pcre       *re = bctx->b_re;
    pcre_extra *ex = bctx->b_ex;
    int   error = 0;
    
    int match_result = pcre_exec(re, ex, str, strlen(str), 0, 0, NULL, 0);

    if (match_result >= 0) {
        return M_Match;
    } else if (match_result == -1) {
        return M_NoMatch;
    } else {
        fprintf(stderr, "bindb_match failed when matching %s with status %d\n",
                str, error);
        return M_Error;
    }
}

static filedb_status_t
visit_file(const char *filename, const struct stat *stat, void *ctx)
{
    struct bindb_context *bctx = ctx;
    
    if (!S_ISREG(stat->st_mode)) {
        return FD_Skip;
    }

    char *pname = xstrdup(filename);
    const char *bname = basename(pname);

    if (!string_eq(bname, "files")) {
        xfree(pname);
        return FD_Skip;
    }

    const char *dname = dirname(pname);

    bindb_context_set_package(bctx, dname);
    
    return FD_OK;
}


static filedb_status_t
process_file(FILE *stream, void *ctx)
{
    struct bindb_context *bctx = ctx;
    
    filedb_status_t status = FD_OK;
    ssize_t ret = 0;
    char *line = NULL;
    size_t n = 0;

    while ((ret = getline(&line, &n, stream)) != -1) {
        if (string_eq(line, "%FILES%")) {
            continue;
        }

        match_status_t m = bindb_match(bctx, line);

        switch (m) {
            case M_Match:
                bindb_add(bctx, line);
                break;
            case M_NoMatch:
                break;
            case M_Error:
                goto error;
                break;
        }
    }

    if (0) {
    error:
        status = FD_Stop;
    }
    
    xfree(line);
    
    return status;
}


void
bindb_update(struct bindb_context *bctx, const char *db_name, const char *repo)
{
    bindb_context_set_repository(bctx, repo);
    filedb_walk(db_name, visit_file, process_file, bctx);
}


struct bindb_qentry *
bindb_qentry_new(const char *repository, const char *package)
{
    struct bindb_qentry *ent = zmalloc(sizeof(*ent));
    ent->repository = xstrdup(repository);
    ent->package    = xstrdup(package);

    return ent;
}


struct slist *
bindb_query(const char *bindb_dir, const char *bindb_filename, const char *bin)
{
    struct cdb cdb;
    struct cdb_find cdbf;

    char *value = NULL;
    unsigned vlen, vpos;

    char *bindb_pathname;

    if (asprintf(&bindb_pathname, "%s/%s", bindb_dir, bindb_filename) == -1) {
        fprintf(stderr, "bindb_query: failed to format bindb pathname\n");
        return NULL;
    }
    
    int fd = open(bindb_pathname, O_RDONLY);
    
    if (fd == -1) {
        xfree(bindb_pathname);
        
        fprintf(stderr, "bindb_query: failed to open bindb\n");
        return NULL;
    }

    struct slist *qlist = slist_new();

    cdb_init(&cdb, fd);

    cdb_findinit(&cdbf, &cdb, bin, strlen(bin));

    while (cdb_findnext(&cdbf) > 0) {
        vpos = cdb_datapos(&cdb);
        vlen = cdb_datalen(&cdb);

        value = xrealloc(value, vlen);

        cdb_read(&cdb, value, vlen, vpos);

        struct bindb_qentry *qent =
            bindb_qentry_new(value, value + strlen(value) + 1);

        slist_append(qlist, slist_entry_new(qent));
    }

    close(fd);
    
    return qlist;
}
