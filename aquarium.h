
#ifndef AQUARIUM_H
#define AQUARIUM_H

#include <X11/Xlib.h>
#include "background.h"
#include "leds.h"


#define ALH_LEFT   (1 << 0)
#define ALH_CENTER (1 << 1)
#define ALH_RIGHT  (1 << 2)
#define ALH (ALH_LEFT|ALH_CENTER|ALH_RIGHT)
#define ALV_TOP    (1 << 3)
#define ALV_CENTER (1 << 4)
#define ALV_BOTTOM (1 << 5)
#define ALV (ALV_TOP|ALV_CENTER|ALV_BOTTOM)

struct aquarium
{
        int w, h;
        int window_w, window_h;
        int fish_scale;
        int num_fish;
        int random_fish;
        int num_bubbles;
        int random_bubbles;
        int no_bottom;
        int no_bottom_animals;
        int background_type;
        int termometer_location;
        int fps;
        int leds[LEDS];
        Display *display;
};

struct aquarium *aquarium_get(void);

void aquarium_transform(int loc, int w, int h, int *x, int *y);


#endif
