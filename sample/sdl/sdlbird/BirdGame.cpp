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

#include "BirdGame.h"
#include "Sprite.h"
#include "Video.h"
#include "Audio.h"

#include <algorithm>
#include <math.h>
#include <stdlib.h>
#include <time.h>
#include <sprintf.h>

static CSprite *gpSprite = NULL;

#define stderr nullptr
#define stdin nullptr 

typedef enum tagGameState
{
	GAMESTATE_INITIAL = 0,
	GAMESTATE_GAMESTART,
	GAMESTATE_GAME,
	GAMESTATE_GAMEOVER,
} GameState;

static GameState g_GameState = GAMESTATE_INITIAL;
static bool g_bMouseDown = false;
static bool g_bNight = false;
static int g_iBirdPic = 0;
static int g_iMouseX = 0;
static int g_iMouseY = 0;
static int g_iScore = 0;
static int g_iHighScore = 0;
static float g_flBirdVelocity = 0;
static double g_flBirdHeight = 0;
static double g_flBirdAngle = 0;
static int g_iPipePosX[3] = { 0, 0, 0 };
static int g_iPipePosY[3] = { 0, 0, 0 };

static void *g_pSfxDie = NULL;
static void *g_pSfxHit = NULL;
static void *g_pSfxPoint = NULL;
static void *g_pSfxSwooshing = NULL;
static void *g_pSfxWing = NULL;

static bool g_bPauseGame = false;

#define GRAVITY      0.32f
#define WINGPOWER    5.2f
#define ROTATION     2.7f
#define PIPEDISTANCE 150
#define PIPEWIDTH    50
#define BIRDWIDTH    48
#define BIRDMARGIN   12

#ifdef __WINPHONE__
extern "C" const char *GetRootPath();
extern "C" const char *GetInstallPath();
extern "C" void RateApp();
#define fopen(a,b) _fsopen((a), (b), 0x40)
#endif

static const char *MakeFileName(const char *path, bool bUserDir = false)
{
#if defined __WINPHONE__
	static char buf[2][4096];
	static int current = 0;
	current ^= 1;
	strcpy(buf[current], bUserDir ? GetRootPath() : GetInstallPath());
	strcat(buf[current], "\\");
	const char *mypath = path + strlen(path) - 1;
	while (mypath >= path && (*mypath != '/' && *mypath != '\\'))
	{
		mypath--;
	}
	mypath++;
	strcat(buf[current], mypath);
	return buf[current];
#elif defined __IPHONEOS__
	static char buf[2][4096];
	static int current = 0;
	current ^= 1;
	if (bUserDir)
	{
		char *p = SDL_GetPrefPath("", "");
		strcpy(buf[current], p);
		strcat(buf[current], path);
		SDL_free(p);
	}
	else
	{
		char *p = SDL_GetBasePath();
		strcpy(buf[current], p);
		strcat(buf[current], "/");
		strcat(buf[current], path);
	}
	return buf[current];
#else
	return path;
#endif
}

static void LoadBestScore()
{
	g_iHighScore = 0;
	FILE *fp = fopen(MakeFileName("sdlbird.ini", true), "r");
	if (fp != NULL)
	{
		fscanf(fp, "%d", &g_iHighScore);
		fclose(fp);
	}
}

static void SaveBestScore()
{
	FILE *fp = fopen(MakeFileName("sdlbird.ini", true), "w");
	if (fp != NULL)
	{
		fprintf(fp, "%d", g_iHighScore);
		fclose(fp);
	}
}

static void LoadWav()
{
	g_pSfxDie = SOUND_LoadWAV(MakeFileName("res/sfx_die.wav"));
	g_pSfxHit = SOUND_LoadWAV(MakeFileName("res/sfx_hit.wav"));
	g_pSfxPoint = SOUND_LoadWAV(MakeFileName("res/sfx_point.wav"));
	g_pSfxSwooshing = SOUND_LoadWAV(MakeFileName("res/sfx_swooshing.wav"));
	g_pSfxWing = SOUND_LoadWAV(MakeFileName("res/sfx_wing.wav"));
}

