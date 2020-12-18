#include "SDLSingleton.h"
#include <SDL.h>
#include <SDL_ttf.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>


const char *CAPTION = "CSCI X070 - projS - jiva";
const char *IMG_bg = "data/background2.bmp";
const char *IMG_pacman_up = "data/pacman-U.bmp";
const char *IMG_pacman_down = "data/pacman-D.bmp";
const char *IMG_pacman_left = "data/pacman-L.bmp";
const char *IMG_pacman_right = "data/pacman-R.bmp";
const char *IMG_small_yummy = "data/small-yummy.bmp";
const char *IMG_big_yummy = "data/big-yummy.bmp";
const char *IMG_redg = "data/redghost.bmp";
const char *IMG_cyang = "data/cyanghost.bmp";
const char *IMG_pinkg = "data/pinkghost.bmp";
const char *IMG_vulng = "data/vulnghost.bmp";
const int pacvelocity = 1;
#define S 0
#define L 1
#define R 2
#define U 3
#define D 4

/* Globals */
SDL_Surface *sur_screen = NULL;
SDL_Surface *sur_bg = NULL;
SDL_Surface *sur_pacman_up = NULL;
SDL_Surface *sur_pacman_down = NULL;
SDL_Surface *sur_pacman_left = NULL;
SDL_Surface *sur_pacman_right = NULL;
SDL_Surface *sur_small_yummy = NULL;
SDL_Surface *sur_big_yummy = NULL;
SDL_Surface *sur_redg = NULL;
SDL_Surface *sur_cyang = NULL;
SDL_Surface *sur_pinkg = NULL;
SDL_Surface *sur_vulng = NULL;
SDL_Rect pacman;
SDL_Rect ghosts[3];
int prev[3];
SDL_Rect teleport[2];
SDL_Rect barriers[41];
SDL_Rect syummy[263];
int syum = 263;
SDL_Rect byummy[4];
int byum = 4;
int pacmoving;
int movewanted;
int score;
SDL_TimerID timer_moveghosts;
SDL_TimerID timer_movepacman;
SDL_TimerID timer_updategstat;
SDL_TimerID timer_vulntimer;
int lives = 3;
int level = 1;
int who = 0;
bool killed = false;
int yeat = 0;
bool levelup = false;
#include <sprintf.h>

TTF_Font * font;
TTF_Font * font2;

//Mix_Chunk *snd_intro = NULL;
//Mix_Chunk *snd_chomp = NULL;
//Mix_Chunk *snd_pacdied = NULL;
//Mix_Chunk *snd_ghostdied = NULL;

/* Function prototypes */
void init_clerical();
void init_game();
void shutdown();
void game();
void draw(SDL_Surface *sur, int x, int y, int mode);
void text(char *str, int x, int y, int mode);
void clear(SDL_Surface *sur, int x, int y, int w, int h, int flip);
bool cd(SDL_Rect *obj1, SDL_Rect *obj2);
bool cd(int x1, int y1, int w1, int h1, int x2, int y2, int w2, int h2);
int playsound(int effect, int halt);
void move(int moving, int movewanted);
void modghost(int ghostnum, int x, int y, int mod, int dir, int def);
void drawg(int ghostnum);
int getranddir(int ghostnum, int nodir);
void updatelives();
int oppdir(int dir);
int adjdir(int dir);
void vulnOn();
void vulnOff();
Uint32 vulntimer(Uint32 interval, void *param);

/* Initialize clerical related things */
void init_clerical()
{
	if (false == SDLSingleton::GetInstance()->InitSystem(WINDOW_WIDTH, WINDOW_HEIGHT))
	{
		exit(1);
	}

	sur_screen = SDLSingleton::GetInstance()->GetSurface();
	if(sur_screen == NULL)
	{
		printf("Unable to set resolution and bit depth: %s\n", SDL_GetError());
		exit(1);
	}

	/* Load images */
	sur_bg = SDL_LoadBMP(IMG_bg);
	if(sur_bg == NULL)
	{
		printf("Unable to load image: %s\n", SDL_GetError());
		exit(1);
	}
	sur_pacman_up = SDL_LoadBMP(IMG_pacman_up);
	if(sur_pacman_up == NULL)
	{
		printf("Unable to load image: %s\n", SDL_GetError());
		exit(1);
	}
	sur_pacman_down = SDL_LoadBMP(IMG_pacman_down);
	if(sur_pacman_down == NULL)
	{
		printf("Unable to load image: %s\n", SDL_GetError());
		exit(1);
	}
	sur_pacman_left = SDL_LoadBMP(IMG_pacman_left);
	if(sur_pacman_left == NULL)
	{
		printf("Unable to load image: %s\n", SDL_GetError());
		exit(1);
	}
	sur_pacman_right = SDL_LoadBMP(IMG_pacman_right);
	if(sur_pacman_right == NULL)
	{
		printf("Unable to load image: %s\n", SDL_GetError());
		exit(1);
	}
	sur_small_yummy = SDL_LoadBMP(IMG_small_yummy);
	if(sur_small_yummy == NULL)
	{
		printf("Unable to load image: %s\n", SDL_GetError());
		exit(1);
	}
	sur_big_yummy = SDL_LoadBMP(IMG_big_yummy);
	if(sur_big_yummy == NULL)
	{
		printf("Unable to load image: %s\n", SDL_GetError());
		exit(1);
	}
	sur_redg = SDL_LoadBMP(IMG_redg);
	if(sur_redg == NULL)
	{
		printf("Unable to load image: %s\n", SDL_GetError());
		exit(1);
	}
	sur_pinkg = SDL_LoadBMP(IMG_pinkg);
	if(sur_pinkg == NULL)
	{
		printf("Unable to load image: %s\n", SDL_GetError());
		exit(1);
	}
	sur_cyang = SDL_LoadBMP(IMG_cyang);
	if(sur_cyang == NULL)
	{
		printf("Unable to load image: %s\n", SDL_GetError());
		exit(1);
	}
	sur_vulng = SDL_LoadBMP(IMG_vulng);
	if(sur_vulng == NULL)
	{
		printf("Unable to load image: %s\n", SDL_GetError());
		exit(1);
	}

	/* Initialize SDL ttf */
	if(TTF_Init() == -1)
	{
		printf("Unable to initialize SDL_ttf: %s \n", TTF_GetError());
		exit(1);
	}
	/* Load font */
	font2 = TTF_OpenFont("data/KOMIKAX_.ttf", 10);
	if(font2 == NULL)
	{
		printf("Unable to load font: %s %s \n", "data/KOMIKAX_.ttf", TTF_GetError());
		exit(1);
	}
	/* Load font */
	font = TTF_OpenFont("data/KOMIKAX_.ttf", 24);
	if(font == NULL)
	{
		printf("Unable to load font: %s %s \n", "data/KOMIKAX_.ttf", TTF_GetError());
		exit(1);
	}

	/* SDL Mixer */
	/*if(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) != 0)
	{
		printf("Unable to initialize SDL: %s\n", SDL_GetError());
		exit(1);
	}*/
	int audio_rate = 22050;
	Uint16 audio_format = AUDIO_S16SYS;
	int audio_channels = 2;
	int audio_buffers = 4096;
	/*if(Mix_OpenAudio(audio_rate, audio_format, audio_channels, audio_buffers) != 0)
	{
		fprintf(stderr, "Unable to initialize audio: %s\n", Mix_GetError());
		exit(1);
	}
	snd_intro = Mix_LoadWAV("data/sounds/wav/intro.wav");
	if(snd_intro == NULL)
	{
		fprintf(stderr, "Unable to load WAV file: %s\n", Mix_GetError());
		exit(1);
	}
	snd_chomp = Mix_LoadWAV("data/sounds/wav/chomping.wav");
	if(snd_chomp == NULL)
	{
		fprintf(stderr, "Unable to load WAV file: %s\n", Mix_GetError());
		exit(1);
	}
	snd_pacdied = Mix_LoadWAV("data/sounds/wav/pacmandied.wav");
	if(snd_pacdied == NULL)
	{
		fprintf(stderr, "Unable to load WAV file: %s\n", Mix_GetError());
		exit(1);
	}
	snd_ghostdied = Mix_LoadWAV("data/sounds/wav/ghostdied.wav");
	if(snd_ghostdied == NULL)
	{
		fprintf(stderr, "Unable to load WAV file: %s\n", Mix_GetError());
		exit(1);
	}*/

}

