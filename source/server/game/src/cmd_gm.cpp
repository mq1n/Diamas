#include "stdafx.h"
#include "cmd.h"
#include "char.h"
#include "utils.h"
#include "quest_manager.h"
#include "config.h"
#include "desc_client.h"
#include "desc_manager.h"
#include "char_manager.h"
#include "item_manager.h"
#include "sectree_manager.h"
#include "mob_manager.h"
#include "regen.h"
#include "guild.h"
#include "guild_manager.h"
#include "p2p.h"
#include "buffer_manager.h"
#include "fishing.h"
#include "mining.h"
#include "vector.h"
#include "affect.h"
#include "db.h"
#include "priv_manager.h"
#include "building.h"
#include "battle.h"
#include "arena.h"
#include "start_position.h"
#include "party.h"
#include "xmas_event.h"
#include "log.h"
#include "threeway_war.h"
#include "unique_item.h"
#include "dragon_soul.h"
#include "anticheat_manager.h"
#include "map_location.h"
#include "gm.h"
#include "battleground.h"

extern bool DropEvent_RefineBox_SetValue(const std::string& name, int32_t value);

// ADD_COMMAND_SLOW_STUN
enum ECommandAffect
{
	COMMANDAFFECT_STUN,
	COMMANDAFFECT_SLOW,
};

void Command_ApplyAffect(LPCHARACTER ch, const char* argument, const char* affectName, int32_t cmdAffect)
{
	char arg1[256];
	one_argument(argument, arg1, sizeof(arg1));

	sys_log(0, arg1);

	if (!*arg1)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, "Usage: %s <name>", affectName);
		return;
	}

	LPCHARACTER tch = CHARACTER_MANAGER::Instance().FindPC(arg1);
	if (!tch)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, "%s is not in same map", arg1);
		return;
	}
	else if (!tch->IsGM())
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("%s GM degil"), arg1);
		return;
	}

	switch (cmdAffect)
	{
		case COMMANDAFFECT_STUN:
			SkillAttackAffect(tch, 1000, IMMUNE_STUN, AFFECT_STUN, POINT_NONE, 0, AFF_STUN, 30, "GM_STUN");
			break;
		case COMMANDAFFECT_SLOW:
			SkillAttackAffect(tch, 1000, IMMUNE_SLOW, AFFECT_SLOW, POINT_MOV_SPEED, -30, AFF_SLOW, 30, "GM_SLOW");
			break;
	}

	sys_log(0, "%s %s", arg1, affectName);

	ch->ChatPacket(CHAT_TYPE_INFO, "%s %s", arg1, affectName);
}
// END_OF_ADD_COMMAND_SLOW_STUN

ACMD(do_stun)
{
	Command_ApplyAffect(ch, argument, "stun", COMMANDAFFECT_STUN);
}

ACMD(do_slow)
{
	Command_ApplyAffect(ch, argument, "slow", COMMANDAFFECT_SLOW);
}

ACMD(do_transfer)
{
	char arg1[256];
	one_argument(argument, arg1, sizeof(arg1));

	if (!*arg1)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, "Usage: transfer <name>");
		return;
	}

	LPCHARACTER tch = CHARACTER_MANAGER::Instance().FindPC(arg1); 
	if (!tch)
	{
		CCI * pkCCI = P2P_MANAGER::Instance().Find(arg1);

		if (pkCCI)
		{
			if (pkCCI->bChannel != g_bChannel)
			{
				ch->ChatPacket(CHAT_TYPE_INFO, "Target(%s) is in %d channel (my channel %d)", arg1, pkCCI->bChannel, g_bChannel);
				return;
			}

			TPacketGGTransfer pgg;

			pgg.bHeader = HEADER_GG_TRANSFER;
			strlcpy(pgg.szName, arg1, sizeof(pgg.szName));
			pgg.lX = ch->GetX();
			pgg.lY = ch->GetY();

			P2P_MANAGER::Instance().Send(&pgg, sizeof(TPacketGGTransfer));
			ch->ChatPacket(CHAT_TYPE_INFO, "Transfer requested.");
		}
		else
			ch->ChatPacket(CHAT_TYPE_INFO, "%s -> There is no character(%s) by that name", ch->GetName(), arg1);

		return;
	}

	if (ch == tch)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, "Transfer me?!?");
		return;
	}

	tch->ChatPacket(CHAT_TYPE_INFO, "%s isimli oyuncunun yanina isinlaniyorsun", ch->GetName());
	//tch->Show(ch->GetMapIndex(), ch->GetX(), ch->GetY(), ch->GetZ());
	tch->WarpSet(ch->GetX(), ch->GetY(), ch->GetMapIndex());
}

// LUA_ADD_GOTO_INFO
struct GotoInfo
{
	std::string 	st_name;

	uint8_t 	empire;
	int32_t 	mapIndex;
	uint32_t 	x, y;

	GotoInfo()
	{
		st_name 	= "";
		empire 		= 0;
		mapIndex 	= 0;

		x = 0;
		y = 0;
	}
	GotoInfo(const GotoInfo& c_src)
	{
		__copy__(c_src);
	}
	void operator = (const GotoInfo& c_src)
	{
		__copy__(c_src);
	}
	void __copy__(const GotoInfo& c_src)
	{
		st_name 	= c_src.st_name;
		empire 		= c_src.empire;
		mapIndex 	= c_src.mapIndex;

		x = c_src.x;
		y = c_src.y;
	}
};

static std::vector<GotoInfo> gs_vec_gotoInfo;

void CHARACTER_AddGotoInfo(const std::string& c_st_name, uint8_t empire, int32_t mapIndex, uint32_t x, uint32_t y)
{
	GotoInfo newGotoInfo;
	newGotoInfo.st_name = c_st_name;
	newGotoInfo.empire = empire;
	newGotoInfo.mapIndex = mapIndex;
	newGotoInfo.x = x;
	newGotoInfo.y = y;
	gs_vec_gotoInfo.push_back(newGotoInfo);

	if (g_bIsTestServer)
		sys_log(0, "AddGotoInfo(name=%s, empire=%d, mapIndex=%d, pos=(%d, %d))", c_st_name.c_str(), empire, mapIndex, x, y);
}

bool FindInString(const char * c_pszFind, const char * c_pszIn)
{
	const char * c = c_pszIn;
	const char * p;

	p = strchr(c, '|');

	if (!p)
		return (0 == strncasecmp(c_pszFind, c_pszIn, strlen(c_pszFind)));
	else
	{
		char sz[64 + 1];

		do
		{
			strlcpy(sz, c, MIN(sizeof(sz), (p - c) + 1));

			if (!strncasecmp(c_pszFind, sz, strlen(c_pszFind)))
				return true;

			c = p + 1;
		} while ((p = strchr(c, '|')));

		strlcpy(sz, c, sizeof(sz));

		if (!strncasecmp(c_pszFind, sz, strlen(c_pszFind)))
			return true;
	}

	return false;
}

bool CHARACTER_GoToName(LPCHARACTER ch, uint8_t empire, int32_t mapIndex, const char* gotoName)
{
	std::vector<GotoInfo>::iterator i;
	for (i = gs_vec_gotoInfo.begin(); i != gs_vec_gotoInfo.end(); ++i)
	{
		const GotoInfo& c_eachGotoInfo = *i;

		if (mapIndex != 0)
		{
			if (mapIndex != c_eachGotoInfo.mapIndex)
				continue;
		}
		else if (!FindInString(gotoName, c_eachGotoInfo.st_name.c_str()))
			continue;

		if (c_eachGotoInfo.empire == 0 || c_eachGotoInfo.empire == empire)
		{
			int32_t x, y;
			if (c_eachGotoInfo.x == 0 && c_eachGotoInfo.y == 0)
			{
				GPOS pos;
				SECTREE_MANAGER::Instance().GetRecallPositionByEmpire(c_eachGotoInfo.mapIndex, empire, pos);
				x = pos.x;
				y = pos.y;
			}
			else
			{
				x = c_eachGotoInfo.x * 100;
				y = c_eachGotoInfo.y * 100;
			}

			ch->ChatPacket(CHAT_TYPE_INFO, "You warp to ( %d, %d )", x, y);
			ch->WarpSet(x, y);
			ch->Stop();
			return true;
		}
	}
	return false;
}

// END_OF_LUA_ADD_GOTO_INFO

/*
   = {
   { "A1|영안읍성",		0, 1,  4693, 9642 },
   { "A3|자양현",		0, 3,  3608, 8776 },

   { "B1|조안읍성",		0, 21,  557, 1579 },
   { "B3|복정현",		0, 23, 1385, 2349 },

   { "C1|평무읍성",		0, 41, 9696, 2784 },
   { "C3|박라현",		0, 43, 8731, 2426 },

// Snow
{ "Snow|서한산",		1, 61, 4342, 2906 },
{ "Snow|서한산",		2, 61, 3752, 1749 },
{ "Snow|서한산",		3, 61, 4918, 1736 },

// Flame
{ "Flame|도염화지|화염",	1, 62, 5994, 7563 },
{ "Flame|도염화지|화염",	2, 62, 5978, 6222 },
{ "Flame|도염화지|화염",	3, 62, 7307, 6898 },

// Desert
{ "Desert|영비사막|사막",	1, 63, 2178, 6272 },
{ "Desert|영비사막|사막",	2, 63, 2219, 5027 },
{ "Desert|영비사막|사막",	3, 63, 3440, 5025 },

// Threeway
{ "Three|승룡곡",		1, 64, 4021, 6739 },
{ "Three|승룡곡",		2, 64, 2704, 7399 },
{ "Three|승룡곡",		3, 64, 3213, 8080 },

// 밀교사원
{ "Milgyo|밀교사원",	1, 65, 5536, 1436 },
{ "Milgyo|밀교사원",	2, 65, 5536, 1436 },
{ "Milgyo|밀교사원",	3, 65, 5536, 1436 },

// 사귀타워입구
{ "사귀타워입구",		1, 65, 5905, 1108 },
{ "사귀타워입구",		2, 65, 5905, 1108 },
{ "사귀타워입구",		3, 65, 5905, 1108 },

{ nullptr,			0,  0,    0,    0 },
};
 */


ACMD(do_goto)
{
	char arg1[256], arg2[256];
	int32_t x = 0, y = 0, z = 0;

	two_arguments(argument, arg1, sizeof(arg1), arg2, sizeof(arg2));

	if (!*arg1 && !*arg2)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, "Usage: goto <x meter> <y meter>");
		return;
	}

	if (isdigit(*arg1) && isdigit(*arg2))
	{
		str_to_number(x, arg1);
		str_to_number(y, arg2);

		GPOS p;

		if (SECTREE_MANAGER::Instance().GetMapBasePosition(ch->GetX(), ch->GetY(), p))
		{
			x += p.x / 100;
			y += p.y / 100;
		}

		ch->ChatPacket(CHAT_TYPE_INFO, "You goto ( %d, %d )", x, y);
	}
	else
	{
		int32_t mapIndex = 0;
		uint8_t empire = 0;

		if (*arg1 == '#')
			str_to_number(mapIndex,  (arg1 + 1));

		if (*arg2 && isdigit(*arg2))
		{
			str_to_number(empire, arg2);
			empire = MINMAX(1, empire, 3);
		}
		else
			empire = ch->GetEmpire();

		if (CHARACTER_GoToName(ch, empire, mapIndex, arg1))
		{
			ch->ChatPacket(CHAT_TYPE_INFO, "Cannot find map command syntax: /goto <mapname> [empire]");
			return;
		}

		return;

		/*
		   int32_t iMapIndex = 0;
		   for (int32_t i = 0; aWarpInfo[i].c_pszName != nullptr; ++i)
		   {
		   if (iMapIndex != 0)
		   {
		   if (iMapIndex != aWarpInfo[i].iMapIndex)
		   continue;
		   }
		   else if (!FindInString(arg1, aWarpInfo[i].c_pszName))
		   continue;

		   if (aWarpInfo[i].bEmpire == 0 || aWarpInfo[i].bEmpire == bEmpire)
		   {
		   x = aWarpInfo[i].x * 100;
		   y = aWarpInfo[i].y * 100;

		   ch->ChatPacket(CHAT_TYPE_INFO, "You warp to ( %d, %d )", x, y);
		   ch->WarpSet(x, y);
		   ch->Stop();
		   return;
		   }
		   }
		 */

	}

	x *= 100;
	y *= 100;

	ch->Show(ch->GetMapIndex(), x, y, z);
	ch->Stop();
}

ACMD(do_warp)
{
	char arg1[256], arg2[256];

	two_arguments(argument, arg1, sizeof(arg1), arg2, sizeof(arg2));

	if (!*arg1)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, "Usage: warp <character name> | <x meter> <y meter>");
		return;
	}

	int32_t x = 0, y = 0;
	int32_t mapIndex = 0;

	if (isdigit(*arg1) && isdigit(*arg2))
	{
		str_to_number(x, arg1);
		str_to_number(y, arg2);
	}
	else
	{
		LPCHARACTER tch = CHARACTER_MANAGER::Instance().FindPC(arg1);

		if (nullptr == tch)
		{
			const CCI* pkCCI = P2P_MANAGER::Instance().Find(arg1);

			if (nullptr != pkCCI)
			{
				if (pkCCI->bChannel != g_bChannel)
				{
					ch->ChatPacket(CHAT_TYPE_INFO, "Target(%s) is in %d channel (my channel %d)", arg1, pkCCI->bChannel, g_bChannel);
					return;
				}

				ch->WarpToPID( pkCCI->dwPID );
			}
			else
			{
				ch->ChatPacket(CHAT_TYPE_INFO, "There is no one(%s) by that name", arg1);
			}

			return;
		}
		else
		{
			x = tch->GetX() / 100;
			y = tch->GetY() / 100;
			mapIndex = tch->GetMapIndex();
		}
	}

	x *= 100;
	y *= 100;

	ch->ChatPacket(CHAT_TYPE_INFO, "You warp to ( %d, %d, %d )", x, y, mapIndex);
	ch->WarpSet(x, y, mapIndex);
	ch->Stop();
}

ACMD(do_rewarp)
{
	ch->ChatPacket(CHAT_TYPE_INFO, "You warp to ( %d, %d )", ch->GetX(), ch->GetY());
	ch->WarpSet(ch->GetX(), ch->GetY());
	ch->Stop();
}

ACMD(do_item)
{
	char arg1[256], arg2[256];
	two_arguments(argument, arg1, sizeof(arg1), arg2, sizeof(arg2));

	if (!*arg1)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, "Usage: item <item vnum>");
		return;
	}

	int32_t iCount = 1;

	if (*arg2)
	{
		str_to_number(iCount, arg2);
		iCount = MINMAX(1, iCount, g_bItemCountLimit);
	}

	uint32_t dwVnum;

	if (isdigit(*arg1))
		str_to_number(dwVnum, arg1);
	else
	{
		if (!ITEM_MANAGER::Instance().GetVnum(arg1, dwVnum))
		{
			ch->ChatPacket(CHAT_TYPE_INFO, "#%u item not exist by that vnum(%s).", dwVnum, arg1);
			return;
		}
	}

	LPITEM item = ITEM_MANAGER::Instance().CreateItem(dwVnum, iCount, 0, true);

	if (item)
	{
		if (item->IsDragonSoul())
		{
			int32_t iEmptyPos = ch->GetEmptyDragonSoulInventory(item);

			if (iEmptyPos != -1)
			{
				item->AddToCharacter(ch, TItemPos(DRAGON_SOUL_INVENTORY, iEmptyPos));
				LogManager::Instance().ItemLog(ch, item, "GM", item->GetName());
			}
			else
			{
				M2_DESTROY_ITEM(item);
				if (!ch->DragonSoul_IsQualified())
				{
					ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("Bunu kullanabilmek icin gerekli yetkiye sahip degilsin."));
				}
				else
					ch->ChatPacket(CHAT_TYPE_INFO, "Not enough inventory space.");
			}
		}
		else
		{
			int32_t iEmptyPos = ch->GetEmptyInventory(item->GetSize());

			if (iEmptyPos != -1)
			{
				item->AddToCharacter(ch, TItemPos(INVENTORY, iEmptyPos));
				LogManager::Instance().ItemLog(ch, item, "GM", item->GetName());
			}
			else
			{
				M2_DESTROY_ITEM(item);
				ch->ChatPacket(CHAT_TYPE_INFO, "Not enough inventory space.");
			}
		}
	}
	else
	{
		ch->ChatPacket(CHAT_TYPE_INFO, "#%u item not exist by that vnum(%s).", dwVnum, arg1);
	}
}

ACMD(do_group_random)
{
	char arg1[256];
	one_argument(argument, arg1, sizeof(arg1));

	if (!*arg1)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, "Usage: grrandom <group vnum>");
		return;
	}

	uint32_t dwVnum = 0;
	str_to_number(dwVnum, arg1);
	CHARACTER_MANAGER::Instance().SpawnGroupGroup(dwVnum, ch->GetMapIndex(), ch->GetX() - 500, ch->GetY() - 500, ch->GetX() + 500, ch->GetY() + 500);
}

ACMD(do_group)
{
	char arg1[256];
	one_argument(argument, arg1, sizeof(arg1));

	if (!*arg1)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, "Usage: group <group vnum>");
		return;
	}

	uint32_t dwVnum = 0;
	str_to_number(dwVnum, arg1);

	if (g_bIsTestServer)
		sys_log(0, "COMMAND GROUP SPAWN %u at %u %u %u", dwVnum, ch->GetMapIndex(), ch->GetX(), ch->GetY());

	CHARACTER_MANAGER::Instance().SpawnGroup(dwVnum, ch->GetMapIndex(), ch->GetX() - 500, ch->GetY() - 500, ch->GetX() + 500, ch->GetY() + 500);
}

ACMD(do_mob_coward)
{
	char	arg1[256], arg2[256];
	uint32_t	vnum = 0;
	LPCHARACTER	tch;

	two_arguments(argument, arg1, sizeof(arg1), arg2, sizeof(arg2));

	if (!*arg1)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, "Usage: mc <vnum>");
		return;
	}

	const CMob * pkMob;

	if (isdigit(*arg1))
	{
		str_to_number(vnum, arg1);

		if ((pkMob = CMobManager::Instance().Get(vnum)) == nullptr)
			vnum = 0;
	}
	else
	{
		pkMob = CMobManager::Instance().Get(arg1, true);

		if (pkMob)
			vnum = pkMob->m_table.dwVnum;
	}

	if (vnum == 0)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, "No such mob(%s) by that vnum", arg1);
		return;
	}

	int32_t iCount = 0;

	if (*arg2)
		str_to_number(iCount, arg2);
	else
		iCount = 1;

	iCount = MIN(20, iCount);

	while (iCount--)
	{
		tch = CHARACTER_MANAGER::Instance().SpawnMobRange(vnum, 
				ch->GetMapIndex(),
				ch->GetX() - number(200, 750), 
				ch->GetY() - number(200, 750), 
				ch->GetX() + number(200, 750), 
				ch->GetY() + number(200, 750), 
				true,
				pkMob->m_table.bType == CHAR_TYPE_STONE);
		if (tch)
			tch->SetCoward();
	}
}

ACMD(do_mob_map)
{
	char arg1[256];
	one_argument(argument, arg1, sizeof(arg1));

	if (!*arg1)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, "Syntax: mm <vnum>");
		return;
	}

	uint32_t vnum = 0;
	str_to_number(vnum, arg1);
	LPCHARACTER tch = CHARACTER_MANAGER::Instance().SpawnMobRandomPosition(vnum, ch->GetMapIndex());

	if (tch)
		ch->ChatPacket(CHAT_TYPE_INFO, "%s spawned in %dx%d", tch->GetName(), tch->GetX(), tch->GetY());
	else
		ch->ChatPacket(CHAT_TYPE_INFO, "Spawn failed.");
}

