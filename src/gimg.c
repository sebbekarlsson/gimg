#include <gimg/gimg.h>
#include <gimg/macros.h>
#include <gimg/serialize.h>
#include <mif/linear/vector4/all.h>

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
#include <stdio.h>

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
			       GIMGColorType *type, int* stride) {
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

  int comps = info.num_components;
  if (comps <= 0) comps = info.output_components;
  if (comps <= 0) comps = 3;

  *channels = comps;
  *type = GIMG_COLOR_TYPE_RGB;
  if (*channels == 4) *type = GIMG_COLOR_TYPE_RGBA;

  *data_size = (*x) * (*y) * comps;

  *stride = comps * info.output_width;


  uint8_t* jdata = (unsigned char *)calloc(*data_size, sizeof(uint8_t));
  while (info.output_scanline < info.output_height)  // loop
  {
    uint8_t* ptr = (unsigned char *)jdata +
      comps * info.output_width * info.output_scanline;


    // NOTE:
    // there is a bug where this function emits uninitialized bytes.
    // So that's why valgrind is complaining, and I can't do anything
    // about it since it's a third-party library.
    jpeg_read_scanlines(&info, &ptr, 1);
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
  image->data = 0;
  image->size_bytes = 0;
  image->stride = 0;
  image->width = 0;
  image->height = 0;
  image->components = 0;
  image->color_type = 0;
  image->color_type_internal = 0;
  image->uri = 0;
  
  if (gimg_file_exists(path) == 0) {
    //fprintf(stderr, "No such file %s\n", path);
    return 0;
  }
  int _components = 4;

  if (strstr(path, ".gimg") != 0) {
    return gimg_deserialize_from_path(image, path);
  }

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
    image->data = img_pixels;
    image->width = img.width;
    image->height = img.height;
    image->specification_type = GIMG_SPECIFICATION_TYPE_PNG;

  } else if (strstr(path, "webp") != 0) {
    image->data = gimg_load_image_webp(path, &image->width, &image->height,
                                        &image->size_bytes, &image->components,
                                        &image->color_type);
    image->specification_type = GIMG_SPECIFICATION_TYPE_WEBP;
  } else if (strstr(path, "jpg") != 0 || strstr(path, "jpeg") != 0) {
    uint64_t data_size = 0;
    unsigned int type = 0;
    image->data = gimg_load_jpeg(path, &image->width, &image->height,
				 &data_size, &_components, &type, &image->stride);

    image->components = _components;
    image->specification_type = GIMG_SPECIFICATION_TYPE_JPG;
    image->size_bytes = data_size;
  }


  else {
    if (strstr(path, ".bmp") != 0 || strstr(path, ".BMP") != 0) {
      image->components = 3;
      _components = 3;
    }


    image->data =
        stbi_load(path, &image->width, &image->height, &image->components, 0);
    image->specification_type = GIMG_SPECIFICATION_TYPE_UNKNOWN;
  }

  if (!gimg_validate(*image)) {
    GIMG_WARN("Unable to load %s\n", path);
    return 0;
  }

  image->uri = strdup(path);
  image->color_type_internal =
      _components >= 4 ? GIMG_COLOR_TYPE_RGBA : GIMG_COLOR_TYPE_RGB;
  image->color_type =
      _components >= 4 ? GIMG_COLOR_TYPE_RGBA : GIMG_COLOR_TYPE_RGB;

  if (image->stride <= 0) {
    int nrChannels = OR(image->components, 4);
    int stride = nrChannels * image->width;
    stride += (stride % nrChannels) ? (nrChannels - stride % nrChannels) : 0;
    image->stride = stride;
  }

  if (image->size_bytes <= 0) {
    int64_t nr_elements = image->width * image->height * image->components;
    image->size_bytes = (nr_elements * sizeof(uint8_t));
  }

  return 1;
}

