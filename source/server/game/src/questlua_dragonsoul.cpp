#include "stdafx.h"
#include "config.h"
#include "quest_manager.h"
#include "char.h"

#undef sys_err
#ifndef __WIN32__
#define sys_err(fmt, args...) quest::CQuestManager::Instance().QuestError(__FUNCTION__, __LINE__, fmt, ##args)
#else
#define sys_err(fmt, ...) quest::CQuestManager::Instance().QuestError(__FUNCTION__, __LINE__, fmt, __VA_ARGS__)
#endif

namespace quest 
{
	int32_t ds_open_refine_window(lua_State* L)
	{
		const LPCHARACTER ch = CQuestManager::Instance().GetCurrentCharacterPtr();
		if (nullptr == ch)
		{
			sys_err ("nullptr POINT ERROR");
			return 0;
		}
		if (ch->DragonSoul_IsQualified())
			ch->DragonSoul_RefineWindow_Open(CQuestManager::Instance().GetCurrentNPCCharacterPtr());

		return 0;
	}

	int32_t ds_give_qualification(lua_State* L)
	{
		const LPCHARACTER ch = CQuestManager::Instance().GetCurrentCharacterPtr();
		if (nullptr == ch)
		{
			sys_err ("nullptr POINT ERROR");
			return 0;
		}
		ch->DragonSoul_GiveQualification();

		return 0;
	}

	int32_t ds_is_qualified(lua_State* L)
	{
		const LPCHARACTER ch = CQuestManager::Instance().GetCurrentCharacterPtr();
		if (nullptr == ch)
		{
			sys_err ("nullptr POINT ERROR");
			lua_pushnumber(L, 0);
			return 1;
		}

		lua_pushnumber(L, ch->DragonSoul_IsQualified());
		return 1;
	}

	void RegisterDragonSoulFunctionTable()
	{
		luaL_reg ds_functions[] = 
		{
			{ "open_refine_window"	, ds_open_refine_window },
			{ "give_qualification"	, ds_give_qualification },
			{ "is_qualified"		, ds_is_qualified		},
			{ nullptr					, nullptr					}
		};

		CQuestManager::Instance().AddLuaFunctionTable("ds", ds_functions);
	}
};
