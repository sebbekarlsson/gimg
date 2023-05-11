#include <gimg/gimg.h>
#include <gimg/serialize.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

int gimg_serialize(GIMG gimg, FILE *fp) {
  if (!fp) return 0;
  if (gimg.data == 0 || gimg.size_bytes <= 0) return 0;

  uint8_t* data = (uint8_t*)gimg.data;
  int divisor = 1;//gimg.components <= 0 ? 1 : gimg.components;

  fwrite(&gimg.color_type, sizeof(GIMGColorType), 1, fp);
  fwrite(&gimg.color_type_internal, sizeof(GIMGColorType), 1, fp);
  fwrite(&gimg.specification_type, sizeof(GIMGSpecificationType), 1, fp);
  fwrite(&gimg.components, sizeof(int), 1, fp);
  fwrite(&gimg.size_bytes, sizeof(uint64_t), 1, fp);
  fwrite(&data[0], gimg.size_bytes / divisor, 1, fp);
  fwrite(&gimg.width, sizeof(int), 1, fp);
  fwrite(&gimg.height, sizeof(int), 1, fp);
  fwrite(&gimg.stride, sizeof(int), 1, fp);

  size_t uri_length = gimg.uri ? strlen(gimg.uri) : 0;
  fwrite(&uri_length, sizeof(size_t), 1, fp);

  if (uri_length > 0 && gimg.uri != 0) {
    fwrite(&gimg.uri[0], sizeof(char), uri_length, fp);
  }
  
  return 1;
}

int gimg_deserialize(GIMG* gimg, FILE *fp) {
  if (!fp) return 0;


  fread(&gimg->color_type, sizeof(GIMGColorType), 1, fp);
  fread(&gimg->color_type_internal, sizeof(GIMGColorType), 1, fp);
  fread(&gimg->specification_type, sizeof(GIMGSpecificationType), 1, fp);
  fread(&gimg->components, sizeof(int), 1, fp);

  int divisor = 1;//gimg->components <= 0 ? 1 : gimg->components;

  fread(&gimg->size_bytes, sizeof(uint64_t), 1, fp);

  if (!gimg->size_bytes) return 0;

  uint8_t* data = (uint8_t*)calloc(gimg->size_bytes, sizeof(uint8_t));

  if (!data) return 0;
  
  fread(&data[0], gimg->size_bytes / divisor, 1, fp);
  gimg->data = data;
  
  fread(&gimg->width, sizeof(int), 1, fp);
  fread(&gimg->height, sizeof(int), 1, fp);
  fread(&gimg->stride, sizeof(int), 1, fp);

  size_t uri_length = 0;
  fread(&uri_length, sizeof(size_t), 1, fp);

  if (uri_length > 0 && gimg->uri != 0) {
    fread(&gimg->uri[0], sizeof(char), uri_length, fp);
  }

  return 1;
}

int gimg_serialize_to_path(GIMG gimg, const char *path) {
  if (!path) return 0;

  FILE* fp = fopen(path, "wb+");
  if (!fp) return 0;

  int status = gimg_serialize(gimg, fp);
  
  fclose(fp);

  return status;
}
int gimg_deserialize_from_path(GIMG *gimg, const char *path) {
  if (!path) return 0;

  FILE* fp = fopen(path, "rb");
  if (!fp) return 0;

  int status = gimg_deserialize(gimg, fp);
  
  fclose(fp);

  return status;
  
}