ACMD(do_mob_aggresive)
{
	char	arg1[256], arg2[256];
	uint32_t	vnum = 0;
	LPCHARACTER	tch;

	two_arguments(argument, arg1, sizeof(arg1), arg2, sizeof(arg2));

	if (!*arg1)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, "Usage: mob <mob vnum>");
		return;
	}

	const CMob * pkMob;

	if (isdigit(*arg1))
	{
		str_to_number(vnum, arg1);

		if ((pkMob = CMobManager::Instance().Get(vnum)) == nullptr)
			vnum = 0;
	}
	else
	{
		pkMob = CMobManager::Instance().Get(arg1, true);

		if (pkMob)
			vnum = pkMob->m_table.dwVnum;
	}

	if (vnum == 0)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, "No such mob(%s) by that vnum", arg1);
		return;
	}

	int32_t iCount = 0;

	if (*arg2)
		str_to_number(iCount, arg2);
	else
		iCount = 1;

	iCount = MIN(20, iCount);

	while (iCount--)
	{
		tch = CHARACTER_MANAGER::Instance().SpawnMobRange(vnum, 
				ch->GetMapIndex(),
				ch->GetX() - number(200, 750), 
				ch->GetY() - number(200, 750), 
				ch->GetX() + number(200, 750), 
				ch->GetY() + number(200, 750), 
				true,
				pkMob->m_table.bType == CHAR_TYPE_STONE);
		if (tch)
			tch->SetAggressive();
	}
}

ACMD(do_mob)
{
	char	arg1[256], arg2[256];
	uint32_t	vnum = 0;

	two_arguments(argument, arg1, sizeof(arg1), arg2, sizeof(arg2));

	if (!*arg1)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, "Usage: mob <mob vnum>");
		return;
	}

	const CMob* pkMob = nullptr;

	if (isdigit(*arg1))
	{
		str_to_number(vnum, arg1);

		if ((pkMob = CMobManager::Instance().Get(vnum)) == nullptr)
			vnum = 0;
	}
	else
	{
		pkMob = CMobManager::Instance().Get(arg1, true);

		if (pkMob)
			vnum = pkMob->m_table.dwVnum;
	}

	if (vnum == 0)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, "No such mob(%s) by that vnum", arg1);
		return;
	}

	int32_t iCount = 0;

	if (*arg2)
		str_to_number(iCount, arg2);
	else
		iCount = 1;

	if (g_bIsTestServer)
		iCount = MIN(40, iCount);
	else
		iCount = MIN(20, iCount);

	while (iCount--)
	{
		CHARACTER_MANAGER::Instance().SpawnMobRange(vnum, 
				ch->GetMapIndex(),
				ch->GetX() - number(200, 750), 
				ch->GetY() - number(200, 750), 
				ch->GetX() + number(200, 750), 
				ch->GetY() + number(200, 750), 
				true,
				pkMob->m_table.bType == CHAR_TYPE_STONE);
	}
}

ACMD(do_mob_ld)
{
	char	arg1[256], arg2[256], arg3[256], arg4[256];
	uint32_t	vnum = 0;

	two_arguments(two_arguments(argument, arg1, sizeof(arg1), arg2, sizeof(arg2)), arg3, sizeof(arg3), arg4, sizeof(arg4));

	if (!*arg1)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, "Usage: mob <mob vnum>");
		return;
	}

	const CMob* pkMob = nullptr;

	if (isdigit(*arg1))
	{
		str_to_number(vnum, arg1);

		if ((pkMob = CMobManager::Instance().Get(vnum)) == nullptr)
			vnum = 0;
	}
	else
	{
		pkMob = CMobManager::Instance().Get(arg1, true);

		if (pkMob)
			vnum = pkMob->m_table.dwVnum;
	}

	if (vnum == 0)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, "No such mob(%s) by that vnum", arg1);
		return;
	}

	int32_t dir = 1;
	int32_t x=0,y=0;

	if (*arg2)
		str_to_number(x, arg2);
	if (*arg3)
		str_to_number(y, arg3);
	if (*arg4)
		str_to_number(dir, arg4);


	CHARACTER_MANAGER::Instance().SpawnMob(vnum, 
		ch->GetMapIndex(),
		x*100, 
		y*100, 
		ch->GetZ(),
		pkMob->m_table.bType == CHAR_TYPE_STONE,
		dir);
}

struct FuncPurge
{
	LPCHARACTER m_pkGM;
	bool	m_bAll;

	FuncPurge(LPCHARACTER ch) : m_pkGM(ch), m_bAll(false)
	{
	}

	void operator () (LPENTITY ent)
	{
		if (!ent->IsType(ENTITY_CHARACTER))
			return;

		LPCHARACTER pkChr = (LPCHARACTER) ent;

		int32_t iDist = DISTANCE_APPROX(pkChr->GetX() - m_pkGM->GetX(), pkChr->GetY() - m_pkGM->GetY());

		if (!m_bAll && iDist >= 1000)	// 10미터 이상에 있는 것들은 purge 하지 않는다.
			return;

		sys_log(0, "PURGE: %s %d", pkChr->GetName(), iDist);

		if (pkChr->IsNPC() && !pkChr->IsPet() && pkChr->GetRider() == nullptr)
		{
			M2_DESTROY_CHARACTER(pkChr);
		}
	}
};

ACMD(do_purge)
{
	char arg1[256];
	one_argument(argument, arg1, sizeof(arg1));

	FuncPurge func(ch);

	bool bMap = false;
	if (*arg1 && (!strcmp(arg1, "all") || !strcmp(arg1, "map")))
	{
		func.m_bAll = true;
		if (!strcmp(arg1, "map"))
			bMap = true;
	}

	if (!bMap)
	{
		LPSECTREE sectree = ch->GetSectree();
		if (sectree)
			sectree->ForEachAround(func);
		else
			sys_err("PURGE_ERROR.NULL_SECTREE(mapIndex=%d, pos=(%d, %d)", ch->GetMapIndex(), ch->GetX(), ch->GetY());
	}
	else
	{
		LPSECTREE_MAP sectree_map = SECTREE_MANAGER::Instance().GetMap(ch->GetMapIndex());
		if (sectree_map)
			sectree_map->for_each(func);
		else
			sys_err("PURGE_ERROR.NULL_MAP(mapIndex=%d)", ch->GetMapIndex());
	}
}

ACMD(do_item_purge)
{
	char arg1[256];
	one_argument(argument, arg1, sizeof(arg1));
	if (!*arg1)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, "Usage: ipurge <window>");
		ch->ChatPacket(CHAT_TYPE_INFO, "List of the available windows:");
		ch->ChatPacket(CHAT_TYPE_INFO, " all");
		ch->ChatPacket(CHAT_TYPE_INFO, " inventory or inv");
		ch->ChatPacket(CHAT_TYPE_INFO, " equipment or equip");
		ch->ChatPacket(CHAT_TYPE_INFO, " dragonsoul or ds");
		ch->ChatPacket(CHAT_TYPE_INFO, " belt");
		return;
	}

	int32_t         i;
	LPITEM      item;

	std::string strArg(arg1);
	if (!strArg.compare(0, 3, "all"))
	{
		for (i = 0; i < INVENTORY_AND_EQUIP_SLOT_MAX; ++i)
		{
			if ((item = ch->GetInventoryItem(i)))
			{
				ITEM_MANAGER::Instance().RemoveItem(item, "PURGE");
				ch->SyncQuickslot(QUICKSLOT_TYPE_ITEM, i, 255);
			}
		}
		for (i = 0; i < DRAGON_SOUL_INVENTORY_MAX_NUM; ++i)
		{
			if ((item = ch->GetItem(TItemPos(DRAGON_SOUL_INVENTORY, i ))))
			{
				ITEM_MANAGER::Instance().RemoveItem(item, "PURGE");
			}
		}
	}
	else if (!strArg.compare(0, 3, "inv"))
	{
		for (i = 0; i < INVENTORY_MAX_NUM; ++i)
		{
			if ((item = ch->GetInventoryItem(i)))
			{
				ITEM_MANAGER::Instance().RemoveItem(item, "PURGE");
				ch->SyncQuickslot(QUICKSLOT_TYPE_ITEM, i, 255);
			}
		}
	}
	else if (!strArg.compare(0, 5, "equip"))
	{
		for (i = 0; i < WEAR_MAX_NUM; ++i)
		{
			if ((item = ch->GetInventoryItem(INVENTORY_MAX_NUM + i)))
			{
				ITEM_MANAGER::Instance().RemoveItem(item, "PURGE");
				ch->SyncQuickslot(QUICKSLOT_TYPE_ITEM, INVENTORY_MAX_NUM + i, 255);
			}
		}
	}
	else if (!strArg.compare(0, 6, "dragon") || !strArg.compare(0, 2, "ds"))
	{
		for (i = 0; i < DRAGON_SOUL_INVENTORY_MAX_NUM; ++i)
		{
			if ((item = ch->GetItem(TItemPos(DRAGON_SOUL_INVENTORY, i ))))
			{
				ITEM_MANAGER::Instance().RemoveItem(item, "PURGE");
			}
		}
	}
	else if (!strArg.compare(0, 4, "belt"))
	{
		for (i = 0; i < BELT_INVENTORY_SLOT_COUNT; ++i)
		{
			if ((item = ch->GetInventoryItem(BELT_INVENTORY_SLOT_START + i)))
			{
				ITEM_MANAGER::Instance().RemoveItem(item, "PURGE");
				ch->SyncQuickslot(QUICKSLOT_TYPE_ITEM, BELT_INVENTORY_SLOT_START + i, 255);
			}
		}
	}
}

ACMD(do_state)
{
	char arg1[256];
	LPCHARACTER tch;

	one_argument(argument, arg1, sizeof(arg1));

	if (*arg1)
	{
		if (arg1[0] == '#')
		{
			tch = CHARACTER_MANAGER::Instance().Find(strtoul(arg1+1, nullptr, 10));
		}
		else
		{
			LPDESC d = DESC_MANAGER::Instance().FindByCharacterName(arg1);

			if (!d)
				tch = nullptr;
			else
				tch = d->GetCharacter();
		}
	}
	else
		tch = ch;

	if (!tch)
	{
		CCI* pkCCI = P2P_MANAGER::Instance().Find (arg1);
		if (!pkCCI || !pkCCI->pkDesc)
		{
			ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT ("Player %s isn't currently online."), arg1);
			return;
		}
		else
		{
			ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT ("Player %s is on channel %d."), arg1, (int32_t) pkCCI->bChannel);
			return;
		}
	}

	std::string state = "";

	if (tch->IsPosition(POS_FIGHTING))
		state = "Battle";
	else if (tch->IsPosition(POS_DEAD))
		state = "Dead";
	else if (tch->IsPosition (POS_FISHING))
		state = "Fishing";
	else
		state = "Standing";

	if (ch->GetShop())
		state += ", Shop";

	if (ch->GetExchange())
		state += ", Exchange";

	char buf[256];
	snprintf (buf, sizeof (buf), "%s's VID:(%d) CH:(%d) State: %s", tch->GetName(), (uint32_t)tch->GetVID(), g_bChannel, state.c_str());

	int32_t len;
	len = snprintf(buf, sizeof(buf), "Coordinate %dx%d (%dx%d) (rotation %.2f)",
			tch->GetX(), tch->GetY(), tch->GetX() / 100, tch->GetY() / 100, tch->GetRotation());

	if (len < 0 || len >= (int32_t) sizeof(buf))
		len = sizeof(buf) - 1;

	LPSECTREE pSec = SECTREE_MANAGER::Instance().Get(tch->GetMapIndex(), tch->GetX(), tch->GetY());

	if (pSec)
	{
		TMapSetting& map_setting = SECTREE_MANAGER::Instance().GetMap(tch->GetMapIndex())->m_setting;
		snprintf(buf + len, sizeof(buf) - len, " MapIndex %d Attribute %08X Local Position (%d x %d)", 
			tch->GetMapIndex(), pSec->GetAttribute(tch->GetX(), tch->GetY()), (tch->GetX() - map_setting.iBaseX)/100, (tch->GetY() - map_setting.iBaseY)/100);
	}

	ch->ChatPacket(CHAT_TYPE_INFO, "%s", buf);

	ch->ChatPacket (CHAT_TYPE_INFO, "Level %d", tch->GetLevel());
	ch->ChatPacket(CHAT_TYPE_INFO, "HP %d/%d", tch->GetHP(), tch->GetMaxHP());
	ch->ChatPacket(CHAT_TYPE_INFO, "SP %d/%d", tch->GetSP(), tch->GetMaxSP());
	ch->ChatPacket(CHAT_TYPE_INFO, "ATT %d MAGIC_ATT %d SPD %d CRIT %d%% PENE %d%% ATT_BONUS %d%%",
			tch->GetPoint(POINT_ATT_GRADE),
			tch->GetPoint(POINT_MAGIC_ATT_GRADE),
			tch->GetPoint(POINT_ATT_SPEED),
			tch->GetPoint(POINT_CRITICAL_PCT),
			tch->GetPoint(POINT_PENETRATE_PCT),
			tch->GetPoint(POINT_ATT_BONUS));
	ch->ChatPacket(CHAT_TYPE_INFO, "DEF %d MAGIC_DEF %d BLOCK %d%% DODGE %d%% DEF_BONUS %d%%", 
			tch->GetPoint(POINT_DEF_GRADE),
			tch->GetPoint(POINT_MAGIC_DEF_GRADE),
			tch->GetPoint(POINT_BLOCK),
			tch->GetPoint(POINT_DODGE),
			tch->GetPoint(POINT_DEF_BONUS));
	ch->ChatPacket(CHAT_TYPE_INFO, "RESISTANCES:");
	ch->ChatPacket(CHAT_TYPE_INFO, "   WARR:%3d%% ASAS:%3d%% SURA:%3d%% SHAM:%3d%%"
#ifdef ENABLE_WOLFMAN_CHARACTER
			" WOLF:%3d%%"
#endif
			,
			tch->GetPoint(POINT_RESIST_WARRIOR),
			tch->GetPoint(POINT_RESIST_ASSASSIN),
			tch->GetPoint(POINT_RESIST_SURA),
			tch->GetPoint(POINT_RESIST_SHAMAN)
#ifdef ENABLE_WOLFMAN_CHARACTER
			,tch->GetPoint(POINT_RESIST_WOLFMAN)
#endif
	);
	ch->ChatPacket(CHAT_TYPE_INFO, "   SWORD:%3d%% THSWORD:%3d%% DAGGER:%3d%% BELL:%3d%% FAN:%3d%% BOW:%3d%%"
#ifdef ENABLE_WOLFMAN_CHARACTER
			" CLAW:%3d%%"
#endif
			,
			tch->GetPoint(POINT_RESIST_SWORD),
			tch->GetPoint(POINT_RESIST_TWOHAND),
			tch->GetPoint(POINT_RESIST_DAGGER),
			tch->GetPoint(POINT_RESIST_BELL),
			tch->GetPoint(POINT_RESIST_FAN),
			tch->GetPoint(POINT_RESIST_BOW)
#ifdef ENABLE_WOLFMAN_CHARACTER
			,tch->GetPoint(POINT_RESIST_CLAW)
#endif
	);
	ch->ChatPacket(CHAT_TYPE_INFO, "   FIRE:%3d%% ELEC:%3d%% MAGIC:%3d%% WIND:%3d%% CRIT:%3d%% PENE:%3d%%",
			tch->GetPoint(POINT_RESIST_FIRE),
			tch->GetPoint(POINT_RESIST_ELEC),
			tch->GetPoint(POINT_RESIST_MAGIC),
			tch->GetPoint(POINT_RESIST_WIND),
			tch->GetPoint(POINT_RESIST_CRITICAL),
			tch->GetPoint(POINT_RESIST_PENETRATE));
	ch->ChatPacket(CHAT_TYPE_INFO, "   ICE:%3d%% EARTH:%3d%% DARK:%3d%%",
			tch->GetPoint(POINT_RESIST_ICE),
			tch->GetPoint(POINT_RESIST_EARTH),
			tch->GetPoint(POINT_RESIST_DARK));

	ch->ChatPacket(CHAT_TYPE_INFO, "   MAGICREDUCT:%3d%%", tch->GetPoint(POINT_RESIST_MAGIC_REDUCTION));

	ch->ChatPacket(CHAT_TYPE_INFO, "MALL:");
	ch->ChatPacket(CHAT_TYPE_INFO, "   ATT:%3d%% DEF:%3d%% EXP:%3d%% ITEMx%d GOLDx%d",
			tch->GetPoint(POINT_MALL_ATTBONUS),
			tch->GetPoint(POINT_MALL_DEFBONUS),
			tch->GetPoint(POINT_MALL_EXPBONUS),
			tch->GetPoint(POINT_MALL_ITEMBONUS) / 10,
			tch->GetPoint(POINT_MALL_GOLDBONUS) / 10);

	ch->ChatPacket(CHAT_TYPE_INFO, "BONUS:");
	ch->ChatPacket(CHAT_TYPE_INFO, "   SKILL:%3d%% NORMAL:%3d%% SKILL_DEF:%3d%% NORMAL_DEF:%3d%%",
			tch->GetPoint(POINT_SKILL_DAMAGE_BONUS),
			tch->GetPoint(POINT_NORMAL_HIT_DAMAGE_BONUS),
			tch->GetPoint(POINT_SKILL_DEFEND_BONUS),
			tch->GetPoint(POINT_NORMAL_HIT_DEFEND_BONUS));

	ch->ChatPacket(CHAT_TYPE_INFO, "   HUMAN:%3d%% ANIMAL:%3d%% ORC:%3d%% MILGYO:%3d%% UNDEAD:%3d%%",
			tch->GetPoint(POINT_ATTBONUS_HUMAN),
			tch->GetPoint(POINT_ATTBONUS_ANIMAL),
			tch->GetPoint(POINT_ATTBONUS_ORC),
			tch->GetPoint(POINT_ATTBONUS_MILGYO),
			tch->GetPoint(POINT_ATTBONUS_UNDEAD));

	ch->ChatPacket(CHAT_TYPE_INFO, "   DEVIL:%3d%% INSECT:%3d%% FIRE:%3d%% ICE:%3d%% DESERT:%3d%%",
			tch->GetPoint(POINT_ATTBONUS_DEVIL),
			tch->GetPoint(POINT_ATTBONUS_INSECT),
			tch->GetPoint(POINT_ATTBONUS_FIRE),
			tch->GetPoint(POINT_ATTBONUS_ICE),
			tch->GetPoint(POINT_ATTBONUS_DESERT));

	ch->ChatPacket(CHAT_TYPE_INFO, "   TREE:%3d%% MONSTER:%3d%%",
			tch->GetPoint(POINT_ATTBONUS_TREE),
			tch->GetPoint(POINT_ATTBONUS_MONSTER));

	ch->ChatPacket(CHAT_TYPE_INFO, "   WARR:%3d%% ASAS:%3d%% SURA:%3d%% SHAM:%3d%%"
#ifdef ENABLE_WOLFMAN_CHARACTER
			" WOLF:%3d%%"
#endif
			,
			tch->GetPoint(POINT_ATTBONUS_WARRIOR),
			tch->GetPoint(POINT_ATTBONUS_ASSASSIN),
			tch->GetPoint(POINT_ATTBONUS_SURA),
			tch->GetPoint(POINT_ATTBONUS_SHAMAN)
#ifdef ENABLE_WOLFMAN_CHARACTER
			,tch->GetPoint(POINT_ATTBONUS_WOLFMAN)
#endif
	);
	ch->ChatPacket(CHAT_TYPE_INFO, "IMMUNE:");
	ch->ChatPacket(CHAT_TYPE_INFO, "   STUN:%d SLOW:%d FALL:%d",
		tch->GetPoint(POINT_IMMUNE_STUN),
		tch->GetPoint(POINT_IMMUNE_SLOW),
		tch->GetPoint(POINT_IMMUNE_FALL));

	ch->ChatPacket(CHAT_TYPE_INFO, "MARRIAGE:");
	ch->ChatPacket(CHAT_TYPE_INFO, "   PENE:%d EXP:%d CRIT:%d TRAN:%d ATT:%d DEF:%d",
		tch->GetMarriageBonus(UNIQUE_ITEM_MARRIAGE_PENETRATE_BONUS),
		tch->GetMarriageBonus(UNIQUE_ITEM_MARRIAGE_EXP_BONUS),
		tch->GetMarriageBonus(UNIQUE_ITEM_MARRIAGE_CRITICAL_BONUS),
		tch->GetMarriageBonus(UNIQUE_ITEM_MARRIAGE_TRANSFER_DAMAGE),
		tch->GetMarriageBonus(UNIQUE_ITEM_MARRIAGE_ATTACK_BONUS),
		tch->GetMarriageBonus(UNIQUE_ITEM_MARRIAGE_DEFENSE_BONUS)
	);

	for (int32_t i = 0; i < MAX_PRIV_NUM; ++i)
	{
		if (CPrivManager::Instance().GetPriv(tch, i))
		{
			int32_t iByEmpire = CPrivManager::Instance().GetPrivByEmpire(tch->GetEmpire(), i);
			int32_t iByGuild = 0;

			if (tch->GetGuild())
				iByGuild = CPrivManager::Instance().GetPrivByGuild(tch->GetGuild()->GetID(), i);

			int32_t iByPlayer = CPrivManager::Instance().GetPrivByCharacter(tch->GetPlayerID(), i);

			if (iByEmpire)
				ch->ChatPacket(CHAT_TYPE_INFO, "%s for empire : %d", LC_TEXT(c_apszPrivNames[i]), iByEmpire);

			if (iByGuild)
				ch->ChatPacket(CHAT_TYPE_INFO, "%s for guild : %d", LC_TEXT(c_apszPrivNames[i]), iByGuild);

			if (iByPlayer)
				ch->ChatPacket(CHAT_TYPE_INFO, "%s for player : %d", LC_TEXT(c_apszPrivNames[i]), iByPlayer);
		}
}
}

