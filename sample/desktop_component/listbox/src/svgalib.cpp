/*************************************************************************
	svgalib.c -- last change: 19-11-2000

	Copyright (C) 1996-2007  Boris Nagels

	This file is part of SVGAgui.

	SVGAgui is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 2 of the License, or
	(at your option) any later version.
	
	SVGAgui is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.
	
	You should have received a copy of the GNU General Public License
	along with SVGAgui; if not, write to the Free Software
	Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

 *************************************************************************/

#include "local.h"
#include <stdio.h>
//#include <stdlib.h>
#include "windef.h"
//#include "bootinfo.h"
#include "memory.h"
//#include "vesa.h"
#include <string.h>
//#include "svga_mouse.h"
//#include "svga_keyb.h"

static char savechar = '\0';

extern "C" void init_svga_mouse()
{
	/*
	if (mouse_init("/dev/mouse", vga_getmousetype(), MOUSE_DEFAULTSAMPLERATE))
		error("Could not initialize mouse.");

	mouse_setdefaulteventhandler();
	mouse_setxrange(0, guiscreen.width - 1);
	mouse_setyrange(0, guiscreen.height - 1);
	mouse_setposition(guiscreen.width / 2, guiscreen.height / 2);
    */
	//mouse.x = mouse_getx();
	//mouse.y = mouse_gety();
}

#include <skyoswindow.h>
extern EVENT svga_event;
extern "C" void get_svga_mouse_position(void)
{
	mouse.x = svga_event.stMouseEvent.stPoint.iX;
	mouse.y = svga_event.stMouseEvent.stPoint.iY - WINDOW_TITLEBAR_HEIGHT;
}

extern "C" int mouse_update();
extern "C" int mouse_getbutton();

extern "C" int get_svga_message(void)
{
	//if ((savechar = vga_getkey()))
	//	return GuiKeyboardEvent;

	if (mouse_update())
		return GuiMouseEvent;

	return FALSE;
}


int get_svga_mouse_button(void)
{
	return mouse_getbutton();
}

char get_svga_keyboard_char(void)
{
	return savechar;
}

void save_svga_screen_to_xpm(void)
{
	//FILE *fd;
	//int x, y, i, count, r, g, b, nr_colors;
	//int buffer[256];
#if 0
	/* determine the number of colors used */
	nr_colors = 0;
	for (i = 0; i < guiscreen.width * guiscreen.height; i++) {
		count = 0;
		while (buffer[count] != guiscreen.data[i] && count < nr_colors)
			count++;
		if (count == nr_colors) {	/* color not found */
			nr_colors++;
			buffer[nr_colors] = guiscreen.data[i];
		}
	}

	//fd = fopen("screen.xpm", "w");
	fd = NULL;
	if (fd == NULL)
		error("Cannot open screen.xpm for writing in save_screen_to_xpm().");

	fprintf(fd, "/* XPM */\n");
	fprintf(fd, "static char * screen_xpm[] = {\n");
	fprintf(fd, "\"%d %d %d 1\",\n", guiscreen.width, guiscreen.height, nr_colors);
	for (i = 0; i < nr_colors; i++) {
		gl_getpalettecolor(color[i], &r, &g, &b);
		r = (int) (r / 64.0 * 255);
		g = (int) (g / 64.0 * 255);
		b = (int) (b / 64.0 * 255);
		fprintf(fd, "\"%c   c #%02x%02x%02x\",\n", i + 65, r, g, b);
	}

	for (y = 0; y < guiscreen.height; y++) {
		fprintf(fd, "\"");
		for (x = 0; x < guiscreen.width; x++) {
			i = 0;
			while (guiscreen.data[x + guiscreen.width * y] != color[i])
				i++;
			fprintf(fd, "%c", i + 65);
		}
		fprintf(fd, "\",\n");
	}
	fclose(fd);
#endif	
}

void set_svga_default_palette(int colors)
{
}


void close_svga_screen(void)
{
	/*if (guiscreen.type == SVGALIB)
		vga_setmode(TEXT);*/
}

void set_svga_palette(int col, int red, int green, int blue)
{
//	gl_setpalettecolor(color, red, green, blue);
}


void get_svga_palette(int col, int *red, int *green, int *blue)
{
//	gl_getpalettecolor(color, red, green, blue);
}


void switch_svga_cursor(int cursor_id)
{

}

extern "C" void set_svga_mouse_position(int x, int y)
{
	mouse.x = x;
	mouse.y = y;

}
