#include <stdio.h>
#include <stdlib.h>
#include "sdl2-util.h"

pixman_image_t* create_checkerboad(int width, int height, int tilesize)
{

	pixman_image_t* checkerboard;
	pixman_image_t* destination;
#define D2F(d) (pixman_double_to_fixed(d))
	pixman_transform_t trans = { {
		{ D2F(-1.96830), D2F(-1.82250), D2F(512.12250)},
		{ D2F(0.00000), D2F(-7.29000), D2F(1458.00000)},
		{ D2F(0.00000), D2F(-0.00911), D2F(0.59231)},
	} };
	int i, j;

	checkerboard = pixman_image_create_bits(PIXMAN_a8r8g8b8,
		width, height,
		NULL, 0);

	destination = pixman_image_create_bits(PIXMAN_a8r8g8b8,
		width, height,
		NULL, 0);

	for (i = 0; i < height / tilesize; ++i)
	{
		for (j = 0; j < width / tilesize; ++j)
		{
			double u = (double)(j + 1) / (width / tilesize);
			double v = (double)(i + 1) / (height / tilesize);
			pixman_color_t black = { 0, 0, 0, 0xffff };
			pixman_color_t white = {
			v * 0xffff,
			u * 0xffff,
			(1 - (double)u) * 0xffff,
			0xffff };
			pixman_color_t* c;
			pixman_image_t* fill;

			if ((j & 1) != (i & 1))
				c = &black;
			else
				c = &white;

			fill = pixman_image_create_solid_fill(c);

			pixman_image_composite(PIXMAN_OP_SRC, fill, NULL, checkerboard,
				0, 0, 0, 0, j * tilesize, i * tilesize,
				tilesize, tilesize);
		}
	}

	pixman_image_set_transform(checkerboard, &trans);
	pixman_image_set_filter(checkerboard, PIXMAN_FILTER_BEST, NULL, 0);
	pixman_image_set_repeat(checkerboard, PIXMAN_REPEAT_NONE);

	pixman_image_composite(PIXMAN_OP_SRC,
		checkerboard, NULL, destination,
		0, 0, 0, 0, 0, 0,
		width, height);

	return checkerboard;
}

#define WIDTH 400
#define HEIGHT 400
#define TILE_SIZE 25


int main(int argc, char** argv)
{
	pixman_image_t* checkerboard = create_checkerboad(WIDTH, HEIGHT, TILE_SIZE);

    cairo_t* cairo_context = init_sdl(WIDTH, HEIGHT);
    render_pixman_image(cairo_context, checkerboard);
    loop_sdl();

    return 0;
}
