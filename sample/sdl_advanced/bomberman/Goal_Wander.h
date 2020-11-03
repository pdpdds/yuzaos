#ifndef GOAL_WANDER_H
#define GOAL_WANDER_H
#pragma warning (disable:4786)
//-----------------------------------------------------------------------------
//
//  Name:   Goal_Wander.h
//
//  Author: Mat Buckland (www.ai-junkie.com)
//
//  Desc:   Causes a bot to wander until terminated
//-----------------------------------------------------------------------------
#include "Goal.h"
#include "Goal_Types.h"
#include "Player.h"


class Goal_Wander : public Goal<Player>
{
private:

public:

	Goal_Wander(Player* pBot) :Goal<Player>(pBot, goal_wander)
  {}

  void Activate();

  int  Process();

  void Terminate();
};





#endif