struct notice_packet_func
{
	const char * m_str;
	bool m_bBigFont;
	notice_packet_func(const char * str, bool bBigFont=false) : m_str(str), m_bBigFont(bBigFont)
	{
	}

	void operator () (LPDESC d)
	{
		if (!d->GetCharacter())
			return;

		d->GetCharacter()->ChatPacket((m_bBigFont) ? CHAT_TYPE_BIG_NOTICE : CHAT_TYPE_NOTICE, "%s", m_str);
	}
};

struct command_chat_packet_func
{
	const char * m_str;

	command_chat_packet_func(const char * str) : m_str(str)
	{
	}

	void operator () (LPDESC d)
	{
		if (!d->GetCharacter())
			return;

		d->GetCharacter()->ChatPacket(CHAT_TYPE_COMMAND, "%s", m_str);
		if (d->GetCharacter()->IsGM())
			d->GetCharacter()->ChatPacket(CHAT_TYPE_INFO, "<CMD-INFO> %s", m_str);
	}
};

void SendNotice(const char * c_pszBuf, bool bBigFont)
{
	const DESC_MANAGER::DESC_SET & c_ref_set = DESC_MANAGER::Instance().GetClientSet();
	std::for_each(c_ref_set.begin(), c_ref_set.end(), notice_packet_func(c_pszBuf, bBigFont));
}

void SendCmdchat(const char * c_pszBuf)
{
	const DESC_MANAGER::DESC_SET & c_ref_set = DESC_MANAGER::Instance().GetClientSet();
	std::for_each(c_ref_set.begin(), c_ref_set.end(), command_chat_packet_func(c_pszBuf));
}

struct notice_map_packet_func
{
	const char* m_str;
	int32_t m_mapIndex;
	bool m_bBigFont;

	notice_map_packet_func(const char* str, int32_t idx, bool bBigFont) : m_str(str), m_mapIndex(idx), m_bBigFont(bBigFont)
	{
	}

	void operator() (LPDESC d)
	{
		if (d->GetCharacter() == nullptr) return;
		if (d->GetCharacter()->GetMapIndex() != m_mapIndex) return;

		d->GetCharacter()->ChatPacket(m_bBigFont == true ? CHAT_TYPE_BIG_NOTICE : CHAT_TYPE_NOTICE, "%s", m_str);
	}
};

void SendNoticeMap(const char* c_pszBuf, int32_t nMapIndex, bool bBigFont)
{
	const DESC_MANAGER::DESC_SET & c_ref_set = DESC_MANAGER::Instance().GetClientSet();
	std::for_each(c_ref_set.begin(), c_ref_set.end(), notice_map_packet_func(c_pszBuf, nMapIndex, bBigFont));
}

struct log_packet_func
{
	const char * m_str;

	log_packet_func(const char * str) : m_str(str)
	{
	}

	void operator () (LPDESC d)
	{
		if (!d->GetCharacter())
			return;

		if (d->GetCharacter()->GetGMLevel() > GM_PLAYER)
			d->GetCharacter()->ChatPacket(CHAT_TYPE_NOTICE, "%s", m_str);
	}
};


void SendLog(const char * c_pszBuf)
{
	const DESC_MANAGER::DESC_SET & c_ref_set = DESC_MANAGER::Instance().GetClientSet();
	std::for_each(c_ref_set.begin(), c_ref_set.end(), log_packet_func(c_pszBuf));
}

void BroadcastNotice(const char * c_pszBuf, bool bBigFont)
{
	TPacketGGNotice p;
	p.bHeader = (bBigFont)?HEADER_GG_BIG_NOTICE:HEADER_GG_NOTICE;
	p.lSize = strlen(c_pszBuf) + 1;

	TEMP_BUFFER buf;
	buf.write(&p, sizeof(p));
	buf.write(c_pszBuf, p.lSize);

	P2P_MANAGER::Instance().Send(buf.read_peek(), buf.size()); // HEADER_GG_NOTICE

	SendNotice(c_pszBuf, bBigFont);
}

void BroadcastCmdchat(const char * c_pszBuf)
{
	SPacketGCChat pack_chat;
	pack_chat.size = sizeof(SPacketGCChat) + strlen(c_pszBuf);
	pack_chat.type = CHAT_TYPE_COMMAND;
	pack_chat.dwVID = 0;

	TEMP_BUFFER buf;
	buf.write(&pack_chat, sizeof(SPacketGCChat));
	buf.write(c_pszBuf, strlen(c_pszBuf));

	P2P_MANAGER::Instance().Send(buf.read_peek(), buf.size());

	SendCmdchat(c_pszBuf);
}

ACMD(do_notice)
{
	BroadcastNotice(argument);
}

ACMD(do_map_notice)
{
	SendNoticeMap(argument, ch->GetMapIndex(), false);
}

ACMD(do_big_notice)
{
	BroadcastNotice(argument, true);
}

ACMD(do_cmdchati)
{
	BroadcastCmdchat(argument);
}

ACMD(do_map_big_notice)
{
	SendNoticeMap(argument, ch->GetMapIndex(), true);
}

ACMD(do_notice_test)
{
	ch->ChatPacket(CHAT_TYPE_NOTICE, "%s", argument);
}

ACMD(do_big_notice_test)
{
	ch->ChatPacket(CHAT_TYPE_BIG_NOTICE, "%s", argument);
}

ACMD(do_get_ip)
{
	char arg1[256], arg2[256], szQuery[1024], szQuery_[1024];
	int32_t limit = 0;

	two_arguments(argument, arg1, sizeof(arg1), arg2, sizeof(arg2));

	if (!*arg1)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, "Wrong syntax, use: /get_ip <char name> [< LIMIT >]");
		return;
	}

	if (*arg2)
		str_to_number(limit, arg2);
	else
		limit = 0;

	snprintf(szQuery, sizeof(szQuery), "SELECT id FROM player.player WHERE name = '%s'", arg1);
	std::unique_ptr<SQLMsg> msg(DBManager::Instance().DirectQuery(szQuery));

	if (msg->Get()->uiNumRows == 0)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, "The character doesn't exist!");
		return;
	}

	MYSQL_ROW row = mysql_fetch_row(msg->Get()->pSQLResult);

	if (limit == 0) {
		snprintf(szQuery_, sizeof(szQuery_), "SELECT INET_NTOA(ip) AS connect_ip FROM log.loginlog2 WHERE pid = %s", row[0]);
	}
	else if (limit > 0) {
		snprintf(szQuery_, sizeof(szQuery_), "SELECT INET_NTOA(ip) AS connect_ip FROM log.loginlog2 WHERE pid = %s LIMIT %d", row[0], limit);
	}
	std::unique_ptr<SQLMsg> msg_(DBManager::Instance().DirectQuery(szQuery_));

	if (msg_->Get()->uiNumRows == 0)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, "The character has never connected to the game!");
		return;
	}
	ch->ChatPacket(CHAT_TYPE_INFO, "The ips from the character %s is:", arg1);

	while (MYSQL_ROW row1 = mysql_fetch_row(msg_->Get()->pSQLResult))
	{
		ch->ChatPacket(CHAT_TYPE_INFO, "%s", row1[0]);
	}
}

ACMD(do_drop_item)
{
	//#Pass 1. With one arg:  args[0] = Cell
	//#Pass 2. With two args: args[0] = BeginCell args[1] = EndCell
	char args[2][256];

	argument = two_arguments(argument, args[0], 256, args[1], 256);
	if (!*args[0])
	{
		ch->ChatPacket(CHAT_TYPE_INFO, "Usage: /drop_item <SlotPos> or");
		ch->ChatPacket(CHAT_TYPE_INFO, "           /drop_item <BeginPos> <EndPos>");
		return;
	}

	if (!*args[1])
	{
		int32_t Cell = 0;
		str_to_number(Cell, args[0]);
		if (Cell >= 0 && Cell < INVENTORY_MAX_NUM)
			ch->DropItem(TItemPos(INVENTORY, Cell));
		else
			ch->ChatPacket(CHAT_TYPE_INFO, "Invalid argument! (Cell:%d)", Cell);
	}
	else
	{
		int32_t beginPos = 0;
		str_to_number(beginPos, args[0]);
		int32_t endPos = 0;
		str_to_number(endPos, args[1]);
		sys_log(0, "do_drop_item: beginPos: %d, endPos: %d", beginPos, endPos);
		if (beginPos >= 0 && endPos < INVENTORY_MAX_NUM && beginPos < endPos)
		{
			for (int32_t Cell = beginPos; Cell <= endPos; Cell++)
				ch->DropItem(TItemPos(INVENTORY, Cell));
		}
		else
			ch->ChatPacket(CHAT_TYPE_INFO, "Invalid arguments! (beginPos:%d; endPos:%d)", beginPos, endPos);
	}
}


ACMD(do_who)
{
	int32_t iTotal;
	int32_t * paiEmpireUserCount;
	int32_t iLocal;

	DESC_MANAGER::Instance().GetUserCount(iTotal, &paiEmpireUserCount, iLocal);

	ch->ChatPacket(CHAT_TYPE_INFO, "Total [%d] %d / %d / %d (this server %d)", 
			iTotal, paiEmpireUserCount[1], paiEmpireUserCount[2], paiEmpireUserCount[3], iLocal);
}

class user_func
{
	public:
		LPCHARACTER	m_ch;
		static int32_t count;
		static char str[128];
		static int32_t str_len;

		user_func()
			: m_ch(nullptr)
		{}

		void initialize(LPCHARACTER ch)
		{
			m_ch = ch;
			str_len = 0;
			count = 0;
			str[0] = '\0';
		}

		void operator () (LPDESC d)
		{
			if (!d->GetCharacter())
				return;

			int32_t len = snprintf(str + str_len, sizeof(str) - str_len, "%-16s ", d->GetCharacter()->GetName());

			if (len < 0 || len >= (int32_t) sizeof(str) - str_len)
				len = (sizeof(str) - str_len) - 1;

			str_len += len;
			++count;

			if (!(count % 4))
			{
				m_ch->ChatPacket(CHAT_TYPE_INFO, str);

				str[0] = '\0';
				str_len = 0;
			}
		}
};

int32_t	user_func::count = 0;
char user_func::str[128] = { 0, };
int32_t	user_func::str_len = 0;

ACMD(do_user)
{
	const DESC_MANAGER::DESC_SET & c_ref_set = DESC_MANAGER::Instance().GetClientSet();
	user_func func;

	func.initialize(ch);
	std::for_each(c_ref_set.begin(), c_ref_set.end(), func);

	if (func.count % 4)
		ch->ChatPacket(CHAT_TYPE_INFO, func.str);

	ch->ChatPacket(CHAT_TYPE_INFO, "Total %d", func.count);
}

ACMD(do_bg_admin)
{
	char arg1[256], arg2[256], arg3[256];
	argument = two_arguments(argument, arg1, sizeof(arg1), arg2, sizeof(arg2));
	one_argument(argument, arg3, sizeof(arg3));

//	if (!*arg1 || !*arg2 || !*arg3)
//		return;

	int32_t nArg1 = 0;
	str_to_number(nArg1, arg1);
	int32_t nArg2 = 0;
	str_to_number(nArg2, arg2);
	int32_t nArg3 = 0;
	str_to_number(nArg3, arg3);

	switch (nArg1)
	{
		case 1: // force initialize battleground
		{
			CBattlegroundManager::Instance().SetStarted(true);
		} break;
		case 2: // force start event
		{
			if (CBattlegroundManager::Instance().StartEvent(nArg2) == false)
			{
				sys_err("Battleground can not started!");
				return;			
			}
		} break;
		case 3: // force close event
		{
			CBattlegroundManager::Instance().CloseEvent(nArg2);
		} break;
		case 4: // force finalize battleground
		{
			CBattlegroundManager::Instance().Destroy();
		} break;
		default:
			sys_err("Unknown first param: %s", arg1);
			break;
	}
}

ACMD(do_disconnect)
{
	char arg1[256];
	one_argument(argument, arg1, sizeof(arg1));

	if (!*arg1)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, "ex) /dc <player name>");
		return;
	}

	LPDESC d = DESC_MANAGER::Instance().FindByCharacterName(arg1);
	LPCHARACTER	tch = d ? d->GetCharacter() : nullptr;

	if (!tch)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, "%s: no such a player.", arg1);
		return;
	}

	if (tch->GetGMLevel() > ch->GetGMLevel()) 
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("Kendinizden yetkili birini oyundan atamazsiniz."));
		return;
	}

	if (tch == ch)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, "cannot disconnect myself");
		return;
	}
	
	DESC_MANAGER::Instance().DestroyDesc(d);
}

ACMD(do_kill)
{
	char arg1[256];
	one_argument(argument, arg1, sizeof(arg1));

	if (!*arg1)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, "ex) /kill <player name>");
		return;
	}

	LPDESC	d = DESC_MANAGER::Instance().FindByCharacterName(arg1);
	LPCHARACTER tch = d ? d->GetCharacter() : nullptr;

	if (!tch)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, "%s: no such a player", arg1);
		return;
	}

	if (tch->GetGMLevel() > ch->GetGMLevel()) 
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("Kendinizden yetkili birini olduremezsiniz."));
		return;
	}

	tch->Dead();
}

ACMD(do_poison)
{
	char arg1[256];
	one_argument(argument, arg1, sizeof(arg1));

	if (!*arg1)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, "ex) /poison <player name>");
		return;
	}

	LPDESC	d = DESC_MANAGER::Instance().FindByCharacterName(arg1);
	LPCHARACTER tch = d ? d->GetCharacter() : nullptr;

	if (!tch)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, "%s: no such a player", arg1);
		return;
	}

	tch->AttackedByPoison(nullptr);
}

#ifdef ENABLE_WOLFMAN_CHARACTER
ACMD(do_bleeding)
{
	char arg1[256];
	one_argument(argument, arg1, sizeof(arg1));

	if (!*arg1)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, "ex) /bleeding <player name>");
		return;
	}

	LPDESC	d = DESC_MANAGER::Instance().FindByCharacterName(arg1);
	LPCHARACTER tch = d ? d->GetCharacter() : nullptr;

	if (!tch)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, "%s: no such a player", arg1);
		return;
	}

	tch->AttackedByBleeding(nullptr);
}
#endif

#define MISC    0
#define BINARY  1
#define NUMBER  2

namespace DoSetTypes{
typedef enum do_set_types_s {GOLD, RACE, SEX, JOB, EXP, MAX_HP, MAX_SP, SKILL, ALIGNMENT, ALIGN} do_set_types_t;
}

const struct set_struct 
{
	const char *cmd;
	const char type;
	const char * help;
} set_fields[] = {
	{ "gold",		NUMBER,	nullptr	},
#ifdef ENABLE_WOLFMAN_CHARACTER
	{ "race",		NUMBER,	"0. Warrior, 1. Ninja, 2. Sura, 3. Shaman, 4. Lycan"		},
#else
	{ "race",		NUMBER,	"0. Warrior, 1. Ninja, 2. Sura, 3. Shaman"		},
#endif
	{ "sex",		NUMBER,	"0. Male, 1. Female"	},
	{ "job",		NUMBER,	"0. None, 1. First, 2. Second"	},
	{ "exp",		NUMBER,	nullptr	},
	{ "max_hp",		NUMBER,	nullptr	},
	{ "max_sp",		NUMBER,	nullptr	},
	{ "skill",		NUMBER,	nullptr	},
	{ "alignment",	NUMBER,	nullptr	},
	{ "align",		NUMBER,	nullptr	},
	{ "\n",			MISC,	nullptr	}
};

ACMD(do_set)
{
	char arg1[256], arg2[256], arg3[256];

	LPCHARACTER tch = nullptr;

	int32_t i, len;
	const char* line;

	line = two_arguments(argument, arg1, sizeof(arg1), arg2, sizeof(arg2));
	one_argument(line, arg3, sizeof(arg3));

	if (!*arg1 || !*arg2 || !*arg3)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, "Usage: set <name> <field> <value>");
		ch->ChatPacket(CHAT_TYPE_INFO, "List of the fields available:");
		for (i = 0; *(set_fields[i].cmd) != '\n'; i++)
		{
			ch->ChatPacket(CHAT_TYPE_INFO, " %d. %s", i+1, set_fields[i].cmd);
			if (set_fields[i].help != nullptr)
				ch->ChatPacket(CHAT_TYPE_INFO, "  Help: %s", set_fields[i].help);
		}
		return;
	}

	tch = CHARACTER_MANAGER::Instance().FindPC(arg1);

	if (!tch)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, "%s not exist", arg1);
		return;
	}

	if (!g_bIsTestServer && !tch->IsGM())
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("%s GM degil"), arg1);
		return;
	}

	len = strlen(arg2);

	for (i = 0; *(set_fields[i].cmd) != '\n'; i++)
		if (!strncmp(arg2, set_fields[i].cmd, len))
			break;

	switch (i)
	{
		case DoSetTypes::GOLD:	// gold
			{
				int32_t gold = 0;
				str_to_number(gold, arg3);
				LogManager::Instance().MoneyLog(MONEY_LOG_MISC, 3, gold);
				tch->PointChange(POINT_GOLD, gold, true);
			}
			break;

		case DoSetTypes::RACE: // race
			{
				int32_t amount = 0;
				str_to_number(amount, arg3);
				amount = MINMAX(0, amount, JOB_MAX_NUM);
				ESex mySex = GET_SEX(tch);
				uint32_t dwRace = MAIN_RACE_WARRIOR_M;
				switch (amount)
				{
					case JOB_WARRIOR:
						dwRace = (mySex==SEX_MALE)?MAIN_RACE_WARRIOR_M:MAIN_RACE_WARRIOR_W;
						break;
					case JOB_ASSASSIN:
						dwRace = (mySex==SEX_MALE)?MAIN_RACE_ASSASSIN_M:MAIN_RACE_ASSASSIN_W;
						break;
					case JOB_SURA:
						dwRace = (mySex==SEX_MALE)?MAIN_RACE_SURA_M:MAIN_RACE_SURA_W;
						break;
					case JOB_SHAMAN:
						dwRace = (mySex==SEX_MALE)?MAIN_RACE_SHAMAN_M:MAIN_RACE_SHAMAN_W;
						break;
#ifdef ENABLE_WOLFMAN_CHARACTER
					case JOB_WOLFMAN:
						dwRace = (mySex==SEX_MALE)?MAIN_RACE_WOLFMAN_M:MAIN_RACE_WOLFMAN_M;
						break;
#endif
				}
				if (dwRace!=tch->GetRaceNum())
				{
					tch->SetRace(dwRace);
					tch->ClearSkill();
					tch->SetSkillGroup(0);
					// quick mesh change workaround begin
					tch->SetPolymorph(101);
					tch->SetPolymorph(0);
					// quick mesh change workaround end
				}
			}
			break;

		case DoSetTypes::SEX: // sex
			{
				int32_t amount = 0;
				str_to_number(amount, arg3);
				amount = MINMAX(SEX_MALE, amount, SEX_FEMALE);
				if (amount != GET_SEX(tch))
				{
					tch->ChangeSex();
					// quick mesh change workaround begin
					tch->SetPolymorph(101);
					tch->SetPolymorph(0);
					// quick mesh change workaround end
				}
			}
			break;

		case DoSetTypes::JOB: // job
			{
				int32_t amount = 0;
				str_to_number(amount, arg3);
				amount = MINMAX(0, amount, 2);
				if (amount != tch->GetSkillGroup())
				{
					tch->ClearSkill();
					tch->SetSkillGroup(amount);
				}
			}
			break;

		case DoSetTypes::EXP: // exp
			{
				int32_t amount = 0;
				str_to_number(amount, arg3);
				tch->PointChange(POINT_EXP, amount, true);
			}
			break;

		case DoSetTypes::MAX_HP: // max_hp
			{
				int32_t amount = 0;
				str_to_number(amount, arg3);
				tch->PointChange(POINT_MAX_HP, amount, true);
			}
			break;

		case DoSetTypes::MAX_SP: // max_sp
			{
				int32_t amount = 0;
				str_to_number(amount, arg3);
				tch->PointChange(POINT_MAX_SP, amount, true);
			}
			break;

		case DoSetTypes::SKILL: // active skill point
			{
				int32_t amount = 0;
				str_to_number(amount, arg3);
				tch->PointChange(POINT_SKILL, amount, true);
			}
			break;

		case DoSetTypes::ALIGN: // alignment
		case DoSetTypes::ALIGNMENT: // alignment
			{
				int32_t	amount = 0;
				str_to_number(amount, arg3);
				tch->UpdateAlignment(amount - tch->GetRealAlignment());
			}
			break;
	}

	if (set_fields[i].type == NUMBER)
	{
		int32_t	amount = 0;
		str_to_number(amount, arg3);
		ch->ChatPacket(CHAT_TYPE_INFO, "%s's %s set to [%d]", tch->GetName(), set_fields[i].cmd, amount);
	}
}