static void FreeWav()
{
	SDL_PauseAudio(1);

	if (g_pSfxDie != NULL)
	{
		SOUND_FreeWAV(g_pSfxDie);
		g_pSfxDie = NULL;
	}
	if (g_pSfxHit != NULL)
	{
		SOUND_FreeWAV(g_pSfxHit);
		g_pSfxHit = NULL;
	}
	if (g_pSfxPoint != NULL)
	{
		SOUND_FreeWAV(g_pSfxPoint);
		g_pSfxPoint = NULL;
	}
	if (g_pSfxSwooshing != NULL)
	{
		SOUND_FreeWAV(g_pSfxSwooshing);
		g_pSfxSwooshing = NULL;
	}
	if (g_pSfxWing != NULL)
	{
		SOUND_FreeWAV(g_pSfxWing);
		g_pSfxWing = NULL;
	}
}

static void UpdateEvents()
{
	SDL_Event evt;

	while (SDL_PollEvent(&evt))
	{
		switch (evt.type)
		{
		case SDL_QUIT:
			ExitThread(0);
			break;

		case SDL_MOUSEBUTTONDOWN:
			g_bMouseDown = true;
			g_iMouseX = evt.button.x;
			g_iMouseY = evt.button.y;
			break;

		case SDL_MOUSEBUTTONUP:
			g_bMouseDown = false;
			break;

		case SDL_FINGERDOWN:
			g_bMouseDown = true;
			g_iMouseX = (int)(evt.tfinger.x * SCREEN_WIDTH);
			g_iMouseY = (int)(evt.tfinger.y * SCREEN_HEIGHT);
			break;

		case SDL_FINGERUP:
			g_bMouseDown = false;
			break;

		case SDL_KEYDOWN:
			if (evt.key.keysym.sym == SDLK_AC_BACK)
			{
				ExitThread(0);
			}
			break;

#ifdef __IPHONEOS__
		case SDL_APP_WILLENTERBACKGROUND:
			g_bPauseGame = true;
			break;

		case SDL_APP_DIDENTERFOREGROUND:
			g_bPauseGame = false;
			break;
#endif
		}
	}
}

static void ShowTitle()
{
	SDL_Surface *pSurfaceTitle = SDL_LoadBMP(MakeFileName("res/splash.bmp"));
	if (pSurfaceTitle == NULL)
	{
		fprintf(stderr, "cannot load res/splash.bmp\n");
		return;
	}

	SDL_Texture *pTextureTitle = SDL_CreateTextureFromSurface(gpRenderer, pSurfaceTitle);
	SDL_FreeSurface(pSurfaceTitle);

	if (pTextureTitle == NULL)
	{
		fprintf(stderr, "cannot create texture from splash.png\n");
		return;
	}

	unsigned int uiStartTime = SDL_GetTicks();

	while (SDL_GetTicks() - uiStartTime < 3000)
	{
		SDL_RenderCopy(gpRenderer, pTextureTitle, NULL, NULL);
		SDL_RenderPresent(gpRenderer);

		UpdateEvents();
		SDL_Delay(100);
	}

	SDL_DestroyTexture(pTextureTitle);
}

static void DrawBackground()
{
	gpSprite->Draw(gpRenderer, g_bNight ? "bg_night" : "bg_day", 0, 0);
}

static void DrawLand(bool bStatic)
{
	static unsigned int time = 0;
	if (!bStatic)
	{
		time++;
	}

	gpSprite->Draw(gpRenderer, "land", -(int)((time * 2) % SCREEN_WIDTH), SCREEN_HEIGHT - 110);
	gpSprite->Draw(gpRenderer, "land", 287 - ((time * 2) % SCREEN_WIDTH), SCREEN_HEIGHT - 110);
}

static void DrawScore(int score)
{
	int iScoreLen = 0;
	int iBeginX = SCREEN_WIDTH / 2;
	int iReverseScore = 0;

	do
	{
		if (score % 10 == 1)
		{
			iBeginX -= 16 / 2 + 1;
		}
		else
		{
			iBeginX -= 24 / 2 + 1;
		}

		iReverseScore *= 10;
		iReverseScore += score % 10;

		score /= 10;
		iScoreLen++;
	} while (score > 0);

	do
	{
		char buf[256];
		sprintf(buf, "font_%.3d", 48 + (iReverseScore % 10));

		gpSprite->Draw(gpRenderer, buf, iBeginX, 60);
		if (iReverseScore % 10 == 1)
		{
			iBeginX += 16 + 2;
		}
		else
		{
			iBeginX += 24 + 2;
		}

		iReverseScore /= 10;
		iScoreLen--;
	} while (iScoreLen > 0);
}

