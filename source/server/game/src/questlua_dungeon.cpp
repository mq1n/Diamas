#include "stdafx.h"
#include "constants.h"
#include "quest_manager.h"
#include "questlua.h"
#include "dungeon.h"
#include "char.h"
#include "party.h"
#include "buffer_manager.h"
#include "char_manager.h"
#include "packet.h"
#include "desc_client.h"
#include "desc_manager.h"
#include "guild.h"
#include "utils.h"
#include "config.h"
#include "guild_manager.h"
#include "../../common/stl.h"
#include "db.h"
#include "affect.h"
#include "p2p.h"
#include "war_map.h"
#include "sectree_manager.h"

#undef sys_err
#ifndef __WIN32__
#define sys_err(fmt, args...) quest::CQuestManager::instance().QuestError(__FUNCTION__, __LINE__, fmt, ##args)
#else
#define sys_err(fmt, ...) quest::CQuestManager::instance().QuestError(__FUNCTION__, __LINE__, fmt, __VA_ARGS__)
#endif

template <class Func> Func CDungeon::ForEachMember(Func f)
{
	for (auto const& ch : m_set_pkCharacter) 
	{
		if (ch)
		{
			sys_log(0, "Dungeon ForEachMember %s", ch->GetName());
			f(ch);
		}
	}

	return f;
}

namespace quest
{
	//
	// "dungeon" lua functions
	//
	int32_t dungeon_notice(lua_State* L)
	{
		if (!lua_isstring(L, 1))
			return 0;

		CQuestManager& q = CQuestManager::instance();
		LPDUNGEON pDungeon = q.GetCurrentDungeon();

		if (pDungeon)
			pDungeon->Notice(lua_tostring(L, 1));
		return 0;
	}

	int32_t dungeon_set_quest_flag(lua_State* L)
	{
		PC* pPC = CQuestManager::instance().GetCurrentPC();
		if (!pPC)
		{
			sys_err("Null pc pointer triggered at %s:%d", __FILE__, __LINE__);
			return 0;
		}

		if (!lua_isstring(L, 1) || !lua_isnumber(L, 2))
			return 0;

		auto flagname = pPC->GetCurrentQuestName() + "." + lua_tostring(L, 1);
		auto value = static_cast<int32_t>(rint(lua_tonumber(L, 2)));

		auto pDungeon = CQuestManager::instance().GetCurrentDungeon();
		if (pDungeon)
		{
			pDungeon->ForEachMember([&](CHARACTER* ch) 
			{
				if (!ch || !ch->IsPC())
					return;

				auto pPC2 = CQuestManager::instance().GetPCForce(ch->GetPlayerID());
				if (pPC2)
					pPC2->SetFlag(flagname, value);
			});
		}
		return 0;
	}

	int32_t dungeon_set_flag(lua_State* L)
	{
		if (!lua_isstring(L,1) || !lua_isnumber(L,2))
		{
			sys_err("wrong set flag");
			return 0;
		} 
		const char* sz = lua_tostring(L, 1);
		int32_t value = int32_t(lua_tonumber(L, 2));

		CQuestManager& q = CQuestManager::instance();
		LPDUNGEON pDungeon = q.GetCurrentDungeon();
		if (!pDungeon) 
		{
			sys_err("no dungeon !!!");
			return 0;
		}

		pDungeon->SetFlag(sz, value);

		return 0;
	}

	int32_t dungeon_get_flag(lua_State* L)
	{
		if (!lua_isstring(L,1))
		{
			sys_err("wrong get flag");
			return 0;
		}
		const char* sz = lua_tostring(L, 1);

		CQuestManager& q = CQuestManager::instance();
		LPDUNGEON pDungeon = q.GetCurrentDungeon();

		if (!pDungeon)
		{
			sys_err("no dungeon !!!");

			lua_pushnumber(L, 0);
			return 1;
		}	

		lua_pushnumber(L, pDungeon->GetFlag(sz));
		return 1;
	}

	int32_t dungeon_get_flag_from_map_index(lua_State* L)
	{
		if (!lua_isstring(L, 1) || !lua_isnumber(L, 2)) 
		{
			sys_err("wrong get flag");
			return 0;
		}
		const char* sz = lua_tostring(L, 1);

		uint32_t dwMapIndex = static_cast<uint32_t>(lua_tonumber(L, 2));
		if (!dwMapIndex)
		{
			lua_pushboolean(L, false);
			return 1;
		}

		LPDUNGEON pDungeon = CDungeonManager::instance().FindByMapIndex(dwMapIndex);
		if (!pDungeon)
		{
			sys_err("no dungeon !!!");

			lua_pushnumber(L, 0);
			return 1;
		}

		lua_pushnumber(L, pDungeon->GetFlag(sz));
		return 1;
	}

	int32_t dungeon_get_map_index(lua_State* L)
	{
		CQuestManager& q = CQuestManager::instance();
		LPDUNGEON pDungeon = q.GetCurrentDungeon();

		if (pDungeon)
		{
			sys_log(0, "Dungeon GetMapIndex %d",pDungeon->GetMapIndex());
			lua_pushnumber(L, pDungeon->GetMapIndex());
		}
		else
		{
			sys_err("no dungeon !!!");
			lua_pushnumber(L, 0);
		}

		return 1;
	}

	int32_t dungeon_regen_file(lua_State* L)
	{
		if (!lua_isstring(L,1))
		{
			sys_err("wrong filename");
			return 0;
		}

		CQuestManager& q = CQuestManager::instance();
		LPDUNGEON pDungeon = q.GetCurrentDungeon();

		if (pDungeon)
			pDungeon->SpawnRegen(lua_tostring(L,1));

		return 0;
	}

	int32_t dungeon_set_regen_file(lua_State* L)
	{
		if (!lua_isstring(L,1))
		{
			sys_err("wrong filename");
			return 0;
		}
		CQuestManager& q = CQuestManager::instance();
		LPDUNGEON pDungeon = q.GetCurrentDungeon();

		if (pDungeon)
			pDungeon->SpawnRegen(lua_tostring(L,1), false);
		return 0;
	}

	int32_t dungeon_clear_regen(lua_State* L)
	{
		CQuestManager& q = CQuestManager::instance();
		LPDUNGEON pDungeon = q.GetCurrentDungeon();
		if (pDungeon)
			pDungeon->ClearRegen();
		return 0;
	}

	int32_t dungeon_check_eliminated(lua_State* L)
	{
		CQuestManager& q = CQuestManager::instance();
		LPDUNGEON pDungeon = q.GetCurrentDungeon();
		if (pDungeon)
			pDungeon->CheckEliminated();
		return 0;
	}

