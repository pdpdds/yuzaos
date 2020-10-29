#include "math.h"

#include "SDL.h"
#include "SDL_ttf.h"
#include "SDL_mixer.h"

#include "sound.h"

#include "CTile.h"
#include "CObject.h"
#include "CGame.h"

extern int ENEMY_SPEED;
extern int ENEMY_HSPEED;
extern int PLAYING_WINDOW;


CEnemyNormalCarObject::CEnemyNormalCarObject(int nx,int ny,CTile *t,int start_delay,CGame *g) : CEnemyCarObject(nx,ny,t,start_delay,g)
{
} /* CEnemyNormalCarObject::CEnemyNormalCarObject */ 


CEnemyNormalCarObject::~CEnemyNormalCarObject(void)
{
} /* CEnemyNormalCarObject::~CEnemyNormalCarObject */ 


