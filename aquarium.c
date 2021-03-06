
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
#include "sun.h"
#include "date.h"
#include "diagram.h"
#include "cpuload.h"

static struct aquarium aquarium;


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
        char help[200];
};

static struct aquarium_option a_opts[];
static int aquarium_loc_usage[3][3][2];

static void aquarium_init(void)
{
        memset(aquarium_loc_usage, 0, sizeof(aquarium_loc_usage));
        aquarium.w = aquarium.window_w - 2 * BORDER_WIDTH;
        aquarium.h = aquarium.window_h - 2 * BORDER_WIDTH;
}

static int show_help(int *data, char *opt)
{
        int i;
        printf("\nSherman's aquarium - light v%s\n\n", VERSION);

        for (i = 0; strnlen(a_opts[i].name, MAX_ARG_LEN) != 0; i++) {
                if (a_opts[i].has_arg)
                        printf("\t%s <val>\t%s\n", a_opts[i].name, a_opts[i].help);
                else
                        printf("\t%s\t\t%s\n", a_opts[i].name, a_opts[i].help);
        }

        exit(0);
        return 0;
}

static int aquarium_location(int *data, char *opt)
{
        int x, y;
        if (strlen(opt) != 3)
                return 1;

        if (opt[1] != 'x')
                return 1;

        if (opt[2] != 't' && opt[2] != 'c' && opt[2] != 'b')
                return 1;

        switch(opt[0]) {
        case 'l': { *data = ALH_LEFT;   x = 0; break;}
        case 'c': { *data = ALH_CENTER; x = 1; break;}
        case 'r': { *data = ALH_RIGHT;  x = 2; break;}
        default: { return 1; }
        }

        switch(opt[2]) {
        case 't': { *data |= ALV_TOP;    y = 0; break;}
        case 'c': { *data |= ALV_CENTER; y = 1; break;}
        case 'b': { *data |= ALV_BOTTOM; y = 2; break;}
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

static int aquarium_double(int *data, char *opt)
{
        double *d = (double *)data;
        float f;
        int n;
        n = sscanf(opt, "%f", &f);

        if(n != 1) {
                printf("Error: Could not parse argument %s as float\n", opt);
                return 1;
        }

        *d = (double)f;

        return 0;
}

void aquarium_transform(int loc, int w, int h, int *x, int *y)
{
        int dw, dh;
        int nw = w, nh = h;

        dw = aquarium_loc_usage[loc & ALH][(loc & ALV) >> ALV_SHIFT][0];
        dh = aquarium_loc_usage[loc & ALH][(loc & ALV) >> ALV_SHIFT][1];

        switch(loc & ALH) {
        case ALH_LEFT:
                (*x) = 1 + dw;
                break;
        case ALH_CENTER:
                if (dw) {
                        (*x) = dw;
                        nw = dw + w;
                } else {
                        (*x) = aquarium.w/2 - w/2;
                        nw = aquarium.w/2 + w/2;
                }
                break;
        case ALH_RIGHT:
                (*x) = aquarium.w - w - dw - 1;
                break;
        default:
                break;
        }

        switch(loc & ALV) {
        case ALV_TOP:
                (*y) = 1 + dh;
                break;
        case ALV_CENTER:
                if (dh) {
                        (*y) = dh;
                        nh = dh + h;
                } else {
                        (*y) = aquarium.h/2 - h/2;
                        nh = aquarium.h/2 + h/2;
                }
                break;
        case ALV_BOTTOM:
                (*y) = aquarium.h - h - dh - 1;
                break;
        default:
                break;
        }


        if ((loc & ALH) != ALH_CENTER && (loc & ALV) == ALV_CENTER) {
                aquarium_loc_usage[loc & ALH][(loc & ALV) >> ALV_SHIFT][0] += nw + 1;
                return;
        }

        if ((loc & ALV) != ALV_CENTER && (loc & ALH) == ALH_CENTER) {
                aquarium_loc_usage[loc & ALH][(loc & ALV) >> ALV_SHIFT][1] += nh + 1;
                return;
        }

        /* laying down */
        if (aquarium.window_w > aquarium.window_h)
                aquarium_loc_usage[loc & ALH][(loc & ALV) >> ALV_SHIFT][0] += nw + 1;
        else
                aquarium_loc_usage[loc & ALH][(loc & ALV) >> ALV_SHIFT][1] += nh + 1;
}

static struct aquarium_option a_opts[] = {
        /* Window width */
        {
                .name     = "-wi",
                .help     = "Window witdh.",
                .has_arg  = true,
                .data     = &aquarium.window_w,
                .std      = DEFAULT_WINDOW_SIZE,
                .min      = 20,
                .max      = 3000,
        },
        /* Window height */
        {
                .name     = "-he",
                .help     = "Window height.",
                .has_arg  = true,
                .data     = &aquarium.window_h,
                .std      = DEFAULT_WINDOW_SIZE,
                .min      = 20,
                .max      = 3000,
        },
         /* No bottom */
        {
                .name     = "-nb",
                .help     = "Remove sea floor.",
                .std      = 0,
                .data     = &aquarium.no_bottom,
        },
         /* No bottom animals */
        {
                .name     = "-na",
                .help     = "No bottom animals.",
                .std      = 0,
                .data     = &aquarium.no_bottom_animals,
        },
        /* Analog clock */
        {
                .name     = "-ac",
                .help     = "No analog clock.",
                .std      = 0,
                .data     = &aquarium.no_analog_clock,
        },
        /* Analog clock - no seconds */
        {
                .name     = "-as",
                .help     = "Analog clock, no second arm.",
                .std      = 0,
                .data     = &aquarium.no_analog_clock_seconds,
        },
        /* Fuzzy clock */
        {
                .name     = "-fu",
                .help     = "Fuzzy clock.",
                .has_arg  = true,
                .data     = &aquarium.fuzzy_clock,
                .std      = AL_NO,
                .func_alt = aquarium_location,
        },
        /* Fuzzy clock - color */
        {
                .name     = "-fc",
                .help     = "Fuzzy clock color.",
                .has_arg  = true,
                .data     = &aquarium.fuzzy_clock_color,
                .std      = 0,
                .func_alt = aquarium_color,
        },

        /* Digital clock */
        {
                .name     = "-di",
                .help     = "Digital clock.",
                .has_arg  = true,
                .data     = &aquarium.digital_clock,
                .std      = AL_NO,
                .func_alt = aquarium_location,
        },
        /* Digital clock - small*/
        {
                .name     = "-dm",
                .help     = "Small digital clock",
                .has_arg  = true,
                .data     = &aquarium.digital_clock_small,
                .std      = AL_NO,
                .func_alt = aquarium_location,
        },

        /* Digital clock - color */
        {
                .name     = "-dc",
                .help     = "Digital clock color. Also for small digital clock.",
                .has_arg  = true,
                .data     = &aquarium.digital_clock_color,
                .std      = 0,
                .func_alt = aquarium_color,
        },
        /* Digital clock - seconds */
        {
                .name     = "-ds",
                .help     = "Show seconds (Digital clock). ",
                .data     = &aquarium.digital_clock_seconds,
                .std      = 0,
        },

        /* Date */
        {
                .name     = "-da",
                .help     = "Show date.",
                .has_arg  = true,
                .data     = &aquarium.date,
                .std      = AL_NO,
                .func_alt = aquarium_location,
        },

        /* Date - color */
        {
                .name     = "-tc",
                .help     = "Date color",
                .has_arg  = true,
                .data     = &aquarium.date_color,
                .std      = 0,
                .func_alt = aquarium_color,
        },

        /* Background type */
        {
                .name     = "-bg",
                .help     = "Background type.",
                .has_arg  = true,
                .data     = &aquarium.background_type,
                .std      = BACKGROUND_SHADE,
                .min      = 0,
                .max      = BACKGROUND_TYPE_END+1,
        },
        /* Random fish max */
        {
                .name     = "-rf",
                .help     = "Max random of fish.",
                .has_arg  = true,
                .data     = &aquarium.random_fish,
                .std      = 15,
                .min      = 0,
                .max      = 1000,
        },
        /* Fixed fish max */
        {
                .name     = "-nf",
                .help     = "Max fixed population.",
                .has_arg  = true,
                .data     = &aquarium.num_fish,
                .std      = -1,
                .min      = 0,
                .max      = 1000,
        },
        /* Fish scale */
        {
                .name     = "-sc",
                .help     = "Fish scale. (50 - 400).",
                .has_arg  = true,
                .data     = &aquarium.fish_scale,
                .std      = 50,
                .min      = 10,
                .max      = 400,
        },

        /* Diagram */
        {
                .name     = "-ia",
                .help     = "CPU load diagram.",
                .has_arg  = true,
                .data     = &aquarium.diagram,
                .std      = AL_NO,
                .func_alt = aquarium_location,
        },

        /* Diagram - bottom color */
        {
                .name     = "-ib",
                .help     = "Diagram bottom color.",
                .has_arg  = true,
                .data     = &aquarium.diagram_color_bottom,
                .std      = 0xa03020,
                .func_alt = aquarium_color,
        },

        /* Diagram - top color */
        {
                .name     = "-it",
                .help     = "Diagram top color.",
                .has_arg  = true,
                .data     = &aquarium.diagram_color_top,
                .std      = 0xffff00,
                .func_alt = aquarium_color,
        },

        /* Random bubble max */
        {
                .name     = "-ru",
                .help     = "Random max number of bubbles.",
                .has_arg  = true,
                .data     = &aquarium.random_bubbles,
                .std      = 15,
                .min      = 0,
                .max      = 1000,
        },
        /* Fixed bubble max */
        {
                .name     = "-nu",
                .help     = "Fixed number of bubbles.",
                .has_arg  = true,
                .data     = &aquarium.num_bubbles,
                .std      = -1,
                .min      = 0,
                .max      = 1000,
        },

        /* Frames per second */
        {
                .name     = "-fps",
                .help     = "Frames per second (sort of).",
                .has_arg  = true,
                .data     = &aquarium.fps,
                .std      = 20,
                .min      = 1,
                .max      = 200,
        },

        /* Restart after X hours */
        {
                .name     = "-rs",
                .help     = "Restart each <val> hour. For variation.",
                .has_arg  = true,
                .data     = &aquarium.restart_h,
                .std      = 48,
                .min      = 1,
                .max      = 500, /* overflow risk otherwise */
        },


        /* Termometer */
        {
                .name     = "-te",
                .help     = "Show thermometer (CPU load).",
                .has_arg  = true,
                .data     = &aquarium.termometer,
                .std      = AL_NO,
                .func_alt = aquarium_location,
        },

        /* Leds - numlock */
        {
                .name     = "-nl",
                .help     = "Show numlock led.",
                .has_arg  = true,
                .data     = &aquarium.leds[LEDS_NUMLOCK],
                .std      = AL_NO,
                .func_alt = aquarium_location,
        },
        /* Leds - capslock */
        {
                .name     = "-cl",
                .help     = "Show capslock led.",
                .has_arg  = true,
                .data     = &aquarium.leds[LEDS_CAPSLOCK],
                .std      = AL_NO,
                .func_alt = aquarium_location,
        },
        /* Leds - scrollock */
        {
                .name     = "-sl",
                .help     = "Show scrollock led.",
                .has_arg  = true,
                .data     = &aquarium.leds[LEDS_SCROLLOCK],
                .std      = AL_NO,
                .func_alt = aquarium_location,
        },

        /* sun rise and set */
        {
                .name     = "-ss",
                .help     = "Show sun rise and sun set.",
                .has_arg  = true,
                .data     = &aquarium.sunriseset,
                .std      = AL_NO,
                .func_alt = aquarium_location,
        },
        /* sun rise and set - color */
        {
                .name     = "-so",
                .help     = "Text color of sun rise and sun set.",
                .has_arg  = true,
                .data     = &aquarium.sunriseset_color,
                .std      = 0,
                .func_alt = aquarium_color,
        },

        /* latitude */
        {
                .name     = "-lat",
                .help     = "Latitude for sun rise and sun set calculations.",
                .has_arg  = true,
                .data     = (int *)&aquarium.lat,
                .std      = 0,
                .func_alt = aquarium_double,
        },
        /* longitude */
        {
                .name     = "-lon",
                .help     = "Longitude for sun rise and sun set calculations.",
                .has_arg  = true,
                .data     = (int *)&aquarium.lon,
                .std      = 0,
                .func_alt = aquarium_double,
        },

        /* Help text */
        {
                .name     = "--help",
                .help     = "Shows help text.",
                .func_alt = show_help,
        },
        /* Help text */
        {
                .name     = "-help",
                .help     = "Shows help text.",
                .func_alt = show_help,
        },
        /* Help text */
        {
                .name     = "--h",
                .help     = "Shows help text.",
                .func_alt = show_help,
        },
        /* Help text */
        {
                .name     = "-h",
                .help     = "Shows help text.",
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


        /* Default: Malmoe, Sweden */
        aquarium.lat = 55.35;
        aquarium.lon = 13.02;

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
                                                if(a_opts[j].func_alt(a_opts[j].data, argv[i + 1])) {
                                                        printf("Invalid argument\n");
                                                        exit(-1);
                                                }
                                        }
                                        i++;

                                } else {
                                        if (a_opts[j].data != NULL)
                                                *a_opts[j].data = !a_opts[j].std;
                                        if (a_opts[j].func_alt)
                                                a_opts[j].func_alt(a_opts[j].data, NULL);
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
        pid_t pid;
        unsigned int restart_count;

        parse_options(argc, argv);

        srand(time(NULL));
        aquarium_init();
        cpuload_init();

        bubble_init(&aquarium);
        background_init(&aquarium);
        fish_init(&aquarium);
        leds_init(&aquarium);
        thermometer_init(&aquarium);
        fuzzy_clock_init(&aquarium);
        digital_clock_init(&aquarium);
        sun_init(&aquarium);
        date_init(&aquarium);
        diagram_init(&aquarium);

        window_create(&aquarium);

        restart_count = aquarium.restart_h * 60 * 60 * aquarium.fps;

        while(restart_count) {

                while(XPending(aquarium.display)) {
                        XNextEvent(aquarium.display, &event);
                        switch(event.type) {
                        case EnterNotify:
                                fish_enter(&aquarium);
                                break;
                        case LeaveNotify:
                                fish_leave(&aquarium);
                                break;
                        case ConfigureNotify:
                                visible = window_visible(event.xconfigurerequest);
                                break;
                        default:
                                break;
                        }
                }

                if(visible) {
                        memset(aquarium_loc_usage, 0, sizeof(aquarium_loc_usage));
                        cpuload_update();
                        background_update(&aquarium);
                        fish_update(&aquarium);
                        bubble_update(&aquarium);
                        leds_update(&aquarium);
                        thermometer_update(&aquarium);
                        analog_clock_update(&aquarium);
                        fuzzy_clock_update(&aquarium);
                        digital_clock_update(&aquarium);
                        sun_update(&aquarium);
                        date_update(&aquarium);
                        diagram_update(&aquarium);

                        window_update();
                }
                /* Not really fps, but close enough */
                usleep(1000000 / aquarium.fps);
                restart_count--;
        }

        window_close();

        pid = fork();
        if(!pid)
                execvp(argv[0], argv);

        return 0;
}

