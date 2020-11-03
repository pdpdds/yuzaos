#ifndef GOAL_FOLLOWPATH_H
#define GOAL_FOLLOWPATH_H
#pragma warning (disable:4786)
//-----------------------------------------------------------------------------
//
//  Name:   Goal_FollowPath.h
//
//  Author: Mat Buckland (www.ai-junkie.com)
//
//  Desc:
//-----------------------------------------------------------------------------
#include "Goal_Composite.h"
#include "Goal_Types.h"
#include "Player.h"
#include "PathPlanner.h"
#include "PathEdge.h"



class Goal_FollowPath : public Goal_Composite<Player>
{
private:

  //a local copy of the path returned by the path planner
  std::list<PathEdge>  m_Path;

public:

	Goal_FollowPath(Player* pBot, std::list<PathEdge> path);

  //the usual suspects
  void Activate();
  int Process();
  void Render();
  void Terminate(){}
};

#endif

