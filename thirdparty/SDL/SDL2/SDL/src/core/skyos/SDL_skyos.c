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
#include "SDL_stdinc.h"
#include "SDL_assert.h"
#include "SDL_hints.h"
#include "SDL_log.h"

#ifdef __SKYOS32__

#include "SDL_system.h"
#include "SDL_skyos.h"

#include "../../events/SDL_events_c.h"
#include "../../video/dummy/SDL_skyoskeyboard.h"
#include "../../video/dummy/SDL_skyosmouse.h"
#include "../../video/dummy/SDL_nullvideo.h"
//#include "../../video/android/SDL_androidwindow.h"
#include "../../joystick/android/SDL_sysjoystick_c.h"

/*

JNIEXPORT void JNICALL SDL_Android_Init(JNIEnv* mEnv, jclass cls)
{    
}



JNIEXPORT void JNICALL Java_org_libsdl_app_SDLActivity_onNativeResize(
                                    JNIEnv* env, jclass jcls,
                                    jint width, jint height, jint format, jfloat rate)
{
    Android_SetScreenResolution(width, height, format, rate);
}


JNIEXPORT void JNICALL Java_org_libsdl_app_SDLActivity_onNativeKeyDown(
                                    JNIEnv* env, jclass jcls, jint keycode)
{
    Android_OnKeyDown(keycode);
}


JNIEXPORT void JNICALL Java_org_libsdl_app_SDLActivity_onNativeKeyUp(
                                    JNIEnv* env, jclass jcls, jint keycode)
{
    Android_OnKeyUp(keycode);
}

JNIEXPORT void JNICALL Java_org_libsdl_app_SDLActivity_onNativeMouse(
                                    JNIEnv* env, jclass jcls,
                                    jint button, jint action, jfloat x, jfloat y)
{
    Android_OnMouse(button, action, x, y);
}*/


#endif /* __ANDROID__ */

/* vi: set ts=4 sw=4 expandtab: */

