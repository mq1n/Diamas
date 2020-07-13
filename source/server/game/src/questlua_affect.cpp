#include "stdafx.h"
#include "config.h"
#include "quest_manager.h"
#include "sectree_manager.h"
#include "char.h"
#include "affect.h"
#include "db.h"

namespace quest
{
	//
	// "affect" Lua functions
	//
	int32_t affect_add(lua_State * L)
	{
		if (!lua_isnumber(L, 1) || !lua_isnumber(L, 2) || !lua_isnumber(L, 3))
		{
			sys_err("invalid argument");
			return 0;
		}
		uint8_t applyOn = (uint8_t) lua_tonumber(L, 1);
		int32_t value = (int32_t) lua_tonumber(L, 2);
		int32_t duration = (int32_t) lua_tonumber(L, 3);

		if (applyOn >= MAX_APPLY_NUM || applyOn < 1)
		{
			sys_err("apply is out of range : %d", applyOn);
			return 0;
		}

		CQuestManager & q = CQuestManager::Instance();
		LPCHARACTER ch = q.GetCurrentCharacterPtr();

		if (!ch)
			return 0;

		if (ch->FindAffect(AFFECT_QUEST_START_IDX, applyOn)) // 퀘스트로 인해 같은 곳에 효과가 걸려있으면 스킵
			return 0;

		ch->AddAffect(AFFECT_QUEST_START_IDX, aApplyInfo[applyOn].bPointType, value, 0, duration, 0, false);

		return 0;
	}

	int32_t affect_remove(lua_State * L)
	{
		PC* pPC = CQuestManager::Instance().GetCurrentPC();
		if (!pPC)
		{
			sys_err("Null pc pointer triggered at %s:%d", __FILE__, __LINE__);
			return 0;
		}

		int32_t iType;

		if (lua_isnumber(L, 1))
		{
			iType = (int32_t) lua_tonumber(L, 1);

			if (iType == 0)
				iType = pPC->GetCurrentQuestIndex() + AFFECT_QUEST_START_IDX;
		}
		else
			iType = pPC->GetCurrentQuestIndex() + AFFECT_QUEST_START_IDX;

		LPCHARACTER ch = CQuestManager::Instance().GetCurrentCharacterPtr();
		if (!ch)
			return 0;

		ch->RemoveAffect(iType);

		return 0;
	}

	int32_t affect_remove_bad(lua_State * L) // 나쁜 효과를 없앰
	{
		LPCHARACTER ch = CQuestManager::Instance().GetCurrentCharacterPtr();
		if (ch)
			ch->RemoveBadAffect();
		return 0;
	}

	int32_t affect_remove_good(lua_State * L) // 좋은 효과를 없앰
	{
		LPCHARACTER ch = CQuestManager::Instance().GetCurrentCharacterPtr();
		if (ch)
			ch->RemoveGoodAffect();
		return 0;
	}

	int32_t affect_add_hair(lua_State * L)
	{
		if (!lua_isnumber(L, 1) || !lua_isnumber(L, 2) || !lua_isnumber(L, 3))
		{
			sys_err("invalid argument");
			return 0;
		}
		uint8_t applyOn = (uint8_t) lua_tonumber(L, 1);
		int32_t value = (int32_t) lua_tonumber(L, 2);
		int32_t duration = (int32_t) lua_tonumber(L, 3);

		if (applyOn >= MAX_APPLY_NUM || applyOn < 1)
		{
			sys_err("apply is out of range : %d", applyOn);
			return 0;
		}

		CQuestManager & q = CQuestManager::Instance();
		LPCHARACTER ch = q.GetCurrentCharacterPtr();

		if (ch)
			ch->AddAffect(AFFECT_HAIR, aApplyInfo[applyOn].bPointType, value, 0, duration, 0, false);

		return 0;
	}

	int32_t affect_remove_hair(lua_State * L) // 헤어 효과를 없앤다.
	{
		LPCHARACTER ch = CQuestManager::Instance().GetCurrentCharacterPtr();
		if (!ch)
		{
			sys_err("null ch ptr");
			return 0;
		}

		CAffect* pkAff = ch->FindAffect( AFFECT_HAIR );
		if (pkAff)
		{
			lua_pushnumber(L, pkAff->lDuration);
			ch->RemoveAffect( pkAff );
		}
		else
		{
			lua_pushnumber(L, 0);
		}

		return 1;
	}
	
	// 현재 캐릭터가 AFFECT_TYPE affect를 갖고있으면 bApplyOn 값을 반환하고 없으면 nil을 반환하는 함수.
	// usage :	applyOn = affect.get_apply(AFFECT_TYPE) 
	int32_t affect_get_apply_on(lua_State * L)
	{
		if (!lua_isnumber(L, 1))
		{
			sys_err("invalid argument");
			return 0;
		}
		uint32_t affectType = (uint32_t)lua_tonumber(L, 1);

		LPCHARACTER ch = CQuestManager::Instance().GetCurrentCharacterPtr();
		if (!ch)
		{
			sys_err("null ch ptr");
			return 0;
		}

		CAffect* pkAff = ch->FindAffect(affectType);

		if (pkAff)
			lua_pushnumber(L, pkAff->bApplyOn);
		else
			lua_pushnil(L);

		return 1;
	}

