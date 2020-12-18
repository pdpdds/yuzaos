#include "clock.h"

Clock::Clock() {
    last_time = 0.0;
    current_time = SDL_GetPerformanceCounter();
    dt = 0.0;
    dt_sec = 0.0;
}

void Clock::Tick() {
    last_time = current_time;
    current_time = SDL_GetPerformanceCounter();
    dt = ((current_time - last_time ) * 1000) / SDL_GetPerformanceFrequency();
    dt_sec = dt * .001;
}
