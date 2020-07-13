#include "stdafx.h"
#include "quest_manager.h"
#include "char.h"
#include "char_manager.h"
#include "arena.h"

namespace quest
{
	int32_t arena_start_duel(lua_State * L)
	{
		if (!lua_isstring(L, 1))
		{
			sys_err("invalid argument");
			return 0;
		}

		LPCHARACTER ch = CQuestManager::Instance().GetCurrentCharacterPtr();
		LPCHARACTER ch2 = CHARACTER_MANAGER::Instance().FindPC(lua_tostring(L,1));
		int32_t nSetPoint = (int32_t)lua_tonumber(L, 2);

		if ( ch == nullptr || ch2 == nullptr )
		{
			lua_pushnumber(L, 0);
			return 1;
		}

		if ( ch->IsHorseRiding() == true )
		{
			ch->StopRiding();
			ch->HorseSummon(false);
		}

		if ( ch2->IsHorseRiding() == true )
		{
			ch2->StopRiding();
			ch2->HorseSummon(false);
		}

		if ( CArenaManager::Instance().IsMember(ch->GetMapIndex(), ch->GetPlayerID()) != MEMBER_NO || 
				CArenaManager::Instance().IsMember(ch2->GetMapIndex(), ch2->GetPlayerID()) != MEMBER_NO	)
		{
			lua_pushnumber(L, 2);
			return 1;
		}

		if ( CArenaManager::Instance().StartDuel(ch, ch2, nSetPoint) == false )
		{
			lua_pushnumber(L, 3);
			return 1;
		}

		lua_pushnumber(L, 1);
		return 1;
	}

	int32_t arena_add_map(lua_State* L)
	{
		if (!lua_isnumber(L, 1))
		{
			sys_err("invalid argument");
			return 0;
		}
		int32_t mapIdx		= (int32_t)lua_tonumber(L, 1);
		int32_t startposAX	= (int32_t)lua_tonumber(L, 2);
		int32_t startposAY	= (int32_t)lua_tonumber(L, 3);
		int32_t startposBX	= (int32_t)lua_tonumber(L, 4);
		int32_t startposBY	= (int32_t)lua_tonumber(L, 5);

		if ( CArenaManager::Instance().AddArena(mapIdx, startposAX, startposAY, startposBX, startposBY) == false )
		{
			sys_log(0, "Failed to load arena map info(map:%d AX:%d AY:%d BX:%d BY:%d", mapIdx, startposAX, startposAY, startposBX, startposBY);
		}
		else
		{
			sys_log(0, "Add Arena Map:%d startA(%d,%d) startB(%d,%d)", mapIdx, startposAX, startposAY, startposBX, startposBY);
		}

		return 1;
	}

	int32_t arena_get_duel_list(lua_State* L)
	{
		CArenaManager::Instance().GetDuelList(L);

		return 1;
	}

	int32_t arena_add_observer(lua_State* L)
	{
		if (!lua_isnumber(L, 1))
		{
			sys_err("invalid argument");
			return 0;
		}
		int32_t mapIdx = (int32_t)lua_tonumber(L, 1);
		int32_t ObPointX = (int32_t)lua_tonumber(L, 2);
		int32_t ObPointY = (int32_t)lua_tonumber(L, 3);
		LPCHARACTER ch = CQuestManager::Instance().GetCurrentCharacterPtr();
		if (!ch)
		{
			sys_err("null ch");
			return 0;
		}

		CArenaManager::Instance().AddObserver(ch, mapIdx, ObPointX, ObPointY);
		return 0;
	}

	int32_t arena_is_in_arena(lua_State* L)
	{
		if (!lua_isnumber(L, 1))
		{
			sys_err("invalid argument");
			return 0;
		}
		uint32_t pid = (uint32_t)lua_tonumber(L, 1);

		LPCHARACTER ch = CHARACTER_MANAGER::Instance().FindByPID(pid);
		if (!ch)
		{
			lua_pushnumber(L, 1);
			return 1;
		}
		
		if (!ch->GetArena() || ch->GetArenaObserverMode() == false)
		{
			lua_pushnumber(L, 0);
			return 1;
		}

		if ( CArenaManager::Instance().IsMember(ch->GetMapIndex(), ch->GetPlayerID()) == MEMBER_DUELIST )
			lua_pushnumber(L, 1);
		else
			lua_pushnumber(L, 0);

		return 1;
	}

	void RegisterArenaFunctionTable()
	{
		luaL_reg arena_functions[] =
		{
			{"start_duel",		arena_start_duel		},
			{"add_map",			arena_add_map			},
			{"get_duel_list",	arena_get_duel_list		},
			{"add_observer",	arena_add_observer		},
			{"is_in_arena",		arena_is_in_arena		},

			{nullptr,	nullptr}
		};

		CQuestManager::Instance().AddLuaFunctionTable("arena", arena_functions);
	}
}

