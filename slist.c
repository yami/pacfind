#include "slist.h"

void
slist_delete(struct slist *slist)
{
    struct slist_entry *next;
    
    for (struct slist_entry *ent = slist_head(slist);
         ent;
         ent = next) {
        next = slist_entry_next(ent);
        
        xfree(ent->data);
        xfree(ent);
    }

    xfree(slist);
}