struct FuncKillAll
{
	LPCHARACTER m_ch;

	FuncKillAll(LPCHARACTER ch) :
		m_ch(ch)
	{
	}

	void operator()(LPENTITY ent)
	{
		if (ent->IsType(ENTITY_CHARACTER))
		{
			LPCHARACTER ch = (LPCHARACTER)ent;

			if (!ch->IsPC() || m_ch == ch || ch->IsGM() || ch->IsDead() || ch->GetHP() <= 0 || ch->GetExchange() || ch->GetMyShop() || ch->IsOpenSafebox() || ch->GetShopOwner() || ch->IsCubeOpen())
				return;

			float fDist = DISTANCE_APPROX(m_ch->GetX() - ch->GetX(), m_ch->GetY() - ch->GetY());
			if (fDist > 7000.f)
				return;

			int32_t damage = ch->GetHP() + number(1, 4250);
			ch->EffectPacket(SE_CRITICAL);
			ch->PointChange(POINT_HP, -damage, false);
			ch->Dead();
		}
	}
};
ACMD(do_kill_all)
{
	LPSECTREE pSec = ch->GetSectree();
	if (pSec)
	{
		FuncKillAll f(ch);
		pSec->ForEachAround(f);
	}
}

ACMD(do_reset)
{
	ch->PointChange(POINT_HP, ch->GetMaxHP() - ch->GetHP());
	ch->PointChange(POINT_SP, ch->GetMaxSP() - ch->GetSP());
	ch->Save();
}

ACMD(do_advance)
{
	char arg1[256], arg2[256];
	two_arguments(argument, arg1, sizeof(arg1), arg2, sizeof(arg2));

	if (!*arg1 || !*arg2)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, "Syntax: advance <name> <level>");
		return;
	}

	LPCHARACTER tch = CHARACTER_MANAGER::Instance().FindPC(arg1);

	if (!tch)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, "%s not exist", arg1);
		return;
	}

	if (!g_bIsTestServer && !tch->IsGM())
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("%s GM degil"), arg1);
		return;
	}

	int32_t level = 0;
	str_to_number(level, arg2);

	tch->ResetPoint(MINMAX(0, level, gPlayerMaxLevel));
}

ACMD(do_respawn)
{
	char arg1[256];
	one_argument(argument, arg1, sizeof(arg1));

	if (*arg1 && !strcasecmp(arg1, "all"))
	{
		ch->ChatPacket(CHAT_TYPE_INFO, "Respaw everywhere");
		regen_reset(0, 0);
	}
	else
	{
		ch->ChatPacket(CHAT_TYPE_INFO, "Respaw around");
		regen_reset(ch->GetX(), ch->GetY());
	}
}

ACMD(do_safebox_size)
{

	char arg1[256];
	one_argument(argument, arg1, sizeof(arg1));

	int32_t size = 0;

	if (*arg1)
		str_to_number(size, arg1);

	if (size > 3 || size < 0)
		size = 0;

	ch->ChatPacket(CHAT_TYPE_INFO, "Safebox size set to %d", size);
	ch->ChangeSafeboxSize(size);
}

ACMD(do_makeguild)
{
	if (ch->GetGuild())
		return;

	CGuildManager& gm = CGuildManager::Instance();

	char arg1[256];
	one_argument(argument, arg1, sizeof(arg1));

	TGuildCreateParameter cp;
	memset(&cp, 0, sizeof(cp));

	cp.master = ch;
	strlcpy(cp.name, arg1, sizeof(cp.name));

	if (!check_name(cp.name))
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("적합하지 않은 길드 이름 입니다."));
		return;
	}

	gm.CreateGuild(cp);
	ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("(%s) 길드가 생성되었습니다. [임시]"), cp.name);
}

ACMD(do_deleteguild)
{
	if (ch->GetGuild())
		ch->GetGuild()->RequestDisband(ch->GetPlayerID());
}

ACMD(do_greset)
{
	if (ch->GetGuild())
		ch->GetGuild()->Reset();
}

// REFINE_ROD_HACK_BUG_FIX
ACMD(do_refine_rod)
{
	char arg1[256];
	one_argument(argument, arg1, sizeof(arg1));

	uint8_t cell = 0;
	str_to_number(cell, arg1);
	LPITEM item = ch->GetInventoryItem(cell);
	if (item)
		fishing::RealRefineRod(ch, item);
}
// END_OF_REFINE_ROD_HACK_BUG_FIX

// REFINE_PICK
ACMD(do_refine_pick)
{
	char arg1[256];
	one_argument(argument, arg1, sizeof(arg1));

	uint8_t cell = 0;
	str_to_number(cell, arg1);
	LPITEM item = ch->GetInventoryItem(cell);
	if (item)
	{
		mining::CHEAT_MAX_PICK(ch, item);
		mining::RealRefinePick(ch, item);
	}
}

ACMD(do_max_pick)
{
	char arg1[256];
	one_argument(argument, arg1, sizeof(arg1));

	uint8_t cell = 0;
	str_to_number(cell, arg1);
	LPITEM item = ch->GetInventoryItem(cell);
	if (item)
	{
		mining::CHEAT_MAX_PICK(ch, item);
	}
}
// END_OF_REFINE_PICK


ACMD(do_fishing_simul)
{
	char arg1[256];
	char arg2[256];
	char arg3[256];
	argument = one_argument(argument, arg1, sizeof(arg1));
	two_arguments(argument, arg2, sizeof(arg2), arg3, sizeof(arg3));

	int32_t count = 1000;
	int32_t prob_idx = 0;
	int32_t level = 100;

	ch->ChatPacket(CHAT_TYPE_INFO, "Usage: fishing_simul <level> <prob index> <count>");

	if (*arg1)
		str_to_number(level, arg1);

	if (*arg2)
		str_to_number(prob_idx, arg2);

	if (*arg3)
		str_to_number(count, arg3);

	fishing::Simulation(level, count, prob_idx, ch);
}

ACMD(do_invisibility)
{
	ch->SetGMInvisible(!ch->IsGMInvisible());
}

ACMD(do_event_flag)
{
	char arg1[256];
	char arg2[256];

	two_arguments(argument, arg1, sizeof(arg1), arg2, sizeof(arg2));

	if (!(*arg1) || !(*arg2))
		return;

	int32_t value = 0;
	str_to_number(value, arg2);

	if (!strcmp(arg1, "mob_item") || 
			!strcmp(arg1, "mob_exp") ||
			!strcmp(arg1, "mob_gold") ||
			!strcmp(arg1, "mob_dam") ||
			!strcmp(arg1, "mob_gold_pct") ||
			!strcmp(arg1, "mob_item_buyer") || 
			!strcmp(arg1, "mob_exp_buyer") ||
			!strcmp(arg1, "mob_gold_buyer") ||
			!strcmp(arg1, "mob_gold_pct_buyer")
	   )
		value = MINMAX(0, value, EVENT_MOB_RATE_LIMIT);

	//quest::CQuestManager::Instance().SetEventFlag(arg1, atoi(arg2));
	quest::CQuestManager::Instance().RequestSetEventFlag(arg1, value);
	ch->ChatPacket(CHAT_TYPE_INFO, "RequestSetEventFlag %s %d", arg1, value);
	sys_log(0, "RequestSetEventFlag %s %d", arg1, value);
}

ACMD(do_get_event_flag)
{
	// Filter
	char arg1[256];
	one_argument(argument, arg1, sizeof(arg1));

	quest::CQuestManager::Instance().SendEventFlagList(ch, arg1);
}

ACMD(do_private)
{
	char arg1[256];
	one_argument(argument, arg1, sizeof(arg1));

	if (!*arg1)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, "Usage: private <map index>");
		return;
	}

	int32_t lMapIndex;
	int32_t map_index = 0;
	str_to_number(map_index, arg1);
	if ((lMapIndex = SECTREE_MANAGER::Instance().CreatePrivateMap(map_index)))
	{
		ch->SaveExitLocation();

		LPSECTREE_MAP pkSectreeMap = SECTREE_MANAGER::Instance().GetMap(lMapIndex);
		ch->WarpSet(pkSectreeMap->m_setting.posSpawn.x, pkSectreeMap->m_setting.posSpawn.y, lMapIndex); 
	}
	else
		ch->ChatPacket(CHAT_TYPE_INFO, "Can't find map by index %d", map_index);
}

ACMD(do_qf)
{
	char arg1[256];

	one_argument(argument, arg1, sizeof(arg1));

	if (!*arg1)
		return;

	quest::PC* pPC = quest::CQuestManager::Instance().GetPCForce(ch->GetPlayerID());
	std::string questname = pPC->GetCurrentQuestName();

	if (!questname.empty())
	{
		int32_t value = quest::CQuestManager::Instance().GetQuestStateIndex(questname, arg1);

		pPC->SetFlag(questname + ".__status", value);
		pPC->ClearTimer();

		quest::PC::QuestInfoIterator it = pPC->quest_begin();
		uint32_t questindex = quest::CQuestManager::Instance().GetQuestIndexByName(questname);

		while (it!= pPC->quest_end())
		{
			if (it->first == questindex)
			{
				it->second.st = value;
				break;
			}

			++it;
		}

		ch->ChatPacket(CHAT_TYPE_INFO, "setting quest state flag %s %s %d", questname.c_str(), arg1, value);
	}
	else
	{
		ch->ChatPacket(CHAT_TYPE_INFO, "setting quest state flag failed");
	}
}

LPCHARACTER chHori, chForge, chLib, chTemple, chTraining, chTree, chPortal, chBall;

ACMD(do_b1)
{
	//호리병 478 579
	chHori = CHARACTER_MANAGER::Instance().SpawnMobRange(14017, ch->GetMapIndex(), 304222, 742858, 304222, 742858, true, false);
	chHori->AddAffect(AFFECT_DUNGEON_UNIQUE, POINT_NONE, 0, AFF_BUILDING_CONSTRUCTION_SMALL, 65535, 0, true);
	chHori->AddAffect(AFFECT_DUNGEON_UNIQUE, POINT_NONE, 0, AFF_DUNGEON_UNIQUE, 65535, 0, true);

	for (int32_t i = 0; i < 30; ++i)
	{
		int32_t rot = number(0, 359);
		float fx, fy;
		GetDeltaByDegree(rot, 800, &fx, &fy);

		LPCHARACTER tch = CHARACTER_MANAGER::Instance().SpawnMobRange(number(701, 706), 
				ch->GetMapIndex(),
				304222 + (int32_t)fx,
				742858 + (int32_t)fy,
				304222 + (int32_t)fx,
				742858 + (int32_t)fy,
				true,
				false);
		tch->SetAggressive();
	}

	for (int32_t i = 0; i < 5; ++i)
	{
		int32_t rot = number(0, 359);
		float fx, fy;
		GetDeltaByDegree(rot, 800, &fx, &fy);

		LPCHARACTER tch = CHARACTER_MANAGER::Instance().SpawnMobRange(8009, 
				ch->GetMapIndex(),
				304222 + (int32_t)fx,
				742858 + (int32_t)fy,
				304222 + (int32_t)fx,
				742858 + (int32_t)fy,
				true,
				false);
		tch->SetAggressive();
	}
}

ACMD(do_b2)
{
	chHori->RemoveAffect(AFFECT_DUNGEON_UNIQUE);
}

ACMD(do_b3)
{
	// 포지 492 547
	chForge = CHARACTER_MANAGER::Instance().SpawnMobRange(14003, ch->GetMapIndex(), 307500, 746300, 307500, 746300, true, false);
	chForge->AddAffect(AFFECT_DUNGEON_UNIQUE, POINT_NONE, 0, AFF_DUNGEON_UNIQUE, 65535, 0, true);
	//높은탑 509 589 -> 도서관
	chLib = CHARACTER_MANAGER::Instance().SpawnMobRange(14007, ch->GetMapIndex(), 307900, 744500, 307900, 744500, true, false);
	chLib->AddAffect(AFFECT_DUNGEON_UNIQUE, POINT_NONE, 0, AFF_DUNGEON_UNIQUE, 65535, 0, true);
	//욕조 513 606 -> 힘의신전
	chTemple = CHARACTER_MANAGER::Instance().SpawnMobRange(14004, ch->GetMapIndex(), 307700, 741600, 307700, 741600, true, false);
	chTemple->AddAffect(AFFECT_DUNGEON_UNIQUE, POINT_NONE, 0, AFF_DUNGEON_UNIQUE, 65535, 0, true);
	//권투장 490 625
	chTraining= CHARACTER_MANAGER::Instance().SpawnMobRange(14010, ch->GetMapIndex(), 307100, 739500, 307100, 739500, true, false);
	chTraining->AddAffect(AFFECT_DUNGEON_UNIQUE, POINT_NONE, 0, AFF_DUNGEON_UNIQUE, 65535, 0, true);
	//나무 466 614
	chTree= CHARACTER_MANAGER::Instance().SpawnMobRange(14013, ch->GetMapIndex(), 300800, 741600, 300800, 741600, true, false);
	chTree->AddAffect(AFFECT_DUNGEON_UNIQUE, POINT_NONE, 0, AFF_DUNGEON_UNIQUE, 65535, 0, true);
	//포탈 439 615
	chPortal= CHARACTER_MANAGER::Instance().SpawnMobRange(14001, ch->GetMapIndex(), 300900, 744500, 300900, 744500, true, false);
	chPortal->AddAffect(AFFECT_DUNGEON_UNIQUE, POINT_NONE, 0, AFF_DUNGEON_UNIQUE, 65535, 0, true);
	// 구슬 436 600
	chBall = CHARACTER_MANAGER::Instance().SpawnMobRange(14012, ch->GetMapIndex(), 302500, 746600, 302500, 746600, true, false);
	chBall->AddAffect(AFFECT_DUNGEON_UNIQUE, POINT_NONE, 0, AFF_DUNGEON_UNIQUE, 65535, 0, true);
}

ACMD(do_b4)
{
	chLib->AddAffect(AFFECT_DUNGEON_UNIQUE, POINT_NONE, 0, AFF_BUILDING_UPGRADE, 65535, 0, true);

	for (int32_t i = 0; i < 30; ++i)
	{
		int32_t rot = number(0, 359);
		float fx, fy;
		GetDeltaByDegree(rot, 1200, &fx, &fy);

		LPCHARACTER tch = CHARACTER_MANAGER::Instance().SpawnMobRange(number(701, 706), 
				ch->GetMapIndex(),
				307900 + (int32_t)fx,
				744500 + (int32_t)fy,
				307900 + (int32_t)fx,
				744500 + (int32_t)fy,
				true,
				false);
		tch->SetAggressive();
	}

	for (int32_t i = 0; i < 5; ++i)
	{
		int32_t rot = number(0, 359);
		float fx, fy;
		GetDeltaByDegree(rot, 1200, &fx, &fy);

		LPCHARACTER tch = CHARACTER_MANAGER::Instance().SpawnMobRange(8009, 
				ch->GetMapIndex(),
				307900 + (int32_t)fx,
				744500 + (int32_t)fy,
				307900 + (int32_t)fx,
				744500 + (int32_t)fy,
				true,
				false);
		tch->SetAggressive();
	}

}

ACMD(do_b5)
{
	M2_DESTROY_CHARACTER(chLib);
	//chHori->RemoveAffect(AFFECT_DUNGEON_UNIQUE);
	chLib = CHARACTER_MANAGER::Instance().SpawnMobRange(14008, ch->GetMapIndex(), 307900, 744500, 307900, 744500, true, false);
	chLib->AddAffect(AFFECT_DUNGEON_UNIQUE, POINT_NONE, 0, AFF_DUNGEON_UNIQUE, 65535, 0, true);
}

ACMD(do_b6)
{
	chLib->AddAffect(AFFECT_DUNGEON_UNIQUE, POINT_NONE, 0, AFF_BUILDING_UPGRADE, 65535, 0, true);
}
ACMD(do_b7)
{
	M2_DESTROY_CHARACTER(chLib);
	//chHori->RemoveAffect(AFFECT_DUNGEON_UNIQUE);
	chLib = CHARACTER_MANAGER::Instance().SpawnMobRange(14009, ch->GetMapIndex(), 307900, 744500, 307900, 744500, true, false);
	chLib->AddAffect(AFFECT_DUNGEON_UNIQUE, POINT_NONE, 0, AFF_DUNGEON_UNIQUE, 65535, 0, true);
}

ACMD(do_book)
{
	char arg1[256];

	one_argument(argument, arg1, sizeof(arg1));

	CSkillProto * pkProto;

	if (isdigit(*arg1))
	{
		uint32_t vnum = 0;
		str_to_number(vnum, arg1);
		pkProto = CSkillManager::Instance().Get(vnum);
	}
	else 
		pkProto = CSkillManager::Instance().Get(arg1);

	if (!pkProto)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, "There is no such a skill.");
		return;
	}

	LPITEM item = ch->AutoGiveItem(50300);
	item->SetSocket(0, pkProto->dwVnum);
}

