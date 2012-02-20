#ifndef PACFIND_BINDB_H
#define PACFIND_BINDB_H

struct slist;

struct bindb_qentry {
    char *repository;
    char *package;
};

struct bindb_context *
bindb_context_new(const char *bindb_dir, const char *bindb_filename);

void
bindb_finalize(struct bindb_context *bctx);

void
bindb_context_delete(struct bindb_context *bctx);

struct slist *
bindb_query(const char *bindb_dir, const char *bindb_filename, const char *bin);

void
bindb_update(struct bindb_context *bctx, const char *db_name, const char *repo);

#endif  /* PACFIND_BINDB_H */
