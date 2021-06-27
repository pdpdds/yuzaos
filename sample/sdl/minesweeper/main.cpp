#include <iostream>
#include <time.h>
#include <stdlib.h>
#include <sstream>
#include <string>
#include "Texture.h"
#include "Constances.h"
#include "Variables.h"

using namespace std;

LButton gButtons[ROW_SIZE][COLUMN_SIZE];

//Starts up SDL and creates window
bool init();

//Loads media
bool loadMedia();

//Frees media and shuts down SDL
void close();

//Initializes playground
void createTableWithMine ();

//Check win flag
bool checkWinning ();

//Render number of flag/mine left
void mineManager();

//Perform win/lose flag
void flagManager();

//Perform play again flag
void playAgainManager(bool &quitGame);

int main( int argc, char* args[] )
{
	//Start up SDL and create window
	if( !init() )
	{
		cout << "Failed to initialize!\n";
	}
	else
	{
		//Load media
		if( !loadMedia() )
		{
			cout << "Failed to load media!\n";
		}
		else
		{
			//Main loop flag
			bool quit = false;

			//Event handler
			SDL_Event e;

			//While application is running
			while( !quit )
			{
			    createTableWithMine();

			    //While game is not over yet
			    while ( !gameOver && !quit && !isWinning)
                {
					Sleep(1);
                    //Handle events on queue
                    while( SDL_PollEvent( &e ) != 0 )
                    {
                        //User requests quit
                        if( e.type == SDL_QUIT || e.key.keysym.sym == SDLK_ESCAPE)
                        {
                            quit = true;
                        }

                        //Handle button events
                        for (int i = 0; i < ROW_SIZE; i++)
                        {
                            for (int j = 0; j < COLUMN_SIZE; j++)
                            {
                                gButtons[i][j].handleEvent( &e );
                            }
                        }
                        isWinning = checkWinning();
                    }

                    //Clear screen
                    SDL_SetRenderDrawColor( gRenderer, 0xFF, 0xFF, 0xFF, 0xFF );
                    SDL_RenderClear( gRenderer );

                    //Render background
                    gBackgroundTexture.render(0, 0);

                    //Render buttons
                    for( int i = 0; i < ROW_SIZE; i++ )
                    {
                        for ( int j = 0; j < COLUMN_SIZE; j++ )
                        {
                            gButtons[i][j].render(i, j);
                        }
                    }
                    //Render mine/flag left
                    mineManager();

                    //Perform win/lose flag
                    flagManager();

                    //Update screen
                    SDL_RenderPresent( gRenderer );
                }
                //Check play again flag
                playAgainManager( quit );
			}
		}
	}

	//Free resources and close SDL
	close();

	return 0;
}

bool init()
{
	//Initialization flag
	bool success = true;

	//Initialize SDL
	if( SDL_Init( SDL_INIT_VIDEO | SDL_INIT_AUDIO ) < 0 )
	{
	    cout << "SDL could not initialize! SDL Error: " << SDL_GetError() << endl;
		success = false;
	}
	else
	{
		//Set texture filtering to linear
		if( !SDL_SetHint( SDL_HINT_RENDER_SCALE_QUALITY, "1" ) )
		{

			cout << "Warning: Linear texture filtering not enabled!";
		}

		//Create window
		gWindow = SDL_CreateWindow( "Minesweeper", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN );
		if( gWindow == NULL )
		{
			cout << "Window could not be created! SDL Error: " << SDL_GetError() << endl;
			success = false;
		}
		else
		{
			//Create vsynced renderer for window
			//gRenderer = SDL_CreateRenderer( gWindow, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC );
			gRenderer = SDL_CreateRenderer(gWindow, -1, SDL_RENDERER_SOFTWARE);
			if( gRenderer == NULL )
			{
			    cout << "Renderer could not be created! SDL Error: " << SDL_GetError() << endl;
				success = false;
			}
			else
			{
				//Initialize PNG loading
				int imgFlags = IMG_INIT_PNG;
				if( !( IMG_Init( imgFlags ) & imgFlags ) )
				{
				    cout << "SDL_image could not initialize! SDL_image Error: " << IMG_GetError() << endl;
					success = false;
				}

                //Initialize SDL_mixer
				if( Mix_OpenAudio( 44100, MIX_DEFAULT_FORMAT, 2, 2048 ) < 0 )
				{
				    cout << "SDL_mixer could not initialize! SDL_mixer Error: " << Mix_GetError() << endl;
					success = false;
				}

                //Initialize SDL_ttf
				if( TTF_Init() == -1 )
				{
				    cout << "SDL_ttf could not initialize! SDL_ttf Error: " << TTF_GetError() << endl;
					success = false;
				}
			}
		}
	}

	return success;
}

