
#ifndef AQUARIUM_H
#define AQUARIUM_H

#include <stdbool.h>
#include "background.h"

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
        int fps;
};

struct aquarium *aquarium_get(void);

#endif
