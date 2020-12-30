/*This source code copyrighted by Lazy Foo' Productions (2004-2015)
and may not be redistributed without written permission.*/

//Using SDL, SDL_image, standard IO, vectors, and strings
#include <SDL.h>
#include <SDL_image.h>
#include <stdio.h>
#include <string>
#include <time.h> 

//Screen dimension constants
const int SCREEN_WIDTH = 640;
const int SCREEN_HEIGHT = 480;

//The dimensions of the pipe		
const int PIPE_WIDTH = 50; 
const int PIPE_GAP = 250;

//The dimensions of the bird
static const int BIRD_WIDTH = 47;
static const int BIRD_HEIGHT = 32;

//Texture wrapper class
class LTexture
{
	public:
		//Initializes variables
		LTexture();

		//Deallocates memory
		~LTexture();

		//Loads image at specified path
		bool loadFromFile( std::string path );
		
		#ifdef _SDL_TTF_H
		//Creates image from font string
		bool loadFromRenderedText( std::string textureText, SDL_Color textColor );
		#endif

		//Deallocates texture
		void free();

		//Set color modulation
		void setColor( Uint8 red, Uint8 green, Uint8 blue );

		//Set blending
		void setBlendMode( SDL_BlendMode blending );

		//Set alpha modulation
		void setAlpha( Uint8 alpha );
		
		//Renders texture at given point
		void render( int x, int y, SDL_Rect* clip = NULL, double angle = 0.0, SDL_Point* center = NULL, SDL_RendererFlip flip = SDL_FLIP_NONE );

		//Gets image dimensions
		int getWidth();
		int getHeight();

	private:
		//The actual hardware texture
		SDL_Texture* mTexture;

		//Image dimensions
		int mWidth;
		int mHeight;
};

//The dot that will move around on the screen
class Bird
{
    public:

		//Initializes the variables
		Bird();
	
		int getmPosX();
		int getmPosY();
		void changeY(float accel);

		//Takes key presses makes the bird jump
		void handleEvent( SDL_Event& e );

		void move(int &loseFlag);

		//Shows the bird on the screen
		void render();

    private:
		//The X and Y offsets of the bird
		int mPosX, mPosY;

		//The velocity of the bird
		float mVelY;
};

class Pipe
{
public:

	Pipe();
	Pipe(int posX);
	int getPipePosX();
	int getSpaceStart();
	void setPipePosX(int posX);
	void setSpaceStart(int spaceY);
	void render();

private:
	int pipePosX;
	int spaceStart;
};

bool checkCollision(Pipe p, Bird b);


//Starts up SDL and creates window
bool init();

//Loads media
bool loadMedia();

//Frees media and shuts down SDL
void close();

//The window we'll be rendering to
SDL_Window* gWindow = NULL;

//The window renderer
SDL_Renderer* gRenderer = NULL;

//Scene textures
LTexture gDotTexture;
LTexture gBGTexture;

LTexture::LTexture()
{
	//Initialize
	mTexture = NULL;
	mWidth = 0;
	mHeight = 0;
}

LTexture::~LTexture()
{
	//Deallocate
	free();
}

bool LTexture::loadFromFile( std::string path )
{
	//Get rid of preexisting texture
	free();

	//The final texture
	SDL_Texture* newTexture = NULL;

	//Load image at specified path
	SDL_Surface* loadedSurface = IMG_Load( path.c_str() );
	if( loadedSurface == NULL )
	{
		printf( "Unable to load image %s! SDL_image Error: %s\n", path.c_str(), IMG_GetError() );
	}
	else
	{
		//Color key image
		SDL_SetColorKey( loadedSurface, SDL_TRUE, SDL_MapRGB( loadedSurface->format, 0xFF, 0xFF, 0xFF ) );

		//Create texture from surface pixels
        newTexture = SDL_CreateTextureFromSurface( gRenderer, loadedSurface );
		if( newTexture == NULL )
		{
			printf( "Unable to create texture from %s! SDL Error: %s\n", path.c_str(), SDL_GetError() );
		}
		else
		{
			//Get image dimensions
			mWidth = loadedSurface->w;
			mHeight = loadedSurface->h;
		}

		//Get rid of old loaded surface
		SDL_FreeSurface( loadedSurface );
	}

	//Return success
	mTexture = newTexture;
	return mTexture != NULL;
}

