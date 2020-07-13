#include "stdafx.h"
#include "quest_manager.h"
#include "char.h"
#include "sectree_manager.h"
#include "target.h"

namespace quest 
{
	//
	// "target" Lua functions
	//
	int32_t target_pos(lua_State* L)
	{
		LPCHARACTER ch = CQuestManager::Instance().GetCurrentCharacterPtr();
		if (!ch)
		{
			sys_err("Null character pointer triggered at %s:%d", __FILE__, __LINE__);
			return 0;
		}

		PC* pPC = CQuestManager::Instance().GetCurrentPC();
		if (!pPC)
		{
			sys_err("Null pc pointer triggered at %s:%d", __FILE__, __LINE__);
			return 0;
		}

		int32_t iQuestIndex = pPC->GetCurrentQuestIndex();
		if (!iQuestIndex)
		{
			sys_err("Null quest index triggered at %s:%d", __FILE__, __LINE__);
			return 0;
		}

		if (!lua_isstring(L, 1) || !lua_isnumber(L, 2) || !lua_isnumber(L, 3))
		{
			sys_err("invalid argument, name: %s, quest_index %d", ch->GetName(), iQuestIndex);
			return 0;
		}

		GPOS pos;

		if (!SECTREE_MANAGER::Instance().GetMapBasePositionByMapIndex(ch->GetMapIndex(), pos))
		{
			sys_err("cannot find base position in this map %d", ch->GetMapIndex());
			return 0;
		}

		int32_t x = pos.x + (int32_t) lua_tonumber(L, 2) * 100;
		int32_t y = pos.y + (int32_t) lua_tonumber(L, 3) * 100;

		CTargetManager::Instance().CreateTarget(ch->GetPlayerID(),
				iQuestIndex,
				lua_tostring(L, 1),
				TARGET_TYPE_POS,
				x,
				y,
				(int32_t) lua_tonumber(L, 4),
				lua_isstring(L, 5) ? lua_tostring(L, 5) : nullptr,
				lua_isnumber(L, 6) ? (int32_t)lua_tonumber(L, 6): 500
		);

		return 0;
	}

	int32_t target_vid(lua_State* L)
	{
		LPCHARACTER ch = CQuestManager::Instance().GetCurrentCharacterPtr();
		if (!ch)
		{
			sys_err("Null character pointer triggered at %s:%d", __FILE__, __LINE__);
			return 0;
		}

		PC* pPC = CQuestManager::Instance().GetCurrentPC();
		if (!pPC)
		{
			sys_err("Null pc pointer triggered at %s:%d", __FILE__, __LINE__);
			return 0;
		}

		int32_t iQuestIndex = pPC->GetCurrentQuestIndex();
		if (!iQuestIndex)
		{
			sys_err("Null quest index triggered at %s:%d", __FILE__, __LINE__);
			return 0;
		}

		if (!lua_isstring(L, 1) || !lua_isnumber(L, 2))
		{
			sys_err("invalid argument, name: %s, quest_index %d", ch->GetName(), iQuestIndex);
			return 0;
		}


		CTargetManager::Instance().CreateTarget(ch->GetPlayerID(),
				iQuestIndex,
				lua_tostring(L, 1),
				TARGET_TYPE_VID,
				(int32_t) lua_tonumber(L, 2),
				0,
				ch->GetMapIndex(),
				lua_isstring(L, 3) ? lua_tostring(L, 3) : nullptr,
				lua_isnumber(L, 4) ? (int32_t)lua_tonumber(L, 4): 1
		);

		return 0;
	}

