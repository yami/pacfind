#include <stdlib.h>
#include <string.h>
#include <float.h>              /* DBL_EPSILON */
#include <math.h>
#include <ctype.h>

#include "util.h"

void *
xmalloc(size_t size)
{
    return hassert_value(malloc(size));
}

void *
zmalloc(size_t size)
{
    return hassert_value(calloc(1, size));
}

void *
xrealloc(void *ptr, size_t size)
{
    hassert(size);
    return hassert_value(realloc(ptr, size));
}

void
xfree(void *p)
{
    free(p);
}

char *
xstrdup(const char *str)
{
    return hassert_value(strdup(str));
}


int double_eq(double x, double y)
{
    return fabs(x - y) < DBL_EPSILON;
}

int string_eq(const char *x, const char *y)
{
    return strcmp(x, y) == 0;
}

char *string_strip(char *str)
{
    char *start, *end;
    
    for (start = str; *start; start++) {
        if (!isspace(*start)) {
            break;
        }
    }

    for (end = start; *end; end++) {
        if (isspace(*end)) {
            *end = '\0';
            break;
        }
    }

    return start;
}
