#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>

#include "sal.h"
#include "config.h"
#include "image.h"

#define MAX_PATH 1024

static char image_path_loaded[MAX_PATH];
static char image_path_default[] = IMAGE_PATH;
static char *image_path = image_path_default;

static Imlib_Image image_load_core(char *fname, int *sw, int *sh, enum scale scale, bool ok_flip, int relative)
{
        int size;
        Imlib_Image orig, scaled;
        char path[MAX_PATH];

        snprintf(path, sizeof(path), "%s/%s", image_path, fname);

        orig = imlib_load_image(path);

        imlib_context_set_image(orig);

        if(random() % 2 && ok_flip)
                imlib_image_flip_horizontal();

        if(scale != NO_SCALE) {

                if(scale == RANDOM_RESCALE)
                        size = (random() % 100) + 50;
                if(scale == RELATIVE)
                        size = relative;

                if(scale == RELATIVE || scale == RANDOM_RESCALE) {
                        *sw = imlib_image_get_width() * size / 100;
                        *sh = imlib_image_get_height() * size / 100;
                }

                scaled = imlib_create_cropped_scaled_image(0, 0,
                                                           imlib_image_get_width(),
                                                           imlib_image_get_height(),
                                                           *sw, *sh);
                imlib_free_image_and_decache();
                imlib_context_set_image(scaled);
                return scaled;
        }

        return orig;

}


Imlib_Image image_load_random(char *fname, int *sw, int *sh)
{
        return image_load_core(fname, sw, sh, RANDOM_RESCALE, true, -1);
}

Imlib_Image image_load_scale(char *fname, int sw, int sh)
{
        return image_load_core(fname, &sw, &sh, SCALE, true, -1);
}

Imlib_Image image_load(char *fname)
{
        return image_load_core(fname, NULL, NULL, NO_SCALE, true, -1);
}


Imlib_Image image_load_relative(char *fname, int scale)
{
        int sw, sh;
        return image_load_core(fname, &sw, &sh, RELATIVE, false, scale);
}


void image_init(char *path)
{
        strncpy(image_path_loaded, path, MAX_PATH);
        image_path_loaded[MAX_PATH - 1] = '\0';
        image_path = image_path_loaded;
}
