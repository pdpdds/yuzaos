#ifndef __GETBOMB_BGOAL_EVALUATOR__
#define __GETBOMB_BGOAL_EVALUATOR__
#pragma warning (disable:4786)

#include "Goal_Evaluator.h"
#include "Player.h"

class GetBombGoal_Evaluator : public Goal_Evaluator
{
public:

	GetBombGoal_Evaluator(double bias) :Goal_Evaluator(bias){}
  
  double CalculateDesirability(Player* pBot);

  void  SetGoal(Player* pEnt);

  void RenderInfo(Vector2D Position, Player* pBot);
};

#endif
    
