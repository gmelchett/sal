
#define _GNU_SOURCE

#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>


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



/* getopt is ok, but with lots of options it requires too much doublicated code */
#define MAX_ARG_LEN 20

struct aquarium_option {
        char name[MAX_ARG_LEN];
        bool has_arg;
        int *data;
        int std;
        void (*func_alt)(char *opt);
        int min;
        int max;
};


static void show_help(char *opt)
{
        printf("Show help\n");
        exit(0);
}


static struct aquarium_option a_opts[] = {
        /* Window width */
        {
                .name     = "-wi",
                .has_arg  = true,
                .data     = &aquarium.window_w,
                .std      = DEFAULT_WINDOW_SIZE,
                .min      = 20,
                .max      = 3000,
        },
        /* Window height */
        {
                .name     = "-he",
                .has_arg  = true,
                .data     = &aquarium.window_h,
                .std      = DEFAULT_WINDOW_SIZE,
                .min      = 20,
                .max      = 3000,
        },
         /* No bottom */
        {
                .name     = "-nb",
                .data     = &aquarium.no_bottom,
        },
         /* No bottom animals */
        {
                .name     = "-na",
                .data     = &aquarium.no_bottom_animals,
        },
        /* Background type */
        {
                .name     = "-bg",
                .has_arg  = true,
                .data     = &aquarium.background_type,
                .std      = BACKGROUND_SHADE,
                .min      = 0,
                .max      = BACKGROUND_TYPE_END,
        },
        /* Random fish max */
        {
                .name     = "-rf",
                .has_arg  = true,
                .data     = &aquarium.random_fish,
                .std      = 15,
                .min      = 0,
                .max      = 1000,
        },
        /* Fixed fish max */
        {
                .name     = "-nf",
                .has_arg  = true,
                .data     = &aquarium.num_fish,
                .std      = -1,
                .min      = 0,
                .max      = 1000,
        },
        /* Fish scale */
        {
                .name     = "-sc",
                .has_arg  = true,
                .data     = &aquarium.fish_scale,
                .std      = 50,
                .min      = 10,
                .max      = 400,
        },



        /* Random bubble max */
        {
                .name     = "-ru",
                .has_arg  = true,
                .data     = &aquarium.random_bubbles,
                .std      = 15,
                .min      = 0,
                .max      = 1000,
        },
        /* Fixed bubble max */
        {
                .name     = "-nu",
                .has_arg  = true,
                .data     = &aquarium.num_bubbles,
                .std      = -1,
                .min      = 0,
                .max      = 1000,
        },


        /* Frames per second */
        {
                .name     = "-fps",
                .has_arg  = true,
                .data     = &aquarium.fps,
                .std      = 20,
                .min      = 1,
                .max      = 200,
        },


        /* Help text */
        {
                .name     = "-help",
                .func_alt = show_help,
        },

        /* Termination */
        {
                .name     = "",
        },

#if 0
        {
                .name     = "",
                .has_arg  = ,
                .data     = ,
                .std      = ,
                .func_alt = ,
                .min      = ,
                .max      = ,
        },
#endif

};


static void parse_options(int argc, char **argv)
{

        int i, j, a_opts_len  = 0;

        for (a_opts_len = 0; strnlen(a_opts[a_opts_len].name, MAX_ARG_LEN) != 0; a_opts_len++) {
                if(a_opts[a_opts_len].data != NULL)
                        *a_opts[a_opts_len].data = a_opts[a_opts_len].std;
        }


#if 1
        {
                for(i = 0; i < a_opts_len; i++)
                        for(j = 0; j < a_opts_len; j++)
                                if (!strcmp(a_opts[i].name, a_opts[j].name) && i != j)
                                        printf("Warning: both %s and %s is %d\n",
                                               a_opts[i].name, a_opts[j].name, i);
        }
#endif


        for (i = 1; i < argc; i++) {

                for (j = 0; j < a_opts_len; j++) {
                        if(!strncmp(a_opts[j].name, argv[i], MAX_ARG_LEN)) {
                                if(a_opts[j].has_arg) {
                                        int val;
                                        if((i + 1) == argc) {
                                                printf("Argument %s requires option.\n",
                                                       a_opts[j].name);
                                                exit(-1);
                                        }

                                        if (a_opts[j].func_alt == NULL) {
                                                val = strtol(argv[i + 1], (char **) NULL, 10);


                                                if (val < a_opts[j].min || val >= a_opts[j].max) {
                                                        printf("Argument %s options out of range. Got %d min %d, max %d\n",
                                                               a_opts[j].name, val, a_opts[j].min, a_opts[j].max - 1);
                                                        exit(-1);
                                                }
                                                if (a_opts[j].data != NULL)
                                                        *a_opts[j].data = val;
                                        } else {
                                                a_opts[j].func_alt(argv[i+1]);
                                        }
                                        i++;

                                } else {
                                        if (a_opts[j].data != NULL)
                                                *a_opts[j].data = !a_opts[j].std;
                                }

                                break;
                        }
                }
                if (j == a_opts_len) {
                        printf("Unknown argument %s\n", argv[i]);
                        exit(-1);
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
                /* Not really fps, but close enough */
                usleep(1000000/aquarium.fps);
        }

        window_close();

        return 0;
}

