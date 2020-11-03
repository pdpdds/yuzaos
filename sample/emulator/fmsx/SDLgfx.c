/** fMSX: portable MSX emulator ******************************/
/**                                                         **/
/**                        SDLgfx.c                         **/
/**                                                         **/
/** This file contains SDL library-dependent subroutines    **/
/** and drivers. It includes common drivers from Common.h.  **/
/**                                                         **/
/** Copyright (C) Vincent van Dam 2001-2002                 **/
/**               Marat Fayzullin 1994-2000                 **/
/**               Elan Feingold   1995                      **/
/**               Ville Hallik    1996                      **/
/**     You are not allowed to distribute this software     **/
/**     commercially. Please, notify me, if you make any    **/
/**     changes to this file.                               **/
/*************************************************************/
#ifdef SDL

/** Private #includes ****************************************/
#include "MSX.h"
#include "Sound.h"
#include "SDLfnt.h"
#include "SDLfilter.h"

/** Standard #includes ***************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef UNIX
#include <signal.h>
#endif

#include "string.h"
#include "sprintf.h"

/** SDL #includes ********************************************/
#include <SDL.h>

/** Public parameters ****************************************/
char *Title = "fMSX SDL 2.5";     /* Window/command line title */
int  UseSound = 22000;         /* Sound driver frequency    */
int  SyncFreq = 50;            /* Screen update freq. in Hz */
char *Disks[2][MAXDISKS + 1];     /* Disk names for each drive */

int UseFilter = 0;
char *filter_name[6] =
{ "\n Full scanlines \n\n","\n Half scanlines \n\n",
  "\n Mix scanlines \n\n","\n No scanlines \n\n",
  "\n Half blur \n\n","\n Full blur \n\n" };

int WIDTH = 640;
int HEIGHT = 480;

#ifdef FULLSCREEN
#define SDL_INIT       SDL_INIT_VIDEO | SDL_INIT_TIMER
#define SDL_VIDEOMODE  SDL_SWSURFACE  | SDL_FULLSCREEN
#else
#define SDL_INIT       SDL_INIT_VIDEO | SDL_INIT_TIMER
#define SDL_VIDEOMODE  SDL_SWSURFACE
#endif

/** Various variables ****************************************/
static unsigned long prev_ticks = 0;

static unsigned int BPal[256], XPal[80], XPal0;
static byte JoyState;
static byte XKeyMap[16];
static int   CurDisk[2];
static pixel *XBuf;

int UseStatic = 0;
int UseZoom = 0;
int SaveCPU = 0;

/** On-screen message related variables **********************/
static char *Message;     /* message to show                 */
static int  MessageTimer; /* duration of message in interupts*/

/** Various SDL related variables ****************************/
SDL_Surface *screen;
#define WINDOW_TITLE "fmsx"
SDL_Window* _window;
SDL_Renderer* _global_renderer;
SDL_Texture* sdl_texture_;


/** These functions are called on signals ********************/
#ifdef UNIX
static void OnBreak(int Arg) { ExitNow = 1; signal(Arg, OnBreak); }
#endif