	int32_t dungeon_set_exit_all_at_eliminate(lua_State* L)
	{
		if (!lua_isnumber(L,1))
		{
			sys_err("wrong time");
			return 0;
		}

		CQuestManager& q = CQuestManager::instance();
		LPDUNGEON pDungeon = q.GetCurrentDungeon();

		if (pDungeon)
			pDungeon->SetExitAllAtEliminate(static_cast<int32_t>(lua_tonumber(L, 1)));

		return 0;
	}

	int32_t dungeon_set_warp_at_eliminate(lua_State* L)
	{
		if (!lua_isnumber(L, 1))
		{
			sys_err("wrong time");
			return 0;
		}

		if (!lua_isnumber(L, 2))
		{
			sys_err("wrong map index");
			return 0;
		}

		if (!lua_isnumber(L, 3)) 
		{
			sys_err("wrong X");
			return 0;
		}

		if (!lua_isnumber(L, 4))
		{
			sys_err("wrong Y");
			return 0;
		}

		const char * c_pszRegenFile = nullptr;

		if (lua_gettop(L) >= 5)
			c_pszRegenFile = lua_tostring(L,5);

		CQuestManager& q = CQuestManager::instance();
		LPDUNGEON pDungeon = q.GetCurrentDungeon();
		if (!pDungeon)
		{
			sys_err("cannot find dungeon");
			return 0;
		}

		pDungeon->SetWarpAtEliminate(
			static_cast<int32_t>(lua_tonumber(L, 1)), static_cast<int32_t>(lua_tonumber(L, 2)),
			static_cast<int32_t>(lua_tonumber(L, 3)), static_cast<int32_t>(lua_tonumber(L, 4)), 
			c_pszRegenFile
		);
		return 0;
	}

	int32_t dungeon_new_jump(lua_State* L)
	{
		if (lua_gettop(L) < 3)
		{
			sys_err("not enough argument");
			return 0;
		}

		if (!lua_isnumber(L, 1) || !lua_isnumber(L, 2) || !lua_isnumber(L, 3))
		{
			sys_err("wrong argument");
			return 0;
		}
		int32_t lMapIndex = static_cast<int32_t>(lua_tonumber(L, 1));

		LPDUNGEON pDungeon = CDungeonManager::instance().Create(lMapIndex);
		if (!pDungeon) 
		{
			sys_err("cannot create dungeon %d", lMapIndex);
			return 0;
		}

		CHARACTER* ch = CQuestManager::instance().GetCurrentCharacterPtr();
		if (!ch)
		{
			sys_err("null ch ptr");
			return 0;
		}

		ch->WarpSet(static_cast<int32_t>(lua_tonumber(L, 2)), static_cast<int32_t>(lua_tonumber(L, 3)), pDungeon->GetMapIndex());
		return 0;
	}

	int32_t dungeon_new_jump_all(lua_State* L)
	{
		if (lua_gettop(L)<3 || !lua_isnumber(L,1) || !lua_isnumber(L, 2) || !lua_isnumber(L,3))
		{
			sys_err("not enough argument");
			return 0;
		}
		auto lMapIndex = static_cast<int32_t>(lua_tonumber(L, 1));

		auto pDungeon = CDungeonManager::instance().Create(lMapIndex);
		if (!pDungeon) 
		{
			sys_err("cannot create dungeon %d", lMapIndex);
			return 0;
		}

		auto ch = CQuestManager::instance().GetCurrentCharacterPtr();
		if (!ch)
		{
			sys_err("null ch ptr");
			return 0;
		}

		pDungeon->JumpAll(ch->GetMapIndex(), static_cast<int32_t>(lua_tonumber(L, 2)), static_cast<int32_t>(lua_tonumber(L, 3)));

		return 0;
	}

	int32_t dungeon_new_jump_party (lua_State* L)
	{
		if (lua_gettop(L)<3 || !lua_isnumber(L,1) || !lua_isnumber(L, 2) || !lua_isnumber(L,3))
		{
			sys_err("not enough argument");
			return 0;
		}
		int32_t lMapIndex = static_cast<int32_t>(lua_tonumber(L, 1));

		LPDUNGEON pDungeon = CDungeonManager::instance().Create(lMapIndex);
		if (!pDungeon) 
		{
			sys_err("cannot create dungeon %d", lMapIndex);
			return 0;
		}

		CHARACTER* ch = CQuestManager::instance().GetCurrentCharacterPtr();
		if (!ch)
		{
			sys_err("null ch ptr");
			return 0;
		}

		if (!ch->GetParty())
		{
			sys_err("cannot go to dungeon alone.");
			return 0;
		}

		pDungeon->JumpParty(ch->GetParty(), ch->GetMapIndex(), static_cast<int32_t>(lua_tonumber(L, 2)), static_cast<int32_t>(lua_tonumber(L, 3)));

		return 0;
	}

	int32_t dungeon_jump_all(lua_State* L)
	{
		if (lua_gettop(L)<2 || !lua_isnumber(L, 1) || !lua_isnumber(L,2))
			return 0;

		LPDUNGEON pDungeon = CQuestManager::instance().GetCurrentDungeon();
		if (!pDungeon)
			return 0;

		pDungeon->JumpAll(pDungeon->GetMapIndex(), static_cast<int32_t>(lua_tonumber(L, 1)), static_cast<int32_t>(lua_tonumber(L, 2)));
		return 0;
	}

	int32_t dungeon_warp_all(lua_State* L)
	{
		if (lua_gettop(L)<2 || !lua_isnumber(L, 1) || !lua_isnumber(L,2))
			return 0;

		LPDUNGEON pDungeon = CQuestManager::instance().GetCurrentDungeon();
		if (!pDungeon)
			return 0;

		pDungeon->WarpAll(pDungeon->GetMapIndex(), static_cast<int32_t>(lua_tonumber(L, 1)), static_cast<int32_t>(lua_tonumber(L, 2)));
		return 0;
	}

	int32_t dungeon_get_kill_stone_count(lua_State* L)
	{
		if (!lua_isnumber(L,1) || !lua_isnumber(L,2))
		{
			lua_pushnumber(L, 0);
			return 1;
		}


		CQuestManager& q = CQuestManager::instance();
		LPDUNGEON pDungeon = q.GetCurrentDungeon();

		if (pDungeon)
		{
			lua_pushnumber(L, pDungeon->GetKillStoneCount());
			return 1;
		}

		lua_pushnumber(L, 0);
		return 1;
	}

