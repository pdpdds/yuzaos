	//
//  SDLGameObject.h
//  SDL Game Programming Book
//
//  Created by shaun mitchell on 19/01/2013.
//  Copyright (c) 2013 shaun mitchell. All rights reserved.
//

#ifndef __SDL_Game_Programming_Book__SDLGameObject__
#define __SDL_Game_Programming_Book__SDLGameObject__

#include <SDL.h>
#include "GameObject.h"

class PlatformerObject : public GameObject
{
    RTTI_DECLARATIONS(PlatformerObject, GameObject)
public:
    
    virtual ~PlatformerObject() {}

    virtual void Load(LoaderParams& params);
    
    virtual void Draw();
	virtual void Update();
    
    virtual void Clean() {}
    virtual void Collision() {}
    
    virtual std::string type() { return "SDLGameObject"; }

	int GetSpeed(){ return m_moveSpeed; }
	void SetSpeed(int speed){ m_moveSpeed = speed; }
    
protected:
    
    PlatformerObject();
    
    bool checkCollideTile(Vector2D newPos);
	virtual bool CheckCollision(GameObject* gameObject) override;
	int CheckCollideTileType(Vector2D& newPos);
	bool BreakTile(Vector2D newPos);
       
    void doDyingAnimation();
    
    int m_bulletFiringSpeed;
    int m_bulletCounter;
    int m_moveSpeed;
    
    // how long the death animation takes, along with a counter
    int m_dyingTime;
    int m_dyingCounter;
    
    // has the explosion sound played?
    bool m_bPlayedDeathSound;
    
    bool m_bFlipped;
    
    bool m_bRunning;
    
    Vector2D m_lastSafePos;        
};

#endif /* defined(__SDL_Game_Programming_Book__SDLGameObject__) */