/* Initialize game related things */
void init_game()
{
	/* Seed the random number generator */
	srand(time(NULL));

	/* Draw background */
	draw(sur_bg, 0, 0, 3);
	
	/* Init the ghosts */
	modghost(0, 0, 0, 2, S, 1);
	modghost(1, 0, 0, 2, S, 1);
	modghost(2, 0, 0, 2, S, 1);
	
	/* Init prev direction */
	prev[0] = L;
	prev[1] = S;
	prev[2] = S;
	
	/* Set pacman's position */
	pacman.x = 234;
	pacman.y = 388;
	pacman.w = 30;
	pacman.h = 30;
	
	/* Draw pacman */
	draw(sur_pacman_left, pacman.x, pacman.y, 3);
	
	/* Initialize teleport points */
	teleport[0].x = 29;
	teleport[0].y = 240;
	teleport[0].w = 1;
	teleport[0].h = 30;
	teleport[1].x = 470;
	teleport[1].y = 240;
	teleport[1].w = 1;
	teleport[1].h = 30;
	
	/* Initialize barriers */
	barriers[0].x = 0;		barriers[0].y = 0;		barriers[0].w = 0;		barriers[0].h = 0;
	barriers[1].x = 59;		barriers[1].y = 56;		barriers[1].w = 52;		barriers[1].h = 36;
	barriers[2].x = 141;	barriers[2].y = 56;		barriers[2].w = 69;		barriers[2].h = 36;
	barriers[3].x = 240;	barriers[3].y = 26;		barriers[3].w = 19;		barriers[3].h = 66;
	barriers[4].x = 289;	barriers[4].y = 56;		barriers[4].w = 69;		barriers[4].h = 36;
	barriers[5].x = 388;	barriers[5].y = 56;		barriers[5].w = 52;		barriers[5].h = 36;
	barriers[6].x = 59;		barriers[6].y = 122;	barriers[6].w = 52;		barriers[6].h = 19;
	barriers[7].x = 141;	barriers[7].y = 122;	barriers[7].w = 19;		barriers[7].h = 118;
	barriers[8].x = 160;	barriers[8].y = 171;	barriers[8].w = 50;		barriers[8].h = 19;
	barriers[9].x = 190;	barriers[9].y = 122;	barriers[9].w = 118;	barriers[9].h = 19;
	barriers[10].x = 240;	barriers[10].y = 141;	barriers[10].w = 19;	barriers[10].h = 49;
	barriers[11].x = 338;	barriers[11].y = 122;	barriers[11].w = 20;	barriers[11].h = 118;
	barriers[12].x = 289;	barriers[12].y = 171;	barriers[12].w = 49;	barriers[12].h = 19;
	barriers[13].x = 388;	barriers[13].y = 122;	barriers[13].w = 52;	barriers[13].h = 19;
	barriers[14].x = 18;	barriers[14].y = 171;	barriers[14].w = 93;	barriers[14].h = 69;
	barriers[15].x = 388;	barriers[15].y = 171;	barriers[15].w = 93;	barriers[15].h = 69;
	barriers[16].x = 190;	barriers[16].y = 220;	barriers[16].w = 118;	barriers[16].h = 69;
	barriers[17].x = 18;	barriers[17].y = 270;	barriers[17].w = 93;	barriers[17].h = 68;
	barriers[18].x = 141;	barriers[18].y = 270;	barriers[18].w = 19;	barriers[18].h = 68;
	barriers[19].x = 190;	barriers[19].y = 319;	barriers[19].w = 118;	barriers[19].h = 19;
	barriers[20].x = 240;	barriers[20].y = 338;	barriers[20].w = 19;	barriers[20].h = 50;
	barriers[21].x = 338;	barriers[21].y = 270;	barriers[21].w = 20;	barriers[21].h = 68;
	barriers[22].x = 388;	barriers[22].y = 270;	barriers[22].w = 93;	barriers[22].h = 68;
	barriers[23].x = 59;	barriers[23].y = 368;	barriers[23].w = 52;	barriers[23].h = 19;
	barriers[24].x = 92;	barriers[24].y = 388;	barriers[24].w = 19;	barriers[24].h = 49;
	barriers[25].x = 141;	barriers[25].y = 368;	barriers[25].w = 69;	barriers[25].h = 20;
	barriers[26].x = 289;	barriers[26].y = 368;	barriers[26].w = 69;	barriers[26].h = 20;
	barriers[27].x = 388;	barriers[27].y = 368;	barriers[27].w = 52;	barriers[27].h = 19;
	barriers[28].x = 388;	barriers[28].y = 387;	barriers[28].w = 19;	barriers[28].h = 50;
	barriers[29].x = 29;	barriers[29].y = 417;	barriers[29].w = 33;	barriers[29].h = 20;
	barriers[30].x = 59;	barriers[30].y = 467;	barriers[30].w = 151;	barriers[30].h = 19;
	barriers[31].x = 141;	barriers[31].y = 418;	barriers[31].w = 20;	barriers[31].h = 49;
	barriers[32].x = 191;	barriers[32].y = 418;	barriers[32].w = 117;	barriers[32].h = 19;
	barriers[33].x = 240;	barriers[33].y = 437;	barriers[33].w = 19;	barriers[33].h = 49;
	barriers[34].x = 338;	barriers[34].y = 418;	barriers[34].w = 20;	barriers[34].h = 49;
	barriers[35].x = 289;	barriers[35].y = 467;	barriers[35].w = 151;	barriers[35].h = 19;
	barriers[36].x = 437;	barriers[36].y = 417;	barriers[36].w = 33;	barriers[36].h = 20;
	barriers[37].x = 0;		barriers[37].y = 0;		barriers[37].w = 500;	barriers[37].h = 26;
	barriers[38].x = 0;		barriers[38].y = 26;	barriers[38].w = 29;	barriers[38].h = 490;
	barriers[39].x = 0;		barriers[39].y = 516;	barriers[39].w = 500;	barriers[39].h = 59;
	barriers[40].x = 470;	barriers[40].y = 15;	barriers[40].w = 30;	barriers[40].h = 501;
	
	/* Initialize small yummies */
	int shift = 17;
	int c = 0;
	int x = 0;
	for(int i = 0; i < 26; i++)
	{
		syummy[i].x = shift*c+35;
		syummy[i].y = 500;
		syummy[i].h = 1; //displayed 1 or not displayed 0
		c++;
	}
	c = 0;
	for(int i = 26; i < 52; i++)
	{
		syummy[i].x = shift*c+35;
		syummy[i].y = 106;
		syummy[i].h = 1; //displayed 1 or not displayed 0
		c++;
	}
	c = 0;
	for(int i = 52; i < 64; i++)
	{
		syummy[i].x = shift*c+35; //42
		syummy[i].y = 40;
		syummy[i].h = 1; //displayed 1 or not displayed 0
		c++;
	}
	c = 0;
	for(int i = 64; i < 76; i++)
	{
		syummy[i].x = shift*c+272;
		syummy[i].y = 40;
		syummy[i].h = 1; //displayed 1 or not displayed 0
		c++;
	}
	c = 0;
	for(int i = 76; i < 88; i++)
	{
		syummy[i].x = shift*c+35; //42
		syummy[i].y = 353;
		syummy[i].h = 1; //displayed 1 or not displayed 0
		c++;
	}
	c = 0;
	for(int i = 88; i < 100; i++)
	{
		syummy[i].x = shift*c+272;
		syummy[i].y = 353;
		syummy[i].h = 1; //displayed 1 or not displayed 0
		c++;
	}
	c = 0;
	for(int i = 100; i < 109; i++)
	{
		syummy[i].x = shift*c+35;
		syummy[i].y = 255;
		syummy[i].h = 1; //displayed 1 or not displayed 0
		c++;
	}
	c = 0;
	for(int i = 109; i < 118; i++)
	{
		syummy[i].x = shift*c+324;
		syummy[i].y = 255;
		syummy[i].h = 1; //displayed 1 or not displayed 0
		c++;
	}
	c = 0;
	x = 118;
	for(int i = 0; i < 26; i++)
	{
		if(c == 6 || c == 7 || c == 12 || c == 13 || c == 18 || c == 19)
		{
			c++;
			continue;
		}
		
		syummy[x].x = shift*c+35;
		syummy[x].y = 155;
		syummy[x].h = 1; //displayed 1 or not displayed 0
		c++;
		x++;
	}
	c = 0;
	x = 138;
	for(int i = 0; i < 26; i++)
	{
		if(c == 6 || c == 7 || c == 12 || c == 13 || c == 18 || c == 19)
		{
			c++;
			continue;
		}
		
		syummy[x].x = shift*c+35;
		syummy[x].y = 452;
		syummy[x].h = 1; //displayed 1 or not displayed 0
		c++;
		x++;
	}
	c = 0;
	x = 158;
	for(int i = 0; i < 26; i++)
	{
		if(c == 4 || c == 12 || c == 13 || c == 21)
		{
			c++;
			continue;
		}
		
		syummy[x].x = shift*c+35;
		syummy[x].y = 402;
		syummy[x].h = 1; //displayed 1 or not displayed 0
		c++;
		x++;
	}
	c = 0;
	x = 180;
	for(int i = 0; i < 26; i++)
	{
		if(c == 0 || c == 1 || c == 2 || c == 3 || c == 4 || c == 6 || c == 7 || c == 18 || c == 19 || c == 21 || c == 22 || c == 23 || c == 24 || c == 25)
		{
			c++;
			continue;
		}
		
		syummy[x].x = shift*c+35;
		syummy[x].y = 205;
		syummy[x].h = 1; //displayed 1 or not displayed 0
		c++;
		x++;
	}
	c = 0;
	x = 192;
	for(int i = 0; i < 26; i++)
	{
		if(c == 0 || c == 1 || c == 2 || c == 3 || c == 4 || c == 6 || c == 7 || c == 18 || c == 19 || c == 21 || c == 22 || c == 23 || c == 24 || c == 25)
		{
			c++;
			continue;
		}
		
		syummy[x].x = shift*c+35;
		syummy[x].y = 303;
		syummy[x].h = 1; //displayed 1 or not displayed 0
		c++;
		x++;
	}
	c = 0;
	x = 204;
	for(int i = 0; i < 26; i++)
	{
		if(c == 0 || c == 1 || c == 2 || c == 3 || c == 4 || c == 6 || c == 7 || c == 18 || c == 19 || c == 21 || c == 22 || c == 23 || c == 24 || c == 25 || c == 9 || c == 10 || c == 11 || c == 12 || c == 13 || c == 14 || c == 15 || c == 16)
		{
			c++;
			continue;
		}
		
		syummy[x].x = shift*c+35;
		syummy[x].y = 280;
		syummy[x].h = 1; //displayed 1 or not displayed 0
		c++;
		x++;
	}
	c = 0;
	x = 208; //-212
	for(int i = 0; i < 26; i++)
	{
		if(c == 0 || c == 1 || c == 2 || c == 3 || c == 4 || c == 6 || c == 7 || c == 18 || c == 19 || c == 21 || c == 22 || c == 23 || c == 24 || c == 25 || c == 9 || c == 10 || c == 11 || c == 12 || c == 13 || c == 14 || c == 15 || c == 16)
		{
			c++;
			continue;
		}
		
		syummy[x].x = shift*c+35;
		syummy[x].y = 230;
		syummy[x].h = 1; //displayed 1 or not displayed 0
		c++;
		x++;
	}
	c = 0;
	x = 212; //-216
	for(int i = 0; i < 26; i++)
	{
		if(c == 0 || c == 1 || c == 2 || c == 3 || c == 4 || c == 6 || c == 7 || c == 18 || c == 19 || c == 21 || c == 22 || c == 23 || c == 24 || c == 25 || c == 9 || c == 10 || c == 11 || c == 12 || c == 13 || c == 14 || c == 15 || c == 16)
		{
			c++;
			continue;
		}
		
		syummy[x].x = shift*c+35;
		syummy[x].y = 326;
		syummy[x].h = 1; //displayed 1 or not displayed 0
		c++;
		x++;
	}
	c = 0;
	x = 216; //-222
	for(int i = 0; i < 26; i++)
	{
		if(	c == 2 || c == 3 || c == 4 || c == 5 || c == 6 || c == 7 || c == 8 || c == 9 || c == 10 || 
			c == 12 || c == 13 || c == 15 || c == 16 || c == 17 || c == 18 || c == 19 || c == 20 || c == 21 || c == 22 || c == 23)
		{
			c++;
			continue;
		}
		
		syummy[x].x = shift*c+35;
		syummy[x].y = 477;
		syummy[x].h = 1; //displayed 1 or not displayed 0
		c++;
		x++;
	}
	c = 0;
	x = 222; //-232
	for(int i = 0; i < 26; i++)
	{
		if(	c == 0 || c == 1 || c == 4 || c == 7 || c == 10 || c == 11 || c == 12 || c == 13 || c == 14 || c == 15 || c == 16 || c == 18 || c == 19 || c == 21 || c == 24 || c == 25)
		{
			c++;
			continue;
		}
		
		syummy[x].x = shift*c+35;
		syummy[x].y = 425;
		syummy[x].h = 1; //displayed 1 or not displayed 0
		c++;
		x++;
	}
	c = 0;
	x = 232; //-241
	for(int i = 0; i < 26; i++)
	{
		if(	c == 2 || c == 3 || c == 4 ||
			c == 7 || c == 8 || c == 9 || c == 10 ||
			c == 12 || c == 13 ||
			c == 15 || c == 16 || c == 17 || c == 18 || c == 19 || 
			c == 21 || c == 22 || c == 23)
		{
			c++;
			continue;
		}
		
		syummy[x].x = shift*c+35;
		syummy[x].y = 377;
		syummy[x].h = 1; //displayed 1 or not displayed 0
		c++;
		x++;
	}
	c = 0;
	x = 241; //-245
	for(int i = 0; i < 26; i++)
	{
		if(	c >= 0 && c <= 4 ||
			c >= 6 && c <= 10 ||
			c >= 12 && c <= 13 ||
			c >= 15 && c <= 19 ||
			c >= 21 && c <= 25)
		{
			c++;
			continue;
		}
		
		syummy[x].x = shift*c+35;
		syummy[x].y = 179;
		syummy[x].h = 1; //displayed 1 or not displayed 0
		c++;
		x++;
	}
	c = 0;
	x = 245; //-254
	for(int i = 0; i < 26; i++)
	{
		if(	c >= 2 && c <= 4 ||
			c >= 7 && c <= 10 ||
			c >= 12 && c <= 13 ||
			c >= 15 && c <= 19 ||
			c >= 21 && c <= 23)
		{
			c++;
			continue;
		}
		
		syummy[x].x = shift*c+35;
		syummy[x].y = 73;
		syummy[x].h = 1; //displayed 1 or not displayed 0
		c++;
		x++;
	}
	c = 0;
	x = 254; //-263
	for(int i = 0; i < 26; i++)
	{
		if(	c >= 2 && c <= 4 ||
			c == 7 ||
			c >= 9 && c <= 16 ||
			c >= 18 && c <= 19 ||
			c >= 21 && c <= 23)
		{
			c++;
			continue;
		}
		
		syummy[x].x = shift*c+35;
		syummy[x].y = 131;
		syummy[x].h = 1; //displayed 1 or not displayed 0
		c++;
		x++;
	}
	
	for(int i = 0; i < 26; i++)
	{
		draw(sur_small_yummy, syummy[i].x, syummy[i].y, 1);
	}
	for(int i = 26; i < 52; i++)
	{
		draw(sur_small_yummy, syummy[i].x, syummy[i].y, 1);
	}
	for(int i = 52; i < 64; i++)
	{
		draw(sur_small_yummy, syummy[i].x, syummy[i].y, 1);
	}
	for(int i = 64; i < 76; i++)
	{
		draw(sur_small_yummy, syummy[i].x, syummy[i].y, 1);
	}
	for(int i = 76; i < 88; i++)
	{
		draw(sur_small_yummy, syummy[i].x, syummy[i].y, 1);
	}
	for(int i = 88; i < 100; i++)
	{
		draw(sur_small_yummy, syummy[i].x, syummy[i].y, 1);
	}
	for(int i = 100; i < 109; i++)
	{
		draw(sur_small_yummy, syummy[i].x, syummy[i].y, 1);
	}
	for(int i = 109; i < 118; i++)
	{
		draw(sur_small_yummy, syummy[i].x, syummy[i].y, 1);
	}
	for(int i = 118; i < 138; i++)
	{
		draw(sur_small_yummy, syummy[i].x, syummy[i].y, 1);
	}
	for(int i = 138; i < 158; i++)
	{
		draw(sur_small_yummy, syummy[i].x, syummy[i].y, 1);
	}
	for(int i = 158; i < 180; i++)
	{
		draw(sur_small_yummy, syummy[i].x, syummy[i].y, 1);
	}
	for(int i = 180; i < 192; i++)
	{
		draw(sur_small_yummy, syummy[i].x, syummy[i].y, 1);
	}
	for(int i = 192; i < 204; i++)
	{
		draw(sur_small_yummy, syummy[i].x, syummy[i].y, 1);
	}
	for(int i = 204; i < 208; i++)
	{
		draw(sur_small_yummy, syummy[i].x, syummy[i].y, 1);
	}
	for(int i = 208; i < 212; i++)
	{
		draw(sur_small_yummy, syummy[i].x, syummy[i].y, 1);
	}
	for(int i = 212; i < 216; i++)
	{
		draw(sur_small_yummy, syummy[i].x, syummy[i].y, 1);
	}
	for(int i = 216; i < 222; i++)
	{
		draw(sur_small_yummy, syummy[i].x, syummy[i].y, 1);
	}
	for(int i = 222; i < 232; i++)
	{
		draw(sur_small_yummy, syummy[i].x, syummy[i].y, 1);
	}
	for(int i = 232; i < 241; i++)
	{
		draw(sur_small_yummy, syummy[i].x, syummy[i].y, 1);
	}
	for(int i = 241; i < 245; i++)
	{
		draw(sur_small_yummy, syummy[i].x, syummy[i].y, 1);
	}
	for(int i = 245; i < 254; i++)
	{
		draw(sur_small_yummy, syummy[i].x, syummy[i].y, 1);
	}
	for(int i = 254; i < 263; i++)
	{
		draw(sur_small_yummy, syummy[i].x, syummy[i].y, 1);
	}
	
	/* Big yummies */
	byummy[0].x = 40;
	byummy[0].y = 83;
	byummy[0].w = 0;
	byummy[0].h = 1; // 1==displayed, 2==not displayed, 0==run over by ghost, needing fix
	byummy[1].x = 451;
	byummy[1].y = 83;
	byummy[1].w = 0;
	byummy[1].h = 1; // 1==displayed, 2==not displayed, 0==run over by ghost, needing fix
	byummy[2].x = 40;
	byummy[2].y = 460;
	byummy[2].w = 0;
	byummy[2].h = 1; // 1==displayed, 2==not displayed, 0==run over by ghost, needing fix
	byummy[3].x = 451;
	byummy[3].y = 460;
	byummy[3].w = 0;
	byummy[3].h = 1; // 1==displayed, 2==not displayed, 0==run over by ghost, needing fix
	for(int i = 0; i < byum; i++)
	{
		draw(sur_big_yummy, byummy[i].x, byummy[i].y, 1);
	}
	
	draw(NULL, 0, 0, 2);
	
	/* Set initial direction of movement */
	pacmoving = S;
	movewanted = S;
}

