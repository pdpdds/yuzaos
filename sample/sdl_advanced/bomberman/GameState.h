//
//  IGameState.h
//  SDL Game Programming Book
//
//  Created by shaun mitchell on 09/02/2013.
//  Copyright (c) 2013 shaun mitchell. All rights reserved.
//

#ifndef SDL_Game_Programming_Book_IGameState_h
#define SDL_Game_Programming_Book_IGameState_h

#include <string>
#include <vector>

class GameState
{
public:
    
    virtual ~GameState() {}
    
	virtual void Update() = 0;
    virtual void Render() = 0;
    
    virtual bool OnEnter() = 0;
    virtual bool onExit() = 0;
    
    virtual void Resume() {}
    
    virtual std::string getStateID() const = 0;
    
protected:
    
    GameState() : m_loadingComplete(false), m_exiting(false)
    {
        
    }
    
    bool m_loadingComplete;
    bool m_exiting;
    
    std::vector<std::string> m_textureIDList;
};

#endif
