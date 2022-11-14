#include <stdio.h>
#include <stdlib.h>
#include <gimg/gimg.h>

int main(int argc, char* argv[]) {
  //if (argc <= 1) {
   // fprintf(stderr, "Please specify input file.\n");
 // }

  GIMG img = {0};
  gimg_make(&img, 640, 480);

  gimg_fill(&img, (GIMGPixel){ 0.0f, 0.0f, 0.0f, 255.0f });

  if (!gimg_set_pixel(&img, 640/2, 480/2, (GIMGPixel){ 255.0f, 0.0f, 0.0f, 255.0f })) {
    printf("Could not set pixel!\n");
  }

  gimg_save(img, "image.png");

//  GIMG gimg = {0};
 // gimg_read_from_path(&gimg, argv[1]);


 //k printf("width: %d\n", gimg.width);
  //printf("height: %d\n", gimg.height);
 // printf("components: %d\n", gimg.components);

  return 0;
}