	int32_t dungeon_get_kill_mob_count(lua_State * L)
	{
		if (!lua_isnumber(L,1) || !lua_isnumber(L,2))
		{
			lua_pushnumber(L, 0);
			return 1;
		}

		CQuestManager& q = CQuestManager::instance();
		LPDUNGEON pDungeon = q.GetCurrentDungeon();

		if (pDungeon)
		{
			lua_pushnumber(L, pDungeon->GetKillMobCount());
			return 1;
		}

		lua_pushnumber(L, 0);
		return 1;
	}

	int32_t dungeon_is_use_potion(lua_State* L)
	{
		if (!lua_isnumber(L, 1) || !lua_isnumber(L, 2))
		{
			lua_pushboolean(L, true);
			return 1;
		}

		CQuestManager& q = CQuestManager::instance();
		LPDUNGEON pDungeon = q.GetCurrentDungeon();

		if (pDungeon)
		{
			lua_pushboolean(L, pDungeon->IsUsePotion());
			return 1;
		}

		lua_pushboolean(L, true);
		return 1;
	}

	int32_t dungeon_revived(lua_State* L) 
	{
		if (!lua_isnumber(L,1) || !lua_isnumber(L,2))
		{
			lua_pushboolean(L, true);
			return 1;
		}

		CQuestManager& q = CQuestManager::instance();
		LPDUNGEON pDungeon = q.GetCurrentDungeon();

		if (pDungeon)
		{
			lua_pushboolean(L, pDungeon->IsUseRevive());
			return 1;
		}

		lua_pushboolean(L, true);
		return 1;
	}

	int32_t dungeon_set_dest(lua_State* L)
	{
		if (!lua_isnumber(L,1) || !lua_isnumber(L,2))
			return 0;

		CHARACTER* ch = CQuestManager::instance().GetCurrentCharacterPtr();
		if (!ch)
			return 0;

		LPPARTY pParty = ch->GetParty();
		CQuestManager& q = CQuestManager::instance();
		LPDUNGEON pDungeon = q.GetCurrentDungeon();

		if (pDungeon && pParty)
			pDungeon->SendDestPositionToParty(pParty, static_cast<int32_t>(lua_tonumber(L, 1)), static_cast<int32_t>(lua_tonumber(L, 2)));

		return 0;
	}

	int32_t dungeon_unique_set_maxhp(lua_State* L)
	{
		if (!lua_isstring(L,1) || !lua_isnumber(L,2))
			return 0;

		CQuestManager& q = CQuestManager::instance();
		LPDUNGEON pDungeon = q.GetCurrentDungeon();

		if (pDungeon)
			pDungeon->UniqueSetMaxHP(lua_tostring(L, 1), static_cast<int32_t>(lua_tonumber(L, 2)));

		return 0;
	}

	int32_t dungeon_unique_set_hp(lua_State* L)
	{
		if (!lua_isstring(L,1) || !lua_isnumber(L,2))
			return 0;

		CQuestManager& q = CQuestManager::instance();
		LPDUNGEON pDungeon = q.GetCurrentDungeon();

		if (pDungeon)
			pDungeon->UniqueSetHP(lua_tostring(L, 1), static_cast<int32_t>(lua_tonumber(L, 2)));

		return 0;
	}

	int32_t dungeon_unique_set_def_grade(lua_State* L)
	{
		if (!lua_isstring(L,1) || !lua_isnumber(L,2))
			return 0;

		CQuestManager& q = CQuestManager::instance();
		LPDUNGEON pDungeon = q.GetCurrentDungeon();

		if (pDungeon)
			pDungeon->UniqueSetDefGrade(lua_tostring(L, 1), static_cast<int32_t>(lua_tonumber(L, 2)));

		return 0;
	}

	int32_t dungeon_unique_get_hp_perc(lua_State* L)
	{
		if (!lua_isstring(L,1))
		{
			lua_pushnumber(L,0);
			return 1;
		}

		CQuestManager& q = CQuestManager::instance();
		LPDUNGEON pDungeon = q.GetCurrentDungeon();

		if (pDungeon)
		{
			lua_pushnumber(L, pDungeon->GetUniqueHpPerc(lua_tostring(L,1)));
			return 1;
		}

		lua_pushnumber(L,0);
		return 1;
	}

	int32_t dungeon_is_unique_dead(lua_State* L)
	{
		if (!lua_isstring(L,1))
		{
			lua_pushboolean(L, false);
			return 1;
		}

		CQuestManager& q = CQuestManager::instance();
		LPDUNGEON pDungeon = q.GetCurrentDungeon();

		if (pDungeon) 
		{
			lua_pushboolean(L, pDungeon->IsUniqueDead(lua_tostring(L, 1)) ? true : false);
			return 1;
		}

		lua_pushboolean(L, false);
		return 1;
	}

	int32_t dungeon_purge_unique(lua_State* L)
	{
		if (!lua_isstring(L,1))
			return 0;

		sys_log(0, "QUEST_DUNGEON_PURGE_UNIQUE %s", lua_tostring(L, 1));

		CQuestManager& q = CQuestManager::instance();
		LPDUNGEON pDungeon = q.GetCurrentDungeon();

		if (pDungeon)
			pDungeon->PurgeUnique(lua_tostring(L,1));

		return 0;
	}

	struct FPurgeArea
	{
		int32_t x1, y1, x2, y2;
		CHARACTER* ExceptChar;

		FPurgeArea(int32_t a, int32_t b, int32_t c, int32_t d, CHARACTER* p) :
			x1(a), y1(b), x2(c), y2(d), ExceptChar(p)
		{
		}

		void operator()(LPENTITY ent) const
		{
			if (true == ent->IsType(ENTITY_CHARACTER)) 
			{
				auto pChar = static_cast<CHARACTER*>(ent);

				if (!pChar || pChar == ExceptChar)
					return;
					
				if (!pChar->IsPet() && (true == pChar->IsMonster() || true == pChar->IsStone()))
				{
					if (x1 <= pChar->GetX() && pChar->GetX() <= x2 && y1 <= pChar->GetY() && pChar->GetY() <= y2)
					{
						M2_DESTROY_CHARACTER(pChar);
					}
				}
			}
		}
	};
	
