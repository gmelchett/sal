
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
        bool no_bottom;
        bool no_bottom_animals;
        enum background_type background_type;
};

struct aquarium *aquarium_get(void);

#endif
