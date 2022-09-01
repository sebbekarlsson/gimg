#include <gimg/gimg.h>

#ifndef STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <gimg/stb_image.h>
#include <gimg/stb_image_write.h>
#endif
#include <png.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#include <webp/decode.h>

#include <jpeglib.h>
#include <jerror.h>

static unsigned int gimg_file_exists(const char *path) {
  return access(path, F_OK) == 0;
}

static uint8_t *gimg_get_file_bytes(const char *filepath, uint32_t *len) {
  FILE *fileptr;
  uint8_t *buffer;
  uint32_t filelen;

  fileptr = fopen(filepath, "rb");

  if (fileptr == 0) {
    printf("Could not open %s\n", filepath);
    return 0;
  }

  fseek(fileptr, 0, SEEK_END);
  filelen = ftell(fileptr);
  rewind(fileptr);

  buffer = (uint8_t *)malloc(filelen * sizeof(uint8_t));
  fread(buffer, filelen, 1, fileptr);
  fclose(fileptr);

  *len = filelen;

  if (buffer == 0) {
    printf("Did not read any data from %s\n", filepath);
  }

  return buffer;
}

static uint8_t *gimg_load_jpeg(const char *file_path, int *x, int *y,
                              uint64_t *data_size, int *channels,
                              GIMGColorType *type) {
  uint8_t *rowptr[1];
  uint8_t *jdata;
  struct jpeg_decompress_struct info;
  struct jpeg_error_mgr err;

  FILE *file = fopen(file_path, "rb");

  info.err = jpeg_std_error(&err);
  jpeg_create_decompress(&info);

  if (!file) {
    fprintf(stderr, "Error reading JPEG file %s!\n", file_path);
    return 0;
  }

  jpeg_stdio_src(&info, file);
  jpeg_read_header(&info, TRUE);

  jpeg_start_decompress(&info);

  *x = info.output_width;
  *y = info.output_height;
  *channels = info.num_components;
  *type = GIMG_COLOR_TYPE_RGB;
  if (*channels == 4) *type = GIMG_COLOR_TYPE_RGBA;

  *data_size = (*x) * (*y) * 3;

  jdata = (unsigned char *)malloc(*data_size);
  while (info.output_scanline < info.output_height)  // loop
  {
    rowptr[0] = (unsigned char *)jdata +  // secret to method
                3 * info.output_width * info.output_scanline;

    jpeg_read_scanlines(&info, rowptr, 1);
  }

  jpeg_finish_decompress(&info);
  jpeg_destroy_decompress(&info);
  fclose(file);

  return jdata;
}

static uint8_t *gimg_load_image_webp(const char *file_path, int *x, int *y,
                                    uint64_t *data_size, int *channels,
                                    GIMGColorType *type) {
  uint8_t *data_8 = 0;
  uint32_t siz = 0;
  uint8_t *data = gimg_get_file_bytes(file_path, &siz);
  if (siz == 0) {
    printf("%s data_size == 0\n", file_path);
    return 0;
  }
  data_8 = WebPDecodeRGBA(data, siz, x, y);
  if (data_8 == 0) {
    printf("Unable to load %s\n", file_path);
    return 0;
  }
  *data_size = siz;
  *type = GIMG_COLOR_TYPE_RGBA;
  return data_8;
}

int gimg_read_from_path(GIMG *image, const char *path) {
  if (gimg_file_exists(path) == 0) {
    //fprintf(stderr, "No such file %s\n", path);
    return 0;
  }
  int _components = 4;

  if (strstr(path, "png") != 0) {
    png_image img = {0};
    img.version = PNG_IMAGE_VERSION;

    if (!png_image_begin_read_from_file(&img, path))
      fprintf(stderr, "Could not read file `%s`: %s\n", path, img.message);

    // just to figure out which format to use
    /** {
       int x = 0;
       int y = 0;
       int comp = 0;
       int req_comp = 0;
       void* tmp = stbi_load(path, &x, &y, &comp, req_comp);
       stbi_image_free(tmp);

       img.format = comp >= 4 ? PNG_FORMAT_RGB : PNG_FORMAT_RGBA;
       }*/

    img.format = PNG_FORMAT_RGBA;

    uint32_t *img_pixels =
        (uint32_t *)malloc(sizeof(uint32_t) * img.width * img.height);
    if (img_pixels == NULL)
      fprintf(stderr, "Could not allocate memory for an img\n");

    if (!png_image_finish_read(&img, NULL, img_pixels, 0, NULL))
      fprintf(stderr, "libpng error: %s\n", img.message);

    image->components = 4;
    image->data_32 = img_pixels;
    image->width = img.width;
    image->height = img.height;
    image->specification_type = GIMG_SPECIFICATION_TYPE_PNG;

  } else if (strstr(path, "webp") != 0) {
    image->data_8 = gimg_load_image_webp(path, &image->width, &image->height,
                                        &image->size_bytes, &image->components,
                                        &image->color_type);
    image->specification_type = GIMG_SPECIFICATION_TYPE_WEBP;
  } else if (strstr(path, "jpg") != 0 || strstr(path, "jpeg") != 0) {
    uint64_t data_size = 0;
    unsigned int type = 0;
    image->data_8 = gimg_load_jpeg(path, &image->width, &image->height,
                                   &data_size, &_components, &type);

    image->components = _components;
    image->specification_type = GIMG_SPECIFICATION_TYPE_JPG;
    image->size_bytes = data_size;
  }


  else {
    if (strstr(path, ".bmp") != 0 || strstr(path, ".BMP") != 0) {
      image->components = 3;
      _components = 3;
    }


    image->data_8 =
        stbi_load(path, &image->width, &image->height, &image->components, 0);
    image->specification_type = GIMG_SPECIFICATION_TYPE_UNKNOWN;
  }

  if (image->data_8 == 0 && image->data_32 == 0 && image->data_16 == 0) {
    fprintf(stderr, "Unable to load %s\n", path);
    return 0;
  }

  if (image->width <= 0 || image->height <= 0) {
    fprintf(stderr, "Unable to load %s (invalid dimensions)\n", path);
    return 0;
  }

  image->uri = strdup(path);
  image->color_type_internal =
      _components >= 4 ? GIMG_COLOR_TYPE_RGBA : GIMG_COLOR_TYPE_RGB;
  image->color_type =
      _components >= 4 ? GIMG_COLOR_TYPE_RGBA : GIMG_COLOR_TYPE_RGB;

  return 1;
}

void gimg_free(GIMG *image, unsigned int completely) {
  if (image->data_8 != 0) {
    free(image->data_8);
    image->data_8 = 0;
  }

  if (image->data_16 != 0) {
    free(image->data_16);
    image->data_16 = 0;
  }

  if (image->data_32 != 0) {
    free(image->data_32);
    image->data_32 = 0;
  }

  if (image->uri != 0) {
    free(image->uri);
    image->uri = 0;
  }
  image->color_type = 0;
  image->width = 0;
  image->height = 0;
  image->color_type_internal = 0;
  image->components = 0;
  image->specification_type = 0;
  image->size_bytes = 0;

  if (completely) {
    free(image);
    image = 0;
  }
}
