#include "ttemp.h"
#include "util.h"

#include <ctype.h>
#include <string.h>

static const char *
ttemp_nv_lookup(struct ttemp_nv *nv, const char *name)
{
    while (nv->name) {
        sassert(nv->value);

        if (string_eq(nv->name, name)) {
            return nv->value;
        }

        nv++;
    }

    return NULL;
}

static char *
ttemp_parse_variable(const char *template, const char ** endp )
{
    const char *t = template;
    const char *vbeg = NULL;
    const char *vend = NULL;

    char *var  = NULL;
    
    int embraced = 0;
    
    sassert(*t == '$');

    *endp = NULL;
    
    t++;                        /* skip $ */

    if (*t == '{') {
        t++;                    /* skip { */
        embraced = 1;
    }
    
    for (vbeg = t; *t; t++) {
        if (!isalnum(*t) && *t != '_') {
            if (embraced && *t != '}') {
                return NULL;
            }
            break;
        }
    }

    vend = t;

    if (vend == vbeg) {
        return NULL;
    }

    var = zmalloc(vend - vbeg + 1);
    memcpy(var, vbeg, vend - vbeg);
    
    *endp = vend;

    return var;
}

const char *
ttemp_fill(const char *template, struct ttemp_nv *nv)
{
    const char *start = template;
    const char *t = template;

    char *filled = zmalloc(1);
    size_t len    = 1;
    size_t nadd   = 0;

    size_t plen   = 0;

    const char *variable;
    const char *value;

    const char *next;
    
    while (*t) {
        if (*t == '$') {
            variable = ttemp_parse_variable(t, &next);

            if (!variable) {
                goto error;
            }

            value = ttemp_nv_lookup(nv, variable);

            if (!value) {
                goto error;
            }

            plen = t - start;
            
            nadd = plen + strlen(value);
            
            filled = xrealloc(filled, nadd + len);

            memcpy(filled + len - 1, start, plen);
            strcpy(filled + len - 1 + plen,  value);

            len += nadd;
            t = start = next;
        } else {
            t++;
        }
    }


    nadd = plen = t - start;

    if (nadd) {
        filled = xrealloc(filled, nadd + len);
        memcpy(filled + len - 1, start, plen);
    }

    return filled;

error:
    xfree(filled);
    return NULL;
}