static void DrawScoreOnBoard(int score, int x, int y)
{
	int iScoreLen = 0;
	int iBeginX = x;
	int iReverseScore = 0;

	do
	{
		iBeginX -= 16;
		iReverseScore *= 10;
		iReverseScore += score % 10;

		score /= 10;
		iScoreLen++;
	} while (score > 0);

	do
	{
		char buf[256];
		sprintf(buf, "number_score_%.2d", iReverseScore % 10);

		gpSprite->Draw(gpRenderer, buf, iBeginX, y);
		iBeginX += 16;

		iReverseScore /= 10;
		iScoreLen--;
	} while (iScoreLen > 0);
}

static void GameThink_Initial()
{
	static unsigned int fading_start_time = 0;
	static GameState enNextGameState;

	if (fading_start_time > 0)
	{
		unsigned int elapsed = SDL_GetTicks() - fading_start_time;

		if (elapsed > 500)
		{
			g_GameState = enNextGameState;
			gpSprite->SetColorMod(255, 255, 255);
			fading_start_time = 0;
			g_bNight = ((rand() % 2) == 1);
			g_iBirdPic = rand() % 3;
			for (int i = 0; i < 3; i++)
			{
				g_iPipePosX[i] = SCREEN_WIDTH + 200 + i * PIPEDISTANCE;
				g_iPipePosY[i] = rand() % 200;
			}
			return;
		}

		elapsed *= 255;
		elapsed /= 500;

		elapsed = 255 - elapsed;

		gpSprite->SetColorMod(elapsed, elapsed, elapsed);
	}

	DrawBackground();
	DrawLand(false);

	gpSprite->Draw(gpRenderer, "title", 55, 110);

	char buf[256];
	sprintf(buf, "bird0_%d", (SDL_GetTicks() / 200) % 3);
	gpSprite->Draw(gpRenderer, buf, 118, 180 + (int)(cos(10) * 5));

	gpSprite->Draw(gpRenderer, "button_rate", 105, 275);
	gpSprite->Draw(gpRenderer, "button_play", 25, 340);
	gpSprite->Draw(gpRenderer, "button_score", 145, 340);

	gpSprite->Draw(gpRenderer, "brand_copyright", 80, 450);

	if (g_bMouseDown)
	{
		if (g_iMouseX > 25 && g_iMouseY > 340 && g_iMouseX < 25 + 100 && g_iMouseY < 340 + 55)
		{
			// user clicked "play" button
			fading_start_time = SDL_GetTicks();
			enNextGameState = GAMESTATE_GAMESTART;
		}
		else if (g_iMouseX > 145 && g_iMouseY > 340 && g_iMouseX < 145 + 100 && g_iMouseY < 340 + 55)
		{
			// user clicked "score" button
			// TODO
		}
		else if (g_iMouseX > 105 && g_iMouseY > 275 && g_iMouseX < 105 + 64 && g_iMouseY < 275 + 32)
		{
			// user clicked "rate" button
#ifdef __WINPHONE__
			RateApp();
#endif
		}
	}
}

static void BirdFly()
{
	g_flBirdVelocity = WINGPOWER;
	g_flBirdAngle = -45;
	SOUND_PlayWAV(1, g_pSfxWing);
}

static void GameThink_GameStart()
{
	static unsigned int fading_start_time = 0;

	if (fading_start_time == 0)
	{
		fading_start_time = SDL_GetTicks();
	}

	unsigned int elapsed = SDL_GetTicks() - fading_start_time;

	if (elapsed < 500)
	{
		elapsed *= 255;
		elapsed /= 500;

		gpSprite->SetColorMod(elapsed, elapsed, elapsed);
	}
	else
	{
		gpSprite->SetColorMod(255, 255, 255);
	}

	DrawBackground();
	DrawLand(false);

	/*int tickCount = SDL_GetTicks();
	tickCount = tickCount * 100;
	tickCount = tickCount % (2 * 314);
	float tick = (tickCount / 100.0f);
	if (tick > 3.14)
		tick = -2 * 3.14 + tick;*/

	char buf[256];
	sprintf(buf, "bird%d_%d", g_iBirdPic, (SDL_GetTicks() / 200) % 3);
	//g_flBirdHeight = 230;
	g_flBirdHeight = 230 + (float)(cos(SDL_GetTicks() / 2 * 3.14 / 180) * 5);
	gpSprite->Draw(gpRenderer, buf, 60, (int)g_flBirdHeight);

	// draw score
	DrawScore(0);

	// draw "get ready" notice
	gpSprite->Draw(gpRenderer, "text_ready", 50, 130);

	// draw hint picture
	gpSprite->Draw(gpRenderer, "tutorial", 90, 220);

	if (g_bMouseDown)
	{
		g_GameState = GAMESTATE_GAME;
		gpSprite->SetColorMod(255, 255, 255);
		fading_start_time = 0;
		g_iScore = 0;
		BirdFly();
	}
}

