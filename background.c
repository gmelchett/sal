
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <X11/Xlib.h>
#include <Imlib2.h>

#include "x.h"
#include "sll.h"

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



static void bottom_animals_and_plants(int bw, int bh, int sand_height)
{
        Imlib_Image plant;
        int num_p;
        int plant_top;
        int i, j;
        int num_plants;
        int sh, sw;
        struct plant_loc plant_loc[MAX_PLANTS];

        for (num_p = 0; plants[num_p] != NULL; num_p++);

        plant_top = sand_height * 75 / 100;

        if(plant_top < 10)
                num_plants = 0;
        else
                num_plants = random() % MAX_PLANTS;

        for (i = 0 ; i < num_plants; i++) {
                int laps = 0;
                bool overlaps = false;
                int x, y;

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

                imlib_context_set_image(foreground);
                imlib_blend_image_onto_image(plant,
                                             true,
                                             0, 0,
                                             sw, sh,
                                             x, bh - plant_top + y,
                                             sw, sh);
                imlib_context_set_image(plant);
                imlib_free_image();
        }
}

static int bottom_create(int bw, int bh)
{
        Imlib_Image bottom;
        int num_b;
        int start_x, end_y;
        int w, sx, dx, px, sw, sh;

        if(!HAVE_BOTTOM)
                return 0;

        for (num_b = 0; bottoms[num_b] != NULL; num_b++);

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
        return end_y;
}


void foreground_load(int bw, int bh)
{
        int sand_height;


        if(0) {
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
        } else {
                unsigned char *buff;
                int x, y, t;
                float d1, d2, d3;

                foreground = imlib_create_image(bw, bh);
                imlib_context_set_image(foreground);
                imlib_image_set_has_alpha(0);
                buff = (unsigned char *)imlib_image_get_data();

                d1 = (float)(RED_END - RED_START) / (float) bh;
                d2 = (float)(GREEN_END - GREEN_START) / (float) bh;
                d3 = (float)(BLUE_END - BLUE_START) / (float) bh;

                for (y = 0; y < bh; y++) {
                        for (x = 0; x < bw; x++) {
                                t = (bw * 4) * y + x * 4;
                                buff[t + 0] = (unsigned char)((float)BLUE_START + ((float) y * d3));
                                buff[t + 1] = (unsigned char)((float)GREEN_START + ((float) y * d2));
                                buff[t + 2] = (unsigned char)((float)RED_START + ((float) y * d1));
                        }
                }
        }


        sand_height = bottom_create(bw, bh);
        if(sand_height)
                bottom_animals_and_plants(bw, bh, sand_height);
}


void foreground_update(void)
{

        imlib_context_set_image(foreground);
        x_draw((unsigned char *)imlib_image_get_data_for_reading_only(),
               0,0,
               imlib_image_get_width(), imlib_image_get_height(),
               0,0,
               (bool)imlib_image_has_alpha());

}

#define BUBBLES_FRAMES 5
#define MAX_BUBBLES 40
#define MAX_BUBBLE_SPEED 13
struct bubbles
{
        float x, y;
        float speed;
};
static int bubbles_nr;
static Imlib_Image bubbles_image;
static struct bubbles bubbles_loc[MAX_BUBBLES];
static int bubbles_h, bubbles_w;

static void bubble_random(struct bubbles *b, bool random_y)
{
        b->x = (float)( random() % bubbles_w);
        if (random_y)
                b->y = (float) (random() % bubbles_h);
        else
                b->y = (float) bubbles_h + 5.0;
        b->speed = (float)(random() % MAX_BUBBLE_SPEED) / (float)MAX_BUBBLE_SPEED;

}

void bubbles_init(int bw, int bh)
{
        int i;
        bubbles_image = load_image("bubbles.png", 0, 0, False);

        bubbles_nr = random() % MAX_BUBBLES;

        bubbles_h = bh;
        bubbles_w = bw;
        for (i = 0; i < bubbles_nr; i++)
                bubble_random(&bubbles_loc[i], True);
}


void bubbles_update(void)
{
        int i;

        imlib_context_set_image(bubbles_image);

        for (i = 0; i < bubbles_nr; i++) {

                bubbles_loc[i].y -= bubbles_loc[i].speed;

                if(bubbles_loc[i].y < -(imlib_image_get_height() / BUBBLES_FRAMES)) {
                        bubble_random(&bubbles_loc[i], False);
                        continue;
                }


                x_draw((unsigned char *)imlib_image_get_data_for_reading_only(),
                       0, imlib_image_get_height() / BUBBLES_FRAMES * (int)bubbles_loc[i].y / bubbles_h * BUBBLES_FRAMES,
                       imlib_image_get_width(),
                       imlib_image_get_height() / BUBBLES_FRAMES,
                       (int)bubbles_loc[i].x, (int)bubbles_loc[i].y,
                       (bool)imlib_image_has_alpha());
        }

}
