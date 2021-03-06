#pragma once

enum eOwnerType
{
	NOT_USED = -1,
	PLAYER = 1,
	ENEMY = 2,
	NEUTRAL = 4
};

enum eEntityType
{
	EMPTY = -1,
	UNIT = 1,
	PROP = 2,
	BASE_BUILING = 4,
	RESOURCE_POINT = 8,
	VICTORY_POINT = 16,
	TOTEM = 32,
	_COUNT,
};

enum eUnitType
{
	NOT_A_UNIT = -1,
	GRUNT,
	RANGER,
	TANK,
	NON_ATTACK_TUTORIAL,
};

enum class ePropType
{
	NOT_A_PROP,
	PINE_TREE_A,
	PINE_TREE_B,
	PINE_TREE_C,
	PINE_TREE_D,
	BIRCH_TREE_A,
	HOUSE_SMALL,
	HOUSE_MEDIUM,
	HOUSE_LARGE,
	HOUSE_GROUP,
	RESOURCE_POLE,
	VICTORY_POLE,
	BUSH_A,
	BUSH_B,
	BUSH_C,
	ELECTRIC_POST_A,
	ROCK_LARGE,
	ROCK_MEDIUM,
	ROCK_SMALL,
	ROCKS_GROUP_A,
	ROCKS_GROUP_B,
	BONES_MEDIUM,
	BONES_SMALL,
	BROKENSHIP_A,
	BROKENSHIP_B,
	BROKENSUBMARINE_A,
	CAR_A,
	FENCE_A,
	FENCE_B,
	JUNKPILE_A,
	JUNKPILE_B,
	LOG_A,
	LOG_GROUP,
	OLDPIER_A,
	PIPES_A,
	PIPES_B,
	RUBBLE_A,
	RUBBLE_B,
	RUBBLE_C,
	RUBBLE_D,
	TANK_A,
	TENT_A,
	TENT_B,
	WHEELS_A,
	WHEELS_B

};

enum class eEntityState : int
{
	IDLE,
	WALKING,
	ATTACKING,
	DYING,
	_COUNT,
};

enum class eComponentType
{
	NOT_USED = -1,
	CONTROLLER, //Controller HAS to be above Actor in this list!!!
	ACTOR,
	ANIMATION,
	TRIGGER,
	COLLISION,
	GRAPHICS,
	MOVEMENT,
	HEALTH,
	BUILDING,
	TOTEM,
	ENRAGE,
	GRENADE,
	SELECTION,
	SOUND,
	_COUNT,
};

enum class eTriggerType
{
	RESOURCE,
	VICTORY,
};
#undef ERROR
enum class eHistoryType
{
	ERROR,
	HISTORY,
	HELP,
	GENERATED_COMMAND,
	WARNING
};

enum class eParticleType : int
{
	NONE = -1,
	BLOOD,
	//WEATHER_SNOW,
	//HEALING,
	GRENADE_EXPLOSION,
	_COUNT
};
