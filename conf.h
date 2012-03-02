#ifndef PACFIND_CONF_H
#define PACFIND_CONF_H

struct config {
    char  *update_server;
    char **repositories;
    char  *bindb_dir;
    char  *bindb_file;
};

struct config *read_config_file(const char *path);
struct config *config_new();
void config_delete(struct config *config);

#endif
