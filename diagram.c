
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "sal.h"
#include "config.h"
#include "window.h"
#include "aquarium.h"
#include "cpuload.h"


static int dh, dw, dlen, dx, dy;
static unsigned char *dhistory;

static void diagram_color(int bottom, int top, int level, int *r, int *g, int *b)
{
        (*r) = level * (RED(top) - RED(bottom)) / 100 + RED(bottom);
        (*g) = level * (GREEN(top) - GREEN(bottom)) / 100 + GREEN(bottom);
        (*b) = level * (BLUE(top) - BLUE(bottom)) / 100 + BLUE(bottom);
}

void diagram_update(struct aquarium *aquarium)
{
        static int dcurrent = 0;
        int i;
        int x, y;

        if (aquarium->diagram == AL_NO)
                return;

        dcurrent++;

        if (aquarium->fps == dcurrent) {
                dcurrent = 0;

                for (i = 0; i < (dlen - 1); i++) {
                        dhistory[i] = dhistory[i + 1];
                }
                /* Between 0 - 100, convert to 0 and dh */
                dhistory[dlen - 1] = (unsigned char) (dh * cpuload() / 100);
        }

	for (x = 0; x < dlen; x++) {
	    for (y = dh - (int)dhistory[x]; y < dh; y++) {
                    int r, g, b;
                    diagram_color(aquarium->diagram_color_bottom, aquarium->diagram_color_top, 100 - 100*y/dh, &r, &g, &b);
                    window_putpixel(dx + x, dy + y, r, g, b, 128);
	    }
	}

}

void diagram_init(struct aquarium *aquarium)
{

        if (aquarium->diagram == AL_NO)
                return;

        if (aquarium->w < aquarium->h) {
                dw = aquarium->w - 4;
                dh = 30;
        } else {
                dw = 30;
                dh = aquarium->h - 4;
        }
        dlen = dw;

        dhistory = malloc(sizeof(unsigned char) * dlen);
        memset(dhistory, 0, sizeof(unsigned char) * dlen);

        aquarium_transform(aquarium->diagram,
                           dw, dh,
                           &dx, &dy);
}
