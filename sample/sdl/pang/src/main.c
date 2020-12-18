#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "structures.h"

#include "PCGfxEngine.h"

#ifdef PC_VERSION
#include "PCSoundEngine.h"
#include <math.h>
#endif


#ifdef GP2X_VERSION
#include <unistd.h>
#endif

#include "plateforme.h"
#include "ball.h"
#include "player.h"
#include "ladder.h"
#include "shoot.h"
#include "levels.h"
#include "bonus.h"
#include "objets.h"

#include "main.h"


#ifdef DREAMCAST_VERSION

extern uint8 romdisk[];

/* Initialisation de KOS et du Romdisk */

KOS_INIT_FLAGS(INIT_DEFAULT | INIT_MALLOCSTATS);

KOS_INIT_ROMDISK(romdisk);
#endif

int fond_loaded = 0;


void showScore(char* string, unsigned int x, unsigned int y)

{

	unsigned char i, temp;
	int positionxduzero = 148 * scale;
	int positionyduzero = 226 * scale;

	temp = strlen(string);
	for (i = 0; i < temp; i++)
	{
		if (((*string - 48) * 13) == 0)
		{
			blitImageToScreen(1, ((*string - 48) * 14 * scale) + positionxduzero, positionyduzero, 13 * scale, 13 * scale, x + (i * 6 * scale), y, 13 * scale, 13 * scale, 320 * scale, 240 * scale);
		}
		else
		{
			blitImageToScreen(1, ((*string - 48) * 14 * scale) + positionxduzero + 1, positionyduzero, 13 * scale, 13 * scale, x + (i * 6 * scale), y, 13 * scale, 13 * scale, 320 * scale, 240 * scale);
		}

		string++;
	}
}

void showFond(int level)
{
	if ((level == 1) && (fond_loaded != 1)) { loadBmp("", "", "romdisk/fond1.png", "/rd/fond1.png", 0); fond_loaded = 1; }
	else if ((level == 4) && (fond_loaded != 4)) { loadBmp("", "", "romdisk/fond2.png", "/rd/fond2.png", 0); fond_loaded = 4; }
	else if ((level == 7) && (fond_loaded != 7)) { loadBmp("", "", "romdisk/fond3.png", "/rd/fond3.png", 0); fond_loaded = 7; }
	else if ((level == 10) && (fond_loaded != 10)) { loadBmp("", "", "romdisk/fond4.png", "/rd/fond4.png", 0); fond_loaded = 10; }
	else if ((level == 13) && (fond_loaded != 13)) { loadBmp("", "", "romdisk/fond5.png", "/rd/fond5.png", 0); fond_loaded = 13; }
	else if ((level == 16) && (fond_loaded != 16)) { loadBmp("", "", "romdisk/fond6.png", "/rd/fond6.png", 0); fond_loaded = 16; }
	else if ((level == 19) && (fond_loaded != 19)) { loadBmp("", "", "romdisk/fond7.png", "/rd/fond7.png", 0); fond_loaded = 19; }
	else if ((level == 22) && (fond_loaded != 22)) { loadBmp("", "", "romdisk/fond8.png", "/rd/fond8.png", 0); fond_loaded = 22; }
	else if ((level == 25) && (fond_loaded != 25)) { loadBmp("", "", "romdisk/fond9.png", "/rd/fond9.png", 0); fond_loaded = 25; }
	else if ((level == 28) && (fond_loaded != 28)) { loadBmp("", "", "romdisk/fond10.png", "/rd/fond10.png", 0); fond_loaded = 28; }
	else if ((level == 31) && (fond_loaded != 31)) { loadBmp("", "", "romdisk/fond11.png", "/rd/fond11.png", 0); fond_loaded = 31; }
	else if ((level == 34) && (fond_loaded != 34)) { loadBmp("", "", "romdisk/fond12.png", "/rd/fond12.png", 0); fond_loaded = 34; }
	else if ((level == 37) && (fond_loaded != 37)) { loadBmp("", "", "romdisk/fond13.png", "/rd/fond13.png", 0); fond_loaded = 37; }

	blitImageToScreen(0, 0, 0, 320 * scale, 240 * scale, 0, 0, 320 * scale, 240 * scale, 320 * scale, 240 * scale);

}

