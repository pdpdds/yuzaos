#include <SDL.h>
#include "PlatformerObject.h"
#include "GameObjectFactory.h"

#define DEFALUT_EXPLOSON_INTERVAL 5000

class Wall : public PlatformerObject
{
public:
	Wall();
	virtual ~Wall();

	virtual void Load(LoaderParams& params);

	virtual void Draw();
	virtual void Update();
	virtual void Clean();

	virtual void Collision();

	virtual std::string type() { return "Wall"; }

private:

};

// for the factory
class WallCreator : public BaseCreator
{
	GameObject* createGameObject() const
	{
		return new Wall();
	}
};