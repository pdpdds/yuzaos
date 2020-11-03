//
//  PlayState.cpp
//  SDL Game Programming Book
//
//  Created by shaun mitchell on 09/02/2013.
//  Copyright (c) 2013 shaun mitchell. All rights reserved.
//

#include <iostream>
#include "PlayState.h"
#include "GameOverState.h"
#include "PauseState.h"
#include "Game.h"
#include "InputHandler.h"
#include "LevelParser.h"
#include "Level.h"
#include "TextureManager.h"
#include "ResourceConstant.h"
#include <iostream>
#include <fstream>

const std::string PlayState::s_playID = "PLAY";

void PlayState::Update()
{
    if(m_loadingComplete && !m_exiting)
    {
        if(TheInputHandler::Instance()->isKeyDown(SDL_SCANCODE_ESCAPE))
        {
            TheGame::Instance()->getStateMachine()->PushState(new PauseState());
        }
        
//        if(TheInputHandler::Instance()->getButtonState(0, 8))
//        {
//            TheGame::Instance()->getStateMachine()->PushState(new PauseState());
//        }
        
        if(TheGame::Instance()->getPlayerLives() == 0)
        {
            TheGame::Instance()->getStateMachine()->ChangeState(new GameOverState());
        }
        
        if(pLevel != 0)
        {
			pLevel->Update();
        }
    }
}

void PlayState::Render()
{
    if(m_loadingComplete)
    {
        if(pLevel != 0)
        {
            pLevel->Render();
        }
        
        for(int i = 0; i < TheGame::Instance()->getPlayerLives(); i++)
        {
            TheTextureManager::Instance()->drawFrame("lives", i * 30, 0, 32, 30, 0, 0, TheGame::Instance()->getRenderer(), 0.0, 255);
        }
    }
}

bool PlayState::OnEnter()
{
	
    TheGame::Instance()->setPlayerLives(3);

	LevelParser levelParser;
	//pLevel = levelParser.parseLevel(TheGame::Instance()->getLevelFiles()[TheGame::Instance()->getCurrentLevel() - 1].c_str());
	pLevel = levelParser.parseLevel("data/map1.tmx");

	char szFilePath[256] = { 0, };
	char keyColor = 0;

	char* file_contents = NULL;

	if (LevelParser::read_text("data/config/images.txt", &file_contents) != true)
	{
		return false;
	}

	std::istringstream stream(file_contents);

	for (int j = 0; j<_IMAGENES; j++)
	{
		stream >> szFilePath;
		stream >> keyColor;
		TextureManager::Instance()->load(szFilePath, szFilePath, TheGame::Instance()->getRenderer(), (char)keyColor);
	}
		
	m_loadingComplete = true;    
    
    std::cout << "entering PlayState\n";
    return true;
}

bool PlayState::onExit()
{
    m_exiting = true;
    
    TheInputHandler::Instance()->reset();
    
    std::cout << "exiting PlayState\n";
    return true;
}
