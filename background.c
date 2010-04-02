
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <X11/Xlib.h>
#include <Imlib2.h>

#include "x.h"
#include "sll.h"

#define IMAGE_PATH "images/"

static Imlib_Image foreground;



static char *plants[] = {
        "bottom/plant1.png",
        "bottom/plant2.png",
        "bottom/plant3.png",
        "bottom/plant4.png",
        "bottom/plant5.png",
        "bottom/plant6.png",
        "bottom/plant7.png",
        "bottom/plant8.png",
        "bottom/plant9.png",
        "bottom/smallstone1.png",
        "bottom/smallstone2.png",
        "bottom/stone1.png",
        "bottom/stone2.png",
        "bottom/stone3.png",
        "bottom/weirdplant1.png",
        "bottom/weirdplant2.png",
        "bottom/octo.png",
        "bottom/bigplant.png",
        NULL};

static char *bottoms[] = {
        "bottom/bottom1.png",
        "bottom/bottom2.png",
        NULL};

#define HAVE_BOTTOM 1
#define MAX_LAPS 6


static Imlib_Image load_image(char *fname, int *sw, int *sh, bool random_rescale)
{
        int size;
        Imlib_Image orig, scaled;
        char path[1024];

        snprintf(path, sizeof(path), "%s/%s", IMAGE_PATH, fname);

        orig = imlib_load_image(path);

        imlib_context_set_image(orig);

        if(random() % 2)
                imlib_image_flip_horizontal();

        if(random_rescale) {

                size = random() % 180 + 20;
                *sw = imlib_image_get_width() * size / 100;
                *sh = imlib_image_get_height() * size / 100;

                scaled = imlib_create_cropped_scaled_image(0, 0,
                                                           imlib_image_get_width(),
                                                           imlib_image_get_height(),
                                                           *sw, *sh);
                imlib_free_image_and_decache();
                imlib_context_set_image(scaled);
                return scaled;
        }
        return orig;
}

struct plant_loc {
        int x, y, w, h;
};



static void bottom_animals_and_plants(int bw, int bh, int sand_height)
{
        Imlib_Image plant;
        int num_p;
        int plant_top;
        int i, j;
        int num_plants;
        int sh, sw;
        struct plant_loc plant_loc[MAX_PLANTS];

        for (num_p = 0; plants[num_p] != NULL; num_p++);

        plant_top = sand_height * 75 / 100;

        if(plant_top < 10)
                num_plants = 0;
        else
                num_plants = random() % MAX_PLANTS;

        for (i = 0 ; i < num_plants; i++) {
                int laps = 0;
                bool overlaps = false;
                int x, y;

                plant = load_image(plants[random() % num_p], &sw, &sh, true);

                do {
                        x = random() % bw;
                        y = random() % plant_top;
                        overlaps = false;
                        for(j = 0; j < i; j ++)
                                if((x >= plant_loc[j].x && x <= (plant_loc[j].x + plant_loc[j].w) &&
                                    y >= plant_loc[j].y && y <= (plant_loc[j].y + plant_loc[j].h)) ||
                                   ((x + sw) >= plant_loc[j].x && (x + sw) <= (plant_loc[j].x + plant_loc[j].w) &&
                                    (y + sh) >= plant_loc[j].y && (y + sh) <= (plant_loc[j].y + plant_loc[j].h))) {
                                        overlaps = true;
                                        laps++;
                                        break;
                                }
                } while(overlaps && laps < MAX_LAPS);

                plant_loc[i].x = x;
                plant_loc[i].y = y;
                plant_loc[i].w = sw;
                plant_loc[i].h = sh;

                if(laps == MAX_LAPS) {
                        imlib_free_image();
                        break;
                }

                imlib_context_set_image(foreground);
                imlib_blend_image_onto_image(plant,
                                             true,
                                             0, 0,
                                             sw, sh,
                                             x, bh - plant_top + y,
                                             sw, sh);
                imlib_context_set_image(plant);
                imlib_free_image();
        }
}

static int bottom_create(int bw, int bh)
{
        Imlib_Image bottom;
        int num_b;
        int start_x, end_y;
        int w, sx, dx, px, sw, sh;

        if(!HAVE_BOTTOM)
                return 0;

        for (num_b = 0; bottoms[num_b] != NULL; num_b++);

        bottom = load_image(bottoms[random() % num_b], &sw, &sh, true);

        start_x = random() % imlib_image_get_width();
        end_y = random() % (imlib_image_get_height() - imlib_image_get_height() / 5);

        imlib_context_set_image(foreground);

        for (dx = -start_x; dx < bw; dx += sw) {
                if (dx < 0) {
                        sx = start_x;
                        w = sw - start_x;
                        px = 0;
                } else {
                        sx = 0;
                        w = sw;
                        px = dx;
                }
                imlib_blend_image_onto_image(bottom,
                                             true,
                                             sx, 0,
                                             w, end_y,
                                             px, bh - end_y,
                                             w, end_y);
        }
        imlib_context_set_image(bottom);
        imlib_free_image_and_decache();
        return end_y;
}