/* House-keeping and such */
void shutdown()
{
	SDL_FreeSurface(sur_screen);
	SDL_FreeSurface(sur_bg);
	SDL_FreeSurface(sur_pacman_up);
	SDL_FreeSurface(sur_pacman_down);
	SDL_FreeSurface(sur_pacman_right);
	SDL_FreeSurface(sur_pacman_left);
	SDL_FreeSurface(sur_small_yummy);
	SDL_FreeSurface(sur_big_yummy);
	SDL_FreeSurface(sur_redg);
	SDL_FreeSurface(sur_cyang);
	SDL_FreeSurface(sur_pinkg);
	SDL_FreeSurface(sur_vulng);
	TTF_Quit();
	//Mix_FreeChunk(snd_intro);
}

/* Render text to screen */
void text(char *str, int x, int y, int mode)
{
	/* Draw text to surface */
	//SDL_Color fontcolor = {0xff, 0xff, 0xff, 0};
	//SDL_Surface *txt;
	//txt = TTF_RenderText_Solid(font, str, fontcolor);

	/* Draw surface to screen */
	//draw(txt, x, y, mode);
}

/* Render text to screen */
void text2(char *str, int x, int y, int mode)
{
	/* Draw text to surface */
	//SDL_Color fontcolor = {0xff, 0xff, 0xff, 0};
	//SDL_Surface *txt;
	//txt = TTF_RenderText_Solid(font2, str, fontcolor);

	/* Draw surface to screen */
	//draw(txt, x, y, mode);
}

