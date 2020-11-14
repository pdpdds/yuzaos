#pragma once

#include <SDL_mixer.h>
#include <sstream>
#include "Texture.h"
#include "Constances.h"
#include "Button.h"

//The window we'll be rendering to
extern SDL_Window* gWindow;

//The window renderer
extern SDL_Renderer* gRenderer;

//The sound effects that will be used
extern Mix_Music *winner;
extern Mix_Music *loser;
extern Mix_Chunk *click;

//Globally used font
extern TTF_Font *gGameOver;
extern TTF_Font *gPlayAgainWin;
extern TTF_Font *gPlayAgainLose;

//Mouse button texture
extern SDL_Rect gSpriteClips[ BUTTON_SPRITE_TOTAL ];
extern LTexture gButtonSpriteSheetTexture;

//Screen texture
extern LTexture gWinningTexture;
extern LTexture gBackgroundTexture;

//Rendered texture
extern LTexture gTextTexture;
extern LTexture gMineLeftTexture;
extern LTexture gPlayAgainWinTexture;
extern LTexture gPlayAgainLoseTexture;

//Gameplay variables
extern int countMineLeft;
extern int countTileLeft;
extern bool gameOver;
extern bool isWinning;

//In memory text stream
extern stringstream mineLeft;

//Board with mine
extern int board[ROW_SIZE][COLUMN_SIZE];

//Board for showing
extern int sBoard[ROW_SIZE][COLUMN_SIZE];




