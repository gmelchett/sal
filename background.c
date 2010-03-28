
#include <stdlib.h>
#include <stdbool.h>
#include <X11/Xlib.h>


#include <Imlib2.h>

#include "x.h"

void foreground_load(int bw, int bh)
{
        Imlib_Image image, image2;
        DATA32 *d;
        image = imlib_load_image("images/water.png");
        imlib_context_set_image(image);

        image2 = imlib_create_cropped_scaled_image(0, 0,
                                                   imlib_image_get_width(),
                                                   imlib_image_get_height(),
                                                   bw, bh);
        imlib_free_image_and_decache();
        imlib_context_set_image(image2);

        x_draw((unsigned char *)imlib_image_get_data_for_reading_only(),
               0,0,
               bw, bh,
               0,0,
               (bool)imlib_image_has_alpha());
        x_update();

        imlib_free_image();

}
