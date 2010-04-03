#include <time.h>
#include <stdlib.h>
#include <unistd.h>

#include "config.h"
#include "sal.h"
#include "aquarium.h"
#include "bubble.h"
#include "background.h"
#include "window.h"
#include "fish.h"

static struct aquarium aquarium;


struct aquarium *aquarium_get(void)
{
        return &aquarium;
}

void aquarium_init(int bw, int bh)
{
        aquarium.w = bw;
        aquarium.h = bh;
}

int main(int argc, char **argv)
{
        int bh = 300, bw = 300;

        srand(time(NULL));

        aquarium_init(bw - 2*BORDER_WIDTH, bh - 2*BORDER_WIDTH);

        bubble_init();
        background_init(BACKGROUND_SHADE, true);
        fish_init(50, 15);

        window_create(bw, bh);

        for(;;) {
                background_update();
                fish_update();
                bubble_update();
                window_update();
                usleep(50000);
        }

        window_close();

        return 0;
}

