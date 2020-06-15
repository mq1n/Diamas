#include "stdafx.h"
#include "questlua.h"
#include "quest_manager.h"
#include "char.h"
#include "affect.h"
#include "config.h"
#include "utils.h"

#undef sys_err
#ifndef __WIN32__
#define sys_err(fmt, args...) quest::CQuestManager::instance().QuestError(__FUNCTION__, __LINE__, fmt, ##args)
#else
#define sys_err(fmt, ...) quest::CQuestManager::instance().QuestError(__FUNCTION__, __LINE__, fmt, __VA_ARGS__)
#endif

extern bool IS_MOUNTABLE_ZONE(int32_t map_index, bool isHorse);
namespace quest
{
	//
	// "horse" Lua functions
	//
	int32_t horse_is_riding(lua_State* L)
	{
		LPCHARACTER ch = CQuestManager::instance().GetCurrentCharacterPtr();
		if (!ch)
		{
			sys_err("null ch ptr");
			return 0;
		}

		if (ch->IsHorseRiding())
			lua_pushnumber(L, 1);
		else
			lua_pushnumber(L, 0);

		return 1;
	}

	int32_t horse_is_summon(lua_State* L)
	{
		LPCHARACTER ch = CQuestManager::instance().GetCurrentCharacterPtr();

		if (ch)
		{
			lua_pushboolean(L, (ch->GetHorse() != nullptr) ? true : false);
		}
		else
		{
			lua_pushboolean(L, false);
		}

		return 1;
	}

	int32_t horse_ride(lua_State* L)
	{
		LPCHARACTER ch = CQuestManager::instance().GetCurrentCharacterPtr();
		if (!ch)
		{
			sys_err("null ch ptr");
			return 0;
		}

		ch->StartRiding();
		return 0;
	}

	int32_t horse_unride(lua_State* L)
	{
		LPCHARACTER ch = CQuestManager::instance().GetCurrentCharacterPtr();
		if (!ch)
		{
			sys_err("null ch ptr");
			return 0;
		}

		ch->StopRiding();
		return 0;
	}

	int32_t horse_summon(lua_State* L)
	{
		LPCHARACTER ch = CQuestManager::instance().GetCurrentCharacterPtr();
		if (!ch)
		{
			sys_err("null ch ptr");
			return 0;
		}

		if (!IS_MOUNTABLE_ZONE(ch->GetMapIndex(), true))
		{
			ch->ChatPacket(CHAT_TYPE_INFO, "RIDING_IS_BLOCKED_HERE");
			return 0;
		}

		bool bFromFar = lua_isboolean(L, 1) ? lua_toboolean(L, 1) : false;
		uint32_t horseVnum = lua_isnumber(L, 2) ? (uint32_t)lua_tonumber(L, 2) : 0;

		ch->HorseSummon(true, bFromFar, horseVnum);
		return 0;
	}

	int32_t horse_unsummon(lua_State* L)
	{
		LPCHARACTER ch = CQuestManager::instance().GetCurrentCharacterPtr();
		if (!ch)
		{
			sys_err("null ch ptr");
			return 0;
		}
		ch->HorseSummon(false);
		return 0;
	}

	int32_t horse_is_mine(lua_State* L)
	{
		LPCHARACTER ch = CQuestManager::instance().GetCurrentCharacterPtr();
		if (!ch)
		{
			sys_err("null ch ptr");
			return 0;
		}
		LPCHARACTER horse = CQuestManager::instance().GetCurrentNPCCharacterPtr();

		lua_pushboolean(L, horse && horse->GetRider() == ch);
		return 1;
	}

	int32_t horse_set_level(lua_State* L)
	{
		LPCHARACTER ch = CQuestManager::instance().GetCurrentCharacterPtr();
		if (!ch)
		{
			sys_err("null ch ptr");
			return 0;
		}

		if (!lua_isnumber(L, 1))
			return 0;

		int32_t newlevel = MINMAX(0, (int32_t)lua_tonumber(L, 1), HORSE_MAX_LEVEL);
		ch->SetHorseLevel(newlevel);
		ch->ComputePoints();
		ch->SkillLevelPacket();
		return 0;
	}

