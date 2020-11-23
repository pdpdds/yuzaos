/*************************************************************************
	general.c -- last change: 20-1-1998

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

int svgagui_initialized = FALSE, sleep_time = 1;


void set_sleep_time(int time)
{
	sleep_time = time;
}


void init_svgagui(QWORD windowId)
{
	guiscreen.data = NULL;

	mouse.savescreen = NULL;
	mouse.visible = FALSE;
	mouse.num_cursors = 0;
	mouse.cursor_id = -1;	/* no mouse defined yet */
	
	set_sleep_time(1);
	svgagui_initialized = TRUE;
	parentScreenId = windowId;
}


int GuiGetMessage(void)
{
	int message = GetMessageEvent();
	return message;
}

int GuiMouseGetButton(void)
{
	int button = get_svga_mouse_button();
	return button;
}


char GuiKeyboardGetChar(void)
{
	char ch = get_svga_keyboard_char();
	return ch;
}
