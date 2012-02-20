#include "bindb.h"
#include "slist.h"

#include <stdio.h>

int do_query(const char *config_file, const char *bin)
{
    const char *bindb_dir = "/tmp/pacfind";
    const char *bindb_file = "bindb";

    struct slist *qlist = bindb_query(bindb_dir, bindb_file, bin);

    if (!qlist) {
        fprintf(stderr, "do_query: failed to query %s\n", bin);
        return 1;
    }

    if (slist_empty(qlist)) {
        fprintf(stderr, "do_query: %s not found\n", bin);
        return 0;
    }
    
    for (struct slist_entry *ent = slist_head(qlist);
         ent;
         ent = slist_entry_next(ent)) {
        struct bindb_qentry *qent = (struct bindb_qentry *) slist_entry_data(ent);

        printf("%s is found in repository %s as package %s\n",
               bin, qent->repository, qent->package);
    }

    slist_delete(qlist);

    return 0;
}
