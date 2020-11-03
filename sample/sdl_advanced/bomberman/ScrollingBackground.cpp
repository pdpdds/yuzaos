//
//  ScrollingBackground.cpp
//  SDL Game Programming Book
//
//  Created by shaun mitchell on 26/03/2013.
//  Copyright (c) 2013 shaun mitchell. All rights reserved.
//

#include "ScrollingBackground.h"
#include "TextureManager.h"
#include "Game.h"

ScrollingBackground::ScrollingBackground() : PlatformerObject()
{
    count = 0;
    maxcount = 10;
}

void ScrollingBackground::Load(LoaderParams& params)
{
	PlatformerObject::Load(params);
	m_scrollSpeed = params.getAnimSpeed();
    
    m_scrollSpeed = 1;
    
    m_srcRect1.x = 0;
    m_destRect1.x = m_position.getX();
    m_srcRect1.y = 0;
    m_destRect1.y = m_position.getY();
    
    m_srcRect1.w = m_destRect1.w = m_srcRect2Width = m_destRect1Width = m_width;
    m_srcRect1.h = m_destRect1.h = m_height;
    
    m_srcRect2.x = 0;
    m_destRect2.x = m_position.getX() + m_width;
    m_srcRect2.y = 0;
    m_destRect2.y = m_position.getY();
    
    m_srcRect2.w = m_destRect2.w = m_srcRect2Width = m_destRect2Width = 0;
    m_srcRect2.h = m_destRect2.h = m_height;
}

void ScrollingBackground::Draw()
{
    // draw first rect

	SDL_Rect rect1;
	rect1.x = SDL_RATIO_X(m_destRect1.x);
	rect1.y = SDL_RATIO_Y(m_destRect1.y);
	rect1.w = SDL_RATIO_X(m_destRect1.w);
	rect1.h = SDL_RATIO_Y(m_destRect1.h);

	SDL_RenderCopyEx(TheGame::Instance()->getRenderer(), TheTextureManager::Instance()->getTextureMap()[m_textureID], &m_srcRect1, &rect1, 0, 0, SDL_FLIP_NONE);
    
	SDL_Rect rect2;
	rect2.x = SDL_RATIO_X(m_destRect2.x);
	rect2.y = SDL_RATIO_Y(m_destRect2.y);
	rect2.w = SDL_RATIO_X(m_destRect2.w);
	rect2.h = SDL_RATIO_Y(m_destRect2.h);

    // draw second rect
	SDL_RenderCopyEx(TheGame::Instance()->getRenderer(), TheTextureManager::Instance()->getTextureMap()[m_textureID], &m_srcRect2, &rect2, 0, 0, SDL_FLIP_NONE);
}

void ScrollingBackground::Update()
{
    if(count == maxcount)
    {
        // make first rectangle smaller
        m_srcRect1.x += m_scrollSpeed;
        m_srcRect1.w -= m_scrollSpeed;
        m_destRect1.w -= m_scrollSpeed;
        
        // make second rectangle bigger
        m_srcRect2.w += m_scrollSpeed;
        m_destRect2.w += m_scrollSpeed;
        m_destRect2.x -= m_scrollSpeed;
        
        // reset and start again
        if(m_destRect2.w >= m_width)
        {
            m_srcRect1.x = 0;
            m_destRect1.x = m_position.getX();
            m_srcRect1.y = 0;
            m_destRect1.y = m_position.getY();
            
            m_srcRect1.w = m_destRect1.w = m_srcRect2Width = m_destRect1Width = m_width;
            m_srcRect1.h = m_destRect1.h = m_height;
            
            m_srcRect2.x = 0;
            m_destRect2.x = m_position.getX() + m_width;
            m_srcRect2.y = 0;
            m_destRect2.y = m_position.getY();
            
            m_srcRect2.w = m_destRect2.w = m_srcRect2Width = m_destRect2Width = 0;
            m_srcRect2.h = m_destRect2.h = m_height;
        }
        count = 0;
    }
    
    count++;
}

void ScrollingBackground::Clean()
{
    PlatformerObject::Clean();
}
