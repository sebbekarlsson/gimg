#include <stdio.h>
#include <stdlib.h>
#include <gimg/gimg.h>

int main(int argc, char* argv[]) {
  if (argc <= 1) {
    fprintf(stderr, "Please specify input file.\n");
  }

  GIMG gimg = {0};
  gimg_read_from_path(&gimg, argv[1]);


  printf("width: %d\n", gimg.width);
  printf("height: %d\n", gimg.height);
  printf("components: %d\n", gimg.components);

  return 0;
}