void foreground_load(int bw, int bh)
{
        int sand_height;


        if(0) {
                Imlib_Image image;
                char fname[1024];
                snprintf(fname, sizeof(fname), "%s/%s", IMAGE_PATH, "water.png");

                image = imlib_load_image(fname);
                imlib_context_set_image(image);

                foreground = imlib_create_cropped_scaled_image(0, 0,
                                                               imlib_image_get_width(),
                                                               imlib_image_get_height(),
                                                               bw, bh);
                imlib_free_image_and_decache();
        } else {
                unsigned char *buff;
                int x, y, t;
                float d1, d2, d3;

                foreground = imlib_create_image(bw, bh);
                imlib_context_set_image(foreground);
                imlib_image_set_has_alpha(0);
                buff = (unsigned char *)imlib_image_get_data();

                d1 = (float)(RED_END - RED_START) / (float) bh;
                d2 = (float)(GREEN_END - GREEN_START) / (float) bh;
                d3 = (float)(BLUE_END - BLUE_START) / (float) bh;

                for (y = 0; y < bh; y++) {
                        for (x = 0; x < bw; x++) {
                                t = (bw * 4) * y + x * 4;
                                buff[t + 0] = (unsigned char)((float)BLUE_START + ((float) y * d3));
                                buff[t + 1] = (unsigned char)((float)GREEN_START + ((float) y * d2));
                                buff[t + 2] = (unsigned char)((float)RED_START + ((float) y * d1));
                        }
                }
        }


        sand_height = bottom_create(bw, bh);
        if(sand_height)
                bottom_animals_and_plants(bw, bh, sand_height);
}


void foreground_update(void)
{

        imlib_context_set_image(foreground);
        x_draw((unsigned char *)imlib_image_get_data_for_reading_only(),
               0,0,
               imlib_image_get_width(), imlib_image_get_height(),
               0,0,
               (bool)imlib_image_has_alpha());

}

#define BUBBLES_FRAMES 5
struct bubbles
{
        float x, y;
        float speed;
};
static int bubbles_nr;
static Imlib_Image bubbles_image;
static struct bubbles bubbles_loc[MAX_BUBBLES];
static int bubbles_h, bubbles_w;
static int bubbles_dh;

static void bubble_random(struct bubbles *b, bool random_y)
{
        b->x = (float)(random() % bubbles_w);
        if (random_y)
                b->y = (float) (random() % bubbles_h);
        else
                b->y = (float) bubbles_h + 5.0;
        b->speed = (float)((random() % MAX_BUBBLE_SPEED) + 1.0)/ (float)MAX_BUBBLE_SPEED;

}

void bubbles_init(int bw, int bh)
{
        int i;
        bubbles_image = load_image("bubbles.png", 0, 0, False);

        imlib_context_set_image(bubbles_image);

        bubbles_nr = random() % MAX_BUBBLES;

        bubbles_h = bh;
        bubbles_w = bw;
        bubbles_dh = imlib_image_get_height() / BUBBLES_FRAMES;

        for (i = 0; i < bubbles_nr; i++)
                bubble_random(&bubbles_loc[i], True);
}


void bubbles_update(void)
{
        int i;

        imlib_context_set_image(bubbles_image);

        for (i = 0; i < bubbles_nr; i++) {

                bubbles_loc[i].y -= bubbles_loc[i].speed;

                if(bubbles_loc[i].y < -(imlib_image_get_height() / BUBBLES_FRAMES)) {
                        bubble_random(&bubbles_loc[i], False);
                        continue;
                }


                x_draw_blend((unsigned char *)imlib_image_get_data_for_reading_only(),
                             0, ((BUBBLES_FRAMES - 1) * (int)bubbles_loc[i].y / bubbles_h) * bubbles_dh,
                             imlib_image_get_width(),
                             bubbles_dh,
                             (int)bubbles_loc[i].x, (int)bubbles_loc[i].y,
                             128);
        }

}


#define FISH1 0
#define FISH2 1
#define FISH3 2
#define FISH4 3
#define FISH5 4
#define FISH6 5
#define SQUID 6
#define SWORDFISH 7
#define BLOWFISH 8
#define ERNEST 9
#define HUNTER 10
#define LORI 11
#define PREY 12
#define SHERMAN 13
#define FILLMORE 14
#define BDWELLER 15
#define HAWTHORNE 16
#define MEGAN 17

