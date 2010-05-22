
#include "sal.h"
#include "config.h"
#include "aquarium.h"
#include "window.h"

#include <math.h>
#include <time.h>

static struct aquarium *aquarium;

#define HOUR_ARM_LEN 0.58
#define MIN_ARM_LEN  0.74
#define SEC_ARM_LEN  0.9

#define HOUR_COLOR ((0   << 16) | (223 << 8) | 223)
#define MIN_COLOR  ((23  << 16) | (223 << 8) |   4)
#define SEC_COLOR  ((255 << 16) | (0   << 8) | 255)

void analog_clock_init(void)
{
        aquarium = aquarium_get();
}

static void analog_clock_loc(int t1, int t2, float t_max, float lx, float ly, int *dx, int *dy)
{
        (*dx) = (int)(lx * cos(2 * M_PI * ((float)t1 + (float)t2 / 60.0) / t_max - M_PI/2));
	(*dy) = (int)(ly * sin(2 * M_PI * ((float)t1 + (float)t2 / 60.0) / t_max - M_PI/2));
}

void analog_clock_update(void)
{
        int dhx, dhy;
        int dmx, dmy;
        int dsx, dsy;
        float lx, ly;

        time_t now;
        struct tm *mt;

        if(aquarium->no_analog_clock)
                return;

	now = time(NULL);
	mt = localtime(&now);

        lx = (float)aquarium->w /2.0;
        ly = (float)aquarium->h /2.0;

        analog_clock_loc(mt->tm_hour, mt->tm_min,
                         12.0,
                         HOUR_ARM_LEN * lx,
                         HOUR_ARM_LEN * ly,
                         &dhx, &dhy);
        analog_clock_loc(mt->tm_min, mt->tm_sec,
                         60.0,
                         MIN_ARM_LEN * lx,
                         MIN_ARM_LEN * ly,
                         &dmx, &dmy);
        if(!aquarium->no_analog_clock_seconds)
                analog_clock_loc(mt->tm_sec, 0,
                                 60.0,
                                 SEC_ARM_LEN * lx,
                                 SEC_ARM_LEN * ly,
                                 &dsx, &dsy);

        window_draw_line((int)lx + dhx, (int)ly + dhy , (int)lx, (int)ly, 1, HOUR_COLOR, 80);
        window_draw_line((int)lx + dmx, (int)ly + dmy , (int)lx, (int)ly, 1, MIN_COLOR,  80);

        if(!aquarium->no_analog_clock_seconds)
                window_draw_line((int)lx + dsx, (int)ly + dsy , (int)lx, (int)ly, 1, SEC_COLOR,  80);

}
