

#include <time.h>
#include <stdlib.h>

#include "sal.h"
#include "config.h"
#include "image.h"
#include "window.h"
#include "aquarium.h"
#include "suncalc.h"

static struct sunsetrise ssr;
static int today = -1;
static int delta_rise, delta_set;
static int s_x, s_y;
static Imlib_Image figures, colon, red_figures;

static void update(struct aquarium *aquarium)
{
        int y_y, y_m, y_d;
        struct sunsetrise yesterday;
        time_t t;
        struct tm *p;
        int tw, th;

        time(&t);
        p = localtime(&t);

        if(today == p->tm_mday)
                return;

        today = p->tm_mday;

        sun(&ssr, p->tm_year + 1900, p->tm_mon + 1, p->tm_mday,
            p->tm_gmtoff / 3600, aquarium->lat, aquarium->lon);

        y_y = p->tm_year + 1900;
        y_m = p->tm_mon + 1;
        y_d = p->tm_mday;

        y_d--;

        if (y_d == 0) {
                y_m--;
                if(y_m == 0) {
                        y_m = 12;
                        y_y--;
                }
                y_d = 31;

                switch(y_m) {
                case 4:
                case 6:
                case 9:
                case 11:
                        y_d = 30;
                        break;
                case 2:
                        y_d = 28;
                        /* Give a damn about 2100 not beeing a leap year */
                        if (!(y_y % 4))
                            y_d++;
                default:
                        break;
                }
        }

        sun(&yesterday, y_y, y_m, y_d,
            p->tm_gmtoff / 3600, aquarium->lat, aquarium->lon);

        delta_set  = (ssr.sunset_h * 60 + ssr.sunset_m) - (yesterday.sunset_h * 60 + yesterday.sunset_m);
        delta_rise = (yesterday.sunrise_h * 60 + yesterday.sunrise_m) - (ssr.sunrise_h * 60 + ssr.sunrise_m);

        imlib_context_set_image(figures);
        tw = (((delta_set > 9) || (delta_rise > 9)) + 4) * imlib_image_get_width();
        th = 2 * imlib_image_get_height() / 10 + 2 + 4;
        imlib_context_set_image(colon);
        tw += imlib_image_get_width();

        aquarium_transform(aquarium->sunriseset,
                           tw, th,
                           &s_x, &s_y);
}

void sun_update(struct aquarium *aquarium)
{
        int t[2][3];
        int x, y;
        int i, j;

        if (aquarium->sunriseset == AL_NO)
                return;

        update(aquarium);

        t[0][0] = ssr.sunrise_h;
        t[0][1] = ssr.sunrise_m;
        t[0][2] = delta_rise;
        t[1][0] = ssr.sunset_h;
        t[1][1] = ssr.sunset_m;
        t[1][2] = delta_set;


        y = s_y;
        for (i = 0 ; i < 2; i++) {
                x = s_x;
                for (j = 0; j < 2; j++) {

                        imlib_context_set_image(figures);

                        window_draw((unsigned char *)imlib_image_get_data_for_reading_only(),
                                    0, (t[i][j] / 10) * imlib_image_get_height() / 10,
                                    imlib_image_get_width(),
                                    imlib_image_get_height() / 10,
                                    x, y,
                                    true);
                        x += imlib_image_get_width();

                        window_draw((unsigned char *)imlib_image_get_data_for_reading_only(),
                                    0, (t[i][j] % 10) * imlib_image_get_height() / 10,
                                    imlib_image_get_width(),
                                    imlib_image_get_height() / 10,
                                    x, y,
                                    true);
                        x += imlib_image_get_width();
                        imlib_context_set_image(colon);
                        if (!j) {
                                window_draw((unsigned char *)imlib_image_get_data_for_reading_only(),
                                            0, 0,
                                            imlib_image_get_width(),
                                            imlib_image_get_height(),
                                            x, y,
                                            true);
                                x += imlib_image_get_width();
                        }
                }
                x += 4;

                if (t[i][2] < 0)
                        imlib_context_set_image(red_figures);
                else
                        imlib_context_set_image(figures);

                if (abs(t[i][2]) > 9) {
                        window_draw((unsigned char *)imlib_image_get_data_for_reading_only(),
                                    0, (abs(t[i][2]) / 10) * imlib_image_get_height() / 10,
                                    imlib_image_get_width(),
                                    imlib_image_get_height() / 10,
                                    x, y,
                                    true);
                        x += imlib_image_get_width();
                }
                window_draw((unsigned char *)imlib_image_get_data_for_reading_only(),
                            0, (abs(t[i][2]) % 10) * imlib_image_get_height() / 10,
                            imlib_image_get_width(),
                            imlib_image_get_height() / 10,
                            x, y,
                            true);
                imlib_context_set_image(colon);
                y += imlib_image_get_height() + 2;
        }
}

void sun_init(struct aquarium *aquarium)
{

        if (aquarium->sunriseset == AL_NO)
                return;

        figures = image_load("digital-clock/smallfigures.png");
        colon = image_load("digital-clock/smallcolon.png");
        imlib_context_set_image(figures);
        red_figures = imlib_clone_image();

        image_change_color(figures,
                           RED(aquarium->sunriseset_color),
                           GREEN(aquarium->sunriseset_color),
                           BLUE(aquarium->sunriseset_color));

        image_change_color(colon,
                           RED(aquarium->sunriseset_color),
                           GREEN(aquarium->sunriseset_color),
                           BLUE(aquarium->sunriseset_color));

        image_change_color(red_figures,
                           255,
                           0,
                           0);

        update(aquarium);

}
