#include <gimg/cliparse.h>
#include <gimg/gimg.h>
#include <linux/limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <string.h>
#include <sys/stat.h>
#include <stdbool.h>
#include <limits.h>

typedef struct {
  const char* ext;
  const char* dirname;
  float scale;
} DownscaleArgs;

typedef void (*FileCallback)(const char* path, DownscaleArgs args);

static bool is_dir(const char *path) {
   struct stat statbuf;
   if (stat(path, &statbuf) != 0)
       return 0;
   return S_ISDIR(statbuf.st_mode);
}

static int listdir(const char *path, FileCallback callback, DownscaleArgs args) {
    struct dirent *entry;
    DIR *dp;

    dp = opendir(path);
    if (dp == NULL) {
        perror("opendir: Path does not exist or could not be read.");
        return -1;
    }

    while ((entry = readdir(dp))) {
      const char* name = entry->d_name;
      if (entry->d_type != DT_DIR && !(strstr(name, "small")) && (strstr(name, ".jpg") || strstr(name, ".jpeg") || strstr(name, ".png"))) {

	char fullpath[PATH_MAX];

	if (path[strlen(path)-1] != '/' && name[0] != '/') { 
	  sprintf(fullpath, "%s/%s", path, name);
	} else {
	  sprintf(fullpath, "%s%s", path, name);
	}
	callback(fullpath, args);
      }
    }

    closedir(dp);
    return 0;
}

static void downscale_callback(const char *path, DownscaleArgs args) {

  GIMG image = {0};
   if (!gimg_read_from_path(&image, path)) return;

  int64_t slen = strlen(path);
  char* dot = strrchr(path, '.');

  int64_t len = dot ? (dot - path) : strlen(path);
  char newname[PATH_MAX];
  memcpy(&newname[0], &path[0], len * sizeof(char));

  int64_t ext_len = slen - len;
  char ext[ext_len];
  memcpy(&ext[0], dot, ext_len);


  char newfull[PATH_MAX];
  sprintf(newfull, "%s_small%s", newname, args.ext ? args.ext : ext);

  if (gimg_downscale(image, args.scale, newfull)) {
    printf("Wrote `%s`\n", newfull);
  }
}

static int downscale(const char* ext, const char *dir, float scale) {
  listdir(dir, downscale_callback, (DownscaleArgs){ .dirname = dir, .scale = scale, .ext = ext });
  return 1;
}

int main(int argc, char *argv[]) {
  if (argc <= 1) {
    fprintf(stderr, "Please specify input file.\n");
  }

  if (
      cliparse_has_arg(argc, argv, "--downscale") &&
      cliparse_has_arg(argc, argv, "--dir")
  ) {
    const char* dir = cliparse_get_arg_string(argc, argv, "--dir");
    const char* ext = cliparse_get_arg_string(argc, argv, "--ext");
    float scale = cliparse_get_arg_float(argc, argv, "--downscale");

    if (!downscale(ext, dir, scale)) return 1;
    return 0;
  }

  return 0;
}