/* Draw function */
void draw(SDL_Surface *sur, int x, int y, int mode)
{
	SDL_Rect dest;
	dest.x = x;
	dest.y = y;

	if(mode == 1) //just blit
	{
		SDL_BlitSurface(sur, NULL, sur_screen, &dest);
	}
	if(mode == 2) //just flip
	{
		
	}
	if(mode == 3) //blit and flip
	{
		SDL_BlitSurface(sur, NULL, sur_screen, &dest);
		
	}
}

/* Update score on screen */
void updatescore()
{
	clear(sur_screen, 20, 530, 180, 40, 1);
	char buf[100];
	sprintf(buf, "Score: %d", score);
	text(buf, 20, 530, 3);
}

/* Update lives on the screen */
void updatelives()
{
	clear(sur_screen, 200, 530, 100, 40, 1);
	char buf[20];
	sprintf(buf, "Lives: %d", lives);
	text(buf, 200, 530, 3);
}

/* Update level on the screen */
void updatelevel()
{
	clear(sur_screen, 350, 530, 100, 40, 1);
	char buf[20];
	sprintf(buf, "Level: %d", level);
	text(buf, 350, 530, 3);
}

/* Update status on the screen */
void updatestatus(int clr = 0)
{
	clear(sur_screen, 150, 580, 300, 40, 1);
	char buf[50];
	if(!clr)
	if(killed)
	{
		if(lives > 0)
		{
			sprintf(buf, "You died. Press any key to continue.");
		}
		else
		{
			sprintf(buf, "Game over. Press any key to exit.");
		}
	}
	if(levelup)
	{
		sprintf(buf, "Press any key for the next level.");
	}
	text2(buf, 150, 580, 3);
}

/* Clear a portion of the screen */
void clear(SDL_Surface *sur, int x, int y, int w, int h, int flip = 0)
{
	SDL_Rect dest;
	dest.x = x;
	dest.y = y;
	dest.w = w;
	dest.h = h;
	SDL_FillRect(sur, &dest, 0);
	if(flip)
	{
		//SDL_Flip(sur);
	}
}

/* Clear a portion of the screen */
void clear(SDL_Surface *sur, SDL_Rect &dest, int flip = 0)
{
	SDL_FillRect(sur, &dest, 0);
	if(flip)
	{		
		//SDL_Flip(sur);
	}
}

/* Checks if a collision has happened between obj1 and obj2 */
bool cd(SDL_Rect *obj1, SDL_Rect *obj2)
{
	int left1, left2;
	int right1, right2;
	int top1, top2;
	int bottom1, bottom2;

	left1 = obj1->x;
	left2 = obj2->x;
	right1 = obj1->x + obj1->w;
	right2 = obj2->x + obj2->w;
	top1 = obj1->y;
	top2 = obj2->y;
	bottom1 = obj1->y + obj1->h;
	bottom2 = obj2->y + obj2->h;

	if(bottom1 < top2)
	{
		return(false);
	}
	if(top1 > bottom2)
	{
		return(false);
	}

	if(right1 < left2)
	{
		return(false);
	}
	if(left1 > right2)
	{
		return(false);
	}

	return(true);
}

/* Checks if a collision has happened between objects using coordinates */
bool cd(int x1, int y1, int w1, int h1, int x2, int y2, int w2, int h2)
{
	int left1, left2;
	int right1, right2;
	int top1, top2;
	int bottom1, bottom2;

	left1 = x1;
	left2 = x2;
	right1 = x1 + w1;
	right2 = x2 + w2;
	top1 = y1;
	top2 = y2;
	bottom1 = y1 + h1;
	bottom2 = y2 + h2;

	if(bottom1 <= top2)
	{
		return(false);
	}
	if(top1 >= bottom2)
	{
		return(false);
	}

	if(right1 <= left2)
	{
		return(false);
	}
	if(left1 >= right2)
	{
		return(false);
	}

	return(true);
}

/* Check if there was a collision between a ghost and a yummy, if so, mark yummy for redraw */
void cdyummy(int ghostnum)
{
	for(int i = 0; i < syum; i++)
	{
		if(syummy[i].h != 2 && cd(ghosts[ghostnum].x, ghosts[ghostnum].y, 30, 30, syummy[i].x, syummy[i].y, 3, 3))
		{
			syummy[i].h = 0;
		}
	}
	for(int i = 0; i < byum; i++)
	{
		if(byummy[i].h != 2 && cd(ghosts[ghostnum].x, ghosts[ghostnum].y, 30, 30, byummy[i].x, byummy[i].y, 9, 9))
		{
			byummy[i].h = 0;
		}
	}
}

/* Fixes the yummies that were run over by ghosts */
void yummyfix()
{
	for(int i = 0; i < syum; i++)
	{
		if(syummy[i].h == 0)
		{
			draw(sur_small_yummy, syummy[i].x, syummy[i].y, 1);
			syummy[i].h = 1;
		}
	}
	for(int i = 0; i < byum; i++)
	{
		if(byummy[i].h == 0)
		{
			draw(sur_big_yummy, byummy[i].x, byummy[i].y, 1);
			byummy[i].h = 1;
		}
	}
}