#ifdef _SDL_TTF_H
bool LTexture::loadFromRenderedText( std::string textureText, SDL_Color textColor )
{
	//Get rid of preexisting texture
	free();

	//Render text surface
	SDL_Surface* textSurface = TTF_RenderText_Solid( gFont, textureText.c_str(), textColor );
	if( textSurface != NULL )
	{
		//Create texture from surface pixels
        mTexture = SDL_CreateTextureFromSurface( gRenderer, textSurface );
		if( mTexture == NULL )
		{
			printf( "Unable to create texture from rendered text! SDL Error: %s\n", SDL_GetError() );
		}
		else
		{
			//Get image dimensions
			mWidth = textSurface->w;
			mHeight = textSurface->h;
		}

		//Get rid of old surface
		SDL_FreeSurface( textSurface );
	}
	else
	{
		printf( "Unable to render text surface! SDL_ttf Error: %s\n", TTF_GetError() );
	}

	
	//Return success
	return mTexture != NULL;
}
#endif

void LTexture::free()
{
	//Free texture if it exists
	if( mTexture != NULL )
	{
		SDL_DestroyTexture( mTexture );
		mTexture = NULL;
		mWidth = 0;
		mHeight = 0;
	}
}

void LTexture::setColor( Uint8 red, Uint8 green, Uint8 blue )
{
	//Modulate texture rgb
	SDL_SetTextureColorMod( mTexture, red, green, blue );
}

void LTexture::setBlendMode( SDL_BlendMode blending )
{
	//Set blending function
	SDL_SetTextureBlendMode( mTexture, blending );
}
		
void LTexture::setAlpha( Uint8 alpha )
{
	//Modulate texture alpha
	SDL_SetTextureAlphaMod( mTexture, alpha );
}

void LTexture::render( int x, int y, SDL_Rect* clip, double angle, SDL_Point* center, SDL_RendererFlip flip )
{
	//Set rendering space and render to screen
	SDL_Rect renderQuad = { x, y, mWidth, mHeight };

	//Set clip rendering dimensions
	if( clip != NULL )
	{
		renderQuad.w = clip->w;
		renderQuad.h = clip->h;
	}

	//Render to screen
	SDL_RenderCopyEx( gRenderer, mTexture, clip, &renderQuad, angle, center, flip );
}

int LTexture::getWidth()
{
	return mWidth;
}

int LTexture::getHeight()
{
	return mHeight;
}

Bird::Bird()
{
    //Initialize the offsets
    mPosX = 60;
    mPosY = 0;

    //Initialize the velocity
    mVelY = 0;
}
int Bird::getmPosX()
{
	return mPosX;
}
int Bird::getmPosY()
{
	return mPosY;
}
void Bird::changeY(float accel)
{
	mVelY += accel;
}

void Bird::handleEvent( SDL_Event& e )
{
    //If a key was pressed, make it jump with upward velocity of 12
	if( e.type == SDL_KEYDOWN && e.key.repeat == 0 )
    {
		mVelY = -12;
	}
}

void Bird::move(int &loseFlag)
{
    //Move the dot up or down
    mPosY += mVelY;

    //If the dot went too far up or down, lose the game
	if (mPosY < 0)
	{
		mPosY = 0;
		loseFlag = 1;
	}
    if( mPosY + BIRD_HEIGHT > SCREEN_HEIGHT )
    {
        mPosY = SCREEN_HEIGHT - BIRD_HEIGHT;
		loseFlag = 1;
    }
}

void Bird::render()
{
    //Show the dot
	gDotTexture.render( mPosX, mPosY );
}

Pipe::Pipe()
{
	pipePosX = SCREEN_WIDTH;
	spaceStart = rand() % (SCREEN_HEIGHT - PIPE_GAP);
}
Pipe::Pipe(int posX)
{
	pipePosX = posX;
	spaceStart = rand() % (SCREEN_HEIGHT - PIPE_GAP);
}
int Pipe::getPipePosX()
{
	return pipePosX;
}
int Pipe::getSpaceStart()
{
	return spaceStart;
}
void Pipe::setPipePosX(int posX)
{
	//if it goes off screen, bring it to the beginning of the screen and randomize the spaceStart
	if (posX + PIPE_WIDTH < 0)
	{
		pipePosX = SCREEN_WIDTH;
		spaceStart = rand() % (SCREEN_HEIGHT - PIPE_GAP);
	}
	else
		pipePosX = posX;
}
void Pipe::setSpaceStart(int spaceY)
{
	spaceStart = spaceY;
}
void Pipe::render()
{
	SDL_Rect pipeTop = { pipePosX, -1, PIPE_WIDTH, spaceStart };
	SDL_Rect pipeBottom = { pipePosX, spaceStart + PIPE_GAP, PIPE_WIDTH, SCREEN_HEIGHT - (spaceStart + PIPE_GAP) + 1 };
	SDL_SetRenderDrawColor(gRenderer, 0, 0xFF, 0, 0xFF);
	SDL_RenderFillRect(gRenderer, &pipeTop);
	SDL_RenderFillRect(gRenderer, &pipeBottom);

	//outline the rectangles, don't outline top line for top pipe, don't outline bottom line for bottom pipe
	SDL_SetRenderDrawColor(gRenderer, 0, 0, 0, 0xFF);
	SDL_RenderDrawRect(gRenderer, &pipeTop);
	SDL_RenderDrawRect(gRenderer, &pipeBottom);
}

