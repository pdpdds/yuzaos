//
//  GameOverState.cpp
//  SDL Game Programming Book
//
//  Created by shaun mitchell on 17/02/2013.
//  Copyright (c) 2013 shaun mitchell. All rights reserved.
//

#include "GameOverState.h"
#include "MainMenuState.h"
#include "PlayState.h"
#include "TextureManager.h"
#include "AnimatedGraphic.h"
#include "Game.h"
#include "MenuButton.h"
#include "InputHandler.h"
#include "StateParser.h"

const std::string GameOverState::s_gameOverID = "GAMEOVER";

void GameOverState::s_gameOverToMain()
{
    TheGame::Instance()->getStateMachine()->ChangeState(new MainMenuState());
}

void GameOverState::s_restartPlay()
{
    TheGame::Instance()->getStateMachine()->ChangeState(new PlayState());
}

void GameOverState::Update()
{
    if(m_loadingComplete && !m_gameObjects.empty())
    {
        for(int i = 0; i < m_gameObjects.size(); i++)
        {
			m_gameObjects[i]->Update();
        }
    }
}

void GameOverState::Render()
{
    if(m_loadingComplete && !m_gameObjects.empty())
    {
        for(int i = 0; i < m_gameObjects.size(); i++)
        {
            m_gameObjects[i]->Draw();
        }
    }
}

bool GameOverState::OnEnter()
{
    // parse the state
    StateParser stateParser;
    stateParser.parseState("assets/conan.xml", s_gameOverID, &m_gameObjects, &m_textureIDList);
    
    m_callbacks.push_back(0);
    m_callbacks.push_back(s_gameOverToMain);
    m_callbacks.push_back(s_restartPlay);
    
    // set the callbacks for menu items
    setCallbacks(m_callbacks);
    
    m_loadingComplete = true;
    
    std::cout << "entering GameOverState\n";
    return true;
}

bool GameOverState::onExit()
{
    if(m_loadingComplete && !m_gameObjects.empty())
    {
        for(int i = 0; i < m_gameObjects.size(); i++)
        {
			m_gameObjects[i]->Clean();
			GameObject* pObject = m_gameObjects[i];
            delete pObject;
        }
        
        m_gameObjects.clear();
    }
    
    std::cout << m_gameObjects.size();
    
    // clear the texture manager
    for(int i = 0; i < m_textureIDList.size(); i++)
    {
        TheTextureManager::Instance()->clearFromTextureMap(m_textureIDList[i]);
    }
    
    TheInputHandler::Instance()->reset();
    
    std::cout << "exiting GameOverState\n";
    return true;
}

void GameOverState::setCallbacks(const std::vector<Callback>& callbacks)
{
    // go through the game objects
    for(int i = 0; i < m_gameObjects.size(); i++)
    {
        // if they are of type MenuButton then assign a callback based on the id passed in from the file
        MenuButton* pButton = m_gameObjects[i]->As<MenuButton>();
		if (pButton)
        {
            pButton->setCallback(callbacks[pButton->getCallbackID()]);
        }
    }
}

