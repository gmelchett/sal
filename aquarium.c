#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
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
        int l, d1, d2;

        aquarium.window_w = DEFAULT_WINDOW_SIZE;
        aquarium.window_h = DEFAULT_WINDOW_SIZE;

        aquarium.background_type = BACKGROUND_SHADE;
        aquarium.random_fish = 15;
        aquarium.fish_scale = 50;

        struct option long_opts[] = {

                {"wi",     true,  NULL, 0x01},
                {"nb",     false, NULL, 0x02},
                {"na",     false, NULL, 0x03},
                {"bg",     true,  NULL, 0x04},
                {"rf",     true,  NULL, 0x05},
                {"nf",     true,  NULL, 0x06},
                {"sc",     true,  NULL, 0x07},
                {"help",   false, NULL, 0xff},
                {0,        false, NULL, 0x00}
        };
#if 1
        {
                int i,j;
                for(i = 0; long_opts[i].val != 0;i++)
                        for(j = 0; long_opts[j].val != 0; j++)
                                if (long_opts[i].val == long_opts[j].val && i != j)
                                        printf("Warning: both %s and %s is %d\n",
                                               long_opts[j].name, long_opts[i].name,
                                               long_opts[i].val);
        }
#endif

        while ((ch = getopt_long_only(argc, argv, "", long_opts, NULL)) != -1) {
                switch(ch) {
                case 0x00:
                        break;
                case 0x01: /* wi */
                        l = sscanf(optarg, "%dx%d", &d1, &d2);
                        if(l != 2) {
                                printf("%s: syntax error. -wi WxH, example: -wi 100x100\n", argv[0]);
                                exit(-1);
                        }
                        if(d1 < 32 || d2 < 32 || d1 > 3000 || d2 > 3000) {
                                printf("%s: -wi, window size is out of range!\n", argv[0]);
                                exit(-2);
                        }
                        aquarium.window_w = d1;
                        aquarium.window_h = d2;

                        break;
                case 0x02: /* nobottom */
                        aquarium.no_bottom = true;
                        break;
                case 0x03: /* nobottomanimals */
                        aquarium.no_bottom_animals = true;
                        break;
                case 0x04: /* bg */
                        l = strtol(optarg, (char **) NULL, 10);
                        if(l < 0 || l >= BACKGROUND_TYPE_END) {
                                printf("%s: -bg out of range (min:0, max:%d)\n", argv[0], BACKGROUND_TYPE_END);
                                exit(-1);
                        } else {
                                aquarium.background_type = l;
                        }
                        break;
                case 0x05: /* rf - max random number of fish */
                        l = strtol(optarg, (char **) NULL, 10);
                        if(l < 0 || l >= 1000) {
                                printf("%s: -rf out of range!\n", argv[0]);
                                exit(-1);
                        } else {
                                aquarium.random_fish = l;
                        }
                        break;
                case 0x06: /* nf - fixed number of fish */
                        l = strtol(optarg, (char **) NULL, 10);
                        if(l < 0 || l >= 1000) {
                                printf("%s: -nf out of range!\n", argv[0]);
                                exit(-1);
                        } else {
                                aquarium.random_fish = -1;
                                aquarium.num_fish = l;
                        }
                        break;
                case 0x07: /* sc - fish scale */
                        l = strtol(optarg, (char **) NULL, 10);
                        if(l < 0 || l >= 1000) {
                                printf("%s: -sc out of range!\n", argv[0]);
                                exit(-1);
                        } else {
                                aquarium.fish_scale = l;
                        }
                        break;
                case 0xff: /* help */
                        printf("help\n");
                        exit(0);
                        break;
                default:
                        printf("%s: Unknown argument!\n", argv[0]);
                        exit(-1);
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
        background_init();
        fish_init();
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

