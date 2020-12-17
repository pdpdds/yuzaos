#pragma once
#include "Goal_Composite.h"
#include "Goal_Types.h"
#include "Player.h"

class Goal_SetBomb : public Goal_Composite<Player>
{
	
public:
		
	Goal_SetBomb(Player* pBot);

	//the usual suspects
	void Activate();
	int Process();
	void Render();
	void Terminate(){}
};