/* Default size for fish */
#define DEFAULT_SCALE 50

#define DEFAULT_SPEED 100

#define FIX_FISH 10
#define MAX_FISH 15

/* The number of fish you have graphic for */
#define NUMOFFISHTYPES 18


struct fish_animation {
    char *file;
    int pics;
    int frames;
    int *animation;
    float *speed;
};

struct fish {
    float tx;			/* current x position */
    int y;			/* current y position */
    int travel;			/* how far to move beyond the screen */
    int rev;			/* going left or right? */
    int frame;			/* current animation frame */
    float delay;		/* how quick we swap frames */
    int updown;			/* Was last moment up or down? - Larger chance to do it again. */
    int type;			/* Kind of fish */

    int w, h;
    struct fish_animation *animation;
    float speed_mul;
    float fast_frame_change;
    int is_dead;			/* Is the fish alive? I.E, eaten prey or punched blowfish? */
};


static float fish1_speed[]        = { 1.0, 1.0, 1.0, 1.0 };
static float fish2_speed[]        = { 0.6, 0.6, 0.6, 0.6 };
static float fish3_speed[]        = { 1.1, 1.1, 1.1, 1.1 };
static float fish4_speed[]        = { 1.0, 1.0, 1.0, 1.0 };
static float fish5_speed[]        = { 0.8, 0.8, 0.7, 0.5, 0.7, 0.8, 0.8 };
static float fish6_speed[]        = { 1.2, 1.2, 1.2, 1.2 };
static float squid_speed[]        = { 0.1, 3.0, 3.0, 3.0, 3.0, 1.5, 1.0 };
static float swordfish_speed[]    = { 1.4, 1.4, 1.4, 1.4 };
static float blowfish_speed[]     = { 0.6, 0.6, 0.6, 0.6 };
static float ernest_speed[]       = { 0.8, 0.8, 0.8, 0.8 };
static float hunter_speed[]       = { 1.1, 1.1, 1.1, 1.1 };
static float lori_speed[]         = { 0.8, 0.8, 0.8, 0.8 };
static float prey_speed[]         = { 1.3, 1.3, 1.3, 1.3 };
static float sherman_speed[]      = { 1.5, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.5, 1.75, 2.25, 2.5, 2.5, 2.5 };
static float fillmore_speed[]     = { 0.7, 0.7, 0.7, 0.7, 0.8, 0.9, 1.0, 1.0, 1.0, 0.7, 0.5, 0.5, 0.5, 0.6, 0.7 };
static float prey_hunter_speed[]  = { 1.5, 1.5, 1.5, 1.5, 1.5, 1.5, 1.5 };
static float blowup_speed[]       = { 0.1, 0.1 };
static float bdweller_speed[]     = { 0.01, 0.01, 0.01, 0.01}
;
static int normal_animation[]      = { 0, 1, 2, 1 };
static int sherman_animation[]     = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 10, 10, 0 };
static int fillmore_animation[]    = { 0, 1, 2, 3, 4, 5, 6, 6, 6, 7, 8, 9, 10, 11,  0 };
static int squid_animation[]       = { 0, 1, 1, 1, 1, 1, 2};
static int fish5_animation[]       = { 0, 0, 1, 2, 1, 0, 0};
static int prey_hunter_animation[] = { 0, 1, 2, 3, 4, 5, 6};


#define FISH(file, pics, animation, speed) {file, pics, sizeof(animation)/sizeof(int), animation, speed}

static struct fish_animation fish_animation[NUMOFFISHTYPES] = {
	FISH("sherman/fish1.png",     3, normal_animation,   fish1_speed     ),
	FISH("sherman/fish2.png",     3, normal_animation,   fish2_speed     ),
	FISH("sherman/fish3.png",     3, normal_animation,   fish3_speed     ),
	FISH("sherman/fish4.png",     3, normal_animation,   fish4_speed     ),
	FISH("sherman/fish5.png",     3, fish5_animation,    fish5_speed     ),
	FISH("sherman/fish6.png",     3, normal_animation,   fish6_speed     ),
	FISH("sherman/squid.png",     3, squid_animation,    squid_speed     ),
	FISH("sherman/swordfish.png", 3, normal_animation,   swordfish_speed ),
	FISH("sherman/blowfish.png",  3, normal_animation,   blowfish_speed  ),
	FISH("sherman/ernest.png",    3, normal_animation,   ernest_speed    ),
	FISH("sherman/hunter.png",    3, normal_animation,   hunter_speed    ),
	FISH("sherman/lori.png",      3, normal_animation,   lori_speed      ),
	FISH("sherman/prey.png",      3, normal_animation,   prey_speed      ),
	FISH("sherman/sherman.png",  11, sherman_animation,  sherman_speed   ),
	FISH("sherman/fillmore.png", 12, fillmore_animation, fillmore_speed  ),
	FISH("sherman/bdweller.png",  3, normal_animation,   bdweller_speed  ),
	FISH("sherman/hawthorne.png", 3, normal_animation,   bdweller_speed  ),
	FISH("sherman/megan.png",    11, sherman_animation,  sherman_speed   ),
};

