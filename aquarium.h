
#ifndef AQUARIUM_H
#define AQUARIUM_H


struct aquarium
{
        int w, h;
        int window_w, window_h;
};

struct aquarium *aquarium_get(void);

#endif
