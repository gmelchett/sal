#ifndef BACKGROUND_H
#define BACKGROUND_H



void background_set(Display *display, Window win, int bw, int bh);
void foreground_load(int bw, int bh);

void bubbles_init(int bw, int bh);
void bubbles_update(void);

void fish_load(int bw, int bh, int scale);
void fish_update(int bw, int bh);
#endif
