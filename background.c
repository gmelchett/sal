
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <X11/Xlib.h>
#include <Imlib2.h>

#include "x.h"

#define IMAGE_PATH "images/"

static Imlib_Image foreground;



static char *plants[] = {
        "bottom/plant1.png",
        "bottom/plant2.png",
        "bottom/plant3.png",
        "bottom/plant4.png",
        "bottom/plant5.png",
        "bottom/plant6.png",
        "bottom/plant7.png",
        "bottom/plant8.png",
        "bottom/plant9.png",
        "bottom/smallstone1.png",
        "bottom/smallstone2.png",
        "bottom/stone1.png",
        "bottom/stone2.png",
        "bottom/stone3.png",
        "bottom/weirdplant1.png",
        "bottom/weirdplant2.png",
        "bottom/octo.png",
        "bottom/bigplant.png",
        NULL};

static char *bottoms[] = {
        "bottom/bottom1.png",
        "bottom/bottom2.png",
        NULL};

#define HAVE_BOTTOM 1
#define MAX_LAPS 6
#define MAX_PLANTS 15

static Imlib_Image load_image(char *fname, int *sw, int *sh, bool random_rescale)
{
        int size;
        Imlib_Image orig, scaled;
        char path[1024];

        snprintf(path, sizeof(path), "%s/%s", IMAGE_PATH, fname);

        orig = imlib_load_image(path);

        imlib_context_set_image(orig);

        if(random() % 2)
                imlib_image_flip_horizontal();

        if(random_rescale) {

                size = random() % 180 + 20;
                *sw = imlib_image_get_width() * size / 100;
                *sh = imlib_image_get_height() * size / 100;

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

struct plant_loc {
        int x, y, w, h;
};


void bottom(int bw, int bh)
{
        Imlib_Image bottom, plant;
        int num_p;
        int num_b;
        int sh, sw;
        int start_x, end_y;
        int w, sx, dx, px;
        int plant_top;
        int i,j;
        int num_plants;
        struct plant_loc plant_loc[MAX_PLANTS];

        if(!HAVE_BOTTOM)
                return;

        for (num_b = 0; bottoms[num_b] != NULL; num_b++);
        for (num_p = 0; plants[num_p] != NULL; num_p++);

        bottom = load_image(bottoms[random() % num_b], &sw, &sh, true);

        start_x = random() % imlib_image_get_width();
        end_y = random() % (imlib_image_get_height() - imlib_image_get_height() / 5);

        imlib_context_set_image(foreground);

        for (dx = -start_x; dx < bw; dx += sw) {
                if (dx < 0) {
                        sx = start_x;
                        w = sw - start_x;
                        px = 0;
                } else {
                        sx = 0;
                        w = sw;
                        px = dx;
                }
                imlib_blend_image_onto_image(bottom,
                                             true,
                                             sx, 0,
                                             w, end_y,
                                             px, bh - end_y,
                                             w, end_y);
        }
        imlib_context_set_image(bottom);
        imlib_free_image_and_decache();

        plant_top = end_y * 75 / 100;

        if(plant_top < 10)
                num_plants = 0;
        else
                num_plants = random() % MAX_PLANTS;

        for (i = 0 ; i < num_plants; i++) {
                int laps = 0;
                bool overlaps = false;
                int x, y;
                int dw, dh;

                plant = load_image(plants[random() % num_p], &sw, &sh, true);

                do {
                        x = random() % bw;
                        y = random() % plant_top;
                        overlaps = false;
                        for(j = 0; j < i; j ++)
                                if((x >= plant_loc[j].x && x <= (plant_loc[j].x + plant_loc[j].w) &&
                                    y >= plant_loc[j].y && y <= (plant_loc[j].y + plant_loc[j].h)) || 
                                   ((x + sw) >= plant_loc[j].x && (x + sw) <= (plant_loc[j].x + plant_loc[j].w) &&
                                    (y + sh) >= plant_loc[j].y && (y + sh) <= (plant_loc[j].y + plant_loc[j].h))) {
                                        overlaps = true;
                                        laps++;
                                        break;
                                }
                } while(overlaps && laps < MAX_LAPS);

                plant_loc[i].x = x;
                plant_loc[i].y = y;
                plant_loc[i].w = sw;
                plant_loc[i].h = sh;

                if(laps == MAX_LAPS) {
                        imlib_free_image();
                        break;
                }

                if (x + sw > bw)
                        dw = bw - x;
                else
                        dw = sw;

                if (bh - plant_top + y + sh > bh)
                        dh = bw - (bh - plant_top + y);
                else
                        dh = sh;

                if(dw <= 0 || dh <= 0) {
                        imlib_free_image();
                        break;
                }

                imlib_context_set_image(foreground);
                imlib_blend_image_onto_image(plant,
                                             true,
                                             0, 0,
                                             sw, sh,
                                             x, bh - plant_top + y,
                                             dw, dh);
                imlib_context_set_image(plant);
                imlib_free_image();
        }

}

void foreground_load(int bw, int bh)
{
        Imlib_Image image;
        char fname[1024];

        snprintf(fname, sizeof(fname), "%s/%s", IMAGE_PATH, "water.png");

        image = imlib_load_image(fname);
        imlib_context_set_image(image);

        foreground = imlib_create_cropped_scaled_image(0, 0,
                                                   imlib_image_get_width(),
                                                   imlib_image_get_height(),
                                                   bw, bh);
        imlib_free_image_and_decache();
        imlib_context_set_image(foreground);

        bottom(bw, bh);
        imlib_context_set_image(foreground);
        x_draw((unsigned char *)imlib_image_get_data_for_reading_only(),
               0,0,
               bw, bh,
               0,0,
               (bool)imlib_image_has_alpha());
        x_update();
}