/** Keyboard bindings ****************************************/
byte Keys[336][2] =
{
  {0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},/* 0000 */
  {7,0x20},{7,0x08},{0,0x00},{0,0x00},{0,0x00},{7,0x80},{0,0x00},{0,0x00},/* 0008 */
  {0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},/* 0010 */
  {0,0x00},{0,0x00},{0,0x00},{7,0x04},{0,0x00},{0,0x00},{0,0x00},{0,0x00},/* 0018 */
  {8,0x01},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{2,0x01},/* 0020 */
  {0,0x00},{0,0x00},{0,0x00},{0,0x00},{2,0x04},{1,0x04},{2,0x08},{2,0x10},/* 0028 */
  {0,0x01},{0,0x02},{0,0x04},{0,0x08},{0,0x10},{0,0x20},{0,0x40},{0,0x80},/* 0030 */
  {1,0x01},{1,0x02},{0,0x00},{1,0x80},{0,0x00},{1,0x08},{0,0x00},{0,0x00},/* 0038 */
  {0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},/* 0040 */
  {0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},/* 0048 */
  {0,0x00},{8,0x10},{8,0x20},{8,0x80},{8,0x40},{0,0x00},{0,0x00},{0,0x00},/* 0050 */
  {0,0x00},{0,0x00},{0,0x00},{1,0x20},{1,0x10},{1,0x40},{0,0x00},{0,0x00},/* 0058 */
  {2,0x02},{2,0x40},{2,0x80},{3,0x01},{3,0x02},{3,0x04},{3,0x08},{3,0x10},/* 0060 */
  {3,0x20},{3,0x40},{3,0x80},{4,0x01},{4,0x02},{4,0x04},{4,0x08},{4,0x10},/* 0068 */
  {4,0x20},{4,0x40},{4,0x80},{5,0x01},{5,0x02},{5,0x04},{5,0x08},{5,0x10},/* 0070 */
  {5,0x20},{5,0x40},{5,0x80},{0,0x00},{0,0x00},{0,0x00},{6,0x04},{8,0x08},/* 0078 */
  {0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},/* 0080 */
  {0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},/* 0088 */
  {0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},/* 0090 */
  {0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},/* 0098 */
  {0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},/* 00A0 */
  {0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},/* 00A8 */
  {0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},/* 00B0 */
  {0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},/* 00B8 */
  {0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},/* 00C0 */
  {0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},/* 00C8 */
  {0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},/* 00D0 */
  {8,0x02},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},/* 00D8 */
  {0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},/* 00E0 */
  {0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},/* 00E8 */
  {0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},/* 00F0 */
  {0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},/* 00F8 */
  {0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},/* 0100 */
  {0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},/* 0108 */
  {0,0x00},{8,0x20},{8,0x40},{8,0x80},{8,0x10},{8,0x04},{8,0x02},{0,0x00},/* 0110 */
  {0,0x00},{0,0x00},{6,0x20},{6,0x40},{6,0x80},{7,0x01},{7,0x02},{0,0x00},/* 0118 */
  {7,0x40},{7,0x10},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},/* 0120 */
  {0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{6,0x01},/* 0128 */
  {6,0x01},{2,0x20},{6,0x02},{6,0x10},{6,0x04},{0,0x00},{0,0x00},{0,0x00},/* 0130 */
  {0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},/* 0138 */
  {0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},/* 0140 */
  {0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00} /* 0148 */
};


/** WaitTimer() **********************************************/
/** Synchronize emulation                                   **/
/*************************************************************/
void WaitTimer(void)
{
	unsigned long ticks;
	unsigned int  delta;

	delta = UPeriod * (1000 / SyncFreq);

	/* wait until delta miliseconds elapsed */
	do ticks = SDL_GetTicks();
	while ((ticks - prev_ticks) < delta);

	prev_ticks = ticks;
}
void CheckScreenSurface()
{
	SendSerialLog("CheckScreenSurface %x %x\n", screen, screen->format);
}

