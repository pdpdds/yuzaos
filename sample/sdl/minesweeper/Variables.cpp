#include "Variables.h"

//The window we'll be rendering to
SDL_Window* gWindow = NULL;

//The window renderer
SDL_Renderer* gRenderer = NULL;

//The sound effects that will be used
Mix_Music *winner = NULL;
Mix_Music *loser = NULL;
Mix_Chunk *click = NULL;

//Globally used font
TTF_Font *gGameOver = NULL;
TTF_Font *gPlayAgainWin = NULL;
TTF_Font *gPlayAgainLose = NULL;

//Mouse button texture
SDL_Rect gSpriteClips[ BUTTON_SPRITE_TOTAL ];
LTexture gButtonSpriteSheetTexture;

//Screen texture
LTexture gWinningTexture;
LTexture gBackgroundTexture;

//Rendered texture
LTexture gTextTexture;
LTexture gMineLeftTexture;
LTexture gPlayAgainWinTexture;
LTexture gPlayAgainLoseTexture;

//Gameplay variables
int countMineLeft = MINE_COUNT;
int countTileLeft = ROW_SIZE * COLUMN_SIZE;
bool gameOver = false;
bool isWinning = false;

//In memory text stream
stringstream mineLeft;

//Board with mine
int board[ROW_SIZE][COLUMN_SIZE];

//Board for showing
int sBoard[ROW_SIZE][COLUMN_SIZE];
