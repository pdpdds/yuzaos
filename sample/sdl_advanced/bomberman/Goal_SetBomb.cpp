#include "Goal_SetBomb.h"
#include "Player.h"

Goal_SetBomb::Goal_SetBomb(Player* pBot)
: Goal_Composite<Player>(pBot, goal_set_bomb)
{
}

void Goal_SetBomb::Activate()
{
	m_iStatus = active;

	PlayerStatInfo& stat = m_pOwner->GetPlayerStat();
	if (m_pOwner->InstallBomb() == true)
	{
		m_iStatus = completed;
	}
	else
		m_iStatus = failed;	
}

int Goal_SetBomb::Process()
{
	//if status is inactive, call Activate()
	ActivateIfInactive();

	return m_iStatus;
}

void Goal_SetBomb::Render()
{
	
}
