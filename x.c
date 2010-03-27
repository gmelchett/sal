#include <stdio.h>

#include <sys/ipc.h>
#include <sys/shm.h>
#include <X11/Xlib.h>
#include <X11/extensions/XShm.h>

#include <X11/Xutil.h>
#include <Imlib2.h>
#include "background.h"

static Display *display;
static int depth;
static Window win;
static XImage *image;
static int background_w, background_h;
static int background_rs;
static XShmSegmentInfo shm_img;
static GC gc;


void x_draw(unsigned char *source,
            int sx, int sy, int sw, int sh,
            int tx, int ty,
            int rs_source, int alpha_source, int ignore_alpha)
{

    int dw, di, dh, ds;
    int w, h;
    int th = background_h, tw = background_w;
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

    printf("Drawing %d-%d, %d-%d\n", di, dw, ds, dh);

    for (h = ds; h < dh; h++)
    {
	/* offset to beginning of current row */
	target_ypos = (h + ty) * background_rs;
	source_ypos = (h + sy) * rs_source;

	for (w = di; w < dw; w++)
	{
	    target_pos = target_ypos + (depth / 8)*(w + tx);
	    source_pos = source_ypos + (alpha_source + 3)*(w + sx);

	    if (alpha_source && !ignore_alpha)
	    {
		if(source[source_pos+3]!=0)
		{
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

        XShmPutImage(display, win, gc, image, 0, 0, 5, 5, background_w, background_h, False);
        XSync(display, False );
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
        background_rs = width * depth / 8;

        printf("%d %d %d\n", background_w, background_h, background_rs);

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