bool checkCollision(Pipe p, Bird b)
{
	SDL_Rect pipeTop = { p.getPipePosX(), -1, PIPE_WIDTH, p.getSpaceStart() };
	SDL_Rect pipeBottom = { p.getPipePosX(), p.getSpaceStart() + PIPE_GAP, PIPE_WIDTH, SCREEN_HEIGHT - (p.getSpaceStart() + PIPE_GAP) + 1 };

	//bird collision dimensions based on the bird picture
	SDL_Rect birdXCollision = { b.getmPosX() + 2, b.getmPosY() + 9, BIRD_WIDTH - 1 - 2, BIRD_HEIGHT - 9 - 1 };
	SDL_Rect birdYCollision = { b.getmPosX() + 17, b.getmPosY(), 11, BIRD_HEIGHT };

	int pipeTopLeft = pipeTop.x;
	int pipeTopRight = pipeTop.x + pipeTop.w;
	int pipeTopTop = -1;
	int pipeTopBottom = 0 + pipeTop.h;

	int pipeBottomLeft = pipeBottom.x;
	int pipeBottomRight = pipeBottom.x + pipeBottom.w;
	int pipeBottomTop = pipeBottom.y;
	int pipeBottomBottom = pipeBottom.y + pipeBottom.h;

	int birdXLeft = birdXCollision.x;
	int birdXRight = birdXCollision.x + birdXCollision.w;
	int birdXTop = birdXCollision.y;
	int birdXBottom = birdXCollision.y + birdXCollision.h;

	int birdYLeft = birdYCollision.x;
	int birdYRight = birdYCollision.x + birdYCollision.w;
	int birdYTop = birdYCollision.y;
	int birdYBottom = birdYCollision.y + birdYCollision.h;

	if (pipeTopBottom > birdXTop && pipeTopTop < birdXBottom && pipeTopRight > birdXLeft && pipeTopLeft < birdXRight)
		return true;
	if (pipeTopBottom > birdYTop && pipeTopTop < birdYBottom && pipeTopRight > birdYLeft && pipeTopLeft < birdYRight)
		return true;
	if (pipeBottomBottom > birdXTop && pipeBottomTop < birdXBottom && pipeBottomRight > birdXLeft && pipeBottomLeft < birdXRight)
		return true;
	if (pipeBottomBottom > birdYTop && pipeBottomTop < birdYBottom && pipeBottomRight > birdYLeft && pipeBottomLeft < birdYRight)
		return true;
	return false;
}

