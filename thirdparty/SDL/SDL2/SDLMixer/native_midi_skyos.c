/*
  native_midi:  Hardware Midi support for the SDL_mixer library
  Copyright (C) 2000,2001  Florian 'Proff' Schulze <florian.proff.schulze@gmx.net>

  This software is provided 'as-is', without any express or implied
  warranty.  In no event will the authors be held liable for any damages
  arising from the use of this software.

  Permission is granted to anyone to use this software for any purpose,
  including commercial applications, and to alter it and redistribute it
  freely, subject to the following restrictions:

  1. The origin of this software must not be misrepresented; you must not
	 claim that you wrote the original software. If you use this software
	 in a product, an acknowledgment in the product documentation would be
	 appreciated but is not required.
  2. Altered source versions must be plainly marked as such, and must not be
	 misrepresented as being the original software.
  3. This notice may not be removed or altered from any source distribution.
*/
#include "SDL_config.h"

/* everything below is currently one very big bad hack ;) Proff */

#if __SKYOS32__
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include "native_midi.h"
#include "native_midi_common.h"


static NativeMidiSong* currentsong;

static int BlockOut(NativeMidiSong* song)
{
	return 1;
}

static void MIDItoStream(NativeMidiSong* song, MIDIEvent* evntlist)
{

}

int native_midi_detect(void)
{
	return 1;
}

NativeMidiSong* native_midi_loadsong_RW(SDL_RWops* src, int freesrc)
{
	
	return NULL;
	
}

void native_midi_freesong(NativeMidiSong* song)
{
	
}

void native_midi_start(NativeMidiSong* song, int loops)
{
	
}

void native_midi_pause(void)
{

}

void native_midi_resume(void)
{
	
}

void native_midi_stop(void)
{
	
}

int native_midi_active(void)
{
	return 1;
}

void native_midi_setvolume(int volume)
{
	
}

const char* native_midi_error(void)
{
	return NULL;
}

#endif 
