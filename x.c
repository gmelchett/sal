#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include <sys/ipc.h>
#include <sys/shm.h>
#include <X11/Xlib.h>
#include <X11/extensions/XShm.h>
#include <X11/extensions/shape.h>
#include <X11/Xutil.h>
#include <Imlib2.h>

#include "sll.h"

static Display *display;
static int depth;
static Window win;
static XImage *image;
static int background_w, background_h;
static int background_rs;
static XShmSegmentInfo shm_img;
static GC gc;


static void background_set(Display *display, Window win, int bw, int bh)
{
        unsigned char *bg;
        int x, y, ypos;
        Imlib_Image imlib_image;
        Pixmap pixmap, mask;

        bg = malloc(bw * bh * 4);

        for (y = 0; y < bh ; y++) {
                ypos = y * 4 * bw;

                for (x = 0; x < bw * 4 ; x += 4) {

                        if ((y < (BORDER_WIDTH - 1) ||
                             x < ((BORDER_WIDTH - 1) * 4) ||
                             x > (4 * (bw - BORDER_WIDTH)) ||
                             y > (bh - BORDER_WIDTH))) {
                                bg[ypos + x + 0] = 0x0;
                                bg[ypos + x + 1] = 0x0;
                                bg[ypos + x + 2] = 0x0;
                                bg[ypos + x + 3] = 0x0;
                                continue;
                        }

                        if ((y == (BORDER_WIDTH - 1) &&
                             (x > 4 * (BORDER_WIDTH - 2) &&
                              x < BORDER_WIDTH * bw - (BORDER_WIDTH - 1) * 4)) ||
                            (x == (BORDER_WIDTH - 1) * 4 &&
                             (y > (BORDER_WIDTH - 2) && y < bh - (BORDER_WIDTH - 1)))) {

                                bg[ypos + x + 0] = 0x0;
                                bg[ypos + x + 1] = 0x0;
                                bg[ypos + x + 2] = 0x0;
                                bg[ypos + x + 3] = 0xff;
                                continue;
                        }

                        if ((y == (bh - BORDER_WIDTH) &&
                             (x > 4 * (BORDER_WIDTH - 2) &&
                              x < 4 * bw - (BORDER_WIDTH - 1) * 4)) ||
                            (x == (4 * (bw - BORDER_WIDTH)) &&
                             (y > (BORDER_WIDTH - 2) &&
                              y < bh - (BORDER_WIDTH - 1)))) {
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

        imlib_free_pixmap_and_mask(pixmap);
        imlib_free_image_and_decache();

        free(bg);

}

void x_draw(unsigned char *source,
            int sx, int sy, int sw, int sh,
            int tx, int ty,
            bool alpha_source)
{

        int dw, di, dh, ds;
        int w, h;
        int th = background_h;
        int tw = background_w;
        int source_ypos, source_pos, target_ypos, target_pos;
        unsigned char *target = (unsigned char *)image->data;

        /* completely off the screen, don't bother drawing */
        if ((ty < -(sh)) || (ty > th) || (tx > tw) || (tx < -(sw)))
                return;

        /* do clipping for top side */
        ds = 0;
        if (ty < 0)
                ds = -(ty);

        /* do clipping for bottom side */
        dh = sh;
        if ((ty + sh) > th)
                dh = th - ty;

        /* do clipping for right side */
        dw = sw;
        if (tx > (tw - sw))
                dw = sw - (tx - (tw - sw));

        /* do clipping for left side */
        di = 0;
        if (tx < 0)
                di = -(tx);

        for (h = ds; h < dh; h++) {
                /* offset to beginning of current row */
                target_ypos = (h + ty) * background_rs;
                source_ypos = (h + sy) * sw * 4;

                for (w = di; w < dw; w++) {
                        target_pos = target_ypos + ((depth / 8) + 1) * (w + tx);
                        source_pos = source_ypos + 4 * (w + sx);

                        if (alpha_source) {
                                if(source[source_pos + 3] != 0) {
                                        target[target_pos] = (
                                                              (int)(256 - source[source_pos + 3]) *
                                                              (int)target[target_pos] +
                                                              (int)source[source_pos + 3] *
                                                              (int)source[source_pos]
                                                              ) >> 8;

                                        target[target_pos + 1] = (
                                                                  (int)(256 - source[source_pos + 3]) *
                                                                  (int)target[target_pos + 1] +
                                                                  (int)source[source_pos + 3] *
                                                                  (int)source[source_pos + 1]
                                                                  ) >> 8;

                                        target[target_pos + 2] = (
                                                                  (int)(256 - source[source_pos + 3]) *
                                                                  (int)target[target_pos + 2] +
                                                                  (int)source[source_pos + 3] *
                                                                  (int)source[source_pos + 2]
                                                                  ) >> 8;
                                }
                        } else {
                                target[target_pos + 0] = source[source_pos + 0];
                                target[target_pos + 1] = source[source_pos + 1];
                                target[target_pos + 2] = source[source_pos + 2];
                        }
                }
        }
}


void x_update(void)
{

        XShmPutImage(display, win, gc, image, 0, 0,
                     BORDER_WIDTH, BORDER_WIDTH,
                     background_w - BORDER_WIDTH,
                     background_h - BORDER_WIDTH,
                     False);
        XSync(display, False);
}


void x_close_window(void)
{
	XDestroyImage(image);
	XCloseDisplay(display);
}

int x_create_window(int width, int height)
{
        Colormap cm;
        Visual *visual;
	display = XOpenDisplay(NULL);
        depth   = DefaultDepth(display,    DefaultScreen(display));
        cm      = DefaultColormap(display, DefaultScreen(display));
        visual  = DefaultVisual(display,   DefaultScreen(display));
	gc      = DefaultGC(display,       DefaultScreen(display));


        win = XCreateSimpleWindow(display, RootWindow(display, DefaultScreen(display)),
                                  0, 0, width, height, 0, 0, 0);

        background_h = width;
        background_w = height;
        background_rs = width * (depth + 8) / 8;

	image = XShmCreateImage(display, visual, depth, ZPixmap, NULL,
                                &shm_img, background_w, background_h);

	shm_img.shmid = shmget(IPC_PRIVATE, image->bytes_per_line * image->height, IPC_CREAT|0600);
	shm_img.readOnly = 0;
	shm_img.shmaddr = shmat(shm_img.shmid, 0, 0);
	image->data = shm_img.shmaddr;
	XShmAttach(display, &shm_img);
	XSync(display, False);

	shmctl(shm_img.shmid, IPC_RMID, 0);

	XSelectInput(display, win, KeyPressMask | ButtonPressMask);

        imlib_context_set_dither(1);
        imlib_set_cache_size(2048 * 1024);
        imlib_context_set_display(display);
        imlib_context_set_visual(visual);
        imlib_context_set_colormap(cm);
        imlib_context_set_drawable(win);

        background_set(display, win, background_w, background_h);

	XRaiseWindow(display, win);
	XMapWindow(display, win);

	XFlush(display);

        return 0;
}