void gimg_free(GIMG *image, unsigned int completely) {
  if (image->data != 0) {
    free(image->data);
    image->data = 0;
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

int gimg_save(GIMG image, const char *path, bool flip) {
  if (!gimg_validate(image)) {
    GIMG_WARN("Image is not valid.\n");
    return 0;
  }

  int components = image.components <= 0 ? 4 : image.components;

  if (image.stride <= 0) {
    int stride = components * image.width;
    stride += (stride % components) ? (components - stride % components) : 0;
    image.stride = stride;
  }

  if (flip) {
    stbi_flip_vertically_on_write(1);
  }

  if (strstr(path, ".jpg") != 0) {
    return stbi_write_jpg(path, image.width, image.height, image.components, image.data, 0);
  }

  return stbi_write_png(path, image.width, image.height, components, image.data,
                        image.stride);
}
/*

int gimg_save(GIMG image, const char *path, bool flip) {
  if (!gimg_validate(image)) {
    GIMG_WARN("Image is not valid.\n");
    return 0;
  }

  
  if (!image.stride) {
    int nrChannels = OR(image.components, 4);
    int stride = nrChannels * image.width;
    stride += (stride % nrChannels) ? (nrChannels - stride % nrChannels) : 0;
    image.stride = stride;
  }

  if (flip) {
    stbi_flip_vertically_on_write(1);
  }

  if (strstr(path, ".jpg") != 0) {
    return stbi_write_jpg(path, image.width, image.height, image.components, image.data, 0);
  }

  return stbi_write_png(path, image.width, image.height, 4, image.data,
                        image.stride);
			}*/


bool gimg_validate(GIMG gimg) {
  if (gimg.width <= 0 || gimg.height <= 0) return false;
  if (gimg.data == 0) return false;
  return true;
}

int gimg_make(GIMG* img, int width, int height, int components) {
  if (width <= 0 || height <= 0) return 0;

  img->width = width;
  img->height = height;
  img->components = components;

  int64_t nr_elements = img->width * img->height * img->components;
  img->size_bytes = (nr_elements * sizeof(uint32_t));
  img->data = (uint32_t*)calloc(nr_elements, sizeof(uint32_t));

  return img->data != 0;
}

Vector4 gimg_get_pixel_vec4(GIMG* img, int x, int y) {
  if (!img) return VEC41(0);
  if (!img->data) return VEC41(0);
  if (!gimg_validate(*img)) return VEC41(0);

  Vector4 color = VEC41(0.0f);


  if (img->components <= 3) {
      GIMGPixelRGB pixel = {0};
      gimg_get_pixel_rgb(img, x, y, &pixel);

      color.x = pixel.r;
      color.y = pixel.g;
      color.z = pixel.b;
      color.w = 255.0f;
  } else {
      GIMGPixel pixel = {0};
      gimg_get_pixel(img, x, y, &pixel);

      color.x = pixel.r;
      color.y = pixel.g;
      color.z = pixel.b;
      color.w = pixel.a;
  }

  return color;
}

int gimg_get_pixels_as_vec4(GIMG* img, Vector4Buffer* pixels) {
  if (!img || !pixels) return 0;
  if (!img->data) return 0;
  if (!gimg_validate(*img)) return 0;

  if (!pixels->initialized) {
    mif_Vector4_buffer_init(pixels, (MifBufferConfig){ .capacity = img->width * img->height * img->components * 4 * 2 });
  }


  for (int x = 0; x < img->width; x++) {
    for (int y = 0; y < img->height; y++) {
      mif_Vector4_buffer_push(pixels, gimg_get_pixel_vec4(img, x, y));
    }
  }

  return pixels->items != 0 && pixels->length > 0;
}

int gimg_set_pixel(GIMG* img, int x, int y, GIMGPixel pixel) {
  if (!img) return 0;
  if (!img->data) return 0;
  if (!gimg_validate(*img)) return 0;


  x = x % img->width;
  y = y % img->height;
  int max_idx = (img->width * img->height);
  int idx = (x + img->width * y) % max_idx;
  GIMGPixel* color = &(((GIMGPixel*)img->data)[idx]);

  *color = pixel;

  return 1;
}

int gimg_set_pixel_vec4(GIMG* img, int x, int y, Vector4 pix) {
  if (!img) return 0;
  if (!img->data) return 0;
  if (!gimg_validate(*img)) return 0;

  if (img->components <= 3) {
    x = x % img->width;
    y = y % img->height;
    int max_idx = (img->width * img->height);
    int idx = (x + img->width * y) % max_idx;
    GIMGPixelRGB* color = &(((GIMGPixelRGB*)img->data)[idx]);
    GIMGPixelRGB pixel = (GIMGPixelRGB){ .r = (uint8_t)pix.x, .g = (uint8_t)pix.y, .b = (uint8_t)pix.z };
    *color = pixel;
  } else {
    x = x % img->width;
    y = y % img->height;
    int max_idx = (img->width * img->height);
    int idx = (x + img->width * y) % max_idx;
    GIMGPixel* color = &(((GIMGPixel*)img->data)[idx]);
    GIMGPixel pixel = (GIMGPixel){ .r = (uint8_t)pix.x, .g = (uint8_t)pix.y, .b = (uint8_t)pix.z, .a = (uint8_t)pix.w };
    *color = pixel;
  }

  return 1;
}

int gimg_get_pixel(GIMG* img, int x, int y, GIMGPixel* out) {
  if (!img) return 0;
  if (!out) return 0;
  if (!img->data) return 0;
  if (!gimg_validate(*img)) return 0;


  x = x % img->width;
  y = y % img->height;
  int max_idx = (img->width * img->height);
  int idx = (x + img->width * y) % max_idx;
  *out = (((GIMGPixel*)img->data)[idx]);

  return 1;
}

uint32_t gimg_get_pixel_uint32(GIMG* img, int x, int y) {
  if (!img) return 0;
  if (!img->data) return 0;
  if (!gimg_validate(*img)) return 0;
  x = x % img->width;
  y = y % img->height;
  int max_idx = (img->width * img->height);
  int idx = (x + img->width * y) % max_idx;
  return ((uint32_t*)img->data)[idx];
}

int gimg_get_pixel_rgb(GIMG* img, int x, int y, GIMGPixelRGB* out) {
  if (!img) return 0;
  if (!out) return 0;
  if (!img->data) return 0;
  if (!gimg_validate(*img)) return 0;


  x = x % img->width;
  y = y % img->height;
  int max_idx = (img->width * img->height);
  int idx = (x + img->width * y) % max_idx;
  *out = (((GIMGPixelRGB*)img->data)[idx]);

  return 1;
}

int gimg_get_average_pixel(GIMG* img, GIMGPixel* out) {
  if (!img) return 0;
  if (!out) return 0;
  if (!img->data) return 0;
  if (!gimg_validate(*img)) return 0;


  float r = 0.0f;
  float g = 0.0f;
  float b = 0.0f;
  float a = 0.0f;
  float count = 0.0f;

  for (int x = 0; x < img->width; x++) {
    for (int y = 0; y < img->height; y++) {

      if (img->components <= 3) {
        GIMGPixelRGB pixel = {0};
        gimg_get_pixel_rgb(img, x, y, &pixel);

        r += pixel.r;
        g += pixel.g;
        b += pixel.b;
        a += 255.0f;
      } else {
        GIMGPixel pixel = {0};
        gimg_get_pixel(img, x, y, &pixel);

        r += pixel.r;
        g += pixel.g;
        b += pixel.b;
        a += 255.0f;
      }

      count += 1.0f;
    }
  }

  if (count <= 0.0f) return 0;

  r /= count;
  g /= count;
  b /= count;
  a /= count;


  out->r = r;
  out->g = g;
  out->b = b;
  out->a = a;

  return 1;
}

int gimg_fill(GIMG* img, GIMGPixel pixel) {
  if (!img) return 0;
  if (!gimg_validate(*img)) return 0;

  for (int x = 0; x < img->width; x++) {
    for (int y = 0; y < img->height; y++) {
      gimg_set_pixel(img, x, y, pixel);
    }
  }

  return 1;
}


int gimg_downscale(GIMG image, float scale, bool keep_aspect_ratio, const char *out_path) {
  if (!gimg_validate(image)) return 0;
  if (scale <= 0.0f) return 0;
  if (!out_path) return 0;


  int w = image.width;
  int h = image.height;

  int dest_w = 0;
  int dest_h = 0;

  if (keep_aspect_ratio) {
    dest_w = (int)roundf(w * scale);
    dest_h = (int)roundf(h * scale);
  } else {
    int s = MIN(w, h);
    dest_w = (int)roundf(s * scale);
    dest_h = (int)roundf(s * scale);
  }
  if (dest_w <= 0 || dest_h <= 0) {
    fprintf(stderr, "Unable to downscale to `%dx%d`.\n", dest_w, dest_h);
  }

  GIMG new_image;
  gimg_make(&new_image, dest_w, dest_h, 3);


  for (int x = 0; x < w; x++) {
    for (int y = 0; y < h; y++) {

      float u = (float)x / (float)w;
      float v = (float)y / (float)h;
      
      Vector4 px = gimg_get_pixel_vec4(&image, x, y);
      gimg_set_pixel_vec4(&new_image, u * dest_w, v * dest_h, px);
    }
  }

  return gimg_save(new_image, out_path, false);
}
