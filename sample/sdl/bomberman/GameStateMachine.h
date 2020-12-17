//
//  GameStateMachine.h
//  SDL Game Programming Book
//
//  Created by shaun mitchell on 09/02/2013.
//  Copyright (c) 2013 shaun mitchell. All rights reserved.
//

#ifndef __SDL_Game_Programming_Book__GameStateMachine__
#define __SDL_Game_Programming_Book__GameStateMachine__

#include <vector>
#include "GameState.h"

class GameStateMachine
{
public:
    
    GameStateMachine() {}
    ~GameStateMachine() {}
    
	void Update();
    void Render();
    
    void PushState(GameState* pState);
    void ChangeState(GameState* pState);
    void PopState();
    
    void Clean();

	std::vector<GameState*>& getGameStates() { return m_gameStates; }
    
private:
    std::vector<GameState*> m_gameStates;
};

#endif /* defined(__SDL_Game_Programming_Book__GameStateMachine__) */
