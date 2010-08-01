
#include <X11/XKBlib.h>

#include "sal.h"
#include "config.h"
#include "image.h"
#include "window.h"
#include "aquarium.h"

static Imlib_Image led_image;

void leds_init(struct aquarium *aquarium)
{
        int i;

        for (i = 0; i < LEDS; i++) {
                if (aquarium->leds[i] != AL_NO) {
                        led_image = image_load("leds.png");
                        break;
                }
        }

}

static void leds_core(struct aquarium *aquarium, int led, int pressed, int loc, int x_delta)
{
        int x, y, d;

        if(loc == AL_NO || !pressed)
                return;

        aquarium_transform(loc,
                          imlib_image_get_width(),
                          imlib_image_get_height() / LEDS,
                          &x, &y);

        if (x > aquarium->w / 2)
                d = -x_delta * imlib_image_get_width();
        else
                d = x_delta * imlib_image_get_width();

        window_draw_blend((unsigned char *)imlib_image_get_data_for_reading_only(),
                          0, (imlib_image_get_height() / LEDS) * led,
                          imlib_image_get_width(),
                          imlib_image_get_height() / LEDS,
                          x + d, y,
                          128);
}

void leds_update(struct aquarium *aquarium)
{
        int i, j;
        unsigned int states;

        if (XkbGetIndicatorState(aquarium->display,
                                 XkbUseCoreKbd, &states) != Success) {
                return;
        }

        imlib_context_set_image(led_image);

        for (i = 0; i < LEDS; i++) {
                int x_delta = 0;
                for (j = 0 ; j < i; j++) {
                        if (aquarium->leds[j] == aquarium->leds[i])
                                x_delta++;
                }
                leds_core(aquarium, i, (states >> i) & 1, aquarium->leds[i], x_delta);
        }

}