int common()
{

	int myBall;
	int tmp;

	int i;
	int x;
	int y = 200;
	int nbBall = 0;
	int pause = 0;

	gbl_evt = EVT_TITLE;

	srand(36547);                /* seed random numbers */

	initGfxEngine();
	initSpriteEngine();
	initSoundEngine();


	loadSound("romdisk/ingame.wav", "/rd/ingame.wav", 0);
	loadSound("romdisk/nextlevel.wav", "/rd/nextlevel.wav", 1);
	loadSound("romdisk/gameover.wav", "/rd/gameover.wav", 2);
	loadSound("romdisk/shoot_sound.wav", "/rd/shoot_sound.wav", 3);
	loadSound("romdisk/ball_explode.wav", "/rd/ball_explode.wav", 4);


	loadBmp("", "", "romdisk/sprite1.png", "/rd/sprite1.png", 1);


	y = y * scale;

	getImage(ANIM_RIGHT1, 70 * scale, 117 * scale, 31 * scale, 32 * scale, 1, 320 * scale, 240 * scale);
	getImage(ANIM_RIGHT2, 118 * scale, 117 * scale, 31 * scale, 32 * scale, 1, 320 * scale, 240 * scale);
	getImage(ANIM_RIGHT3, 164 * scale, 117 * scale, 31 * scale, 32 * scale, 1, 320 * scale, 240 * scale);
	getImage(ANIM_LEFT1, 70 * scale, 153 * scale, 31 * scale, 32 * scale, 1, 320 * scale, 240 * scale);
	getImage(ANIM_LEFT2, 118 * scale, 153 * scale, 31 * scale, 32 * scale, 1, 320 * scale, 240 * scale);
	getImage(ANIM_LEFT3, 164 * scale, 153 * scale, 31 * scale, 32 * scale, 1, 320 * scale, 240 * scale);
	getImage(ANIM_SHOOT, 67 * scale, 70 * scale, 31 * scale, 32 * scale, 1, 320 * scale, 240 * scale);
	getImage(ANIM_STOP, 67 * scale, 70 * scale, 31 * scale, 32 * scale, 1, 320 * scale, 240 * scale);
	getImage(ANIM_DEAD, 168 * scale, 54 * scale, 48 * scale, 40 * scale, 1, 320 * scale, 240 * scale);
	getImage(BIG, 10 * scale, 7 * scale, 48 * scale, 40 * scale, 1, 320 * scale, 240 * scale);
	getImage(NORMAL, 70 * scale, 6 * scale, 32 * scale, 26 * scale, 1, 320 * scale, 240 * scale);
	getImage(SMALL, 109 * scale, 20 * scale, 16 * scale, 14 * scale, 1, 320 * scale, 240 * scale);
	getImage(MICRO, 131 * scale, 28 * scale, 8 * scale, 7 * scale, 1, 320 * scale, 240 * scale);
	getImage(PF_CASSABLE, 20 * scale, 78 * scale, 32 * scale, 8 * scale, 1, 320 * scale, 240 * scale);
	getImage(PF_INCASSABLE, 20 * scale, 88 * scale, 32 * scale, 8 * scale, 1, 320 * scale, 240 * scale);
	getImage(PF_CASSABLEV, 11 * scale, 102 * scale, 8 * scale, 32 * scale, 1, 320 * scale, 240 * scale);
	getImage(PF_INCASSABLEV, 21 * scale, 102 * scale, 8 * scale, 32 * scale, 1, 320 * scale, 240 * scale);
	getImage(PF_MOYEN_INCASSABLE, 38 * scale, 102 * scale, 16 * scale, 8 * scale, 1, 320 * scale, 240 * scale);
	getImage(PF_MOYEN_CASSABLE, 38 * scale, 112 * scale, 16 * scale, 8 * scale, 1, 320 * scale, 240 * scale);
	getImage(PF_MICRO_INCASSABLE, 45 * scale, 122 * scale, 8 * scale, 8 * scale, 1, 320 * scale, 240 * scale);
	getImage(PF_MICRO_CASSABLE, 45 * scale, 132 * scale, 8 * scale, 8 * scale, 1, 320 * scale, 240 * scale);
	getImage(ANIM_LADDER1, 200 * scale, 191 * scale, 26 * scale, 32 * scale, 1, 320 * scale, 240 * scale);
	getImage(ANIM_LADDER2, 230 * scale, 191 * scale, 26 * scale, 32 * scale, 1, 320 * scale, 240 * scale);


	initBalls();
	initPlateformes();
	initPlayer();
	initLadders();
	initShoot();


	float now = SDL_GetTicks();
	while (1)
	{


		if (gbl_evt == EVT_TITLE)
		{
			int i;
			releaseAllSprite();


#ifdef GP2X_VERSION
			loadBmp("", "", "romdisk/title-gp2x.png", "/rd/title-gp2x.png", 2);
#else
			loadBmp("", "", "romdisk/title.png", "/rd/title.png", 2);
#endif

			blitImageToScreen(2, 0 * scale, 0 * scale, 320 * scale, 240 * scale, 0 * scale, 0 * scale, 320 * scale, 240 * scale, 320 * scale, 240 * scale);

			flipScreen();
			//	    		waitInMs(100);
			while (keyAction1 == 0 && keyQuit == 0) 
			{ 
				Sleep(1);
				checkController(); i = rand(); 
			}
			while (keyAction1 == 1 && keyQuit == 0) 
			{ 
				checkController(); i = rand(); 
			}


			gbl_evt = EVT_NULL;
			//			printf("init player \n");
			initPlayer();
			//			printf("init level \n");
			initLevel(currentLevel);
		}
		else if (gbl_evt == EVT_NULL) {
			synchroStart();

			while (keyAction4 == 1)
			{
				pause = 1;
				checkController();
			}

			while (pause == 1)
			{
				checkController();
				if (keyAction4 == 1)
				{
					while (keyAction4 == 1)
					{
						checkController();
					}
					pause = 0;

				}
			}


			playSound(0, -1);

			int old_score = player.score;
			showFond(currentLevel);
			if (player.bonus_protection == 1)
			{
				if (player.bonus_protection_timer == -1)
				{
					blitImageToScreen(1, 231 * scale, 123 * scale, 43 * scale, 43 * scale, player.posx - (5 * scale), player.posy - (5 * scale), 43 * scale, 43 * scale, 320 * scale, 240 * scale);
				}
				else {
					if (gbl_timer % 2 == 0) {
						blitImageToScreen(1, 231 * scale, 123 * scale, 43 * scale, 43 * scale, player.posx - (5 * scale), player.posy - (5 * scale), 43 * scale, 43 * scale, 320 * scale, 240 * scale);
					}
				}
			}

			for (i = 0; i < MAX_PLATEFORME; i++)
			{
				showPlateforme(i);
			}
			for (i = 0; i < MAX_ECHELLE; i++) showLadder(i);
			updatePlayer();
			for (i = 0; i < MAX_SHOOT; i++)
			{
				updateShoot(i);
				showShoot(i);
			}
			nbBall = 0;
			for (i = 0; i < MAX_BALL; i++)
			{
				nbBall += ball[i].utilise;

				if ((onetwo == 0) || (onetwo == 2) || (onetwo == 4))
					updateBalls(i);

				if ((player.bonus_freeze > 0) && (player.bonus_freeze < 81))
				{
					if (gbl_timer % 2 == 0) showBall(i);
				}
				else
				{
					showBall(i);
				}
			}
			for (i = 0; i < MAX_BONUS; i++)
			{
				updateBonus(i);
				showBonus(i);
			}
			if (nbBall == 0)
			{
				gbl_evt = EVT_NEXT_LEVEL;
			}
			showPlayer();
			for (i = 0; i < MAX_OBJETS; i++)
			{
				showObjet(i);
			}
			for (i = 0; i < player.nblive; i++)
			{
				if (i < 3)
				{
					//blitImageToScreen(1,358,382,36,36,(10+i*20)*scale,430,36,36,320*scale,240*scale);
					blitImageToScreen(1, 179 * scale, 191 * scale, 18 * scale, 18 * scale, (10 + i * 20) * scale, 214 * scale, 18 * scale, 18 * scale, 320 * scale, 240 * scale);
				}
			}
			char chaine[10];
			sprintf(chaine, "%d", player.score);
			//        		blitImageToScreen(1,484,130,76,22,(110+20)*scale,420,76,22,320*scale,240*scale); // Affiche SCORE:
			blitImageToScreen(1, 242 * scale, 65 * scale, 38 * scale, 11 * scale, (110 + 20) * scale, 210 * scale, 38 * scale, 11 * scale, 320 * scale, 240 * scale); // Affiche SCORE:
			showScore(chaine, 170 * scale, 209 * scale);
			sprintf(chaine, "%d", currentLevel);
			//        		blitImageToScreen(1,198,88,72,28,260,448,72,28,320*scale,240*scale); // Affiche LEVEL:
			blitImageToScreen(1, 99 * scale, 44 * scale, 36 * scale, 14 * scale, 130 * scale, 224 * scale, 36 * scale, 14 * scale, 320 * scale, 240 * scale); // Affiche LEVEL:
			showScore(chaine, 170 * scale, 224 * scale);
			sprintf(chaine, "%d", chrono);

			sprintf(debug, "%s", "");
			if ((old_score < 600) && (player.score >= 600)) { createObjet(OBJ_1UP, player.posx, player.posy, 0); player.nblive++; }
			if ((old_score < 1500) && (player.score >= 1500)) { createObjet(OBJ_1UP, player.posx, player.posy, 0); player.nblive++; }
			if ((old_score < 5000) && (player.score >= 5000)) { createObjet(OBJ_1UP, player.posx, player.posy, 0); player.nblive++; }
			if (gbl_timer == 1) chrono--;
			if (chrono < 0) chrono = 0;


		}
		else if (gbl_evt == EVT_LOOSE_LIFE)
		{
			/* Animation de perte de vie ! */
			int cpt = 0;
			while (cpt != 70)
			{
				showFond(currentLevel);
				for (i = 0; i < MAX_ECHELLE; i++) showLadder(i);
				for (i = 0; i < MAX_BALL; i++)
				{
					showBall(i);
				}
				for (i = 0; i < MAX_PLATEFORME; i++)
				{
					showPlateforme(i);
				}

				showPlayer();
				if (cpt > 20) player.anim_courante = ANIM_DEAD;
				if ((cpt > 20) && (cpt < 40)) player.posy--;
				if (cpt > 40) { player.posy++; player.posx++; }
				cpt++;
				flipScreen();

			}
			forceClearSound(0);
			releaseAllSprite();
			/* R?nitialiser d'abord le joueur avant les niveaux pour histoire de sprite ! */
			reInitPlayer();
			initLevel(currentLevel);
			gbl_evt = EVT_NULL;
			if (player.nblive <= 0) gbl_evt = EVT_GAME_OVER;
		}
		else if (gbl_evt == EVT_NEXT_LEVEL) {
			forceClearSound(0);
			releaseAllSprite();
			playSound(1, 0);
			if ((currentLevel % 2) == 0)
			{
				loadBmp("", "", "romdisk/nextlevel1.png", "/rd/nextlevel1.png", 2);
				blitImageToScreen(2, 0 * scale, 0 * scale, 320 * scale, 240 * scale, 0 * scale, 0 * scale, 320 * scale, 240 * scale, 320 * scale, 240 * scale);
			}
			else {
				loadBmp("", "", "romdisk/nextlevel2.png", "/rd/nextlevel2.png", 2);
				blitImageToScreen(2, 0 * scale, 0 * scale, 320 * scale, 240 * scale, 0 * scale, 0 * scale, 320 * scale, 240 * scale, 320 * scale, 240 * scale);
			}
			char chaine[10];
			sprintf(chaine, "%d", player.score);
			blitImageToScreen(1, 242 * scale, 65 * scale, 38 * scale, 11 * scale, 110 * scale, 217 * scale, 38 * scale, 11 * scale, 320 * scale, 240 * scale); // Affiche SCORE:
			showScore(chaine, 183 * scale, 217 * scale);

			flipScreen();
			waitInMs();

			checkController();
			while (keyAction1 == 0) { checkController(); i = rand(); }
			while (keyAction1 == 1) { checkController(); i = rand(); }

			currentLevel++;
			if (currentLevel > MAX_LEVEL) currentLevel = 1;
			reInitPlayer();
			initLevel(currentLevel);
			gbl_evt = EVT_NULL;
			forceClearSound(1);
		}
		else if (gbl_evt == EVT_GAME_OVER) {
			forceClearSound(0);
			playSound(2, 0);
			releaseAllSprite();
			/* Animation de perte de gameOver ! */
			int cpt = 0;
			while (cpt < 230)
			{
				/* reaffiche le fond */
				showFond(currentLevel);
				/* anime le game over */
				blitImageToScreen(1, 8 * scale, 218 * scale, 130 * scale, 15 * scale, (320 - cpt) * scale, 100 * scale, 130 * scale, 15 * scale, 320 * scale, 240 * scale);
				cpt += 3;
				char chaine[10];
				sprintf(chaine, "%d", player.score);
				blitImageToScreen(1, 242 * scale, 65 * scale, 38 * scale, 11 * scale, (110 + 20) * scale, 210 * scale, 38 * scale, 11 * scale, 320 * scale, 240 * scale); // Affiche SCORE:
				showScore(chaine, (80 + 70 + 20) * scale, 209 * scale);
				sprintf(chaine, "%d", currentLevel);
				blitImageToScreen(1, 99 * scale, 44 * scale, 36 * scale, 14 * scale, (110 + 20) * scale, 224 * scale, 36 * scale, 14 * scale, 320 * scale, 240 * scale); // Affiche LEVEL:
				showScore(chaine, (80 + 70 + 20) * scale, 224 * scale);

				flipScreen();
			}
			while (keyAction1 == 0) { checkController(); i = rand(); }
			while (keyAction1 == 1) { checkController(); i = rand(); }
			forceClearSound(2);
			initPlayer();
			initLevel(1);
			gbl_evt = EVT_TITLE;
			currentLevel = 1;
		}
		if (fpsshow == 1) showfps();
		flipScreen();
		clearSound(3);
		clearSound(4);
		onetwo++;
		if (onetwo > 5) onetwo = 0;
		gbl_timer++;
		if (gbl_timer == 51) gbl_timer = 1;

		checkController();
		if (keyQuit == 1) return 0;
		synchroEnd(20);
	}
}

int main(int argc, char* argv[])
{
	common();

	quitSoundEngine();

#ifdef GP2X_VERSION
	chdir("/usr/gp2x");
	execl("/usr/gp2x/gp2xmenu", "/usr/gp2x/gp2xmenu", NULL);
#endif


	return 0;
}


void showfps()
{
	static float lasttime = 0;
	float currenttime = 0;
	static int framespersecond;
	static char strframespersecond[10];

	currenttime = SDL_GetTicks() * 0.001;
	framespersecond++;

	if (currenttime - lasttime > 1.0) {
		lasttime = currenttime;
		sprintf(strframespersecond, "FPS %d", framespersecond);
		framespersecond = 0;
	}
	showScore(strframespersecond, 10, 10);
}
