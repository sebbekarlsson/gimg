#ifndef LIBGIMG_H
#define LIBGIMG_H
#include <stdint.h>

typedef enum {
  GIMG_COLOR_TYPE_RGB,
  GIMG_COLOR_TYPE_RGBA
} GIMGColorType;

typedef enum {
  GIMG_SPECIFICATION_TYPE_UNKNOWN,
  GIMG_SPECIFICATION_TYPE_JPG,
  GIMG_SPECIFICATION_TYPE_PNG,
  GIMG_SPECIFICATION_TYPE_WEBP,
  GIMG_SPECIFICATION_TYPE_BMP
} GIMGSpecificationType;

typedef struct {
  char* uri;
  uint8_t* data_8;
  uint32_t* data_32;
  uint16_t* data_16;
  uint64_t size_bytes;
  int width;
  int height;
  int components;
  GIMGColorType color_type;
  GIMGColorType color_type_internal;
  GIMGSpecificationType specification_type;
} GIMG;

int gimg_read_from_path(GIMG* image, const char* path);

void gimg_free(GIMG* image, unsigned int completely);


#endif
