#pragma once
#include <SDL.h>

class Clock{
    private:
        double current_time;
        double last_time;
    public:
        
        double dt;
        double dt_sec;

        Clock();
        void Tick();
};