bool loadMedia()
{
	//Loading success flag
	bool success = true;

	//Open the font
	gGameOver = TTF_OpenFont( "Font/DTM-Sans.ttf", 40 );
	if( gGameOver == NULL )
	{
	    cout << "Failed to load DTM-Sans font! SDL_ttf Error: " << TTF_GetError() << endl;
		success = false;
	}
	else
	{
		//Render text
		SDL_Color textColor = { 140, 140, 140 };
		if( !gTextTexture.loadFromRenderedText( "GAME OVER :(", textColor ) )
		{
			cout << "Failed to render text texture!\n";
			success = false;
		}
	}

	gPlayAgainWin = TTF_OpenFont( "Font/DTM-Sans.ttf", 40 );
	if( gPlayAgainWin == NULL )
	{
	    cout << "Failed to load DTM-Sans font! SDL_ttf Error: " << TTF_GetError() << endl;
		success = false;
	}
	else
    {
        //Render text
		SDL_Color playAgainWin = { 30, 100, 100 };
		if( !gPlayAgainWinTexture.loadFromRenderedText( "Press s to play again!", playAgainWin ) )
		{
			cout << "Failed to render text texture!\n";
			success = false;
		}
    }

    gPlayAgainLose = TTF_OpenFont( "Font/DTM-Sans.ttf", 40 );
    if( gPlayAgainLose == NULL )
	{
	    cout << "Failed to load DTM-Sans font! SDL_ttf Error: " << TTF_GetError() << endl;
		success = false;
	}
	else
    {
        //Render text
		SDL_Color playAgainLose = { 140, 140, 140 };
		if( !gPlayAgainLoseTexture.loadFromRenderedText( "Press s to play again!", playAgainLose ) )
		{
			cout << "Failed to render text texture!\n";
			success = false;
		}
    }

	//Load scene
	if ( !gWinningTexture.loadFromFile( "Image/Winner.png" ) )
    {
        cout << "Failed to load winning texture!\n";
        success = false;
    }
    if ( !gBackgroundTexture.loadFromFile( "Image/Background.png" ) )
    {
        cout << "Failed to load background texture!\n";
        success = false;
    }

	//Load sprites
	if( !gButtonSpriteSheetTexture.loadFromFile( "Image/Tiles.png" ) )
	{
		cout << "Failed to load sprites texture!\n";
		success = false;
	}
	else
    {
        //Set sprites
		for( int i = 0; i < BUTTON_SPRITE_TOTAL; i++ )
		{
			gSpriteClips[ i ].x = i * 32;
			gSpriteClips[ i ].y = 0;
			gSpriteClips[ i ].w = TILE_SIZE;
			gSpriteClips[ i ].h = TILE_SIZE;
		}
		//Set buttons position
		for (int i = 0; i < ROW_SIZE; i++)
        {
            for (int j = 0; j < COLUMN_SIZE; j++)
            {
                gButtons[i][j].setPosition(j * TILE_SIZE + DISTANCE_BETWEEN, i * TILE_SIZE + DISTANCE_BETWEEN);
            }
        }
    }

	//Load sound effects
	winner = Mix_LoadMUS( "Sounds/winner.wav" );
	if( winner == NULL )
	{
		cout << "Failed to load winner sound effect! SDL_mixer Error: " << Mix_GetError() << endl;
		success = false;
	}

	loser = Mix_LoadMUS( "Sounds/loser.wav" );
	if( loser == NULL )
	{
		cout << "Failed to load loser sound effect! SDL_mixer Error: " << Mix_GetError() << endl;
		success = false;
	}

	click = Mix_LoadWAV( "Sounds/click.wav" );
	if( click == NULL )
	{
		cout << "Failed to load click sound effect! SDL_mixer Error: " << Mix_GetError() << endl;
		success = false;
	}

	return success;
}

