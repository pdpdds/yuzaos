//
//  InputHandler.cpp
//  SDL Game Programming Book
//
//  Created by shaun mitchell on 24/01/2013.
//  Copyright (c) 2013 shaun mitchell. All rights reserved.
//

#include "InputHandler.h"
#include "Game.h"
#include <iostream>

InputHandler* InputHandler::s_pInstance = 0;

InputHandler::InputHandler() : m_prevKeystates(0), m_curkeystates(0),
m_bJoysticksInitialised(false),
m_mousePosition(new Vector2D(0,0)),
m_eventtype(-1)
{
    
    memset(m_prevMouseButtonStates, 0, 3);
    memset(m_curMouseButtonStates, 0, 3);
   

    m_prevKeystates = new UINT8[512];
   
	 m_fingerTouch = false;
	 m_fingerX = 0;
	 m_fingerY = 0;
}

InputHandler::~InputHandler()
{
    // delete anything we created dynamically
    delete m_prevKeystates;
    delete m_mousePosition;
    
    // clear our arrays
    m_joystickValues.clear();
    m_joysticks.clear();
    m_buttonStates.clear();

}

void InputHandler::Clean()
{
    // we need to clean up after ourselves and close the joysticks we opened
    if(m_bJoysticksInitialised)
    {
        for(unsigned int i = 0; i < SDL_NumJoysticks(); i++)
        {
            SDL_JoystickClose(m_joysticks[i]);
        }
    }
}

void InputHandler::initialiseJoysticks()
{
    // if we haven't already initialised the joystick subystem, we will do it here
    if(SDL_WasInit(SDL_INIT_JOYSTICK) == 0)
    {
        SDL_InitSubSystem(SDL_INIT_JOYSTICK);
    }
    
    // get the number of joysticks, skip init if there aren't any
    if(SDL_NumJoysticks() > 0)
    {
        for(int i = 0; i < SDL_NumJoysticks(); i++)
        {
            // create a new joystick
            SDL_Joystick* joy = SDL_JoystickOpen(i);
            
            // if the joystick opened correctly we need to populate our arrays
            if(SDL_JoystickOpen(i))
            {
                // push back into the array to be closed later
                m_joysticks.push_back(joy);
                
                // create a pair of values for the axes, a vector for each stick
                m_joystickValues.push_back(std::make_pair(new Vector2D(0,0),new Vector2D(0,0)));
                
                // create an array to hold the button values
                std::vector<bool> tempButtons;
                
                // fill the array with a false value for each button
                for(int j = 0; j <  SDL_JoystickNumButtons(joy); j++)
                {
                    tempButtons.push_back(false);
                }
                // push the button array into the button state array
                m_buttonStates.push_back(tempButtons);
            }
            else
            {
                // if there was an error initialising a joystick we want to know about it
                std::cout << SDL_GetError();
            }
        }
        
        // enable joystick events
        SDL_JoystickEventState(SDL_ENABLE);
        m_bJoysticksInitialised = true;
        
        std::cout << "Initialised " << m_joysticks.size() << " joystick(s)";
    }
    else
    {
        m_bJoysticksInitialised = false;
    }
}

void InputHandler::reset()
{
    memset(m_prevMouseButtonStates, 0, 3);
    memset(m_curMouseButtonStates, 0, 3);
}

bool InputHandler::isKeyDown(SDL_Scancode key) const
{
    if(m_curkeystates != 0)
    {
        if(m_curkeystates[key] == 1)
        {
            return true;
        }
        else
        {
            return false;
        }
    }
    
    return false;
}

bool InputHandler::isKeyUp(SDL_Scancode key) const
{
    if (m_curkeystates != 0)
    {
        if (m_curkeystates[key] == 0 && m_prevKeystates[key] == 1)
        {
            return true;
        }
        else
        {
            return false;
        }
    }

    return false;
}

Uint32 InputHandler::GetEventType() const
{
	return m_eventtype;
}

int InputHandler::getAxisX(int joy, int stick) const
{
    if(m_joystickValues.size() > 0)
    {
        if(stick == 1)
        {
            return m_joystickValues[joy].first->getX();
        }
        else if(stick == 2)
        {
            return m_joystickValues[joy].second->getX();
        }
    }
    return 0;
}

int InputHandler::getAxisY(int joy, int stick) const
{
    if(m_joystickValues.size() > 0)
    {
        if(stick == 1)
        {
            return m_joystickValues[joy].first->getY();
        }
        else if(stick == 2)
        {
            return m_joystickValues[joy].second->getY();
        }
    }
    return 0;
}

bool InputHandler::getButtonState(int joy, int buttonNumber) const
{
    return m_buttonStates[joy][buttonNumber];
}

bool InputHandler::isMouseButtonDown(int buttonNumber) const
{
    return m_curMouseButtonStates[buttonNumber];
}

bool InputHandler::isMouseButtonUp(int buttonNumber) const
{
    return m_curMouseButtonStates[buttonNumber] == 0 && m_prevMouseButtonStates[buttonNumber] == 1;
}

Vector2D* InputHandler::getMousePosition() const
{
    return m_mousePosition;
}

