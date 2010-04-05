
#include <stdlib.h>
#include <unistd.h>

#include "config.h"
#include "sal.h"
#include "image.h"
#include "aquarium.h"
#include "window.h"

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

struct aquarium *aquarium;

static void load_fish(int type, int scale)
{
        if(fish_image[type].loaded)
                return;

        fish_image[type].loaded = true;
        fish_image[type].image[0] = image_load_relative(fish_animation[type].file, scale);

        imlib_context_set_image(fish_image[type].image[0]);
        fish_image[type].image[1] = imlib_clone_image();

        imlib_context_set_image(fish_image[type].image[1]);
        imlib_image_flip_horizontal();

}

void fish_init(void)
{
        int i;

        aquarium = aquarium_get();

        if(aquarium->random_fish > -1)
                num_fish = (random() % aquarium->random_fish) + 1;
        else
                num_fish = aquarium->num_fish;


        for (i = 0; i < NUMOFFISHTYPES; i++)
                fish_image[i].loaded = false;


        fish = malloc(sizeof(struct fish) * num_fish);

        for (i = 0; i < num_fish; i++) {

                do{
                        fish[i].type = random() % NUMOFFISHTYPES;
		} while(fish[i].type == BDWELLER || fish[i].type == HAWTHORNE);

                fish[i].animation = &fish_animation[fish[i].type];

                load_fish(fish[i].type, aquarium->fish_scale);

                imlib_context_set_image(fish_image[fish[i].type].image[0]);

		fish[i].h = imlib_image_get_height() / fish[i].animation->pics;
		fish[i].w = imlib_image_get_width();

                fish[i].frame = random() % fish[i].animation->frames;
                fish[i].rev = random() % 2;
                fish[i].tx = (random() % aquarium->w * 2) - aquarium->w / 2;
                fish[i].speed_mul =  1.0 + (float)((random() % 30) - 15)/ 100.0;

                fish[i].fast_frame_change = 1.0;
                fish[i].is_dead = false;

                fish[i].updown = 0;
                fish[i].travel = random() % 100;

                fish[i].y = random() % aquarium->h;
                fish[i].delay = 0;
        }

}

void fish_update(void)
{
        int i, j;
	float dx;

        for (i = 0; i < num_fish; i++) {

		dx = fish[i].animation->speed[fish[i].frame] * fish[i].speed_mul;
		if(fish[i].rev)
			fish[i].tx += dx;
		else
			fish[i].tx -= dx;

                if((!fish[i].rev && fish[i].tx < (- fish[i].w - fish[i].travel)) ||
                   (fish[i].rev && fish[i].tx > (aquarium->h + fish[i].travel + fish[i].w))) {
                        fish[i].rev = !fish[i].rev;
                        fish[i].y = random() % aquarium->h;
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

		window_draw((unsigned char *)imlib_image_get_data_for_reading_only(),
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

