#ifndef PACFIND_LOGGING_H
#define PACFIND_LOGGING_H

#define DLOG(LEVEL, FMT, ARGS...) fprintf(stderr, FMT, ARGS)

static inline void
logging_config(const char *spec) {}

#endif  /* PACFIND_LOGGING_H */
