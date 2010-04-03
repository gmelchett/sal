

#ifndef IMAGE_H
#define IMAGE_H
#include <Imlib2.h>

enum scale {
        NO_SCALE,
        RANDOM_RESCALE,
        SCALE,
        RELATIVE
};

Imlib_Image image_load_random(char *fname, int *sw, int *sh);
Imlib_Image image_load_scale(char *fname, int sw, int sh);
Imlib_Image image_load_relative(char *fname, int scale);
Imlib_Image image_load(char *fname);
void image_init(char *path);

#endif
