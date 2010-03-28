#include <X11/Xlib.h>

#include "sll.h"
#include "x.h"
#include "background.h"

int main(int argc, char **argv)
{
        x_create_window(300, 300);

        x_update();

        for(;;) {
                foreground_load(300 - 2*BORDER_WIDTH,
                                300 - 2*BORDER_WIDTH);
                sleep(1);
        }
        x_close_window();

        return 0;
}
