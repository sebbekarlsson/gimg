#ifndef GIMG_MACROS_H
#define GIMG_MACROS_H
#ifndef OR
#define OR(a, b) (a ? a : b)
#endif

#define GIMG_WARN(...) { fprintf(stderr, "(GIMG): (%s) Warning:\n", __func__);fprintf(stderr, __VA_ARGS__); }

#endif
