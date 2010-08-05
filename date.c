#include <time.h>
#include <stdio.h>

#include "sal.h"
#include "config.h"
#include "image.h"
#include "window.h"
#include "aquarium.h"

static Imlib_Image figures, months, weekdays;

void date_update(struct aquarium *aquarium)
{
        time_t now;
        struct tm *mt;
        int tw;
        int x, y;

        if (aquarium->date == AL_NO)
                return;

	now = time(NULL);
	mt = localtime(&now);

        imlib_context_set_image(months);
        tw = imlib_image_get_width();

        imlib_context_set_image(figures);
        tw += (1 + (mt->tm_mday > 9)) * imlib_image_get_width();

        aquarium_transform(aquarium->date,
                           tw, imlib_image_get_height(),
                           &x, &y);

        if (mt->tm_mday > 9) {
                window_draw((unsigned char *)imlib_image_get_data_for_reading_only(),
                            0, (mt->tm_mday / 10) * (imlib_image_get_height() / 10),
                            imlib_image_get_width(),
                            imlib_image_get_height() / 10,
                            x, y,
                            true);
                x += imlib_image_get_width();
        }
        window_draw((unsigned char *)imlib_image_get_data_for_reading_only(),
                    0, (mt->tm_mday % 10) * (imlib_image_get_height() / 10),
                    imlib_image_get_width(),
                    imlib_image_get_height() / 10,
                    x, y,
                    true);
        x += imlib_image_get_width() + 3;

        imlib_context_set_image(months);
        window_draw((unsigned char *)imlib_image_get_data_for_reading_only(),
                    0, mt->tm_mon * imlib_image_get_height() / 12,
                    imlib_image_get_width(),
                    imlib_image_get_height() / 12,
                    x, y,
                    true);
}

void date_init(struct aquarium *aquarium)
{
        if (aquarium->date == AL_NO)
                return;

        figures = image_load("date/figures.png");
        months = image_load("date/months.png");
        weekdays = image_load("date/weekdays.png");

        image_change_color(figures,
                           RED(aquarium->date_color),
                           GREEN(aquarium->date_color),
                           BLUE(aquarium->date_color));
        image_change_color(months,
                           RED(aquarium->date_color),
                           GREEN(aquarium->date_color),
                           BLUE(aquarium->date_color));
        image_change_color(weekdays,
                           RED(aquarium->date_color),
                           GREEN(aquarium->date_color),
                           BLUE(aquarium->date_color));
}


