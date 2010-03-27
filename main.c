

#include "x.h"

int main(int argc, char **argv)
{
        x_create_window(300, 300);

        x_update();

        for(;;);
        x_close_window();

        return 0;
}
