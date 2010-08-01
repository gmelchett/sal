
#include <stdio.h>
#include <time.h>

#include "sal.h"
#include "config.h"
#include "image.h"
#include "window.h"
#include "aquarium.h"

static Imlib_Image figures, colon;
static struct aquarium *aquarium;
static int dc_x, dc_y;

void digital_clock_update(void)
{
        time_t now;
        struct tm *mt;
        int x = dc_x;
        int t[3];
        int l, i;

        if(aquarium->digital_clock == AL_NO)
                return;

	now = time(NULL);
	mt = localtime(&now);

        t[0] = mt->tm_hour;
        t[1] = mt->tm_min;
        t[2] = mt->tm_sec;

        l = 2 + aquarium->digital_clock_seconds;

        for (i = 0; i < l; i++) {

                imlib_context_set_image(figures);

                window_draw_blend((unsigned char *)imlib_image_get_data_for_reading_only(),
                                  0, (t[i] / 10) * imlib_image_get_height() / 10,
                                  imlib_image_get_width(),
                                  imlib_image_get_height() / 10,
                                  x, dc_y,
                                  128);
                x += imlib_image_get_width();

                window_draw_blend((unsigned char *)imlib_image_get_data_for_reading_only(),
                                  0, (t[i] % 10) * imlib_image_get_height() / 10,
                                  imlib_image_get_width(),
                                  imlib_image_get_height() / 10,
                                  x, dc_y,
                                  128);
                x += imlib_image_get_width();

                if (i + 1 != l) {
                        imlib_context_set_image(colon);
                        window_draw_blend((unsigned char *)imlib_image_get_data_for_reading_only(),
                                          0, 0,
                                          imlib_image_get_width(),
                                          imlib_image_get_height(),
                                          x, dc_y,
                                          128);
                        x += imlib_image_get_width();
                }
        }
}

void digital_clock_init(void)
{
        int total_width;
        aquarium = aquarium_get();

        if(aquarium->digital_clock != AL_NO && aquarium->digital_clock_small != AL_NO) {
                printf("Error: You can't have both a small and a large digital clock at the same time.\n");
                return;
        }

        if(aquarium->digital_clock != AL_NO) {
                figures = image_load("digital-clock/bigfigures.png");
                colon = image_load("digital-clock/bigcolon.png");

        } else if(aquarium->digital_clock_small != AL_NO) {
                figures = image_load("digital-clock/smallfigures.png");
                colon = image_load("digital-clock/smallcolon.png");

                aquarium->digital_clock = aquarium->digital_clock_small;
        } else
                return;

        image_change_color(figures,
                           RED(aquarium->digital_clock_color),
                           GREEN(aquarium->digital_clock_color),
                           BLUE(aquarium->digital_clock_color));
        image_change_color(colon,
                           RED(aquarium->digital_clock_color),
                           GREEN(aquarium->digital_clock_color),
                           BLUE(aquarium->digital_clock_color));

        imlib_context_set_image(figures);
        total_width = (4 + 2 * aquarium->digital_clock_seconds) * imlib_image_get_width();
        imlib_context_set_image(colon);
        total_width += (1 + aquarium->digital_clock_seconds) *  imlib_image_get_width();

        aquarium_transform(aquarium->digital_clock,
                           total_width, imlib_image_get_height(),
                           &dc_x, &dc_y);
}
