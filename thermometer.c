

#include "sal.h"
#include "config.h"
#include "image.h"
#include "window.h"
#include "aquarium.h"
#include "cpuload.h"

static struct aquarium *aquarium;
static Imlib_Image thermometer_image;

void thermometer_init(void)
{
        aquarium = aquarium_get();

        if (aquarium->termometer == AL_NO)
                return;

        thermometer_image = image_load("thermometer.png");
}

void thermometer_update(void)
{
        int x, y, h;

        if (aquarium->termometer == AL_NO)
                return;

        imlib_context_set_image(thermometer_image);

        aquarium_transform(aquarium->termometer,
                          imlib_image_get_width(),
                          imlib_image_get_height(),
                          &x, &y);

        h = (100 - cpuload()) * (imlib_image_get_height() - 7) / 100;


        imlib_context_set_color(0xa0, 0x0, 0x0, 0xff);
        imlib_image_draw_line(3, 2,
                              3, h+2,
                              0);
        imlib_image_draw_line(4, 2,
                              4, h+2,
                              0);

        imlib_context_set_color(0xff, 0x0, 0x0, 0xff);
        imlib_image_draw_line(3, h+2,
                              3, imlib_image_get_height() - 5,
                              0);
        imlib_image_draw_line(4, h+2,
                              4, imlib_image_get_height() - 5,
                              0);

        window_draw_blend((unsigned char *)imlib_image_get_data_for_reading_only(),
                          0, 0,
                          imlib_image_get_width(),
                          imlib_image_get_height(),
                          x, y,
                          220);


}