static void GameThink_Game()
{
	static bool bPrevMouseDown = false;
	bool bGameOver = false;

	static bool bPrevInRange = false;

	int i;

	g_flBirdHeight -= g_flBirdVelocity;
	g_flBirdVelocity -= GRAVITY;

	g_flBirdAngle += ROTATION;
	if (g_flBirdAngle > 85)
	{
		g_flBirdAngle = 85;
	}

	if (g_flBirdHeight < -50)
	{
		g_flBirdHeight = -50;
	}
	else if (g_flBirdHeight > SCREEN_HEIGHT - 150)
	{
		// bird has hit the ground
		g_flBirdHeight = SCREEN_HEIGHT - 150;
		bGameOver = true;
	}

	DrawBackground();

	// move pipes
	for (i = 0; i < 3; i++)
	{
		g_iPipePosX[i] -= 2;
	}

	if (g_iPipePosX[0] < -PIPEWIDTH)
	{
		g_iPipePosX[0] = g_iPipePosX[1];
		g_iPipePosX[1] = g_iPipePosX[2];
		g_iPipePosX[2] = g_iPipePosX[1] + PIPEDISTANCE;

		g_iPipePosY[0] = g_iPipePosY[1];
		g_iPipePosY[1] = g_iPipePosY[2];
		g_iPipePosY[2] = rand() % 200;
	}

	// draw pipes
	for (i = 0; i < 3; i++)
	{
		gpSprite->Draw(gpRenderer, "pipe_down", g_iPipePosX[i], -320 + 60 + g_iPipePosY[i]);
		gpSprite->Draw(gpRenderer, "pipe_up", g_iPipePosX[i], SCREEN_HEIGHT - 110 - 250 + g_iPipePosY[i]);
	}

	DrawScore(g_iScore);
	DrawLand(false);

	// draw bird
	char buf[256];
	sprintf(buf, "bird%d_%d", g_iBirdPic, (SDL_GetTicks() / 200) % 3);
	gpSprite->DrawEx(gpRenderer, buf, 60, (int)g_flBirdHeight, g_flBirdAngle, SDL_FLIP_NONE);

	// check if bird is in the range of a pipe
	if (g_iPipePosX[0] < 60 + BIRDWIDTH - BIRDMARGIN && g_iPipePosX[0] + PIPEWIDTH > 60 + BIRDMARGIN)
	{
		if (!bPrevInRange && g_iPipePosX[0] + PIPEWIDTH / 2 < 60 + BIRDMARGIN)
		{
			g_iScore++;
			SOUND_PlayWAV(0, g_pSfxPoint);
			bPrevInRange = true;
		}

		// check if the bird hits the pipe
		if (g_flBirdHeight + BIRDMARGIN < 60 + g_iPipePosY[0] ||
			g_flBirdHeight + BIRDWIDTH - BIRDMARGIN > SCREEN_HEIGHT - 110 - 250 + g_iPipePosY[0])
		{
			bGameOver = true;
		}
	}
	else
	{
		bPrevInRange = false;
	}

	if (bGameOver)
	{
		bPrevMouseDown = false;
		bPrevInRange = false;
		g_GameState = GAMESTATE_GAMEOVER;
		return;
	}

	if (g_bMouseDown && !bPrevMouseDown)
	{
		BirdFly();
	}

	bPrevMouseDown = g_bMouseDown;
}

