//
//  Game.cpp
//  SDL Game Programming Book
//
//
#include "Game.h"
#include "TextureManager.h"
#include "InputHandler.h"
#include "MainMenuState.h"
#include "PlayState.h"
#include "GameObjectFactory.h"
#include "MenuButton.h"
#include "AnimatedGraphic.h"
#include "Player.h"
#include "ScrollingBackground.h"
#include "SoundManager.h"
#include "GameOverState.h"
#include "Bomb.h"
#include "Wall.h"
#include "Explosion.h"
#include <iostream>
#include "ResourceConstant.h"

using namespace std;

Game* Game::s_pInstance = 0;

Game::Game():
m_pWindow(0),
m_pRenderer(0),
m_bRunning(false),
m_pGameStateMachine(0),
m_playerLives(3),
m_bLevelComplete(false)
{
    // add some level files to an array
    m_levelFiles.push_back("assets/map1.tmx");
    
    // start at this level
    m_currentLevel = 1;
}

Game::~Game()
{
    // we must clean up after ourselves to prevent memory leaks
    m_pRenderer= 0;
    m_pWindow = 0;
}


bool Game::init(const char* title, int xpos, int ypos, int width, int height, bool fullscreen)
{
    int flags = 0;
    
    // store the game width and height
    m_gameWidth = width;
    m_gameHeight = height;
    
    if(fullscreen)
    {
        flags = SDL_WINDOW_FULLSCREEN;
    }
    
    // attempt to initialise SDL
    //if(SDL_Init(SDL_INIT_EVERYTHING) == 0)
    if (SDL_Init(SDL_INIT_AUDIO | SDL_INIT_VIDEO) == 0)
    {
        cout << "SDL init success\n";
        // init the window
        m_pWindow = SDL_CreateWindow(title, xpos, ypos, width, height, flags);
        
        if(m_pWindow != 0) // window init success
        {
			int w, h;
			SDL_GetWindowSize(m_pWindow, &w, &h);

			m_gameWidth = w;
			m_gameHeight = h;

            cout << "window creation success\n";
            m_pRenderer = SDL_CreateRenderer(m_pWindow, -1, SDL_RENDERER_SOFTWARE);
            
            if(m_pRenderer != 0) // renderer init success
            {
                cout << "renderer creation success\n";
                SDL_SetRenderDrawColor(m_pRenderer, 0,0,0,255);
            }
            else
            {
                cout << "renderer init fail\n";
                return false; // renderer init fail
            }
        }
        else
        {
            cout << "window init fail\n";
            return false; // window init fail
        }
    }
    else
    {
        cout << "SDL init fail\n";
        return false; // SDL init fail
    }

	/*if (false == LoadSound())
	{
		cout << "Sound Load Fail\n";
		return false; 
	}*/
	
	
	
    // add some sound effects - TODO move to better place
    TheSoundManager::Instance()->load("assets/DST-Away.ogg", "music1", SOUND_MUSIC);
    TheSoundManager::Instance()->load("assets/jump.wav", "jump", SOUND_SFX);
    //TheSoundManager::Instance()->load("phaser.wav", "shoot", SOUND_SFX);
    
    TheSoundManager::Instance()->playMusic("music1", -1);
    
    
    TheInputHandler::Instance()->initialiseJoysticks();
    
    // register the types for the game
    TheGameObjectFactory::Instance()->registerType("MenuButton", new MenuButtonCreator());
    TheGameObjectFactory::Instance()->registerType("AnimatedGraphic", new AnimatedGraphicCreator());
    TheGameObjectFactory::Instance()->registerType("ScrollingBackground", new ScrollingBackgroundCreator());    
	TheGameObjectFactory::Instance()->registerType("Wall", new WallCreator());
	TheGameObjectFactory::Instance()->registerType("Explosion", new ExplosionCreator());
    

    // start the menu state
    m_pGameStateMachine = new GameStateMachine();
	//TheGame::Instance()->getStateMachine()->ChangeState(new PlayState());
    m_pGameStateMachine->ChangeState(new MainMenuState());
    
    m_bRunning = true; // everything inited successfully, start the main loop
    return true;
}

bool Game::LoadSound()
{
	char szFileName[256] = { 0, };
	char szResourceName[256] = { 0, };
	
	for (int i = 0; i < _SFX_FILE_COUNT; i++)
	{
		sprintf(szFileName, "data/sound/musica_%d.mid", i + 1);
		sprintf(szResourceName, "music%d", i + 1);
		if (false == TheSoundManager::Instance()->load(szFileName, szResourceName, SOUND_MUSIC))
			return false;
	}

	for (int i = 0; i < _SOUND_FILE_COUNT; i++)
	{
		sprintf(szFileName, "data/sound/ping_%d.wav", i + 1);
		sprintf(szResourceName, "sound%d", i + 1);
		if (false == TheSoundManager::Instance()->load(szFileName, szResourceName, SOUND_SFX))
			return false;
	}

	return true;
}

void Game::setCurrentLevel(int currentLevel)
{
    m_currentLevel = currentLevel;
    m_pGameStateMachine->ChangeState(new GameOverState());
    m_bLevelComplete = false;
}

void Game::Render()
{
    SDL_RenderClear(m_pRenderer);
    
    m_pGameStateMachine->Render();
    
    SDL_RenderPresent(m_pRenderer);
}

void Game::Update()
{
	m_pGameStateMachine->Update();
}

void Game::handleEvents()
{
	TheInputHandler::Instance()->Update();
}

void Game::Clean()
{
    cout << "cleaning game\n";
    
    TheInputHandler::Instance()->Clean();
    
    m_pGameStateMachine->Clean();
    
    m_pGameStateMachine = 0;
    delete m_pGameStateMachine;
    
    TheTextureManager::Instance()->clearTextureMap();
    
    SDL_DestroyWindow(m_pWindow);
    SDL_DestroyRenderer(m_pRenderer);
    SDL_Quit();
}


