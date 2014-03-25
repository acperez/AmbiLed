#ifndef COMMON_H
#define COMMON_H

#define clear(x) memset(&(x), 0, sizeof(x))

#define print(FMT, ARGS...) fprintf(stdout, FMT, ## ARGS)
#define error(FMT, ARGS...) fprintf(stderr, FMT, ## ARGS)

#endif
