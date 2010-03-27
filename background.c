
#include <stdlib.h>
#include <X11/Xlib.h>
#include <X11/extensions/shape.h>
#include <Imlib2.h>

#include "x.h"

void background_set(Display *display, Window win, int bw, int bh)
{
        unsigned char *bg;
        int x, y, ypos;
        Imlib_Image imlib_image;
        Pixmap pixmap, mask;

        bg = malloc(bw * bh * 4);

        for (y = 0; y < bh ; y++) {
                ypos = y * 4 * bw; //background_rs;

                for (x = 0; x < bw * 4 ; x += 4) {

                        if ((y < 3 || x < (3 * 4) || x > (4*(bw - 4)) ||
                            y > (bh - 4))) {
                                bg[ypos + x + 0] = 0x0;
                                bg[ypos + x + 1] = 0x0;
                                bg[ypos + x + 2] = 0x0;
                                bg[ypos + x + 3] = 0x0;
                                continue;
                        }

                        if ((y == 3 && (x > 4 * 2 && x < 4 * bw - 3 * 4)) ||
                            (x == 3 * 4 &&(y > 2 && y < bh - 3))) {

                                bg[ypos + x + 0] = 0x0;
                                bg[ypos + x + 1] = 0x0;
                                bg[ypos + x + 2] = 0x0;
                                bg[ypos + x + 3] = 0xff;
                                continue;
                        }

                        if ((y == (bh - 4) && (x > 4 * 2 && x < 4 * bw - 3 * 4)) ||
                            (x == (4 * (bw - 4)) && (y > 2 && y < bh - 3))) {
                                bg[ypos + x + 0] = 0xC6;
                                bg[ypos + x + 1] = 0xBA;
                                bg[ypos + x + 2] = 0xAB;
                                bg[ypos + x + 3] = 0xFF;
                                continue;
                        }
                        bg[ypos + x + 0] = 0x87;
                        bg[ypos + x + 1] = 0x5f;
                        bg[ypos + x + 2] = 0x37;
                        bg[ypos + x + 3] = 0xFF;
                }
	}


        imlib_image = imlib_create_image_using_data(bw, bh, (DATA32*)bg);
        imlib_context_set_image(imlib_image);
        imlib_image_set_has_alpha(1);
        imlib_render_pixmaps_for_whole_image(&pixmap, &mask);

        XShapeCombineMask(display, win, ShapeBounding, 0, 0, mask, ShapeSet);
        XSetWindowBackgroundPixmap(display, win, pixmap);

        imlib_free_image_and_decache();

        free(bg);

}

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

        x_draw((unsigned char *)imlib_image_get_data(),
               0,0,
               bw,bh,
               4,4,
               bw*(3+(int)imlib_image_has_alpha()),
               (int)imlib_image_has_alpha(), 1);
        x_update();

}
