#ifndef PACFIND_UTIL_H
#define PACFIND_UTIL_H

#include <stddef.h>
#include <stdio.h>

#define die(FMT, ARGS...) do {                  \
        fprintf(stderr, "DIE at %s:%d @ " FMT,  \
                __FILE__, __LINE__, ##ARGS);    \
        abort();                                \
    } while (0)

#ifdef SOFT_ASSERT
#define sassert(E) do {                             \
        if (!(E)) {                                 \
            die(#E);                                \
        }                                           \
    } while (0)
#else
#define sassert(E) 
#endif


#define hassert(E) do {                         \
        if (!(E)) {                             \
            die(#E);                            \
        }                                       \
    } while (0)

#define hassert_value(V) ({                     \
    typeof(V) _v = (V);                         \
    if (!_v) {                                  \
              die(#V);                          \
    }                                           \
    _v; })

#define sassert_value(V) ({                     \
    typeof(V) _v = (V);                         \
    if (!_v) {                                  \
              die(#V);                          \
    }                                           \
    _v; })

void *xmalloc(size_t size);
void *zmalloc(size_t size);
void *xrealloc(void *ptr, size_t size);
void  xfree(void *p);

char *xstrdup(const char *str);

int double_eq(double x, double y);

int string_eq(const char *x, const char *y);

char *string_strip(char *str);

#endif
