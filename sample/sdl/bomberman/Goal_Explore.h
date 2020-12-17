#ifndef GOAL_EXPLORE_H
#define GOAL_EXPLORE_H
#pragma warning (disable:4786)

#include "Goal_Composite.h"
#include "Goal_Types.h"


class Player;


class Goal_Explore : public Goal_Composite<Player>
{
private:
  
  Vector2D  m_CurrentDestination;

  //set to true when the destination for the exploration has been established
  bool      m_bDestinationIsSet;

public:

	Goal_Explore(Player* pOwner) :Goal_Composite<Player>(pOwner,
                                                            goal_explore),
                                  m_bDestinationIsSet(false)
  {}


  void Activate();

  int Process();

  void Terminate(){}

  bool HandleMessage(const Telegram& msg);
};





#endif
