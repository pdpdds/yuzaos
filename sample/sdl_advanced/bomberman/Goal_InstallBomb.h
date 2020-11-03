#pragma once

#include "Goal_Composite.h"
#include "Goal_Types.h"

class Player;

class Goal_InstallBomb : public Goal_Composite<Player>
{
private:

	Vector2D  m_CurrentDestination;

	//set to true when the destination for the exploration has been established
	bool      m_bDestinationIsSet;

public:
	Goal_InstallBomb(Player* pOwner) :Goal_Composite<Player>(pOwner,
		goal_install_bomb),
		m_bDestinationIsSet(false)
	{}
	virtual ~Goal_InstallBomb(){}

	void Activate();

	int Process();

	void Terminate(){}

	bool HandleMessage(const Telegram& msg);
};