ACMD(do_setskillother)
{
	char arg1[256], arg2[256], arg3[256];
	argument = two_arguments(argument, arg1, sizeof(arg1), arg2, sizeof(arg2));
	one_argument(argument, arg3, sizeof(arg3));

	if (!*arg1 || !*arg2 || !*arg3 || !isdigit(*arg3))
	{
		ch->ChatPacket(CHAT_TYPE_INFO, "Syntax: setskillother <target> <skillname> <lev>");
		return;
	}

	LPCHARACTER tch;

	tch = CHARACTER_MANAGER::Instance().FindPC(arg1);

	if (!tch)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, "There is no such character.");
		return;
	}

	if (!g_bIsTestServer && !tch->IsGM())
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("%s GM degil"), arg1);
		return;
	}

	CSkillProto * pk;

	if (isdigit(*arg2))
	{
		uint32_t vnum = 0;
		str_to_number(vnum, arg2);
		pk = CSkillManager::Instance().Get(vnum);
	}
	else
		pk = CSkillManager::Instance().Get(arg2);

	if (!pk)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, "No such a skill by that name.");
		return;
	}

	uint8_t level = 0;
	str_to_number(level, arg3);
	tch->SetSkillLevel(pk->dwVnum, level);
	tch->ComputePoints();
	tch->SkillLevelPacket();
}

ACMD(do_setskill)
{
	char arg1[256], arg2[256];
	two_arguments(argument, arg1, sizeof(arg1), arg2, sizeof(arg2));

	if (!*arg1 || !*arg2 || !isdigit(*arg2))
	{
		ch->ChatPacket(CHAT_TYPE_INFO, "Syntax: setskill <name> <lev>");
		return;
	}

	CSkillProto * pk;

	if (isdigit(*arg1))
	{
		uint32_t vnum = 0;
		str_to_number(vnum, arg1);
		pk = CSkillManager::Instance().Get(vnum);
	}

	else
		pk = CSkillManager::Instance().Get(arg1);

	if (!pk)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, "No such a skill by that name.");
		return;
	}

	uint8_t level = 0;
	str_to_number(level, arg2);
	ch->SetSkillLevel(pk->dwVnum, level);
	ch->ComputePoints();
	ch->SkillLevelPacket();
}

ACMD(do_set_skill_point)
{
	char arg1[256];
	one_argument(argument, arg1, sizeof(arg1));

	int32_t skill_point = 0;
	if (*arg1)
		str_to_number(skill_point, arg1);

	ch->SetRealPoint(POINT_SKILL, skill_point);
	ch->SetPoint(POINT_SKILL, ch->GetRealPoint(POINT_SKILL));
	ch->PointChange(POINT_SKILL, 0);
}

ACMD(do_set_skill_group)
{
	char arg1[256];
	one_argument(argument, arg1, sizeof(arg1));

	int32_t skill_group = 0;
	if (*arg1)
		str_to_number(skill_group, arg1);

	ch->SetSkillGroup(skill_group);
	
	ch->ClearSkill();
	ch->ChatPacket(CHAT_TYPE_INFO, "skill group to %d.", skill_group);
}

ACMD(do_reload)
{
	char arg1[256];
	one_argument(argument, arg1, sizeof(arg1));

	TPacketGGReloadCommand p2p_packet;
	p2p_packet.header = HEADER_GG_RELOAD_COMMAND;
	strlcpy(p2p_packet.argument, arg1, sizeof(p2p_packet.argument));
	bool bSendP2P = false;

	if (*arg1)
	{
		switch (LOWER(*arg1))
		{
			case 'p':
				if (ch)
					ch->ChatPacket(CHAT_TYPE_INFO, "Reloading prototype tables,");
				db_clientdesc->DBPacket(HEADER_GD_RELOAD_PROTO, 0, nullptr, 0);
				break;

			case 'q':
				if (ch)
					ch->ChatPacket(CHAT_TYPE_INFO, "Reloading quest.");
				quest::CQuestManager::Instance().Reload();
				bSendP2P = true;
				break;

			case 'f':
				fishing::Initialize();
				if (ch)
					ch->ChatPacket(CHAT_TYPE_INFO, "Reloading fishing infomation.");
				bSendP2P = true;
				break;

				//RELOAD_ADMIN
			case 'a':
				if (ch)
					ch->ChatPacket(CHAT_TYPE_INFO, "Reloading Admin infomation.");

				TPacketReloadAdmin pack;
				strlcpy(pack.szIP, g_szPublicIP, sizeof(pack.szIP));
				db_clientdesc->DBPacket(HEADER_GD_RELOAD_ADMIN, 0, &pack, sizeof(TPacketReloadAdmin));

				sys_log(0, "Reloading admin infomation.");
				break;
				//END_RELOAD_ADMIN

			case 'c':	// cube
				if (ch)
					ch->ChatPacket(CHAT_TYPE_INFO, "Reloading cube infomation.");
				Cube_init();
				bSendP2P = true;
				break;

			case 'm':
				char szMOBDropItemFileName[256];
				snprintf(szMOBDropItemFileName, sizeof(szMOBDropItemFileName),
					"%s/mob_drop_item.txt", LocaleService_GetBasePath().c_str());

				ITEM_MANAGER::Instance().DestroyMobDropItem();


				if (!ITEM_MANAGER::Instance().ReadMonsterDropItemGroup(szMOBDropItemFileName))
				{
					ch->ChatPacket(CHAT_TYPE_INFO, "Mob_drop_item.txt yenilenemiyor.");
					return;
				}
				ch->ChatPacket(CHAT_TYPE_INFO, "Mob_drop_item.txt yenilendi!");
				bSendP2P = true;
				break;
				
			case 'x':
				ch->ChatPacket(CHAT_TYPE_INFO, "Reloading anticheat blacklist.");
				if (CAnticheatManager::Instance().ReloadCheatBlacklists() == false)
				{
					ch->ChatPacket(CHAT_TYPE_INFO, "Anticheat blacklist reload fail.");
					return;
				}
				bSendP2P = true;
				break;
		}
	}
	else
	{
		if (ch)
			ch->ChatPacket(CHAT_TYPE_INFO, "Reloading state_user_count.");

		if (ch)
			ch->ChatPacket(CHAT_TYPE_INFO, "Reloading prototype tables,");
		db_clientdesc->DBPacket(HEADER_GD_RELOAD_PROTO, 0, nullptr, 0);
	}

	if (ch && bSendP2P)
	{
		P2P_MANAGER::Instance().Send(&p2p_packet, sizeof(p2p_packet));
		ch->ChatPacket(CHAT_TYPE_INFO, "Reloading other cores / channels.");
	}
}

ACMD(do_cooltime)
{
	ch->DisableCooltime();
}

ACMD(do_level)
{
	char arg2[256];
	one_argument(argument, arg2, sizeof(arg2));

	if (!*arg2)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, "Syntax: level <level>");
		return;
	}

	int32_t	level = 0;
	str_to_number(level, arg2);

	ch->ResetPoint(MINMAX(1, level, gPlayerMaxLevel));

	ch->ClearSkill();
	ch->ClearSubSkill();
}

ACMD(do_gwlist)
{
	ch->ChatPacket(CHAT_TYPE_NOTICE, LC_TEXT("현재 전쟁중인 길드 입니다"));
	CGuildManager::Instance().ShowGuildWarList(ch);
}

ACMD(do_stop_guild_war)
{
	char arg1[256], arg2[256];
	two_arguments(argument, arg1, sizeof(arg1), arg2, sizeof(arg2));

	if (!*arg1 || !*arg2)
		return;

	int32_t id1 = 0, id2 = 0;

	str_to_number(id1, arg1);
	str_to_number(id2, arg2);
	
	if (!id1 || !id2)
		return;

	if (id1 > id2)
	{
		std::swap(id1, id2);
	}

	ch->ChatPacket(CHAT_TYPE_TALKING, "%d %d", id1, id2);
	CGuildManager::Instance().RequestEndWar(id1, id2);
}

ACMD(do_cancel_guild_war)
{
	char arg1[256], arg2[256];
	two_arguments(argument, arg1, sizeof(arg1), arg2, sizeof(arg2));

	int32_t id1 = 0, id2 = 0;
	str_to_number(id1, arg1);
	str_to_number(id2, arg2);

	if (id1 > id2)
		std::swap(id1, id2);

	CGuildManager::Instance().RequestCancelWar(id1, id2);
}

ACMD(do_guild_state)
{
	char arg1[256];
	one_argument(argument, arg1, sizeof(arg1));

	CGuild* pGuild = CGuildManager::Instance().FindGuildByName(arg1);
	if (pGuild != nullptr)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, "GuildID: %d", pGuild->GetID());
		ch->ChatPacket(CHAT_TYPE_INFO, "GuildMasterPID: %d", pGuild->GetMasterPID());
		ch->ChatPacket(CHAT_TYPE_INFO, "IsInWar: %d", pGuild->UnderAnyWar());
	}
	else
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("%s: 존재하지 않는 길드 입니다."), arg1);
	}
}

struct FuncWeaken
{
	LPCHARACTER m_pkGM;
	bool	m_bAll;

	FuncWeaken(LPCHARACTER ch) : m_pkGM(ch), m_bAll(false)
	{
	}

	void operator () (LPENTITY ent)
	{
		if (!ent->IsType(ENTITY_CHARACTER))
			return;

		LPCHARACTER pkChr = (LPCHARACTER) ent;

		int32_t iDist = DISTANCE_APPROX(pkChr->GetX() - m_pkGM->GetX(), pkChr->GetY() - m_pkGM->GetY());

		if (!m_bAll && iDist >= 1000)	// 10미터 이상에 있는 것들은 purge 하지 않는다.
			return;

		if (pkChr->IsNPC())
			pkChr->PointChange(POINT_HP, (10 - pkChr->GetHP()));
	}
};

ACMD(do_weaken)
{
	char arg1[256];
	one_argument(argument, arg1, sizeof(arg1));

	FuncWeaken func(ch);

	if (*arg1 && !strcmp(arg1, "all"))
		func.m_bAll = true;

	ch->GetSectree()->ForEachAround(func);
}

ACMD(do_getqf)
{
	char arg1[256];

	one_argument(argument, arg1, sizeof(arg1));

	LPCHARACTER tch;

	if (!*arg1)
		tch = ch;
	else
	{
		tch = CHARACTER_MANAGER::Instance().FindPC(arg1);

		if (!tch)
		{
			ch->ChatPacket(CHAT_TYPE_INFO, "There is no such character.");
			return;
		}
	}

	quest::PC* pPC = quest::CQuestManager::Instance().GetPC(tch->GetPlayerID());

	if (pPC)
		pPC->SendFlagList(ch);
}

ACMD(do_set_state)
{
	char arg1[256];
	char arg2[256];

	two_arguments(argument, arg1, sizeof(arg1), arg2, sizeof(arg2));

	if (!*arg1 || !*arg2)
	{
		ch->ChatPacket(CHAT_TYPE_INFO,
			"Syntax: set_state <questname> <statename>"
			" [<character name>]"
		);
		return;
	}

	LPCHARACTER tch = ch;
	char arg3[256];
	argument = one_argument(argument, arg3, sizeof(arg3));
	if (*arg3)
	{
		tch = CHARACTER_MANAGER::Instance().FindPC(arg3);
		if (!tch)
		{
			ch->ChatPacket(CHAT_TYPE_INFO, "There is no such character.");
			return;
		}
	}
	quest::PC* pPC = quest::CQuestManager::Instance().GetPCForce(tch->GetPlayerID());

	std::string questname = arg1;
	std::string statename = arg2;

	if (!questname.empty())
	{
		int32_t value = quest::CQuestManager::Instance().GetQuestStateIndex(questname, statename);

		pPC->SetFlag(questname + ".__status", value);
		pPC->ClearTimer();

		quest::PC::QuestInfoIterator it = pPC->quest_begin();
		uint32_t questindex = quest::CQuestManager::Instance().GetQuestIndexByName(questname);

		while (it!= pPC->quest_end())
		{
			if (it->first == questindex)
			{
				it->second.st = value;
				break;
			}

			++it;
		}

		ch->ChatPacket(CHAT_TYPE_INFO, "setting quest state flag %s %s %d", questname.c_str(), arg1, value);
	}
	else
	{
		ch->ChatPacket(CHAT_TYPE_INFO, "setting quest state flag failed");
	}
}

ACMD(do_setqf)
{
	char arg1[256];
	char arg2[256];
	char arg3[256];

	one_argument(two_arguments(argument, arg1, sizeof(arg1), arg2, sizeof(arg2)), arg3, sizeof(arg3));

	if (!*arg1)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, "Syntax: setqf <flagname> <value> [<character name>]");
		return;
	}

	LPCHARACTER tch = ch;

	if (*arg3)
		tch = CHARACTER_MANAGER::Instance().FindPC(arg3);

	if (!tch)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, "There is no such character.");
		return;
	}

	quest::PC* pPC = quest::CQuestManager::Instance().GetPC(tch->GetPlayerID());

	if (pPC)
	{
		int32_t value = 0;
		str_to_number(value, arg2);
		pPC->SetFlag(arg1, value);
		ch->ChatPacket(CHAT_TYPE_INFO, "Quest flag set: %s %d", arg1, value);
	}
}

ACMD(do_delqf)
{
	char arg1[256];
	char arg2[256];

	two_arguments(argument, arg1, sizeof(arg1), arg2, sizeof(arg2));

	if (!*arg1)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, "Syntax: delqf <flagname> [<character name>]");
		return;
	}

	LPCHARACTER tch = ch;

	if (*arg2)
		tch = CHARACTER_MANAGER::Instance().FindPC(arg2);

	if (!tch)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, "There is no such character.");
		return;
	}

	quest::PC* pPC = quest::CQuestManager::Instance().GetPC(tch->GetPlayerID());

	if (pPC)
	{
		if (pPC->DeleteFlag(arg1))
			ch->ChatPacket(CHAT_TYPE_INFO, "Delete success.");
		else
			ch->ChatPacket(CHAT_TYPE_INFO, "Delete failed. Quest flag does not exist.");
	}
}

ACMD(do_forgetme)
{
	ch->ForgetMyAttacker();
}

ACMD(do_aggregate)
{
	ch->AggregateMonster();
}

ACMD(do_attract_ranger)
{
	ch->AttractRanger();
}

ACMD(do_pull_monster)
{
	ch->PullMonster();
}

ACMD(do_polymorph)
{
	char arg1[256], arg2[256];

	two_arguments(argument, arg1, sizeof(arg1), arg2, sizeof(arg2));
	if (*arg1)
	{
		uint32_t dwVnum = 0;
		str_to_number(dwVnum, arg1);
		bool bMaintainStat = false;
		if (*arg2)
		{
			int32_t value = 0;
			str_to_number(value, arg2);
			bMaintainStat = (value>0);
		}

		ch->SetPolymorph(dwVnum, bMaintainStat);
	}
}

ACMD(do_polymorph_item)
{
	char arg1[256];

	one_argument(argument, arg1, sizeof(arg1));

	if (*arg1)
	{
		uint32_t dwVnum = 0;
		str_to_number(dwVnum, arg1);

		LPITEM item = ITEM_MANAGER::Instance().CreateItem(70104, 1, 0, true);
		if (item)
		{
			item->SetSocket(0, dwVnum);
			int32_t iEmptyPos = ch->GetEmptyInventory(item->GetSize());

			if (iEmptyPos != -1)
			{
				item->AddToCharacter(ch, TItemPos(INVENTORY, iEmptyPos));
				LogManager::Instance().ItemLog(ch, item, "GM", item->GetName());
			}
			else
			{
				M2_DESTROY_ITEM(item);
				ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("Not enough inventory space."));
			}
		}
		else
		{
			ch->ChatPacket(CHAT_TYPE_INFO, "#%d item not exist by that vnum.", 70103);
		}
		//ch->SetPolymorph(dwVnum, bMaintainStat);
	}
}

ACMD(do_priv_empire)
{
	char arg1[256] = {0};
	char arg2[256] = {0};
	char arg3[256] = {0};
	char arg4[256] = {0};
	int32_t empire = 0;
	int32_t type = 0;
	int32_t value = 0;
	int32_t duration = 0;

	const char* line = two_arguments(argument, arg1, sizeof(arg1), arg2, sizeof(arg2));

	if (!*arg1 || !*arg2)
		goto USAGE;

	if (!line)
		goto USAGE;

	two_arguments(line, arg3, sizeof(arg3), arg4, sizeof(arg4));

	if (!*arg3 || !*arg4)
		goto USAGE;

	str_to_number(empire, arg1);
	str_to_number(type,	arg2);
	str_to_number(value,	arg3);
	value = MINMAX(0, value, PRIV_EMPIRE_RATE_LIMIT);
	str_to_number(duration, arg4);

	if (empire < 0 || 3 < empire)
		goto USAGE;

	if (type < 1 || 4 < type)
		goto USAGE;

	if (value < 0)
		goto USAGE;

	if (duration < 0)
		goto USAGE;

	// 시간 단위로 변경
	duration = duration * (60*60);

	sys_log(0, "_give_empire_privileage(empire=%d, type=%d, value=%d, duration=%d) by command", 
			empire, type, value, duration);
	CPrivManager::Instance().RequestGiveEmpirePriv(empire, type, value, duration);
	return;

USAGE:
	ch->ChatPacket(CHAT_TYPE_INFO, "usage : priv_empire <empire> <type> <value> <duration>");
	ch->ChatPacket(CHAT_TYPE_INFO, "  <empire>    0 - 3 (0==all)");
	ch->ChatPacket(CHAT_TYPE_INFO, "  <type>      1:item_drop, 2:gold_drop, 3:gold10_drop, 4:exp");
	ch->ChatPacket(CHAT_TYPE_INFO, "  <value>     percent");
	ch->ChatPacket(CHAT_TYPE_INFO, "  <duration>  hour");
}

/**
 * @version 05/06/08	Bang2ni - 길드 보너스 퀘스트 진행 안되는 문제 수정.(스크립트가 작성안됨.)
 * 			          quest/priv_guild.quest 로 부터 스크립트 읽어오게 수정됨
 */
ACMD(do_priv_guild)
{
	char arg1[256];
	one_argument(argument, arg1, sizeof(arg1));

	if (*arg1)
	{
		CGuild * g = CGuildManager::Instance().FindGuildByName(arg1);

		if (!g)
		{
			uint32_t guild_id = 0;
			str_to_number(guild_id, arg1);
			g = CGuildManager::Instance().FindGuild(guild_id);
		}

		if (!g)
			ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("그런 이름 또는 번호의 길드가 없습니다."));
		else
		{
			char buf[1024+1];
			snprintf(buf, sizeof(buf), "%u", g->GetID());

			using namespace quest;
			auto pc = CQuestManager::Instance().GetPC(ch->GetPlayerID());
			QuestState qs = CQuestManager::Instance().OpenState("ADMIN_QUEST", QUEST_FISH_REFINE_STATE_INDEX);
			luaL_loadbuffer(qs.co, buf, strlen(buf), "ADMIN_QUEST");
			pc->SetQuest("ADMIN_QUEST", qs);

			QuestState & rqs = *pc->GetRunningQuestState();

			if (!CQuestManager::Instance().RunState(rqs))
			{
				CQuestManager::Instance().CloseState(rqs);
				pc->EndRunning();
				return;
			}
		}
	}
}

ACMD(do_mount_test)
{
	char arg1[256];

	one_argument(argument, arg1, sizeof(arg1));

	if (*arg1)
	{
		uint32_t vnum = 0;
		str_to_number(vnum, arg1);
		ch->MountVnum(vnum);
	}
}

ACMD(do_observer)
{
	ch->SetObserverMode(!ch->IsObserverMode());
}

ACMD(do_socket_item)
{
	char arg1[256], arg2[256];
	two_arguments(argument, arg1, sizeof(arg1), arg2, sizeof(arg2));

	if (*arg1)
	{
		uint32_t dwVnum = 0;
		str_to_number(dwVnum, arg1);
	
		int32_t iSocketCount = 0;
		str_to_number(iSocketCount, arg2);
	
		if (!iSocketCount || iSocketCount >= ITEM_SOCKET_MAX_NUM)
			iSocketCount = 3;
	
		if (!dwVnum)
		{
			if (!ITEM_MANAGER::Instance().GetVnum(arg1, dwVnum))
			{
				ch->ChatPacket(CHAT_TYPE_INFO, "#%d item not exist by that vnum.", dwVnum);
				return;
			}
		}

		LPITEM item = ch->AutoGiveItem(dwVnum);
	
		if (item)
		{
			for (int32_t i = 0; i < iSocketCount; ++i)
				item->SetSocket(i, 1);
		}
		else
		{
			ch->ChatPacket(CHAT_TYPE_INFO, "#%d cannot create item.", dwVnum);
		}
	}
}

