#pragma once
#include "Goal_Evaluator.h"
#include "Player.h"

class InstallBombGoal_Evaluator : public Goal_Evaluator
{
public:

	InstallBombGoal_Evaluator(double bias) :Goal_Evaluator(bias){}

	double CalculateDesirability(Player* pBot);

	void  SetGoal(Player* pEnt);

	void RenderInfo(Vector2D Position, Player* pBot);
};

