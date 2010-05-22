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

#include "config.h"
#include "sal.h"
#include "aquarium.h"

struct window {
        Display *display;
        int depth;
        Window win;
        XImage *image;
        XShmSegmentInfo shm_img;
        GC gc;
        struct aquarium *aquarium;
};

static struct window window;

void window_draw(unsigned char *source,
                 int sx, int sy, int sw, int sh,
                 int tx, int ty,
                 bool alpha_source)
{

        int dw, di, dh, ds;
        int w, h;
        int th = window.aquarium->window_h;
        int tw = window.aquarium->window_w;
        int source_ypos, source_pos, target_ypos, target_pos;
        unsigned char *target = (unsigned char *)window.image->data;

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
                target_ypos = (h + ty) * window.image->bytes_per_line;
                source_ypos = (h + sy) * sw * 4;

                for (w = di; w < dw; w++) {
                        target_pos = target_ypos + window.depth * (w + tx);
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


void window_draw_blend(unsigned char *source,
                       int sx, int sy, int sw, int sh,
                       int tx, int ty,
                       int alpha)
{

        int dw, di, dh, ds;
        int w, h;
        int th = window.aquarium->window_h;
        int tw = window.aquarium->window_w;
        int source_ypos, source_pos, target_ypos, target_pos;
        unsigned char *target = (unsigned char *)window.image->data;

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
                target_ypos = (h + ty) * window.image->bytes_per_line;
                source_ypos = (h + sy) * sw * 4;

                for (w = di; w < dw; w++) {
                        target_pos = target_ypos + window.depth * (w + tx);
                        source_pos = source_ypos + 4 * (w + sx);

                        if(source[source_pos + 3] == 0)
                                continue;

                        target[target_pos] = (
                                              (int)(256 - alpha) *
                                              (int)target[target_pos] +
                                              (int)alpha *
                                              (int)source[source_pos]
                                              ) >> 8;

                        target[target_pos + 1] = (
                                                  (int)(256 - alpha) *
                                                  (int)target[target_pos + 1] +
                                                  (int)alpha *
                                                  (int)source[source_pos + 1]
                                                  ) >> 8;

                        target[target_pos + 2] = (
                                                  (int)(256 - alpha) *
                                                  (int)target[target_pos + 2] +
                                                  (int)alpha *
                                                  (int)source[source_pos + 2]
                                                  ) >> 8;
                }
        }
}


static void putpixel(int x, int y, float alpha, int linewidth, int colour)
{

        int dx, dy;
        int pos, a;

        unsigned char *target = (unsigned char *)window.image->data;

        if(x < 0 || x > window.aquarium->window_w || y < 0 || y > window.aquarium->window_h)
                return;

        a = (int)(alpha * 256.0 + 0.5);

        for (dx = x; dx < x + linewidth; dx++) {
                for (dy = y; dy < y + linewidth; dy++) {
                        pos = dy * window.image->bytes_per_line  + dx * 4;

                        target[pos + 0] = (((colour >> 16) & 0xff) * a + (target[pos + 0]) * (256 - a)) >> 8;
                        target[pos + 1] = (((colour >>  8) & 0xff) * a + (target[pos + 1]) * (256 - a)) >> 8;
                        target[pos + 2] = (((colour >>  0) & 0xff) * a + (target[pos + 2]) * (256 - a)) >> 8;
                }
        }
}

/* draw antialiased line from (x1, y1) to (x2, y2), with width linewidth
 * colour is an int like 0xRRGGBB */
void window_draw_line(int x1, int y1, int x2, int y2, int linewidth, int colour, int shaded)
{

        int dx,dy;
        int error, sign, tmp;
        float ipix;
        int step = linewidth;

        dx = abs(x1 - x2);
        dy = abs(y1 - y2);

        if (dx >= dy) {
                if (x1 > x2) {
                        tmp = x1;
                        x1 = x2;
                        x2 = tmp;
                        tmp = y1;
                        y1 = y2;
                        y2 = tmp;
                }
                error = dx / 2;
                if (y2 > y1)
                        sign = step;
                else
                        sign = -step;

                putpixel(x1, y1, 1.0, linewidth, colour);

                while (x1 < x2) {
                        if ((error -= dy) < 0) {
                                y1 += sign;
                                error += dx;
                        }
                        x1 += step;
                        ipix = (float) error / dx;

                        if (sign == step)
                                ipix = 1.0 - ipix;

                        if(shaded){
                                putpixel(x1, y1 - step, (1.0 - ipix), linewidth, colour);
                                putpixel(x1, y1 + step, ipix, linewidth, colour);
                        }
                        putpixel(x1, y1, 1.0, linewidth, colour);

                }
                putpixel(x2, y2, 1.0, linewidth, colour);
        } else {
                if (y1 > y2) {
                        tmp = x1;
                        x1 = x2;
                        x2 = tmp;
                        tmp = y1;
                        y1 = y2;
                        y2 = tmp;
                }
                error = dy / 2;

                if (x2 > x1)
                        sign = step;
                else
                        sign = -step;

                putpixel(x1, y1, 1.0, linewidth, colour);

                while (y1 < y2) {
                        if ((error -= dx) < 0) {
                                x1 += sign;
                                error += dy;
                        }
                        y1 += step;
                        ipix = (float) error / dy;

                        if (sign == step)
                                ipix = 1.0 - ipix;
                        if(shaded){
                                putpixel(x1 - step, y1, (1.0 - ipix), linewidth, colour);
                                putpixel(x1 + step, y1, ipix, linewidth, colour);
                        }

                        putpixel(x1, y1, 1.0, linewidth, colour);
                }
                putpixel(x2, y2, 1.0, linewidth, colour);
        }
}

void window_update(void)
{

        XShmPutImage(window.display, window.win, window.gc, window.image, 0, 0,
                     BORDER_WIDTH, BORDER_WIDTH,
                     window.aquarium->w,
                     window.aquarium->h,
                     False);
        XSync(window.display, False);
}


void window_close(void)
{
	XDestroyImage(window.image);
	XCloseDisplay(window.display);
}

static void background_set(void)
{
        unsigned char *bg;
        int x, y, ypos;
        Imlib_Image imlib_image;
        Pixmap pixmap, mask;

        bg = malloc(window.aquarium->window_w * window.aquarium->window_h * 4);

        for (y = 0; y < window.aquarium->window_h ; y++) {
                ypos = y * 4 * window.aquarium->window_w;

                for (x = 0; x < window.aquarium->window_w * 4 ; x += 4) {

                        if ((y < (BORDER_WIDTH - 1) ||
                             x < ((BORDER_WIDTH - 1) * 4) ||
                             x > (4 * (window.aquarium->window_w - BORDER_WIDTH)) ||
                             y > (window.aquarium->window_h - BORDER_WIDTH))) {
                                bg[ypos + x + 0] = 0x0;
                                bg[ypos + x + 1] = 0x0;
                                bg[ypos + x + 2] = 0x0;
                                bg[ypos + x + 3] = 0x0;
                                continue;
                        }

                        if ((y == (BORDER_WIDTH - 1) &&
                             (x > 4 * (BORDER_WIDTH - 2) &&
                              x < BORDER_WIDTH * window.aquarium->window_w - (BORDER_WIDTH - 1) * 4)) ||
                            (x == (BORDER_WIDTH - 1) * 4 &&
                             (y > (BORDER_WIDTH - 2) && y < window.aquarium->window_h - (BORDER_WIDTH - 1)))) {

                                bg[ypos + x + 0] = 0x0;
                                bg[ypos + x + 1] = 0x0;
                                bg[ypos + x + 2] = 0x0;
                                bg[ypos + x + 3] = 0xff;
                                continue;
                        }

                        if ((y == (window.aquarium->window_h - BORDER_WIDTH) &&
                             (x > 4 * (BORDER_WIDTH - 2) &&
                              x < 4 * window.aquarium->window_w - (BORDER_WIDTH - 1) * 4)) ||
                            (x == (4 * (window.aquarium->window_w - BORDER_WIDTH)) &&
                             (y > (BORDER_WIDTH - 2) &&
                              y < window.aquarium->window_h - (BORDER_WIDTH - 1)))) {
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


        imlib_image = imlib_create_image_using_data(window.aquarium->window_w, window.aquarium->window_h, (DATA32*)bg);
        imlib_context_set_image(imlib_image);
        imlib_image_set_has_alpha(1);
        imlib_render_pixmaps_for_whole_image(&pixmap, &mask);

        XShapeCombineMask(window.display, window.win, ShapeBounding, 0, 0, mask, ShapeSet);
        XSetWindowBackgroundPixmap(window.display, window.win, pixmap);

        imlib_free_pixmap_and_mask(pixmap);
        imlib_free_image_and_decache();

        free(bg);

}


int window_create(void)
{
        Colormap cm;
        Visual *visual;
        static const char *window_title = "Sherman's Aquarium";
        XSizeHints size_hints;

        window.aquarium = aquarium_get();

	window.display = XOpenDisplay(NULL);
        window.depth   = (DefaultDepth(window.display,   DefaultScreen(window.display))/8) + 1;
        cm             = DefaultColormap(window.display, DefaultScreen(window.display));
        visual         = DefaultVisual(window.display,   DefaultScreen(window.display));
	window.gc      = DefaultGC(window.display,       DefaultScreen(window.display));
        window.aquarium->display = window.display;


        window.win = XCreateSimpleWindow(window.display,
                                         RootWindow(window.display, DefaultScreen(window.display)),
                                         0, 0,
                                         window.aquarium->window_w, window.aquarium->window_h,
                                         0, 0, 0);

	window.image = XShmCreateImage(window.display, visual,
                                       DefaultDepth(window.display, DefaultScreen(window.display)),
                                       ZPixmap, NULL,
                                       &window.shm_img, window.aquarium->window_w, window.aquarium->window_h);

	window.shm_img.shmid = shmget(IPC_PRIVATE, window.image->bytes_per_line * window.image->height, IPC_CREAT|0600);
	window.shm_img.readOnly = 0;
	window.shm_img.shmaddr = shmat(window.shm_img.shmid, 0, 0);
	window.image->data = window.shm_img.shmaddr;
	XShmAttach(window.display, &window.shm_img);
	XSync(window.display, False);

	shmctl(window.shm_img.shmid, IPC_RMID, 0);

	XSelectInput(window.display, window.win,
                     KeyPressMask | ButtonPressMask |
                     ExposureMask | ButtonReleaseMask |
                     EnterWindowMask | LeaveWindowMask);

        imlib_context_set_dither(1);
        imlib_set_cache_size(2048 * 1024);
        imlib_context_set_display(window.display);
        imlib_context_set_visual(visual);
        imlib_context_set_colormap(cm);
        imlib_context_set_drawable(window.win);

        size_hints.min_width = size_hints.max_width = window.aquarium->window_w;
        size_hints.min_height = size_hints.max_height = window.aquarium->window_h;
        size_hints.flags = PMaxSize|PMinSize;

        XSetWMNormalHints(window.display, window.win, &size_hints);


        XmbSetWMProperties(window.display, window.win, window_title, window_title,
                           NULL, 0, NULL, NULL, NULL);

        background_set();

	XRaiseWindow(window.display, window.win);
	XMapWindow(window.display, window.win);

	XFlush(window.display);

        return 0;
}
