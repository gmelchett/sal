

#include <time.h>
#include <stdio.h>

#include "sal.h"
#include "config.h"
#include "image.h"
#include "window.h"
#include "aquarium.h"
#include "suncalc.h"

static struct sunsetrise ssr;
static int today = -1;
static int s_x, s_y;
static Imlib_Image figures, colon;

static void update(struct aquarium *aquarium)
{
        time_t t;
        struct tm *p;
        time(&t);
        p = localtime(&t);

        if(today == p->tm_mday)
                return;

        today = p->tm_mday;

        sun(&ssr, p->tm_year + 1900, p->tm_mon + 1, p->tm_mday,
            p->tm_gmtoff / 3600, aquarium->lat, aquarium->lon);

}

void sun_update(struct aquarium *aquarium)
{
        int t[2][2];
        int x, y;
        int i, j;

        if (aquarium->sunriseset == AL_NO)
                return;

        update(aquarium);

        t[0][0] = ssr.sunrise_h;
        t[0][1] = ssr.sunrise_m;
        t[1][0] = ssr.sunset_h;
        t[1][1] = ssr.sunset_m;

        y = s_y;
        for (i = 0 ; i < 2; i++) {
                x = s_x;
                for (j = 0; j < 2; j++) {

                        imlib_context_set_image(figures);

                        window_draw_blend((unsigned char *)imlib_image_get_data_for_reading_only(),
                                          0, (t[i][j] / 10) * imlib_image_get_height() / 10,
                                          imlib_image_get_width(),
                                          imlib_image_get_height() / 10,
                                          x, y,
                                          128);
                        x += imlib_image_get_width();

                        window_draw_blend((unsigned char *)imlib_image_get_data_for_reading_only(),
                                          0, (t[i][j] % 10) * imlib_image_get_height() / 10,
                                          imlib_image_get_width(),
                                          imlib_image_get_height() / 10,
                                          x, y,
                                          128);
                        x += imlib_image_get_width();
                        imlib_context_set_image(colon);
                        if (!j) {
                                window_draw_blend((unsigned char *)imlib_image_get_data_for_reading_only(),
                                                  0, 0,
                                                  imlib_image_get_width(),
                                                  imlib_image_get_height(),
                                                  x, y,
                                                  128);
                                x += imlib_image_get_width();
                        }
                }
                y += imlib_image_get_height() + 2;
        }
}

void sun_init(struct aquarium *aquarium)
{
        int tw, th;

        if (aquarium->sunriseset == AL_NO)
                return;

        update(aquarium);

        figures = image_load("digital-clock/smallfigures.png");
        colon = image_load("digital-clock/smallcolon.png");

        imlib_context_set_image(figures);
        tw = 4 * imlib_image_get_width();
        th = 2 * imlib_image_get_height() / 10 + 2;
        imlib_context_set_image(colon);
        tw += imlib_image_get_width();

        aquarium_transform(aquarium->sunriseset,
                           tw, th,
                           &s_x, &s_y);

}