static int num_fish;
static struct fish *fish;

struct fish_image
{
        bool loaded;
        Imlib_Image image[2];
};

static struct fish_image fish_image[NUMOFFISHTYPES];

static void load_fish(int type, int scale)
{
        Imlib_Image orig;
        char path[1024];

        if(fish_image[type].loaded)
                return;

        snprintf(path, sizeof(path), "%s/%s", IMAGE_PATH, fish_animation[type].file);

        orig = imlib_load_image(path);
        imlib_context_set_image(orig);

	printf("%s * %d %d\n", fish_animation[type].file, imlib_image_get_width() * scale / 100, imlib_image_get_height() * scale / 100);
        fish_image[type].loaded = true;



        fish_image[type].image[0] = imlib_create_cropped_scaled_image(0, 0,
                                                                      imlib_image_get_width(),
                                                                      imlib_image_get_height(),
                                                                      imlib_image_get_width() * scale / 100,
                                                                      imlib_image_get_height() * scale / 100);

        imlib_free_image_and_decache();

        imlib_context_set_image(fish_image[type].image[0]);
        fish_image[type].image[1] = imlib_clone_image();

        imlib_context_set_image(fish_image[type].image[1]);
        imlib_image_flip_horizontal();

}

void fish_load(int bw, int bh, int scale)
{
        int i;
        num_fish = (random() % MAX_FISH) + 1;

        for (i = 0; i < NUMOFFISHTYPES; i++)
                fish_image[i].loaded = false;


        fish = malloc(sizeof(struct fish) * num_fish);

        for (i = 0; i < num_fish; i++) {


                do{
                        fish[i].type = random() % NUMOFFISHTYPES;
		} while(fish[i].type == BDWELLER || fish[i].type == HAWTHORNE);

                fish[i].animation = &fish_animation[fish[i].type];

                load_fish(fish[i].type, scale);

		fish[i].h = imlib_image_get_height() / fish[i].animation->pics;
		fish[i].w = imlib_image_get_width();

                fish[i].frame = random() % fish[i].animation->frames;
                fish[i].rev = random() % 2;
                fish[i].tx = (random() % bw * 2) - bw / 2;
                fish[i].speed_mul =  1.0 + (float)((random() % 30) - 15)/ 100.0;

                fish[i].fast_frame_change = 1.0;
                fish[i].is_dead = false;

                fish[i].updown = 0;
                fish[i].travel = random() % 100;

                fish[i].y = random() % bh;
        }

}

void fish_update(int bw, int bh)
{
        int i, j;
	int dx;

        for (i = 0; i < num_fish; i++) {

		dx = fish[i].animation->speed[fish[i].frame] * fish[i].speed_mul;
		if(fish[i].rev)
			fish[i].tx += dx;
		else
			fish[i].tx -= dx;


                if((!fish[i].rev && fish[i].tx < (- fish[i].w - fish[i].travel)) ||
                   (fish[i].rev && fish[i].tx > (bh + fish[i].travel + fish[i].w))) {
                        fish[i].rev = !fish[i].rev;
                        fish[i].y = random() % bh;
                }

                j = random() % 16;

                if (((fish[i].updown == 1) && (j == 6 || j == 7 || j == 8))
                    || j == 8) {
                        fish[i].y++;
                        fish[i].updown = 1;
                } else if (((fish[i].updown == -1) && (j == 12 || j == 13 || j == 14)) || j == 12) {
                        fish[i].y--;
                        fish[i].updown = -1;
                } else
                        fish[i].updown = 0;

		imlib_context_set_image(fish_image[fish[i].type].image[fish[i].rev]);

		x_draw((unsigned char *)imlib_image_get_data_for_reading_only(),
		       0, fish[i].h * fish[i].animation->animation[fish[i].frame],
		       fish[i].w,
		       fish[i].h,
		       (int)fish[i].tx, fish[i].y,
		       (bool)imlib_image_has_alpha());

                fish[i].delay += fish[i].animation->speed[fish[i].frame] * fish[i].fast_frame_change;

                if (fish[i].delay >= (7 * fish[i].animation->speed[fish[i].frame])) {
                        if (fish[i].frame >= (fish[i].animation->frames - 1))
                                fish[i].frame = 0;
                        else
                                fish[i].frame++;
                        fish[i].delay = 0;
                }
        }
}

