#ifndef TRIGGERSYSTEM_H
#define TRIGGERSYSTEM_H

template <typename trigger_type>
class TriggerSystem
{
public:

	typedef std::list<trigger_type*> TriggerList;

private:

	TriggerList   m_Triggers;

	void UpdateTriggers()
	{
		typename TriggerList::iterator curTrg = m_Triggers.begin();
		while (curTrg != m_Triggers.end())
		{
			//remove trigger if dead
			if ((*curTrg)->isToBeRemoved())
			{
				delete *curTrg;

				curTrg = m_Triggers.erase(curTrg);
			}
			else
			{				
				(*curTrg)->Update();

				++curTrg;
			}
		}
	}

	template <typename ContainerOfEntities>
	void TryTriggers(ContainerOfEntities& entities)
	{
		//test each entity against the triggers
		typename ContainerOfEntities::iterator curEnt = entities.begin();
		for (curEnt; curEnt != entities.end(); ++curEnt)
		{
			//an entity must be ready for its next trigger update and it must be 
			//alive before it is tested against each trigger.
			//if ((*curEnt)->isReadyForTriggerUpdate() && (*curEnt)->isAlive())
			/*if (!(*curEnt)->dead() && !(*curEnt)->dying())
			{
				TriggerList::const_iterator curTrg;
				for (curTrg = m_Triggers.begin(); curTrg != m_Triggers.end(); ++curTrg)
				{
					(*curTrg)->Try(*curEnt);
				}
			}*/
		}
	}


public:

	~TriggerSystem()
	{
		Clear();
	}

	//this deletes any current triggers and empties the trigger list
	void Clear()
	{
		typename TriggerList::iterator curTrg;
		for (curTrg = m_Triggers.begin(); curTrg != m_Triggers.end(); ++curTrg)
		{
			delete *curTrg;
		}

		m_Triggers.clear();
	}

	//This method should be called each update-step of the game. It will first
	//update the internal state odf the triggers and then try each entity against
	//each active trigger to test if any should be triggered.
	template <typename ContainerOfEntities>
	void Update(ContainerOfEntities& entities)
	{
		UpdateTriggers();
		TryTriggers(entities);
	}

	//this is used to register triggers with the TriggerSystem (the TriggerSystem
	//will take care of tidying up memory used by a trigger)
	void Register(trigger_type* trigger)
	{
		m_Triggers.push_back(trigger);
	}

	//some triggers are required to be rendered (like giver-triggers for example)
	void Render()
	{
		typename TriggerList::iterator curTrg;
		for (curTrg = m_Triggers.begin(); curTrg != m_Triggers.end(); ++curTrg)
		{
			(*curTrg)->Draw();
		}
	}

	const TriggerList& GetTriggers()const{ return m_Triggers; }

};


#endif