ACMD(do_xmas)
{
	char arg1[256];
	one_argument(argument, arg1, sizeof(arg1));

	int32_t flag = 0;

	if (*arg1)
		str_to_number(flag, arg1);

	switch (subcmd)
	{
		case SCMD_XMAS_SNOW:
			quest::CQuestManager::Instance().RequestSetEventFlag("xmas_snow", flag);
			break;

		case SCMD_XMAS_BOOM:
			quest::CQuestManager::Instance().RequestSetEventFlag("xmas_boom", flag);
			break;

		case SCMD_XMAS_SANTA:
			quest::CQuestManager::Instance().RequestSetEventFlag("xmas_santa", flag);
			break;
	}
}


// BLOCK_CHAT
ACMD(do_block_chat_list)
{
	// GM이 아니거나 block_chat_privilege가 없는 사람은 명령어 사용 불가
	if (!ch || (ch->GetGMLevel() < GM_HIGH_WIZARD && ch->GetQuestFlag("chat_privilege.block") <= 0))
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("그런 명령어는 없습니다"));
		return;
	}

	DBManager::Instance().ReturnQuery(QID_BLOCK_CHAT_LIST, ch->GetPlayerID(), nullptr, 
			"SELECT p.name, a.lDuration FROM affect as a, player as p WHERE a.bType = %d AND a.dwPID = p.id",
			AFFECT_BLOCK_CHAT);
}

ACMD(do_block_chat)
{
	// GM이 아니거나 block_chat_privilege가 없는 사람은 명령어 사용 불가
	if (ch && (ch->GetGMLevel() < GM_HIGH_WIZARD && ch->GetQuestFlag("chat_privilege.block") <= 0))
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("그런 명령어는 없습니다"));
		return;
	}

	char arg1[256];
	argument = one_argument(argument, arg1, sizeof(arg1));

	if (!*arg1)
	{
		if (ch)
			ch->ChatPacket(CHAT_TYPE_INFO, "Usage: block_chat <name> <time> (0 to off)");

		return;
	}

	const char* name = arg1;
	int32_t lBlockDuration = parse_time_str(argument);

	if (lBlockDuration < 0)
	{
		if (ch)
		{
			ch->ChatPacket(CHAT_TYPE_INFO, "잘못된 형식의 시간입니다. h, m, s를 붙여서 지정해 주십시오.");
			ch->ChatPacket(CHAT_TYPE_INFO, "예) 10s, 10m, 1m 30s");
		}
		return;
	}

	sys_log(0, "BLOCK CHAT %s %d", name, lBlockDuration);

	LPCHARACTER tch = CHARACTER_MANAGER::Instance().FindPC(name);

	if (!tch)
	{
		CCI * pkCCI = P2P_MANAGER::Instance().Find(name);

		if (pkCCI)
		{
			TPacketGGBlockChat p;

			p.bHeader = HEADER_GG_BLOCK_CHAT;
			strlcpy(p.szName, name, sizeof(p.szName));
			p.lBlockDuration = lBlockDuration;
			P2P_MANAGER::Instance().Send(&p, sizeof(TPacketGGBlockChat));
		}
		else
		{
			TPacketBlockChat p;

			strlcpy(p.szName, name, sizeof(p.szName));
			p.lDuration = lBlockDuration;
			db_clientdesc->DBPacket(HEADER_GD_BLOCK_CHAT, ch ? ch->GetDesc()->GetHandle() : 0, &p, sizeof(p));
		}

		if (ch)
			ch->ChatPacket(CHAT_TYPE_INFO, "Chat block requested.");

		return;
	}

	if (tch && ch != tch)
		tch->AddAffect(AFFECT_BLOCK_CHAT, POINT_NONE, 0, AFF_NONE, lBlockDuration, 0, true);
}
// END_OF_BLOCK_CHAT

// BUILD_BUILDING
ACMD(do_build)
{
	using namespace building;

	char arg1[256], arg2[256], arg3[256], arg4[256];
	const char * line = one_argument(argument, arg1, sizeof(arg1));
	uint8_t GMLevel = ch->GetGMLevel();

	CLand * pkLand = CManager::Instance().FindLand(ch->GetMapIndex(), ch->GetX(), ch->GetY());

	// NOTE: 조건 체크들은 클라이언트와 서버가 함께 하기 때문에 문제가 있을 때는
	//       메세지를 전송하지 않고 에러를 출력한다.
	if (!pkLand)
	{
		sys_err("%s trying to build on not buildable area.", ch->GetName());
		return;
	}

	if (!*arg1)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, "Invalid syntax: no command");
		return;
	}

	// 건설 권한 체크
	if (GMLevel == GM_PLAYER)
	{
		// 플레이어가 집을 지을 때는 땅이 내껀지 확인해야 한다.
		if ((!ch->GetGuild() || ch->GetGuild()->GetID() != pkLand->GetOwner()))
		{
			sys_err("%s trying to build on not owned land.", ch->GetName());
			return;
		}

		// 내가 길마인가?
		if (ch->GetGuild()->GetMasterPID() != ch->GetPlayerID())
		{
			sys_err("%s trying to build while not the guild master.", ch->GetName());
			return;
		}
	}

	switch (LOWER(*arg1))
	{
		case 'c':
			{
				// /build c vnum x y x_rot y_rot z_rot
				char arg5[256], arg6[256];
				line = one_argument(two_arguments(line, arg1, sizeof(arg1), arg2, sizeof(arg2)), arg3, sizeof(arg3)); // vnum x y
				one_argument(two_arguments(line, arg4, sizeof(arg4), arg5, sizeof(arg5)), arg6, sizeof(arg6)); // x_rot y_rot z_rot

				if (!*arg1 || !*arg2 || !*arg3 || !*arg4 || !*arg5 || !*arg6)
				{
					ch->ChatPacket(CHAT_TYPE_INFO, "Invalid syntax");
					return;
				}

				uint32_t dwVnum = 0;
				str_to_number(dwVnum,  arg1);

				using namespace building;

				const TObjectProto * t = CManager::Instance().GetObjectProto(dwVnum);
				if (!t)
				{
					ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("존재하지 않는 건물입니다."));
					return;
				}

				const uint32_t BUILDING_MAX_PRICE = 100000000;

				if (t->dwGroupVnum)
				{
					if (pkLand->FindObjectByGroup(t->dwGroupVnum))
					{
						ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("같이 지을 수 없는 종류의 건물이 지어져 있습니다."));
						return;
					}
				}

				// 건물 종속성 체크 (이 건물이 지어져 있어야함)
				if (t->dwDependOnGroupVnum)
				{
					//		const TObjectProto * dependent = CManager::Instance().GetObjectProto(dwVnum);
					//		if (dependent)
					{
						// 지어져있는가?
						if (!pkLand->FindObjectByGroup(t->dwDependOnGroupVnum))
						{
							ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("건설에 필요한 건물이 지어져 있지 않습니다."));
							return;
						}
					}
				}

				if (g_bIsTestServer || GMLevel == GM_PLAYER)
				{
					// GM이 아닐경우만 (테섭에서는 GM도 소모)
					// 건설 비용 체크
					if (t->dwPrice > BUILDING_MAX_PRICE)
					{
						ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("건물 비용 정보 이상으로 건설 작업에 실패했습니다."));
						return;
					}

					if (ch->GetGold() < (int32_t)t->dwPrice)
					{
						ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("건설 비용이 부족합니다."));
						return;
					}

					// 아이템 자재 개수 체크

					int32_t i;
					for (i = 0; i < OBJECT_MATERIAL_MAX_NUM; ++i)
					{
						uint32_t dwItemVnum = t->kMaterials[i].dwItemVnum;
						uint32_t dwItemCount = t->kMaterials[i].dwCount;

						if (dwItemVnum == 0)
							break;

						if ((int32_t) dwItemCount > ch->CountSpecifyItem(dwItemVnum))
						{
							ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("자재가 부족하여 건설할 수 없습니다."));
							return;
						}
					}
				}

				float x_rot = atof(arg4);
				float y_rot = atof(arg5);
				float z_rot = atof(arg6);

				int32_t map_x = 0;
				str_to_number(map_x, arg2);
				int32_t map_y = 0;
				str_to_number(map_y, arg3);

				bool isSuccess = pkLand->RequestCreateObject(dwVnum, 
						ch->GetMapIndex(),
						map_x,
						map_y,
						x_rot,
						y_rot,
						z_rot, true);

				if (!isSuccess)
				{
					if (g_bIsTestServer)
						ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("건물을 지을 수 없는 위치입니다."));
					return;
				}

				if (g_bIsTestServer || GMLevel == GM_PLAYER)
					// 건설 재료 소모하기 (테섭에서는 GM도 소모)
				{
					// 건설 비용 소모
					ch->PointChange(POINT_GOLD, -t->dwPrice);

					// 아이템 자재 사용하기 
					{
						int32_t i;
						for (i = 0; i < OBJECT_MATERIAL_MAX_NUM; ++i)
						{
							uint32_t dwItemVnum = t->kMaterials[i].dwItemVnum;
							uint32_t dwItemCount = t->kMaterials[i].dwCount;

							if (dwItemVnum == 0)
								break;

							sys_log(0, "BUILD: material %d %u %u", i, dwItemVnum, dwItemCount);
							ch->RemoveSpecifyItem(dwItemVnum, dwItemCount);
						}
					}
				}
			}
			break;

		case 'd' :
			// build (d)elete ObjectID
			{
				one_argument(line, arg1, sizeof(arg1));

				if (!*arg1)
				{
					ch->ChatPacket(CHAT_TYPE_INFO, "Invalid syntax");
					return;
				}

				uint32_t vid = 0;
				str_to_number(vid, arg1);
				pkLand->RequestDeleteObjectByVID(vid);
			}
			break;

			// BUILD_WALL	

			// build w n/e/w/s
		case 'w' :
			if (GMLevel > GM_PLAYER) 
			{
				int32_t mapIndex = ch->GetMapIndex();

				one_argument(line, arg1, sizeof(arg1));
				
				sys_log(0, "guild.wall.build map[%d] direction[%s]", mapIndex, arg1);

				switch (arg1[0])
				{
					case 's':
						pkLand->RequestCreateWall(mapIndex,   0.0f);
						break;
					case 'n':
						pkLand->RequestCreateWall(mapIndex, 180.0f);
						break;
					case 'e':
						pkLand->RequestCreateWall(mapIndex,  90.0f);
						break;
					case 'w':
						pkLand->RequestCreateWall(mapIndex, 270.0f);
						break;
					default:
						ch->ChatPacket(CHAT_TYPE_INFO, "guild.wall.build unknown_direction[%s]", arg1);
						sys_err("guild.wall.build unknown_direction[%s]", arg1);
						break;
				}

			}
			break;

		case 'e':
			if (GMLevel > GM_PLAYER)
			{
				pkLand->RequestDeleteWall();
			}
			break;

		case 'W' :
			// 담장 세우기
			// build (w)all 담장번호 담장크기 대문동 대문서 대문남 대문북

			if (GMLevel >  GM_PLAYER) 
			{
				int32_t setID = 0, wallSize = 0;
				char arg5[256], arg6[256];
				line = two_arguments(line, arg1, sizeof(arg1), arg2, sizeof(arg2));
				line = two_arguments(line, arg3, sizeof(arg3), arg4, sizeof(arg4));
				two_arguments(line, arg5, sizeof(arg5), arg6, sizeof(arg6));

				str_to_number(setID, arg1);
				str_to_number(wallSize, arg2);

				if (setID != 14105 && setID != 14115 && setID != 14125)
				{
					sys_log(0, "BUILD_WALL: wrong wall set id %d", setID);
					break;
				}
				else 
				{
					bool door_east = false;
					str_to_number(door_east, arg3);
					bool door_west = false;
					str_to_number(door_west, arg4);
					bool door_south = false;
					str_to_number(door_south, arg5);
					bool door_north = false;
					str_to_number(door_north, arg6);
					pkLand->RequestCreateWallBlocks(setID, ch->GetMapIndex(), wallSize, door_east, door_west, door_south, door_north);
				}
			}
			break;

		case 'E' :
			// 담장 지우기
			// build (e)rase 담장셋ID
			if (GMLevel > GM_PLAYER) 
			{
				one_argument(line, arg1, sizeof(arg1));
				uint32_t id = 0;
				str_to_number(id, arg1);
				pkLand->RequestDeleteWallBlocks(id);
			}
			break;

		default:
			ch->ChatPacket(CHAT_TYPE_INFO, "Invalid command %s", arg1);
			break;
	}
}
// END_OF_BUILD_BUILDING

ACMD(do_clear_quest)
{
	char arg1[256];

	one_argument(argument, arg1, sizeof(arg1));

	if (!*arg1)
		return;

	quest::PC* pPC = quest::CQuestManager::Instance().GetPCForce(ch->GetPlayerID());
	pPC->ClearQuest(arg1);
}

ACMD(do_horse_state)
{
	ch->ChatPacket(CHAT_TYPE_INFO, "Horse Information:");
	ch->ChatPacket(CHAT_TYPE_INFO, "    Level  %d", ch->GetHorseLevel());
	ch->ChatPacket(CHAT_TYPE_INFO, "    Health %d/%d (%d%%)", ch->GetHorseHealth(), ch->GetHorseMaxHealth(), ch->GetHorseHealth() * 100 / ch->GetHorseMaxHealth());
	ch->ChatPacket(CHAT_TYPE_INFO, "    Stam   %d/%d (%d%%)", ch->GetHorseStamina(), ch->GetHorseMaxStamina(), ch->GetHorseStamina() * 100 / ch->GetHorseMaxStamina());
}

ACMD(do_horse_level)
{
	char arg1[256] = {0};
	char arg2[256] = {0};
	LPCHARACTER victim;
	int32_t	level = 0;

	two_arguments(argument, arg1, sizeof(arg1), arg2, sizeof(arg2));

	if (!*arg1 || !*arg2)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, "usage : /horse_level <name> <level>");
		return;
	}

	victim = CHARACTER_MANAGER::Instance().FindPC(arg1);

	if (nullptr == victim)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("존재하지 않는 캐릭터 입니다."));
		return;
	}

	str_to_number(level, arg2);
	level = MINMAX(0, level, HORSE_MAX_LEVEL);

	ch->ChatPacket(CHAT_TYPE_INFO, "horse level set (%s: %d)", victim->GetName(), level);

	victim->SetHorseLevel(level);
	victim->ComputePoints();
	victim->SkillLevelPacket();
	return;

/*-----
	char arg1[256];
	one_argument(argument, arg1, sizeof(arg1));

	int32_t level = MINMAX(0, atoi(arg1), HORSE_MAX_LEVEL);

	ch->ChatPacket(CHAT_TYPE_INFO, "horse level set to %d.", level);
	ch->SetHorseLevel(level);
	ch->ComputePoints();
	ch->SkillLevelPacket();
	return;
-----*/
}

ACMD(do_horse_ride)
{
	if (ch->IsHorseRiding())
		ch->StopRiding(); 
	else
		ch->StartRiding();
}

ACMD(do_horse_summon)
{
	ch->HorseSummon(true, true);
}

ACMD(do_horse_unsummon)
{
	ch->HorseSummon(false, true);
}

ACMD(do_horse_set_stat)
{
	char arg1[256], arg2[256];

	two_arguments(argument, arg1, sizeof(arg1), arg2, sizeof(arg2));

	if (*arg1 && *arg2)
	{
		int32_t hp = 0;
		str_to_number(hp, arg1);
		int32_t stam = 0;
		str_to_number(stam, arg2);
		ch->UpdateHorseHealth(hp - ch->GetHorseHealth());
		ch->UpdateHorseStamina(stam - ch->GetHorseStamina());
	}
	else
	{
		ch->ChatPacket(CHAT_TYPE_INFO, "Usage : /horse_set_stat <hp> <stamina>");
	}
}

ACMD(do_save_attribute_to_image) // command "/saveati" for alias
{
	char szFileName[256];
	char szMapIndex[256];

	two_arguments(argument, szMapIndex, sizeof(szMapIndex), szFileName, sizeof(szFileName));

	if (!*szMapIndex || !*szFileName)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, "Syntax: /saveati <map_index> <filename>");
		return;
	}

	int32_t lMapIndex = 0;
	str_to_number(lMapIndex, szMapIndex);

	if (SECTREE_MANAGER::Instance().SaveAttributeToImage(lMapIndex, szFileName))
		ch->ChatPacket(CHAT_TYPE_INFO, "Save done.");
	else
		ch->ChatPacket(CHAT_TYPE_INFO, "Save failed.");
}

ACMD(do_affect_remove)
{
	char arg1[256];
	char arg2[256];

	two_arguments(argument, arg1, sizeof(arg1), arg2, sizeof(arg2));

	if (!*arg1 || !*arg2)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, "Syntax: /affect_remove <player name>");
		ch->ChatPacket(CHAT_TYPE_INFO, "Syntax: /affect_remove <type> <point>");

		LPCHARACTER tch = ch;

		if (*arg1)
			if (!(tch = CHARACTER_MANAGER::Instance().FindPC(arg1)))
				tch = ch;

		ch->ChatPacket(CHAT_TYPE_INFO, "-- Affect List of %s -------------------------------", tch->GetName());
		ch->ChatPacket(CHAT_TYPE_INFO, "Type Point Modif Duration Flag");

		const std::list<CAffect *> & cont = tch->GetAffectContainer();

		auto it = cont.begin();

		while (it != cont.end())
		{
			CAffect * pkAff = *it++;

			ch->ChatPacket(CHAT_TYPE_INFO, "%4d %5d %5d %8d %u", 
					pkAff->dwType, pkAff->bApplyOn, pkAff->lApplyValue, pkAff->lDuration, pkAff->dwFlag);
		}
		return;
	}

	bool removed = false;

	CAffect * af;

	uint32_t	type = 0;
	str_to_number(type, arg1);
	uint8_t	point = 0;
	str_to_number(point, arg2);
	while ((af = ch->FindAffect(type, point)))
	{
		ch->RemoveAffect(af);
		removed = true;
	}

	if (removed)
		ch->ChatPacket(CHAT_TYPE_INFO, "Affect successfully removed.");
	else
		ch->ChatPacket(CHAT_TYPE_INFO, "Not affected by that type and point.");
}

ACMD(do_change_attr)
{
	char cArg1[256];
	one_argument(argument, cArg1, sizeof(cArg1));

	if (!*cArg1)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, "Syntax: /change_attr <EWearPositions>");
		return;
	}

	uint8_t byType = 0;
	str_to_number(byType, cArg1);
	uint8_t byFlags = WEAR_BODY | WEAR_HEAD | WEAR_FOOTS | WEAR_WRIST | WEAR_WEAPON | WEAR_NECK | WEAR_EAR | WEAR_SHIELD;

	if (byFlags & byType)
	{
		LPITEM item = ch->GetWear(byType);
		if (item)
			item->ChangeAttribute();
	}
}

ACMD(do_add_attr)
{
	LPITEM weapon = ch->GetWear(WEAR_WEAPON);
	if (weapon)
		weapon->AddAttribute();
}

ACMD(do_add_socket)
{
	LPITEM weapon = ch->GetWear(WEAR_WEAPON);
	if (weapon)
		weapon->AddSocket();
}

ACMD(do_change_rare_attr)
{
	LPITEM weapon = ch->GetWear(WEAR_WEAPON);
	if (weapon)
		weapon->ChangeRareAttribute();
}

ACMD(do_add_rare_attr)
{
	LPITEM weapon = ch->GetWear(WEAR_WEAPON);
	if (weapon)
		weapon->AddRareAttribute();
}

ACMD(do_show_arena_list)
{
	CArenaManager::Instance().SendArenaMapListTo(ch);
}

ACMD(do_end_all_duel)
{
	CArenaManager::Instance().EndAllDuel();
}