	int32_t dungeon_purge_area(lua_State* L)
	{
		if (!lua_isnumber(L,1) || !lua_isnumber(L,2) || !lua_isnumber(L,3) || !lua_isnumber(L,4))
			return 0;

		sys_log(0, "QUEST_DUNGEON_PURGE_AREA");

		auto x1 = static_cast<int32_t>(lua_tonumber(L, 1));
		auto y1 = static_cast<int32_t>(lua_tonumber(L, 2));
		auto x2 = static_cast<int32_t>(lua_tonumber(L, 3));
		auto y2 = static_cast<int32_t>(lua_tonumber(L, 4));

		auto& q = CQuestManager::instance();
		auto pDungeon = q.GetCurrentDungeon();
		if (!pDungeon)
			return 0;

		const auto mapIndex = pDungeon->GetMapIndex();
		if (0 == mapIndex) 
		{
			sys_err("_purge_area: cannot get a map index with (%u, %u)", x1, y1);
			return 0;
		}

		LPSECTREE_MAP pSectree = SECTREE_MANAGER::instance().GetMap(mapIndex);
		if (pSectree) 
		{
			FPurgeArea func(x1, y1, x2, y2, CQuestManager::instance().GetCurrentNPCCharacterPtr());
			pSectree->for_each(func);
		}

		return 0;
	}

	int32_t dungeon_kill_unique(lua_State* L)
	{
		if (!lua_isstring(L,1))
			return 0;

		auto pChar = CQuestManager::instance().GetCurrentCharacterPtr();
		if (!pChar)
		{
			sys_err("Attempt to kill unique monster without char!? Cowardly refusing..");
			return 0;
		}

		sys_log(0,"QUEST_DUNGEON_KILL_UNIQUE %s", lua_tostring(L,1));

		CQuestManager& q = CQuestManager::instance();
		LPDUNGEON pDungeon = q.GetCurrentDungeon();

		if (pDungeon)
			pDungeon->KillUnique(lua_tostring(L,1));

		return 0;
	}

	int32_t dungeon_spawn_stone_door(lua_State* L)
	{
		if (!lua_isstring(L,1) || !lua_isstring(L,2))
			return 0;
		sys_log(0,"QUEST_DUNGEON_SPAWN_STONE_DOOR %s %s", lua_tostring(L,1), lua_tostring(L,2));

		CQuestManager& q = CQuestManager::instance();
		LPDUNGEON pDungeon = q.GetCurrentDungeon();

		if (pDungeon)
			pDungeon->SpawnStoneDoor(lua_tostring(L,1), lua_tostring(L,2));

		return 0;
	}

	int32_t dungeon_spawn_wooden_door(lua_State* L)
	{
		if (!lua_isstring(L,1) || !lua_isstring(L,2))
			return 0;
		sys_log(0,"QUEST_DUNGEON_SPAWN_WOODEN_DOOR %s %s", lua_tostring(L,1), lua_tostring(L,2));

		CQuestManager& q = CQuestManager::instance();
		LPDUNGEON pDungeon = q.GetCurrentDungeon();

		if (pDungeon)
			pDungeon->SpawnWoodenDoor(lua_tostring(L,1), lua_tostring(L,2));

		return 0;
	}

	int32_t dungeon_spawn_move_group(lua_State* L)
	{
		if (!lua_isnumber(L,1) || !lua_isstring(L,2) || !lua_isstring(L,3))
			return 0;

		sys_log(0, "QUEST_DUNGEON_SPAWN_MOVE_GROUP %d %s %s", static_cast<int32_t>(lua_tonumber(L, 1)), lua_tostring(L, 2), lua_tostring(L, 3));

		auto& q = CQuestManager::instance();
		auto pDungeon = q.GetCurrentDungeon();

		if (pDungeon)
			pDungeon->SpawnMoveGroup(static_cast<uint32_t>(lua_tonumber(L, 1)), lua_tostring(L, 2), lua_tostring(L, 3));

		return 0;
	}

	int32_t dungeon_spawn_move_unique(lua_State* L)
	{
		if (!lua_isstring(L,1) || !lua_isnumber(L,2) || !lua_isstring(L,3) || !lua_isstring(L,4))
			return 0;
		sys_log(0, "QUEST_DUNGEON_SPAWN_MOVE_UNIQUE %s %d %s %s", lua_tostring(L, 1), static_cast<int32_t>(lua_tonumber(L, 2)), lua_tostring(L, 3), lua_tostring(L, 4));

		CQuestManager& q = CQuestManager::instance();
		LPDUNGEON pDungeon = q.GetCurrentDungeon();

		if (pDungeon)
			pDungeon->SpawnMoveUnique(lua_tostring(L, 1), static_cast<uint32_t>(lua_tonumber(L, 2)), lua_tostring(L, 3), lua_tostring(L, 4));

		return 0;
	}

	int32_t dungeon_spawn_unique(lua_State* L)
	{
		if (!lua_isstring(L,1) || !lua_isnumber(L,2) || !lua_isstring(L,3))
			return 0;
		sys_log(0, "QUEST_DUNGEON_SPAWN_UNIQUE %s %d %s", lua_tostring(L, 1), static_cast<int32_t>(lua_tonumber(L, 2)), lua_tostring(L, 3));

		CQuestManager& q = CQuestManager::instance();
		LPDUNGEON pDungeon = q.GetCurrentDungeon();

		if (pDungeon)
			pDungeon->SpawnUnique(lua_tostring(L, 1), static_cast<uint32_t>(lua_tonumber(L, 2)), lua_tostring(L, 3));

		return 0;
	}

	int32_t dungeon_spawn(lua_State* L)
	{
		if (!lua_isnumber(L,1) || !lua_isstring(L,2))
			return 0;

		sys_log(0, "QUEST_DUNGEON_SPAWN %d %s", static_cast<int32_t>(lua_tonumber(L, 1)), lua_tostring(L, 2));

		CQuestManager& q = CQuestManager::instance();
		LPDUNGEON pDungeon = q.GetCurrentDungeon();

		if (pDungeon)
			pDungeon->Spawn(static_cast<uint32_t>(lua_tonumber(L, 1)), lua_tostring(L, 2));

		return 0;
	}

	int32_t dungeon_set_unique(lua_State* L)
	{
		if (!lua_isstring(L, 1) || !lua_isnumber(L, 2))
			return 0;

		uint32_t vid = static_cast<uint32_t>(lua_tonumber(L, 2));

		CQuestManager& q = CQuestManager::instance();
		LPDUNGEON pDungeon = q.GetCurrentDungeon();

		if (pDungeon)
			pDungeon->SetUnique(lua_tostring(L, 1), vid);

		return 0;
	}

	int32_t dungeon_get_unique_vid(lua_State* L)
	{
		if (!lua_isstring(L,1))
		{
			lua_pushnumber(L,0);
			return 1;
		}

		CQuestManager& q = CQuestManager::instance();
		LPDUNGEON pDungeon = q.GetCurrentDungeon();

		if (pDungeon)
		{
			lua_pushnumber(L, pDungeon->GetUniqueVid(lua_tostring(L,1)));
			return 1;
		}

		lua_pushnumber(L,0);
		return 1;
	}