void createTableWithMine ()
{
    srand(time(NULL));
    int mine = 0;
    for (int i = 0; i < ROW_SIZE; i++)
    {
        for (int j = 0; j < COLUMN_SIZE; j++)
        {
            sBoard[i][j] = 10;
            board[i][j] = 0;
        }
    }
    while (mine < MINE_COUNT)
    {
        int i = rand() % ROW_SIZE;
        int j = rand() % COLUMN_SIZE;
        if ( board[i][j] == 9 )
        {
            continue;
        }
        else
        {
            board[i][j] = 9;
            mine++;
            if (board[i-1][j] != 9 && i > 0)
                board[i-1][j]++;
            if (board[i][j-1] != 9 && j > 0)
                board[i][j-1]++;
            if (board[i+1][j] != 9 && i < ROW_SIZE - 1)
                board[i+1][j]++;
            if (board[i][j+1] != 9 && j < COLUMN_SIZE - 1)
                board[i][j+1]++;
            if (board[i-1][j-1] != 9 && i > 0 && j > 0)
                board[i-1][j-1]++;
            if (board[i-1][j+1] != 9 && i > 0 && j < COLUMN_SIZE - 1)
                board[i-1][j+1]++;
            if (board[i+1][j-1] != 9 && j > 0 && i < ROW_SIZE - 1)
                board[i+1][j-1]++;
            if (board[i+1][j+1] != 9 && i < ROW_SIZE - 1 && j < COLUMN_SIZE - 1)
                board[i+1][j+1]++;
        }
    }
}

bool checkWinning ()
{
    bool win = false;
    if (countTileLeft == MINE_COUNT)
    {
        win = true;
    }
    return win;
}

void mineManager()
{
    //Render text
    if ( !gameOver && !isWinning )
    {
        //Set text color
        SDL_Color textColor = { 140, 140, 140, 255 };

        //Erase the buffer
        mineLeft.str ( "" );
        mineLeft << "Mine left: " << countMineLeft;
        if( !gMineLeftTexture.loadFromRenderedText( mineLeft.str().c_str(), textColor ) )
        {
            cout << "Unable to render mine left texture!\n";
        }

        //Render text
        gMineLeftTexture.render( ( SCREEN_WIDTH - gMineLeftTexture.getWidth() ) / 2, 0 );
    }
}

void flagManager()
{
    //Check if win
    if ( isWinning && !gameOver )
    {
        //Update screen
        SDL_RenderPresent( gRenderer );

        //Delay loading screen
        SDL_Delay(500);

        //Play victory music
        Mix_PlayMusic(winner, 0);

        //Render winning scene
        gWinningTexture.render( 0, 0 );

        //Render playAgain
        gPlayAgainWinTexture.render( ( SCREEN_WIDTH - gPlayAgainWinTexture.getWidth() ) / 2, SCREEN_HEIGHT - gPlayAgainWinTexture.getHeight() );
    }

    //Check if lose
    if ( gameOver )
    {
        //Render game over text
        gTextTexture.render( ( SCREEN_WIDTH - gTextTexture.getWidth() ) / 2, 0 );

        //Play losing music
        Mix_PlayMusic(loser, 0);

        for( int i = 0; i < ROW_SIZE; i++ )
        {
            for ( int j = 0; j < COLUMN_SIZE; j++ )
            {
                sBoard[i][j] = board[i][j];
                gButtons[i][j].render(i, j);
            }
        }
        //Render play again
        gPlayAgainLoseTexture.render( ( SCREEN_WIDTH - gPlayAgainLoseTexture.getWidth() ) / 2, SCREEN_HEIGHT - gPlayAgainLoseTexture.getHeight() );
    }
}

void playAgainManager(bool &quitGame)
{
    //Event handler
    SDL_Event e;

    //Handle events on queue
    while( SDL_PollEvent( &e ) != 0 )
    {
        //User requests play again
        if( e.key.keysym.sym == SDLK_s )
        {
            //Stop the music
            Mix_HaltMusic();

            //Recreate constants
            countMineLeft = MINE_COUNT;
            countTileLeft = ROW_SIZE * COLUMN_SIZE;

            //Recreate flag
            gameOver = false;
            isWinning = false;
            quitGame = false;
        }
        else if ( e.key.keysym.sym == SDLK_ESCAPE ) quitGame = true;
    }
}

void close()
{
	//Free loaded images
	gButtonSpriteSheetTexture.free();
	gMineLeftTexture.free();
	gBackgroundTexture.free();
	gWinningTexture.free();
	gTextTexture.free();

	//Free global font
	TTF_CloseFont( gGameOver );
	TTF_CloseFont( gPlayAgainLose );
	TTF_CloseFont( gPlayAgainWin );
	gGameOver = NULL;
	gPlayAgainLose = NULL;
	gPlayAgainWin = NULL;

	//Free the sound effects
	Mix_FreeMusic( winner );
	Mix_FreeMusic( loser );
	Mix_FreeChunk( click );
	winner = NULL;
	loser = NULL;
	click = NULL;

	//Destroy window
	SDL_DestroyRenderer( gRenderer );
	SDL_DestroyWindow( gWindow );
	gWindow = NULL;
	gRenderer = NULL;

	//Quit SDL subsystems
	Mix_Quit();
	IMG_Quit();
	SDL_Quit();
}
