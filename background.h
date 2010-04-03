#ifndef BACKGROUND_H
#define BACKGROUND_H

#include <stdbool.h>


enum background_type {
        BACKGROUND_WATER,
        BACKGROUND_SHADE,
        BACKGROUND_BLACK,
};

void background_init(enum background_type type, bool has_bottom);

void background_update(void);


#endif