	int32_t dungeon_spawn_mob(lua_State* L)
	{
		if (!lua_isnumber(L, 1) || !lua_isnumber(L, 2) || !lua_isnumber(L, 3))
		{
			sys_err("invalid argument");
			return 0;
		}

		CQuestManager& q = CQuestManager::instance();
		LPDUNGEON pDungeon = q.GetCurrentDungeon();

		uint32_t vid = 0;

		if (pDungeon)
		{
			uint32_t dwVnum = static_cast<uint32_t>(lua_tonumber(L, 1));
			int32_t x = static_cast<int32_t>(lua_tonumber(L, 2));
			int32_t y = static_cast<int32_t>(lua_tonumber(L, 3));
			float radius = lua_isnumber(L, 4) ? static_cast<float>(lua_tonumber(L, 4)) : 0;
			uint32_t count = lua_isnumber(L, 5) ? static_cast<uint32_t>(lua_tonumber(L, 5)) : 1;
			bool noReward = lua_toboolean(L, 6);

			sys_log(0, "dungeon_spawn_mob %u %d %d", dwVnum, x, y);

			if (count <= 0)
				count = 1;

			if (count > 200)
				count = 200;

			while (count--) 
			{
				CHARACTER* ch;

				if (radius < 1) 
				{
					ch = pDungeon->SpawnMob(dwVnum, x, y);
				} 
				else
				{
					double angle = fnumber(0.0f, 999.9f) * M_PI * 2.0f / 1000.0f;
					double r = fnumber(0.0f, 999.9f) * radius / 1000.0f;

					int32_t nx = x + static_cast<int32_t>(r * cos(angle));
					int32_t ny = y + static_cast<int32_t>(r * sin(angle));

					ch = pDungeon->SpawnMob(dwVnum, nx, ny);
				}

				if (ch) 
				{
					if (noReward) {
						ch->SetNoRewardFlag();
					}

					if (!vid) {
						vid = ch->GetVID();
					}
				}
			}
		}

		lua_pushnumber(L, vid);
		return 1;
	}

	int32_t dungeon_spawn_mob_dir(lua_State* L)
	{
		if (!lua_isnumber(L, 1) || !lua_isnumber(L, 2) || !lua_isnumber(L, 3) || !lua_isnumber(L, 4))
		{
			sys_err("invalid argument");
			return 0;
		}

		CQuestManager& q = CQuestManager::instance();
		LPDUNGEON pDungeon = q.GetCurrentDungeon();

		uint32_t vid = 0;

		if (pDungeon)
		{
			uint32_t dwVnum = static_cast<uint32_t>(lua_tonumber(L, 1));
			int32_t x = static_cast<int32_t>(lua_tonumber(L, 2));
			int32_t y = static_cast<int32_t>(lua_tonumber(L, 3));
			uint8_t dir = static_cast<int32_t>(lua_tonumber(L, 4));

			CHARACTER* ch = pDungeon->SpawnMob(dwVnum, x, y, dir);
			if (ch)
				vid = ch->GetVID();
		}
		lua_pushnumber(L, vid);
		return 1;
	}
	
	int32_t dungeon_spawn_mob_ac_dir(lua_State* L)
	{
		if (!lua_isnumber(L, 1) || !lua_isnumber(L, 2) || !lua_isnumber(L, 3) || !lua_isnumber(L, 4))
		{
			sys_err("invalid argument");
			return 0;
		}

		CQuestManager& q = CQuestManager::instance();
		LPDUNGEON pDungeon = q.GetCurrentDungeon();

		uint32_t vid = 0;

		if (pDungeon)
		{
			uint32_t dwVnum = static_cast<uint32_t>(lua_tonumber(L, 1));
			int32_t x = static_cast<int32_t>(lua_tonumber(L, 2));
			int32_t y = static_cast<int32_t>(lua_tonumber(L, 3));
			uint8_t dir = static_cast<int32_t>(lua_tonumber(L, 4));

			CHARACTER* ch = pDungeon->SpawnMob_ac_dir(dwVnum, x, y, dir);
			if (ch)
				vid = ch->GetVID();
		}
		lua_pushnumber(L, vid);
		return 1;
	}

	int32_t dungeon_spawn_goto_mob(lua_State* L)
	{
		if (!lua_isnumber(L, 1) || !lua_isnumber(L, 2) || !lua_isnumber(L, 3) || !lua_isnumber(L, 4))
			return 0;

		auto lFromX = static_cast<int32_t>(lua_tonumber(L, 1));
		auto lFromY = static_cast<int32_t>(lua_tonumber(L, 2));
		auto lToX = static_cast<int32_t>(lua_tonumber(L, 3));
		auto lToY = static_cast<int32_t>(lua_tonumber(L, 4));

		auto& q = CQuestManager::instance();
		auto pDungeon = q.GetCurrentDungeon();

		if (pDungeon)
			pDungeon->SpawnGotoMob(lFromX, lFromY, lToX, lToY);

		return 0;
	}

	int32_t dungeon_spawn_name_mob(lua_State* L)
	{
		if (!lua_isnumber(L, 1) || !lua_isnumber(L, 2) || !lua_isnumber(L, 3) || !lua_isstring(L, 4))
			return 0;

		auto& q = CQuestManager::instance();
		auto pDungeon = q.GetCurrentDungeon();
		if (pDungeon)
		{
			auto dwVnum = static_cast<uint32_t>(lua_tonumber(L, 1));
			auto x = static_cast<int32_t>(lua_tonumber(L, 2));
			auto y = static_cast<int32_t>(lua_tonumber(L, 3));

			pDungeon->SpawnNameMob(dwVnum, x, y, lua_tostring(L, 4));
		}
		return 0;
	}