/** InitMachine() ********************************************/
/** Allocate resources needed by Unix/X-dependent code.     **/
/*************************************************************/
int InitMachine(void)
{
	int J, I;

	/* Reset all variables */
	memset(XKeyMap, 0xFF, sizeof(XKeyMap));
	JoyState = 0x00;
	CurDisk[0] = CurDisk[1] = 0;

	/* Init the SDL library */
	if (Verbose)
		printf("Initializing SDL drivers:\n  Init SDL library...");
	if (SDL_Init(SDL_INIT) < 0) {
		if (Verbose) printf("FAILED\n");
		return(0);
	}
	//20191207
	//atexit(SDL_Quit);

#ifdef SKYOS32
	if (SDL_CreateWindowAndRenderer(WIDTH, HEIGHT, 0, &_window, &_global_renderer) < 0)
	{
		
	}
#else
	_window = SDL_CreateWindow(WINDOW_TITLE, 100, 100, WIDTH, HEIGHT, SDL_WINDOW_SHOWN);
	//_global_renderer = SDL_CreateRenderer(_window, -1, SDL_RENDERER_ACCELERATED);
	_global_renderer = SDL_CreateRenderer(_window, -1, 0);
	if (!_global_renderer) {

		//exit(1);
	}
#endif

	int RMask = 0x7c00;
	int GMask = 0x03e0;
	int BMask = 0x001f;

	//-msx2+ -ram 16 -vram 16 -diska 1278127358_ko_all.dsk
	screen = SDL_CreateRGBSurface(0, WIDTH, HEIGHT, 16, RMask, GMask, BMask, 0);
	
	/* Hide mouse cursor */
	SDL_ShowCursor(0);

	/* Init image buffer */
	XBuf = malloc(WIDTH*HEIGHT * sizeof(pixel));
	memset(XBuf, 0, WIDTH*HEIGHT * sizeof(pixel));

	/* Reset the palette */
	for (J = 0; J < 16; J++) XPal[J] = 0;
	XPal0 = 0;

	/* Set SCREEN8 colors */
	for (J = 0; J < 64; J++) {
		I = (J & 0x03) + (J & 0x0C) * 16 + (J & 0x30) / 2;
		XPal[J + 16] = SDL_MapRGB(screen->format,
			(J >> 4) * 255 / 3, ((J >> 2) & 0x03) * 255 / 3, (J & 0x03) * 255 / 3);
		BPal[I] = BPal[I | 0x04] = BPal[I | 0x20] = BPal[I | 0x24] = XPal[J + 16];
	}
	
	char* name = SDL_GetPixelFormatName(screen->format->format);
	
	sdl_texture_ = SDL_CreateTexture(_global_renderer, SDL_PIXELFORMAT_RGB555, SDL_TEXTUREACCESS_STREAMING, WIDTH, HEIGHT);
	
#ifdef SOUND
	/* Initialize sound */
	InitSound(UseSound, Verbose);
#endif SOUND
	
#ifdef UNIX
	/* Catch all signals */
	signal(SIGHUP, OnBreak); signal(SIGINT, OnBreak);
	signal(SIGQUIT, OnBreak); signal(SIGTERM, OnBreak);
#endif
	
	/* clear on-screen message */
	Message = NULL;
	MessageTimer = 0;
	
	return(1);
}

/** TrashMachine() *******************************************/
/** Deallocate all resources taken by InitMachine().        **/
/*************************************************************/
void TrashMachine(void)
{
	if (Verbose) printf("Shutting down...\n");

	/* Delete graphic buffer */
	free(XBuf);

#ifdef SOUND
	TrashSound();
#endif SOUND
}


/** PutMessage() *********************************************/
/** Display a message over the current screen image.        **/
/*************************************************************/
void PutMessage()
{
	int  i, j, n, c, l, v;
	int  lines = 1;
	int  curWidth = 0;
	int  width = 0;
	int  currentChar;
	int  px, py, cx;
	pixel *p;

	/* count number of lines in message, and determine the  */
	/* widest line to calculate the size of the message box */
	for (n = 0; n < strlen(Message); n++) {
		if (Message[n] == '\n') {
			curWidth = 0; lines++;
		}
		else if (++curWidth > width) width = curWidth;
	}

	/* calculate postion of message */
	px = WIDTH / 2 - (width * 4);
	py = HEIGHT / 2 - (lines * 4);
	if (px < 0) px = 0;
	if (py < 0) py = 0;

	/* insert message */
	for (n = l = 0; n < strlen(Message); n++) {

		if (py > HEIGHT) break;

		if (Message[n] == '\n') {

			for (i = 0; i < 8; i++) {
				cx = px;
				for (c = 0; c < width; c++) {
					currentChar = (c + l) < n ? fontData[(int)Message[c + l]][i] : 0;
					for (j = 0; j < 8; j++) {

						if (cx > WIDTH) break;
						if (currentChar & 128) v = 0xFFFF; else v = 0x0000;

						p = (pixel*)screen->pixels +
							cx * (screen->format->BytesPerPixel / 2) +
							py * WIDTH*(screen->format->BytesPerPixel / 2);

						*p = v;

						currentChar = currentChar << 1;
						cx++;
					}
				}
				py++;
			}
			l = n + 1;
		}
	}

}