static void GameThink_GameOver()
{
	static enum { FLASH, DROP, SHOWTITLE, SHOWSCORE } gameoverState = FLASH;
	static int time = 0;
	static bool bIsHighscore = false;
	static int fading_start_time = 0;

	if (gameoverState == FLASH)
	{
		SDL_Surface *surface = SDL_CreateRGBSurface(0, 1, 1, 32, 0x00FF0000, 0x0000FF00, 0x000000FF, 0xFF000000);
		SDL_FillRect(surface, NULL, 0xFFFFFFFF);

		SDL_Texture *texture = SDL_CreateTextureFromSurface(gpRenderer, surface);
		SDL_FreeSurface(surface);

		SDL_RenderCopy(gpRenderer, texture, NULL, NULL);
		SDL_DestroyTexture(texture);

		if (time == 0)
		{
			SOUND_PlayWAV(0, g_pSfxHit);
		}
		else if (time > 2)
		{
			gameoverState = DROP;
			time = 0;
			return;
		}
		time++;
	}
	else if (gameoverState == DROP)
	{
		if (g_flBirdHeight < SCREEN_HEIGHT - 150 || !time)
		{
			if (time == 15)
			{
				SOUND_PlayWAV(1, g_pSfxDie);
			}
			g_flBirdAngle = 85;
			g_flBirdHeight += 8;

			if (g_flBirdHeight > SCREEN_HEIGHT - 150)
			{
				g_flBirdHeight = SCREEN_HEIGHT - 150;
			}

			DrawBackground();

			// draw pipes
			for (int i = 0; i < 3; i++)
			{
				gpSprite->Draw(gpRenderer, "pipe_down", g_iPipePosX[i], -320 + 60 + g_iPipePosY[i]);
				gpSprite->Draw(gpRenderer, "pipe_up", g_iPipePosX[i], SCREEN_HEIGHT - 110 - 250 + g_iPipePosY[i]);
			}

			DrawLand(true);

			// draw bird
			char buf[256];
			sprintf(buf, "bird%d_%d", g_iBirdPic, (SDL_GetTicks() / 200) % 3);
			gpSprite->DrawEx(gpRenderer, buf, 60, (int)g_flBirdHeight, g_flBirdAngle, SDL_FLIP_NONE);

			DrawScore(g_iScore);
			time++;
		}
		else
		{
			gameoverState = SHOWTITLE;
			time = 0;
		}
	}
	else if (gameoverState == SHOWTITLE)
	{
		DrawBackground();

		// draw pipes
		for (int i = 0; i < 3; i++)
		{
			gpSprite->Draw(gpRenderer, "pipe_down", g_iPipePosX[i], -320 + 60 + g_iPipePosY[i]);
			gpSprite->Draw(gpRenderer, "pipe_up", g_iPipePosX[i], SCREEN_HEIGHT - 110 - 250 + g_iPipePosY[i]);
		}

		DrawLand(true);

		// draw bird
		char buf[256];
		sprintf(buf, "bird%d_0", g_iBirdPic);
		gpSprite->DrawEx(gpRenderer, buf, 60, (int)g_flBirdHeight, g_flBirdAngle, SDL_FLIP_NONE);

		if (time > 30)
		{
			if (time < 30 + 5)
			{
				if (time == 30 + 1)
				{
					SOUND_PlayWAV(0, g_pSfxSwooshing);
				}
				gpSprite->Draw(gpRenderer, "text_game_over", 45, 110 - (time - 30) * 6);
				time++;
			}
			else if (time < 30 + 15)
			{
				gpSprite->Draw(gpRenderer, "text_game_over", 45, 80 + (time - 30) * 3);
				time++;
			}
			else if (time < 30 + 25)
			{
				gpSprite->Draw(gpRenderer, "text_game_over", 45, 80 + 15 * 3);
				time++;
			}
			else
			{
				gpSprite->Draw(gpRenderer, "text_game_over", 45, 80 + 15 * 3);
				gameoverState = SHOWSCORE;
				time = 0;

				if (g_iScore > g_iHighScore)
				{
					g_iHighScore = g_iScore;
					bIsHighscore = true;
					SaveBestScore();
				}
			}
		}
		else
		{
			DrawScore(g_iScore);
			time++;
		}
	}
	else if (gameoverState == SHOWSCORE)
	{
		if (fading_start_time > 0)
		{
			unsigned int elapsed = SDL_GetTicks() - fading_start_time;

			if (elapsed > 500)
			{
				g_GameState = GAMESTATE_GAMESTART;
				gpSprite->SetColorMod(255, 255, 255);
				fading_start_time = 0;
				gameoverState = FLASH;
				time = 0;
				bIsHighscore = false;
				g_bNight = ((rand() % 2) == 1);
				g_iBirdPic = rand() % 3;
				for (int i = 0; i < 3; i++)
				{
					g_iPipePosX[i] = SCREEN_WIDTH + 200 + i * PIPEDISTANCE;
					g_iPipePosY[i] = rand() % 200;
				}
				return;
			}

			elapsed *= 255;
			elapsed /= 500;

			elapsed = 255 - elapsed;

			gpSprite->SetColorMod(elapsed, elapsed, elapsed);
		}

		DrawBackground();

		// draw pipes
		for (int i = 0; i < 3; i++)
		{
			gpSprite->Draw(gpRenderer, "pipe_down", g_iPipePosX[i], -320 + 60 + g_iPipePosY[i]);
			gpSprite->Draw(gpRenderer, "pipe_up", g_iPipePosX[i], SCREEN_HEIGHT - 110 - 250 + g_iPipePosY[i]);
		}

		DrawLand(true);

		// draw bird
		char buf[256];
		sprintf(buf, "bird%d_0", g_iBirdPic);
		gpSprite->DrawEx(gpRenderer, buf, 60, (int)g_flBirdHeight, g_flBirdAngle, SDL_FLIP_NONE);

		gpSprite->Draw(gpRenderer, "text_game_over", 45, 80 + 15 * 3);

		if (time < 15)
		{
			if (time == 0)
			{
				SOUND_PlayWAV(0, g_pSfxSwooshing);
			}
			gpSprite->Draw(gpRenderer, "score_panel", 31, 190 + (15 - time) * 20);
		}
		else
		{
			gpSprite->Draw(gpRenderer, "score_panel", 31, 190);
			DrawScoreOnBoard(MIN(g_iScore, (time - 15) / 2), 240, 225);
			DrawScoreOnBoard(g_iHighScore, 240, 265);

			if (bIsHighscore)
			{
				gpSprite->Draw(gpRenderer, "new", 170, 250);
			}

			if (g_iScore >= 40)
			{
				gpSprite->Draw(gpRenderer, "medals_0", 62, 235);
			}
			else if (g_iScore >= 30)
			{
				gpSprite->Draw(gpRenderer, "medals_1", 62, 235);
			}
			else if (g_iScore >= 20)
			{
				gpSprite->Draw(gpRenderer, "medals_2", 62, 235);
			}
			else if (g_iScore >= 10)
			{
				gpSprite->Draw(gpRenderer, "medals_3", 62, 235);
			}

			gpSprite->Draw(gpRenderer, "button_play", 30, 340);
			gpSprite->Draw(gpRenderer, "button_score", 150, 340);

			if (fading_start_time == 0 && g_bMouseDown)
			{
				if (g_iMouseX > 30 && g_iMouseY > 340 && g_iMouseX < 30 + 100 && g_iMouseY < 340 + 55)
				{
					// user clicked "play" button
					fading_start_time = SDL_GetTicks();
				}
				else if (g_iMouseX > 150 && g_iMouseY > 340 && g_iMouseX < 150 + 100 && g_iMouseY < 340 + 55)
				{
					// user clicked "score" button
					// TODO
				}
			}

		}

		time++;
	}
}

