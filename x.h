#ifndef X_H
#define X_H

void x_draw(unsigned char *source,
            int sx, int sy, int sw, int sh,
            int tx, int ty,
            int rs_source, int alpha_source, int ignore_alpha);
void x_update(void);
void x_close_window(void);
int x_create_window(int width, int height);

#endif