/* Play a sound */
int playsound(int effect, int halt)
{
	int channel = -1;
	/*if(halt)
	{
		Mix_HaltChannel(-1);
		return channel;
	}
	switch(effect)
	{
		case 1:
			channel = Mix_PlayChannel(-1, snd_intro, 0);
			break;
		case 2:
			channel = Mix_PlayChannel(-1, snd_chomp, -1);
			break;
		case 3:
			channel = Mix_PlayChannel(-1, snd_pacdied, 0);
			break;
		case 4:
			channel = Mix_PlayChannel(-1, snd_ghostdied, 0);
			break;
	}*/
	return channel;
}

/* Returns a random moveable direction */
int getranddir(int ghostnum, int nodir)
{
	if(ghosts[ghostnum].w == 2)
	{
		return 0;
	}
	
	if(ghosts[ghostnum].h == S)
	{
		return 0;
	}
	
	int goodarr[4];
	goodarr[0]=0;
	goodarr[1]=0;
	goodarr[2]=0;
	goodarr[3]=0;
	
	bool left=true, right=true, up=true, down=true;
	
	srand(time(NULL));
	
	switch(oppdir(ghosts[ghostnum].h))
	{
		case L:
			left = false;
			break;
		case R:
			right = false;
			break;
		case U:
			up = false;
			break;
		case D:
			down = false;
			break;
	}
	
	switch(nodir)
	{
		case L:
			left = false;
			break;
		case R:
			right = false;
			break;
		case U:
			up = false;
			break;
		case D:
			down = false;
			break;
	}
	
	for(int j = 0; j < 41; j++)
	{
		//check left
		if(left)
		if(cd(ghosts[ghostnum].x-1, ghosts[ghostnum].y, 30, 30, barriers[j].x, barriers[j].y, barriers[j].w, barriers[j].h))
		{
			left = false;
		}
		
		//check right
		if(right)
		if(cd(ghosts[ghostnum].x+1, ghosts[ghostnum].y, 30, 30, barriers[j].x, barriers[j].y, barriers[j].w, barriers[j].h))
		{
			right = false;
		}
		
		//check up
		if(up)
		if(cd(ghosts[ghostnum].x, ghosts[ghostnum].y-1, 30, 30, barriers[j].x, barriers[j].y, barriers[j].w, barriers[j].h))
		{
			up = false;
		}
		
		//check down
		if(down)
		if(cd(ghosts[ghostnum].x, ghosts[ghostnum].y+1, 30, 30, barriers[j].x, barriers[j].y, barriers[j].w, barriers[j].h))
		{
			down = false;
		}
	}
	
	int c = 0;
	for(int i = 0; i < 4; i++)
	{
		if(left && i == 0)
		{
			goodarr[c] = L;
			c++;
		}
		if(right && i == 1)
		{
			goodarr[c] = R;
			c++;
		}
		if(up && i == 2)
		{
			goodarr[c] = U;
			c++;
		}
		if(down && i == 3)
		{
			goodarr[c] = D;
			c++;
		}
	}
	
	int r = rand() % c;
	return goodarr[r];
}

/* Returns a random adjacent direction */
int adjdir(int dir)
{
	srand(time(NULL));
	
	if(dir == L)
	{
		int r = rand() % 2;
		if(r) return U;
		else return D;
	}
	if(dir == R)
	{
		int r = rand() % 2;
		if(r) return U;
		else return D;
	}
	if(dir == U)
	{
		int r = rand() % 2;
		if(r) return L;
		else return R;
	}
	if(dir == D)
	{
		int r = rand() % 2;
		if(r) return L;
		else return R;
	}
}

/* Returns opposite direction */
int oppdir(int dir)
{
	if(dir == L)
	{
		return R;
	}
	if(dir == R)
	{
		return L;
	}
	if(dir == U)
	{
		return D;
	}
	if(dir == D)
	{
		return U;
	}
}

/* Move ghosts */
Uint32 moveghosts(Uint32 interval, void *param)
{
	if(killed)
	{
		return 0;
	}
	srand(time(NULL));
	bool good = true;
	for(int i = 0; i < 3; i++)
	{
		switch(adjdir(ghosts[i].h))
		{
			case L:
				good = true;
				for(int j = 0; j < 41; j++)
				{
					if(cd(ghosts[i].x-1, ghosts[i].y, 30, 30, barriers[j].x, barriers[j].y, barriers[j].w, barriers[j].h))
					{
						good = false;
						break;
					}
				}
				if(good)
				{
					ghosts[i].h = L;
				}
				break;
			case R:
				good = true;
				for(int j = 0; j < 41; j++)
				{
					if(cd(ghosts[i].x+1, ghosts[i].y, 30, 30, barriers[j].x, barriers[j].y, barriers[j].w, barriers[j].h))
					{
						good = false;
						break;
					}
				}
				if(good)
				{
					ghosts[i].h = R;
				}
				break;
			case D:
				good = true;
				for(int j = 0; j < 41; j++)
				{
					if(cd(ghosts[i].x, ghosts[i].y+1, 30, 30, barriers[j].x, barriers[j].y, barriers[j].w, barriers[j].h))
					{
						good = false;
						break;
					}
				}
				if(good)
				{
					ghosts[i].h = D;
				}
				break;
			case U:
				good = true;
				for(int j = 0; j < 41; j++)
				{
					if(cd(ghosts[i].x, ghosts[i].y-1, 30, 30, barriers[j].x, barriers[j].y, barriers[j].w, barriers[j].h))
					{
						good = false;
						break;
					}
				}
				if(good)
				{
					ghosts[i].h = U;
				}
				break;
		}
		
		bool good = true;
		switch(ghosts[i].h)
		{
			case L:
				/* Check cd with pacman */
				if(cd(ghosts[i].x, ghosts[i].y, 30, 30, pacman.x, pacman.y, pacman.w, pacman.h))
				{
					if(ghosts[i].w == 3)
					{
						clear(sur_screen, ghosts[i].x, ghosts[i].y, 30, 30, 1);
						playsound(4, 0);
						score+=200;
						modghost(i, 0, 0, 2, S, 2);
					}
					else
					{
						killed = true;
						updatestatus();
						playsound(0, 1);
						playsound(3, 0);
						return 0;
					}
				}
				/* Check teleport point */
				if(cd(ghosts[i].x-1, ghosts[i].y, 30, 30, teleport[0].x, teleport[0].y, teleport[0].w, teleport[0].h))
				{
					clear(sur_screen, ghosts[i].x, ghosts[i].y, 30, 30, 1);
					cdyummy(i);
					yummyfix();
					ghosts[i].x = 440;
					drawg(i);
				}
				for(int j = 0; j < 41; j++)
				{
					if(cd(ghosts[i].x-1, ghosts[i].y, 30, 30, barriers[j].x, barriers[j].y, barriers[j].w, barriers[j].h))
					{
						// ghosts[i].h = rand() % 4 + 1;
						// prev[i] = ghosts[i].h;
						ghosts[i].h = getranddir(i, L);
						good = false;
						break;
					}
				}
				if(!good)
				{
					break;
				}
				clear(sur_screen, ghosts[i].x, ghosts[i].y, 30, 30);
				cdyummy(i);
				yummyfix();
				ghosts[i].x -= 1;
				drawg(i);
				break;
			case R:
				/* Check cd with pacman */
				if(cd(ghosts[i].x, ghosts[i].y, 30, 30, pacman.x, pacman.y, pacman.w, pacman.h))
				{
					if(ghosts[i].w == 3)
					{
						clear(sur_screen, ghosts[i].x, ghosts[i].y, 30, 30, 1);
						playsound(4, 0);
						score+=200;
						modghost(i, 0, 0, 2, S, 2);
					}
					else
					{
						killed = true;
						updatestatus();
						playsound(0, 1);
						playsound(3, 0);
						return 0;
					}
				}
				/* Check teleport point */
				if(cd(ghosts[i].x+1, ghosts[i].y, 30, 30, teleport[1].x, teleport[1].y, teleport[1].w, teleport[1].h))
				{
					clear(sur_screen, ghosts[i].x, ghosts[i].y, 30, 30, 1);
					cdyummy(i);
					yummyfix();
					ghosts[i].x = 30;
					drawg(i);
				}
				for(int j = 0; j < 41; j++)
				{
					if(cd(ghosts[i].x+1, ghosts[i].y, 30, 30, barriers[j].x, barriers[j].y, barriers[j].w, barriers[j].h))
					{
						// ghosts[i].h = rand() % 4 + 1;
						// prev[i] = ghosts[i].h;
						ghosts[i].h = getranddir(i, R);
						good = false;
						break;
					}
				}
				if(!good)
				{
					break;
				}
				clear(sur_screen, ghosts[i].x, ghosts[i].y, 30, 30);
				cdyummy(i);
				yummyfix();
				ghosts[i].x += 1;
				drawg(i);
				break;
			case U:
				/* Check cd with pacman */
				if(cd(ghosts[i].x, ghosts[i].y, 30, 30, pacman.x, pacman.y, pacman.w, pacman.h))
				{
					if(ghosts[i].w == 3)
					{
						clear(sur_screen, ghosts[i].x, ghosts[i].y, 30, 30, 1);
						playsound(4, 0);
						score+=200;
						modghost(i, 0, 0, 2, S, 2);
					}
					else
					{
						killed = true;
						updatestatus();
						playsound(0, 1);
						playsound(3, 0);
						return 0;
					}
				}
				for(int j = 0; j < 41; j++)
				{
					if(cd(ghosts[i].x, ghosts[i].y-1, 30, 30, barriers[j].x, barriers[j].y, barriers[j].w, barriers[j].h))
					{
						// ghosts[i].h = rand() % 4 + 1;
						// prev[i] = ghosts[i].h;
						ghosts[i].h = getranddir(i, U);
						good = false;
						break;
					}
				}
				if(!good)
				{
					break;
				}
				clear(sur_screen, ghosts[i].x, ghosts[i].y, 30, 30);
				cdyummy(i);
				yummyfix();
				ghosts[i].y -= 1;
				drawg(i);
				break;
			case D:
				/* Check cd with pacman */
				if(cd(ghosts[i].x, ghosts[i].y, 30, 30, pacman.x, pacman.y, pacman.w, pacman.h))
				{
					if(ghosts[i].w == 3)
					{
						clear(sur_screen, ghosts[i].x, ghosts[i].y, 30, 30, 1);
						playsound(4, 0);
						score+=200;
						modghost(i, 0, 0, 2, S, 2);
					}
					else
					{
						killed = true;
						updatestatus();
						playsound(0, 1);
						playsound(3, 0);
						return 0;
					}
				}
				for(int j = 0; j < 41; j++)
				{
					if(cd(ghosts[i].x, ghosts[i].y+1, 30, 30, barriers[j].x, barriers[j].y, barriers[j].w, barriers[j].h))
					{
						// ghosts[i].h = rand() % 4 + 1;
						// prev[i] = ghosts[i].h;
						ghosts[i].h = getranddir(i, D);
						good = false;
						break;
					}
				}
				if(!good)
				{
					break;
				}
				clear(sur_screen, ghosts[i].x, ghosts[i].y, 30, 30);
				cdyummy(i);
				yummyfix();
				ghosts[i].y += 1;
				drawg(i);
				break;
		}
	}
	
	/* Allow timer to continue */
	return(interval);
}