	int32_t dungeon_spawn_group(lua_State* L)
	{
		//
		// argument: vnum,x,y,radius,aggressive,count
		//
		if (!lua_isnumber(L, 1) || !lua_isnumber(L, 2) || !lua_isnumber(L, 3) || !lua_isnumber(L, 4) || !lua_isboolean(L, 5) || !lua_isnumber(L, 6))
		{
			sys_err("invalid argument");
			return 0;
		}

		uint32_t vid = 0;

		auto& q = CQuestManager::instance();
		auto pDungeon = q.GetCurrentDungeon();
		if (pDungeon)
		{
			auto group_vnum = static_cast<uint32_t>(lua_tonumber(L, 1));
			auto local_x = static_cast<int32_t>(lua_tonumber(L, 2)) * 100;
			auto local_y = static_cast<int32_t>(lua_tonumber(L, 3)) * 100;
			auto radius = static_cast<float>(lua_tonumber(L, 4)) * 100;
			bool bAggressive = lua_toboolean(L, 5);
			auto count = static_cast<uint32_t>(lua_tonumber(L, 6));

			auto chRet = pDungeon->SpawnGroup(group_vnum, local_x, local_y, radius, bAggressive, count);
			if (chRet)
				vid = chRet->GetVID();
		}

		lua_pushnumber(L, vid);
		return 1;
	}

	int32_t dungeon_join(lua_State* L)
	{
		if (lua_gettop(L) < 1 || !lua_isnumber(L, 1))
			return 0;

		auto lMapIndex = static_cast<int32_t>(lua_tonumber(L, 1));
		auto pDungeon = CDungeonManager::instance().Create(lMapIndex);
		if (!pDungeon)
			return 0;

		auto ch = CQuestManager::instance().GetCurrentCharacterPtr();
		if (!ch)
			return 0;

		if (ch->GetParty() && ch->GetParty()->GetLeaderPID() == ch->GetPlayerID())
			pDungeon->JoinParty(ch->GetParty());
		else if (!ch->GetParty())
			pDungeon->Join(ch);

		return 0;
	}

	int32_t dungeon_exit(lua_State* L) // 던전에 들어오기 전 위치로 보냄
	{
		auto ch = CQuestManager::instance().GetCurrentCharacterPtr();
		if (ch)
			ch->ExitToSavedLocation();
		return 0;
	}

	int32_t dungeon_exit_all(lua_State* L) // 던전에 있는 모든 사람을 던전에 들어오기 전 위치로 보냄
	{
		auto& q = CQuestManager::instance();
		auto pDungeon = q.GetCurrentDungeon();

		if (pDungeon)
			pDungeon->ExitAll();

		return 0;
	}

	struct FSayDungeonByItemGroup
	{
		const CDungeon::ItemGroup* item_group;
		std::string can_enter_ment;
		std::string cant_enter_ment;
		void operator()(LPENTITY ent)
		{
			if (ent->IsType(ENTITY_CHARACTER))
			{
				auto ch = static_cast<CHARACTER*>(ent);

				if (ch && ch->IsPC() && ch->GetDesc()) 
				{
					packet_script packet_script;
					TEMP_BUFFER buf;

					for (const auto& it : *item_group) 
					{
						if (ch->CountSpecifyItem(it.first) >= it.second)
						{
							packet_script.header = HEADER_GC_SCRIPT;
							packet_script.skin = quest::CQuestManager::QUEST_SKIN_NORMAL;
							packet_script.src_size = static_cast<uint16_t>(can_enter_ment.size());
							packet_script.size = packet_script.src_size + sizeof(struct packet_script);

							buf.write(&packet_script, sizeof(struct packet_script));
							buf.write(&can_enter_ment[0], can_enter_ment.size());
							ch->GetDesc()->Packet(buf.read_peek(), buf.size());
							return;
						}
					}

					packet_script.header = HEADER_GC_SCRIPT;
					packet_script.skin = quest::CQuestManager::QUEST_SKIN_NORMAL;
					packet_script.src_size = static_cast<uint16_t>(cant_enter_ment.size());
					packet_script.size = packet_script.src_size + sizeof(struct packet_script);

					buf.write(&packet_script, sizeof(struct packet_script));
					buf.write(&cant_enter_ment[0], cant_enter_ment.size());
					ch->GetDesc()->Packet(buf.read_peek(), buf.size());
				}
			}
		}
	};


	int32_t dungeon_say_diff_by_item_group(lua_State* L)
	{
		if (!lua_isstring(L, 1) || !lua_isstring(L, 2) || !lua_isstring(L, 3))
		{
			sys_log(0, "QUEST wrong set flag");
			return 0;
		}
		std::string group_name(lua_tostring(L, 1));

		CQuestManager& q = CQuestManager::instance();
		CDungeon * pDungeon = q.GetCurrentDungeon();

		if(!pDungeon)
		{
			sys_err("QUEST : no dungeon");
			return 0;
		}

		SECTREE_MAP * pMap = SECTREE_MANAGER::instance().GetMap(pDungeon->GetMapIndex());

		if (!pMap)
		{
			sys_err("cannot find map by index %d", pDungeon->GetMapIndex());
			return 0;
		}
		FSayDungeonByItemGroup f;
		f.item_group = pDungeon->GetItemGroup (group_name);

		if (f.item_group == nullptr)
		{
			sys_err ("invalid item group");
			return 0;
		}

		f.can_enter_ment = lua_tostring(L, 2);
		f.can_enter_ment+= "[ENTER][ENTER][ENTER][ENTER][DONE]";
		f.cant_enter_ment = lua_tostring(L, 3);
		f.cant_enter_ment+= "[ENTER][ENTER][ENTER][ENTER][DONE]";

		pMap -> for_each( f );

		return 0;
	}

	int32_t dungeon_exit_all_by_item_group (lua_State* L) // 특정 아이템 그룹에 속한 아이템이 없는사람은 강퇴
	{
		if (!lua_isstring(L, 1))
		{
			sys_log(0, "QUEST wrong set flag");
			return 0;
		}
		std::string group_name(lua_tostring(L, 1));

		CQuestManager& q = CQuestManager::instance();
		CDungeon * pDungeon = q.GetCurrentDungeon();

		if(!pDungeon)
		{
			sys_err("QUEST : no dungeon");
			return 0;
		}

		SECTREE_MAP * pMap = SECTREE_MANAGER::instance().GetMap(pDungeon->GetMapIndex());

		if (!pMap)
		{
			sys_err("cannot find map by index %d", pDungeon->GetMapIndex());
			return 0;
		}

		auto f = [&](CEntity* ent) 
		{
			auto ch = static_cast<CHARACTER*>(ent);
			if (ch && ch->IsPC()) 
			{
				for (const auto& it : *pDungeon->GetItemGroup(group_name)) 
				{
					if (ch->CountSpecifyItem(it.first) >= it.second)
						return;
				}
				ch->ExitToSavedLocation();
			}

		};
		pMap->ForEachOfType(f, ENTITY_CHARACTER);

		return 0;
	}

	
	int32_t dungeon_delete_item_in_item_group_from_all(lua_State* L) // 특정 아이템을 던전 내 pc에게서 삭제.
	{
		if (!lua_isstring(L, 1))
		{
			sys_log(0, "QUEST wrong set flag");
			return 0;
		}
		std::string group_name(lua_tostring(L, 1));

		CQuestManager& q = CQuestManager::instance();
		CDungeon * pDungeon = q.GetCurrentDungeon();

		if(!pDungeon)
		{
			sys_err("QUEST : no dungeon");
			return 0;
		}

		SECTREE_MAP * pMap = SECTREE_MANAGER::instance().GetMap(pDungeon->GetMapIndex());

		if (!pMap)
		{
			sys_err("cannot find map by index %d", pDungeon->GetMapIndex());
			return 0;
		}

		auto f = [&](CEntity* ent) 
		{
			auto ch = static_cast<CHARACTER*>(ent);
			if (ch && ch->IsPC())
			{
				for (const auto& it : *pDungeon->GetItemGroup(group_name)) 
				{
					if (ch->CountSpecifyItem(it.first) >= it.second) 
					{
						ch->RemoveSpecifyItem(it.first, it.second);
						return;
					}
				}
			}
		};

		pMap->ForEachOfType(f, ENTITY_CHARACTER);

		return 0;
	}


