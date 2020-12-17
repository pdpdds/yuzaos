#pragma once
#include "PlatformerObject.h"
#include "GameObjectFactory.h"

#define MAX_EXPLOSION_ROW 5

typedef struct tag_FlameInfo
{
	int frameindex;
	int offsetX;
	int offsetY;

}FlameInfo;

class Explosion : public PlatformerObject
{
public:
	Explosion();
	virtual ~Explosion();

	void MakeFlame(int bombPower, int bombType);
	void MakeTNTFlame();

	virtual void Load(LoaderParams& params);

	virtual void Draw();
	virtual void Update();
	virtual void Clean();

	virtual void Collision();

	virtual std::string type() { return "Explosion"; }

	virtual bool CheckCollision(GameObject* gameObject) override;

	void DrawCenter();
	void DrawFlame(int frameIndex, int xOffset, int yOffset);

	int GetExplosionLength(){ return m_explosionLenth; }

	bool InRange(GameObject* pObject);

	std::vector<FlameInfo> m_flameList;

private:	
	int m_lifeTime;
	int m_generatedTime;
	int m_explosionLenth;

	
};

// for the factory
class ExplosionCreator : public BaseCreator
{
	GameObject* createGameObject() const
	{
		return new Explosion();
	}
};

