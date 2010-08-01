
#ifndef AQUARIUM_H
#define AQUARIUM_H

#include <X11/Xlib.h>
#include "sal.h"


#define ALH_LEFT   1
#define ALH_CENTER 2
#define ALH_RIGHT  3
#define ALH (0x3)
#define ALV_SHIFT 3
#define ALV_TOP    (1 << ALV_SHIFT)
#define ALV_CENTER (2 << ALV_SHIFT)
#define ALV_BOTTOM (3 << ALV_SHIFT)
#define ALV (0x3 << ALV_SHIFT)

#define AL_NO -1

#define RED(i) ((i >> 16) & 0xff)
#define GREEN(i) ((i >> 8) & 0xff)
#define BLUE(i) ((i >> 0) & 0xff)

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
        int termometer;
        int no_analog_clock;
        int no_analog_clock_seconds;
        int fuzzy_clock;
        int fuzzy_clock_color;
        int digital_clock;
        int digital_clock_small;
        int digital_clock_color;
        int digital_clock_seconds;
        int sunriseset;
        int sunriseset_color;
        double lat, lon;
        int fps;
        int leds[LEDS];
        Display *display;
};

void aquarium_transform(int loc, int w, int h, int *x, int *y);

#endif