bool init()
{
	//Initialization flag
	bool success = true;

	//initialize rand
	srand(time(NULL));

	//Initialize SDL
	if( SDL_Init( SDL_INIT_VIDEO ) < 0 )
	{
		printf( "SDL could not initialize! SDL Error: %s\n", SDL_GetError() );
		success = false;
	}
	else
	{
		//Set texture filtering to linear
		if( !SDL_SetHint( SDL_HINT_RENDER_SCALE_QUALITY, "1" ) )
		{
			printf( "Warning: Linear texture filtering not enabled!" );
		}

		//Create window
		gWindow = SDL_CreateWindow( "SDL Tutorial", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN );
		if( gWindow == NULL )
		{
			printf( "Window could not be created! SDL Error: %s\n", SDL_GetError() );
			success = false;
		}
		else
		{
			//Create vsynced renderer for window
			gRenderer = SDL_CreateRenderer( gWindow, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC );
			if( gRenderer == NULL )
			{
				printf( "Renderer could not be created! SDL Error: %s\n", SDL_GetError() );
				success = false;
			}
			else
			{
				//Initialize renderer color
				SDL_SetRenderDrawColor( gRenderer, 0xFF, 0xFF, 0xFF, 0xFF );

				//Initialize PNG loading
				int imgFlags = IMG_INIT_PNG;
				if( !( IMG_Init( imgFlags ) & imgFlags ) )
				{
					printf( "SDL_image could not initialize! SDL_image Error: %s\n", IMG_GetError() );
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

	//Load dot texture
	if( !gDotTexture.loadFromFile( "bird.png" ) )
	{
		printf( "Failed to load dot texture!\n" );
		success = false;
	}

	//Load background texture
	if( !gBGTexture.loadFromFile( "flappybg.jpg" ) )
	{
		printf( "Failed to load background texture!\n" );
		success = false;
	}

	return success;
}

void close()
{
	//Free loaded images
	gDotTexture.free();
	gBGTexture.free();

	//Destroy window	
	SDL_DestroyRenderer( gRenderer );
	SDL_DestroyWindow( gWindow );
	gWindow = NULL;
	gRenderer = NULL;

	//Quit SDL subsystems
	IMG_Quit();
	SDL_Quit();
}

int main( int argc, char* args[] )
{
	//Start up SDL and create window
	if( !init() )
	{
		printf( "Failed to initialize!\n" );
	}
	else
	{
		//Load media
		if( !loadMedia() )
		{
			printf( "Failed to load media!\n" );
		}
		else
		{	
			//Main loop flag
			bool quit = false;

			//Event handler
			SDL_Event e;

			//The bird that will be moving around on the screen
			Bird bird;
			Pipe pipe1 = Pipe(2 * SCREEN_WIDTH / 3 - PIPE_WIDTH /3);
			Pipe pipe2 = Pipe(SCREEN_WIDTH);
			Pipe pipe3 = Pipe(4 * SCREEN_WIDTH / 3 + PIPE_WIDTH / 3);


			//The background scrolling offset
			int scrollingOffset = 0;
			int speed = 2;

			//acceleration 
			float accel = 0.5;

			int loseFlag = 0;

			//While application is running
			while( !quit )
			{
					//Handle events on queue
					while (SDL_PollEvent(&e) != 0)
					{
						//User requests quit
						if (e.type == SDL_QUIT)
						{
							quit = true;
						}

						//Handle input for the bird
						bird.handleEvent(e);
					}

					bird.changeY(accel);

					//Move the bird
					bird.move(loseFlag);

					if (checkCollision(pipe1, bird) == true || checkCollision(pipe2, bird) == true || checkCollision(pipe3, bird) == true)
						loseFlag = 1;

					//Scroll background
					scrollingOffset -= speed;
					if (scrollingOffset < -gBGTexture.getWidth())
					{
						scrollingOffset = 0;
					}

					//Clear screen
					SDL_SetRenderDrawColor(gRenderer, 0xFF, 0xFF, 0xFF, 0xFF);
					SDL_RenderClear(gRenderer);

					if (loseFlag == 1)
					{
						scrollingOffset = 0;
						speed = 0;
					}

					//Render background
					gBGTexture.render(scrollingOffset, 0);
					gBGTexture.render(scrollingOffset + gBGTexture.getWidth(), 0);
					gBGTexture.render(scrollingOffset + 2 * gBGTexture.getWidth(), 0);
					gBGTexture.render(scrollingOffset + 3 * gBGTexture.getWidth(), 0);

					//Render objects
					pipe1.setPipePosX(pipe1.getPipePosX() - speed);
					pipe1.render();
					pipe2.setPipePosX(pipe2.getPipePosX() - speed);
					pipe2.render();
					pipe3.setPipePosX(pipe3.getPipePosX() - speed);
					pipe3.render();
					bird.render();
					
					//show hit box
					/*
					SDL_SetRenderDrawColor(gRenderer, 0xFF, 0, 0, 0xFF);
					SDL_Rect birdXCollision = { bird.getmPosX() + 2, bird.getmPosY() + 9, BIRD_WIDTH - 1 - 2, BIRD_HEIGHT - 9 - 1 };
					SDL_Rect birdYCollision = { bird.getmPosX() + 17, bird.getmPosY(), 11, BIRD_HEIGHT };
					SDL_RenderDrawRect(gRenderer, &birdXCollision);
					SDL_RenderDrawRect(gRenderer, &birdYCollision);*/
					
					//Update screen
					SDL_RenderPresent(gRenderer);
			}
		}
	}

	//Free resources and close SDL
	close();

	return 0;
}