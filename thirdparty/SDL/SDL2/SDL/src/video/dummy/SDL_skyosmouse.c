/*
  Simple DirectMedia Layer
  Copyright (C) 1997-2016 Sam Lantinga <slouken@libsdl.org>

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

#include "../../SDL_internal.h"

#if SDL_VIDEO_DRIVER_DUMMY

#include "SDL_skyosmouse.h"

#include "SDL_events.h"
#include "../../events/SDL_mouse_c.h"

#include "../../core/skyos/SDL_skyos.h"

#include "skyoswindow.h"

static Uint8 SDLButton;

void
Skyos_InitMouse(void)
{
    SDLButton = 0;
}

void SKYOS_OnMouse(SDL_Window* pWindow, QWORD action, float x, float y)
{
   
    switch(action) {
        case EVENT_MOUSE_LBUTTONDOWN:            
            SDLButton = SDL_BUTTON_LEFT;
            
            SDL_SendMouseMotion(pWindow, 0, 0, x, y);
            SDL_SendMouseButton(pWindow, 0, SDL_PRESSED, SDLButton);
            break;

        case EVENT_MOUSE_LBUTTONUP:
			SDLButton = SDL_BUTTON_LEFT;
            SDL_SendMouseMotion(pWindow, 0, 0, x, y);
            SDL_SendMouseButton(pWindow, 0, SDL_RELEASED, SDLButton);
            break;

		case EVENT_MOUSE_RBUTTONDOWN:
			SDLButton = SDL_BUTTON_RIGHT;

			SDL_SendMouseMotion(pWindow, 0, 0, x, y);
			SDL_SendMouseButton(pWindow, 0, SDL_PRESSED, SDLButton);
			break;

		case EVENT_MOUSE_RBUTTONUP:
			SDLButton = SDL_BUTTON_RIGHT;
			SDL_SendMouseMotion(pWindow, 0, 0, x, y);
			SDL_SendMouseButton(pWindow, 0, SDL_RELEASED, SDLButton);
			break;

        case EVENT_MOUSE_MOVE:        
            SDL_SendMouseMotion(pWindow, 0, 0, x, y);
            break;

        case EVENT_MOUSE_MBUTTONDOWN:
		case EVENT_MOUSE_MBUTTONUP:
            SDL_SendMouseWheel(pWindow, 0, x, y, SDL_MOUSEWHEEL_NORMAL);
            break;

        default:
            break;
    }
}

#endif /* SDL_VIDEO_DRIVER_ANDROID */

/* vi: set ts=4 sw=4 expandtab: */

