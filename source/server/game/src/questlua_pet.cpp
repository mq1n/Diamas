#include "stdafx.h"
#include "questlua.h"
#include "quest_manager.h"
#include "char.h"
#include "affect.h"
#include "config.h"
#include "utils.h"
#include "pet_system.h"

#undef sys_err
#ifndef __WIN32__
#define sys_err(fmt, args...) quest::CQuestManager::instance().QuestError(__FUNCTION__, __LINE__, fmt, ##args)
#else
#define sys_err(fmt, ...) quest::CQuestManager::instance().QuestError(__FUNCTION__, __LINE__, fmt, __VA_ARGS__)
#endif

namespace quest
{
	// syntax in LUA: pet.summon(mob_vnum, pet's name, (bool)run to me from far away)
	int32_t pet_summon(lua_State* L)
	{
		LPCHARACTER ch = CQuestManager::instance().GetCurrentCharacterPtr();
		if (!ch)
		{
			lua_pushnumber(L, 0);
			return 1;
		}

		CPetSystem* petSystem = ch->GetPetSystem();
		CItem * pItem = CQuestManager::instance().GetCurrentItem();
		if (!petSystem || !pItem)
		{
			lua_pushnumber (L, 0);
			return 1;
		}

		// 소환수의 vnum
		uint32_t mobVnum = lua_isnumber(L, 1) ? (uint32_t)lua_tonumber(L, 1) : 0;

		// 소환수의 이름
		const char* petName = lua_isstring(L, 2) ? lua_tostring(L, 2) : nullptr;

		// 소환하면 멀리서부터 달려오는지 여부
		bool bFromFar = lua_isboolean(L, 3) ? lua_toboolean(L, 3) : false;

		auto pet = petSystem->Summon(mobVnum, pItem, petName, bFromFar);
		if (pet != nullptr)
			lua_pushnumber (L, pet->GetVID());
		else
			lua_pushnumber (L, 0);

		return 1;
	}

	// syntax: pet.unsummon(mob_vnum)
	int32_t pet_unsummon(lua_State* L)
	{
		LPCHARACTER ch = CQuestManager::instance().GetCurrentCharacterPtr();
		if (nullptr == ch)
			return 0;

		CPetSystem* petSystem = ch->GetPetSystem();
		if (nullptr == petSystem)
			return 0;

		uint32_t mobVnum= lua_isnumber(L, 1) ? (uint32_t)lua_tonumber(L, 1) : 0;

		petSystem->Unsummon(mobVnum);
		return 1;
	}

	// syntax: pet.unsummon(mob_vnum)
	int32_t pet_count_summoned(lua_State* L)
	{
		LPCHARACTER ch = CQuestManager::instance().GetCurrentCharacterPtr();
		if (nullptr == ch)
			return 0;
		
		CPetSystem* petSystem = ch->GetPetSystem();

		uint32_t count = 0;

		if (nullptr != petSystem)
			count = petSystem->CountSummoned();

		lua_pushnumber(L, count);

		return 1;
	}

	// syntax: pet.is_summon(mob_vnum)
	int32_t pet_is_summon(lua_State* L)
	{
		LPCHARACTER ch = CQuestManager::instance().GetCurrentCharacterPtr();
		if (nullptr == ch)
			return 0;
		
		CPetSystem* petSystem = ch->GetPetSystem();
		if (nullptr == petSystem)
			return 0;

		uint32_t mobVnum= lua_isnumber(L, 1) ? (uint32_t)lua_tonumber(L, 1) : 0;

		CPetActor* petActor = petSystem->GetByVnum(mobVnum);

		if (nullptr == petActor)
			lua_pushboolean(L, false);
		else
			lua_pushboolean(L, petActor->IsSummoned());

		return 1;
	}

	int32_t pet_spawn_effect(lua_State* L)
	{
		LPCHARACTER ch = CQuestManager::instance().GetCurrentCharacterPtr();
		if (nullptr == ch)
			return 0;

		CPetSystem* petSystem = ch->GetPetSystem();
		if (nullptr == petSystem)
			return 0;

		uint32_t mobVnum = lua_isnumber(L, 1) ? (uint32_t)lua_tonumber(L, 1) : 0;

		CPetActor* petActor = petSystem->GetByVnum(mobVnum);
		if (nullptr == petActor)
			return 0;

		LPCHARACTER pet_ch = petActor->GetCharacter();
		if (nullptr == pet_ch)
			return 0;

		if (lua_isstring(L, 2))
			pet_ch->SpecificEffectPacket (lua_tostring(L, 2));
		
		return 0;
	}

	int32_t pet_is_mine(lua_State* L)
	{
		CQuestManager& q = CQuestManager::instance();
		LPCHARACTER mch = q.GetCurrentCharacterPtr();
		LPCHARACTER tch = q.GetCurrentNPCCharacterPtr();
		CPetSystem* petSystem = mch->GetPetSystem();
		CPetActor* petActor = petSystem->GetByVID(tch->GetVID());

		lua_pushboolean(L, tch && tch->IsPet() && petActor && petActor->GetOwner() == mch);
		return 1;
	}

	void RegisterPetFunctionTable()
	{
		luaL_reg pet_functions[] =
		{
			{ "summon",			pet_summon			},
			{ "unsummon",		pet_unsummon		},
			{ "is_summon",		pet_is_summon		},
			{ "count_summoned",	pet_count_summoned	},
			{ "spawn_effect",	pet_spawn_effect	},
			{ "is_mine",        pet_is_mine			},
			{ nullptr,				nullptr				}
		};

		CQuestManager::instance().AddLuaFunctionTable("pet", pet_functions);
	}
}