	int32_t affect_add_collect(lua_State * L)
	{
		if (!lua_isnumber(L, 1) || !lua_isnumber(L, 2) || !lua_isnumber(L, 3))
		{
			sys_err("invalid argument");
			return 0;
		}
		uint8_t applyOn = (uint8_t) lua_tonumber(L, 1);
		int32_t value = (int32_t) lua_tonumber(L, 2);
		int32_t duration = (int32_t) lua_tonumber(L, 3);

		if (applyOn >= MAX_APPLY_NUM || applyOn < 1)
		{
			sys_err("apply is out of range : %d", applyOn);
			return 0;
		}

		CQuestManager & q = CQuestManager::Instance();
		LPCHARACTER ch = q.GetCurrentCharacterPtr();

		if (ch)
			ch->AddAffect(AFFECT_COLLECT, aApplyInfo[applyOn].bPointType, value, 0, duration, 0, false);

		return 0;
	}

	int32_t affect_add_collect_point(lua_State * L)
	{
		if (!lua_isnumber(L, 1) || !lua_isnumber(L, 2) || !lua_isnumber(L, 3))
		{
			sys_err("invalid argument");
			return 0;
		}
		uint8_t point_type = (uint8_t) lua_tonumber(L, 1);
		int32_t value = (int32_t) lua_tonumber(L, 2);
		int32_t duration = (int32_t) lua_tonumber(L, 3);

		if (point_type >= POINT_MAX_NUM || point_type < 1)
		{
			sys_err("point is out of range : %d", point_type);
			return 0;
		}

		CQuestManager & q = CQuestManager::Instance();
		LPCHARACTER ch = q.GetCurrentCharacterPtr();

		if (ch)
			ch->AddAffect(AFFECT_COLLECT, point_type, value, 0, duration, 0, false);

		return 0;
	}

	int32_t affect_remove_collect(lua_State* L)
	{
		if (!lua_isnumber(L, 1) || !lua_isnumber(L, 2))
		{
			sys_err("invalid argument");
			return 0;
		}
		uint8_t bApply = static_cast<uint8_t>(lua_tonumber(L, 1));
		int32_t value = static_cast<int32_t>(lua_tonumber(L, 2));

		LPCHARACTER ch = CQuestManager::Instance().GetCurrentCharacterPtr();
		if (!ch)
		{
			sys_err("null ch ptr");
			return 0;
		}

		if (bApply >= MAX_APPLY_NUM) 
			return 0;

		bApply = aApplyInfo[bApply].bPointType;

		CAffect* pAffect = nullptr;
		auto& rList = ch->GetAffectContainer();
		for (auto & iter : rList)
		{
			pAffect = iter;
			if (iter->dwType == AFFECT_COLLECT)
			{
				if (iter->bApplyOn == bApply && iter->lApplyValue == value)
				{
					break;
				}
			}
			pAffect = nullptr;
		}

		if (pAffect)
			ch->RemoveAffect(pAffect);

		return 0;
	}

	int32_t affect_remove_all_collect( lua_State* L )
	{
		LPCHARACTER ch = CQuestManager::Instance().GetCurrentCharacterPtr();
		if (ch)
			ch->RemoveAffect(AFFECT_COLLECT);

		return 0;
	}

	int32_t affect_find(lua_State* L)
	{
		if (!lua_isnumber(L, 1))
		{
			sys_err("invalid argument");
			return 0;
		}
		uint32_t dwType = (uint32_t)lua_tonumber(L, 1);

		CHARACTER* ch = CQuestManager::Instance().GetCurrentCharacterPtr();
		if (!ch)
		{
			sys_err("Null character pointer triggered at %s:%d", __FILE__, __LINE__);
			return 0;
		}

		uint8_t bApply = lua_isnumber(L, 2) ? (uint8_t)lua_tonumber(L, 2) : APPLY_NONE;

		lua_pushboolean(L, ch->FindAffect(dwType, bApply) ? true : false);
		return 1;
	}


	void RegisterAffectFunctionTable()
	{
		luaL_reg affect_functions[] =
		{
			{ "add",		affect_add		},
			{ "add_hair",			affect_add_hair				},
			{ "add_collect",		affect_add_collect			},
			{ "add_collect_point",	affect_add_collect_point	},

			{ "remove",				affect_remove				},
			{ "remove_bad",			affect_remove_bad			},
			{ "remove_good",		affect_remove_good			},
			{ "remove_hair",		affect_remove_hair			},
			{ "remove_collect",		affect_remove_collect		},
			{ "remove_all_collect",	affect_remove_all_collect	},

			{ "get_apply_on",		affect_get_apply_on			},

			{ "find",				affect_find					},

			{ nullptr,				nullptr						}
		};

		CQuestManager::Instance().AddLuaFunctionTable("affect", affect_functions);
	}
};