	int32_t dungeon_kill_all(lua_State* L)
	{
		CQuestManager& q = CQuestManager::instance();
		LPDUNGEON pDungeon = q.GetCurrentDungeon();

		if (pDungeon)
			pDungeon->KillAll();

		return 0;
	}

	int32_t dungeon_purge(lua_State* L)
	{
		CQuestManager& q = CQuestManager::instance();
		LPDUNGEON pDungeon = q.GetCurrentDungeon();

		if (pDungeon)
			pDungeon->Purge();

		return 0;
	}

	int32_t dungeon_exit_all_to_start_position(lua_State * L)
	{
		CQuestManager& q = CQuestManager::instance();
		LPDUNGEON pDungeon = q.GetCurrentDungeon();

		if (pDungeon)
			pDungeon->ExitAllToStartPosition();

		return 0;
	}

	int32_t dungeon_count_monster(lua_State * L)
	{
		CQuestManager& q = CQuestManager::instance();
		LPDUNGEON pDungeon = q.GetCurrentDungeon();

		if (!pDungeon)
		{
			sys_err("not in a dungeon");
			lua_pushnumber(L, LONG_MAX);
			return 1;
		}

		lua_pushnumber(L, pDungeon->CountMonster());
		return 1;
	}

	int32_t dungeon_select(lua_State* L)
	{
		uint32_t dwMapIndex = static_cast<uint32_t>(lua_tonumber(L, 1));
		if (!dwMapIndex)
		{
			CQuestManager::instance().SelectDungeon(nullptr);

			lua_pushboolean(L, false);
			return 1;
		}

		LPDUNGEON pDungeon = CDungeonManager::instance().FindByMapIndex(dwMapIndex);
		if (!pDungeon)
		{
			CQuestManager::instance().SelectDungeon(nullptr);

			lua_pushboolean(L, false);
			return 1;
		}

		CQuestManager::instance().SelectDungeon(pDungeon);

		lua_pushboolean(L, true);
		return 1;
	}

	int32_t dungeon_find(lua_State* L)
	{
		uint32_t dwMapIndex = static_cast<uint32_t>(lua_tonumber(L, 1));
		if (dwMapIndex) 
		{
			LPDUNGEON pDungeon = CDungeonManager::instance().FindByMapIndex(dwMapIndex);
			if (pDungeon)
				lua_pushboolean(L, true);
			else
				lua_pushboolean(L, false);
		}

		lua_pushboolean(L, false);
		return 1;
	}

	int32_t dungeon_all_near_to( lua_State* L)
	{
		if (lua_gettop(L) < 2 || !lua_isnumber(L, 1) || !lua_isnumber(L, 2))
			return 0;

		LPDUNGEON pDungeon = CQuestManager::instance().GetCurrentDungeon();
		if (pDungeon) 
		{
			lua_pushboolean(L, pDungeon->IsAllPCNearTo(static_cast<int32_t>(lua_tonumber(L, 1)), static_cast<int32_t>(lua_tonumber(L, 2)), 30));
			return 1;
		} 

		lua_pushboolean(L, false);
		return 1;
	}

	int32_t dungeon_set_warp_location (lua_State* L)
	{
		LPDUNGEON pDungeon = CQuestManager::instance().GetCurrentDungeon();
		if (!pDungeon)
			return 0;

		if (lua_gettop(L) < 3 || !lua_isnumber(L, 1) || !lua_isnumber(L, 2) || !lua_isnumber(L, 3))
			return 0;

		pDungeon->ForEachMember([&](CHARACTER* ch)
		{
			if (ch && ch->IsPC()) 
			{
				ch->SetWarpLocation((int32_t)lua_tonumber(L, 1), (int32_t)lua_tonumber(L, 2), (int32_t)lua_tonumber(L, 3));
			}
		});

		return 0;
	}

	int32_t dungeon_set_item_group (lua_State* L)
	{
		if (!lua_isstring (L, 1) || !lua_isnumber (L, 2))
		{
			return 0;
		}
		std::string group_name(lua_tostring(L, 1));
		int32_t size = static_cast<int32_t>(lua_tonumber(L, 2));

		CDungeon::ItemGroup item_group;

		for (int32_t i = 0; i < size; i++) 
		{
			if (!lua_isnumber(L, i * 2 + 3) || !lua_isnumber(L, i * 2 + 4))
				return 0;

			item_group.push_back(std::pair<uint32_t, int32_t>
			(
				static_cast<uint32_t>(lua_tonumber(L, i * 2 + 3)),
				static_cast<int32_t>(lua_tonumber(L, i * 2 + 4)))
			);
		}

		LPDUNGEON pDungeon = CQuestManager::instance().GetCurrentDungeon();
		if (!pDungeon) 
			return 0;

		pDungeon->CreateItemGroup(group_name, item_group);
		return 0;
	}

	int32_t dungeon_set_quest_flag2(lua_State* L)
	{
		if (!lua_isstring(L, 1) || !lua_isstring(L, 2) || !lua_isnumber(L, 3)) 
		{
			sys_err("Invalid Argument");
			return 0;
		}
		auto flagname = std::string(lua_tostring(L, 1)) + "." + lua_tostring(L, 2);
		auto value = static_cast<int32_t>(rint(lua_tonumber(L, 3)));

		auto& q = CQuestManager::instance();
		auto pDungeon = q.GetCurrentDungeon();

		if (pDungeon)
		{
			pDungeon->ForEachMember([&](CHARACTER* ch) 
			{
				if (!ch || !ch->IsPC())
					return;

				auto pPC = CQuestManager::instance().GetPCForce(ch->GetPlayerID());
				if (pPC)
					pPC->SetFlag(flagname, value);
			});
		}
		return 0;
	}