	int32_t horse_get_level(lua_State* L)
	{
		LPCHARACTER ch = CQuestManager::instance().GetCurrentCharacterPtr();
		if (!ch)
		{
			sys_err("null ch ptr");
			return 0;
		}
		lua_pushnumber(L, ch->GetHorseLevel());
		return 1;
	}

	int32_t horse_advance(lua_State* L)
	{
		LPCHARACTER ch = CQuestManager::instance().GetCurrentCharacterPtr();
		if (!ch)
		{
			sys_err("null ch ptr");
			return 0;
		}

		if (ch->GetHorseLevel() >= HORSE_MAX_LEVEL)
			return 0;

		ch->SetHorseLevel(ch->GetHorseLevel() + 1);
		ch->ComputePoints();
		ch->SkillLevelPacket();
		return 0;
	}

	int32_t horse_get_health(lua_State* L)
	{
		LPCHARACTER ch = CQuestManager::instance().GetCurrentCharacterPtr();
		if (!ch)
		{
			sys_err("null ch ptr");
			return 0;
		}

		if (ch->GetHorseLevel())
			lua_pushnumber(L, ch->GetHorseHealth());
		else
			lua_pushnumber(L, 0);

		return 1;
	}

	int32_t horse_get_health_pct(lua_State* L)
	{
		LPCHARACTER ch = CQuestManager::instance().GetCurrentCharacterPtr();
		if (!ch)
		{
			sys_err("null ch ptr");
			return 0;
		}

		int32_t pct = MINMAX(0, ch->GetHorseHealth() * 100 / ch->GetHorseMaxHealth(), 100);

		if (ch->GetHorseLevel())
			lua_pushnumber(L, pct);
		else
			lua_pushnumber(L, 0);

		return 1;
	}

	int32_t horse_get_stamina(lua_State* L)
	{
		LPCHARACTER ch = CQuestManager::instance().GetCurrentCharacterPtr();
		if (!ch)
		{
			sys_err("null ch ptr");
			return 0;
		}

		if (ch->GetHorseLevel())
			lua_pushnumber(L, ch->GetHorseStamina());
		else
			lua_pushnumber(L, 0);

		return 1;
	}

	int32_t horse_get_stamina_pct(lua_State* L)
	{
		LPCHARACTER ch = CQuestManager::instance().GetCurrentCharacterPtr();
		if (!ch)
		{
			sys_err("null ch ptr");
			return 0;
		}
		int32_t pct = MINMAX(0, ch->GetHorseStamina() * 100 / ch->GetHorseMaxStamina(), 100);

		if (ch->GetHorseLevel())
			lua_pushnumber(L, pct);
		else
			lua_pushnumber(L, 0);

		return 1;
	}

	int32_t horse_get_grade(lua_State* L)
	{
		LPCHARACTER ch = CQuestManager::instance().GetCurrentCharacterPtr();
		if (!ch)
		{
			sys_err("null ch ptr");
			return 0;
		}
		lua_pushnumber(L, ch->GetHorseGrade());
		return 1;
	}

	int32_t horse_is_dead(lua_State* L)
	{
		LPCHARACTER ch = CQuestManager::instance().GetCurrentCharacterPtr();
		if (!ch)
		{
			sys_err("null ch ptr");
			return 0;
		}
		lua_pushboolean(L, ch->GetHorseHealth()<=0);
		return 1;
	}

	int32_t horse_revive(lua_State* L)
	{
		LPCHARACTER ch = CQuestManager::instance().GetCurrentCharacterPtr();
		if (!ch)
		{
			sys_err("null ch ptr");
			return 0;
		}
		if (ch->GetHorseLevel() > 0 && ch->GetHorseHealth() <= 0)
		{
			ch->ReviveHorse();
		}
		return 0;
	}

