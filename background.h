#ifndef BACKGROUND_H
#define BACKGROUND_H

#include "aquarium.h"

#define BACKGROUND_WATER 0
#define BACKGROUND_SHADE 1
#define BACKGROUND_BLACK 2
#define BACKGROUND_TYPE_END BACKGROUND_BLACK

void background_init(struct aquarium *aquarium);

void background_update(struct aquarium *aquarium);


#endif
