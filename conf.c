#include <jansson.h>
#include <stdio.h>

#include "conf.h"
#include "util.h"

typedef enum j_status {
    J_OK,
    J_NotFound,
    J_BadType,
    J_EmptyArray,
    J_Max,
} j_status_t;

static const char *J_Status_String [] = {
    "OK",
    "NotFound",
    "BadType",
    "EmptyArray",
};

static const char *
j_status_string(j_status_t s)
{
    sassert(s < J_Max);
    return J_Status_String[s];
}

static j_status_t
j_object_get_string(json_t *obj, const char *member, char **value)
{
    *value = NULL;

    json_t *jvalue = json_object_get(obj, member);

    if (!jvalue) {
        return J_NotFound;
    }

    if (!json_is_string(jvalue)) {
        return J_BadType;
    }

    *value = xstrdup(json_string_value(jvalue));

    return J_OK;
}

static j_status_t
j_object_get_string_array0(json_t *obj, const char *member, char*** value)
{
    *value = NULL;
    
    json_t *jarray = json_object_get(obj, member);

    if (!jarray) {
        return J_NotFound;
    }

    if (!json_is_array(jarray)) {
        return J_BadType;
    }

    size_t n = json_array_size(jarray) + 1;

    if (n == 1) {
        return J_EmptyArray;
    }

    *value = zmalloc(sizeof(char*) * n);

    j_status_t s = J_OK;
    for (size_t i = 0; i < n - 1; i++) {
        json_t *jstr = json_array_get(jarray, i);
        if (!json_is_string(jstr)) {
            s = J_BadType;
            break;
        }

        (*value)[i] = xstrdup(json_string_value(jstr));
    }

    if (s == J_OK) {
        return s;
    }

    for (size_t i = 0; i < n - 1; i++) {
        xfree((*value)[i]);
    }

    xfree(*value);
    *value = NULL;

    return s;
}

struct config *
read_config_file(const char *path)
{
    json_error_t jerror;
    struct config *config = config_new();
    json_t        *root = json_load_file(path, 0, &jerror);

    j_status_t s = J_OK;
    
    if (!root) {
        fprintf(stderr, "failed to read config file %s with error at line %d: %s\n",
                path, jerror.line, jerror.text);
        goto error;
    }

    if ((s = j_object_get_string(root, "update_server", &config->update_server)) != J_OK) {
        fprintf(stderr, "failed to parse config file for update_server: %s\n",
                j_status_string(s));
        goto error;
    }

    if ((s = j_object_get_string_array0(root, "repositories", &config->repositories)) != J_OK) {
        fprintf(stderr, "failed to parse config file for repositories: %s\n",
                j_status_string(s));
        goto error;
    }

    if ((s = j_object_get_string(root, "bindb_dir", &config->bindb_dir)) != J_OK) {
        fprintf(stderr, "failed to parse config file for bindb_dir: %s\n",
                j_status_string(s));
        goto error;
    }

    if ((s = j_object_get_string(root, "bindb_file", &config->bindb_file)) != J_OK) {
        fprintf(stderr, "failed to parse config file for bindb_file: %s\n",
                j_status_string(s));
        goto error;
    }

    return config;
    
error:    
    config_delete(config);

    return NULL;
}

struct config *
config_new()
{
    return zmalloc(sizeof(struct config));
}

void config_delete(struct config *config)
{
    if (!config)
        return;
    
    xfree(config->update_server);

    if (config->repositories) {
        for (char **repo = config->repositories; *repo; repo++) {
            xfree(*repo);
        }
        xfree(config->repositories);
    }
    
    xfree(config->bindb_dir);
    xfree(config->bindb_file);
    
}
