#ifndef FISH_H
#define FISH_H

#include "aquarium.h"

void fish_update(struct aquarium *aquarium);
void fish_init(struct aquarium *aquarium);
void fish_leave(struct aquarium *aquarium);
void fish_enter(struct aquarium *aquarium);

#endif