ACMD(do_end_duel)
{
	char szName[256];

	one_argument(argument, szName, sizeof(szName));

	LPCHARACTER pChar = CHARACTER_MANAGER::Instance().FindPC(szName);
	if (pChar == nullptr)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("존재하지 않는 캐릭터 입니다."));
		return;
	}

	if (CArenaManager::Instance().EndDuel(pChar->GetPlayerID()) == false)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("대련 강제 종료 실패"));
	}
	else
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("대련 강제 종료 성공"));
	}
}

ACMD(do_duel)
{
	char szName1[256];
	char szName2[256];
	char szSet[256];
	char szMinute[256];
	int32_t set = 0;
	int32_t minute = 0;

	argument = two_arguments(argument, szName1, sizeof(szName1), szName2, sizeof(szName2));
	two_arguments(argument, szSet, sizeof(szSet), szMinute, sizeof(szMinute));

	str_to_number(set, szSet);
	
	if (set < 0) set = 1;
	if (set > 5) set = 5;

	if (!str_to_number(minute, szMinute))
		minute = 5;

	if (minute < 5)
		minute = 5;

	LPCHARACTER pChar1 = CHARACTER_MANAGER::Instance().FindPC(szName1);
	LPCHARACTER pChar2 = CHARACTER_MANAGER::Instance().FindPC(szName2);

	if (pChar1 != nullptr && pChar2 != nullptr)
	{
		pChar1->RemoveGoodAffect();
		pChar2->RemoveGoodAffect();

		pChar1->RemoveBadAffect();
		pChar2->RemoveBadAffect();

		LPPARTY pParty = pChar1->GetParty();
		if (pParty != nullptr)
		{
			if (pParty->GetMemberCount() == 2)
			{
				CPartyManager::Instance().DeleteParty(pParty);
			}
			else
			{
				pChar1->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("<파티> 파티에서 나가셨습니다."));
				pParty->Quit(pChar1->GetPlayerID());
			}
		}
	
		pParty = pChar2->GetParty();
		if (pParty != nullptr)
		{
			if (pParty->GetMemberCount() == 2)
			{
				CPartyManager::Instance().DeleteParty(pParty);
			}
			else
			{
				pChar2->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("<파티> 파티에서 나가셨습니다."));
				pParty->Quit(pChar2->GetPlayerID());
			}
		}
		
		if (CArenaManager::Instance().StartDuel(pChar1, pChar2, set, minute) == true)
		{
			ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("대련이 성공적으로 시작 되었습니다."));
		}
		else
		{
			ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("대련 시작에 문제가 있습니다."));
		}
	}
	else
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("대련자가 없습니다."));
	}
}

ACMD(do_stat_plus_amount)
{
	char szPoint[256];

	one_argument(argument, szPoint, sizeof(szPoint));

	if (*szPoint == '\0')
		return;

	if (ch->IsPolymorphed())
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("둔갑 중에는 능력을 올릴 수 없습니다."));
		return;
	}

	int32_t nRemainPoint = ch->GetPoint(POINT_STAT);

	if (nRemainPoint <= 0)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("남은 스탯 포인트가 없습니다."));
		return;
	}

	int32_t nPoint = 0;
	str_to_number(nPoint, szPoint);

	if (nRemainPoint < nPoint)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("남은 스탯 포인트가 적습니다."));
		return;
	}

	if (nPoint < 0)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("값을 잘못 입력하였습니다."));
		return;
	}
	
	switch (subcmd)
	{
		case POINT_HT : // 체력
			if (nPoint + ch->GetPoint(POINT_HT) > 90)
			{
				nPoint = 90 - ch->GetPoint(POINT_HT);
			}
			break;

		case POINT_IQ : // 지능
			if (nPoint + ch->GetPoint(POINT_IQ) > 90)
			{
				nPoint = 90 - ch->GetPoint(POINT_IQ);
			}
			break;
			
		case POINT_ST : // 근력
			if (nPoint + ch->GetPoint(POINT_ST) > 90)
			{
				nPoint = 90 - ch->GetPoint(POINT_ST);
			}
			break;
			
		case POINT_DX : // 민첩
			if (nPoint + ch->GetPoint(POINT_DX) > 90)
			{
				nPoint = 90 - ch->GetPoint(POINT_DX);
			}
			break;

		default :
			ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("명령어의 서브 커맨드가 잘못 되었습니다."));
			return;
			break;
	}

	if (nPoint != 0)
	{
		ch->SetRealPoint(subcmd, ch->GetRealPoint(subcmd) + nPoint);
		ch->SetPoint(subcmd, ch->GetPoint(subcmd) + nPoint);
		ch->ComputePoints();
		ch->PointChange(subcmd, 0);

		ch->PointChange(POINT_STAT, -nPoint);
		ch->ComputePoints();
	}
}

struct tTwoPID
{
	int32_t pid1;
	int32_t pid2;
};

ACMD(do_break_marriage)
{
	char arg1[256], arg2[256];
	two_arguments(argument, arg1, sizeof(arg1), arg2, sizeof(arg2));

	tTwoPID pids = { 0, 0 };

	str_to_number(pids.pid1, arg1);
	str_to_number(pids.pid2, arg2);
	
	ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("플레이어 %d 와 플레이어  %d를 파혼시킵니다.."), pids.pid1, pids.pid2);
	db_clientdesc->DBPacket(HEADER_GD_BREAK_MARRIAGE, 0, &pids, sizeof(pids));
}

ACMD(do_effect)
{
	char arg1[256];

	one_argument(argument, arg1, sizeof(arg1));

	int32_t	effect_type = 0;
	str_to_number(effect_type, arg1);
	ch->EffectPacket(effect_type);
}


struct FCountInMap
{
	int32_t m_Count[4];
	FCountInMap() { memset(m_Count, 0, sizeof(int32_t) * 4); }
	void operator()(LPENTITY ent)
	{
		if (ent->IsType(ENTITY_CHARACTER))
		{
			LPCHARACTER ch = (LPCHARACTER) ent;
			if (ch && ch->IsPC())
				++m_Count[ch->GetEmpire()];
		}
	}
	int32_t GetCount(uint8_t bEmpire) { return m_Count[bEmpire]; } 
};

ACMD(do_threeway_war_info)
{
	ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("각제국 진행 정보"));
	ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("선택 맵 정보 성지 %d 통로 %d %d %d"), GetSungziMapIndex(), GetPassMapIndex(1), GetPassMapIndex(2), GetPassMapIndex(3)); 
	ch->ChatPacket(CHAT_TYPE_INFO, "ThreewayPhase %d", CThreeWayWar::Instance().GetRegenFlag());

	for (int32_t n = 1; n < 4; ++n)
	{
		LPSECTREE_MAP pSecMap = SECTREE_MANAGER::Instance().GetMap(GetSungziMapIndex());

		FCountInMap c;

		if (pSecMap)
		{
			pSecMap->for_each(c);
		}

		ch->ChatPacket(CHAT_TYPE_INFO, "%s killscore %d usercount %d",
				EMPIRE_NAME(n),
			   	CThreeWayWar::Instance().GetKillScore(n),
				c.GetCount(n));
	}
}

ACMD(do_threeway_war_myinfo)
{
	ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("나의 삼거리 진행정보"));
	ch->ChatPacket(CHAT_TYPE_INFO, "Deadcount %d",
			CThreeWayWar::Instance().GetReviveTokenForPlayer(ch->GetPlayerID()));
}

ACMD(do_reset_subskill)
{
	char arg1[256];

	one_argument(argument, arg1, sizeof(arg1));
	
	if (!*arg1)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, "Usage: reset_subskill <name>");
		return;
	}
	
	LPCHARACTER tch = CHARACTER_MANAGER::Instance().FindPC(arg1);
	
	if (tch == nullptr)
		return;

	tch->ClearSubSkill();
	ch->ChatPacket(CHAT_TYPE_INFO, "Subskill of [%s] was reset", tch->GetName());
}

ACMD(do_flush)
{
	char arg1[256];
	one_argument(argument, arg1, sizeof(arg1));

	if (0 == arg1[0])
	{
		ch->ChatPacket(CHAT_TYPE_INFO, "usage : /flush player_id");
		return;
	}

	uint32_t pid = (uint32_t) strtoul(arg1, nullptr, 10);

	db_clientdesc->DBPacketHeader(HEADER_GD_FLUSH_CACHE, 0, sizeof(uint32_t));
	db_clientdesc->Packet(&pid, sizeof(uint32_t));
}

ACMD(do_eclipse)
{
	char arg1[256];
	one_argument(argument, arg1, sizeof(arg1));

	if (strtol(arg1, nullptr, 10) == 1)
	{
		quest::CQuestManager::Instance().RequestSetEventFlag("eclipse", 1);
	}
	else
	{
		quest::CQuestManager::Instance().RequestSetEventFlag("eclipse", 0);
	}
}

ACMD(do_event_helper)
{
	char arg1[256];
	int32_t mode = 0;

	one_argument(argument, arg1, sizeof(arg1));
	str_to_number(mode, arg1);

	if (mode == 1)
	{
		xmas::SpawnEventHelper(true);
		ch->ChatPacket(CHAT_TYPE_INFO, "Event Helper Spawn");
	}
	else
	{
		xmas::SpawnEventHelper(false);
		ch->ChatPacket(CHAT_TYPE_INFO, "Event Helper Delete");
	}
}

struct FMobCounter
{
	int32_t nCount;
	int32_t nStoneCount;
	uint32_t dwSpecificVnum;

	FMobCounter(uint32_t tdwSpecificVnum) 
	{
		dwSpecificVnum = tdwSpecificVnum;
		nCount = 0;
		nStoneCount = 0;
	}

	void operator () (LPENTITY ent)
	{
		if (ent->IsType(ENTITY_CHARACTER))
		{
			LPCHARACTER pChar = static_cast<LPCHARACTER>(ent);

			if (dwSpecificVnum) 
			{
				if (pChar->GetRaceNum() == dwSpecificVnum)
					nCount++;

				return;
			}

			if (pChar->IsMonster())
				nCount++;

			if (pChar->IsStone())
				nStoneCount++;
		}
	}
};

ACMD(do_get_mob_count)
{
	char arg1[50];
	one_argument(argument, arg1, sizeof(arg1));
	if (!*arg1)
		return;

	uint32_t specificVnum = 0;
	str_to_number(specificVnum, arg1);

	LPSECTREE_MAP pSectree = SECTREE_MANAGER::Instance().GetMap(ch->GetMapIndex());
	if (pSectree == nullptr)
		return;

	FMobCounter f(specificVnum);
	pSectree->for_each(f);

	if (specificVnum)
		ch->ChatPacket(CHAT_TYPE_INFO, "MapIndex: %d - Count of %u: %d", ch->GetMapIndex(), specificVnum, f.nCount);
	else
		ch->ChatPacket(CHAT_TYPE_INFO, "MapIndex: %d - Mob count: %d, Stone count: %d", ch->GetMapIndex(), f.nCount, f.nStoneCount);
}

ACMD(do_clear_land)
{
	const building::CLand* pLand = building::CManager::Instance().FindLand(ch->GetMapIndex(), ch->GetX(), ch->GetY());

	if( nullptr == pLand )
	{
		return;
	}

	ch->ChatPacket(CHAT_TYPE_INFO, "Guild Land(%d) Cleared", pLand->GetID());

	building::CManager::Instance().ClearLand(pLand->GetID());
}

ACMD(do_special_item)
{
    ITEM_MANAGER::Instance().ConvSpecialDropItemFile();
}

ACMD(do_set_stat)
{
	char szName [256];
	char szChangeAmount[256];

	two_arguments (argument, szName, sizeof (szName), szChangeAmount, sizeof(szChangeAmount));

	if (*szName == '\0' || *szChangeAmount == '\0')
	{
		ch->ChatPacket (CHAT_TYPE_INFO, "Invalid argument.");
		return;
	}

	LPCHARACTER tch = CHARACTER_MANAGER::Instance().FindPC(szName);

	if (!tch)
	{
		CCI * pkCCI = P2P_MANAGER::Instance().Find(szName);

		if (pkCCI)
		{
			ch->ChatPacket (CHAT_TYPE_INFO, "Cannot find player(%s). %s is not in your game server.", szName, szName);
			return;
		}
		else
		{
			ch->ChatPacket (CHAT_TYPE_INFO, "Cannot find player(%s). Perhaps %s doesn't login or exist.", szName, szName);
			return;
		}
	}
	else
	{
		if (tch->IsPolymorphed())
		{
			ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("둔갑 중에는 능력을 올릴 수 없습니다."));
			return;
		}

		if (subcmd != POINT_HT && subcmd != POINT_IQ && subcmd != POINT_ST && subcmd != POINT_DX)
		{
			ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("명령어의 서브 커맨드가 잘못 되었습니다."));
			return;
		}
		int32_t nRemainPoint = tch->GetPoint(POINT_STAT);
		int32_t nCurPoint = tch->GetRealPoint(subcmd);
		int32_t nChangeAmount = 0;
		str_to_number(nChangeAmount, szChangeAmount);
		int32_t nPoint = nCurPoint + nChangeAmount;
		
		int32_t n = -1;
		switch (subcmd)
		{
		case POINT_HT:
			if (nPoint < JobInitialPoints[tch->GetJob()].ht)
			{
				ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("Cannot set stat under initial stat."));
				return;
			}
			n = 0;
			break;
		case POINT_IQ:
			if (nPoint < JobInitialPoints[tch->GetJob()].iq)
			{
				ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("Cannot set stat under initial stat."));
				return;
			}
			n = 1;
			break;
		case POINT_ST:
			if (nPoint < JobInitialPoints[tch->GetJob()].st)
			{
				ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("Cannot set stat under initial stat."));
				return;
			}
			n = 2;
			break;
		case POINT_DX:
			if (nPoint < JobInitialPoints[tch->GetJob()].dx)
			{
				ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("Cannot set stat under initial stat."));
				return;
			}
			n = 3;
			break;
		}

		if (nPoint > 90)
		{
			nChangeAmount -= nPoint - 90;
			nPoint = 90;
		}

		if (nRemainPoint < nChangeAmount)
		{
			ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("남은 스탯 포인트가 적습니다."));
			return;
		}

		tch->SetRealPoint(subcmd, nPoint);
		tch->SetPoint(subcmd, tch->GetPoint(subcmd) + nChangeAmount);
		tch->ComputePoints();
		tch->PointChange(subcmd, 0);

		tch->PointChange(POINT_STAT, -nChangeAmount);
		tch->ComputePoints();

		const char* stat_name[4] = {"con", "int32_t", "str", "dex"};
		if (-1 == n)
			return;
		ch->ChatPacket(CHAT_TYPE_INFO, "%s's %s change %d to %d", szName, stat_name[n], nCurPoint, nPoint);
	}
}

ACMD(do_get_item_id_list)
{
	for (int32_t i = 0; i < INVENTORY_AND_EQUIP_SLOT_MAX; i++)
	{
		LPITEM item = ch->GetInventoryItem(i);
		if (item != nullptr)
			ch->ChatPacket(CHAT_TYPE_INFO, "cell : %d, name : %s, id : %d", item->GetCell(), item->GetName(), item->GetID());
	}
}

ACMD(do_set_socket)
{
	char arg1 [256];
	char arg2 [256];
	char arg3 [256];

	one_argument (two_arguments (argument, arg1, sizeof (arg1), arg2, sizeof(arg2)), arg3, sizeof (arg3));
	
	int32_t item_id, socket_num, value;
	if (!str_to_number (item_id, arg1) || !str_to_number (socket_num, arg2) || !str_to_number (value, arg3))
		return;
	
	LPITEM item = ITEM_MANAGER::Instance().Find (item_id);
	if (item)
		item->SetSocket (socket_num, value);
}

ACMD (do_can_dead)
{
	if (subcmd)
		ch->SetArmada();
	else
		ch->ResetArmada();
}

ACMD (do_all_skill_master)
{
	ch->SetHorseLevel(SKILL_MAX_LEVEL);
	for (int32_t i = 0; i < SKILL_MAX_NUM; i++)
	{
		if (true == ch->CanUseSkill(i))
		{
			switch(i)
			{
				// taking out the it->second->bMaxLevel from map_pkSkillProto (&& 1==40|SKILL_MAX_LEVEL) will be very resource-wasting, so we go full ugly so far
				case SKILL_COMBO:
					ch->SetSkillLevel(i, 2);
					break;
				case SKILL_LANGUAGE1:
				case SKILL_LANGUAGE2:
				case SKILL_LANGUAGE3:
					ch->SetSkillLevel(i, 20);
					break;
				case SKILL_HORSE_SUMMON:
					ch->SetSkillLevel(i, 10);
					break;
				case SKILL_HORSE:
					ch->SetSkillLevel(i, HORSE_MAX_LEVEL);
					break;
				// CanUseSkill will be true for skill_horse_skills if riding
				case SKILL_HORSE_WILDATTACK:
				case SKILL_HORSE_CHARGE:
				case SKILL_HORSE_ESCAPE:
				case SKILL_HORSE_WILDATTACK_RANGE:
					ch->SetSkillLevel(i, 20);
					break;
				default:
					ch->SetSkillLevel(i, SKILL_MAX_LEVEL);
					break;
			}
		}
		else
		{
			switch(i)
			{
			case SKILL_HORSE_WILDATTACK:
			case SKILL_HORSE_CHARGE:
			case SKILL_HORSE_ESCAPE:
			case SKILL_HORSE_WILDATTACK_RANGE:
				ch->SetSkillLevel(i, 20);
				break;
			}
		}
	}
	ch->SetHorseLevel(HORSE_MAX_LEVEL);
	ch->ComputePoints();
	ch->SkillLevelPacket();
}

