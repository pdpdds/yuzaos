#pragma once

enum trigger_type {
	TRIGGER_BOMB = 0,
	TRIGGER_BOMBPOWER,
	TRIGGER_SPEEDUP,
	TRIGGER_TNT,
	TRIGGER_COIN10,
	TRIGGER_COIN20,
	TRIGGER_COIN30,
	MAX_TRIGGER_TYPE,
};

enum BombType{
	BOMB_NORMAL = 1,
	BOMB_TNT
};

enum
{
	type_wall,
	type_bot,
	type_unused,
	type_waypoint,
	type_spawn_point,
	type_obstacle,
	type_bomb
};