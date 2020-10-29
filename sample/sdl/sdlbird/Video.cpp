/*
 * Copyright (c) 2014, Wei Mingzhi <whistler_wmz@users.sf.net>.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. The name of the author and contributors may not be used to endorse
 *    or promote products derived from this software without specific prior
 *    written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
 * TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL WASABI SYSTEMS, INC
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */
#include <minwindef.h>
#include <SDL.h>

#include "Video.h"
#include  <stdio.h>
SDL_Window* gpWindow = NULL;
SDL_Renderer* gpRenderer = NULL;
SDL_Texture* gpTexture = NULL;

bool VideoInit()
{

	gpRenderer = SDL_CreateRenderer(gpWindow, -1, 0);

	if (SDL_CreateWindowAndRenderer(SCREEN_WIDTH, SCREEN_HEIGHT, 0, &gpWindow, &gpRenderer) < 0)
	{
		printf("SDL_CreateWindowAndRenderer Error\n");
		return 0;
	}


	if (gpRenderer == NULL)
	{
		SDL_DestroyWindow(gpWindow);
		return false;
	}

#if defined (__IPHONEOS__)
	SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 0);
	SDL_GL_SetAttribute(SDL_GL_RETAINED_BACKING, 1);
#endif

	gpTexture = SDL_CreateTexture(gpRenderer, SDL_PIXELFORMAT_ARGB8888,
		SDL_TEXTUREACCESS_TARGET, SCREEN_WIDTH, SCREEN_HEIGHT);

	if (gpTexture == NULL)
	{
		SDL_DestroyRenderer(gpRenderer);
		SDL_DestroyWindow(gpWindow);
		return false;
	}

	return true;
}

void VideoDestroy()
{
	if (gpTexture != NULL)
	{
		SDL_DestroyTexture(gpTexture);
		gpTexture = NULL;
	}

	if (gpRenderer != NULL)
	{
		SDL_DestroyRenderer(gpRenderer);
		gpRenderer = NULL;
	}

	if (gpWindow != NULL)
	{
		SDL_DestroyWindow(gpWindow);
		gpWindow = NULL;
	}
}

void FrameBegin()
{
	SDL_SetRenderTarget(gpRenderer, gpTexture);
}

void FrameEnd()
{

	SDL_SetRenderTarget(gpRenderer, NULL);

	SDL_RenderCopy(gpRenderer, gpTexture, NULL, NULL);
	SDL_RenderPresent(gpRenderer);
}
