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

#define SDL_Init reSDL_Init
#define SDL_InitSubSystem reSDL_InitSubSystem
#define SDL_GetError reSDL_GetError
#define SDL_SetError reSDL_SetError
#define SDL_ClearError reSDL_ClearError
#define SDL_Error reSDL_Error
#define SDL_QuitSubSystem reSDL_QuitSubSystem
#define SDL_Quit reSDL_Quit
#define SDL_WasInit reSDL_WasInit
#define SDL_SetHint reSDL_SetHint

/* Timer subsystem */
#define SDL_GetTicks reSDL_GetTicks
#define SDL_Delay reSDL_Delay
#define SDL_AddTimer reSDL_AddTimer
#define SDL_RemoveTimer reSDL_RemoveTimer

/* Video subsystem */
#define SDL_VideoInit reSDL_VideoInit
#define SDL_VideoQuit reSDL_VideoQuit
#define SDL_GetCurrentVideoDriver reSDL_GetCurrentVideoDriver
#define SDL_GetNumVideoDisplays reSDL_GetNumVideoDisplays
#define SDL_GetNumDisplayModes reSDL_GetNumDisplayModes
#define SDL_GetDisplayMode reSDL_GetDisplayMode
#define SDL_GetDesktopDisplayMode reSDL_GetDesktopDisplayMode
#define SDL_GetPixelFormatName reSDL_GetPixelFormatName
#define SDL_CreateWindow reSDL_CreateWindow
#define SDL_GetWindowSize reSDL_GetWindowSize
#define SDL_DestroyWindow reSDL_DestroyWindow

#define SDL_CreateRGBSurface reSDL_CreateRGBSurface
#define SDL_CreateRGBSurfaceFrom reSDL_CreateRGBSurfaceFrom
#define SDL_PixelFormatEnumToMasks reSDL_PixelFormatEnumToMasks
#define SDL_GetSurfaceBlendMode reSDL_GetSurfaceBlendMode
#define SDL_GetSurfaceAlphaMod reSDL_GetSurfaceAlphaMod
#define SDL_GetColorKey reSDL_GetColorKey
#define SDL_SetSurfaceBlendMode reSDL_SetSurfaceBlendMode
#define SDL_SetSurfaceAlphaMod reSDL_SetSurfaceAlphaMod
#define SDL_SetColorKey reSDL_SetColorKey
#define SDL_SetSurfaceRLE reSDL_SetSurfaceRLE
#define SDL_GetClipRect reSDL_GetClipRect
#define SDL_SetClipRect reSDL_SetClipRect
#define SDL_FreeSurface reSDL_FreeSurface
#define SDL_LockSurface reSDL_LockSurface
#define SDL_UnlockSurface reSDL_UnlockSurface
#define SDL_FillRect reSDL_FillRect
#define SDL_UpperBlit reSDL_UpperBlit
#define SDL_LowerBlit reSDL_LowerBlit
#define SDL_ConvertSurface reSDL_ConvertSurface
#define SDL_LoadBMP_RW reSDL_LoadBMP_RW
#define SDL_SaveBMP_RW reSDL_SaveBMP_RW

#define SDL_MapRGBA reSDL_MapRGBA
#define SDL_MapRGB reSDL_MapRGB
#define SDL_GetRGBA reSDL_GetRGBA
#define SDL_GetRGB reSDL_GetRGB

#define SDL_SetPaletteColors reSDL_SetPaletteColors

#define SDL_CreateCursor reSDL_CreateCursor
#define SDL_FreeCursor reSDL_FreeCursor
#define SDL_SetCursor reSDL_SetCursor
#define SDL_GetCursor reSDL_GetCursor
#define SDL_ShowCursor reSDL_ShowCursor
#define SDL_WarpMouseInWindow reSDL_WarpMouseInWindow

#define SDL_CalculateGammaRamp reSDL_CalculateGammaRamp
#define SDL_GetWindowGammaRamp reSDL_GetWindowGammaRamp
#define SDL_SetWindowGammaRamp reSDL_SetWindowGammaRamp