/* Mod a ghost to a position */
void modghost(int ghostnum, int x, int y, int mod, int dir, int def = 1)
{
	/* Red == 0 */
	if(ghostnum == 0)
	{
		if(mod == 1 && ghosts[ghostnum].w == 2)
		{
			clear(sur_screen, ghosts[ghostnum].x, ghosts[ghostnum].y, 30, 30, 1);
		}
		if(def)
		{
			ghosts[ghostnum].x = 267;
			ghosts[ghostnum].y = 248;
		}
		else
		{
			ghosts[ghostnum].x = x;
			ghosts[ghostnum].y = y;
		}
		ghosts[ghostnum].w = mod; //1==alive-and-moving, 2==dormant-in-cave, 3==vuln-and-retreating
		ghosts[ghostnum].h = dir;
		drawg(0);
	}
	/* Cyan == 1 */
	if(ghostnum == 1)
	{
		if(mod == 1 && ghosts[ghostnum].w == 2)
		{
			clear(sur_screen, ghosts[ghostnum].x, ghosts[ghostnum].y, 30, 30, 1);
		}
		if(def)
		{
			ghosts[ghostnum].x = 202;
			ghosts[ghostnum].y = 248;
		}
		else
		{
			ghosts[ghostnum].x = x;
			ghosts[ghostnum].y = y;
		}
		ghosts[ghostnum].w = mod; //1==alive-and-moving, 2==dormant-in-cave, 3==vuln-and-retreating
		ghosts[ghostnum].h = dir;
		drawg(1);
	}
	/* Pink == 2 */
	if(ghostnum == 2)
	{
		if(mod == 1 && ghosts[ghostnum].w == 2)
		{
			clear(sur_screen, ghosts[ghostnum].x, ghosts[ghostnum].y, 30, 30, 1);
		}
		if(def)
		{
			ghosts[ghostnum].x = 235;
			ghosts[ghostnum].y = 248;
		}
		else
		{
			ghosts[ghostnum].x = x;
			ghosts[ghostnum].y = y;
		}
		ghosts[ghostnum].w = mod; //1==alive-and-moving, 2==dormant-in-cave, 3==vuln-and-retreating
		ghosts[ghostnum].h = dir;
		drawg(2);
	}
}

/* Draw ghost on screen */
void drawg(int ghostnum)
{
	switch(ghostnum)
	{
		case 0: //Red
			clear(sur_screen, ghosts[0].x, ghosts[0].y, 30, 30);
			if(ghosts[0].w == 3)
			{
				draw(sur_vulng, ghosts[0].x, ghosts[0].y, 3);
			}
			else
			{
				draw(sur_redg, ghosts[0].x, ghosts[0].y, 3);
			}
			break;
		case 1: //Cyan
			clear(sur_screen, ghosts[1].x, ghosts[1].y, 30, 30);
			if(ghosts[1].w == 3)
			{
				draw(sur_vulng, ghosts[1].x, ghosts[1].y, 3);
			}
			else
			{
				draw(sur_cyang, ghosts[1].x, ghosts[1].y, 3);
			}
			break;
		case 2: //Pink
			clear(sur_screen, ghosts[2].x, ghosts[2].y, 30, 30);
			if(ghosts[2].w == 3)
			{
				draw(sur_vulng, ghosts[2].x, ghosts[2].y, 3);
			}
			else
			{
				draw(sur_pinkg, ghosts[2].x, ghosts[2].y, 3);
			}
			break;
	}
}

void RemoveAllTimer()
{
	if (timer_moveghosts)
		SDL_RemoveTimer(timer_moveghosts);
	if (timer_movepacman)
		SDL_RemoveTimer(timer_movepacman);
	if (timer_updategstat)
		SDL_RemoveTimer(timer_updategstat);
	if (timer_vulntimer)
		SDL_RemoveTimer(timer_vulntimer);

	timer_moveghosts = 0;
	timer_movepacman = 0;
	timer_updategstat = 0;
	timer_vulntimer = 0;

}

