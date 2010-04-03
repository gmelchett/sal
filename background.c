
#include <string.h>
#include <stdlib.h>

#include "aquarium.h"
#include "config.h"
#include "sal.h"
#include "image.h"
#include "window.h"
#include "background.h"

#define MAX_LAPS 6

static struct aquarium *aquarium;

struct plant_loc {
        int x, y, w, h;
};

static Imlib_Image bg;

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


static void background_create(enum background_type type)
{
        unsigned char *buff;

        if(type == BACKGROUND_WATER) {
                bg = image_load_scale("water.png", aquarium->w, aquarium->h);

                return;
        }

        bg = imlib_create_image(aquarium->w, aquarium->h);
        imlib_context_set_image(bg);
        imlib_image_set_has_alpha(0);
        buff = (unsigned char *)imlib_image_get_data();

        if (type == BACKGROUND_SHADE) {

                int x, y, t;
                float d1, d2, d3;

                d1 = (float)(RED_END - RED_START) / (float) aquarium->h;
                d2 = (float)(GREEN_END - GREEN_START) / (float) aquarium->h;
                d3 = (float)(BLUE_END - BLUE_START) / (float) aquarium->h;

                for (y = 0; y < aquarium->h; y++) {
                        for (x = 0; x < aquarium->w; x++) {
                                t = (aquarium->w * 4) * y + x * 4;
                                buff[t + 0] = (unsigned char)((float)BLUE_START + ((float) y * d3));
                                buff[t + 1] = (unsigned char)((float)GREEN_START + ((float) y * d2));
                                buff[t + 2] = (unsigned char)((float)RED_START + ((float) y * d1));
                        }
                }
        }
        if (type == BACKGROUND_BLACK)
                memset(buff, 0, aquarium->w * aquarium->h * 4);


}



static int bottom_create(void)
{
        Imlib_Image bottom;
        int num_b;
        int start_x, end_y;
        int w, sx, dx, px, sw, sh;


        for (num_b = 0; bottoms[num_b] != NULL; num_b++);

        bottom = image_load_random(bottoms[random() % num_b], &sw, &sh);

        start_x = random() % imlib_image_get_width();
        end_y = random() % (imlib_image_get_height() - imlib_image_get_height() / 5);

        imlib_context_set_image(bg);

        for (dx = -start_x; dx < aquarium->w; dx += sw) {
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
                                             px, aquarium->h - end_y,
                                             w, end_y);
        }
        imlib_context_set_image(bottom);
        imlib_free_image_and_decache();
        return end_y;
}

static void bottom_animals_and_plants_create(int sand_height)
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

                plant = image_load_random(plants[random() % num_p], &sw, &sh);

                do {
                        x = random() % (aquarium->w + sw) - sw;
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

                imlib_context_set_image(bg);
                imlib_blend_image_onto_image(plant,
                                             true,
                                             0, 0,
                                             sw, sh,
                                             x, aquarium->h - plant_top + y,
                                             sw, sh);
                imlib_context_set_image(plant);
                imlib_free_image();
        }
}


void background_init(enum background_type type, bool has_bottom)
{
        int sand_height = 0;

        aquarium = aquarium_get();

        background_create(type);

        if(has_bottom)
                sand_height = bottom_create();
        if(sand_height)
                bottom_animals_and_plants_create(sand_height);

}


void background_update(void)
{

        imlib_context_set_image(bg);
        window_draw((unsigned char *)imlib_image_get_data_for_reading_only(),
                    0, 0,
                    imlib_image_get_width(), imlib_image_get_height(),
                    0, 0,
                    (bool)imlib_image_has_alpha());

}

