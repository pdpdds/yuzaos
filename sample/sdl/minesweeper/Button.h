#pragma once

#include "Variables.h"
#include "Constances.h"

//Reveal hidden tiles
void reveal(int i, int j);

//The mouse button
class LButton
{
	public:
		//Initializes internal variables
		LButton();

		//Sets top left position
		void setPosition( int x, int y );

		//Handles mouse event
		void handleEvent( SDL_Event* e );

		//Shows button sprite
		void render(int i, int j);

	private:
		//Top left position
		SDL_Point mPosition;
};

