#ifndef X_H
#define X_H

#include <stdbool.h>

void x_draw(unsigned char *source,
            int sx, int sy, int sw, int sh,
            int tx, int ty,
            bool alpha_source);
void x_draw_blend(unsigned char *source,
                  int sx, int sy, int sw, int sh,
                  int tx, int ty,
                  int alpha);

void x_update(void);
void x_close_window(void);
int x_create_window(int width, int height);

#endif
