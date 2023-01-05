#ifndef CLIPARSE_H
#define CLIPARSE_H
#include <stdbool.h>
#include <string.h>
#include <stddef.h>
#include <ctype.h>
#include <stdlib.h>

#ifndef MIN
#define MIN(a, b) ((a) < (b) ? (a) : (b))
#endif

#ifndef MAX
#define MAX(a, b) ((a) > (b) ? (a) : (b))
#endif

bool cliparse_has_arg(int argc, char* argv[], const char* key) {
  if (argc <= 1) return false;

  for (int i = 0; i < argc; i++)
    if (strcmp(argv[i], key) == 0) return true;

  return false;
}

const char* cliparse_get_arg_string(int argc, char* argv[], const char* key) {
  for (int i = 0; i < argc; i++) {
    if (strcmp(argv[i], key) == 0) {
      const char* value = argv[MIN(i+1, argc-1)];
      return value;
    }
  }

  return 0;
}

float cliparse_get_arg_float(int argc, char* argv[], const char* key) {
  const char* strval = cliparse_get_arg_string(argc, argv, key);
  if (!strval) return 0;
  return atof(strval);
}

#endif
