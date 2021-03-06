#ifndef WINDOW_H
#define WINDOW_H

#include <stdbool.h>
#include "aquarium.h"

void window_draw(unsigned char *source,
                 int sx, int sy, int sw, int sh,
                 int tx, int ty,
                 bool alpha_source);

void window_draw_blend(unsigned char *source,
                       int sx, int sy, int sw, int sh,
                       int tx, int ty,
                       int alpha);
void window_draw_line(int x1, int y1, int x2, int y2, int linewidth, int colour, int shaded);

void window_update(void);
void window_close(void);
int window_create(struct aquarium *aquarium);
bool window_visible(XConfigureRequestEvent e);
void window_putpixel(int x, int y, int r, int g, int b, int a);

#endif
