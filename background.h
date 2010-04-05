#ifndef BACKGROUND_H
#define BACKGROUND_H

#include <stdbool.h>



enum background_type {
        BACKGROUND_WATER = 0,
        BACKGROUND_SHADE,
        BACKGROUND_BLACK,
        BACKGROUND_TYPE_END
};

void background_init(void);

void background_update(void);


#endif
