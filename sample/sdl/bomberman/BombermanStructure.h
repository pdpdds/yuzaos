#pragma once
#include "GameConstant.h"

typedef struct tag_EdgeInfo
{
	int from;
	int to;
}EdgeInfo;

typedef struct tag_NodeInfo
{
	float x;
	float y;
	int id;
}NodeInfo;

typedef struct tag_PlayerStatInfo
{
	int maxBombCount;
	int currentBombCount;
	int bombPower;
	int bombType;

	tag_PlayerStatInfo()
	{
		maxBombCount = 1; //플레이어가 설치할 수 있는 폭탄의 최대 수
		currentBombCount = 0; //플레이어가 필드에 설치한 폭탄 수
		bombPower = 1; //폭탄의 파워. 
		bombType = 0;//폭탄의 타입 0 : 일반, 1 TNT
	}

}PlayerStatInfo;