/** PutImage() ***********************************************/
/** Put an image on the screen.                             **/
/*************************************************************/
void PutImage(void)
{

	if (SDL_MUSTLOCK(screen) && SDL_LockSurface(screen) < 0) ExitNow = 1;

	/* Apply image filter */
	if (WIDTH == 640)

		switch (UseFilter) {

		case 0: /*full_scanline(screen,&XBuf);*/
			break;
		case 1: half_scanline(screen, &XBuf);
			break;
		case 2: mix_scanline(screen, &XBuf);
			break;
		case 3: remove_scanline(screen, &XBuf);
			break;
		case 4: blur(screen, &XBuf);
			half_scanline(screen, &XBuf);
			break;
		case 5: blur(screen, &XBuf);
			mix_scanline(screen, &XBuf);
			break;
		}

	/* Copy image */
	memcpy(screen->pixels, XBuf, WIDTH*HEIGHT * sizeof(pixel));

	/* Handle on-screen message */
	if (Message != NULL) {

		PutMessage();

		if (--MessageTimer < 0) {
			free(Message);
			Message = NULL;
			MessageTimer = 0;
		}
	}

	/* Synchronize emulation */
	//WaitTimer();

	/* Lock surface */
	if (SDL_MUSTLOCK(screen)) SDL_UnlockSurface(screen);

	/* Update screen */
	//SDL_UpdateRect(screen, 0, 0, 0, 0);
	SDL_UpdateTexture(sdl_texture_, NULL, screen->pixels, screen->pitch);
	SDL_Rect srcRect;
	SDL_Rect destRect;

	srcRect.x = 0;
	srcRect.y = 0;
	srcRect.w = destRect.w = WIDTH;
	srcRect.h = destRect.h = HEIGHT;
	destRect.x = 0;
	destRect.y = 0;
	
	int REAL_SCREEN_WIDTH = WIDTH;
	int REAL_SCREEN_HEIGHT = HEIGHT;
	destRect.w = REAL_SCREEN_WIDTH;
	destRect.h = REAL_SCREEN_HEIGHT;

	SDL_RenderCopyEx(_global_renderer, sdl_texture_, &srcRect, &destRect, 0, 0, SDL_FLIP_NONE);
	SDL_RenderPresent(_global_renderer);

	/* Set timer frequency */
	SyncFreq = PALVideo ? 50 : 60;
}


/** NewMessage() *********************************************/
/** Set a new message.                                      **/
/*************************************************************/
void NewMessage(char* NewMessage)
{
	if (Message != NULL) free(Message);
	Message = NewMessage;
	MessageTimer = 50;
}

int SDL_GetOldSDLKetCodeFromKeyCode(int key)
{
	if (key == SDLK_KP_0)
		return 256;

	if (key == SDLK_KP_1)
		return 257;

	if (key == SDLK_KP_2)
		return 258;

	if (key == SDLK_KP_3)
		return 259;

	if (key == SDLK_KP_4)
		return 260;

	if (key == SDLK_KP_5)
		return 261;

	if (key == SDLK_KP_6)
		return 262;

	if (key == SDLK_KP_7)
		return 263;

	if (key == SDLK_KP_8)
		return 264;

	if (key == SDLK_KP_9)
		return 265;

	if (key == SDLK_KP_PERIOD)
		return 266;

	if (key == SDLK_KP_DIVIDE)
		return 267;
		
	if (key == SDLK_KP_MULTIPLY)
		return 268;

	if (key == SDLK_KP_MINUS)
		return 269;

	if (key == SDLK_KP_PLUS)
		return 270;

	if (key == SDLK_KP_ENTER)
		return 271;

	if (key == SDLK_KP_EQUALS)
		return 272;

	if (key == SDLK_UP)
		return 273;

	if (key == SDLK_DOWN)
		return 274;

	if (key == SDLK_RIGHT)
		return 275;

	if (key == SDLK_LEFT)
		return 276;

	if (key == SDLK_INSERT)
		return 277;

	if (key == SDLK_HOME)
		return 278;

	if (key == SDLK_END)
		return 279;

	if (key == SDLK_PAGEUP)
		return 280;

	if (key == SDLK_PAGEDOWN)
		return 281;

	if (key == SDLK_F1)
		return 282;

	if (key == SDLK_F2)
		return 283;

	if (key == SDLK_F3)
		return 284;

	if (key == SDLK_F4)
		return 285;

	if (key == SDLK_F5)
		return 286;

	if (key == SDLK_F6)
		return 287;

	if (key == SDLK_F7)
		return 288;

	if (key == SDLK_F8)
		return 289;

	if (key == SDLK_F9)
		return 290;

	if (key == SDLK_F10)
		return 291;

	if (key == SDLK_F11)
		return 292;

	if (key == SDLK_F12)
		return 293;

	if (key == SDLK_F13)
		return 294;

	if (key == SDLK_F14)
		return 295;

	if (key == SDLK_F15)
		return 296;

	if (key == SDLK_NUMLOCKCLEAR)
		return 300;

	if (key == SDLK_CAPSLOCK)
		return 301;

	if (key == SDLK_SCROLLLOCK)
		return 302;

	if (key == SDLK_RSHIFT)
		return 303;

	if (key == SDLK_LSHIFT)
		return 304;

	if (key == SDLK_RCTRL)
		return 305;

	if (key == SDLK_LCTRL)
		return 306;

	if (key == SDLK_RALT)
		return 307;

	if (key == SDLK_LALT)
		return 308;

	if (key == SDLK_MODE)
		return 313;

	if (key == SDLK_HELP)
		return 315;

	if (key == SDLK_SYSREQ)
		return 317;

	if (key == SDLK_MENU)
		return 319;

	if (key == SDLK_POWER)
		return 320;
	
	if (key == SDLK_UNDO)
		return 322;

	//SDLK_RMETA		= 309,
	//SDLK_LMETA = 310,
	//SDLK_COMPOSE		= 314,		/**< Multi-key compose key */
	//SDLK_PRINT
	//SDLK_BREAK
	//SDLK_EURO		
	return -1;
}