ACMD (do_item_full_set)
{
	uint8_t job = ch->GetJob();
	LPITEM item;
	for (int32_t i = 0; i < 6; i++)
	{
		item = ch->GetWear(i);
		if (item != nullptr)
			ch->UnequipItem(item);
	}
	item = ch->GetWear(WEAR_SHIELD);
	if (item != nullptr)
		ch->UnequipItem(item);

	switch (job)
	{
	case JOB_SURA:
		{
			
			item = ITEM_MANAGER::Instance().CreateItem(11699);
			if (!item || !item->EquipTo(ch, item->FindEquipCell(ch)))
				M2_DESTROY_ITEM(item);
			item = ITEM_MANAGER::Instance().CreateItem(13049);
			if (!item || !item->EquipTo(ch, item->FindEquipCell(ch)))
				M2_DESTROY_ITEM(item);
			item = ITEM_MANAGER::Instance().CreateItem(15189 );
			if (!item || !item->EquipTo(ch, item->FindEquipCell(ch)))
				M2_DESTROY_ITEM(item);
			item = ITEM_MANAGER::Instance().CreateItem(189 );
			if (!item || !item->EquipTo(ch, item->FindEquipCell(ch)))
				M2_DESTROY_ITEM(item);
			item = ITEM_MANAGER::Instance().CreateItem(12529 );
			if (!item || !item->EquipTo(ch, item->FindEquipCell(ch)))
				M2_DESTROY_ITEM(item);
			item = ITEM_MANAGER::Instance().CreateItem(14109 );
			if (!item || !item->EquipTo(ch, item->FindEquipCell(ch)))
				M2_DESTROY_ITEM(item);
			item = ITEM_MANAGER::Instance().CreateItem(17209 );
			if (!item || !item->EquipTo(ch, item->FindEquipCell(ch)))
				M2_DESTROY_ITEM(item);
			item = ITEM_MANAGER::Instance().CreateItem(16209 );
			if (!item || !item->EquipTo(ch, item->FindEquipCell(ch)))
				M2_DESTROY_ITEM(item);
		}
		break;
	case JOB_WARRIOR:
		{
			
			item = ITEM_MANAGER::Instance().CreateItem(11299);
			if (!item || !item->EquipTo(ch, item->FindEquipCell(ch)))
				M2_DESTROY_ITEM(item);
			item = ITEM_MANAGER::Instance().CreateItem(13049);
			if (!item || !item->EquipTo(ch, item->FindEquipCell(ch)))
				M2_DESTROY_ITEM(item);
			item = ITEM_MANAGER::Instance().CreateItem(15189 );
			if (!item || !item->EquipTo(ch, item->FindEquipCell(ch)))
				M2_DESTROY_ITEM(item);
			item = ITEM_MANAGER::Instance().CreateItem(3159 );
			if (!item || !item->EquipTo(ch, item->FindEquipCell(ch)))
				M2_DESTROY_ITEM(item);
			item = ITEM_MANAGER::Instance().CreateItem(12249 );
			if (!item || !item->EquipTo(ch, item->FindEquipCell(ch)))
				M2_DESTROY_ITEM(item);
			item = ITEM_MANAGER::Instance().CreateItem(14109 );
			if (!item || !item->EquipTo(ch, item->FindEquipCell(ch)))
				M2_DESTROY_ITEM(item);
			item = ITEM_MANAGER::Instance().CreateItem(17109 );
			if (!item || !item->EquipTo(ch, item->FindEquipCell(ch)))
				M2_DESTROY_ITEM(item);
			item = ITEM_MANAGER::Instance().CreateItem(16109 );
			if (!item || !item->EquipTo(ch, item->FindEquipCell(ch)))
				M2_DESTROY_ITEM(item);
		}
		break;
	case JOB_SHAMAN:
		{
			
			item = ITEM_MANAGER::Instance().CreateItem(11899);
			if (!item || !item->EquipTo(ch, item->FindEquipCell(ch)))
				M2_DESTROY_ITEM(item);
			item = ITEM_MANAGER::Instance().CreateItem(13049);
			if (!item || !item->EquipTo(ch, item->FindEquipCell(ch)))
				M2_DESTROY_ITEM(item);
			item = ITEM_MANAGER::Instance().CreateItem(15189 );
			if (!item || !item->EquipTo(ch, item->FindEquipCell(ch)))
				M2_DESTROY_ITEM(item);
			item = ITEM_MANAGER::Instance().CreateItem(7159 );
			if (!item || !item->EquipTo(ch, item->FindEquipCell(ch)))
				M2_DESTROY_ITEM(item);
			item = ITEM_MANAGER::Instance().CreateItem(12669 );
			if (!item || !item->EquipTo(ch, item->FindEquipCell(ch)))
				M2_DESTROY_ITEM(item);
			item = ITEM_MANAGER::Instance().CreateItem(14109 );
			if (!item || !item->EquipTo(ch, item->FindEquipCell(ch)))
				M2_DESTROY_ITEM(item);
			item = ITEM_MANAGER::Instance().CreateItem(17209 );
			if (!item || !item->EquipTo(ch, item->FindEquipCell(ch)))
				M2_DESTROY_ITEM(item);
			item = ITEM_MANAGER::Instance().CreateItem(16209 );
			if (!item || !item->EquipTo(ch, item->FindEquipCell(ch)))
				M2_DESTROY_ITEM(item);
		}
		break;
	case JOB_ASSASSIN:
		{
			
			item = ITEM_MANAGER::Instance().CreateItem(11499);
			if (!item || !item->EquipTo(ch, item->FindEquipCell(ch)))
				M2_DESTROY_ITEM(item);
			item = ITEM_MANAGER::Instance().CreateItem(13049);
			if (!item || !item->EquipTo(ch, item->FindEquipCell(ch)))
				M2_DESTROY_ITEM(item);
			item = ITEM_MANAGER::Instance().CreateItem(15189 );
			if (!item || !item->EquipTo(ch, item->FindEquipCell(ch)))
				M2_DESTROY_ITEM(item);
			item = ITEM_MANAGER::Instance().CreateItem(1139 );
			if (!item || !item->EquipTo(ch, item->FindEquipCell(ch)))
				M2_DESTROY_ITEM(item);
			item = ITEM_MANAGER::Instance().CreateItem(12389 );
			if (!item || !item->EquipTo(ch, item->FindEquipCell(ch)))
				M2_DESTROY_ITEM(item);
			item = ITEM_MANAGER::Instance().CreateItem(14109 );
			if (!item || !item->EquipTo(ch, item->FindEquipCell(ch)))
				M2_DESTROY_ITEM(item);
			item = ITEM_MANAGER::Instance().CreateItem(17189 );
			if (!item || !item->EquipTo(ch, item->FindEquipCell(ch)))
				M2_DESTROY_ITEM(item);
			item = ITEM_MANAGER::Instance().CreateItem(16189 );
			if (!item || !item->EquipTo(ch, item->FindEquipCell(ch)))
				M2_DESTROY_ITEM(item);
		}
		break;
#ifdef ENABLE_WOLFMAN_CHARACTER
	case JOB_WOLFMAN:
		{

			item = ITEM_MANAGER::Instance().CreateItem(21049);
			if (!item || !item->EquipTo(ch, item->FindEquipCell(ch)))
				M2_DESTROY_ITEM(item);
			item = ITEM_MANAGER::Instance().CreateItem(13049);
			if (!item || !item->EquipTo(ch, item->FindEquipCell(ch)))
				M2_DESTROY_ITEM(item);
			item = ITEM_MANAGER::Instance().CreateItem(15189);
			if (!item || !item->EquipTo(ch, item->FindEquipCell(ch)))
				M2_DESTROY_ITEM(item);
			item = ITEM_MANAGER::Instance().CreateItem(6049);
			if (!item || !item->EquipTo(ch, item->FindEquipCell(ch)))
				M2_DESTROY_ITEM(item);
			item = ITEM_MANAGER::Instance().CreateItem(21559);
			if (!item || !item->EquipTo(ch, item->FindEquipCell(ch)))
				M2_DESTROY_ITEM(item);
			item = ITEM_MANAGER::Instance().CreateItem(14109);
			if (!item || !item->EquipTo(ch, item->FindEquipCell(ch)))
				M2_DESTROY_ITEM(item);
			item = ITEM_MANAGER::Instance().CreateItem(17209);
			if (!item || !item->EquipTo(ch, item->FindEquipCell(ch)))
				M2_DESTROY_ITEM(item);
			item = ITEM_MANAGER::Instance().CreateItem(16209);
			if (!item || !item->EquipTo(ch, item->FindEquipCell(ch)))
				M2_DESTROY_ITEM(item);
		}
		break;
#endif
	}
}

ACMD (do_attr_full_set)
{
	uint8_t job = ch->GetJob();
	LPITEM item;

	switch (job)
	{
	case JOB_WARRIOR:
	case JOB_ASSASSIN:
	case JOB_SURA:
	case JOB_SHAMAN:
#ifdef ENABLE_WOLFMAN_CHARACTER
	case JOB_WOLFMAN:
#endif
		{
			// 무사 몸빵 셋팅.
			// 이것만 나와 있어서 임시로 모든 직군 다 이런 속성 따름.
			item = ch->GetWear(WEAR_HEAD);
			if (item != nullptr)
			{
				item->ClearAttribute();
				item->SetForceAttribute( 0, APPLY_ATT_SPEED, 8);
				item->SetForceAttribute( 1, APPLY_HP_REGEN, 30);
				item->SetForceAttribute( 2, APPLY_SP_REGEN, 30);
				item->SetForceAttribute( 3, APPLY_DODGE, 15);
				item->SetForceAttribute( 4, APPLY_STEAL_SP, 10);
			}

			item = ch->GetWear(WEAR_WEAPON);
			if (item != nullptr)
			{
				item->ClearAttribute();
				item->SetForceAttribute( 0, APPLY_CAST_SPEED, 20);
				item->SetForceAttribute( 1, APPLY_CRITICAL_PCT, 10);
				item->SetForceAttribute( 2, APPLY_PENETRATE_PCT, 10);
				item->SetForceAttribute( 3, APPLY_ATTBONUS_DEVIL, 20);
				item->SetForceAttribute( 4, APPLY_STR, 12);
			}

			item = ch->GetWear(WEAR_SHIELD);
			if (item != nullptr)
			{
				item->ClearAttribute();
				item->SetForceAttribute( 0, APPLY_CON, 12);
				item->SetForceAttribute( 1, APPLY_BLOCK, 15);
				item->SetForceAttribute( 2, APPLY_REFLECT_MELEE, 10);
				item->SetForceAttribute( 3, APPLY_IMMUNE_STUN, 1);
				item->SetForceAttribute( 4, APPLY_IMMUNE_SLOW, 1);
			}

			item = ch->GetWear(WEAR_BODY);
			if (item != nullptr)
			{
				item->ClearAttribute();
				item->SetForceAttribute( 0, APPLY_MAX_HP, 2000);
				item->SetForceAttribute( 1, APPLY_CAST_SPEED, 20);
				item->SetForceAttribute( 2, APPLY_STEAL_HP, 10);
				item->SetForceAttribute( 3, APPLY_REFLECT_MELEE, 10);
				item->SetForceAttribute( 4, APPLY_ATT_GRADE_BONUS, 50);
			}

			item = ch->GetWear(WEAR_FOOTS);
			if (item != nullptr)
			{
				item->ClearAttribute();
				item->SetForceAttribute( 0, APPLY_MAX_HP, 2000);
				item->SetForceAttribute( 1, APPLY_MAX_SP, 80);
				item->SetForceAttribute( 2, APPLY_MOV_SPEED, 8);
				item->SetForceAttribute( 3, APPLY_ATT_SPEED, 8);
				item->SetForceAttribute( 4, APPLY_CRITICAL_PCT, 10);
			}

			item = ch->GetWear(WEAR_WRIST);
			if (item != nullptr)
			{
				item->ClearAttribute();
				item->SetForceAttribute( 0, APPLY_MAX_HP, 2000);
				item->SetForceAttribute( 1, APPLY_MAX_SP, 80);
				item->SetForceAttribute( 2, APPLY_PENETRATE_PCT, 10);
				item->SetForceAttribute( 3, APPLY_STEAL_HP, 10);
				item->SetForceAttribute( 4, APPLY_MANA_BURN_PCT, 10);
			}
			item = ch->GetWear(WEAR_NECK);
			if (item != nullptr)
			{
				item->ClearAttribute();
				item->SetForceAttribute( 0, APPLY_MAX_HP, 2000);
				item->SetForceAttribute( 1, APPLY_MAX_SP, 80);
				item->SetForceAttribute( 2, APPLY_CRITICAL_PCT, 10);
				item->SetForceAttribute( 3, APPLY_PENETRATE_PCT, 10);
				item->SetForceAttribute( 4, APPLY_STEAL_SP, 10);
			}
			item = ch->GetWear(WEAR_EAR);
			if (item != nullptr)
			{
				item->ClearAttribute();
				item->SetForceAttribute( 0, APPLY_MOV_SPEED, 20);
				item->SetForceAttribute( 1, APPLY_MANA_BURN_PCT, 10);
				item->SetForceAttribute( 2, APPLY_POISON_REDUCE, 5);
				item->SetForceAttribute( 3, APPLY_ATTBONUS_DEVIL, 20);
				item->SetForceAttribute( 4, APPLY_ATTBONUS_UNDEAD, 20);
			}
		}
		break;
	}
}

ACMD (do_full_set)
{
	do_all_skill_master(ch, nullptr, 0, 0);
	do_item_full_set(ch, nullptr, 0, 0);
	do_attr_full_set(ch, nullptr, 0, 0);
}

ACMD (do_use_item)
{
	char arg1 [256];

	one_argument (argument, arg1, sizeof (arg1));

	int32_t cell = 0;
	str_to_number(cell, arg1);
	
	LPITEM item = ch->GetInventoryItem(cell);
	if (item)
	{
		ch->UseItem(TItemPos (INVENTORY, cell));
	}
	else
	{
		ch->ChatPacket(CHAT_TYPE_INFO, "아이템이 없어서 착용할 수 없어.");
	}
}

ACMD (do_clear_affect)
{
	ch->ClearAffect(true);
}

ACMD (do_dragon_soul)
{
	char arg1[512];
	const char* rest = one_argument (argument, arg1, sizeof(arg1));
	switch (arg1[0])
	{
	case 'a':
		{
			one_argument (rest, arg1, sizeof(arg1));
			int32_t deck_idx;
			if (str_to_number(deck_idx, arg1) == false)
			{
				return;
			}
			ch->DragonSoul_ActivateDeck(deck_idx);
		}
		break;
	case 'd':
		{
			ch->DragonSoul_DeactivateAll();
		}
		break;
	}
}

ACMD (do_ds_list)
{
	for (int32_t i = 0; i < DRAGON_SOUL_INVENTORY_MAX_NUM; i++)
	{
		TItemPos cell(DRAGON_SOUL_INVENTORY, i);
		
		LPITEM item = ch->GetItem(cell);
		if (item != nullptr)
			ch->ChatPacket(CHAT_TYPE_INFO, "cell : %d, name : %s, id : %d", item->GetCell(), item->GetName(), item->GetID());
	}
}

ACMD(do_remove_rights)
{
	char arg1[256];
	one_argument(argument, arg1, sizeof(arg1));

	if (!*arg1)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, "usage: remove_rights <player name>");
		return;
	}

	if (!strcasecmp(ch->GetName(), arg1))
	{
		ch->ChatPacket(CHAT_TYPE_INFO, "You cannot remove your own rights.");
		return;
	}

	if (GM::get_level(arg1) == GM_PLAYER)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, "No player %s with GM-Rights has been found.", arg1);
		return;
	}

	GM::remove(arg1);

	LPCHARACTER tch = CHARACTER_MANAGER::Instance().FindPC(arg1);
	if (tch)
		tch->SetGMLevel();

	TPacketGGUpdateRights packet;
	packet.header = HEADER_GG_UPDATE_RIGHTS;
	strlcpy(packet.name, arg1, sizeof(packet.name));
	packet.gm_level = GM_PLAYER;
	P2P_MANAGER::Instance().Send(&packet, sizeof(packet));

	ch->ChatPacket(CHAT_TYPE_INFO, "The rights of %s has been removed (new status: GM_PLAYER).", arg1);
}

ACMD(do_give_rights)
{
	char arg1[256], arg2[256];
	two_arguments(argument, arg1, sizeof(arg1), arg2, sizeof(arg2));

	if (!*arg1 || !*arg2)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, "usage: give_rights <right name[\"LOW_WIZARD\",\"WIZARD\",\"HIGH_WIZARD\",\"GOD\",\"IMPLEMENTOR\"]/id[1-5]) <player name>");
		return;
	}

	uint8_t bAuthority;
	if (str_is_number(arg1))
	{
		str_to_number(bAuthority, arg1);
		if (bAuthority <= GM_PLAYER || bAuthority > GM_IMPLEMENTOR)
		{
			ch->ChatPacket(CHAT_TYPE_INFO, "Unkown right-ID %u [expected number between 1 and 5]", bAuthority);
			return;
		}
	}
	else
	{
		if (!strcasecmp(arg1, "LOW_WIZARD"))
			bAuthority = GM_LOW_WIZARD;
		else if (!strcasecmp(arg1, "WIZARD"))
			bAuthority = GM_WIZARD;
		else if (!strcasecmp(arg1, "HIGH_WIZARD"))
			bAuthority = GM_HIGH_WIZARD;
		else if (!strcasecmp(arg1, "GOD"))
			bAuthority = GM_GOD;
		else if (!strcasecmp(arg1, "IMPLEMENTOR"))
			bAuthority = GM_IMPLEMENTOR;
		else
		{
			ch->ChatPacket(CHAT_TYPE_INFO, "Unkown right-name %s [expected \"LOW_WIZARD\", \"WIZARD\", \"HIGH_WIZARD\", \"GOD\" or \"IMPLEMENTOR\"]", arg1);
			return;
		}
	}

	if (!strcasecmp(ch->GetName(), arg2))
	{
		ch->ChatPacket(CHAT_TYPE_INFO, "You cannot give rights to yourself.");
		return;
	}

	LPCHARACTER tch = CHARACTER_MANAGER::Instance().FindPC(arg2);
	if (!tch)
	{
		CCI* p2pCCI = P2P_MANAGER::Instance().Find(arg2);
		if (!p2pCCI)
		{
			std::unique_ptr<SQLMsg> pMsg(DBManager::Instance().DirectQuery("SELECT name FROM player WHERE name LIKE '%s'", arg2));
			if (pMsg->Get()->uiNumRows == 0)
			{
				ch->ChatPacket(CHAT_TYPE_INFO, "The player %s does not exist.", arg2);
				return;
			}

			strlcpy(arg2, *mysql_fetch_row(pMsg->Get()->pSQLResult), sizeof(arg2));
		}
		else
			strlcpy(arg2, p2pCCI->szName, sizeof(arg2));
	}
	else
		strlcpy(arg2, tch->GetName(), sizeof(arg2));

	tAdminInfo info;
	memset(&info, 0, sizeof(info));
	info.m_Authority = bAuthority;
	strlcpy(info.m_szName, arg2, sizeof(info.m_szName));
	strlcpy(info.m_szAccount, "[ALL]", sizeof(info.m_szAccount));
	GM::insert(info);

	if (tch)
		tch->SetGMLevel();

	TPacketGGUpdateRights packet;
	packet.header = HEADER_GG_UPDATE_RIGHTS;
	strlcpy(packet.name, arg2, sizeof(packet.name));
	packet.gm_level = bAuthority;
	P2P_MANAGER::Instance().Send(&packet, sizeof(packet));

	ch->ChatPacket(CHAT_TYPE_INFO, "The rights of %s has been changed to %s.", arg2, arg1);
}

ACMD(do_get_distance)
{
	char arg1[256], arg2[256];
	two_arguments(argument, arg1, sizeof(arg1), arg2, sizeof(arg2));
	if (!*arg1 || !str_is_number(arg1) || !*arg2 || !str_is_number(arg2))
	{
		ch->ChatPacket(CHAT_TYPE_INFO, "usage: get_distance <local_x> <local_y>");
		return;
	}

	int32_t lX, lY;
	str_to_number(lX, arg1);
	str_to_number(lY, arg2);

	GPOS basePos;
	if (!SECTREE_MANAGER::Instance().GetMapBasePositionByMapIndex(ch->GetMapIndex(), basePos))
		return;

	int32_t iDistance = DISTANCE_APPROX(ch->GetX() - (lX + basePos.x), ch->GetY() - (lY + basePos.y));
	ch->ChatPacket(CHAT_TYPE_INFO, "Distance to (%ld, %ld) is %d.", lX, lY, iDistance);
}

ACMD(do_gamemaster)
{
	char arg1[256];
	one_argument(argument, arg1, sizeof(arg1));

	if (!*arg1 || !str_is_number(arg1))
	{
		ch->ChatPacket(CHAT_TYPE_INFO, "usage: gamemaster <mode [0/1]>");
		return;
	}

	int32_t arg1_int = 0;
	str_to_number(arg1_int, arg1);

	bool mode = (arg1_int > 0);

	uint32_t dwVnum = 0;

	if (mode)
		dwVnum = 20082;
	else
		dwVnum = 1;

	ch->SetGMInvisible(true);
	ch->SetPolymorph(dwVnum, false);
	ch->WarpSet(ch->GetX() + (rand() % 100) + 50, ch->GetY() + (rand() % 100) + 50, ch->GetMapIndex());
	ch->SetGMInvisible(false);
}

ACMD(do_create_discord_lobby)
{
	ch->CreateDiscordLobby();
}
ACMD(do_join_discord_lobby)
{
	char arg1[256], arg2[256];
	two_arguments(argument, arg1, sizeof(arg1), arg2, sizeof(arg2));
	if (!*arg1 || !str_is_number(arg1) || !*arg2)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, "usage: join_discord_lobby <lobby_id> <lobby_secret>");
		return;
	}

	int64_t lobby_id = 0;
	str_to_number(lobby_id, arg1);

	ch->JoinDiscordLobby(lobby_id, arg2);
}
ACMD(do_get_discord_lobby)
{
	ch->ChatPacket(CHAT_TYPE_INFO, "Lobby ID: %lld Lobby secret: %s", ch->GetDiscordLobbyID(), ch->GetDiscordLobbySecret());
}
