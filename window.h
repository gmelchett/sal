#ifndef WINDOW_H
#define WINDOW_H

#include <stdbool.h>

void window_draw(unsigned char *source,
                 int sx, int sy, int sw, int sh,
                 int tx, int ty,
                 bool alpha_source);

void window_draw_blend(unsigned char *source,
                       int sx, int sy, int sw, int sh,
                       int tx, int ty,
                       int alpha);

void window_update(void);
void window_close(void);
int window_create(int width, int height);

#endif