/** Keyboard() ***********************************************/
/** Check for keyboard events, parse them, and modify MSX   **/
/** keyboard matrix.                                        **/
/*************************************************************/
void Keyboard(void)
{

	SDL_Event event;
	static byte Control = 0;
	int key, I;

	/* Check for keypresses/keyreleases */
	while (SDL_PollEvent(&event)) {

		key = event.key.keysym.sym;
		int oldKeyCode = SDL_GetOldSDLKetCodeFromKeyCode(key);

		if (event.type == SDL_QUIT) ExitNow = 1;

		/***************/
		/* KEY PRESSED */
		/***************/

		if (event.type == SDL_KEYDOWN) {
			
			switch (key)
			{

				/******************************/
				/* F6: SWITCH AUTOFIRE ON/OFF */
				/******************************/

			case SDLK_F6:
				if (Control) {
					AutoFire = !AutoFire;
					//if (AutoFire) NewMessage(_strdup("\n Autofire On \n\n"));
					//else NewMessage(_strdup("\n Autofire Off \n\n"));
				}
				break;


				/************************/
				/* CTRL+F7: TAPE REWIND */
				/************************/

			case SDLK_F7:
				if (Control) {
					RewindTape();
					//NewMessage(_strdup("\n Tape Rewound \n\n"));
				}
				break;


				/***********************/
				/* F9: LOAD STATE      */
				/* CTRL+F9: SAVE STATE */
				/***********************/

			case SDLK_F9:
				if (StateName) {
					char StateNotification[512];
					sprintf(StateNotification, "\n State %s %s \n\n",
						StateName, Control ? "saved" : "loaded");
					if (Control) SaveState(StateName); else LoadState(StateName);
					//NewMessage(_strdup(StateNotification));
				}
				break;


				/******************************/
				/* END: SWITCH IMAGE FILTER   */
				/* CTRL+END: FLIP STEREO/MONO */
				/******************************/

			case SDLK_END:
				if (Control) {
					if (UseStereo) {
						UseStereo = 0;
						//NewMessage(strdup("\n Mono \n\n"));
					}
					else {
						UseStereo = 1;
						//NewMessage(strdup("\n Stereo \n\n"));
					}
				}
				else {
					if (WIDTH == 640) {
						UseFilter++;
						if (UseFilter > 5) UseFilter = 0;
						//NewMessage(strdup(filter_name[UseFilter]));
						full_scanline(screen, &XBuf);/* clear scanlines */
					}
				}
				break;


				/**************************/
				/* CTRL+F10: SWITCH DISKA */
				/* CTRL+F11: SWITCH DISKB */
				/**************************/

			case SDLK_F10:
			case SDLK_F11:
				if (Control) {
					I = (key == SDLK_F11) ? 1 : 0;
					if (Disks[I][0]) {
						char DiskNotification[512];
						CurDisk[I] = (CurDisk[I] + 1) % MAXDISKS;
						if (!Disks[I][CurDisk[I]]) CurDisk[I] = 0;
						ChangeDisk(I, Disks[I][CurDisk[I]]);
						/* notify user */
						sprintf(DiskNotification,
							"\n Disk %c: %s \n\n",
							'A' + I, Disks[I][CurDisk[I]]);
						//NewMessage(_strdup(DiskNotification));
					}
				}
				break;


				/**********************/
				/* F12: EXIT EMULATOR */
				/* CTRL+F12: DEBUGER  */
				/**********************/

			case SDLK_F12:
#ifdef DEBUG
				if (Control) CPU.Trace = !CPU.Trace;
				else
#endif
					ExitNow = 1;
				break;


				/***************************/
				/* SCROLL-LOCK: RESET      */
				/* CTRL+SCROLL: HARD RESET */
				/***************************/

			case SDLK_SCROLLLOCK:
				if (Control)
					ResetMSX(1);
				else
					ResetMSX(0);
				break;


				/********************************/
				/* CURSOR KEYS: CURSOR MOVEMENT */
				/********************************/

			case SDLK_UP:    JoyState |= 0x01; break;
			case SDLK_DOWN:  JoyState |= 0x02; break;
			case SDLK_LEFT:  JoyState |= 0x04; break;
			case SDLK_RIGHT: JoyState |= 0x08; break;
			case SDLK_LALT:  JoyState |= 0x10; break;
			case SDLK_LCTRL:
				if (key == SDLK_LCTRL) JoyState |= 0x20;
				Control = 1;
				break;
			}

			/***********************************/
			/* OTHER KEYS: reset bit in KeyMap */
			/***********************************/
			if(oldKeyCode != -1)
				XKeyMap[Keys[oldKeyCode][0]] &= ~Keys[oldKeyCode][1];
			if (key < 0x150) 
				XKeyMap[Keys[key][0]] &= ~Keys[key][1];

		}

		/****************/
		/* KEY RELEASED */
		/****************/

		if (event.type == SDL_KEYUP) {
			
			/* Special keys released... */
			switch (key)
			{
			case SDLK_UP:    JoyState &= 0xFE; break;
			case SDLK_DOWN:  JoyState &= 0xFD; break;
			case SDLK_LEFT:  JoyState &= 0xFB; break;
			case SDLK_RIGHT: JoyState &= 0xF7; break;
			case SDLK_LALT:  JoyState &= 0xEF; break;
			case SDLK_LCTRL: JoyState &= 0xDF; Control = 0; break;
			}

			/* Key released: set bit in KeyMap */			
			if (oldKeyCode != -1)
				XKeyMap[Keys[oldKeyCode][0]] |= Keys[oldKeyCode][1];
			if (key < 0x150)
				XKeyMap[Keys[key][0]] |=  Keys[key][1];
		}
	}

	/* Copy local keymap to the global one */
	memcpy(KeyMap, XKeyMap, sizeof(XKeyMap));
}

/** Joystick() ***********************************************/
/** Query position of a joystick connected to port N.       **/
/** Returns 0.0.F2.F1.R.L.D.U.                              **/
/*************************************************************/
byte Joystick(register byte N) { return(JoyState); }

/** Mouse() **************************************************/
/** Query coordinates of a mouse connected to port N.       **/
/** Returns F2.F1.Y.Y.Y.Y.Y.Y.Y.Y.X.X.X.X.X.X.X.X.          **/
/*************************************************************/
int Mouse(register byte N)
{
	return 0;
}

/** SetColor() ***********************************************/
/** Set color N (0..15) to R,G,B.                           **/
/*************************************************************/
void SetColor(byte N, byte R, byte G, byte B)
{
	unsigned int J = 0;
	J = SDL_MapRGB(screen->format, R, G, B);

	if (N) XPal[N] = J; else XPal0 = J;
}

#ifdef NARROW
#include "Common.h"
#endif
#include "Hires.h"

#endif /* SDL */
