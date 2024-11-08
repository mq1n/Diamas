#include "stdafx.h"
#include "quest_manager.h"
#include "char.h"
#include "char_manager.h"
#include "ox_event.h"
#include "config.h"
#include "locale_service.h"

namespace quest
{
	int32_t oxevent_get_status(lua_State* L)
	{
		OXEventStatus ret = COXEventManager::Instance().GetStatus();

		lua_pushnumber(L, (int32_t)ret);

		return 1;
	}

	int32_t oxevent_open(lua_State* L)
	{
		COXEventManager::Instance().ClearQuiz();

		char script[256];
		snprintf(script, sizeof(script), "%s/oxquiz.lua", LocaleService_GetBasePath().c_str());
		int32_t result = lua_dofile(quest::CQuestManager::Instance().GetLuaState(), script);

		if (result != 0)
		{
			lua_pushnumber(L, 0);
			return 1;
		}
		else
		{
			lua_pushnumber(L, 1);
		}
		
		COXEventManager::Instance().SetStatus(OXEVENT_OPEN);

		return 1;
	}
	
	int32_t oxevent_close(lua_State* L)
	{
		COXEventManager::Instance().SetStatus(OXEVENT_CLOSE);
		
		return 0;
	}
	
	int32_t oxevent_quiz(lua_State* L)
	{
		if (lua_isnumber(L, 1) && lua_isnumber(L, 2))
		{
			bool ret = COXEventManager::Instance().Quiz((int32_t)lua_tonumber(L, 1), (int32_t)lua_tonumber(L, 2));

			if (ret == false)
			{
				lua_pushnumber(L, 0);
			}
			else
			{
				lua_pushnumber(L, 1);
			}
			return 1;
		}

		return 0;
	}
	
	int32_t oxevent_get_attender(lua_State* L)
	{
		lua_pushnumber(L, (int32_t)COXEventManager::Instance().GetAttenderCount());
		return 1;
	}

	EVENTINFO(end_oxevent_info)
	{
		int32_t empty;

		end_oxevent_info() : 
			empty(0)
		{
		}
	};

	EVENTFUNC(end_oxevent)
	{
		COXEventManager::Instance().CloseEvent();
		return 0;
	}

	int32_t oxevent_end_event(lua_State* L)
	{
		COXEventManager::Instance().SetStatus(OXEVENT_FINISH);

		end_oxevent_info* info = AllocEventInfo<end_oxevent_info>();
		event_create(end_oxevent, info, PASSES_PER_SEC(5));

		return 0;
	}

	int32_t oxevent_end_event_force(lua_State* L)
	{
		COXEventManager::Instance().CloseEvent();
		COXEventManager::Instance().SetStatus(OXEVENT_FINISH);

		return 0;
	}

	int32_t oxevent_give_item(lua_State* L)
	{
		if (lua_isnumber(L, 1) && lua_isnumber(L, 2))
		{
			COXEventManager::Instance().GiveItemToAttender((int32_t)lua_tonumber(L, 1), (int32_t)lua_tonumber(L, 2));
		}

		return 0;
	}

	int32_t oxevent_is_banned(lua_State* L)
	{
		CHARACTER* ch = CQuestManager::Instance().GetCurrentCharacterPtr();
		if (ch)
		{
			lua_pushboolean(L, COXEventManager::Instance().IsBanned(ch));
			return 1;
		}
		return 0;
	}
	
	void RegisterOXEventFunctionTable()
	{
		luaL_reg oxevent_functions[] = 
		{
			{	"get_status",	oxevent_get_status	},
			{	"open",			oxevent_open		},
			{	"close",		oxevent_close		},
			{	"quiz",			oxevent_quiz		},
			{	"get_attender",	oxevent_get_attender},
			{	"end_event",	oxevent_end_event	},
			{	"end_event_force",	oxevent_end_event_force	},
			{	"is_banned",		oxevent_is_banned		},
			{	"give_item",	oxevent_give_item	},

			{ nullptr, nullptr}
		};

		CQuestManager::Instance().AddLuaFunctionTable("oxevent", oxevent_functions);
	}
}

