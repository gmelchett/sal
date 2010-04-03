#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <unistd.h>
#include <getopt.h>

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

static void aquarium_init(void)
{
        aquarium.w = aquarium.window_w - 2 * BORDER_WIDTH;
        aquarium.h = aquarium.window_h - 2 * BORDER_WIDTH;
}

static void parse_options(int argc, char **argv)
{
        int ch;
        int l, d1,d2;

        aquarium.window_w = DEFAULT_WINDOW_SIZE;
        aquarium.window_h = DEFAULT_WINDOW_SIZE;

        struct option long_opts[] = {

                {"w", true, NULL, 'w'},
                {"window", true, NULL, 'w'},
                {0, false, NULL, 0}
        };
        while ((ch = getopt_long_only(argc, argv, "", long_opts, NULL)) != -1) {
                switch(ch) {
                case 0:
                        break;
                case 'w':
                        l = sscanf(optarg, "%dx%d", &d1, &d2);
                        if(l != 2) {
                                printf("%s: syntax error. -w WxH, example: -w 100x100\n", argv[0]);
                                exit(-1);
                        }
                        if(d1 < 32 || d2 < 32 || d1 > 3000 || d2 > 3000) {
                                printf("%s: window size is out of range!\n", argv[0]);
                                exit(-2);
                        }
                        aquarium.window_w = d1;
                        aquarium.window_h = d2;

                        break;
                default:
                        printf("Unknown argument: %d!\n", ch);
                        break;
                }
        }

}

int main(int argc, char **argv)
{

        parse_options(argc, argv);

        srand(time(NULL));

        aquarium_init();
        bubble_init();
        background_init(BACKGROUND_SHADE, true);
        fish_init(50, 15);


        window_create();

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

