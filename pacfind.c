#include <unistd.h>

#include "bindb.h"
#include "util.h"
#include "slist.h"
#include "logging.h"
#include "query.h"
#include "update.h"

#define OPT_UPDATE  (1<<0)
#define OPT_CONFIG  (1<<1)
#define OPT_PCONFIG (1<<2)
#define OPT_QUERY   (1<<3)
#define OPT_LOGGING (1<<4)
#define OPT_HELP    (1<<5)

static void
usage()
{
}

int are_options_valid(int options, const char* argument)
{
    if ((options & OPT_HELP) && (options & ~OPT_HELP)) {
        return 0;
    }

    if ((options & OPT_UPDATE) && (options & OPT_QUERY)) {
        return 0;
    }

    if ((options & (OPT_CONFIG | OPT_PCONFIG)) && !(options & OPT_UPDATE)) {
        return 0;
    }

    return 1;
}


int main(int argc, char *argv[])
{
    int opt;

    int options = 0;
    
    char *config_file  = NULL;
    char *pconfig_file = NULL;
    char *query_string = NULL;
    char *logging_spec = NULL;

    char *argument = NULL;

    int ret = 0;
    
    while ( (opt = getopt(argc, argv, "uc:p:q:l:h")) != -1) {
        switch (opt) {
            case 'u':
                options |= OPT_UPDATE;
                break;
            case 'c':
                options |= OPT_CONFIG;
                config_file = xstrdup(optarg);
                break;
            case 'p':
                options |= OPT_PCONFIG;
                pconfig_file = xstrdup(optarg);
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

    if (options & OPT_LOGGING) {
        logging_config(logging_spec);
    }

    if (options & OPT_UPDATE) {
        ret = do_update(config_file, pconfig_file);
    }

    if (options & OPT_QUERY) {
        ret = do_query(config_file, query_string);
    }

    xfree(config_file);
    xfree(pconfig_file);
    xfree(query_string);
    xfree(logging_spec);

    return ret;
}