/* Move Pacman */
Uint32 move(Uint32 interval, void *param)
{
	if(killed)
	{
		return 0;
	}
	bool canmove = true;
	switch(movewanted)
	{
		case L:
			for(int i = 0; i < 41; i++)
			{
				if(cd(pacman.x-pacvelocity, pacman.y, pacman.w, pacman.h, barriers[i].x, barriers[i].y, barriers[i].w, barriers[i].h))
				{
					canmove = false;
					break;
				}
			}
			if(canmove)
			{
				for(int i = 0; i < syum; i++)
				{
					if(syummy[i].h == 1 && cd(pacman.x-pacvelocity, pacman.y, pacman.w, pacman.h, syummy[i].x, syummy[i].y, 3, 3))
					{
						clear(sur_screen, syummy[i].x, syummy[i].y, 3, 3);
						syummy[i].h = 2;
						score = score + 10;
						updatescore();
						yeat++;
						if(yeat == syum)
						{
							levelup = true;
							updatestatus();
							playsound(0, 1);

							RemoveAllTimer();
							
							return(interval);
						}
					}
				}
				for(int i = 0; i < byum; i++)
				{
					if(byummy[i].h == 1 && cd(pacman.x-pacvelocity, pacman.y, pacman.w, pacman.h, byummy[i].x, byummy[i].y, 9, 9))
					{
						byummy[i].h = 2;
						vulnOn();
					}
				}
				clear(sur_screen, pacman.x, pacman.y, pacman.w, pacman.h);
				pacman.x = pacman.x-pacvelocity;
				draw(sur_pacman_left, pacman.x, pacman.y, 3);
				pacmoving = L;
				movewanted = S;
			}
			break;
		case R:
			for(int i = 0; i < 41; i++)
			{
				if(cd(pacman.x+pacvelocity, pacman.y, pacman.w, pacman.h, barriers[i].x, barriers[i].y, barriers[i].w, barriers[i].h))
				{
					canmove = false;
					break;
				}
			}
			if(canmove)
			{
				for(int i = 0; i < syum; i++)
				{
					if(syummy[i].h == 1 && cd(pacman.x+pacvelocity, pacman.y, pacman.w, pacman.h, syummy[i].x, syummy[i].y, 3, 3))
					{
						clear(sur_screen, syummy[i].x, syummy[i].y, 3, 3);
						syummy[i].h = 2;
						score = score + 10;
						updatescore();
						yeat++;
						if(yeat == syum)
						{
							levelup = true;
							updatestatus();
							playsound(0, 1);
							RemoveAllTimer();
							return(interval);
						}
					}
				}
				for(int i = 0; i < byum; i++)
				{
					if(byummy[i].h == 1 && cd(pacman.x+pacvelocity, pacman.y, pacman.w, pacman.h, byummy[i].x, byummy[i].y, 9, 9))
					{
						byummy[i].h = 2;
						vulnOn();
					}
				}
				clear(sur_screen, pacman.x, pacman.y, pacman.w, pacman.h);
				pacman.x = pacman.x+pacvelocity;
				draw(sur_pacman_right, pacman.x, pacman.y, 3);
				pacmoving = R;
				movewanted = S;
			}
			break;
		case U:
			for(int i = 0; i < 41; i++)
			{
				if(cd(pacman.x, pacman.y-pacvelocity, pacman.w, pacman.h, barriers[i].x, barriers[i].y, barriers[i].w, barriers[i].h))
				{
					canmove = false;
					break;
				}
			}
			if(canmove)
			{
				for(int i = 0; i < syum; i++)
				{
					if(syummy[i].h == 1 && cd(pacman.x, pacman.y-pacvelocity, pacman.w, pacman.h, syummy[i].x, syummy[i].y, 3, 3))
					{
						clear(sur_screen, syummy[i].x, syummy[i].y, 3, 3);
						syummy[i].h = 2;
						score = score + 10;
						updatescore();
						yeat++;
						if(yeat == syum)
						{
							levelup = true;
							updatestatus();
							playsound(0, 1);
							RemoveAllTimer();
							return(interval);
						}
					}
				}
				for(int i = 0; i < byum; i++)
				{
					if(byummy[i].h == 1 && cd(pacman.x, pacman.y-pacvelocity, pacman.w, pacman.h, byummy[i].x, byummy[i].y, 9, 9))
					{
						byummy[i].h = 2;
						vulnOn();
					}
				}
				clear(sur_screen, pacman.x, pacman.y, pacman.w, pacman.h);
				pacman.y = pacman.y-pacvelocity;
				draw(sur_pacman_up, pacman.x, pacman.y, 3);
				pacmoving = U;
				movewanted = S;
			}
			break;
		case D:
			for(int i = 0; i < 41; i++)
			{
				if(cd(pacman.x, pacman.y+pacvelocity, pacman.w, pacman.h, barriers[i].x, barriers[i].y, barriers[i].w, barriers[i].h))
				{
					canmove = false;
					break;
				}
			}
			if(canmove)
			{
				for(int i = 0; i < syum; i++)
				{
					if(syummy[i].h == 1 && cd(pacman.x, pacman.y+pacvelocity, pacman.w, pacman.h, syummy[i].x, syummy[i].y, 3, 3))
					{
						clear(sur_screen, syummy[i].x, syummy[i].y, 3, 3);
						syummy[i].h = 2;
						score = score + 10;
						updatescore();
						yeat++;
						if(yeat == syum)
						{
							levelup = true;
							updatestatus();
							playsound(0, 1);
							RemoveAllTimer();
							return(interval);
						}
					}
				}
				for(int i = 0; i < byum; i++)
				{
					if(byummy[i].h == 1 && cd(pacman.x, pacman.y+pacvelocity, pacman.w, pacman.h, byummy[i].x, byummy[i].y, 9, 9))
					{
						byummy[i].h = 2;
						vulnOn();
					}
				}
				clear(sur_screen, pacman.x, pacman.y, pacman.w, pacman.h);
				pacman.y = pacman.y+pacvelocity;
				draw(sur_pacman_down, pacman.x, pacman.y, 3);
				pacmoving = D;
				movewanted = S;
			}
			break;
	}
	
	canmove = true;
	switch(pacmoving)
	{
		case L:
			/* Check teleport point */
			if(cd(pacman.x-pacvelocity, pacman.y, pacman.w, pacman.h, teleport[0].x, teleport[0].y, teleport[0].w, teleport[0].h))
			{
				clear(sur_screen, pacman.x, pacman.y, pacman.w, pacman.h, 1);
				pacman.x = 439;
				draw(sur_pacman_left, pacman.x, pacman.y, 3);
				canmove = true;
			}
			for(int i = 0; i < 41; i++)
			{
				if(cd(pacman.x-pacvelocity, pacman.y, pacman.w, pacman.h, barriers[i].x, barriers[i].y, barriers[i].w, barriers[i].h))
				{
					canmove = false;
					break;
				}
			}
			if(canmove)
			{
				for(int i = 0; i < syum; i++)
				{
					if(syummy[i].h == 1 && cd(pacman.x-pacvelocity, pacman.y, pacman.w, pacman.h, syummy[i].x, syummy[i].y, 3, 3))
					{
						clear(sur_screen, syummy[i].x, syummy[i].y, 3, 3);
						syummy[i].h = 2;
						score = score + 10;
						updatescore();
						yeat++;
						if(yeat == syum)
						{
							levelup = true;
							updatestatus();
							playsound(0, 1);
							RemoveAllTimer();
							return(interval);
						}
					}
				}
				for(int i = 0; i < byum; i++)
				{
					if(byummy[i].h == 1 && cd(pacman.x-pacvelocity, pacman.y, pacman.w, pacman.h, byummy[i].x, byummy[i].y, 9, 9))
					{
						byummy[i].h = 2;
						vulnOn();
					}
				}
				clear(sur_screen, pacman.x, pacman.y, pacman.w, pacman.h);
				pacman.x = pacman.x-pacvelocity;
				draw(sur_pacman_left, pacman.x, pacman.y, 3);
			}
			else
			{
				pacmoving = S;
			}
			break;
		case R:
			/* Check teleport point */
			if(cd(pacman.x+pacvelocity, pacman.y, pacman.w, pacman.h, teleport[1].x, teleport[1].y, teleport[1].w, teleport[1].h))
			{
				clear(sur_screen, pacman.x, pacman.y, pacman.w, pacman.h, 1);
				pacman.x = 29;
				draw(sur_pacman_right, pacman.x, pacman.y, 3);
				canmove = true;
			}
			for(int i = 0; i < 41; i++)
			{
				if(cd(pacman.x+pacvelocity, pacman.y, pacman.w, pacman.h, barriers[i].x, barriers[i].y, barriers[i].w, barriers[i].h))
				{
					canmove = false;
					break;
				}
			}
			if(canmove)
			{
				for(int i = 0; i < syum; i++)
				{
					if(syummy[i].h == 1 && cd(pacman.x+pacvelocity, pacman.y, pacman.w, pacman.h, syummy[i].x, syummy[i].y, 3, 3))
					{
						clear(sur_screen, syummy[i].x, syummy[i].y, 3, 3);
						syummy[i].h = 2;
						score = score + 10;
						updatescore();
						yeat++;
						if(yeat == syum)
						{
							levelup = true;
							updatestatus();
							playsound(0, 1);
							RemoveAllTimer();
							return(interval);
						}
					}
				}
				for(int i = 0; i < byum; i++)
				{
					if(byummy[i].h == 1 && cd(pacman.x+pacvelocity, pacman.y, pacman.w, pacman.h, byummy[i].x, byummy[i].y, 9, 9))
					{
						byummy[i].h = 2;
						vulnOn();
					}
				}
				clear(sur_screen, pacman.x, pacman.y, pacman.w, pacman.h);
				pacman.x = pacman.x+pacvelocity;
				draw(sur_pacman_right, pacman.x, pacman.y, 3);
			}
			else
			{
				pacmoving = S;
			}
			break;
		case U:
			for(int i = 0; i < 41; i++)
			{
				if(cd(pacman.x, pacman.y-pacvelocity, pacman.w, pacman.h, barriers[i].x, barriers[i].y, barriers[i].w, barriers[i].h))
				{
					canmove = false;
					break;
				}
			}
			if(canmove)
			{
				for(int i = 0; i < syum; i++)
				{
					if(syummy[i].h == 1 && cd(pacman.x, pacman.y-pacvelocity, pacman.w, pacman.h, syummy[i].x, syummy[i].y, 3, 3))
					{
						clear(sur_screen, syummy[i].x, syummy[i].y, 3, 3);
						syummy[i].h = 2;
						score = score + 10;
						updatescore();
						yeat++;
						if(yeat == syum)
						{
							levelup = true;
							updatestatus();
							playsound(0, 1);
							RemoveAllTimer();
							return(interval);
						}
					}
				}
				for(int i = 0; i < byum; i++)
				{
					if(byummy[i].h == 1 && cd(pacman.x, pacman.y-pacvelocity, pacman.w, pacman.h, byummy[i].x, byummy[i].y, 9, 9))
					{
						byummy[i].h = 2;
						vulnOn();
					}
				}
				clear(sur_screen, pacman.x, pacman.y, pacman.w, pacman.h);
				pacman.y = pacman.y-pacvelocity;
				draw(sur_pacman_up, pacman.x, pacman.y, 3);
			}
			else
			{
				pacmoving = S;
			}
			break;
		case D:
			for(int i = 0; i < 41; i++)
			{
				if(cd(pacman.x, pacman.y+pacvelocity, pacman.w, pacman.h, barriers[i].x, barriers[i].y, barriers[i].w, barriers[i].h))
				{
					canmove = false;
					break;
				}
			}
			if(canmove)
			{
				for(int i = 0; i < syum; i++)
				{
					if(syummy[i].h == 1 && cd(pacman.x, pacman.y+pacvelocity, pacman.w, pacman.h, syummy[i].x, syummy[i].y, 3, 3))
					{
						clear(sur_screen, syummy[i].x, syummy[i].y, 3, 3);
						syummy[i].h = 2;
						score = score + 10;
						updatescore();
						yeat++;
						if(yeat == syum)
						{
							levelup = true;
							updatestatus();
							playsound(0, 1);
							RemoveAllTimer();
							return(interval);
						}
					}
				}
				for(int i = 0; i < byum; i++)
				{
					if(byummy[i].h == 1 && cd(pacman.x, pacman.y+pacvelocity, pacman.w, pacman.h, byummy[i].x, byummy[i].y, 9, 9))
					{
						byummy[i].h = 2;
						vulnOn();
					}
				}
				clear(sur_screen, pacman.x, pacman.y, pacman.w, pacman.h);
				pacman.y = pacman.y+pacvelocity;
				draw(sur_pacman_down, pacman.x, pacman.y, 3);
			}
			else
			{
				pacmoving = S;
			}
			break;
	}
	
	/* Allow timer to continue */
	return(interval);
}