	int32_t dungeon_new(lua_State* L)
	{
		if (lua_gettop(L) < 1)
		{
			sys_err("not enough argument");
			return 0;
		}

		if (!lua_isnumber(L, 1))
		{
			sys_err("wrong argument");
			return 0;
		}

		int32_t lMapIndex = (int32_t)lua_tonumber(L, 1);

		LPDUNGEON pDungeon = CDungeonManager::instance().Create(lMapIndex);

		if (!pDungeon)
		{
			sys_err("cannot create dungeon %d", lMapIndex);
			return 0;
		}

		return 0;
	}

	int32_t dungeon_new_jump_guild(lua_State* L)
	{
		if (lua_gettop(L) < 3 || !lua_isnumber(L, 1) || !lua_isnumber(L, 2) || !lua_isnumber(L, 3))
		{
			sys_err("not enough argument");
			return 0;
		}
		int32_t lMapIndex = (int32_t)lua_tonumber(L, 1);

		LPDUNGEON pDungeon = CDungeonManager::instance().Create(lMapIndex);
		if (!pDungeon)
		{
			sys_err("cannot create dungeon %d", lMapIndex);
			return 0;
		}

		CHARACTER* ch = CQuestManager::instance().GetCurrentCharacterPtr();
		if (!ch || !ch->GetGuild())
		{
			sys_err("cannot go to dungeon alone.");
			return 0;
		}

		pDungeon->JumpGuild(ch->GetGuild(), ch->GetMapIndex(), (int32_t)lua_tonumber(L, 2), (int32_t)lua_tonumber(L, 3));

		return 0;
	}

	int32_t dungeon_is_available(lua_State* L)
	{
		CQuestManager& q = CQuestManager::instance();
		LPDUNGEON pDungeon = q.GetCurrentDungeon();

		lua_pushboolean(L, pDungeon != nullptr);
		return 1;
	}

	void RegisterDungeonFunctionTable() 
	{
		luaL_reg dungeon_functions[] = 
		{
			{ "join",			dungeon_join		},
			{ "exit",			dungeon_exit		},
			{ "exit_all",		dungeon_exit_all	},
			{ "set_item_group",	dungeon_set_item_group	},
			{ "exit_all_by_item_group",	dungeon_exit_all_by_item_group},
			{ "say_diff_by_item_group",	dungeon_say_diff_by_item_group},
			{ "delete_item_in_item_group_from_all", dungeon_delete_item_in_item_group_from_all},
			{ "purge",			dungeon_purge		},
			{ "kill_all",		dungeon_kill_all	},
			{ "spawn",			dungeon_spawn		},
			{ "spawn_mob",		dungeon_spawn_mob	},
			{ "spawn_mob_dir",	dungeon_spawn_mob_dir	},
			{ "spawn_mob_ac_dir",	dungeon_spawn_mob_ac_dir	},
			{ "spawn_name_mob",	dungeon_spawn_name_mob	},
			{ "spawn_goto_mob",		dungeon_spawn_goto_mob	},
			{ "spawn_group",		dungeon_spawn_group	},
			{ "spawn_unique",		dungeon_spawn_unique	},
			{ "spawn_move_unique",		dungeon_spawn_move_unique},
			{ "spawn_move_group",		dungeon_spawn_move_group},
			{ "spawn_stone_door",		dungeon_spawn_stone_door},
			{ "spawn_wooden_door",		dungeon_spawn_wooden_door},
			{ "purge_unique",		dungeon_purge_unique	},
			{ "purge_area",			dungeon_purge_area	},
			{ "kill_unique",		dungeon_kill_unique	},
			{ "is_unique_dead",		dungeon_is_unique_dead	},
			{ "unique_get_hp_perc",		dungeon_unique_get_hp_perc},
			{ "unique_set_def_grade",	dungeon_unique_set_def_grade},
			{ "unique_set_hp",		dungeon_unique_set_hp	},
			{ "unique_set_maxhp",		dungeon_unique_set_maxhp},
			{ "get_unique_vid",		dungeon_get_unique_vid},
			{ "get_kill_stone_count",	dungeon_get_kill_stone_count},
			{ "get_kill_mob_count",		dungeon_get_kill_mob_count},
			{ "is_use_potion",		dungeon_is_use_potion	},
			{ "revived",			dungeon_revived		},
			{ "set_dest",			dungeon_set_dest	},
			{ "jump_all",			dungeon_jump_all	},
			{ "warp_all",		dungeon_warp_all	},
			{ "new_jump_all",		dungeon_new_jump_all	},
			{ "new_jump_party",		dungeon_new_jump_party	},
			{ "new_jump",			dungeon_new_jump	},
			{ "regen_file",			dungeon_regen_file	},
			{ "set_regen_file",		dungeon_set_regen_file	},
			{ "clear_regen",		dungeon_clear_regen	},
			{ "set_exit_all_at_eliminate",	dungeon_set_exit_all_at_eliminate},
			{ "set_warp_at_eliminate",	dungeon_set_warp_at_eliminate},
			{ "get_map_index",		dungeon_get_map_index	},
			{ "check_eliminated",		dungeon_check_eliminated},
			{ "exit_all_to_start_position",	dungeon_exit_all_to_start_position },
			{ "count_monster",		dungeon_count_monster	},
			{ "setf",					dungeon_set_flag	},
			{ "getf",					dungeon_get_flag	},
			{ "getf_from_map_index",	dungeon_get_flag_from_map_index	},
			{ "set_unique",			dungeon_set_unique	},
			{ "select",			dungeon_select		},
			{ "find",			dungeon_find		},
			{ "notice",			dungeon_notice		},
			{ "setqf",			dungeon_set_quest_flag	},
			{ "all_near_to",	dungeon_all_near_to	},
			{ "set_warp_location",	dungeon_set_warp_location	},
			{ "setqf2",			dungeon_set_quest_flag2	},

			{ "new_jump_all_guild",					dungeon_new_jump_guild				},
			{ "new_jump_guild",						dungeon_new_jump_guild				},
			{ "is_available",						dungeon_is_available				},
			{ "new",								dungeon_new							},

			{ nullptr,				nullptr			}
		};

		CQuestManager::instance().AddLuaFunctionTable("d", dungeon_functions);
	}
}
