
#define _GNU_SOURCE

#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <X11/Xlib.h>

#include "config.h"
#include "sal.h"
#include "aquarium.h"
#include "bubble.h"
#include "background.h"
#include "window.h"
#include "fish.h"
#include "leds.h"
#include "thermometer.h"
#include "analog-clock.h"
#include "fuzzy-clock.h"
#include "digital-clock.h"

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
        int (*func_alt)(int *data, char *opt);
        int min;
        int max;
};


static int show_help(int *data, char *opt)
{
        printf("Show help\n");
        exit(0);
        return 0;
}

static int aquarium_location(int *data, char *opt)
{
        if (strlen(opt) != 3)
                return 1;

        if (opt[1] != 'x')
                return 1;

        if (opt[2] != 't' && opt[2] != 'c' && opt[2] != 'b')
                return 1;

        switch(opt[0]) {
        case 'l': { *data = ALH_LEFT;   break;}
        case 'c': { *data = ALH_CENTER; break;}
        case 'r': { *data = ALH_RIGHT;  break;}
        default: { return 1; }
        }

        switch(opt[2]) {
        case 't': { *data |= ALV_TOP;    break;}
        case 'c': { *data |= ALV_CENTER; break;}
        case 'b': { *data |= ALV_BOTTOM; break;}
        default: { return 1; }
        }

        return 0;
}

static int aquarium_color(int *data, char *opt)
{
        int r, g, b, n;
        n = sscanf(opt, "%d,%d,%d", &r, &g, &b);
        if (n != 3 || r < 0 || r > 255 || g < 0 || g > 255 || b < 0 || b > 255) {
                printf("Bad color. Syntax: r,g,b. where r,g and b are 0 to 255.\n");
                return 1;
        }

        *data = r << 16 | g << 8 | b;
        return 0;
}


void aquarium_transform(int loc, int w, int h, int *x, int *y)
{

        switch(loc & ALH) {
        case ALH_LEFT:
                (*x) = 1;
                break;
        case ALH_CENTER:
                (*x) = aquarium.w / 2 - w / 2;
                break;
        case ALH_RIGHT:
                (*x) = aquarium.w - w - 1;
                break;
        default:
                break;
        }

        switch(loc & ALV) {
        case ALV_TOP:
                (*y) = 1;
                break;
        case ALV_CENTER:
                (*y) = aquarium.h / 2 - h / 2;
                break;
        case ALV_BOTTOM:
                (*y) = aquarium.h - h - 1;
                break;
        default:
                break;
        }

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
                .std      = 0,
                .data     = &aquarium.no_bottom,
        },
         /* No bottom animals */
        {
                .name     = "-na",
                .std      = 0,
                .data     = &aquarium.no_bottom_animals,
        },
        /* Analog clock */
        {
                .name     = "-ac",
                .std      = 0,
                .data     = &aquarium.no_analog_clock,
        },
        /* Analog clock - no seconds */
        {
                .name     = "-as",
                .std      = 0,
                .data     = &aquarium.no_analog_clock_seconds,
        },
        /* Fuzzy clock */
        {
                .name     = "-fu",
                .has_arg  = true,
                .data     = &aquarium.fuzzy_clock,
                .std      = AL_NO,
                .func_alt = aquarium_location,
        },
        /* Fuzzy clock - color */
        {
                .name     = "-fc",
                .has_arg  = true,
                .data     = &aquarium.fuzzy_clock_color,
                .std      = 0,
                .func_alt = aquarium_color,
        },

        /* Digital clock */
        {
                .name     = "-di",
                .has_arg  = true,
                .data     = &aquarium.digital_clock,
                .std      = AL_NO,
                .func_alt = aquarium_location,
        },
        /* Digital clock - small*/
        {
                .name     = "-dm",
                .has_arg  = true,
                .data     = &aquarium.digital_clock_small,
                .std      = AL_NO,
                .func_alt = aquarium_location,
        },

        /* Digital clock - color */
        {
                .name     = "-dc",
                .has_arg  = true,
                .data     = &aquarium.digital_clock_color,
                .std      = 0,
                .func_alt = aquarium_color,
        },
        /* Digital clock - seconds */
        {
                .name     = "-ds",
                .data     = &aquarium.digital_clock_seconds,
                .std      = 0,
        },

        /* Background type */
        {
                .name     = "-bg",
                .has_arg  = true,
                .data     = &aquarium.background_type,
                .std      = BACKGROUND_SHADE,
                .min      = 0,
                .max      = BACKGROUND_TYPE_END+1,
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

        /* Termometer */
        {
                .name     = "-te",
                .has_arg  = true,
                .data     = &aquarium.termometer,
                .std      = AL_NO,
                .func_alt = aquarium_location,
        },

        /* Leds - numlock */
        {
                .name     = "-nl",
                .has_arg  = true,
                .data     = &aquarium.leds[LEDS_NUMLOCK],
                .std      = AL_NO,
                .func_alt = aquarium_location,
        },
        /* Leds - capslock */
        {
                .name     = "-cl",
                .has_arg  = true,
                .data     = &aquarium.leds[LEDS_CAPSLOCK],
                .std      = AL_NO,
                .func_alt = aquarium_location,
        },
        /* Leds - scrollock */
        {
                .name     = "-sl",
                .has_arg  = true,
                .data     = &aquarium.leds[LEDS_SCROLLOCK],
                .std      = AL_NO,
                .func_alt = aquarium_location,
        },


        /* Help text */
        {
                .name     = "--help",
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
                                                if(a_opts[j].func_alt(a_opts[j].data, argv[i+1])) {
                                                        printf("Invalid argument\n");
                                                        exit(-1);
                                                }
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
        bool visible = true;;
        XEvent event;
        parse_options(argc, argv);

        srand(time(NULL));
        aquarium_init();

        bubble_init();
        background_init();
        fish_init();
        leds_init();
        thermometer_init();
        analog_clock_init();
        fuzzy_clock_init();
        digital_clock_init();

        window_create();

        for(;;) {

                while(XPending(aquarium.display)) {
                        XNextEvent(aquarium.display, &event);
                        switch(event.type) {
                        case EnterNotify:
                                fish_enter();
                                break;
                        case LeaveNotify:
                                fish_leave();
                                break;
                        case ConfigureNotify:
                                visible = window_visible(event.xconfigurerequest);
                                break;
                        default:
                                break;
                        }
                }

                if(visible) {
                        background_update();
                        fish_update();
                        bubble_update();
                        leds_update();
                        thermometer_update();
                        analog_clock_update();
                        fuzzy_clock_update();
                        digital_clock_update();

                        window_update();
                }
                /* Not really fps, but close enough */
                usleep(1000000 / aquarium.fps);
        }

        window_close();

        return 0;
}

