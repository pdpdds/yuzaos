//
//  CollisionManager.cpp
//  SDL Game Programming Book
//
//  Created by shaun mitchell on 28/03/2013.
//  Copyright (c) 2013 shaun mitchell. All rights reserved.
//

#include "CollisionManager.h"
#include "Collision.h"
#include "Player.h"
#include "Enemy.h"
#include "TileLayer.h"

void CollisionManager::checkPlayerEnemyCollision(Player* pPlayer, const std::vector<GameObject*> &objects)
{
    SDL_Rect rect1;
	rect1.x = pPlayer->getPosition().getX();
	rect1.y = pPlayer->getPosition().getY();
	rect1.w = pPlayer->getWidth();
	rect1.h = pPlayer->getHeight();
    
    for(int i = 0; i < objects.size(); i++)
    {
        if(objects[i]->type() != std::string("Enemy") || !objects[i]->updating())
        {
            continue;
        }
        
		SDL_Rect rect2;
		rect2.x = objects[i]->getPosition().getX();
		rect2.y = objects[i]->getPosition().getY();
		rect2.w = objects[i]->getWidth();
		rect2.h = objects[i]->getHeight();
        
		if (RectRect(&rect1, &rect2))
        {
            if(!objects[i]->dead() && !objects[i]->dying())
            {
                pPlayer->Collision();
            }
        }              
    }
}