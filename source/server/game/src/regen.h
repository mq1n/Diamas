#pragma once
#include "dungeon.h"

enum ERegenModes
{
	MODE_TYPE			= 0,
	MODE_SX				= 1,
	MODE_SY				= 2,
	MODE_EX				= 3,
	MODE_EY				= 4,
	MODE_Z_SECTION		= 5,
	MODE_DIRECTION		= 6,
	MODE_REGEN_TIME		= 7,
	MODE_REGEN_PERCENT	= 8,
	MODE_MAX_COUNT		= 9,
	MODE_VNUM			= 10,
};

enum ERegenTypes
{
	REGEN_TYPE_MOB			= 0,
	REGEN_TYPE_GROUP		= 1,
	REGEN_TYPE_EXCEPTION	= 2,
	REGEN_TYPE_GROUP_GROUP	= 3,
	REGEN_TYPE_ANYWHERE		= 4,
	REGEN_TYPE_MAX_NUM		= 5
};

typedef struct regen
{
	LPREGEN	prev, next;
	int32_t	lMapIndex;
	int32_t		type;
	int32_t		sx, sy, ex, ey;
	uint8_t	z_section;

	uint8_t	direction;

	uint32_t	time;

	int32_t		max_count;
	int32_t		count;
	int32_t 	vnum;

	bool	is_aggressive;

	LPEVENT	event;

	size_t id; // to help dungeon regen identification

	regen() :
		prev(nullptr), next(nullptr),
		lMapIndex(0),
		type(0),
		sx(0), sy(0), ex(0), ey(0),
		z_section(0),
		direction(0),
		time(0),
		max_count(0),
		count(0),
		vnum(0),
		is_aggressive(0),
		event(nullptr),
		id(0)
	{}
} REGEN;

EVENTINFO(regen_event_info)
{
	LPREGEN 	regen;

	regen_event_info() 
	: regen( 0 )
	{
	}
};

typedef regen_event_info REGEN_EVENT_INFO;

typedef struct regen_exception
{
	LPREGEN_EXCEPTION prev, next;

	int32_t		sx, sy, ex, ey;
	uint8_t	z_section;
} REGEN_EXCEPTION;

class CDungeon;

EVENTINFO(dungeon_regen_event_info)
{
	LPREGEN 	regen;
	CDungeon::IdType dungeon_id;

	dungeon_regen_event_info() 
	: regen( 0 )
	, dungeon_id( 0 )
	{
	}
};

extern bool	regen_load(const char *filename, int32_t lMapIndex, int32_t base_x, int32_t base_y); 
extern bool	regen_do(const char* filename, int32_t lMapIndex, int32_t base_x, int32_t base_y, LPDUNGEON pDungeon, bool bOnce = true );
extern bool	regen_load_in_file(const char* filename, int32_t lMapIndex, int32_t base_x, int32_t base_y );
extern void	regen_free();

extern bool	is_regen_exception(int32_t x, int32_t y);
extern void	regen_reset(int32_t x, int32_t y);