	// 현재 퀘스트에 등록된 타겟을 삭제 한다.
	int32_t target_delete(lua_State* L)
	{
		LPCHARACTER ch = CQuestManager::Instance().GetCurrentCharacterPtr();
		if (!ch)
		{
			sys_err("Null character pointer triggered at %s:%d", __FILE__, __LINE__);
			return 0;
		}

		PC* pPC = CQuestManager::Instance().GetCurrentPC();
		if (!pPC)
		{
			sys_err("Null pc pointer triggered at %s:%d", __FILE__, __LINE__);
			return 0;
		}

		int32_t iQuestIndex = pPC->GetCurrentQuestIndex();
		if (!iQuestIndex)
		{
			sys_err("Null quest index triggered at %s:%d", __FILE__, __LINE__);
			return 0;
		}

		if (!lua_isstring(L, 1))
		{
			sys_err("invalid argument, name: %s, quest_index %d", ch->GetName(), iQuestIndex);
			return 0;
		}

		CTargetManager::Instance().DeleteTarget(ch->GetPlayerID(), iQuestIndex, lua_tostring(L, 1));

		return 0;
	}

	// 현재 퀘스트 인덱스로 되어있는 타겟을 모두 삭제한다.
	int32_t target_clear(lua_State* L)
	{
		LPCHARACTER ch = CQuestManager::Instance().GetCurrentCharacterPtr();
		if (!ch)
		{
			sys_err("Null character pointer triggered at %s:%d", __FILE__, __LINE__);
			return 0;
		}

		PC* pPC = CQuestManager::Instance().GetCurrentPC();
		if (!pPC)
		{
			sys_err("Null pc pointer triggered at %s:%d", __FILE__, __LINE__);
			return 0;
		}

		int32_t iQuestIndex = pPC->GetCurrentQuestIndex();
		if (!iQuestIndex)
		{
			sys_err("Null quest index triggered at %s:%d", __FILE__, __LINE__);
			return 0;
		}
		CTargetManager::Instance().DeleteTarget(ch->GetPlayerID(), iQuestIndex, nullptr);

		return 0;
	}

	int32_t target_id(lua_State* L)
	{
		LPCHARACTER ch = CQuestManager::Instance().GetCurrentCharacterPtr();
		if (!ch)
		{
			sys_err("Null character pointer triggered at %s:%d", __FILE__, __LINE__);
			return 0;
		}

		PC* pPC = CQuestManager::Instance().GetCurrentPC();
		if (!pPC)
		{
			sys_err("Null pc pointer triggered at %s:%d", __FILE__, __LINE__);
			return 0;
		}

		uint32_t dwQuestIndex = pPC->GetCurrentQuestIndex();
		if (!dwQuestIndex)
		{
			sys_err("Null quest index triggered at %s:%d", __FILE__, __LINE__);
			return 0;
		}

		if (!lua_isstring(L, 1))
		{
			sys_err("invalid argument, name: %s, quest_index %u", ch->GetName(), dwQuestIndex);
			lua_pushnumber(L, 0);
			return 1;
		}

		LPEVENT pkEvent = CTargetManager::Instance().GetTargetEvent(ch->GetPlayerID(), dwQuestIndex, (const char *) lua_tostring(L, 1));

		if (pkEvent)
		{
			TargetInfo * pInfo = dynamic_cast<TargetInfo *>(pkEvent->info);

			if ( pInfo == nullptr )
			{
				sys_err( "target_id> <Factor> Null pointer" );
				lua_pushnumber(L, 0);
				return 1;
			}

			if (pInfo->iType == TARGET_TYPE_VID)
			{
				lua_pushnumber(L, pInfo->iArg1);
				return 1;
			}
		}

		lua_pushnumber(L, 0);
		return 1;
	}

	void RegisterTargetFunctionTable()
	{
		luaL_reg target_functions[] =
		{
			{ "pos",			target_pos		},
			{ "vid",			target_vid		},
			{ "npc",			target_vid		}, // TODO: delete this
			{ "pc",			target_vid		}, // TODO: delete this
			{ "delete",			target_delete		},
			{ "clear",			target_clear		},
			{ "id",			target_id		},
			{ nullptr,			nullptr			},
		};

		CQuestManager::Instance().AddLuaFunctionTable("target", target_functions);
	}
};

