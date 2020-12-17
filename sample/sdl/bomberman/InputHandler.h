//
//  InputHandler.h
//  SDL Game Programming Book
//
//  Created by shaun mitchell on 24/01/2013.
//  Copyright (c) 2013 shaun mitchell. All rights reserved.
//

#ifndef __SDL_Game_Programming_Book__InputHandler__
#define __SDL_Game_Programming_Book__InputHandler__

#include <iostream>
#include <vector>

#include "SDL.h"
#include "Vector2D.h"

enum mouse_buttons
{
    LEFT = 0,
    MIDDLE = 1,
    RIGHT = 2
};

class InputHandler
{
public:
    
    static InputHandler* Instance()
    {
        if(s_pInstance == 0)
        {
            s_pInstance = new InputHandler();
        }
        
        return s_pInstance;
    }
    
    // init joysticks
    void initialiseJoysticks();
    bool joysticksInitialised() const { return m_bJoysticksInitialised; }
    
    void reset();
    
    // update and clean the input handler
	void Update();
    void Clean();
    
    // keyboard events
    bool isKeyDown(SDL_Scancode key) const;
    bool isKeyUp(SDL_Scancode key) const;
	Uint32 GetEventType() const;
    
    // joystick events
    int getAxisX(int joy, int stick) const;
    int getAxisY(int joy, int stick) const;
    bool getButtonState(int joy, int buttonNumber) const;
    
    // mouse events
    bool isMouseButtonDown(int buttonNumber) const;
    bool isMouseButtonUp(int buttonNumber) const;
    Vector2D* getMousePosition() const;

	bool m_fingerTouch;
	float m_fingerX;
	float m_fingerY;
    
private:
    
    InputHandler();
    ~InputHandler();
    
    InputHandler(const InputHandler&);
	InputHandler& operator=(const InputHandler&);
    
    // private functions to handle different event types
    
    // handle keyboard events
    void onKeyDown();
    void onKeyUp();
    
    // handle mouse events
    void onMouseMove(SDL_Event& event);
    void onMouseButtonDown(SDL_Event& event);
    void onMouseButtonUp(SDL_Event& event);

	void onTouchDown(SDL_Event &event);
	void onTouchUp(SDL_Event &event);
    
    // handle joysticks events
    void onJoystickAxisMove(SDL_Event& event);
    void onJoystickButtonDown(SDL_Event& event);
    void onJoystickButtonUp(SDL_Event& event);

    // member variables
    
    // keyboard specific
    const Uint8* m_prevKeystates;
    const Uint8* m_curkeystates;
	Uint32 m_eventtype;
    
    // joystick specific
    std::vector<std::pair<Vector2D*, Vector2D*> > m_joystickValues;
    std::vector<SDL_Joystick*> m_joysticks;
    std::vector<std::vector<bool> > m_buttonStates;
    bool m_bJoysticksInitialised;
    static const int m_joystickDeadZone = 10000;
    
    // mouse specific
    Uint8 m_prevMouseButtonStates[3];
    Uint8 m_curMouseButtonStates[3];
    Vector2D* m_mousePosition;
    
    // singleton
    static InputHandler* s_pInstance;
};
typedef InputHandler TheInputHandler;


#endif /* defined(__SDL_Game_Programming_Book__InputHandler__) */
