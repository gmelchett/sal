#include <stdlib.h>

#include "sal.h"
#include "config.h"
#include "image.h"
#include "window.h"
#include "aquarium.h"

#define BUBBLES_FRAMES 5
struct bubbles
{
        float x, y;
        float speed;
};

struct bubble {
        int h;
        int nr;
        Imlib_Image image;
        struct bubbles loc[MAX_BUBBLES];
};

static struct aquarium *aquarium;
static struct bubble bubble;

static void bubble_random(struct bubbles *b, bool random_y)
{
        b->x = (float)(random() % aquarium->w);
        if (random_y)
                b->y = (float) (random() % aquarium->h);
        else
                b->y = (float) aquarium->h + 5.0;
        b->speed = (float)((random() % MAX_BUBBLE_SPEED) + 1.0)/ (float)MAX_BUBBLE_SPEED;

}



void bubble_update(void)
{
        int i;

        imlib_context_set_image(bubble.image);

        for (i = 0; i < bubble.nr; i++) {

                bubble.loc[i].y -= bubble.loc[i].speed;

                if(bubble.loc[i].y < -(imlib_image_get_height() / BUBBLES_FRAMES)) {
                        bubble_random(&bubble.loc[i], False);
                        continue;
                }


                window_draw_blend((unsigned char *)imlib_image_get_data_for_reading_only(),
                                  0, ((BUBBLES_FRAMES - 1) * (int)bubble.loc[i].y / aquarium->h) * bubble.h,
                                  imlib_image_get_width(),
                                  bubble.h,
                                  (int)bubble.loc[i].x, (int)bubble.loc[i].y,
                                  128);
        }

}




void bubble_init(void)
{
        int i;

        aquarium = aquarium_get();

        bubble.image = image_load("bubbles.png");

        imlib_context_set_image(bubble.image);

        bubble.nr = random() % MAX_BUBBLES;

        bubble.h = imlib_image_get_height() / BUBBLES_FRAMES;

        for (i = 0; i < bubble.nr; i++)
                bubble_random(&bubble.loc[i], true);
}
