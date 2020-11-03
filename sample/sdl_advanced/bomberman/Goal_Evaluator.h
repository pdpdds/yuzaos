#ifndef GOAL_EVALUATOR_H
#define GOAL_EVALUATOR_H

class Player;
class Vector2D;

class Goal_Evaluator
{
protected:

  //when the desirability score for a goal has been evaluated it is multiplied 
  //by this value. It can be used to create bots with preferences based upon
  //their personality
  double       m_dCharacterBias;

public:

  Goal_Evaluator(double CharacterBias):m_dCharacterBias(CharacterBias){}
  
  virtual ~Goal_Evaluator(){}
  
  //returns a score between 0 and 1 representing the desirability of the
  //strategy the concrete subclass represents
  virtual double CalculateDesirability(Player* pBot) = 0;
  
  //adds the appropriate goal to the given bot's brain
  virtual void  SetGoal(Player* pBot) = 0;

  //used to provide debugging/tweaking support
  virtual void  RenderInfo(Vector2D Position, Player* pBot) = 0;
};

#endif

