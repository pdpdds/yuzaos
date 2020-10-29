#ifndef __RF_OBJECT_CLASS
#define __RF_OBJECT_CLASS

#define CONSTITUTION_NONE		1
#define CONSTITUTION_SOLID		2
#define CONSTITUTION_CAR		4
#define CONSTITUTION_PLAYER		8
#define CONSTITUTION_FUEL		16
#define CONSTITUTION_WATER		32
#define CONSTITUTION_OIL		64

#include "List.h"

class CObject {
	friend class CGame;

public:
	CObject();	
	CObject(int x,int y,CTile *t,int constitution,class CGame *game);	/* Simple constructor */ 
	~CObject(void);

	virtual bool cycle(unsigned char *keyboard,unsigned char *old_keyboard);
	virtual void draw(int sx,int sy,SDL_Surface *screen);
	bool collision(int offsx,int offst,CObject *o);

	long get_x(void) {return x;};
	long get_y(void) {return y;};
	int get_dx(void);
	int get_dy(void);

	void set_state(int s) {state=s;};
	int get_state(void) {return state;};

	bool constitution_test(int cons);

protected:
	int state;	/*	0 - Stopped
					1 - ...	*/ 
	int constitution;

	CGame *game;
	CTile **tiles;
	int ntiles,tile;

	long x,y;
	long draw_x,draw_y;
};

class CSemaphoreObject : public CObject {
public:
	CSemaphoreObject(int x,int y,CTile *t1,CTile *t2,CTile *t3,CTile *t4,CTile *t5,class CGame *game);
	~CSemaphoreObject(void);

	virtual bool cycle(unsigned char *keyboard,unsigned char *old_keyboard);

protected:
	int timmer;
};


class CCarObject : public CObject {
public:
	CCarObject(void);
	CCarObject(int x,int y,CTile *t,class CGame *game);
	~CCarObject(void);

	virtual bool cycle(unsigned char *keyboard,unsigned char *old_keyboard);

	int get_y_speed(void) {return y_speed;};
	void car_collision(CCarObject *car);

	long int compute_next_x(void);
	long int compute_next_y(void);

protected:
	CCarObject *last_collision;

	long int y_precision,x_precision;
	int y_speed,x_speed;
};


class CFuelObject : public CObject {
public:
	CFuelObject(int x,int y,CTile *t,class CGame *game);
	~CFuelObject(void);

	virtual bool cycle(unsigned char *keyboard,unsigned char *old_keyboard);

protected:
	int y_precision,x_precision;
	int y_speed,x_speed;
};


class CPlayerCarObject : public CCarObject {
public:
	CPlayerCarObject(int x,int y,List<CTile> *l,int first_tile,int last_tile,
					 int lk,int rk,int fk,int sc,int init_delay,class CGame *game);
	~CPlayerCarObject(void);

	virtual bool cycle(unsigned char *keyboard,unsigned char *old_keyboard);
	virtual void draw(int sx,int sy,SDL_Surface *screen);


	int get_fuel(void) {return fuel;};
	int get_score(void) {return score;};

	void reach_goal(void);

protected:
	int car_tile(int angle);	/* in degrees */ 
	void tyre_coordinates(int angle,int *x1,int *y1,int *x2,int *y2);

/*	int state;		0 - Starting
					1 - Driving
					2 - Bouncing right
					3 - Bouncing left
					4 - Exploding
					5 - Sliding	right 
					6 - Slidding left */ 

	int sound_timmer;
	int state_timmer;
	int rotating_angle;
	int blinking_time;
	int old_angle;

	int enginesound_channel,skidsound_channel;
	Mix_Chunk S_carengine_working;
	Mix_Chunk S_carskid_working;

	int score;
	int bonus,next_bonus,bonus_timmer;
	int fuel;
	int left_key,right_key,fire_key;

	bool goal_reached;
};



class CEnemyCarObject : public CCarObject {
	friend class CGame;

public:
	CEnemyCarObject(int x,int y,CTile *t,int start_delay,class CGame *game);
	~CEnemyCarObject(void);

	virtual bool cycle(unsigned char *keyboard,unsigned char *old_keyboard);

protected:
	int slide_direction;
	int slide_speed;
	int slide_timmer;

	int state_timmer;
	bool following_right_border;
	int distance_to_border;

};

/* Green cars */ 
class CEnemyNormalCarObject : public CEnemyCarObject {
	friend class CGame;

public:
	CEnemyNormalCarObject(int x,int y,CTile *t,int start_delay,class CGame *game);
	~CEnemyNormalCarObject(void);

protected:

};

/* Blue cars */ 
class CEnemyRacerCarObject : public CEnemyCarObject {
	friend class CGame;

public:
	CEnemyRacerCarObject(int x,int y,CTile *t,int start_delay,class CGame *game);
	~CEnemyRacerCarObject(void);

	virtual bool cycle(unsigned char *keyboard,unsigned char *old_keyboard);

protected:
	bool advanced;

};

/* Fast Blue cars */ 
class CEnemyFastCarObject : public CEnemyCarObject {
	friend class CGame;

public:
	CEnemyFastCarObject(int x,int y,CTile *t,int start_delay,class CGame *game);
	~CEnemyFastCarObject(void);

	virtual bool cycle(unsigned char *keyboard,unsigned char *old_keyboard);

protected:

};

/* Pink cars */ 
class CEnemySlidderCarObject : public CEnemyCarObject {
	friend class CGame;

public:
	CEnemySlidderCarObject(int x,int y,CTile *t,int start_delay,class CGame *game);
	~CEnemySlidderCarObject(void);

	virtual bool cycle(unsigned char *keyboard,unsigned char *old_keyboard);

protected:

};

/* Trucks */ 
class CEnemyTruckObject : public CCarObject {
	friend class CGame;

public:
	CEnemyTruckObject(int x,int y,CTile *t,int start_delay,class CGame *game);
	~CEnemyTruckObject(void);

	virtual bool cycle(unsigned char *keyboard,unsigned char *old_keyboard);

protected:
	int state_timmer;
	bool following_right_border;
	int distance_to_border;

	bool advanced;
};


class CExplosionObject : public CObject {
public:
	CExplosionObject(int x,int y,List<CTile> *l,int first_tile,int last_tile,class CGame *game);
	~CExplosionObject(void);

	virtual bool cycle(unsigned char *keyboard,unsigned char *old_keyboard);

protected:
	int timmer;
};


#endif

