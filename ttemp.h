#ifndef PACFIND_TTEMP_H
#define PACFIND_TTEMP_H

struct ttemp_nv {
    const char *name;
    const char *value;
};

const char *ttemp_fill(const char *template, struct ttemp_nv *nv);

#endif  /* PACFIND_TTEMP_H */
