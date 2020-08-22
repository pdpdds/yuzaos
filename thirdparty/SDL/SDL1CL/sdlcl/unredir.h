/*
 * SDLCL - SDL Compatibility Library
 * Copyright (C) 2017 Alan Williams <mralert@gmail.com>
 * 
 * Portions taken from SDL 1.2.15
 * Copyright (C) 1997-2012 Sam Latinga <slouken@libsdl.org>
 * 
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 * 
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */

#undef SDL_Init
#undef SDL_InitSubSystem
#undef SDL_GetError
#undef SDL_SetError
#undef SDL_ClearError
#undef SDL_Error
#undef SDL_QuitSubSystem
#undef SDL_Quit
#undef SDL_WasInit
#undef SDL_SetHint

/* Timer subsystem */
#undef SDL_GetTicks
#undef SDL_Delay
#undef SDL_AddTimer
#undef SDL_RemoveTimer

/* Video subsystem */
#undef SDL_VideoInit
#undef SDL_VideoQuit
#undef SDL_GetCurrentVideoDriver
#undef SDL_GetNumVideoDisplays
#undef SDL_GetNumDisplayModes
#undef SDL_GetDisplayMode
#undef SDL_GetDesktopDisplayMode
#undef SDL_GetPixelFormatName
#undef SDL_CreateWindow
#undef SDL_GetWindowSize
#undef SDL_DestroyWindow

#undef SDL_CreateRGBSurface
#undef SDL_CreateRGBSurfaceFrom
#undef SDL_PixelFormatEnumToMasks
#undef SDL_GetSurfaceBlendMode
#undef SDL_GetSurfaceAlphaMod
#undef SDL_GetColorKey
#undef SDL_SetSurfaceBlendMode
#undef SDL_SetSurfaceAlphaMod
#undef SDL_SetColorKey
#undef SDL_SetSurfaceRLE
#undef SDL_GetClipRect
#undef SDL_SetClipRect
#undef SDL_FreeSurface
#undef SDL_LockSurface
#undef SDL_UnlockSurface
#undef SDL_FillRect
#undef SDL_UpperBlit
#undef SDL_LowerBlit
#undef SDL_ConvertSurface
#undef SDL_LoadBMP_RW
#undef SDL_SaveBMP_RW

#undef SDL_MapRGBA
#undef SDL_MapRGB
#undef SDL_GetRGBA
#undef SDL_GetRGB

#undef SDL_SetPaletteColors

#undef SDL_CreateCursor
#undef SDL_FreeCursor
#undef SDL_SetCursor
#undef SDL_GetCursor
#undef SDL_ShowCursor
#undef SDL_WarpMouseInWindow

#undef SDL_CalculateGammaRamp
#undef SDL_GetWindowGammaRamp
#undef SDL_SetWindowGammaRamp

#undef SDL_SetWindowTitle
#undef SDL_GetWindowGrab
#undef SDL_SetWindowGrab
#undef SDL_MinimizeWindow
#undef SDL_SetRelativeMouseMode

#undef SDL_CreateRenderer
#undef SDL_SetRenderDrawColor
#undef SDL_RenderClear
#undef SDL_RenderCopy
#undef SDL_RenderPresent
#undef SDL_DestroyRenderer
#undef SDL_CreateTexture
#undef SDL_SetTextureBlendMode
#undef SDL_LockTexture
#undef SDL_UnlockTexture
#undef SDL_UpdateTexture
#undef SDL_UpdateYUVTexture
#undef SDL_DestroyTexture

#undef SDL_GL_LoadLibrary
#undef SDL_GL_GetProcAddress
#undef SDL_GL_SwapWindow
#undef SDL_GL_SetAttribute
#undef SDL_GL_GetAttribute
#undef SDL_GL_CreateContext
#undef SDL_GL_MakeCurrent
#undef SDL_GL_DeleteContext

#undef SDL_GetWindowWMInfo

#undef SDL_SoftStretch

/* Audio subsystem */
#undef SDL_AudioInit
#undef SDL_AudioQuit
#undef SDL_OpenAudio
#undef SDL_PauseAudio
#undef SDL_GetAudioStatus
#undef SDL_MixAudio
#undef SDL_LockAudio
#undef SDL_UnlockAudio
#undef SDL_CloseAudio
#undef SDL_GetCurrentAudioDriver

#undef SDL_BuildAudioCVT
#undef SDL_ConvertAudio
#undef SDL_LoadWAV_RW
#undef SDL_FreeWAV

/* Events subsystem */
#undef SDL_JoystickEventState
#undef SDL_GetModState
#undef SDL_SetModState
#undef SDL_GetKeyName
#undef SDL_GetMouseState
#undef SDL_GetRelativeMouseState
#undef SDL_StartTextInput
#undef SDL_StopTextInput
#undef SDL_PollEvent
#undef SDL_PeepEvents
#undef SDL_WaitEvent
#undef SDL_PumpEvents
#undef SDL_PushEvent
#undef SDL_SetEventFilter
#undef SDL_GetEventFilter
#undef SDL_EventState

/* Joystick subsystem */
#undef SDL_NumJoysticks
#undef SDL_JoystickOpen
#undef SDL_JoystickName
#undef SDL_JoystickNameForIndex
#undef SDL_JoystickNumAxes
#undef SDL_JoystickNumBalls
#undef SDL_JoystickNumButtons
#undef SDL_JoystickNumHats
#undef SDL_JoystickGetAxis
#undef SDL_JoystickGetBall
#undef SDL_JoystickGetButton
#undef SDL_JoystickGetHat
#undef SDL_JoystickUpdate
#undef SDL_JoystickClose

/* RWops */
#undef SDL_AllocRW
#undef SDL_FreeRW
#undef SDL_RWFromFP
#undef SDL_RWFromFile
#undef SDL_RWFromMem
#undef SDL_RWFromConstMem

#undef SDL_ReadLE16
#undef SDL_ReadBE16
#undef SDL_ReadLE32
#undef SDL_ReadBE32
#undef SDL_ReadLE64
#undef SDL_ReadBE64

#undef SDL_WriteLE16
#undef SDL_WriteBE16
#undef SDL_WriteLE32
#undef SDL_WriteBE32
#undef SDL_WriteLE64
#undef SDL_WriteBE64

/* Threading */
#undef SDL_CreateThread
#undef SDL_WaitThread
#undef SDL_ThreadID
#undef SDL_GetThreadID

#undef SDL_CreateMutex
#undef SDL_LockMutex
#undef SDL_UnlockMutex
#undef SDL_DestroyMutex

#undef SDL_CreateSemaphore
#undef SDL_SemWait
#undef SDL_SemTryWait
#undef SDL_SemWaitTimeout
#undef SDL_SemPost
#undef SDL_SemValue
#undef SDL_DestroySemaphore

#undef SDL_CreateCond
#undef SDL_CondSignal
#undef SDL_CondBroadcast
#undef SDL_CondWait
#undef SDL_CondWaitTimeout
#undef SDL_DestroyCond

/* CPU capabilities */
#undef SDL_Has3DNow
#undef SDL_HasAltiVec
#undef SDL_HasMMX
#undef SDL_HasRDTSC
#undef SDL_HasSSE
#undef SDL_HasSSE2

/* Standard library functions */
#undef SDL_ltoa
#undef SDL_ultoa
#undef SDL_lltoa
#undef SDL_ulltoa
#undef SDL_strlcpy
#undef SDL_strlcat
#undef SDL_strrev
#undef SDL_strupr
#undef SDL_strlwr

#undef SDL_iconv_open
#undef SDL_iconv_close
#undef SDL_iconv
#undef SDL_iconv_string
