#pragma once
#include <stdint.h>
#include "../../common/defines.h"

class CItemVnumHelper
{
public:
	static const bool	IsPhoenix(uint32_t vnum)				{ return 53001 == vnum; }
	static const bool	IsRamadanMoonRing(uint32_t vnum)		{ return 71135 == vnum; }
	static const bool	IsHalloweenCandy(uint32_t vnum)		{ return 71136 == vnum; }
	static const bool	IsHappinessRing(uint32_t vnum)		{ return 71143 == vnum; }

	static const bool	IsLovePendant(uint32_t vnum)		{ return 71145 == vnum; }

#ifdef ENABLE_ACCE_SYSTEM
	static	bool	IsAcceItem(uint32_t vnum)		{ return (vnum >= 85001 && vnum <= 85008) || (vnum >= 85011 && vnum <= 85018); }
#endif

};

class CMobVnumHelper
{
public:
	static	bool	IsPhoenix(uint32_t vnum)				{ return 34001 == vnum; }
	static	bool	IsIcePhoenix(uint32_t vnum)				{ return 34003 == vnum; }
	static	bool	IsPetUsingPetSystem(uint32_t vnum)	{ return (IsPhoenix(vnum) || IsReindeerYoung(vnum)) || IsIcePhoenix(vnum); }
	static	bool	IsReindeerYoung(uint32_t vnum)	{ return 34002 == vnum; }
	static	bool	IsRamadanBlackHorse(uint32_t vnum)		{ return 20119 == vnum || 20219 == vnum || 22022 == vnum; }
};

class CVnumHelper
{
};