void InputHandler::Update()
{
    if(m_curkeystates)
        memcpy((void*)m_prevKeystates, m_curkeystates, 512);

        memcpy((void*)m_prevMouseButtonStates, m_curMouseButtonStates, 3);
    
    SDL_Event event;
    while(SDL_PollEvent(&event))
    {
        switch (event.type)
        {
            case SDL_QUIT:
                TheGame::Instance()->quit();
                break;
                
            case SDL_JOYAXISMOTION:
                onJoystickAxisMove(event);
                break;
                
            case SDL_JOYBUTTONDOWN:
                onJoystickButtonDown(event);
                break;
                
            case SDL_JOYBUTTONUP:
                onJoystickButtonUp(event);
                break;
                
            case SDL_MOUSEMOTION:
                onMouseMove(event);
                break;
                
            case SDL_MOUSEBUTTONDOWN:
                onMouseButtonDown(event);
                break;
                
            case SDL_MOUSEBUTTONUP:
                onMouseButtonUp(event);
                break;

			case SDL_FINGERDOWN:
				onTouchDown(event);
				break;

			case SDL_FINGERUP:
				onTouchUp(event);
				break;

            case SDL_KEYDOWN:
				onKeyDown();
                break;
                
            case SDL_KEYUP:  
				onKeyUp();
                break;
                
            default:
                break;
        }
    }
}

void InputHandler::onKeyDown()
{
    
    m_curkeystates = SDL_GetKeyboardState(0);
}

void InputHandler::onKeyUp()
{
    
    m_curkeystates = SDL_GetKeyboardState(0);
}

void InputHandler::onMouseMove(SDL_Event &event)
{
    m_mousePosition->setX(event.motion.x);
    m_mousePosition->setY(event.motion.y);
}

void InputHandler::onMouseButtonDown(SDL_Event &event)
{
    if(event.button.button == SDL_BUTTON_LEFT)
    {
        m_curMouseButtonStates[LEFT] = true;
    }
    
    if(event.button.button == SDL_BUTTON_MIDDLE)
    {
        m_curMouseButtonStates[MIDDLE] = true;
    }
    
    if(event.button.button == SDL_BUTTON_RIGHT)
    {
        m_curMouseButtonStates[RIGHT] = true;
    }
}

void InputHandler::onMouseButtonUp(SDL_Event &event)
{
    if(event.button.button == SDL_BUTTON_LEFT)
    {
        m_curMouseButtonStates[LEFT] = false;
    }
    
    if(event.button.button == SDL_BUTTON_MIDDLE)
    {
        m_curMouseButtonStates[MIDDLE] = false;
    }
    
    if(event.button.button == SDL_BUTTON_RIGHT)
    {
        m_curMouseButtonStates[RIGHT] = false;
    }
}


void InputHandler::onTouchDown(SDL_Event &event)
{
	//m_fingerTouch = true;
	//m_fingerX = event.tfinger.x;
	//m_fingerY = event.tfinger.y;
	
}

void InputHandler::onTouchUp(SDL_Event &event)
{
	m_fingerX = event.tfinger.x;
	m_fingerY = event.tfinger.y;
	m_fingerTouch = true;
}

void InputHandler::onJoystickAxisMove(SDL_Event &event)
{
    int whichOne = event.jaxis.which;
    
    // left stick move left or right
    if(event.jaxis.axis == 0)
    {
        if (event.jaxis.value > m_joystickDeadZone)
        {
            m_joystickValues[whichOne].first->setX(1);
        }
        else if(event.jaxis.value < -m_joystickDeadZone)
        {
            m_joystickValues[whichOne].first->setX(-1);
        }
        else
        {
            m_joystickValues[whichOne].first->setX(0);
        }
    }
    
    // left stick move up or down
    if(event.jaxis.axis == 1)
    {
        if (event.jaxis.value > m_joystickDeadZone)
        {
            m_joystickValues[whichOne].first->setY(1);
        }
        else if(event.jaxis.value < -m_joystickDeadZone)
        {
            m_joystickValues[whichOne].first->setY(-1);
        }
        else
        {
            m_joystickValues[whichOne].first->setY(0);
        }
    }
    
    // right stick move left or right
    if(event.jaxis.axis == 3)
    {
        if (event.jaxis.value > m_joystickDeadZone)
        {
            m_joystickValues[whichOne].second->setX(1);
        }
        else if(event.jaxis.value < -m_joystickDeadZone)
        {
            m_joystickValues[whichOne].second->setX(-1);
        }
        else
        {
            m_joystickValues[whichOne].second->setX(0);
        }
    }
    
    // right stick move up or down
    if(event.jaxis.axis == 4)
    {
        if (event.jaxis.value > m_joystickDeadZone)
        {
            m_joystickValues[whichOne].second->setY(1);
        }
        else if(event.jaxis.value < -m_joystickDeadZone)
        {
            m_joystickValues[whichOne].second->setY(-1);
        }
        else
        {
            m_joystickValues[whichOne].second->setY(0);
        }
    }
}

void InputHandler::onJoystickButtonDown(SDL_Event &event)
{
    int whichOne = event.jaxis.which;
    
    m_buttonStates[whichOne][event.jbutton.button] = true;
}

void InputHandler::onJoystickButtonUp(SDL_Event &event)
{
    int whichOne = event.jaxis.which;
    
    m_buttonStates[whichOne][event.jbutton.button] = false;
}