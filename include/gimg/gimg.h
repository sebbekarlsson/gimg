#ifndef LIBGIMG_H
#define LIBGIMG_H
#include <stdint.h>
#include <stdbool.h>
#include <gimg/pixel.h>

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
  void* data;
  uint64_t size_bytes;
  int width;
  int height;
  int components;
  int stride;
  GIMGColorType color_type;
  GIMGColorType color_type_internal;
  GIMGSpecificationType specification_type;
} GIMG;

int gimg_read_from_path(GIMG* image, const char* path);

void gimg_free(GIMG* image, unsigned int completely);

bool gimg_validate(GIMG gimg);

int gimg_save(GIMG image, const char *path);

int gimg_make(GIMG* img, int width, int height);

int gimg_set_pixel(GIMG* img, int x, int y, GIMGPixel pixel);
int gimg_get_pixel(GIMG* gimg, int x, int y, GIMGPixel* out);
int gimg_get_pixel_rgb(GIMG* gimg, int x, int y, GIMGPixelRGB* out);
int gimg_get_average_pixel(GIMG* gimg, GIMGPixel* out);

int gimg_fill(GIMG* img, GIMGPixel pixel);








#endif
