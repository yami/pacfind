#include <unistd.h>

#include "bindb.h"
#include "util.h"
#include "slist.h"
#include "logging.h"
#include "query.h"
#include "update.h"
#include "conf.h"

#define OPT_UPDATE  (1<<0)
#define OPT_CONFIG  (1<<1)
#define OPT_QUERY   (1<<2)
#define OPT_LOGGING (1<<3)
#define OPT_HELP    (1<<4)

const char *Default_Config_Path = "/etc/pacfind.conf";

static void
usage()
{
    printf("Usage: \n"
           "  Query a binary\n"
           "    pacfind [-c <config file>] -q <binary_name>\n"
           "  Update the database\n"
           "    pacfind [-c <config file>] -u\n"
           "\n"
           "Default config file is /etc/pacfind.conf\n"
        );
}

int are_options_valid(int options, const char* argument)
{
    if ((options & OPT_HELP) && (options & ~OPT_HELP)) {
        return 0;
    }

    if ((options & OPT_UPDATE) && (options & OPT_QUERY)) {
        return 0;
    }
    
    return 1;
}


int main(int argc, char *argv[])
{
    int opt;

    int options = 0;
    
    char *config_file  = xstrdup(Default_Config_Path);
    char *query_string = NULL;
    char *logging_spec = NULL;

    char *argument = NULL;

    int ret = 0;
    
    while ( (opt = getopt(argc, argv, "uc:q:l:h")) != -1) {
        switch (opt) {
            case 'u':
                options |= OPT_UPDATE;
                break;
            case 'c':
                options |= OPT_CONFIG;
                config_file = xstrdup(optarg);
                break;
            case 'q':
                options |= OPT_QUERY;
                query_string = xstrdup(optarg);
                break;
            case 'l':
                options |= OPT_LOGGING;
                logging_spec = xstrdup(optarg);
                break;
            case 'h':
                options |= OPT_HELP;
                break;
            default:
                usage();
                return 1;
        } 
    }

    if (optind < argc) {
        argument = xstrdup(argv[optind]);
    }

    if (!are_options_valid(options, argument)) {
        usage();
        return 1;
    }

    struct config *config = read_config_file(config_file);

    if (options & OPT_LOGGING) {
        logging_config(logging_spec);
    }

    if (options & OPT_UPDATE) {
        ret = do_update(config);
    }

    if (options & OPT_QUERY) {
        ret = do_query(config, query_string);
    }

    xfree(config_file);
    xfree(query_string);
    xfree(logging_spec);

    config_delete(config);
    
    return ret;
}
