#include <sys/utsname.h>
#include <string.h>
#include <errno.h>
#include <stdio.h>

#include "ttemp.h"
#include "bindb.h"
#include "util.h"
#include "download.h"
#include "conf.h"

int do_update(struct config *config)
{
    struct utsname sysinfo;

    if (uname(&sysinfo) == -1) {
        fprintf(stderr, "do_update: failed to call uname: %s\n",
                strerror(errno));
        return 1;
    }

    const char *arch = sysinfo.machine;
    
    struct ttemp_nv nv[] = {
        { "arch", arch },
        { "repo", NULL },
        { NULL,   NULL }
    };

    struct bindb_context *bctx =
        bindb_context_new(config->bindb_dir, config->bindb_file);
    
    char *url = NULL;
    char *file = NULL;

    const char *repo;
    
    for (int irepo = 0; config->repositories[irepo]; irepo++) {
        repo = nv[1].value = config->repositories[irepo];

        const char *root_url = ttemp_fill(config->update_server, nv);

        if (asprintf(&file, "%s/%s.files.tar.gz", config->bindb_dir, repo) == -1) {
            return 1;
        }
        
        if (asprintf(&url, "%s/%s.files.tar.gz", root_url, repo) == -1) {
            xfree(file);
            return 1;
        }

        download(url, file);

        bindb_update(bctx, file, repo);

        xfree(file);
        xfree(url);
    }

    bindb_finalize(bctx);
    bindb_context_delete(bctx);
    
    return 0;
}
