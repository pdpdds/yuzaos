#ifndef __PLAYER_H__
#define __PLAYER_H__

#include <iostream>
#include <vector>
#include "BomberManStructure.h"
#include "PlatformerObject.h"
#include "GameObjectFactory.h"
#include "PathPlanner.h"
#include "SteeringBehaviors.h"

class Goal_Think;
class Level;
class Regulator;

class Player : public PlatformerObject
{
public:
    
    Player(Level* pLevel);
	virtual ~Player();
    
	virtual void Load(LoaderParams& params);
    
    virtual void Draw();
	virtual void Update();
    virtual void Clean();
    
    virtual void Collision();
    
    virtual std::string type() { return "Player"; }

//애니메이션 처리를 위한 임시 저장 구조체
//Deprecated
	LoaderParams* m_liveParam;
	LoaderParams* m_dieParam;

    // bring the player back if there are lives left
    void ressurect();

	bool CheckCollision(std::vector<GameObject*>& gameObject);
    
// 조이스틱, 마우스, 키보드, 터치 이벤트를 다룬다.
    void HandleInput();
    
    bool handleMovement();

//플레이어의 애니메이션 처리. SDL, Cocos2d-x 사용에 따라 해당 코드는 변경되어야 한다.
	void HandleAnimation();

//AI 처리를 위한 메소드
	PathPlanner* const GetPathPlanner(){ return m_pPathPlanner; }
	Steering* const GetSteering(){ return m_pSteering; }
	Goal_Think* const GetBrain(){ return m_pBrain; }
	bool isPossessed()const{ return m_bPossessed; }
	void SetPossessed(bool bPossessed) { m_bPossessed = bPossessed; }

//유저의 제어에서 벗어나 AI가 객체를 조종하도록 한다.
//서버 사이드 로직 개발시 중요한 역할을 할 것이다.
	void Exorcise();

//플레이어가 해당 위치에 있는지를 체크.	상황에 따라 오차 범위를 둘 필요가 있는데
//현재는 오차를 0로 설정했다.
	bool IsAtPosition(Vector2D pos);

//현재 설정된 속도로 이동할 시 목적지에 도착하는 시간을 구한다. 
//FrameRate 변수를 60으로 고정했는데 추후 수정해야 한다.
	double CalculateTimeToReachPosition(Vector2D pos);

//통보받은 메시지를 처리한다.
	bool HandleMessage(const Telegram& msg);

//게임 로직과 관련된 메소드
	bool IsMyBomb(GameObject* pGameObject);
	bool InstallBomb();

//폭탄 타입을 변경한다.
	void SetBombType(int bombType){ m_playerStat.bombType = bombType; }
	void IncreaseBombCount(){ m_playerStat.maxBombCount++;}
	void IncreaseBombPower(){ m_playerStat.bombPower++; }

	PlayerStatInfo& GetPlayerStat(){ return m_playerStat; }

	bool IsOverlappedWithBomb(){return m_bObjectOverlapped;}
	void SetOverlappedWithBomb(bool bOverlapped){ m_bObjectOverlapped = bOverlapped; }

private:
	int m_frameAnchorIndex;

//봄버맨 게임 로직과 직접적으로 관계된 변수들
	std::vector<int> m_installedBomb;
//폭탄 설치후 플레이어와 폭탄이 겹치는데 이 경우 플레이어가 움직일 수 있어야 한다.
//개선 소지가 있으며 임시로 일단 처리
	bool m_bObjectOverlapped;	
	
//플레이어 스탯
	PlayerStatInfo m_playerStat;

//플레이어가 잠시동안 무적상태에 있게 한다. 리스폰했을시에 필요
    int m_bInvulnerable;
    int m_invulnerableTime;
    int m_invulnerableCounter;

//플레이어 AI
	//이 플래그가 세팅이 되면 유저가 플레이어를 조종한다. 그렇지 않으면 AI로 플레이어를 조종한다.
	bool m_bPossessed;
	//이동 경로를 계획하기 위한 경로 계획자. PathManager에 등록하여 매번 업데이트 하지 않고 PathManager가 업데이트를 수행하도록 해서 CPU 부하를 줄인다.
	PathPlanner* m_pPathPlanner;
	//AI를 조종하기 위한 객체. 상황에 따른 알고리즘을 사용한다(배회, 이동, 숨기, 회피 등등)
	Steering* m_pSteering;
	//AI의 목적을 중재하여 상황에 따른 목적을 설정하고 해당 목적을 수행한다.
	Goal_Think* m_pBrain;

	//목표 판단
	Regulator*  m_pGoalArbitrationRegulator;
};

#endif
