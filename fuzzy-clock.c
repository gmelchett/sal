
#include <time.h>
#include <stdio.h>

#include "sal.h"
#include "config.h"
#include "image.h"
#include "window.h"
#include "aquarium.h"

#define FUZZY_IMAGES 18
#define TWENTY 12
#define QUARTER 13
#define HALF 14
#define OCLOCK 15
#define PAST 16
#define TO 17

static char *fuzzy_image_names[] = {
	"fuzzy-clock/One.png",
	"fuzzy-clock/Two.png",
	"fuzzy-clock/Three.png",
	"fuzzy-clock/Four.png",
	"fuzzy-clock/Five.png",
	"fuzzy-clock/Six.png",
	"fuzzy-clock/Seven.png",
	"fuzzy-clock/Eight.png",
	"fuzzy-clock/Nine.png",
	"fuzzy-clock/Ten.png",
	"fuzzy-clock/Eleven.png",
	"fuzzy-clock/Twelve.png",
	"fuzzy-clock/Twenty.png",
	"fuzzy-clock/Quarter.png",
	"fuzzy-clock/Half.png",
	"fuzzy-clock/Oclock.png",
	"fuzzy-clock/Past.png",
	"fuzzy-clock/To.png",
	NULL
};

static Imlib_Image fuzzy_images[FUZZY_IMAGES];

void fuzzy_clock_update(struct aquarium *aquarium)
{
        time_t now;
        struct tm *mt;
        int th = 0;
        int i;
        int mi;
        int x, y, cy = 0;
        int img[5] = {-1, -1, -1, -1, -1};


        if(aquarium->fuzzy_clock == AL_NO)
                return;

	now = time(NULL);
	mt = localtime(&now);

        if (mt->tm_hour == 0 || mt->tm_hour == 12)
                img[2] = 12 - 1;
        else
                img[2] = (mt->tm_hour % 12) - 1;

        mi = mt->tm_min;

        if (mi >= 30) {
                img[1] = TO;
                img[2]++;
                img[2] = img[2] % 12;
        } else
                img[1] = PAST;

        mi = mi / 5;

        switch(mi) {
        case 0: /* H o'clock*/
                img[0] = img[2];
                img[1] = OCLOCK;
                img[2] = -1;
                break;
        case 6:
                img[0] = HALF;
                img[1] = PAST;
                img[2]--;
                if (img[2] < 0)
                        img[2] = 11;
                break;
        case 1: /* Five to/past H */
        case 11:
                img[0] = 5 - 1;
                break;
        case 2: /* Ten to/past H */
        case 10:
                img[0] = 10 - 1;
                break;
        case 3:/* Quarter to/past H */
        case 9:
                img[0] = QUARTER;
                break;
        case 4: /* Twenty to/past H */
        case 8:
                img[0] = TWENTY;
                break;
        case 5: /* Twenty five to/past H */
        case 7:
                img[0] = TWENTY;
                img[3] = img[2];
                img[2] = img[1];
                img[1] = 5 - 1;
                break;
        default:
                break;
        }

        for (i = 0 ; img[i] != -1; i++) {
                imlib_context_set_image(fuzzy_images[img[i]]);
                th += imlib_image_get_height();
        }

        for (i = 0 ; img[i] != -1; i++) {
                imlib_context_set_image(fuzzy_images[img[i]]);
                aquarium_transform(aquarium->fuzzy_clock,
                                   imlib_image_get_width(), th,
                                   &x, &y);
                window_draw_blend((unsigned char *)imlib_image_get_data_for_reading_only(),
                                  0, 0,
                                  imlib_image_get_width(),
                                  imlib_image_get_height(),
                                  x, y + cy,
                                  128);
                cy += imlib_image_get_height();
        }
}

void fuzzy_clock_init(struct aquarium *aquarium)
{
        int i;

        if(aquarium->fuzzy_clock == AL_NO)
                return;

        for (i = 0; i < FUZZY_IMAGES; i++) {
                fuzzy_images[i] = image_load(fuzzy_image_names[i]);
                image_change_color(fuzzy_images[i],
                                   RED(aquarium->fuzzy_clock_color),
                                   GREEN(aquarium->fuzzy_clock_color),
                                   BLUE(aquarium->fuzzy_clock_color));
        }
}
