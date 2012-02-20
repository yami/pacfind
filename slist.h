#ifndef PACFIND_SLIST_H
#define PACFIND_SLIST_H

#include "util.h"

struct slist_entry;

struct slist {
    struct slist_entry *head;
    struct slist_entry *tail;
};

struct slist_entry {
    void *data;
    struct slist_entry *next;
};


static inline struct slist *
slist_new()
{
    return zmalloc(sizeof(struct slist));
}

static inline struct slist_entry *
slist_head(struct slist *slist)
{
    return slist->head;
}

static inline struct slist_entry *
slist_tail(struct slist *slist)
{
    return slist->tail;
}

static inline int
slist_empty(struct slist *slist)
{
    return !slist->head && !slist->tail;
}

static inline void
slist_append(struct slist *slist, struct slist_entry *ent)
{
    if (slist_empty(slist)) {
        slist->head = ent;
        slist->tail = ent;
    } else {
        slist->tail->next = ent;
        slist->tail = ent;
    }
}

void
slist_delete(struct slist *slist);

static inline struct slist_entry *
slist_entry_next(struct slist_entry *ent)
{
    return ent->next;
}

static inline void *
slist_entry_data(struct slist_entry *ent)
{
    return ent->data;
}

static inline void *
slist_entry_new(void *data)
{
    struct slist_entry *ent = zmalloc(sizeof(*ent));

    ent->data = data;

    return ent;
}

#endif  /* PACFIND_SLIST_H */
