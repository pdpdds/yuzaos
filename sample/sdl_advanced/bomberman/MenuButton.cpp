//
//  MenuObject.cpp
//  SDL Game Programming Book
//
//  Created by shaun mitchell on 10/02/2013.
//  Copyright (c) 2013 shaun mitchell. All rights reserved.
//

#include "MenuButton.h"
#include "InputHandler.h"
#include "Game.h"

MenuButton::MenuButton() : PlatformerObject(), m_callback(0)
{
}

void MenuButton::Load(LoaderParams& params)
{
	PlatformerObject::Load(params);
	m_callbackID = params.getCallbackID();
    m_currentFrame = MOUSE_OUT;
}

void MenuButton::Draw()
{
    PlatformerObject::Draw();
}

void MenuButton::Update()
{
    Vector2D* pMousePos = TheInputHandler::Instance()->getMousePosition();
    
	if (pMousePos->getX() < (SDL_RATIO_X(m_position.getX()) + SDL_RATIO_X(m_width)) && pMousePos->getX() > SDL_RATIO_X(m_position.getX())
		&& pMousePos->getY() < (SDL_RATIO_Y(m_position.getY()) + SDL_RATIO_Y(m_height)) && pMousePos->getY() > SDL_RATIO_Y(m_position.getY()))
    {
        if(TheInputHandler::Instance()->isMouseButtonUp(LEFT))
        {
            m_currentFrame = CLICKED;
            
            if(m_callback != 0)
            {
                m_callback();
            }
        }
        else if(!TheInputHandler::Instance()->isMouseButtonDown(LEFT))
        {
            m_currentFrame = MOUSE_OVER;
        }
    }
    else
    {
        m_currentFrame = MOUSE_OUT;
    }
}

void MenuButton::Clean()
{
    PlatformerObject::Clean();
}