#define SDL_SetWindowTitle reSDL_SetWindowTitle
#define SDL_GetWindowGrab reSDL_GetWindowGrab
#define SDL_SetWindowGrab reSDL_SetWindowGrab
#define SDL_MinimizeWindow reSDL_MinimizeWindow
#define SDL_SetRelativeMouseMode reSDL_SetRelativeMouseMode

#define SDL_CreateRenderer reSDL_CreateRenderer
#define SDL_SetRenderDrawColor reSDL_SetRenderDrawColor
#define SDL_RenderClear reSDL_RenderClear
#define SDL_RenderCopy reSDL_RenderCopy
#define SDL_RenderPresent reSDL_RenderPresent
#define SDL_DestroyRenderer reSDL_DestroyRenderer
#define SDL_CreateTexture reSDL_CreateTexture
#define SDL_SetTextureBlendMode reSDL_SetTextureBlendMode
#define SDL_LockTexture reSDL_LockTexture
#define SDL_UnlockTexture reSDL_UnlockTexture
#define SDL_UpdateTexture reSDL_UpdateTexture
#define SDL_UpdateYUVTexture reSDL_UpdateYUVTexture
#define SDL_DestroyTexture reSDL_DestroyTexture

#define SDL_GL_LoadLibrary reSDL_GL_LoadLibrary
#define SDL_GL_GetProcAddress reSDL_GL_GetProcAddress
#define SDL_GL_SwapWindow reSDL_GL_SwapWindow
#define SDL_GL_SetAttribute reSDL_GL_SetAttribute
#define SDL_GL_GetAttribute reSDL_GL_GetAttribute
#define SDL_GL_CreateContext reSDL_GL_CreateContext
#define SDL_GL_MakeCurrent reSDL_GL_MakeCurrent
#define SDL_GL_DeleteContext reSDL_GL_DeleteContext

#define SDL_GetWindowWMInfo reSDL_GetWindowWMInfo

#define SDL_SoftStretch reSDL_SoftStretch

/* Audio subsystem */
#define SDL_AudioInit reSDL_AudioInit
#define SDL_AudioQuit reSDL_AudioQuit
#define SDL_OpenAudio reSDL_OpenAudio
#define SDL_PauseAudio reSDL_PauseAudio
#define SDL_GetAudioStatus reSDL_GetAudioStatus
#define SDL_MixAudio reSDL_MixAudio
#define SDL_LockAudio reSDL_LockAudio
#define SDL_UnlockAudio reSDL_UnlockAudio
#define SDL_CloseAudio reSDL_CloseAudio
#define SDL_GetCurrentAudioDriver reSDL_GetCurrentAudioDriver

#define SDL_BuildAudioCVT reSDL_BuildAudioCVT
#define SDL_ConvertAudio reSDL_ConvertAudio
#define SDL_LoadWAV_RW reSDL_LoadWAV_RW
#define SDL_FreeWAV reSDL_FreeWAV

/* Events subsystem */
#define SDL_JoystickEventState reSDL_JoystickEventState
#define SDL_GetModState reSDL_GetModState
#define SDL_SetModState reSDL_SetModState
#define SDL_GetKeyName reSDL_GetKeyName
#define SDL_GetMouseState reSDL_GetMouseState
#define SDL_GetRelativeMouseState reSDL_GetRelativeMouseState
#define SDL_StartTextInput reSDL_StartTextInput
#define SDL_StopTextInput reSDL_StopTextInput
#define SDL_PollEvent reSDL_PollEvent
#define SDL_PeepEvents reSDL_PeepEvents
#define SDL_WaitEvent reSDL_WaitEvent
#define SDL_PumpEvents reSDL_PumpEvents
#define SDL_PushEvent reSDL_PushEvent
#define SDL_SetEventFilter reSDL_SetEventFilter
#define SDL_GetEventFilter reSDL_GetEventFilter
#define SDL_EventState reSDL_EventState