	int32_t horse_feed(lua_State* L)
	{
		LPCHARACTER ch = CQuestManager::instance().GetCurrentCharacterPtr();
		if (!ch)
		{
			sys_err("null ch ptr");
			return 0;
		}
		//uint32_t dwHorseFood = ch->GetHorseLevel() + ITEM_HORSE_FOOD_1 - 1;
		if (ch->GetHorseLevel() > 0 && ch->GetHorseHealth() > 0)
		{
			ch->FeedHorse();
		}
		return 0;
	}

	int32_t horse_set_name(lua_State* L)
	{
		if ( !lua_isstring(L, -1) ) 
			return 0;
		const char* pHorseName = lua_tostring(L, -1);

		LPCHARACTER ch = CQuestManager::instance().GetCurrentCharacterPtr();
		if (!ch)
		{
			sys_err("null ch ptr");
			return 0;
		}

		if ( ch->GetHorseLevel() > 0 )
		{
			if ( pHorseName == nullptr || check_name(pHorseName) == 0 )
			{
				lua_pushnumber(L, 1);
			}
			else
			{
				int32_t nHorseNameDuration = g_bIsTestServer ? 60 * 5 : 60 * 60 * 24 * 30;

				ch->SetQuestFlag("horse_name.valid_till", get_global_time() + nHorseNameDuration);
				ch->AddAffect(AFFECT_HORSE_NAME, 0, 0, 0, PASSES_PER_SEC(nHorseNameDuration), 0, true);

				ch->HorseSummon(false, true);
				ch->HorseSummon(true, true);

				lua_pushnumber(L, 2);
			}
		}
		else
		{
			lua_pushnumber(L, 0);
		}

		return 1;
	}

	int32_t horse_get_name(lua_State* L)
	{
		LPCHARACTER ch = CQuestManager::instance().GetCurrentCharacterPtr();

		if ( ch != nullptr )
		{
			lua_pushstring(L, ch->GetHorseName());
			return 1;
		}

		lua_pushstring(L, "");

		return 1;
	}
	int32_t horse_in_summonable_area(lua_State* L)
	{
		LPCHARACTER ch = CQuestManager::instance().GetCurrentCharacterPtr();
		if (!ch)
		{
			sys_err("null ch ptr");
			return 0;
		}

		bool isMount = lua_isboolean(L, 1) ? lua_toboolean(L, 1) : true;

		lua_pushboolean(L, IS_MOUNTABLE_ZONE(ch->GetMapIndex(), isMount));
		return 1;
	}

	void RegisterHorseFunctionTable()
	{
		luaL_reg horse_functions[] =
		{
			{ "is_mine",		horse_is_mine			},
			{ "is_riding",		horse_is_riding			},
			{ "is_summon",		horse_is_summon			},
			{ "in_summonable_area", horse_in_summonable_area},
			{ "ride",			horse_ride				},
			{ "unride",			horse_unride			},
			{ "summon",			horse_summon			},
			{ "unsummon",		horse_unsummon			},
			{ "advance",		horse_advance			},
			{ "get_level",		horse_get_level			},
			{ "set_level",		horse_set_level			},
			{ "get_health",		horse_get_health		},
			{ "get_health_pct",	horse_get_health_pct	},
			{ "get_stamina",	horse_get_stamina		},
			{ "get_stamina_pct",horse_get_stamina_pct	},
			{ "get_grade",      horse_get_grade         },
			{ "is_dead",		horse_is_dead			},
			{ "revive",			horse_revive			},
			{ "feed",			horse_feed				},
			{ "set_name",		horse_set_name			},
			{ "get_name",		horse_get_name			},

			{ nullptr,				nullptr					}
		};

		CQuestManager::instance().AddLuaFunctionTable("horse", horse_functions);
	}
}




