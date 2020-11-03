#include <SDL.h>
#include "PlatformerObject.h"
#include "GameObjectFactory.h"

#define DEFALUT_EXPLOSON_INTERVAL 5000

class Bomb : public PlatformerObject
{
public:
	Bomb(int ownerId, int explosionInterval = DEFALUT_EXPLOSON_INTERVAL);
	virtual ~Bomb();

	virtual void Load(LoaderParams& params);

	virtual void Draw();
	virtual void Update();
	virtual void Clean();

	virtual void Collision();
	virtual void ProcessDeadAction() override;

	virtual std::string type() { return "Bomb"; }
	void SetBombPower(int bombPower){ m_bombPower = bombPower; }
	void SetBombType(int bombType){ m_bombType = bombType; }

public:
	int m_explosionInterval;
	int m_lifeTime;
	int m_generatedTime;
	int m_bombPower;
	bool m_bChainedExplosion;
	int m_bombType;
	int m_ownerId;
};