/* Joystick subsystem */
#define SDL_NumJoysticks reSDL_NumJoysticks
#define SDL_JoystickOpen reSDL_JoystickOpen
#define SDL_JoystickName reSDL_JoystickName
#define SDL_JoystickNameForIndex reSDL_JoystickNameForIndex
#define SDL_JoystickNumAxes reSDL_JoystickNumAxes
#define SDL_JoystickNumBalls reSDL_JoystickNumBalls
#define SDL_JoystickNumButtons reSDL_JoystickNumButtons
#define SDL_JoystickNumHats reSDL_JoystickNumHats
#define SDL_JoystickGetAxis reSDL_JoystickGetAxis
#define SDL_JoystickGetBall reSDL_JoystickGetBall
#define SDL_JoystickGetButton reSDL_JoystickGetButton
#define SDL_JoystickGetHat reSDL_JoystickGetHat
#define SDL_JoystickUpdate reSDL_JoystickUpdate
#define SDL_JoystickClose reSDL_JoystickClose

/* RWops */
#define SDL_AllocRW reSDL_AllocRW
#define SDL_FreeRW reSDL_FreeRW
#define SDL_RWFromFP reSDL_RWFromFP
#define SDL_RWFromFile reSDL_RWFromFile
#define SDL_RWFromMem reSDL_RWFromMem
#define SDL_RWFromConstMem reSDL_RWFromConstMem

#define SDL_ReadLE16 reSDL_ReadLE16
#define SDL_ReadBE16 reSDL_ReadBE16
#define SDL_ReadLE32 reSDL_ReadLE32
#define SDL_ReadBE32 reSDL_ReadBE32
#define SDL_ReadLE64 reSDL_ReadLE64
#define SDL_ReadBE64 reSDL_ReadBE64

#define SDL_WriteLE16 reSDL_WriteLE16
#define SDL_WriteBE16 reSDL_WriteBE16
#define SDL_WriteLE32 reSDL_WriteLE32
#define SDL_WriteBE32 reSDL_WriteBE32
#define SDL_WriteLE64 reSDL_WriteLE64
#define SDL_WriteBE64 reSDL_WriteBE64

/* Threading */
#define SDL_CreateThread reSDL_CreateThread
#define SDL_WaitThread reSDL_WaitThread
#define SDL_ThreadID reSDL_ThreadID
#define SDL_GetThreadID reSDL_GetThreadID

#define SDL_CreateMutex reSDL_CreateMutex
#define SDL_LockMutex reSDL_LockMutex
#define SDL_UnlockMutex reSDL_UnlockMutex
#define SDL_DestroyMutex reSDL_DestroyMutex

#define SDL_CreateSemaphore reSDL_CreateSemaphore
#define SDL_SemWait reSDL_SemWait
#define SDL_SemTryWait reSDL_SemTryWait
#define SDL_SemWaitTimeout reSDL_SemWaitTimeout
#define SDL_SemPost reSDL_SemPost
#define SDL_SemValue reSDL_SemValue
#define SDL_DestroySemaphore reSDL_DestroySemaphore

#define SDL_CreateCond reSDL_CreateCond
#define SDL_CondSignal reSDL_CondSignal
#define SDL_CondBroadcast reSDL_CondBroadcast
#define SDL_CondWait reSDL_CondWait
#define SDL_CondWaitTimeout reSDL_CondWaitTimeout
#define SDL_DestroyCond reSDL_DestroyCond

/* CPU capabilities */
#define SDL_Has3DNow reSDL_Has3DNow
#define SDL_HasAltiVec reSDL_HasAltiVec
#define SDL_HasMMX reSDL_HasMMX
#define SDL_HasRDTSC reSDL_HasRDTSC
#define SDL_HasSSE reSDL_HasSSE
#define SDL_HasSSE2 reSDL_HasSSE2

/* Standard library functions */
#define SDL_ltoa reSDL_ltoa
#define SDL_ultoa reSDL_ultoa
#define SDL_lltoa reSDL_lltoa
#define SDL_ulltoa reSDL_ulltoa
#define SDL_strlcpy reSDL_strlcpy
#define SDL_strlcat reSDL_strlcat
#define SDL_strrev reSDL_strrev
#define SDL_strupr reSDL_strupr
#define SDL_strlwr reSDL_strlwr

#define SDL_iconv_open reSDL_iconv_open
#define SDL_iconv_close reSDL_iconv_close
#define SDL_iconv reSDL_iconv
#define SDL_iconv_string reSDL_iconv_string