int GameMain()
{
	srand((unsigned int)time(NULL));
	LoadBestScore();

	gpSprite = new CSprite(gpRenderer, MakeFileName("res/atlas.bmp"), MakeFileName("res/atlas.txt"));

	atexit([](void) { delete gpSprite; });

	LoadWav();

	atexit(FreeWav);
    atexit(SOUND_CloseAudio);

	ShowTitle();

	g_GameState = GAMESTATE_INITIAL;

	unsigned int uiNextFrameTime = SDL_GetTicks();
	unsigned int uiCurrentTime = SDL_GetTicks();

	while (1)
	{
		// 60fps
		do
		{
			uiCurrentTime = SDL_GetTicks();
			UpdateEvents();
			SDL_Delay(1);
		} while (uiCurrentTime < uiNextFrameTime);

		if ((int)(uiCurrentTime - uiNextFrameTime) > 1000)
		{
			uiNextFrameTime = uiCurrentTime + 1000 / 60;
		}
		else
		{
			uiNextFrameTime += 1000 / 60;
		}

		if (g_bPauseGame)
		{
			continue;
		}

		FrameBegin();
		switch (g_GameState)
		{
		case GAMESTATE_INITIAL:
			GameThink_Initial();
			break;

		case GAMESTATE_GAMESTART:
			GameThink_GameStart();
			break;

		case GAMESTATE_GAME:
			GameThink_Game();
			break;

		case GAMESTATE_GAMEOVER:
			GameThink_GameOver();
			break;

		default:
			fprintf(stderr, "invalid game state: %d\n", (int)g_GameState);
			ExitThread(255);
		}

		FrameEnd();
	}

	return 255; // shouldn't really reach here
}