/* Updates the ghost status */
Uint32 updategstat(Uint32 interval, void *param)
{
	if(killed)
	{
		return 0;
	}
	if(who == 0) //Red
	{
		who = 1;
		if(ghosts[0].w == 2)
		{
			modghost(0, 234, 190, 1, L, 0);
			return(interval);
		}
	}
	else if(who == 1) //Cyan
	{
		who = 2;
		if(ghosts[1].w == 2)
		{
			modghost(1, 234, 190, 1, R, 0);
			return(interval);
		}
	}
	else if(who == 2) //Pink
	{
		who = 0;
		if(ghosts[2].w == 2)
		{
			modghost(2, 234, 190, 1, L, 0);
			return(interval);
		}
	}
	
	/* Allow timer to continue */
	return(interval);
}

/* Turn on vulnerable ghosts */
void vulnOn()
{
	if(!timer_vulntimer)
		timer_vulntimer = SDL_AddTimer(10000, vulntimer, NULL);
	for(int i = 0; i < 3; i++)
	{
		if(ghosts[i].w == 1 || ghosts[i].w == 3)
		{
			modghost(i, ghosts[i].x, ghosts[i].y, 3, ghosts[i].h, 0);
		}
	}
}

/* Turn off vulnerable ghosts */
void vulnOff()
{
	for(int i = 0; i < 3; i++)
	{
		if(ghosts[i].w == 3)
		{
			modghost(i, ghosts[i].x, ghosts[i].y, 1, ghosts[i].h, 0);
		}
	}
}

/* Vulnerability timer */
Uint32 vulntimer(Uint32 interval, void *param)
{
	vulnOff();
	SDL_RemoveTimer(timer_vulntimer);
	timer_vulntimer = 0;
	return(interval);
}

/* The main game loop */
void game()
{
	/* Show status */
	updatestatus(0);
	
	/* Show score */
	updatescore();
	
	/* Show lives */
	updatelives();
	
	/* Show level */
	updatelevel();
	
	/* Set speed */
	int speed = 5;
	
	/* Start ghost timer */
	if (!timer_moveghosts)
		timer_moveghosts = SDL_AddTimer(speed, moveghosts, NULL);
	
	if (!timer_movepacman)
	/* Start pacman timer */
		timer_movepacman = SDL_AddTimer(speed, move, NULL);
	if (!timer_updategstat)
	/* Start ghost status update timer */
		timer_updategstat = SDL_AddTimer(5000, updategstat, NULL);
	
	/* Start chomping sound */
	playsound(2, 0);
	
	bool done = false;
	SDL_Event event;
	while(!done)
	{
		if(killed)
		{
			RemoveAllTimer();			
			lives--;
			done = true;
			break;
		}
		if(levelup)
		{
			done = true;
			break;
		}
		while(SDL_PollEvent(&event)) //quit being a fucking CPU hog
		{
			switch(event.type)
			{
			case SDL_FINGERUP:
			{
				float fingerX = event.tfinger.x;
				float fingerY = event.tfinger.y;

				fingerX *= SDLSingleton::GetInstance()->m_winWidth;
				fingerY *= SDLSingleton::GetInstance()->m_winHeight;

				if (fingerX < (SDLSingleton::GetInstance()->m_winWidth / 4))
					movewanted = L;
				if (fingerX > (SDLSingleton::GetInstance()->m_winWidth - (SDLSingleton::GetInstance()->m_winWidth /4)))
					movewanted = R;
				if (fingerY < (SDLSingleton::GetInstance()->m_winHeight / 4))
					movewanted = U;
				if (fingerY >(SDLSingleton::GetInstance()->m_winHeight - (SDLSingleton::GetInstance()->m_winHeight / 4)))
					movewanted = D;
				}
				case SDL_KEYDOWN:
					switch(event.key.keysym.sym)
					{
						case SDLK_ESCAPE:
							done = true;
							shutdown();
							exit(1);
							break;
						case SDLK_UP:
							movewanted = U;
							break;
						case SDLK_DOWN:
							movewanted = D;
							break;
						case SDLK_LEFT:
							movewanted = L;
							break;
						case SDLK_RIGHT:
							movewanted = R;
							break;
						default:
							break;
					}
					break;
				case SDL_QUIT:
					done = true;
					shutdown();
					exit(1);
					break;
				default:
					break;
			}

			
		}

		SDL_Delay(16);

		SDLSingleton::GetInstance()->DoRender();

	}
}


int main(int argc, char** argv)
{
	/* Init score */
	score = 0;
	init_clerical();
	init_game();
	SDLSingleton::GetInstance()->DoRender();
	int chan = playsound(1, 0);
	//while(Mix_Playing(chan) != 0);
	while(true)
	{
		game();
		if(killed)
		{
			killed = false;
			if(lives < 0)
			{
				break;
			}
			clear(sur_screen, pacman.x, pacman.y, pacman.w, pacman.h);
			for(int i = 0; i < 3; i++)
			{
				clear(sur_screen, ghosts[i].x, ghosts[i].y, 30, 30);
			}
			modghost(0, 0, 0, 2, S, 2);
			modghost(1, 0, 0, 2, S, 2);
			modghost(2, 0, 0, 2, S, 2);
			/* Set pacman's position */
			pacman.x = 234;
			pacman.y = 388;
			pacman.w = 30;
			pacman.h = 30;
			/* Draw pacman */
			draw(sur_pacman_left, pacman.x, pacman.y, 3);
		}
		if(levelup)
		{
			clear(sur_screen, pacman.x, pacman.y, pacman.w, pacman.h);
			for(int i = 0; i < 3; i++)
			{
				clear(sur_screen, ghosts[i].x, ghosts[i].y, 30, 30);
			}
			levelup = false;
			yeat = 0;
			lives++;
			level++;
			modghost(0, 0, 0, 2, S, 2);
			modghost(1, 0, 0, 2, S, 2);
			modghost(2, 0, 0, 2, S, 2);
			/* Set pacman's position */
			pacman.x = 234;
			pacman.y = 388;
			pacman.w = 30;
			pacman.h = 30;
			/* Draw pacman */
			draw(sur_pacman_left, pacman.x, pacman.y, 3);
			init_game();
		}
	}
	shutdown();
	
	return 0;
}
