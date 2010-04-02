#include <X11/Xlib.h>

#include <time.h>
#include <stdlib.h>


#include "sll.h"
#include "x.h"
#include "background.h"

int main(int argc, char **argv)
{
        int bh, bw;
        srand(time(NULL));

        x_create_window(300, 300);

        bw = 300 - 2*BORDER_WIDTH;
        bh = 300 - 2*BORDER_WIDTH;
        foreground_load(bw, bh);

        bubbles_init(bw, bh);
        fish_load(bw, bh, 50);

        for(;;) {
                foreground_update();
                fish_update(bw, bh);
                bubbles_update();
                x_update();
                usleep(10000);
        }
        x_close_window();

        return 0;
}
