#ifndef GIMG_SERIALIZE_H
#define GIMG_SERIALIZE_H
#include <gimg/gimg.h>
#include <stdio.h>

int gimg_serialize(GIMG gimg, FILE* fp);
int gimg_deserialize(GIMG* gimg, FILE* fp);

int gimg_serialize_to_path(GIMG gimg, const char* path);
int gimg_deserialize_from_path(GIMG* gimg, const char* path);

#endif
