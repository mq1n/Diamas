#include "stdafx.h"
#include "config.h"
#include "quest_manager.h"
#include "sectree_manager.h"
#include "char.h"
#include "char_manager.h"
#include "affect.h"
#include "item.h"
#include "item_manager.h"
#include "guild_manager.h"
#include "war_map.h"
#include "start_position.h"
#include "marriage.h"
#include "mining.h"
#include "p2p.h"
#include "polymorph.h"
#include "desc_client.h"
#include "messenger_manager.h"
#include "log.h"
#include "utils.h"
#include "unique_item.h"
#include "mob_manager.h"
#include "pvp.h"
#include "battle.h"
#include "pvp.h"
#include "shop.h"
#include "shop_manager.h"
#include "party.h"
#include <cctype>
#include "../../libsql/include/AsyncSQL.h"
#undef sys_err
#ifndef __WIN32__
#define sys_err(fmt, args...) quest::CQuestManager::Instance().QuestError(__FUNCTION__, __LINE__, fmt, ##args)
#else
#define sys_err(fmt, ...) quest::CQuestManager::Instance().QuestError(__FUNCTION__, __LINE__, fmt, __VA_ARGS__)
#endif


const int32_t ITEM_BROKEN_METIN_VNUM = 28960;
extern bool IS_MOUNTABLE_ZONE(int32_t map_index, bool isHorse);

namespace quest 
{
	//
	// "pc" Lua functions
	//
	int32_t pc_has_master_skill(lua_State* L)
	{
		CHARACTER* ch = CQuestManager::Instance().GetCurrentCharacterPtr();
		if (!ch)
		{
			sys_err("Null character pointer triggered at %s:%d", __FILE__, __LINE__);
			return 0;
		}
		bool bHasMasterSkill = false;
		for (int32_t i=0; i< SKILL_MAX_NUM; i++)
		{
			if (ch->GetSkillMasterType(i) >= SKILL_MASTER && ch->GetSkillLevel(i) >= 21) 
			{
				bHasMasterSkill = true;
				break;
			}
		}

		lua_pushboolean(L, bHasMasterSkill);
		return 1;
	}

	int32_t pc_remove_skill_book_no_delay(lua_State* L)
	{
		CHARACTER* ch = CQuestManager::Instance().GetCurrentCharacterPtr();
		if (!ch)
		{
			sys_err("Null character pointer triggered at %s:%d", __FILE__, __LINE__);
			return 0;
		}

		ch->RemoveAffect(AFFECT_SKILL_NO_BOOK_DELAY);
		return 0;
	}

	int32_t pc_is_skill_book_no_delay(lua_State* L)
	{
		CHARACTER* ch = CQuestManager::Instance().GetCurrentCharacterPtr();
		if (!ch)
		{
			sys_err("Null character pointer triggered at %s:%d", __FILE__, __LINE__);
			return 0;
		}

		lua_pushboolean(L, ch->FindAffect(AFFECT_SKILL_NO_BOOK_DELAY) ? true : false);
		return 1;
	}

	int32_t pc_learn_grand_master_skill(lua_State* L)
	{
		CHARACTER* ch = CQuestManager::Instance().GetCurrentCharacterPtr();
		if (!ch)
		{
			sys_err("Null character pointer triggered at %s:%d", __FILE__, __LINE__);
			return 0;
		}

		if (!lua_isnumber(L, 1)) 
		{
			sys_err("wrong skill index");
			return 0;
		}

		lua_pushboolean(L, ch->LearnGrandMasterSkill((int32_t)lua_tonumber(L, 1)));
		return 1;
	}

	int32_t pc_set_warp_location(lua_State * L)
	{
		CHARACTER* ch = CQuestManager::Instance().GetCurrentCharacterPtr();
		if (!ch)
		{
			sys_err("Null character pointer triggered at %s:%d", __FILE__, __LINE__);
			return 0;
		}

		if (!lua_isnumber(L, 1)) 
		{
			sys_err("wrong map index");
			return 0;
		}

		if (!lua_isnumber(L, 2) || !lua_isnumber(L, 3))
		{
			sys_err("wrong coodinate");
			return 0;
		}

		ch->SetWarpLocation((int32_t)lua_tonumber(L,1), (int32_t)lua_tonumber(L,2), (int32_t)lua_tonumber(L,3));
		return 0;
	}

	int32_t pc_set_warp_location_local(lua_State * L)
	{
		CHARACTER* ch = CQuestManager::Instance().GetCurrentCharacterPtr();
		if (!ch)
		{
			sys_err("Null character pointer triggered at %s:%d", __FILE__, __LINE__);
			return 0;
		}

		if (!lua_isnumber(L, 1)) 
		{
			sys_err("wrong map index");
			return 0;
		}

		if (!lua_isnumber(L, 2) || !lua_isnumber(L, 3))
		{
			sys_err("wrong coodinate");
			return 0;
		}

		int32_t lMapIndex = (int32_t) lua_tonumber(L, 1);

		auto region = SECTREE_MANAGER::Instance().GetMapRegion(lMapIndex);
		if (!region)
		{
			sys_err("invalid map index %d", lMapIndex);
			return 0;
		}

		int32_t x = (int32_t) lua_tonumber(L, 2);
		int32_t y = (int32_t) lua_tonumber(L, 3);

		if (x > region->ex - region->sx)
		{
			sys_err("x coordinate overflow max: %d input: %d", region->ex - region->sx, x);
			return 0;
		}

		if (y > region->ey - region->sy)
		{
			sys_err("y coordinate overflow max: %d input: %d", region->ey - region->sy, y);
			return 0;
		}

		ch->SetWarpLocation(lMapIndex, x, y);
		return 0;
	}

	int32_t pc_get_start_location(lua_State * L)
	{
		CHARACTER* ch = CQuestManager::Instance().GetCurrentCharacterPtr();
		if (!ch)
		{
			sys_err("Null character pointer triggered at %s:%d", __FILE__, __LINE__);
			return 0;
		}

		lua_pushnumber(L, g_start_map[ch->GetEmpire()]);
		lua_pushnumber(L, g_start_position[ch->GetEmpire()][0] / 100);
		lua_pushnumber(L, g_start_position[ch->GetEmpire()][1] / 100);
		return 3;
	}

	int32_t pc_warp(lua_State * L)
	{
		CHARACTER* ch = CQuestManager::Instance().GetCurrentCharacterPtr();
		if (!ch)
		{
			sys_err("Null character pointer triggered at %s:%d", __FILE__, __LINE__);
			return 0;
		}

		if (!lua_isnumber(L, 1) || !lua_isnumber(L, 2))
		{
			lua_pushboolean(L, false);
			return 1;
		}

		int32_t map_index = 0;

		if (lua_isnumber(L, 3))
			map_index = (int32_t) lua_tonumber(L,3);

		//PREVENT_HACK
		if ( ch->IsHack() )
		{
			lua_pushboolean(L, false);
			return 1;
		}
		//END_PREVENT_HACK
	
		if ( g_bIsTestServer )
		{
			ch->ChatPacket(CHAT_TYPE_INFO, "pc_warp %d %d %d", (int32_t)lua_tonumber(L, 1), (int32_t)lua_tonumber(L, 2), map_index);
		}
		ch->WarpSet((int32_t)lua_tonumber(L, 1), (int32_t)lua_tonumber(L, 2), map_index);
		
		lua_pushboolean(L, true);

		return 1;
	}

	int32_t pc_warp_local(lua_State * L)
	{
		CHARACTER* ch = CQuestManager::Instance().GetCurrentCharacterPtr();
		if (!ch)
		{
			sys_err("Null character pointer triggered at %s:%d", __FILE__, __LINE__);
			return 0;
		}

		if (!lua_isnumber(L, 1)) 
		{
			sys_err("no map index argument");
			return 0;
		}

		if (!lua_isnumber(L, 2) || !lua_isnumber(L, 3))
		{
			sys_err("no coodinate argument");
			return 0;
		}

		int32_t lMapIndex = (int32_t) lua_tonumber(L, 1);

		auto region = SECTREE_MANAGER::Instance().GetMapRegion(lMapIndex);
		if (!region)
		{
			sys_err("invalid map index %d", lMapIndex);
			return 0;
		}

		int32_t x = (int32_t) lua_tonumber(L, 2);
		int32_t y = (int32_t) lua_tonumber(L, 3);

		if (x > region->ex - region->sx)
		{
			sys_err("x coordinate overflow max: %d input: %d", region->ex - region->sx, x);
			return 0;
		}

		if (y > region->ey - region->sy)
		{
			sys_err("y coordinate overflow max: %d input: %d", region->ey - region->sy, y);
			return 0;
		}

		ch->WarpSet(region->sx + x, region->sy + y);
		return 0;
	}

	int32_t pc_warp_exit(lua_State * L)
	{
		CHARACTER* ch = CQuestManager::Instance().GetCurrentCharacterPtr();
		if (!ch)
		{
			sys_err("Null character pointer triggered at %s:%d", __FILE__, __LINE__);
			return 0;
		}

		ch->ExitToSavedLocation();
		return 0;
	}

	int32_t pc_in_dungeon(lua_State* L)
	{
		CHARACTER* ch = CQuestManager::Instance().GetCurrentCharacterPtr();
		if (!ch)
		{
			sys_err("Null character pointer triggered at %s:%d", __FILE__, __LINE__);
			return 0;
		}

		lua_pushboolean(L, ch->GetDungeon() ? true : false);
		return 1;
	}

	int32_t pc_hasguild(lua_State* L)
	{
		CHARACTER* ch = CQuestManager::Instance().GetCurrentCharacterPtr();
		if (!ch)
		{
			sys_err("Null character pointer triggered at %s:%d", __FILE__, __LINE__);
			return 0;
		}

		lua_pushboolean(L, ch->GetGuild() ? true : false);
		return 1;
	}

	int32_t pc_getguild(lua_State* L)
	{
		CHARACTER* ch = CQuestManager::Instance().GetCurrentCharacterPtr();
		if (!ch)
		{
			sys_err("Null character pointer triggered at %s:%d", __FILE__, __LINE__);
			return 0;
		}

		lua_pushnumber(L, ch->GetGuild() ? ch->GetGuild()->GetID() : 0);
		return 1;
	}

	int32_t pc_isguildmaster(lua_State * L)
	{
		CHARACTER* ch = CQuestManager::Instance().GetCurrentCharacterPtr();
		if (!ch)
		{
			sys_err("Null character pointer triggered at %s:%d", __FILE__, __LINE__);
			return 0;
		}

		CGuild* g = ch->GetGuild();
		if (!g)
		{
			lua_pushboolean(L, false);
			return 1;
		}

		lua_pushboolean(L, (ch->GetPlayerID() == g->GetMasterPID()));
		return 1;
	}

	int32_t pc_destroy_guild(lua_State * L)
	{
		CHARACTER* ch = CQuestManager::Instance().GetCurrentCharacterPtr();
		if (!ch)
		{
			sys_err("Null character pointer triggered at %s:%d", __FILE__, __LINE__);
			return 0;
		}

		CGuild* g = ch->GetGuild();
		if (!g)
		{
			sys_err("Null guild pointer triggered at %s:%d", __FILE__, __LINE__);
			return 0;
		}

		g->RequestDisband(ch->GetPlayerID());
		return 0;
	}

	int32_t pc_remove_from_guild(lua_State* L)
	{
		CHARACTER* ch = CQuestManager::Instance().GetCurrentCharacterPtr();
		if (!ch)
		{
			sys_err("Null character pointer triggered at %s:%d", __FILE__, __LINE__);
			return 0;
		}

		CGuild* g = ch->GetGuild();
		if (!g)
		{
			sys_err("Null guild pointer triggered at %s:%d", __FILE__, __LINE__);
			return 0;
		}

		g->RequestRemoveMember(ch->GetPlayerID());
		return 0;
	}

	int32_t pc_give_gold(lua_State* L)
	{
		CHARACTER* ch = CQuestManager::Instance().GetCurrentCharacterPtr();
		if (!ch)
		{
			sys_err("Null character pointer triggered at %s:%d", __FILE__, __LINE__);
			return 0;
		}

		if (!lua_isnumber(L, 1))
		{
			sys_err("QUEST : wrong argument");
			return 0;
		}

		int32_t iAmount = (int32_t) lua_tonumber(L, 1);

		if (iAmount <= 0)
		{
			sys_err("QUEST : gold amount less then zero");
			return 0;
		}

		LogManager::Instance().MoneyLog(MONEY_LOG_QUEST, ch->GetPlayerID(), iAmount);
		ch->PointChange(POINT_GOLD, iAmount, true);
		return 0;
	}

	int32_t pc_warp_to_guild_war_observer_position(lua_State* L)
	{
		CHARACTER* ch = CQuestManager::Instance().GetCurrentCharacterPtr();
		if (!ch)
		{
			sys_err("Null character pointer triggered at %s:%d", __FILE__, __LINE__);
			return 0;
		}

		if (!lua_isnumber(L, 1) || !lua_isnumber(L, 2))
		{
			sys_err("invalid param");
			return 0;
		}

		uint32_t gid1 = (uint32_t)lua_tonumber(L, 1);
		uint32_t gid2 = (uint32_t)lua_tonumber(L, 2);

		CGuild* g1 = CGuildManager::Instance().FindGuild(gid1);
		if (!g1) 
		{
			luaL_error(L, "no such guild with id %d %d", gid1, gid2);
			return 0;
		}

		GPOS pos;
		uint32_t dwMapIndex = g1->GetGuildWarMapIndex(gid2);

		if (!CWarMapManager::Instance().GetStartPosition(dwMapIndex, 2, pos)) 
		{
			luaL_error(L, "not under warp guild war between guild %d %d", gid1, gid2);
			return 0;
		}

		//PREVENT_HACK
		if ( ch->IsHack() )
			return 0;
		//END_PREVENT_HACK

		ch->SetQuestFlag("war.is_war_member", 0);
		ch->SaveExitLocation();
		ch->WarpSet(pos.x, pos.y, dwMapIndex);
		return 0;
	}

	int32_t pc_give_item_from_special_item_group(lua_State* L)
	{
		CHARACTER* ch = CQuestManager::Instance().GetCurrentCharacterPtr();
		if (!ch)
		{
			sys_err("Null character pointer triggered at %s:%d", __FILE__, __LINE__);
			return 0;
		}

		if (!lua_isnumber(L, 1))
		{
			sys_err("invalid param");
			return 0;
		}

		uint32_t dwGroupVnum = (uint32_t)lua_tonumber(L, 1);

		std::vector<uint32_t> dwVnums;
		std::vector<uint32_t> dwCounts;
		std::vector<CItem*> item_gets(0);

		int32_t count = 0;

		if (!ch->GiveItemFromSpecialItemGroup(dwGroupVnum, dwVnums, dwCounts, item_gets, count))
		{
			sys_err("GiveItemFromSpecialItemGroup fail");
			return 0;
		}

		for (int32_t i = 0; i < count; i++)
		{
			if (!item_gets[i])
			{
				if (dwVnums[i] == 1)
				{
					ch->ChatPacket(CHAT_TYPE_INFO, ("YOU_GAINED_%d_GOLD"), dwCounts[i]);
				}

				else if (dwVnums[i] == 2)
				{
					ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("나무에서 부터 신비한 빛이 나옵니다."));
					ch->ChatPacket(CHAT_TYPE_INFO, ("YOU_GAINED_%d_EXP"), dwCounts[i]);
				}
			}
		}
		return 0;
	}

	int32_t pc_enough_inventory(lua_State* L)
	{
		CHARACTER* ch = CQuestManager::Instance().GetCurrentCharacterPtr();
		if (!ch)
		{
			sys_err("Null character pointer triggered at %s:%d", __FILE__, __LINE__);
			return 0;
		}
		if (!lua_isnumber(L, 1))
		{
			lua_pushboolean(L, false);
			return 1;
		}

		uint32_t item_vnum = (uint32_t)lua_tonumber(L, 1);
		SItemTable_Server* pTable = ITEM_MANAGER::Instance().GetTable(item_vnum);
		if (!pTable) {
			lua_pushboolean(L, false);
			return 1;
		}

		bool bEnoughInventoryForItem = ch->GetEmptyInventory(pTable->bSize) != -1;
		lua_pushboolean(L, bEnoughInventoryForItem);
		return 1;
	}

	int32_t pc_give_item(lua_State* L)
	{
		PC* pPC = CQuestManager::Instance().GetCurrentPC();
		if (!pPC)
		{
			sys_err("Null pc pointer triggered at %s:%d", __FILE__, __LINE__);
			return 0;
		}

		CHARACTER* ch = CQuestManager::Instance().GetCurrentCharacterPtr();
		if (!ch)
		{
			sys_err("Null character pointer triggered at %s:%d", __FILE__, __LINE__);
			return 0;
		}

		if (!lua_isstring(L, 1) || !(lua_isstring(L, 2) || lua_isnumber(L, 2)))
		{
			sys_err("QUEST : wrong argument");
			return 0;
		}

		uint32_t dwVnum = 0;
		if (lua_isnumber(L, 2))
		{
			dwVnum = (uint32_t)lua_tonumber(L, 2);
		}
		else if (!ITEM_MANAGER::Instance().GetVnum(lua_tostring(L, 2), dwVnum))
		{
			sys_err("QUEST Make item call error : wrong item name : %s", lua_tostring(L,1));
			return 0;
		}

		int32_t icount = 1;

		if (lua_isnumber(L, 3) && lua_tonumber(L, 3) > 0)
		{
			icount = (int32_t)rint(lua_tonumber(L, 3));

			if (icount <= 0) 
			{
				sys_err("QUEST Make item call error : wrong item count : %g", lua_tonumber(L, 2));
				return 0;
			}
		}

		pPC->GiveItem(lua_tostring(L, 1), dwVnum, icount);

		LogManager::Instance().QuestRewardLog(pPC->GetCurrentQuestName().c_str(), ch->GetPlayerID(), ch->GetLevel(), dwVnum, icount);
		return 0;
	}

	int32_t pc_give_or_drop_item(lua_State* L)
	{
		PC* pPC = CQuestManager::Instance().GetCurrentPC();
		if (!pPC)
		{
			sys_err("Null pc pointer triggered at %s:%d", __FILE__, __LINE__);
			return 0;
		}

		CHARACTER* ch = CQuestManager::Instance().GetCurrentCharacterPtr();
		if (!ch)
		{
			sys_err("Null character pointer triggered at %s:%d", __FILE__, __LINE__);
			return 0;
		}

		if (!lua_isstring(L, 1) && !lua_isnumber(L, 1)) 
		{
			sys_err("QUEST Make item call error : wrong argument");
			lua_pushnumber (L, 0);
			return 1;
		}

		uint32_t dwVnum = 0;
		if (lua_isnumber(L, 1))
		{
			dwVnum = (uint32_t)lua_tonumber(L, 1);
		}
		else if (!ITEM_MANAGER::Instance().GetVnum(lua_tostring(L, 1), dwVnum))
		{
			sys_err("QUEST Make item call error : wrong item name : %s", lua_tostring(L,1));
			lua_pushnumber (L, 0);

			return 1;
		}

		int32_t icount = 1;
		if (lua_isnumber(L,2) && lua_tonumber(L,2)>0)
		{
			icount = (int32_t)rint(lua_tonumber(L,2));
			if (icount<=0) 
			{
				sys_err("QUEST Make item call error : wrong item count : %g", lua_tonumber(L, 2));
				lua_pushnumber(L, 0);
				return 1;
			}
		}

		if (lua_isstring(L, 1))
			sys_log(0, "QUEST [REWARD] item %s to %s", lua_tostring(L, 1), ch->GetName());
		else
			sys_log(0, "QUEST [REWARD] item %u to %s", (uint32_t)lua_tonumber(L, 1), ch->GetName());

		LogManager::Instance().QuestRewardLog(pPC->GetCurrentQuestName().c_str(), ch->GetPlayerID(), ch->GetLevel(), dwVnum, icount);

		CItem* item = ch->AutoGiveItem(dwVnum, icount);
		if (item) 
		{
			if ((dwVnum >= 80003 && dwVnum <= 80007) ||
				(dwVnum >= 80018 && dwVnum <= 80020)) 
			{
				LogManager::Instance().GoldBarLog(ch->GetPlayerID(), item->GetID(), QUEST, "quest: give_item2");
			}
			lua_pushnumber(L, item->GetID());
		} 
		else
		{
			lua_pushnumber(L, 0);
		}
		return 1;
	}

	#ifdef ENABLE_DICE_SYSTEM
	int32_t pc_give_or_drop_item_with_dice(lua_State* L)
	{
		PC* pPC = CQuestManager::Instance().GetCurrentPC();
		if (!pPC)
		{
			sys_err("Null pc pointer triggered at %s:%d", __FILE__, __LINE__);
			return 0;
		}

		CHARACTER* ch = CQuestManager::Instance().GetCurrentCharacterPtr();
		if (!ch)
		{
			sys_err("Null character pointer triggered at %s:%d", __FILE__, __LINE__);
			return 0;
		}

		if (!lua_isstring(L, 1) && !lua_isnumber(L, 1)) 
		{
			sys_err("QUEST Make item call error : wrong argument");
			lua_pushnumber(L, 0);
			return 1;
		}

		uint32_t dwVnum = 0;
		if (lua_isnumber(L, 1))
		{
			dwVnum = static_cast<uint32_t>(lua_tonumber(L, 1));
		} 
		else if (!ITEM_MANAGER::Instance().GetVnum(lua_tostring(L, 1), dwVnum))
		{
			sys_err("QUEST Make item call error : wrong item name : %s", lua_tostring(L, 1));
			lua_pushnumber(L, 0);
			return 1;
		}

		int32_t icount = 1;
		if (lua_isnumber(L, 2) && lua_tonumber(L, 2) > 0) 
		{
			icount = static_cast<int32_t>(rint(lua_tonumber(L, 2)));
			if (icount <= 0) 
			{
				sys_err("QUEST Make item call error : wrong item count : %g", lua_tonumber(L, 2));
				lua_pushnumber(L, 0);
				return 1;
			}
		}

		CItem* item = ch->AutoGiveItem(dwVnum, icount);
		if (!item)
		{
			sys_err("Null item pointer triggered at %s:%d", __FILE__, __LINE__);
			return 0;
		}

		if (ch->GetParty()) 
		{
			FPartyDropDiceRoll f(item, ch);
			f.Process(nullptr);
		}

		if (lua_isstring(L, 1))
			sys_log(0, "QUEST [REWARD] item %s to %s", lua_tostring(L, 1), ch->GetName());
		else
			sys_log(0, "QUEST [REWARD] item %u to %s", (uint32_t)lua_tonumber(L, 1), ch->GetName());

		LogManager::Instance().QuestRewardLog(pPC->GetCurrentQuestName().c_str(), ch->GetPlayerID(), ch->GetLevel(), dwVnum, icount);

		lua_pushnumber(L, item->GetID());
		return 1;
	}
	#endif

	int32_t pc_give_or_drop_item_and_select(lua_State* L)
	{
		PC* pPC = CQuestManager::Instance().GetCurrentPC();
		if (!pPC)
		{
			sys_err("Null pc pointer triggered at %s:%d", __FILE__, __LINE__);
			return 0;
		}

		CHARACTER* ch = CQuestManager::Instance().GetCurrentCharacterPtr();
		if (!ch)
		{
			sys_err("Null character pointer triggered at %s:%d", __FILE__, __LINE__);
			return 0;
		}

		if (!lua_isstring(L, 1) && !lua_isnumber(L, 1))
		{
			sys_err("QUEST Make item call error : wrong argument");
			return 0;
		}

		uint32_t dwVnum = 0;
		if (lua_isnumber(L, 1))
		{
			dwVnum = (uint32_t)lua_tonumber(L, 1);
		}
		else if (!ITEM_MANAGER::Instance().GetVnum(lua_tostring(L, 1), dwVnum))
		{
			sys_err("QUEST Make item call error : wrong item name : %s", lua_tostring(L,1));
			return 0;
		}

		int32_t icount = 1;
		if (lua_isnumber(L,2) && lua_tonumber(L,2)>0)
		{
			icount = (int32_t)rint(lua_tonumber(L,2));
			if (icount<=0) 
			{
				sys_err("QUEST Make item call error : wrong item count : %g", lua_tonumber(L,2));
				return 0;
			}
		}

		if (lua_isstring(L, 1))
			sys_log(0, "QUEST [REWARD] item %s to %s", lua_tostring(L, 1), ch->GetName());
		else
			sys_log(0, "QUEST [REWARD] item %u to %s", (uint32_t)lua_tonumber(L, 1), ch->GetName());

		LogManager::Instance().QuestRewardLog(pPC->GetCurrentQuestName().c_str(), ch->GetPlayerID(), ch->GetLevel(), dwVnum, icount);

		CItem* item = ch->AutoGiveItem(dwVnum, icount);
		if (!item)
		{
			sys_err("Null item pointer triggered at %s:%d", __FILE__, __LINE__);
			return 0;
		}

		CQuestManager::Instance().SetCurrentItem(item);
		CQuestManager::Instance().UpdateStateItem(ch, item);

		if ((dwVnum >= 80003 && dwVnum <= 80007) ||
			(dwVnum >= 80018 && dwVnum <= 80020))
		{
			LogManager::Instance().GoldBarLog(ch->GetPlayerID(), item->GetID(), QUEST, "quest: give_item2");
		}
		
		return 0;
	}

	int32_t pc_get_current_map_index(lua_State* L)
	{
		CHARACTER* ch = CQuestManager::Instance().GetCurrentCharacterPtr();
		if (!ch)
		{
			sys_err("Null character pointer triggered at %s:%d", __FILE__, __LINE__);
			return 0;
		}

		lua_pushnumber(L, ch->GetMapIndex());
		return 1;
	}

	int32_t pc_get_x(lua_State* L)
	{
		CHARACTER* ch = CQuestManager::Instance().GetCurrentCharacterPtr();
		if (!ch)
		{
			sys_err("Null character pointer triggered at %s:%d", __FILE__, __LINE__);
			return 0;
		}

		lua_pushnumber(L, ch->GetX() / 100);
		return 1;
	}

	int32_t pc_get_y(lua_State* L)
	{
		CHARACTER* ch = CQuestManager::Instance().GetCurrentCharacterPtr();
		if (!ch)
		{
			sys_err("Null character pointer triggered at %s:%d", __FILE__, __LINE__);
			return 0;
		}

		lua_pushnumber(L, ch->GetY() / 100);
		return 1;
	}

	int32_t pc_get_local_x(lua_State* L)
	{
		CHARACTER* ch = CQuestManager::Instance().GetCurrentCharacterPtr();
		if (!ch)
		{
			sys_err("Null character pointer triggered at %s:%d", __FILE__, __LINE__);
			return 0;
		}

		LPSECTREE_MAP pMap = SECTREE_MANAGER::Instance().GetMap(ch->GetMapIndex());

		if (pMap)
			lua_pushnumber(L, (ch->GetX() - pMap->m_setting.iBaseX) / 100);
		else
			lua_pushnumber(L, ch->GetX() / 100);

		return 1;
	}

	int32_t pc_get_local_y(lua_State* L)
	{
		CHARACTER* ch = CQuestManager::Instance().GetCurrentCharacterPtr();
		if (!ch)
		{
			sys_err("Null character pointer triggered at %s:%d", __FILE__, __LINE__);
			return 0;
		}

		LPSECTREE_MAP pMap = SECTREE_MANAGER::Instance().GetMap(ch->GetMapIndex());

		if (pMap)
			lua_pushnumber(L, (ch->GetY() - pMap->m_setting.iBaseY) / 100);
		else
			lua_pushnumber(L, ch->GetY() / 100);

		return 1;
	}

	int32_t pc_count_item(lua_State* L)
	{
		CHARACTER* ch = CQuestManager::Instance().GetCurrentCharacterPtr();
		if (!ch)
		{
			sys_err("Null character pointer triggered at %s:%d", __FILE__, __LINE__);
			return 0;
		}

		if (lua_isnumber(L, -1))
		{
			lua_pushnumber(L, ch->CountSpecifyItem((uint32_t)lua_tonumber(L, -1)));
		}
		else if (lua_isstring(L, -1)) 
		{
			uint32_t item_vnum = 0;
			if (!ITEM_MANAGER::Instance().GetVnum(lua_tostring(L, 1), item_vnum)) 
			{
				sys_err("QUEST count_item call error : wrong item name : %s", lua_tostring(L, 1));
				lua_pushnumber(L, 0);
			} 
			else 
			{
				lua_pushnumber(L, ch->CountSpecifyItem(item_vnum));
			}
		} 
		else
		{
			lua_pushnumber(L, 0);
		}

		return 1;
	}

	int32_t pc_find_first_item(lua_State* L)
	{
		CHARACTER* ch = CQuestManager::Instance().GetCurrentCharacterPtr();
		if (!ch)
		{
			sys_err("Null character pointer triggered at %s:%d", __FILE__, __LINE__);
			return 0;
		}

		if (!lua_isnumber(L, -1)) 
		{
			lua_pushnumber(L, -1);
			return 1;
		}

		CItem* item = ch->FindSpecifyItem((uint32_t)lua_tonumber(L, -1));
		if (!item) 
		{
			lua_pushnumber(L, -1);
			return 1;
		}

		lua_pushnumber(L, item->GetCell());
		return 1;
	}

	int32_t pc_remove_item(lua_State* L)
	{
		CHARACTER* ch = CQuestManager::Instance().GetCurrentCharacterPtr();
		if (!ch)
		{
			sys_err("Null character pointer triggered at %s:%d", __FILE__, __LINE__);
			return 0;
		}

		if (lua_gettop(L) == 1)
		{
			uint32_t item_vnum = 0;

			if (lua_isnumber(L, 1)) 
			{
				item_vnum = (uint32_t)lua_tonumber(L, 1);
			}
			else if (lua_isstring(L,1))
			{
				if (!ITEM_MANAGER::Instance().GetVnum(lua_tostring(L,1), item_vnum))
				{
					sys_err("QUEST remove_item call error : wrong item name : %s", lua_tostring(L,1));
					return 0;
				}
			}
			else
			{
				sys_err("QUEST remove_item wrong argument");
				return 0;
			}

			sys_log(0, "QUEST remove a item vnum %d of %s[%d]", item_vnum, ch->GetName(), ch->GetPlayerID());
			ch->RemoveSpecifyItem(item_vnum);
		} 
		else if (lua_gettop(L) == 2) 
		{
			uint32_t item_vnum = 0;

			if (lua_isnumber(L, 1))
			{
				item_vnum = (uint32_t)lua_tonumber(L, 1);
			}
			else if (lua_isstring(L, 1))
			{
				if (!ITEM_MANAGER::Instance().GetVnum(lua_tostring(L,1), item_vnum))
				{
					sys_err("QUEST remove_item call error : wrong item name : %s", lua_tostring(L,1));
					return 0;
				}
			}
			else
			{
				sys_err("QUEST remove_item wrong argument");
				return 0;
			}

			if (!lua_isnumber(L, 2))
			{
				sys_err("wrong 2.arg");
				return 0;
			}

			uint32_t item_count = (uint32_t)lua_tonumber(L, 2);
			if (item_count <= 0)
			{
				sys_err("wrong item count");
				return 0;
			}

			sys_log(0, "QUEST remove items(vnum %d) count %d of %s[%d]", item_vnum, item_count, ch->GetName(), ch->GetPlayerID());
			ch->RemoveSpecifyItem(item_vnum, item_count);
		}
		return 0;
	}

	int32_t pc_get_leadership(lua_State * L)
	{
		CHARACTER* ch = CQuestManager::Instance().GetCurrentCharacterPtr();
		if (!ch)
		{
			sys_err("Null character pointer triggered at %s:%d", __FILE__, __LINE__);
			return 0;
		}

		lua_pushnumber(L, ch->GetLeadershipSkillLevel());
		return 1;
	}

	int32_t pc_reset_point(lua_State * L)
	{
		CHARACTER* ch = CQuestManager::Instance().GetCurrentCharacterPtr();
		if (!ch)
		{
			sys_err("Null character pointer triggered at %s:%d", __FILE__, __LINE__);
			return 0;
		}

		ch->ResetPoint(ch->GetLevel());
		return 0;
	}

	int32_t pc_get_playtime(lua_State* L)
	{
		CHARACTER* ch = CQuestManager::Instance().GetCurrentCharacterPtr();
		if (!ch)
		{
			sys_err("Null character pointer triggered at %s:%d", __FILE__, __LINE__);
			return 0;
		}

		lua_pushnumber(L, static_cast<int32_t>(ch->GetRealPoint(POINT_PLAYTIME)));
		return 1;
	}

	int32_t pc_get_vid(lua_State* L)
	{
		CHARACTER* ch = CQuestManager::Instance().GetCurrentCharacterPtr();
		if (!ch)
		{
			sys_err("Null character pointer triggered at %s:%d", __FILE__, __LINE__);
			return 0;
		}

		lua_pushnumber(L, ch->GetVID());
		return 1;
	}
	int32_t pc_get_name(lua_State* L)
	{
		CHARACTER* ch = CQuestManager::Instance().GetCurrentCharacterPtr();
		if (!ch)
		{
			sys_err("Null character pointer triggered at %s:%d", __FILE__, __LINE__);
			return 0;
		}

		lua_pushstring(L, ch->GetName());
		return 1;
	}

	int32_t pc_get_next_exp(lua_State* L)  
	{
		CHARACTER* ch = CQuestManager::Instance().GetCurrentCharacterPtr();
		if (!ch)
		{
			sys_err("Null character pointer triggered at %s:%d", __FILE__, __LINE__);
			return 0;
		}

		lua_pushnumber(L, ch->GetNextExp());
		return 1;
	}

	int32_t pc_get_exp(lua_State* L)  
	{
		CHARACTER* ch = CQuestManager::Instance().GetCurrentCharacterPtr();
		if (!ch)
		{
			sys_err("Null character pointer triggered at %s:%d", __FILE__, __LINE__);
			return 0;
		}

		lua_pushnumber(L, ch->GetExp());
		return 1;
	}

	int32_t pc_get_race(lua_State* L)  
	{
		CHARACTER* ch = CQuestManager::Instance().GetCurrentCharacterPtr();
		if (!ch)
		{
			sys_err("Null character pointer triggered at %s:%d", __FILE__, __LINE__);
			return 0;
		}

		lua_pushnumber(L, ch->GetRaceNum());
		return 1;
	}

	int32_t pc_change_sex(lua_State* L)  
	{
		CHARACTER* ch = CQuestManager::Instance().GetCurrentCharacterPtr();
		if (!ch)
		{
			sys_err("Null character pointer triggered at %s:%d", __FILE__, __LINE__);
			return 0;
		}

		lua_pushnumber(L, ch->ChangeSex());
		return 1;
	}

	int32_t pc_get_job(lua_State* L)  
	{
		CHARACTER* ch = CQuestManager::Instance().GetCurrentCharacterPtr();
		if (!ch)
		{
			sys_err("Null character pointer triggered at %s:%d", __FILE__, __LINE__);
			return 0;
		}

		lua_pushnumber(L, ch->GetJob());
		return 1;
	}

	int32_t pc_get_max_sp(lua_State* L)  
	{
		CHARACTER* ch = CQuestManager::Instance().GetCurrentCharacterPtr();
		if (!ch)
		{
			sys_err("Null character pointer triggered at %s:%d", __FILE__, __LINE__);
			return 0;
		}

		lua_pushnumber(L, ch->GetMaxSP());
		return 1;
	}

	int32_t pc_get_sp(lua_State * L)  
	{
		CHARACTER* ch = CQuestManager::Instance().GetCurrentCharacterPtr();
		if (!ch)
		{
			sys_err("Null character pointer triggered at %s:%d", __FILE__, __LINE__);
			return 0;
		}

		lua_pushnumber(L, ch->GetSP());
		return 1;
	}

	int32_t pc_change_sp(lua_State * L)
	{
		CHARACTER* ch = CQuestManager::Instance().GetCurrentCharacterPtr();
		if (!ch)
		{
			sys_err("Null character pointer triggered at %s:%d", __FILE__, __LINE__);
			return 0;
		}

		if (!lua_isnumber(L, 1))
		{
			sys_err("invalid argument");

			lua_pushboolean(L, false);
			return 1;
		}

		int32_t val = (int32_t)lua_tonumber(L, 1);
		if (val == 0) 
		{
			lua_pushboolean(L, false);
			return 1;
		}

		if (val > 0)
		{
			ch->PointChange(POINT_SP, val);
		}
		else
		{
			if (ch->GetSP() < -val)
			{
				lua_pushboolean(L, false);
				return 1;
			}

			ch->PointChange(POINT_SP, val);
		}

		lua_pushboolean(L, true);
		return 1;
	}

	int32_t pc_get_max_hp(lua_State * L)  
	{
		CHARACTER* ch = CQuestManager::Instance().GetCurrentCharacterPtr();
		if (!ch)
		{
			sys_err("Null character pointer triggered at %s:%d", __FILE__, __LINE__);
			return 0;
		}

		lua_pushnumber(L, ch->GetMaxHP());
		return 1;
	}

	int32_t pc_get_hp(lua_State * L)  
	{
		CHARACTER* ch = CQuestManager::Instance().GetCurrentCharacterPtr();
		if (!ch)
		{
			sys_err("Null character pointer triggered at %s:%d", __FILE__, __LINE__);
			return 0;
		}

		lua_pushnumber(L, ch->GetHP());
		return 1;
	}

	int32_t pc_get_level(lua_State * L)  
	{
		CHARACTER* ch = CQuestManager::Instance().GetCurrentCharacterPtr();
		if (!ch)
		{
			sys_err("Null character pointer triggered at %s:%d", __FILE__, __LINE__);
			return 0;
		}

		lua_pushnumber(L, ch->GetLevel());
		return 1;
	}

	int32_t pc_set_level(lua_State* L)
	{
		PC* pPC = CQuestManager::Instance().GetCurrentPC();
		if (!pPC)
		{
			sys_err("Null pc pointer triggered at %s:%d", __FILE__, __LINE__);
			return 0;
		}

		CHARACTER* ch = CQuestManager::Instance().GetCurrentCharacterPtr();
		if (!ch)
		{
			sys_err("Null character pointer triggered at %s:%d", __FILE__, __LINE__);
			return 0;
		}
		
		if (!lua_isnumber(L, 1))
		{
			sys_err("invalid argument");
			return 0;
		}

		int32_t newLevel = (int32_t)lua_tonumber(L, 1);

		sys_log(0, "QUEST [LEVEL] %s jumpint to level %d", ch->GetName(), (int32_t)rint(lua_tonumber(L, 1)));

		LogManager::Instance().QuestRewardLog(pPC->GetCurrentQuestName().c_str(), ch->GetPlayerID(), ch->GetLevel(), newLevel, 0);

		ch->PointChange(POINT_SKILL, newLevel - ch->GetLevel());
		ch->PointChange(POINT_SUB_SKILL, newLevel < 10 ? 0 : newLevel - MAX(ch->GetLevel(), 9));
		ch->PointChange(POINT_STAT, ((MINMAX(1, newLevel, gPlayerMaxLevel) - ch->GetLevel()) * 3) + ch->GetPoint(POINT_LEVEL_STEP));

		ch->PointChange(POINT_LEVEL, newLevel - ch->GetLevel());

		ch->SetRandomHP((newLevel - 1) * number(JobInitialPoints[ch->GetJob()].hp_per_lv_begin, JobInitialPoints[ch->GetJob()].hp_per_lv_end));

		ch->SetRandomSP((newLevel - 1) * number(JobInitialPoints[ch->GetJob()].sp_per_lv_begin, JobInitialPoints[ch->GetJob()].sp_per_lv_end));

		ch->PointChange(POINT_HP, ch->GetMaxHP() - ch->GetHP());
		ch->PointChange(POINT_SP, ch->GetMaxSP() - ch->GetSP());

		ch->ComputePoints();
		ch->PointsPacket();
		ch->SkillLevelPacket();

		return 0;
	}

	int32_t pc_get_weapon(lua_State* L)
	{
		CHARACTER* ch = CQuestManager::Instance().GetCurrentCharacterPtr();
		if (!ch)
		{
			sys_err("Null character pointer triggered at %s:%d", __FILE__, __LINE__);
			return 0;
		}

		CItem* item = ch->GetWear(WEAR_WEAPON);
		if (!item)
			lua_pushnumber(L, 0);
		else
			lua_pushnumber(L, item->GetVnum());

		return 1;
	}

	int32_t pc_get_armor(lua_State * L)
	{
		CHARACTER* ch = CQuestManager::Instance().GetCurrentCharacterPtr();
		if (!ch)
		{
			sys_err("Null character pointer triggered at %s:%d", __FILE__, __LINE__);
			return 0;
		}

		CItem* item = ch->GetWear(WEAR_BODY);
		if (!item)
			lua_pushnumber(L, 0);
		else
			lua_pushnumber(L, item->GetVnum());

		return 1;
	}

	int32_t pc_get_wear(lua_State * L)
	{		
		CHARACTER* ch = CQuestManager::Instance().GetCurrentCharacterPtr();
		if (!ch)
		{
			sys_err("Null character pointer triggered at %s:%d", __FILE__, __LINE__);
			return 0;
		}

		if (!lua_isnumber(L, 1)) 
		{
			sys_err("QUEST wrong set flag");
			return 0;
		}
		uint16_t bCell = (uint16_t)lua_tonumber(L, 1);

		CItem* item = ch->GetWear(bCell);

		if (!item)
			lua_pushnil(L);
		else
			lua_pushnumber(L, item->GetVnum());

		return 1;
	}

	int32_t pc_get_money(lua_State * L)
	{ 
		CHARACTER* ch = CQuestManager::Instance().GetCurrentCharacterPtr();
		if (!ch)
		{
			sys_err("Null character pointer triggered at %s:%d", __FILE__, __LINE__);
			return 0;
		}

		lua_pushnumber(L, static_cast<int32_t>(ch->GetGold()));
		return 1;
	}

	// 은둔의 망토 사용중 혼석 수련시 선악치가 두배 소모되는 버그가 발생해
	// 실제 선악치를 이용해 계산을 하게 한다.
	int32_t pc_get_real_alignment(lua_State* L)
	{
		CHARACTER* ch = CQuestManager::Instance().GetCurrentCharacterPtr();
		if (!ch)
		{
			sys_err("Null character pointer triggered at %s:%d", __FILE__, __LINE__);
			return 0;
		}

		lua_pushnumber(L, ch->GetRealAlignment() / 10);
		return 1;
	}

	int32_t pc_get_alignment(lua_State* L)
	{
		CHARACTER* ch = CQuestManager::Instance().GetCurrentCharacterPtr();
		if (!ch)
		{
			sys_err("Null character pointer triggered at %s:%d", __FILE__, __LINE__);
			return 0;
		}

		lua_pushnumber(L, ch->GetAlignment() / 10);
		return 1;
	}

	int32_t pc_change_alignment(lua_State * L)
	{
		CHARACTER* ch = CQuestManager::Instance().GetCurrentCharacterPtr();
		if (!ch)
		{
			sys_err("Null character pointer triggered at %s:%d", __FILE__, __LINE__);
			return 0;
		}

		if (!lua_isnumber(L, 1))
		{
			sys_err("invalid argument");
			return 0;
		}
		
		int32_t alignment = (int32_t)(lua_tonumber(L, 1) * 10);

		ch->UpdateAlignment(alignment);
		return 0;
	}

	int32_t pc_change_money(lua_State * L)
	{
		CHARACTER* ch = CQuestManager::Instance().GetCurrentCharacterPtr();
		if (!ch)
		{
			sys_err("Null character pointer triggered at %s:%d", __FILE__, __LINE__);
			return 0;
		}

		if (!lua_isnumber(L, 1))
		{
			sys_err("invalid argument");
			return 0;
		}
		int32_t gold = static_cast<int32_t>(lua_tonumber(L, -1));

		if (gold + ch->GetGold() < 0)
			sys_err("QUEST wrong ChangeGold %d (now %d)", gold, ch->GetGold());
		else
		{
			LogManager::Instance().MoneyLog(MONEY_LOG_QUEST, ch->GetPlayerID(), gold);
			ch->PointChange(POINT_GOLD, gold, true);
		}
		return 0;
	}

	int32_t pc_set_another_quest_flag(lua_State* L)
	{
		PC* pPC = CQuestManager::Instance().GetCurrentPC();
		if (!pPC)
		{
			sys_err("Null pc pointer triggered at %s:%d", __FILE__, __LINE__);
			return 0;
		}

		if (!lua_isstring(L, 1) || !lua_isstring(L, 2) || !lua_isnumber(L, 3)) 
		{
			sys_err("QUEST wrong set flag");
			return 0;
		}
		const char* sz = lua_tostring(L, 1);
		const char* sz2 = lua_tostring(L, 2);

		pPC->SetFlag(std::string(sz) + "." + sz2, int32_t(rint(lua_tonumber(L, 3))));
		return 0;
	}

	int32_t pc_get_another_quest_flag(lua_State* L)
	{
		PC* pPC = CQuestManager::Instance().GetCurrentPC();
		if (!pPC)
		{
			sys_err("Null pc pointer triggered at %s:%d", __FILE__, __LINE__);
			return 0;
		}

		if (!lua_isstring(L, 1) || !lua_isstring(L, 2)) 
		{
			sys_err("QUEST wrong get flag");
			return 0;
		}
		
		const char* sz = lua_tostring(L, 1);
		const char* sz2 = lua_tostring(L, 2);

		lua_pushnumber(L, pPC->GetFlag(std::string(sz) + "." + sz2));
		return 1;
	}

	int32_t pc_get_flag(lua_State* L)
	{
		PC* pPC = CQuestManager::Instance().GetCurrentPC();
		if (!pPC)
		{
			sys_err("Null pc pointer triggered at %s:%d", __FILE__, __LINE__);
			return 0;
		}

		if (!lua_isstring(L, -1)) 
		{
			sys_err("QUEST wrong get flag");
			return 0;
		}	
		const char* sz = lua_tostring(L, -1);

		lua_pushnumber(L, pPC->GetFlag(sz));
		return 1;
	}

	int32_t pc_get_quest_flag(lua_State* L)
	{
		PC* pPC = CQuestManager::Instance().GetCurrentPC();
		if (!pPC)
		{
			sys_err("Null pc pointer triggered at %s:%d", __FILE__, __LINE__);
			return 0;
		}

		if (!lua_isstring(L, -1))
		{
			sys_err("QUEST wrong get flag");
			return 0;
		}	
		const char* sz = lua_tostring(L, -1);

		lua_pushnumber(L, pPC->GetFlag(pPC->GetCurrentQuestName() + "." + sz));
		if (g_bIsTestServer)
			sys_log(0, "GetQF ( %s . %s )", pPC->GetCurrentQuestName().c_str(), sz);

		return 1;
	}

	int32_t pc_set_flag(lua_State* L)
	{
		PC* pPC = CQuestManager::Instance().GetCurrentPC();
		if (!pPC)
		{
			sys_err("Null pc pointer triggered at %s:%d", __FILE__, __LINE__);
			return 0;
		}

		if (!lua_isstring(L, 1) || !lua_isnumber(L, 2)) 
		{
			sys_err("QUEST wrong set flag");
			return 0;
		}
		const char* sz = lua_tostring(L, 1);

		pPC->SetFlag(sz, int32_t(rint(lua_tonumber(L, 2))));
		return 0;
	}

	int32_t pc_set_quest_flag(lua_State* L)
	{
		PC* pPC = CQuestManager::Instance().GetCurrentPC();
		if (!pPC)
		{
			sys_err("Null pc pointer triggered at %s:%d", __FILE__, __LINE__);
			return 0;
		}

		if (!lua_isstring(L, 1) || !lua_isnumber(L, 2)) 
		{
			sys_err("QUEST wrong set flag");
			return 0;
		} 
		const char* sz = lua_tostring(L, 1);
		
		pPC->SetFlag(pPC->GetCurrentQuestName() + "." + sz, int32_t(rint(lua_tonumber(L, 2))));
		return 0;
	}

	int32_t pc_del_quest_flag(lua_State* L)
	{
		PC* pPC = CQuestManager::Instance().GetCurrentPC();
		if (!pPC)
		{
			sys_err("Null pc pointer triggered at %s:%d", __FILE__, __LINE__);
			return 0;
		}

		if (!lua_isstring(L, 1)) 
		{
			sys_err("argument error");
			return 0;
		}
		const char* sz = lua_tostring(L, 1);

		pPC->DeleteFlag(pPC->GetCurrentQuestName() + "." + sz);
		return 0;
	}

	int32_t pc_give_exp2(lua_State* L)
	{
		CHARACTER* ch = CQuestManager::Instance().GetCurrentCharacterPtr();
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

		if (!lua_isnumber(L, 1))
			return 0;

		uint32_t exp = (uint32_t)rint(lua_tonumber(L, 1));
		sys_log(0, "QUEST [REWARD] %s give exp2 %u", ch->GetName(), exp);

		LogManager::Instance().QuestRewardLog(pPC->GetCurrentQuestName().c_str(), ch->GetPlayerID(), ch->GetLevel(), exp, 0);

		ch->PointChange(POINT_EXP, exp);

		return 0;
	}

	int32_t pc_give_exp(lua_State* L)
	{
		CHARACTER* ch = CQuestManager::Instance().GetCurrentCharacterPtr();
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

		if (!lua_isstring(L, 1) || !lua_isnumber(L, 2))
			return 0;

		uint32_t exp = (uint32_t)rint(lua_tonumber(L, 2));
		sys_log(0, "QUEST [REWARD] %s give exp %s %u", ch->GetName(), lua_tostring(L, 1), exp);

		LogManager::Instance().QuestRewardLog(pPC->GetCurrentQuestName().c_str(), ch->GetPlayerID(), ch->GetLevel(), exp, 0);

		pPC->GiveExp(lua_tostring(L,1), exp);
		return 0;
	}

	int32_t pc_give_exp_perc(lua_State* L)
	{
		CHARACTER* ch = CQuestManager::Instance().GetCurrentCharacterPtr();
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

		if (!lua_isstring(L, 1) || !lua_isnumber(L, 2) || !lua_isnumber(L, 3))
			return 0;

		int32_t lev = (int32_t)rint(lua_tonumber(L, 2));
		double proc = (lua_tonumber(L, 3));

		sys_log(0, "QUEST [REWARD] %s give exp %s lev %d percent %g%%", ch->GetName(), lua_tostring(L, 1), lev, proc);

		uint32_t exp = (uint32_t)((exp_table[MINMAX(0, lev, PLAYER_EXP_TABLE_MAX)] * proc) / 100);

		LogManager::Instance().QuestRewardLog(pPC->GetCurrentQuestName().c_str(), ch->GetPlayerID(), ch->GetLevel(), exp, 0);

		pPC->GiveExp(lua_tostring(L, 1), exp);
		return 0;
	}

	int32_t pc_get_exp_perc(lua_State* L)
	{
		if (!lua_isnumber(L, 1) || !lua_isnumber(L, 2))
			return 0;

		int32_t lev = (int32_t)rint(lua_tonumber(L, 1));
		double proc = (lua_tonumber(L, 2));

		lua_pushnumber(L, (uint32_t)((exp_table[MINMAX(0, lev, PLAYER_EXP_TABLE_MAX)] * proc) / 100));
		return 1;
	}

	int32_t pc_get_empire(lua_State* L)
	{
		CHARACTER* ch = CQuestManager::Instance().GetCurrentCharacterPtr();
		if (!ch) 
		{
			lua_pushnil(L);
			return 1;
		}

		lua_pushnumber(L, ch->GetEmpire());
		return 1;
	}

	int32_t pc_get_part(lua_State* L)
	{
		CHARACTER* ch = CQuestManager::Instance().GetCurrentCharacterPtr();
		if (!ch)
		{
			sys_err("Null character pointer triggered at %s:%d", __FILE__, __LINE__);
			return 0;
		}

		if (!lua_isnumber(L, 1)) 
		{
			lua_pushnumber(L, 0);
			return 1;
		}
		int32_t part_idx = (int32_t)lua_tonumber(L, 1);
		lua_pushnumber(L, ch->GetPart(part_idx));
		return 1;
	}

	int32_t pc_set_part(lua_State* L)
	{
		CHARACTER* ch = CQuestManager::Instance().GetCurrentCharacterPtr();
		if (!ch)
		{
			sys_err("Null character pointer triggered at %s:%d", __FILE__, __LINE__);
			return 0;
		}

		if (!lua_isnumber(L,1) || !lua_isnumber(L,2))
		{
			return 0;
		}
		int32_t part_idx = (int32_t)lua_tonumber(L, 1);
		int32_t part_value = (int32_t)lua_tonumber(L, 2);
		ch->SetPart(part_idx, part_value);
		ch->UpdatePacket();
		return 0;
	}

	int32_t pc_get_skillgroup(lua_State* L)  
	{
		CHARACTER* ch = CQuestManager::Instance().GetCurrentCharacterPtr();
		if (!ch)
		{
			sys_err("Null character pointer triggered at %s:%d", __FILE__, __LINE__);
			return 0;
		}

		lua_pushnumber(L, ch->GetSkillGroup());
		return 1;
	}

	int32_t pc_set_skillgroup(lua_State* L)
	{
		CHARACTER* ch = CQuestManager::Instance().GetCurrentCharacterPtr();
		if (!ch)
		{
			sys_err("Null character pointer triggered at %s:%d", __FILE__, __LINE__);
			return 0;
		}

		if (!lua_isnumber(L, 1))
		{
			sys_err("QUEST wrong skillgroup number");
			return 0;
		}
		
		ch->RemoveGoodAffect();
		ch->SetSkillGroup((uint8_t)rint(lua_tonumber(L, 1)));

		return 0;
	}

	int32_t pc_is_polymorphed(lua_State* L)
	{
		CHARACTER* ch = CQuestManager::Instance().GetCurrentCharacterPtr();
		if (!ch)
		{
			sys_err("Null character pointer triggered at %s:%d", __FILE__, __LINE__);
			return 0;
		}
		lua_pushboolean(L, ch->IsPolymorphed());
		return 1;
	}

	int32_t pc_remove_polymorph(lua_State* L)
	{
		CHARACTER* ch = CQuestManager::Instance().GetCurrentCharacterPtr();
		if (!ch)
		{
			sys_err("Null character pointer triggered at %s:%d", __FILE__, __LINE__);
			return 0;
		}

		ch->RemoveAffect(AFFECT_POLYMORPH);
		ch->SetPolymorph(0);
		return 0;
	}

	int32_t pc_polymorph(lua_State* L)
	{
		CHARACTER* ch = CQuestManager::Instance().GetCurrentCharacterPtr();
		if (!ch)
		{
			sys_err("Null character pointer triggered at %s:%d", __FILE__, __LINE__);
			return 0;
		}

		//if (CPolymorphUtils::Instance().IsOnPolymorphMapBlacklist(ch->GetMapIndex())) 
		//{
		//	ch->ChatPacket(CHAT_TYPE_INFO, ("둔갑에 실패 하였습니다"));
		//	return 0;
		//}

		if (!lua_isnumber(L, 1) || !lua_isnumber(L, 2))
		{
			sys_err("wrong arg");
			return 0;
		}
		uint32_t dwVnum = (uint32_t) lua_tonumber(L, 1);
		int32_t iDuration = (int32_t) lua_tonumber(L, 2);
		ch->AddAffect(AFFECT_POLYMORPH, POINT_POLYMORPH, dwVnum, AFF_POLYMORPH, iDuration, 0, true);
		return 0;
	}

	int32_t pc_is_mount(lua_State* L)
	{
		CHARACTER* ch = CQuestManager::Instance().GetCurrentCharacterPtr();
		if (!ch)
		{
			sys_err("Null character pointer triggered at %s:%d", __FILE__, __LINE__);

			lua_pushboolean(L, false);
			return 0;
		}

		lua_pushboolean(L, ch->GetMountVnum() != 0);
		return 1;
	}

	int32_t pc_mount(lua_State* L)
	{
		CHARACTER* ch = CQuestManager::Instance().GetCurrentCharacterPtr();
		if (!ch)
		{
			sys_err("Null character pointer triggered at %s:%d", __FILE__, __LINE__);
			return 0;
		}

		if (!lua_isnumber(L, 1))
			return 0;
		uint32_t mount_vnum = (uint32_t)lua_tonumber(L, 1);

		int32_t length = 60;
		if (lua_isnumber(L, 2))
			length = (int32_t)lua_tonumber(L, 2);

		if (length < 0)
			length = 60;

		if ((get_unix_ms_time() - ch->GetLastMoveTime()) < 1000) 
		{
			ch->ChatPacket(CHAT_TYPE_INFO, "Hareket ederken bu eylemi gerceklestiremezsin");
			return 0;
		}

		if (!IS_MOUNTABLE_ZONE(ch->GetMapIndex(), mount_vnum == ch->GetMyHorseVnum())) 
		{
			ch->ChatPacket(CHAT_TYPE_INFO, ("RIDING_IS_BLOCKED_HERE"));
			return 0;
		}

		ch->RemoveAffect(AFFECT_MOUNT);
		ch->RemoveAffect(AFFECT_MOUNT_BONUS);

		if (ch->GetWarMap())
		{
			if (ch->IsRiding())
				ch->StopRiding();
			return 0;
		}

		if (ch->GetHorse())
			ch->HorseSummon(false);

		if (mount_vnum)
		{
			ch->AddAffect(AFFECT_MOUNT, POINT_MOUNT, mount_vnum, AFF_NONE, length, 0, true);
			switch (mount_vnum)
			{
			case 20201:
			case 20202:
			case 20203:
			case 20204:
			case 20213:
			case 20216:
				{
					if (ch->GetPoint(POINT_MOV_SPEED) < ch->GetLimitPoint(POINT_MOV_SPEED))
						ch->AddAffect(AFFECT_MOUNT, POINT_MOV_SPEED, 30, AFF_NONE, length, 0, true, true);
					else
						sys_err("%s mount bonus overflow", ch->GetName());
				}
				break;

				case 20205:
				case 20206:
				case 20207:
				case 20208:
				case 20214:
				case 20217:
				{
					if (ch->GetPoint(POINT_MOV_SPEED) < ch->GetLimitPoint(POINT_MOV_SPEED))
						ch->AddAffect(AFFECT_MOUNT, POINT_MOV_SPEED, 40, AFF_NONE, length, 0, true, true);
					else
						sys_err("%s mount bonus overflow", ch->GetName());
				}
				break;

				case 20209:
				case 20210:
				case 20211:
				case 20212:
				case 20215:
				case 20218:
				{
					if (ch->GetPoint(POINT_MOV_SPEED) < ch->GetLimitPoint(POINT_MOV_SPEED))
						ch->AddAffect(AFFECT_MOUNT, POINT_MOV_SPEED, 50, AFF_NONE, length, 0, true, true);
					else
						sys_err("%s mount bonus overflow", ch->GetName());
				}
				break;

			default: 
				;
			}
		}
		
		return 0;
	}

	int32_t pc_mount_bonus(lua_State* L)
	{
		CHARACTER* ch = CQuestManager::Instance().GetCurrentCharacterPtr();
		if (!ch)
		{
			sys_err("Null character pointer triggered at %s:%d", __FILE__, __LINE__);
			return 0;
		}

		if (!lua_isnumber(L, 1) || !lua_isnumber(L, 2) || !lua_isnumber(L, 3))
		{
			sys_err("wrong arg");
			return 0;
		}
		uint8_t applyOn = static_cast<uint8_t>(lua_tonumber(L, 1));
		int32_t value = static_cast<int32_t>(lua_tonumber(L, 2));
		int32_t duration = static_cast<int32_t>(lua_tonumber(L, 3));

		if (!ch->GetMountVnum())
			return 0;

		ch->RemoveAffect(AFFECT_MOUNT_BONUS);

		if (ch->GetPoint(aApplyInfo[applyOn].bPointType) < ch->GetLimitPoint(aApplyInfo[applyOn].bPointType))
			ch->AddAffect(AFFECT_MOUNT_BONUS, aApplyInfo[applyOn].bPointType, value, AFF_NONE, duration, 0, false);
		else
			sys_err("%s mount bonus overflow", ch->GetName());

		return 0;
	}

	int32_t pc_unmount(lua_State* L)
	{
		CHARACTER* ch = CQuestManager::Instance().GetCurrentCharacterPtr();
		if (!ch)
		{
			sys_err("Null character pointer triggered at %s:%d", __FILE__, __LINE__);
			return 0;
		}
		ch->RemoveAffect(AFFECT_MOUNT);
		ch->RemoveAffect(AFFECT_MOUNT_BONUS);
		if (ch->IsHorseRiding())
			ch->StopRiding();
		return 0;
	}

	int32_t pc_get_horse_level(lua_State* L)
	{
		CHARACTER* ch = CQuestManager::Instance().GetCurrentCharacterPtr();
		if (!ch)
		{
			sys_err("Null character pointer triggered at %s:%d", __FILE__, __LINE__);
			return 0;
		}
		lua_pushnumber(L, ch->GetHorseLevel());
		return 1;
	}

	int32_t pc_get_horse_hp(lua_State* L)
	{
		CHARACTER* ch = CQuestManager::Instance().GetCurrentCharacterPtr();
		if (!ch)
		{
			sys_err("Null character pointer triggered at %s:%d", __FILE__, __LINE__);
			return 0;
		}
		if (ch->GetHorseLevel())
			lua_pushnumber(L, ch->GetHorseHealth());
		else
			lua_pushnumber(L, 0);

		return 1;
	}

	int32_t pc_get_horse_stamina(lua_State* L)
	{
		CHARACTER* ch = CQuestManager::Instance().GetCurrentCharacterPtr();
		if (!ch)
		{
			sys_err("Null character pointer triggered at %s:%d", __FILE__, __LINE__);
			return 0;
		}
		if (ch->GetHorseLevel())
			lua_pushnumber(L, ch->GetHorseStamina());
		else
			lua_pushnumber(L, 0);

		return 1;
	}

	int32_t pc_is_horse_alive(lua_State* L)
	{
		CHARACTER* ch = CQuestManager::Instance().GetCurrentCharacterPtr();
		if (!ch)
		{
			sys_err("Null character pointer triggered at %s:%d", __FILE__, __LINE__);
			return 0;
		}
		lua_pushboolean(L, ch->GetHorseLevel() > 0 && ch->GetHorseHealth()>0);
		return 1;
	}

	int32_t pc_revive_horse(lua_State* L)
	{
		CHARACTER* ch = CQuestManager::Instance().GetCurrentCharacterPtr();
		if (!ch)
		{
			sys_err("Null character pointer triggered at %s:%d", __FILE__, __LINE__);
			return 0;
		}
		ch->ReviveHorse();
		return 0;
	}

	int32_t pc_have_map_scroll(lua_State* L)
	{
		CHARACTER* ch = CQuestManager::Instance().GetCurrentCharacterPtr();
		if (!ch)
		{
			sys_err("Null character pointer triggered at %s:%d", __FILE__, __LINE__);

			lua_pushboolean(L, false);
			return 1;
		}

		if (!lua_isstring(L, 1)) 
		{
			lua_pushboolean(L, false);
			return 1;
		}
		const char* szMapName = lua_tostring(L, 1);

		auto region = SECTREE_MANAGER::Instance().FindRegionByPartialName(szMapName);
		if (!region) 
		{
			lua_pushboolean(L, false);
			return 1;
		}

		bool bFind = false;
		for (int32_t iCell = 0; iCell < INVENTORY_MAX_NUM; iCell++) 
		{
			CItem* item = ch->GetInventoryItem(iCell);
			if (!item)
				continue;

			if (item->GetType() == ITEM_USE && item->GetSubType() == USE_TALISMAN &&
					(item->GetValue(0) == 1 || item->GetValue(0) == 2))
			{
				int32_t x = item->GetSocket(0);
				int32_t y = item->GetSocket(1);
				//if ((x-item_x)*(x-item_x)+(y-item_y)*(y-item_y)<r*r)
				if (region->sx <= x && region->sy <= y && 
					x <= region->ex && y <= region->ey) 
				{
					bFind = true;
					break;
				}
			}
		}

		lua_pushboolean(L, bFind);
		return 1;
	}

	int32_t pc_get_war_map(lua_State* L)
	{
		CHARACTER* ch = CQuestManager::Instance().GetCurrentCharacterPtr();
		if (!ch)
		{
			sys_err("Null character pointer triggered at %s:%d", __FILE__, __LINE__);
			return 0;
		}
		lua_pushnumber(L, ch->GetWarMap() ? ch->GetWarMap()->GetMapIndex() : 0);
		return 1;
	}

	int32_t pc_have_pos_scroll(lua_State* L)
	{
		CHARACTER* ch = CQuestManager::Instance().GetCurrentCharacterPtr();
		if (!ch)
		{
			sys_err("Null character pointer triggered at %s:%d", __FILE__, __LINE__);
			lua_pushboolean(L, false);
			return 1;
		}

		if (!lua_isnumber(L, 1) || !lua_isnumber(L, 2)) 
		{
			sys_err("invalid x y position");

			lua_pushboolean(L, false);
			return 1;
		}
		int32_t x = (int32_t)lua_tonumber(L, 1);
		int32_t y = (int32_t)lua_tonumber(L, 2);

		if (!lua_isnumber(L, 3)) 
		{
			sys_err("invalid radius");

			lua_pushboolean(L, false);
			return 1;
		}
		float r = (float)lua_tonumber(L, 3);

		bool bFind = false;
		for (int32_t iCell = 0; iCell < INVENTORY_MAX_NUM; iCell++) 
		{
			CItem* item = ch->GetInventoryItem(iCell);
			if (!item)
				continue;

			if (item->GetType() == ITEM_USE && item->GetSubType() == USE_TALISMAN &&
					(item->GetValue(0) == 1 || item->GetValue(0) == 2))
			{
				int32_t item_x = item->GetSocket(0);
				int32_t item_y = item->GetSocket(1);
				if ((x-item_x)*(x-item_x)+(y-item_y)*(y-item_y)<r*r)
				{
					bFind = true;
					break;
				}
			}
		}

		lua_pushboolean(L, bFind);
		return 1;
	}

	int32_t pc_get_equip_refine_level(lua_State* L)
	{
		CHARACTER* ch = CQuestManager::Instance().GetCurrentCharacterPtr();
		if (!ch)
		{
			sys_err("Null character pointer triggered at %s:%d", __FILE__, __LINE__);
			return 0;
		}

		if (!lua_isnumber(L, 1))
		{
			sys_err("wrong arg");

			lua_pushnumber(L, 0);
			return 1;
		}
		int32_t cell = (int32_t)lua_tonumber(L, 1);

		if (cell < 0 || cell >= WEAR_MAX_NUM) 
		{
			sys_err("invalid wear position %d", cell);

			lua_pushnumber(L, 0);
			return 1;
		}

		CItem* item = ch->GetWear(cell);
		if (!item)
		{
			lua_pushnumber(L, 0);
			return 1;
		}

		lua_pushnumber(L, item->GetRefineLevel());
		return 1;
	}

	int32_t pc_refine_equip(lua_State* L)
	{
		CHARACTER* ch = CQuestManager::Instance().GetCurrentCharacterPtr();
		if (!ch)
		{
			sys_err("Null character pointer triggered at %s:%d", __FILE__, __LINE__);

			lua_pushboolean(L, false);
			return 1;
		}

		if (!lua_isnumber(L, 1) || !lua_isnumber(L, 2)) 
		{
			sys_err("invalid argument");

			lua_pushboolean(L, false);
			return 1;
		}
		int32_t cell = (int32_t)lua_tonumber(L, 1);
		int32_t level_limit = (int32_t)lua_tonumber(L, 2);

		int32_t pct = lua_isnumber(L, 3) ? (int32_t)lua_tonumber(L, 3) : 100;

		CItem* item = ch->GetWear(cell);
		if (!item) 
		{
			lua_pushboolean(L, false);
			return 1;
		}

		if (item->GetRefinedVnum() == 0)
		{
			lua_pushboolean(L, false);
			return 1;
		}

		if (item->GetRefineLevel() > level_limit)
		{
			lua_pushboolean(L, false);
			return 1;
		}

		if (pct == 100 || number(1, 100) <= pct) 
		{
			lua_pushboolean(L, true);

			CItem* pkNewItem = ITEM_MANAGER::Instance().CreateItem(item->GetRefinedVnum(), 1, 0, false);
			if (pkNewItem) 
			{
				for (int32_t i = 0; i < ITEM_SOCKET_MAX_NUM; ++i)
				{
					if (!item->GetSocket(i))
						break;

					pkNewItem->SetSocket(i, 1);
				}

				int32_t set = 0;
				for (int32_t i = 0; i < ITEM_SOCKET_MAX_NUM; i++) 
				{
					int32_t socket = item->GetSocket(i);
					if (socket > 2 && socket != 28960) 
					{
						pkNewItem->SetSocket(set++, socket);
					}
				}

				item->CopyAttributeTo(pkNewItem);

				ITEM_MANAGER::Instance().RemoveItem(item, "REMOVE (REFINE SUCCESS)");

				pkNewItem->EquipTo(ch, cell);

				ITEM_MANAGER::Instance().FlushDelayedSave(pkNewItem);

				LogManager::Instance().ItemLog(ch, pkNewItem, "REFINE SUCCESS (QUEST)", pkNewItem->GetName());
			}
			return 1;
		} 

		lua_pushboolean(L, false);
		return 1;
	}

	int32_t pc_get_skill_level(lua_State * L)
	{
		CHARACTER* ch = CQuestManager::Instance().GetCurrentCharacterPtr();
		if (!ch)
		{
			sys_err("Null character pointer triggered at %s:%d", __FILE__, __LINE__);
	
			lua_pushnumber(L, 0);
			return 1;
		}

		if (!lua_isnumber(L, 1))
		{
			sys_err("invalid argument");

			lua_pushnumber(L, 0);
			return 1;
		}
		uint32_t dwVnum = (uint32_t)lua_tonumber(L, 1);

		lua_pushnumber(L, ch->GetSkillLevel(dwVnum));
		return 1;
	}

	int32_t pc_aggregate_monster(lua_State* L)
	{
		CHARACTER* ch = CQuestManager::Instance().GetCurrentCharacterPtr();
		if (!ch)
		{
			sys_err("Null character pointer triggered at %s:%d", __FILE__, __LINE__);
			return 0;
		}
		ch->AggregateMonster();
		return 0;
	}

	int32_t pc_forget_my_attacker(lua_State* L)
	{
		CHARACTER* ch = CQuestManager::Instance().GetCurrentCharacterPtr();
		if (!ch)
		{
			sys_err("Null character pointer triggered at %s:%d", __FILE__, __LINE__);
			return 0;
		}
		ch->ForgetMyAttacker();
		return 0;
	}

	int32_t pc_attract_ranger(lua_State* L)
	{
		CHARACTER* ch = CQuestManager::Instance().GetCurrentCharacterPtr();
		if (!ch)
		{
			sys_err("Null character pointer triggered at %s:%d", __FILE__, __LINE__);
			return 0;
		}
		ch->AttractRanger();
		return 0;
	}

	int32_t pc_select_pid(lua_State* L)
	{
		if (!lua_isnumber(L, 1))
		{
			sys_err("invalid argument");

			lua_pushnumber(L, 0);
			return 1;
		}
		uint32_t pid = (uint32_t)lua_tonumber(L, 1);

		CHARACTER* ch = CQuestManager::Instance().GetCurrentCharacterPtr();
		if (!ch)
		{
			sys_err("Null character pointer triggered at %s:%d", __FILE__, __LINE__);

			lua_pushnumber(L, 0);
			return 1;
		}

		CHARACTER* new_ch = CHARACTER_MANAGER::Instance().FindByPID(pid);
		if (new_ch) 
		{
			CQuestManager::Instance().GetPC(new_ch->GetPlayerID());

			lua_pushnumber(L, ch->GetPlayerID());
			return 1;
		}

		lua_pushnumber(L, 0);
		return 1;
	}

	int32_t pc_select_vid(lua_State* L)
	{
		if (!lua_isnumber(L, 1))
		{
			sys_err("invalid argument");

			lua_pushnumber(L, 0);
			return 1;
		}
		uint32_t vid = (uint32_t)lua_tonumber(L, 1);

		CHARACTER* ch = CQuestManager::Instance().GetCurrentCharacterPtr();
		if (!ch)
		{
			sys_err("Null character pointer triggered at %s:%d", __FILE__, __LINE__);

			lua_pushnumber(L, 0);
			return 1;
		}

		CHARACTER* new_ch = CHARACTER_MANAGER::Instance().Find(vid);
		if (new_ch) 
		{
			CQuestManager::Instance().GetPC(new_ch->GetPlayerID());

			lua_pushnumber(L, (uint32_t)ch->GetVID());
			return 1;
		}

		lua_pushnumber(L, 0);
		return 1;
	}

	int32_t pc_get_sex(lua_State* L)
	{
		CHARACTER* ch = CQuestManager::Instance().GetCurrentCharacterPtr();
		if (!ch)
		{
			sys_err("Null character pointer triggered at %s:%d", __FILE__, __LINE__);
			return 0;
		}
		lua_pushnumber(L, GET_SEX(ch)); /* 0==MALE, 1==FEMALE */
		return 1;
	}

	int32_t pc_is_engaged(lua_State* L)
	{
		CHARACTER* ch = CQuestManager::Instance().GetCurrentCharacterPtr();
		if (!ch)
		{
			sys_err("Null character pointer triggered at %s:%d", __FILE__, __LINE__);
			return 0;
		}
		lua_pushboolean(L, marriage::CManager::Instance().IsEngaged(ch->GetPlayerID()));
		return 1;
	}

	int32_t pc_is_married(lua_State* L)
	{
		CHARACTER* ch = CQuestManager::Instance().GetCurrentCharacterPtr();
		if (!ch)
		{
			sys_err("Null character pointer triggered at %s:%d", __FILE__, __LINE__);
			return 0;
		}
		lua_pushboolean(L, marriage::CManager::Instance().IsMarried(ch->GetPlayerID()));
		return 1;
	}

	int32_t pc_is_engaged_or_married(lua_State* L)
	{
		CHARACTER* ch = CQuestManager::Instance().GetCurrentCharacterPtr();
		if (!ch)
		{
			sys_err("Null character pointer triggered at %s:%d", __FILE__, __LINE__);
			return 0;
		}
		lua_pushboolean(L, marriage::CManager::Instance().IsEngagedOrMarried(ch->GetPlayerID()));
		return 1;
	}

	int32_t pc_is_gm(lua_State* L)
	{
		CHARACTER* ch = CQuestManager::Instance().GetCurrentCharacterPtr();
		if (!ch)
		{
			sys_err("Null character pointer triggered at %s:%d", __FILE__, __LINE__);
			return 0;
		}
		lua_pushboolean(L, ch->GetGMLevel() >= GM_HIGH_WIZARD);
		return 1;
	}

	int32_t pc_get_gm_level(lua_State* L)
	{
		CHARACTER* ch = CQuestManager::Instance().GetCurrentCharacterPtr();
		if (!ch)
		{
			sys_err("Null character pointer triggered at %s:%d", __FILE__, __LINE__);
			return 0;
		}
		lua_pushnumber(L, ch->GetGMLevel());
		return 1;
	}

	int32_t pc_mining(lua_State* L)
	{
		CHARACTER* ch = CQuestManager::Instance().GetCurrentCharacterPtr();
		if (!ch)
		{
			sys_err("Null character pointer triggered at %s:%d", __FILE__, __LINE__);
			return 0;
		}

		CHARACTER* npc = CQuestManager::Instance().GetCurrentNPCCharacterPtr();
		if (!npc)
		{
			sys_err("Null npc pointer triggered at %s:%d", __FILE__, __LINE__);
			return 0;
		}

		ch->mining(npc);
		return 0;
	}

	int32_t pc_diamond_refine(lua_State* L)
	{
		if (!lua_isnumber(L, 1) || !lua_isnumber(L, 2))
		{
			lua_pushboolean(L, false);
			return 1;
		}
		int32_t cost = (int32_t)lua_tonumber(L, 1);
		int32_t pct = (int32_t)lua_tonumber(L, 2);

		CHARACTER* ch = CQuestManager::Instance().GetCurrentCharacterPtr();
		CHARACTER* npc = CQuestManager::Instance().GetCurrentNPCCharacterPtr();
		CItem* item = CQuestManager::Instance().GetCurrentItem();

		if (ch && npc && item)
		{
			lua_pushboolean(L, mining::OreRefine(ch, npc, item, cost, pct, nullptr));
			return 1;
		}

		lua_pushboolean(L, false);
		return 1;
	}

	int32_t pc_ore_refine(lua_State* L)
	{
		if (!lua_isnumber(L, 1) || !lua_isnumber(L, 2) || !lua_isnumber(L, 3)) 
		{
			lua_pushboolean(L, false);
			return 1;
		}
		int32_t cost = (int32_t)lua_tonumber(L, 1);
		int32_t pct = (int32_t)lua_tonumber(L, 2);
		int32_t metinstone_cell = (int32_t)lua_tonumber(L, 3);

		CHARACTER* ch = CQuestManager::Instance().GetCurrentCharacterPtr();
		CHARACTER* npc = CQuestManager::Instance().GetCurrentNPCCharacterPtr();
		CItem* item = CQuestManager::Instance().GetCurrentItem();

		if (ch && npc && item)
		{
			CItem* metinstone_item = ch->GetInventoryItem(metinstone_cell);
			if (metinstone_item)
				lua_pushboolean(L, mining::OreRefine(ch, npc, item, cost, pct, metinstone_item));
		}

		lua_pushboolean(L, false);
		return 1;
	}

	int32_t pc_clear_skill(lua_State* L)
	{
		CHARACTER* ch = CQuestManager::Instance().GetCurrentCharacterPtr();
		if (!ch)
		{
			sys_err("Null character pointer triggered at %s:%d", __FILE__, __LINE__);
			return 0;
		}

		ch->ClearSkill();
		return 0;
	}

	int32_t pc_clear_sub_skill(lua_State* L)
	{
		CHARACTER* ch = CQuestManager::Instance().GetCurrentCharacterPtr();
		if (!ch)
		{
			sys_err("Null character pointer triggered at %s:%d", __FILE__, __LINE__);
			return 0;
		}

		ch->ClearSubSkill();
		return 0;
	}

	int32_t pc_set_skill_point(lua_State* L)
	{
		CHARACTER* ch = CQuestManager::Instance().GetCurrentCharacterPtr();
		if (!ch)
		{
			sys_err("Null character pointer triggered at %s:%d", __FILE__, __LINE__);
			return 0;
		}

		if (!lua_isnumber(L, 1))
		{
			return 0;
		}
		int32_t newPoint = (int32_t)lua_tonumber(L, 1);

		ch->SetRealPoint(POINT_SKILL, newPoint);
		ch->SetPoint(POINT_SKILL, ch->GetRealPoint(POINT_SKILL));
		ch->PointChange(POINT_SKILL, 0);
		ch->ComputePoints();
		ch->PointsPacket();

		return 0;
	}

	// RESET_ONE_SKILL	
	int32_t pc_clear_one_skill(lua_State* L)
	{
		CHARACTER* ch = CQuestManager::Instance().GetCurrentCharacterPtr();
		if (!ch)
		{
			sys_err("Null character pointer triggered at %s:%d", __FILE__, __LINE__);

			lua_pushnumber(L, 0);
			return 1;
		}

		if (!lua_isnumber(L, 1))
		{
			return 0;
		}
		int32_t vnum = (int32_t)lua_tonumber(L, 1);

		sys_log(0, "%d skill clear", vnum);

		ch->ResetOneSkill(vnum);
		return 0;
	}
	// END_RESET_ONE_SKILL

	int32_t pc_is_clear_skill_group(lua_State* L)
	{
		CHARACTER* ch = CQuestManager::Instance().GetCurrentCharacterPtr();
		if (!ch)
		{
			sys_err("Null character pointer triggered at %s:%d", __FILE__, __LINE__);
			return 0;
		}

		lua_pushboolean(L, ch->GetQuestFlag("skill_group_clear.clear") == 1);
		return 1;
	}

	int32_t pc_save_exit_location(lua_State* L)
	{
		CHARACTER* ch = CQuestManager::Instance().GetCurrentCharacterPtr();
		if (!ch)
		{
			sys_err("Null character pointer triggered at %s:%d", __FILE__, __LINE__);
			return 0;
		}

		ch->SaveExitLocation();
		return 0;
	}

	//텔레포트 
	int32_t pc_teleport ( lua_State * L )
	{
		CHARACTER* ch = CQuestManager::Instance().GetCurrentCharacterPtr();
		if (!ch)
		{
			sys_err("Null character pointer triggered at %s:%d", __FILE__, __LINE__);
			return 0;
		}

		int32_t x=0,y=0;	
		if ( lua_isnumber(L, 1) )
		{
			// 지역명 워프
			const int32_t TOWN_NUM = 10;
			struct warp_by_town_name
			{
				const char* name;
				uint32_t x;
				uint32_t y;
			} ws[TOWN_NUM] = 
			{
				{"영안읍성",	4743,	9548},
				{"임지곡",		3235,	9086},
				{"자양현",		3531,	8829},
				{"조안읍성",	638,	1664},
				{"승룡곡",		1745,	1909},
				{"복정현",		1455,	2400},
				{"평무읍성",	9599,	2692},
				{"방산곡",		8036,	2984},
				{"박라현",		8639,	2460},
				{"서한산",		4350,	2143},
			};
			int32_t idx  = (int32_t)lua_tonumber(L, 1);

			x = ws[idx].x;
			y = ws[idx].y;
			goto teleport_area;
		}
		else
		{
			const char * arg1 = lua_tostring(L, 1);

			LPCHARACTER tch = CHARACTER_MANAGER::Instance().FindPC(arg1);

			if (!tch)
			{
				auto cci = P2P_MANAGER::Instance().Find(arg1);
				if (cci) 
				{
					if (cci->bChannel != g_bChannel) 
					{
						ch->ChatPacket(CHAT_TYPE_INFO, "Target is in %d channel (my channel %d)", cci->bChannel, g_bChannel);
					} 
					else 
					{
						GPOS pos;
						if (!SECTREE_MANAGER::Instance().GetCenterPositionOfMap(cci->lMapIndex, pos)) 
						{
							ch->ChatPacket(CHAT_TYPE_INFO, "Cannot find map (index %d)", cci->lMapIndex);
						} 
						else 
						{
							ch->ChatPacket(CHAT_TYPE_INFO, "You warp to ( %d, %d )", pos.x, pos.y);
							ch->WarpSet(pos.x, pos.y);
							lua_pushnumber(L, 1);
						}
					}
				} 
				else if (nullptr == CHARACTER_MANAGER::Instance().FindPC(arg1))
				{
					ch->ChatPacket(CHAT_TYPE_INFO, "There is no one by that name");
				}

				lua_pushnumber(L, 0);
				return 1;
			}
			else
			{
				x = tch->GetX() / 100;
				y = tch->GetY() / 100;
			}
		}

teleport_area:

		x *= 100;
		y *= 100;

		ch->ChatPacket(CHAT_TYPE_INFO, "You warp to ( %d, %d )", x, y);
		ch->WarpSet(x,y);
		ch->Stop();
		lua_pushnumber(L, 1 );
		return 1;
	}

	int32_t pc_set_skill_level(lua_State* L)
	{
		CHARACTER* ch = CQuestManager::Instance().GetCurrentCharacterPtr();
		if (!ch)
		{
			sys_err("Null character pointer triggered at %s:%d", __FILE__, __LINE__);
			return 0;
		}


		if (!lua_isnumber(L, 1) || !lua_isnumber(L, 2))
		{
			return 0;
		}
		uint32_t dwVnum = (uint32_t)lua_tonumber(L, 1);
		uint8_t byLev = (uint8_t)lua_tonumber(L, 2);

		ch->SetSkillLevel(dwVnum, byLev);
		ch->SkillLevelPacket();

		return 0;
	}

	int32_t pc_give_polymorph_book(lua_State* L)
	{
		CHARACTER* ch = CQuestManager::Instance().GetCurrentCharacterPtr();
		if (!ch)
		{
			sys_err("Null character pointer triggered at %s:%d", __FILE__, __LINE__);
			return 0;
		}

		if (!lua_isnumber(L, 1) && !lua_isnumber(L, 2) && !lua_isnumber(L, 3) && !lua_isnumber(L, 4))
		{
			sys_err("Wrong Quest Function Arguments: pc_give_polymorph_book");
			return 0;
		}

		CPolymorphUtils::Instance().GiveBook(ch, (uint32_t)lua_tonumber(L, 1), (uint32_t)lua_tonumber(L, 2), (uint8_t)lua_tonumber(L, 3), (uint8_t)lua_tonumber(L, 4));

		return 0;
	}

	int32_t pc_upgrade_polymorph_book(lua_State* L)
	{
		CHARACTER* ch = CQuestManager::Instance().GetCurrentCharacterPtr();
		if (!ch)
		{
			sys_err("Null character pointer triggered at %s:%d", __FILE__, __LINE__);
			return 0;
		}

		CItem* pItem = CQuestManager::Instance().GetCurrentItem();
		if (!pItem)
		{
			sys_err("Null item pointer triggered at %s:%d", __FILE__, __LINE__);
			return 0;
		}

		bool ret = CPolymorphUtils::Instance().BookUpgrade(ch, pItem);

		lua_pushboolean(L, ret);
		return 1;
	}

	int32_t pc_get_premium_remain_sec(lua_State* L)
	{
		CHARACTER* ch = CQuestManager::Instance().GetCurrentCharacterPtr();
		if (!ch)
		{
			sys_err("Null character pointer triggered at %s:%d", __FILE__, __LINE__);
			return 0;
		}

		if (!lua_isnumber(L, 1)) 
		{
			sys_err("wrong premium index (is not number)");
			return 0;
		}

		int32_t premium_type = (int32_t)lua_tonumber(L, 1);
		switch (premium_type)
		{
			case PREMIUM_EXP:
			case PREMIUM_ITEM:
			case PREMIUM_SAFEBOX:
			case PREMIUM_AUTOLOOT:
			case PREMIUM_FISH_MIND:
			case PREMIUM_MARRIAGE_FAST:
			case PREMIUM_GOLD:
				break;

			default:
				sys_err("wrong premium index %d", premium_type);
				return 0;
		}

		int32_t remain_seconds = ch->GetPremiumRemainSeconds(premium_type);

		lua_pushnumber(L, remain_seconds);
		return 1;
	}	

	int32_t pc_send_block_mode(lua_State* L)
	{
		CHARACTER* ch = CQuestManager::Instance().GetCurrentCharacterPtr();
		if (!ch)
		{
			sys_err("Null character pointer triggered at %s:%d", __FILE__, __LINE__);
			return 0;
		}

		ch->SetBlockModeForce((uint32_t)lua_tonumber(L, 1));
		return 0;
	}

	int32_t pc_change_empire(lua_State* L)
	{
		CHARACTER* ch = CQuestManager::Instance().GetCurrentCharacterPtr();
		if (!ch)
		{
			sys_err("Null character pointer triggered at %s:%d", __FILE__, __LINE__);
			return 0;
		}

		if (ch->GetParty())
		{
			ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("Gruptayken bayrak degisemezsin."));
			lua_pushnumber(L, 4);
			return 0;
		}


		lua_pushnumber(L, ch->ChangeEmpire((uint8_t)lua_tonumber(L, 1)));
		return 1;
	}

	int32_t pc_get_change_empire_count(lua_State* L)
	{
		CHARACTER* ch = CQuestManager::Instance().GetCurrentCharacterPtr();
		if (!ch)
		{
			sys_err("Null character pointer triggered at %s:%d", __FILE__, __LINE__);
			return 0;
		}

		lua_pushnumber(L, ch->GetChangeEmpireCount());
		return 1;
	}

	int32_t pc_set_change_empire_count(lua_State* L)
	{
		CHARACTER* ch = CQuestManager::Instance().GetCurrentCharacterPtr();
		if (!ch)
		{
			sys_err("Null character pointer triggered at %s:%d", __FILE__, __LINE__);
			return 0;
		}

		ch->SetChangeEmpireCount();
		return 0;
	}
	
	int32_t pc_change_name(lua_State* L)
	{
		CHARACTER* ch = CQuestManager::Instance().GetCurrentCharacterPtr();
		if (!ch)
		{
			sys_err("Null character pointer triggered at %s:%d", __FILE__, __LINE__);
			return 0;
		}

		if ( ch->GetNewName().size() != 0 )
		{
			lua_pushnumber(L, 0);
			return 1;
		}

		if (!lua_isstring(L, 1)) 
		{
			lua_pushnumber(L, 1);
			return 1;
		}

		const char * szName = lua_tostring(L, 1);

		if ( check_name(szName) == false )
		{
			lua_pushnumber(L, 2);
			return 1;
		}

		char szQuery[1024];
		snprintf(szQuery, sizeof(szQuery), "SELECT COUNT(*) FROM player WHERE name='%s'", szName);
		std::unique_ptr<SQLMsg> pmsg(DBManager::Instance().DirectQuery(szQuery));

		if ( pmsg->Get()->uiNumRows > 0 )
		{
			MYSQL_ROW row = mysql_fetch_row(pmsg->Get()->pSQLResult);

			int32_t	count = 0;
			str_to_number(count, row[0]);

			// 이미 해당 이름을 가진 캐릭터가 있음
			if ( count != 0 )
			{
				lua_pushnumber(L, 3);
				return 1;
			}
		}

		uint32_t pid = ch->GetPlayerID();
		db_clientdesc->DBPacketHeader(HEADER_GD_FLUSH_CACHE, 0, sizeof(uint32_t));
		db_clientdesc->Packet(&pid, sizeof(uint32_t));

		/* delete messenger list */
		MessengerManager::Instance().RemoveAllList(ch->GetName());

		/* change_name_log */
		LogManager::Instance().ChangeNameLog(pid, ch->GetName(), szName, ch->GetDesc()->GetHostName());

		snprintf(szQuery, sizeof(szQuery), "UPDATE player SET name='%s' WHERE id=%u", szName, pid);
		std::unique_ptr<SQLMsg> pmsg2(DBManager::Instance().DirectQuery(szQuery));

		ch->SetNewName(szName);

		lua_pushnumber(L, 4);
		return 1;
	}

	int32_t pc_is_dead(lua_State* L)
	{
		CHARACTER* ch = CQuestManager::Instance().GetCurrentCharacterPtr();
		if (ch) 
		{
			lua_pushboolean(L, ch->IsDead());
			return 1;
		}

		lua_pushboolean(L, true);
		return 1;
	}

	int32_t pc_reset_status( lua_State* L )
	{
		CHARACTER* ch = CQuestManager::Instance().GetCurrentCharacterPtr();
		if (!ch)
		{
			sys_err("Null character pointer triggered at %s:%d", __FILE__, __LINE__);
			return 0;
		}

		if (!lua_isnumber(L, 1))
		{
			lua_pushboolean(L, false);
			return 1;
		}
		int32_t idx = (int32_t)lua_tonumber(L, 1);

		if (idx >= 0 && idx < 4) 
		{
			int32_t point = 0;
			
			switch (idx) 
			{
				case 0:
					point = POINT_HT;
					break;
				case 1:
					point = POINT_IQ;
					break;
				case 2:
					point = POINT_ST;
					break;
				case 3:
					point = POINT_DX;
					break;
				default:
					lua_pushboolean(L, false);
					return 1;
			}

				int32_t old_val = ch->GetRealPoint(point);
				int32_t old_stat = ch->GetRealPoint(POINT_STAT);

				ch->SetRealPoint(point, 1);
				ch->SetPoint(point, ch->GetRealPoint(point));

				ch->PointChange(POINT_STAT, old_val-1);

				if ( point == POINT_HT )
				{
					uint8_t job = ch->GetJob();
					ch->SetRandomHP((ch->GetLevel()-1) * number(JobInitialPoints[job].hp_per_lv_begin, JobInitialPoints[job].hp_per_lv_end));
				}
				else if ( point == POINT_IQ )
				{
					uint8_t job = ch->GetJob();
					ch->SetRandomSP((ch->GetLevel()-1) * number(JobInitialPoints[job].sp_per_lv_begin, JobInitialPoints[job].sp_per_lv_end));
				}

				ch->ComputePoints();
				ch->PointsPacket();

				if ( point == POINT_HT )
				{
					ch->PointChange(POINT_HP, ch->GetMaxHP() - ch->GetHP());
				}
				else if ( point == POINT_IQ )
				{
					ch->PointChange(POINT_SP, ch->GetMaxSP() - ch->GetSP());
				}

			char buf[128] = { 0 };
			switch (idx)
			{
				case 0:
					snprintf(buf, sizeof(buf), "reset ht(%d)->1 stat_point(%d)->(%d)", old_val, old_stat, ch->GetRealPoint(POINT_STAT));
					break;
				case 1:
					snprintf(buf, sizeof(buf), "reset iq(%d)->1 stat_point(%d)->(%d)", old_val, old_stat, ch->GetRealPoint(POINT_STAT));
						break;
				case 2:
					snprintf(buf, sizeof(buf), "reset st(%d)->1 stat_point(%d)->(%d)", old_val, old_stat, ch->GetRealPoint(POINT_STAT));
					break;
				case 3:
					snprintf(buf, sizeof(buf), "reset dx(%d)->1 stat_point(%d)->(%d)", old_val, old_stat, ch->GetRealPoint(POINT_STAT));
					break;
				default:
					;
			}

			LogManager::Instance().CharLog(ch, 0, "RESET_ONE_STATUS", buf);

			lua_pushboolean(L, true);
			return 1;
		}

		lua_pushboolean(L, false);
		return 1;
	}

	int32_t pc_get_ht( lua_State* L )
	{
		CHARACTER* ch = CQuestManager::Instance().GetCurrentCharacterPtr();
		if (!ch)
		{
			sys_err("Null character pointer triggered at %s:%d", __FILE__, __LINE__);
			return 0;
		}

		lua_pushnumber(L, static_cast<int32_t>(ch->GetRealPoint(POINT_HT)));
		return 1;
	}

	int32_t pc_set_ht( lua_State* L )
	{
		CHARACTER* ch = CQuestManager::Instance().GetCurrentCharacterPtr();
		if (!ch)
		{
			sys_err("Null character pointer triggered at %s:%d", __FILE__, __LINE__);
			return 0;
		}
		
		if (lua_isnumber(L, 1) == false)
			return 1;
		int32_t newPoint = (int32_t)lua_tonumber(L, 1);

		int32_t usedPoint = newPoint - ch->GetRealPoint(POINT_HT);
		ch->SetRealPoint(POINT_HT, newPoint);

		ch->PointChange(POINT_HT, 0);
		ch->PointChange(POINT_STAT, -usedPoint);
		ch->ComputePoints();
		ch->PointsPacket();
		return 1;
	}

	int32_t pc_get_iq( lua_State* L )
	{
		CHARACTER* ch = CQuestManager::Instance().GetCurrentCharacterPtr();
		if (!ch)
		{
			sys_err("Null character pointer triggered at %s:%d", __FILE__, __LINE__);
			return 0;
		}

		lua_pushnumber(L, static_cast<int32_t>(ch->GetRealPoint(POINT_IQ)));
		return 1;
	}

	int32_t pc_set_iq( lua_State* L )
	{
		CHARACTER* ch = CQuestManager::Instance().GetCurrentCharacterPtr();
		if (!ch)
		{
			sys_err("Null character pointer triggered at %s:%d", __FILE__, __LINE__);
			return 0;
		}

		if (!lua_isnumber(L, 1))
			return 1;
		int32_t newPoint = (int32_t)lua_tonumber(L, 1);

		int32_t usedPoint = newPoint - ch->GetRealPoint(POINT_IQ);
		ch->SetRealPoint(POINT_IQ, newPoint);

		ch->PointChange(POINT_IQ, 0);
		ch->PointChange(POINT_STAT, -usedPoint);
		ch->ComputePoints();
		ch->PointsPacket();
		return 1;
	}
	
	int32_t pc_get_st( lua_State* L )
	{
		CHARACTER* ch = CQuestManager::Instance().GetCurrentCharacterPtr();
		if (!ch)
		{
			sys_err("Null character pointer triggered at %s:%d", __FILE__, __LINE__);
			return 0;
		}

		lua_pushnumber(L, static_cast<int32_t>(ch->GetRealPoint(POINT_ST)));
		return 1;
	}

	int32_t pc_set_st( lua_State* L )
	{
		CHARACTER* ch = CQuestManager::Instance().GetCurrentCharacterPtr();
		if (!ch)
		{
			sys_err("Null character pointer triggered at %s:%d", __FILE__, __LINE__);
			return 0;
		}
		
		if (!lua_isnumber(L, 1))
			return 1;
		int32_t newPoint = (int64_t)lua_tonumber(L, 1);

		int32_t usedPoint = newPoint - ch->GetRealPoint(POINT_ST);
		ch->SetRealPoint(POINT_ST, newPoint);
		ch->PointChange(POINT_ST, 0);
		ch->PointChange(POINT_STAT, -usedPoint);
		ch->ComputePoints();
		ch->PointsPacket();
		return 1;
	}
	
	int32_t pc_get_dx( lua_State* L )
	{
		CHARACTER* ch = CQuestManager::Instance().GetCurrentCharacterPtr();
		if (!ch)
		{
			sys_err("Null character pointer triggered at %s:%d", __FILE__, __LINE__);
			return 0;
		}

		lua_pushnumber(L, static_cast<int32_t>(ch->GetRealPoint(POINT_DX)));
		return 1;
	}

	int32_t pc_set_dx( lua_State* L )
	{
		CHARACTER* ch = CQuestManager::Instance().GetCurrentCharacterPtr();
		if (!ch)
		{
			sys_err("Null character pointer triggered at %s:%d", __FILE__, __LINE__);
			return 0;
		}

		if (!lua_isnumber(L, 1))
			return 0;
		int32_t newPoint = (int32_t)lua_tonumber(L, 1);

		auto usedPoint = newPoint - ch->GetRealPoint(POINT_DX);
		ch->SetRealPoint(POINT_DX, newPoint);

		ch->PointChange(POINT_DX, 0);
		ch->PointChange(POINT_STAT, -usedPoint);

		ch->ComputePoints();
		ch->PointsPacket();

		return 1;
	}

	int32_t pc_is_near_vid( lua_State* L )
	{
		CHARACTER* pMe = CQuestManager::Instance().GetCurrentCharacterPtr();
		if (!pMe)
		{
			sys_err("Null character pointer triggered at %s:%d", __FILE__, __LINE__);

			lua_pushboolean(L, false);
			return 1;
		}

		if (!lua_isnumber(L, 1) || !lua_isnumber(L, 2))
		{
			lua_pushboolean(L, false);
			return 1;
		}
		
		CHARACTER* pOther = CHARACTER_MANAGER::Instance().Find((uint32_t)lua_tonumber(L, 1));
		if (!pOther)
		{
			sys_err("Null other character pointer triggered at %s:%d", __FILE__, __LINE__);

			lua_pushboolean(L, false);
			return 1;
		}

		lua_pushboolean(L, (DISTANCE_APPROX(pMe->GetX() - pOther->GetX(), pMe->GetY() - pOther->GetY()) < (int32_t)lua_tonumber(L, 2) * 100));
		return 1;
	}

	int32_t pc_get_socket_items( lua_State* L )
	{
		CHARACTER* pChar = CQuestManager::Instance().GetCurrentCharacterPtr();
		if (!pChar)
		{
			sys_err("Null character pointer triggered at %s:%d", __FILE__, __LINE__);
			return 0;
		}

		lua_newtable(L);

		int32_t idx = 1;
		for (int32_t i = 0; i < INVENTORY_MAX_NUM + WEAR_MAX_NUM; i++) 
		{
			CItem* pItem = pChar->GetInventoryItem(i);
			if (pItem)
			{
				if (pItem->IsEquipped() == false) 
				{
					int32_t j = 0;
					for (j = 0; j < ITEM_SOCKET_MAX_NUM; j++)
					{
						int32_t socket = pItem->GetSocket(j);

						if (socket > 2 && socket != ITEM_BROKEN_METIN_VNUM) 
						{
							SItemTable_Server* pItemInfo = ITEM_MANAGER::Instance().GetTable(socket);
							if (pItemInfo) 
							{
								if (pItemInfo->bType == ITEM_METIN)
									break;
							}
						}
					}

					if (j >= ITEM_SOCKET_MAX_NUM)
						continue;

					lua_newtable( L );

					{
						lua_pushstring( L, pItem->GetName() );
						lua_rawseti( L, -2, 1 );

						lua_pushnumber( L, i );
						lua_rawseti( L, -2, 2 );
					}

					lua_rawseti( L, -2, idx++ );
				}
			}
		}

		return 1;
	}

	int32_t pc_get_empty_inventory_count(lua_State* L)
	{
		CHARACTER* pChar = CQuestManager::Instance().GetCurrentCharacterPtr();

		if (pChar)
			lua_pushnumber(L, pChar->CountEmptyInventory());
		else
			lua_pushnumber(L, 0);

		return 1;
	}

	int32_t pc_get_logoff_interval(lua_State* L)
	{
		CHARACTER* pChar = CQuestManager::Instance().GetCurrentCharacterPtr();

		if (pChar) 
			lua_pushnumber(L, pChar->GetLogOffInterval());
		else
			lua_pushnumber(L, 0);

		return 1;
	}

	int32_t pc_get_player_id( lua_State* L )
	{
		CHARACTER* pChar = CQuestManager::Instance().GetCurrentCharacterPtr();

		if (pChar)
			lua_pushnumber(L, pChar->GetPlayerID());
		else
			lua_pushnumber(L, 0);

		return 1;
	}

	int32_t pc_get_account_id( lua_State* L )
	{
		CHARACTER* pChar = CQuestManager::Instance().GetCurrentCharacterPtr();
		if (!pChar)
		{
			lua_pushnumber(L, 0);
			return 1;
		}
		
		if (pChar->GetDesc())
		{
			lua_pushnumber(L, pChar->GetDesc()->GetAccountTable().id);
			return 1;
		}

		lua_pushnumber(L, 0);
		return 1;
	}

	int32_t pc_get_account( lua_State* L )
	{
		CHARACTER* pChar = CQuestManager::Instance().GetCurrentCharacterPtr();
		if (!pChar)
		{
			lua_pushstring(L, "");
			return 1;
		}

		if (!pChar->GetDesc())
		{
			lua_pushstring(L, "");
			return 1;
		}

		lua_pushstring(L, pChar->GetDesc()->GetAccountTable().login);
		return 1;
	}

	int32_t pc_is_riding(lua_State* L)
	{
		CHARACTER* pChar = CQuestManager::Instance().GetCurrentCharacterPtr();
		if (!pChar)
		{
			lua_pushboolean(L, false);
			return 0;
		}

		bool is_riding = pChar->IsRiding();

		lua_pushboolean(L, is_riding);
		return 1;
	}

	int32_t pc_get_special_ride_vnum(lua_State* L)
	{
		CHARACTER* ch = CQuestManager::Instance().GetCurrentCharacterPtr();
		if (!ch)
		{
			sys_err("Null character pointer triggered at %s:%d", __FILE__, __LINE__);
			return 0;
		}

#ifdef ENABLE_MOUNT_COSTUME_SYSTEM
		CItem* CostumeMount = ch->GetWear(WEAR_COSTUME_MOUNT);
		if (!CostumeMount)
		{
			lua_pushnumber(L, 0);
			lua_pushnumber(L, 0);
			return 2;
		}

		if (ITEM_COSTUME == CostumeMount->GetType() && COSTUME_MOUNT == CostumeMount->GetSubType()) 
		{
			lua_pushnumber(L, CostumeMount->GetVnum());
			lua_pushnumber(L, CostumeMount->GetSocket(2));
			return 2;
		}
#endif

		lua_pushnumber(L, 0);
		lua_pushnumber(L, 0);
		return 2;
	}

	int32_t pc_can_warp(lua_State* L)
	{
		CHARACTER* pChar = CQuestManager::Instance().GetCurrentCharacterPtr();

		if (pChar)
			lua_pushboolean(L, pChar->CanWarp());
		else
			lua_pushboolean(L, false);

		return 1;
	}

	int32_t pc_dec_skill_point(lua_State* L)
	{
		CHARACTER* pChar = CQuestManager::Instance().GetCurrentCharacterPtr();
		if (pChar)
			pChar->PointChange(POINT_SKILL, -1);

		return 0;
	}

	int32_t pc_get_skill_point(lua_State* L)
	{
		CHARACTER* pChar = CQuestManager::Instance().GetCurrentCharacterPtr();

		if (pChar)
			lua_pushnumber(L, static_cast<int32_t>(pChar->GetPoint(POINT_SKILL)));
		else 
			lua_pushnumber(L, 0);

		return 1;
	}

	int32_t pc_get_channel_id(lua_State* L)
	{
		lua_pushnumber(L, g_bChannel);

		return 1;
	}

	int32_t pc_give_poly_marble(lua_State* L)
	{
		PC* pPC = CQuestManager::Instance().GetCurrentPC();
		if (!pPC)
		{
			sys_err("Null pc pointer triggered at %s:%d", __FILE__, __LINE__);
			return 0;
		}

		CHARACTER* ch = CQuestManager::Instance().GetCurrentCharacterPtr();
		if (!ch)
		{
			sys_err("Null character pointer triggered at %s:%d", __FILE__, __LINE__);
			return 0;
		}

		if (!lua_isnumber(L, 1))
		{
			sys_err("wrong argument");
			return 0;
		}
		int32_t dwVnum = (int32_t)lua_tonumber(L, 1);

		auto pMobInfo = CMobManager::Instance().Get(dwVnum);
		if (!pMobInfo)
		{
			lua_pushboolean(L, false);
			return 1;
		}

		if (pMobInfo->m_table.dwPolymorphItemVnum == 0)
		{
			lua_pushboolean(L, false);
			return 1;
		}

		auto item = ITEM_MANAGER::Instance().CreateItem(pMobInfo->m_table.dwPolymorphItemVnum);
		if (!item) 
		{
			lua_pushboolean(L, false);
			return 1;
		}

		item->SetSocket(0, dwVnum);

		int32_t iEmptyCell = ch->GetEmptyInventory(item->GetSize());
		if (-1 == iEmptyCell)
		{
			M2_DESTROY_ITEM(item);
			lua_pushboolean(L, false);
			return 1;
		}

		item->AddToCharacter(ch, TItemPos(INVENTORY, iEmptyCell));

		LogManager::Instance().QuestRewardLog(pPC->GetCurrentQuestName().c_str(), ch->GetPlayerID(), ch->GetLevel(), pMobInfo->m_table.dwPolymorphItemVnum, dwVnum);

		lua_pushboolean(L, true);
		return 1;
	}

	int32_t pc_get_sig_items (lua_State* L)
	{
		CHARACTER* ch = CQuestManager::Instance().GetCurrentCharacterPtr();
		if (!ch)
		{
			sys_err("Null character pointer triggered at %s:%d", __FILE__, __LINE__);
			return 0;
		}

		if (!lua_isnumber(L, 1))
		{
			sys_err("wrong argument");
			return 0;
		}
		uint32_t group_vnum = (uint32_t)lua_tonumber(L, 1);

		int32_t count = 0;
		for (int32_t i = 0; i < INVENTORY_MAX_NUM; ++i)
		{
			if (ch->GetInventoryItem(i) && ch->GetInventoryItem(i)->GetSIGVnum() == group_vnum) 
			{
				lua_pushnumber(L, ch->GetInventoryItem(i)->GetID());
				count++;
			}
		}

		return count;
	}		

	int32_t pc_charge_cash(lua_State * L)
	{
		CHARACTER* ch = CQuestManager::Instance().GetCurrentCharacterPtr();
		if (!ch)
		{
			sys_err("Null character pointer triggered at %s:%d", __FILE__, __LINE__);
			lua_pushboolean(L, false);
			return 1;
		}

		LPDESC desc = ch->GetDesc();
		if (!desc)
		{
			sys_err("Null desc pointer triggered at %s:%d", __FILE__, __LINE__);
			lua_pushboolean(L, false);
			return 1;
		}

		int32_t amount = lua_isnumber(L, 1) ? (int32_t)lua_tonumber(L, 1) : 0;
		std::string strChargeType = lua_isstring(L, 2) ? lua_tostring(L, 2) : "";

		if (1 > amount || 50000 < amount) 
		{
			lua_pushboolean(L, false);
			return 1;
		}

		TRequestChargeCash packet;
		packet.dwAID		= desc->GetAccountTable().id;
		packet.dwAmount		= (uint32_t)amount;
		packet.eChargeType	= ERequestCharge_Cash;

		if (!strChargeType.empty())
			stl_lowers(strChargeType);

		if ("mileage" == strChargeType)
			packet.eChargeType = ERequestCharge_Mileage;

		db_clientdesc->DBPacket(HEADER_GD_REQUEST_CHARGE_CASH, 0, &packet, sizeof(TRequestChargeCash));

		lua_pushboolean(L, true);
		return 1;
	}

	int32_t pc_give_award(lua_State* L)
	{
		CHARACTER* ch = CQuestManager::Instance().GetCurrentCharacterPtr();
		if (!ch)
		{
			sys_err("Null character pointer triggered at %s:%d", __FILE__, __LINE__);
			return 0;
		}

		LPDESC desc = ch->GetDesc();
		if (!desc)
		{
			sys_err("Null desc pointer triggered at %s:%d", __FILE__, __LINE__);
			lua_pushnumber(L, 0);
			return 1;
		}

		if (!lua_isnumber(L, 1) || !lua_isnumber(L, 2) || !lua_isstring(L, 3))
		{
			sys_err("QUEST give award call error : wrong argument");
			lua_pushnumber(L, 0);
			return 1;
		}

		uint32_t dwVnum = (int32_t)lua_tonumber(L, 1);
		int32_t icount = (int32_t)lua_tonumber(L, 2);

		sys_log(0, "QUEST [award] item %d to login %s", dwVnum, desc->GetAccountTable().login);

		DBManager::Instance().Query(
			"INSERT INTO item_award (login, vnum, count, given_time, why, "
			"mall)select '%s', %d, %d, now(), '%s', 1 from DUAL where not exists "
			"(select login, why from item_award where login = '%s' and why  = "
			"'%s') ;",
			desc->GetAccountTable().login, dwVnum, icount, lua_tostring(L, 3), 
			desc->GetAccountTable().login,
			lua_tostring(L, 3)
		);

		lua_pushnumber(L, 0);
		return 1;
	}
	int32_t pc_give_award_socket(lua_State* L)
	{
		CHARACTER* ch = CQuestManager::Instance().GetCurrentCharacterPtr();
		if (!ch)
		{
			sys_err("Null character pointer triggered at %s:%d", __FILE__, __LINE__);
			return 0;
		}

		LPDESC desc = ch->GetDesc();
		if (!desc)
		{
			sys_err("Null desc pointer triggered at %s:%d", __FILE__, __LINE__);
			lua_pushnumber(L, 0);
			return 1;
		}

		if (!lua_isnumber(L, 1) || !lua_isnumber(L, 2) || !lua_isstring(L, 3) || !lua_isstring(L, 4) || !lua_isstring(L, 5) || !lua_isstring(L, 6))
		{
			sys_err("QUEST give award call error : wrong argument");
			lua_pushnumber(L, 0);
			return 1;
		}

		uint32_t dwVnum = (int32_t)lua_tonumber(L, 1);
		int32_t icount = (int32_t)lua_tonumber(L, 2);

		sys_log(0, "QUEST [award] item %d to login %s", dwVnum, desc->GetAccountTable().login);

		DBManager::Instance().Query(
			"INSERT INTO item_award (login, vnum, count, given_time, why, mall, "
			"socket0, socket1, socket2)select '%s', %d, %d, now(), '%s', 1, %s, "
			"%s, %s from DUAL where not exists (select login, why from item_award "
			"where login = '%s' and why  = '%s') ;",
			desc->GetAccountTable().login, dwVnum, icount, lua_tostring(L, 3), lua_tostring(L, 4), 
			lua_tostring(L, 5), lua_tostring(L, 6),
			desc->GetAccountTable().login, lua_tostring(L, 3)
		);

		lua_pushnumber(L, 0);
		return 1;
	}

	int32_t pc_get_informer_type(lua_State* L)	//독일 선물 기능
	{
		CHARACTER* ch = CQuestManager::Instance().GetCurrentCharacterPtr();
		if (!ch)
		{
			sys_err("Null character pointer triggered at %s:%d", __FILE__, __LINE__);

			lua_pushstring(L, "");
			return 1;
		}

		lua_pushstring(L, ch->GetItemAward_cmd());
		return 1;
	}

	int32_t pc_get_informer_item(lua_State* L)
	{
		CHARACTER* ch = CQuestManager::Instance().GetCurrentCharacterPtr();
		if (!ch)
		{
			sys_err("Null character pointer triggered at %s:%d", __FILE__, __LINE__);

			lua_pushnumber(L, 0);
			return 1;
		}

		lua_pushnumber(L, ch->GetItemAward_vnum());
		return 1;
	}

	int32_t pc_get_killee_drop_pct(lua_State* L)
	{
		CHARACTER* pChar = CQuestManager::Instance().GetCurrentCharacterPtr();
		if (!pChar)
		{
			sys_err("Null character pointer triggered at %s:%d", __FILE__, __LINE__);
			return 0;
		}

		CHARACTER* pKillee = pChar->GetQuestNPC();
		if (!pKillee)
		{
			sys_err("Null npc pointer triggered at %s:%d", __FILE__, __LINE__);

			lua_pushnumber(L, -1);
			lua_pushnumber(L, -1);
			return 2;
		}

		int32_t iDeltaPercent, iRandRange;
		if (!ITEM_MANAGER::Instance().GetDropPct(pKillee, pChar, iDeltaPercent, iRandRange))
		{
			sys_err("GetDropPct fail");

			lua_pushnumber(L, -1);
			lua_pushnumber(L, -1);
			return 2;
		}

		lua_pushnumber(L, iDeltaPercent);
		lua_pushnumber(L, iRandRange);
		
		return 2;
	}


	int32_t pc_specific_effect(lua_State* L)
	{
		CHARACTER* ch = CQuestManager::Instance().GetCurrentCharacterPtr();
		if (!ch)
		{
			sys_err("Null character pointer triggered at %s:%d", __FILE__, __LINE__);
			return 0;
		}

		if (lua_isstring(L, 1)) 
			ch->SpecificEffectPacket(lua_tostring(L, 1));

		return 0;
	}

	int32_t pc_start_duel(lua_State* L)
	{
		CHARACTER* ch = CQuestManager::Instance().GetCurrentCharacterPtr();
		if (!ch)
		{
			sys_err("Null character pointer triggered at %s:%d", __FILE__, __LINE__);
			return 0;
		}

		if (!lua_isnumber(L, 1))
		{
			sys_err("invalid param");
			return 0;
		}

		uint32_t vid = (int32_t)lua_tonumber(L, 1);
		if (!vid) 
		{
			lua_pushnumber(L, 0);
			return 1;
		}

		CHARACTER* pkVictim = CHARACTER_MANAGER::Instance().Find(vid);
		if (!pkVictim) 
		{
			lua_pushnumber(L, 0);
			return 1;
		}

		if (!pkVictim->IsPC()) 
		{
			lua_pushnumber(L, 0);
			return 1;
		}

		CPVPManager::Instance().Insert(ch, pkVictim);
		CPVPManager::Instance().Insert(pkVictim, ch);

		lua_pushnumber(L, 0);
		return 1;
	}

	int32_t pc_stun_player(lua_State* L)
	{
		CHARACTER* ch = CQuestManager::Instance().GetCurrentCharacterPtr();
		if (!ch)
		{
			sys_err("Null character pointer triggered at %s:%d", __FILE__, __LINE__);
			return 0;
		}

		uint32_t time = 2;
		if (lua_isnumber(L, 1) && (int32_t)lua_tonumber(L, 1) > 0)
			time = (int32_t)lua_tonumber(L, 1);

		std::string reason = "QUEST_STUN";
		if (lua_isstring(L, 2))
			reason = lua_tostring(L, 2);

		int32_t IMMUNE_FLAG = IMMUNE_STUN;
		if (lua_isboolean(L, 3) && lua_toboolean(L, 3))
			IMMUNE_FLAG = 0;

		SkillAttackAffect(ch, 1000, IMMUNE_FLAG, AFFECT_STUN, POINT_NONE, 0, AFF_STUN, time, reason.c_str());
		return 0;
	}

	int32_t pc_acce_open_combine(lua_State* L)
	{
#ifdef ENABLE_ACCE_COSTUME_SYSTEM
		CHARACTER* ch = CQuestManager::Instance().GetCurrentCharacterPtr();
		if (!ch)
		{
			sys_err("Null character pointer triggered at %s:%d", __FILE__, __LINE__);
			return 0;
		}

		if (!ch->IsAcceWindowOpen()) 
		{
			ch->SetAcceWindowType(ACCE_SLOT_TYPE_COMBINE);
			ch->ChatPacket(CHAT_TYPE_COMMAND, "ShowAcceCombineDialog");
		} 
		else 
		{
			ch->ChatPacket(CHAT_TYPE_INFO, "ACCE_WINDOW_ALREADY_OPEN");
		}
#endif
		return 0;
	}

	int32_t pc_acce_open_absorb(lua_State* L)
	{
#ifdef ENABLE_ACCE_COSTUME_SYSTEM
		CHARACTER* ch = CQuestManager::Instance().GetCurrentCharacterPtr();
		if (!ch)
		{
			sys_err("Null character pointer triggered at %s:%d", __FILE__, __LINE__);
			return 0;
		}

		if (!ch->IsAcceWindowOpen()) 
		{
			ch->SetAcceWindowType(ACCE_SLOT_TYPE_ABSORB);
			ch->ChatPacket(CHAT_TYPE_COMMAND, "ShowAcceAbsorbDialog");
		} 
		else 
		{
			ch->ChatPacket(CHAT_TYPE_INFO, ("ACCE_WINDOW_ALREADY_OPEN"));
		}
#endif
		return 0;
	}

	int32_t pc_reencode_view(lua_State* L)
	{
		CHARACTER* ch = CQuestManager::Instance().GetCurrentCharacterPtr();
		if (!ch)
		{
			sys_err("Null character pointer triggered at %s:%d", __FILE__, __LINE__);
			return 0;
		}

		ch->ViewReencode();
		return 0;
	}

	int32_t pc_get_mount_vnum(lua_State * L)
	{
		CHARACTER* ch = CQuestManager::Instance().GetCurrentCharacterPtr();
		if (!ch) 
		{
			sys_err("Null character pointer triggered at %s:%d", __FILE__, __LINE__);
			return 0;
		}

		lua_pushnumber(L, (ch && ch->IsRiding()) ? ch->GetMountVnum() : 0);
		return 1;
	}

	int32_t pc_get_point(lua_State * L)
	{
		CHARACTER* ch = CQuestManager::Instance().GetCurrentCharacterPtr();
		if (!ch)
		{
			lua_pushnumber(L, 0);
			return 1;
		}

		if (!lua_isnumber(L, 1))
		{
			sys_err("Argument error.");

			lua_pushnumber(L, 0);
			return 1;
		}

		uint8_t bPoint = (uint8_t)lua_tonumber(L, 1);
		if (bPoint < 0 || bPoint >= POINT_MAX_NUM)
		{
			sys_err("Invalid point (%d).", bPoint);

			lua_pushnumber(L, 0);
			return 1;
		}

		lua_pushnumber(L, static_cast<int32_t>(ch->GetPoint(bPoint)));
		return 1;
	}

	int32_t pc_get_real_point(lua_State * L)
	{
		CHARACTER* ch = CQuestManager::Instance().GetCurrentCharacterPtr();
		if (!ch) 
		{
			sys_err("Null character pointer triggered at %s:%d", __FILE__, __LINE__);
			return 0;
		}

		if (!lua_isnumber(L, 1))
		{
			sys_err("Argument error.");

			lua_pushnumber(L, 0);
			return 1;
		}

		uint8_t bPoint = (uint8_t)lua_tonumber(L, 1);
		if (bPoint <= POINT_NONE || bPoint >= POINT_MAX_NUM)
		{
			sys_err("Invalid point (%d).", bPoint);

			lua_pushnumber(L, 0);
			return 1;
		}

		lua_pushnumber(L, static_cast<int32_t>(ch->GetRealPoint(bPoint)));
		return 1;
	}

	int32_t pc_disconnect_with_delay(lua_State * L)
	{
		CHARACTER* ch = CQuestManager::Instance().GetCurrentCharacterPtr();
		if (!ch)
		{
			sys_err("Null character pointer triggered at %s:%d", __FILE__, __LINE__);
			return 0;
		}

		if (!ch->GetDesc())
			return 0;

		ch->GetDesc()->DelayedDisconnect(lua_isnumber(L, 1) ? (int32_t)lua_tonumber(L, 1) : 10);
		return 0;
	}

	int32_t pc_get_max_level(lua_State* L)
	{
		lua_pushnumber(L, gPlayerMaxLevel);
		return 1;
	}

	int32_t pc_get_ip(lua_State* L)
	{
		CHARACTER* ch = CQuestManager::Instance().GetCurrentCharacterPtr();
		if (!ch)
			return 0;

		if (!ch->GetDesc())
			return 0;

		lua_pushstring(L, ch->GetDesc()->GetHostName());
		return 1;
	}

	int32_t pc_kill(lua_State* L)
	{
		CHARACTER* ch = CQuestManager::Instance().GetCurrentCharacterPtr();
		if (!ch)
			return 0;

		ch->Dead();
		return 0;
	}

	int32_t pc_set_coins(lua_State * L)
	{
		if (!lua_isnumber(L, 1))
		{
			sys_err("invalid argument");
			return 0;
		}
		int32_t val = (int32_t)lua_tonumber(L, 1);

		CHARACTER* ch = CQuestManager::Instance().GetCurrentCharacterPtr();
		if (!ch) 
		{
			sys_err("Null character pointer triggered at %s:%d", __FILE__, __LINE__);
			return 0;
		}

		std::unique_ptr<SQLMsg> pmsg(DBManager::Instance().DirectQuery("UPDATE account.account SET coins = coins + '%ld' WHERE id = '%d'", val, ch->GetAID()));
		if (pmsg->uiSQLErrno != 0)
		{
			sys_err("pc_update_coins query failed");
			return 0;
		}

		return 0;
	}

	int32_t pc_get_empire_name(lua_State* L)
	{
		CHARACTER* ch = CQuestManager::Instance().GetCurrentCharacterPtr();
		if (!ch) 
		{
			sys_err("Null character pointer triggered at %s:%d", __FILE__, __LINE__);
			return 0;
		}

		const char* table[3] = { "Shinsoo", "Chunjo", "Jinno" };
		int32_t _empire = ch->GetEmpire() - 1;

		lua_pushstring(L, table[_empire]);
		return 1;
	}

	int32_t pc_set_race(lua_State* L)
	{
		CHARACTER* ch = CQuestManager::Instance().GetCurrentCharacterPtr();
		if (!ch) 
		{
			sys_err("Null character pointer triggered at %s:%d", __FILE__, __LINE__);
			return 0;
		}

		ESex mySex = GET_SEX(ch);
		uint32_t dwRace = MAIN_RACE_WARRIOR_M;

		int32_t amount = MINMAX(0, (int32_t)lua_tonumber(L, 1), JOB_MAX_NUM);
		switch (amount)
		{
			case JOB_WARRIOR:
				dwRace = (mySex == SEX_MALE) ? MAIN_RACE_WARRIOR_M : MAIN_RACE_WARRIOR_W;
				break;
			case JOB_ASSASSIN:
				dwRace = (mySex == SEX_MALE) ? MAIN_RACE_ASSASSIN_M : MAIN_RACE_ASSASSIN_W;
				break;
			case JOB_SURA:
				dwRace = (mySex == SEX_MALE) ? MAIN_RACE_SURA_M : MAIN_RACE_SURA_W;
				break;
			case JOB_SHAMAN:
				dwRace = (mySex == SEX_MALE) ? MAIN_RACE_SHAMAN_M : MAIN_RACE_SHAMAN_W;
				break;
#ifdef ENABLE_WOLFMAN_CHARACTER
			case JOB_WOLFMAN:
				dwRace = (mySex == SEX_MALE) ? MAIN_RACE_WOLFMAN_M : MAIN_RACE_WOLFMAN_M;
				break;
#endif
		}

		if (dwRace != ch->GetRaceNum())
		{
			ch->SetRace(dwRace);
			ch->ClearSkill();
			ch->SetSkillGroup(0);

			// quick mesh change workaround begin
			ch->SetPolymorph(101);
			ch->SetPolymorph(0);
			// quick mesh change workaround end
		}
		return 0;
	}

	int32_t pc_del_another_quest_flag(lua_State* L)
	{
		PC* pPC = CQuestManager::Instance().GetCurrentPC();
		if (!pPC)
		{
			sys_err("Null pc pointer triggered at %s:%d", __FILE__, __LINE__);
			return 0;
		}

		if (!lua_isstring(L, 1) || !lua_isstring(L, 2))
		{
			sys_err("QUEST wrong del flag");
			return 0;
		}	
		const char * sz = lua_tostring(L, 1);
		const char * sz2 = lua_tostring(L, 2);

		lua_pushboolean(L, pPC->DeleteFlag(std::string(sz) + "." + sz2));
		return 1;
	}

	int32_t pc_pointchange(lua_State* L)
	{
		CHARACTER* ch = CQuestManager::Instance().GetCurrentCharacterPtr();
		if (!ch) 
		{
			sys_err("Null character pointer triggered at %s:%d", __FILE__, __LINE__);
			return 0;
		}

		if (!lua_isnumber(L, 1) || !lua_isnumber(L, 2) || !lua_isboolean(L, 3) || !lua_isboolean(L, 4))
		{
			sys_err("QUEST wrong arg");
			return 0;
		}

		ch->PointChange((uint8_t)lua_tonumber(L, 1), (int64_t)lua_tonumber(L, 2), lua_toboolean(L, 3), lua_toboolean(L, 4));
		return 0;
	}

	int32_t pc_pullmob(lua_State* L)
	{
		CHARACTER* ch = CQuestManager::Instance().GetCurrentCharacterPtr();
		if (!ch) 
		{
			sys_err("Null character pointer triggered at %s:%d", __FILE__, __LINE__);
			return 0;
		}

		ch->PullMonster();
		return 0;
	}

	int32_t pc_set_level2(lua_State* L)
	{
		CHARACTER* ch = CQuestManager::Instance().GetCurrentCharacterPtr();
		if (!ch) 
		{
			sys_err("Null character pointer triggered at %s:%d", __FILE__, __LINE__);
			return 0;
		}

		if (!lua_isnumber(L, 1))
		{
			sys_err("QUEST wrong arg");
			return 0;
		}
		ch->ResetPoint((int32_t)lua_tonumber(L, 1));

		ch->ClearSkill();
		ch->ClearSubSkill();

		return 0;
	}

	int32_t pc_set_gm_level(lua_State* L)
	{
		CHARACTER* ch = CQuestManager::Instance().GetCurrentCharacterPtr();
		if (!ch) 
		{
			sys_err("Null character pointer triggered at %s:%d", __FILE__, __LINE__);
			return 0;
		}

		ch->SetGMLevel();
		return 0;
	}

	int32_t pc_if_fire(lua_State* L)
	{
		CHARACTER* ch = CQuestManager::Instance().GetCurrentCharacterPtr();
		if (!ch)
		{
			sys_err("Null character pointer triggered at %s:%d", __FILE__, __LINE__);
			return 0;
		}

		lua_pushboolean(L, ch->IsAffectFlag(AFF_FIRE));
		return 1;
	}
	int32_t pc_if_invisible(lua_State* L)
	{
		CHARACTER* ch = CQuestManager::Instance().GetCurrentCharacterPtr();
		if (!ch)
		{
			sys_err("Null character pointer triggered at %s:%d", __FILE__, __LINE__);
			return 0;
		}

		lua_pushboolean(L, ch->IsAffectFlag(AFF_INVISIBILITY));
		return 1;
	}
	int32_t pc_if_poison(lua_State* L)
	{
		CHARACTER* ch = CQuestManager::Instance().GetCurrentCharacterPtr();
		if (!ch) 
		{
			sys_err("Null character pointer triggered at %s:%d", __FILE__, __LINE__);
			return 0;
		}

		lua_pushboolean(L, ch->IsAffectFlag(AFF_POISON));
		return 1;
	}
	int32_t pc_if_slow(lua_State* L)
	{
		CHARACTER* ch = CQuestManager::Instance().GetCurrentCharacterPtr();
		if (!ch) 
		{
			sys_err("Null character pointer triggered at %s:%d", __FILE__, __LINE__);
			return 0;
		}

		lua_pushboolean(L, ch->IsAffectFlag(AFF_SLOW));
		return 1;
	}
	int32_t pc_if_stun(lua_State* L)
	{
		CHARACTER* ch = CQuestManager::Instance().GetCurrentCharacterPtr();
		if (!ch) 
		{
			sys_err("Null character pointer triggered at %s:%d", __FILE__, __LINE__);
			return 0;
		}

		lua_pushboolean(L, ch->IsAffectFlag(AFF_STUN));
		return 1;
	}

	int32_t pc_sf_fire(lua_State* L)
	{
		CHARACTER* ch = CQuestManager::Instance().GetCurrentCharacterPtr();
		if (!ch)
		{
			sys_err("Null character pointer triggered at %s:%d", __FILE__, __LINE__);
			return 0;
		}

		if (!lua_isboolean(L, 1))
		{
			sys_err("QUEST wrong arg");
			return 0;
		}

		if (lua_toboolean(L, 1))
			ch->AddAffect(AFFECT_FIRE, 0, 0, AFF_FIRE, 3 * 5 + 1, 0, 1, 0);
		else
			ch->RemoveAffect(AFFECT_FIRE);

		return 0;
	}

	int32_t pc_sf_invisible(lua_State* L)
	{
		CHARACTER* ch = CQuestManager::Instance().GetCurrentCharacterPtr();
		if (!ch) 
		{
			sys_err("Null character pointer triggered at %s:%d", __FILE__, __LINE__);
			return 0;
		}

		if (!lua_isboolean(L, 1))
		{
			sys_err("QUEST wrong arg");
			return 0;
		}

		if (lua_toboolean(L, 1))
			ch->AddAffect(AFFECT_INVISIBILITY, 0, 0, AFF_INVISIBILITY, 60 * 60 * 24 * 365 * 60 + 1, 0, 1, 0);
		else
			ch->RemoveAffect(AFFECT_INVISIBILITY);

		return 0;
	}

	int32_t pc_sf_poison(lua_State* L)
	{
		CHARACTER* ch = CQuestManager::Instance().GetCurrentCharacterPtr();
		if (!ch) 
		{
			sys_err("Null character pointer triggered at %s:%d", __FILE__, __LINE__);
			return 0;
		}

		if (!lua_isboolean(L, 1))
		{
			sys_err("QUEST wrong arg");
			return 0;
		}

		if (lua_toboolean(L, 1))
			ch->AddAffect(AFFECT_POISON, 0, 0, AFF_POISON, 30 + 1, 0, 1, 0);
		else
			ch->RemoveAffect(AFFECT_POISON);

		return 0;
	}

	int32_t pc_sf_slow(lua_State* L)
	{
		CHARACTER* ch = CQuestManager::Instance().GetCurrentCharacterPtr();
		if (!ch) 
		{
			sys_err("Null character pointer triggered at %s:%d", __FILE__, __LINE__);
			return 0;
		}

		if (!lua_isboolean(L, 1))
		{
			sys_err("QUEST wrong arg");
			return 0;
		}

		if (lua_toboolean(L, 1))
			ch->AddAffect(AFFECT_SLOW, 19, -30, AFF_SLOW, 30, 0, 1, 0);
		else
			ch->RemoveAffect(AFFECT_SLOW);

		return 0;
	}
	int32_t pc_sf_stun(lua_State* L)
	{
		CHARACTER* ch = CQuestManager::Instance().GetCurrentCharacterPtr();
		if (!ch)
		{
			sys_err("Null character pointer triggered at %s:%d", __FILE__, __LINE__);
			return 0;
		}

		if (!lua_isboolean(L, 1))
		{
			sys_err("QUEST wrong arg");
			return 0;
		}

		if (lua_toboolean(L, 1))
			ch->AddAffect(AFFECT_STUN, 0, 0, AFF_STUN, 30, 0, 1, 0);
		else
			ch->RemoveAffect(AFFECT_STUN);

		return 0;
	}

	int32_t pc_open_shop(lua_State* L)
	{
		CHARACTER* ch = CQuestManager::Instance().GetCurrentCharacterPtr();
		if (!ch) 
		{
			sys_err("Null character pointer triggered at %s:%d", __FILE__, __LINE__);
			return 0;
		}

		if (!lua_isnumber(L, 1))
		{
			sys_err("QUEST wrong arg");
			return 0;
		}

		//PREVENT_TRADE_WINDOW
		if (ch->IsOpenSafebox() || ch->GetExchange() || ch->GetMyShop() || ch->IsCubeOpen())
		{
			ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("다른 거래창이 열린상태에서는 상점거래를 할수 가 없습니다."));
			return 0;
		}
		//END_PREVENT_TRADE_WINDOW

		LPSHOP sh = CShopManager::Instance().Get((uint32_t)lua_tonumber(L, 1)); 
		if (sh)
		{
			sh->AddGuest(ch, 0, false);
		}

		return 0;
	}

	int32_t pc_sf_kill(lua_State* L)
	{
		CHARACTER* ch = CHARACTER_MANAGER::Instance().FindPC(lua_tostring(L, 1));
		if (ch)
			ch->Dead(0, 0);

		return 0;
	}

	int32_t pc_sf_dead(lua_State* L)
	{
		CHARACTER* ch = CQuestManager::Instance().GetCurrentCharacterPtr();
		if (!ch) 
		{
			sys_err("Null character pointer triggered at %s:%d", __FILE__, __LINE__);
			return 0;
		}

		ch->Dead(0, 0);
		return 0;
	}

	int32_t pc_get_exp_level(lua_State* L)
	{
		if (!lua_isnumber(L, 1))
		{
			sys_err("arg1 must be a number");
			return 0;
		}

		lua_pushnumber(L, (uint32_t)(exp_table[MINMAX(0, (int32_t)lua_tonumber(L, 1), PLAYER_MAX_LEVEL_CONST)] / 100));
		return 1;
	}

	int32_t pc_set_max_health(lua_State* L)
	{
		CHARACTER* ch = CQuestManager::Instance().GetCurrentCharacterPtr();
		if (!ch) 
		{
			sys_err("Null character pointer triggered at %s:%d", __FILE__, __LINE__);
			return 0;
		}

		ch->PointChange(POINT_HP, ch->GetMaxHP() - ch->GetHP());
		ch->PointChange(POINT_SP, ch->GetMaxSP() - ch->GetSP());
		return 0;
	}

	int32_t pc_dc_delayed(lua_State* L)
	{
		CHARACTER* ch = CQuestManager::Instance().GetCurrentCharacterPtr();
		if (!ch) 
		{
			sys_err("Null character pointer triggered at %s:%d", __FILE__, __LINE__);
			return 0;
		}

		if (!ch->GetDesc())
			return 0;

		bool bRet = ch->GetDesc()->DelayedDisconnect(MINMAX(0, (int32_t)lua_tonumber(L, 1), 60 * 5));
		lua_pushboolean(L, bRet);
		return 1;
	}

	int32_t pc_dc_direct(lua_State* L)
	{
		CHARACTER* ch = CQuestManager::Instance().GetCurrentCharacterPtr();
		if (!ch)
			return 0;

		if (!lua_isstring(L, 1))
		{
			sys_err("argument error");
			return 0;
		}

		ch->Disconnect(lua_tostring(L, 1));
		return 0;
	}

	int32_t pc_is_trade(lua_State* L)
	{
		CHARACTER* ch = CQuestManager::Instance().GetCurrentCharacterPtr();
		if (!ch) 
		{
			sys_err("Null character pointer triggered at %s:%d", __FILE__, __LINE__);
			return 0;
		}

		lua_pushboolean(L, ch->GetExchange() != nullptr);
		return 1;
	}

	int32_t pc_is_busy(lua_State* L)
	{
		CHARACTER* ch = CQuestManager::Instance().GetCurrentCharacterPtr();
		if (!ch) 
		{
			sys_err("Null character pointer triggered at %s:%d", __FILE__, __LINE__);
			return 0;
		}

		lua_pushboolean(L, (ch->GetExchange() || ch->GetMyShop() || ch->IsOpenSafebox() || ch->IsCubeOpen()));
		return 1;
	}

	int32_t pc_is_arena(lua_State* L)
	{
		CHARACTER* ch = CQuestManager::Instance().GetCurrentCharacterPtr();
		if (!ch) 
		{
			sys_err("Null character pointer triggered at %s:%d", __FILE__, __LINE__);
			return 0;
		}

		lua_pushboolean(L, ch->GetArena() != nullptr);
		return 1;
	}

	int32_t pc_is_arena_observer(lua_State* L)
	{
		CHARACTER* ch = CQuestManager::Instance().GetCurrentCharacterPtr();
		if (!ch) 
		{
			sys_err("Null character pointer triggered at %s:%d", __FILE__, __LINE__);
			return 0;
		}

		lua_pushboolean(L, ch->GetArenaObserverMode());
		return 1;
	}

	int32_t pc_equip_slot(lua_State* L)
	{
		CHARACTER* ch = CQuestManager::Instance().GetCurrentCharacterPtr();
		if (!ch) 
		{
			sys_err("Null character pointer triggered at %s:%d", __FILE__, __LINE__);
			return 0;
		}

		if (!lua_isnumber(L, 1))
		{
			sys_err("argument error");
			return 0;
		}

		CItem * item = ch->GetInventoryItem((uint16_t)lua_tonumber(L, 1));
		if (!item)
			return 0;

		lua_pushboolean(L, item ? ch->EquipItem(item) : false);
		return 1;
	}

	int32_t pc_unequip_slot(lua_State* L)
	{
		CHARACTER* ch = CQuestManager::Instance().GetCurrentCharacterPtr();
		if (!ch) 
		{
			sys_err("Null character pointer triggered at %s:%d", __FILE__, __LINE__);
			return 0;
		}

		if (!lua_isnumber(L, 1))
		{
			sys_err("argument error");
			return 0;
		}

		CItem * item = ch->GetWear((uint16_t)lua_tonumber(L, 1));
		if (!item)
			return 0;

		lua_pushboolean(L, item ? ch->UnequipItem(item) : false);
		return 1;
	}

	int32_t pc_is_available(lua_State* L)
	{
		if (!lua_isnumber(L, 1))
			return 0;
		int32_t iForceCheck = (int32_t)lua_tonumber(L, 1);

		CHARACTER* ch = CQuestManager::Instance().GetCurrentCharacterPtr();
		if (!iForceCheck)
			lua_pushboolean(L, ch != nullptr);
		else
			lua_pushboolean(L, ch->GetDesc() != nullptr);

		return 1;
	}

	int32_t pc_is_pvp(lua_State* L)
	{
		CHARACTER* ch = CQuestManager::Instance().GetCurrentCharacterPtr();
		if (!ch)
		{
			sys_err("Null character pointer triggered at %s:%d", __FILE__, __LINE__);
			return 0;
		}

		CHARACTER* npc = CQuestManager::Instance().GetCurrentNPCCharacterPtr();
		if (!npc) 
		{
			sys_err("Null npc pointer triggered at %s:%d", __FILE__, __LINE__);
			return 0;
		}

		if (!npc->IsPC() || !ch->IsPC())
		{
			lua_pushboolean(L, 0);
			return 1;
		}

		CPVP kPVP(ch->GetPlayerID(), npc->GetPlayerID());
		CPVP * pkPVP = CPVPManager::Instance().Find(kPVP.GetCRC());

		if (!pkPVP || !pkPVP->IsFight())
		{
			lua_pushboolean(L, 0);
			return 1;
		}

		lua_pushboolean(L, 1);
		return 1;
	}

	int32_t pc_costume_hide(lua_State* L)
	{
		CHARACTER* ch = CQuestManager::Instance().GetCurrentCharacterPtr();
		if (!ch) 
		{
			sys_err("Null character pointer triggered at %s:%d", __FILE__, __LINE__);
			return 0;
		}

		if (!lua_isnumber(L, 1))
			return 0;

		int32_t part_value = 0;
		bool enable = (bool)lua_tonumber(L, 1);
		if (enable)
		{
			const CItem* pArmor = ch->GetWear(WEAR_BODY);
			part_value = (nullptr != pArmor) ? pArmor->GetVnum() : ch->GetOriginalPart(PART_MAIN);
		}
		else
		{
			const CItem* pArmor = ch->GetWear(WEAR_COSTUME_BODY);
			part_value = (nullptr != pArmor) ? pArmor->GetVnum() : ch->GetOriginalPart(PART_MAIN);
		}

		//ch->ChatPacket(CHAT_TYPE_INFO, "CostumeVisible : %d -> %d", (int32_t)ch->CostumeVisible(), (int32_t)!enable);
		//ch->SetCostumeVisible(!enable);

		ch->SetPart(PART_MAIN, part_value);
		ch->UpdatePacket();

		return 0;
	}

	int32_t pc_set_observer_mode(lua_State* L)
	{
		CHARACTER* ch = CQuestManager::Instance().GetCurrentCharacterPtr();
		if (!ch) 
		{
			sys_err("Null character pointer triggered at %s:%d", __FILE__, __LINE__);
			return 0;
		}

		ch->SetObserverMode(lua_tonumber(L, 1) ? true : false);
		return 0;
	}

	int32_t pc_get_real_x(lua_State* L)
	{
		CHARACTER* ch = CQuestManager::Instance().GetCurrentCharacterPtr();
		if (!ch) 
		{
			sys_err("Null character pointer triggered at %s:%d", __FILE__, __LINE__);
			return 0;
		}

		lua_pushnumber(L, ch->GetX());
		return 1;
	}

	int32_t pc_get_real_y(lua_State* L)
	{
		CHARACTER* ch = CQuestManager::Instance().GetCurrentCharacterPtr();
		if (!ch) 
		{
			sys_err("Null character pointer triggered at %s:%d", __FILE__, __LINE__);
			return 0;
		}

		lua_pushnumber(L, ch->GetY());
		return 1;
	}

	int32_t pc_get_quest_data(lua_State* L)
	{
		PC* pPC = CQuestManager::Instance().GetCurrentPC();
		if (!pPC)
		{
			sys_err("Null pc pointer triggered at %s:%d", __FILE__, __LINE__);
			return 0;
		}

		if (!lua_isstring(L, -1))
			return 0;
		const char* sz = lua_tostring(L, -1);
			
		lua_pushnumber(L, pPC->GetFlag(std::string(sz)));
		return 1;
	}

	int32_t pc_find_item_select(lua_State* L)
	{
		LPCHARACTER ch = quest::CQuestManager::Instance().GetCurrentCharacterPtr();
		if (!ch || !lua_isnumber(L, 1)) 
		{
			return 0;
		}

		CItem * item = ch->FindSpecifyItem((uint32_t)lua_tonumber(L, 1));
		if (!item) 
		{
			lua_pushboolean(L, false);
			return 1;
		}

		quest::CQuestManager::Instance().SetCurrentItem(item);
		lua_pushboolean(L, true);
		return 1;
	}

	int32_t pc_damage(lua_State* L)
	{
		CHARACTER* ch = quest::CQuestManager::Instance().GetCurrentCharacterPtr();
		if (!ch) 
		{
			sys_err("Null character pointer triggered at %s:%d", __FILE__, __LINE__);
			return 0;
		}

		if (!lua_isnumber(L, 1) || !lua_isnumber(L, 2)) 
		{
			return 0;
		}
		uint32_t vid = (uint32_t)lua_tonumber(L, 1);
		int32_t dam = (int32_t)lua_tonumber(L, 2);

		CHARACTER* pkVictim = CHARACTER_MANAGER::Instance().Find(vid);
		if (pkVictim)
			pkVictim->Damage(ch, dam, DAMAGE_TYPE_NORMAL);

		return 0;
	}

	int32_t pc_pickup_item(lua_State* L)
	{
		CHARACTER* ch = CQuestManager::Instance().GetCurrentCharacterPtr();
		if (!lua_isnumber(L, 1) || !ch) 
		{
			lua_pushboolean(L, false);
			return 0;
		}

		if (ch->PickupItem((uint32_t)lua_tonumber(L, 1)))
			lua_pushboolean(L, true);
		else
			lua_pushboolean(L, false);

		return 1;
	}

	int32_t pc_get_damage(lua_State* L)
	{
		LPCHARACTER pc = quest::CQuestManager::Instance().GetCurrentCharacterPtr();
		if (!pc)
		{
			lua_pushnumber(L, 0);
			return 1;
		}

		uint32_t raceStat = 1;
		switch (pc->GetJob())
		{
			case JOB_WARRIOR:
			case JOB_SURA:
				raceStat = static_cast<uint32_t>(pc->GetPoint(POINT_ST));
				break;
			case JOB_ASSASSIN:
#ifdef ENABLE_WOLFMAN_CHARACTER
			case JOB_WOLFMAN:
#endif
				raceStat = static_cast<uint32_t>(pc->GetPoint(POINT_DX));
				break;
			case JOB_SHAMAN:
				raceStat = static_cast<uint32_t>(pc->GetPoint(POINT_IQ));
				break;
		}

		uint32_t weaponAtkMin = 0;
		uint32_t weaponAtkMax = 0;
		uint32_t weaponRefineBonus = 0;

		CItem * weapon = pc->GetWear(WEAR_WEAPON);
		if (weapon)
		{
			if (weapon->GetType() != ITEM_ROD || weapon->GetType() != ITEM_PICK) 
			{
				weaponRefineBonus = weapon->GetValue(5);
				weaponAtkMin = 2 * (weaponRefineBonus + weapon->GetValue(3));
				weaponAtkMax = 2 * (weaponRefineBonus + weapon->GetValue(4));
			}
		}

		int32_t hitRateSrc = (static_cast<int32_t>(pc->GetPoint(POINT_DX)) * 4 + pc->GetLevel() * 2) / 6;
		int32_t hitRate = 100 * (MIN(90, hitRateSrc) + 210) / 300;

		uint32_t gradeBonus = static_cast<uint32_t>(pc->GetPoint(POINT_ATT_GRADE_BONUS)) + static_cast<uint32_t>(pc->GetPoint(POINT_PARTY_BUFFER_BONUS));

		uint32_t levelAtk = 2 * pc->GetLevel();
		uint32_t statAtk = (4 * static_cast<uint32_t>(pc->GetPoint(POINT_ST)) + 2 * raceStat) / 3;
		lua_pushnumber(L, (levelAtk + (statAtk + weaponAtkMin) * hitRate / 100) + gradeBonus);
		lua_pushnumber(L, (levelAtk + (statAtk + weaponAtkMax) * hitRate / 100) + gradeBonus);
		return 2;
	}

	int32_t pc_set_hp(lua_State* L)
	{
		LPCHARACTER ch = quest::CQuestManager::Instance().GetCurrentCharacterPtr();
		if (!ch)
		{
			sys_err("Null character pointer triggered at %s:%d", __FILE__, __LINE__);
			return 0;
		}

		if (!lua_isnumber(L, 1))
			return 0;

		ch->PointChange(POINT_HP, (int64_t)lua_tonumber(L, 1), true, false);
		return 0;
	}

	int32_t pc_block_exp(lua_State* L)
	{
		CHARACTER* ch = CQuestManager::Instance().GetCurrentCharacterPtr();
		if (!ch) 
		{
			sys_err("Null character pointer triggered at %s:%d", __FILE__, __LINE__);
			return 0;
		}

		ch->BlockExp();
		ch->ChatPacket(CHAT_TYPE_INFO, "Anti Exp AKTIF!");

		return 0;
	}

	int32_t pc_unblock_exp(lua_State* L)
	{
		CHARACTER* ch = CQuestManager::Instance().GetCurrentCharacterPtr();
		if (!ch) 
		{
			sys_err("Null character pointer triggered at %s:%d", __FILE__, __LINE__);
			return 0;
		}

		ch->UnblockExp();
		ch->ChatPacket(CHAT_TYPE_INFO, "Anti Exp PASIF!");

		return 0;
	}

	int32_t pc_if_in_duel(lua_State* L)
	{
		CHARACTER* ch = CQuestManager::Instance().GetCurrentCharacterPtr();
		if (!ch) 
		{
			sys_err("Null character pointer triggered at %s:%d", __FILE__, __LINE__);
			return 0;
		}

		if (CPVPManager::Instance().IsDuelingInstance(ch))
			lua_pushboolean(L, true);
		else
			lua_pushboolean(L, false);

		return 0;
	}

	void RegisterPCFunctionTable()
	{
		luaL_reg pc_functions[] = 
		{
			{	"get_wear",								pc_get_wear								},

			{	"get_player_id",						pc_get_player_id						},
			{	"get_account_id",						pc_get_account_id						},
			{	"get_account",							pc_get_account							},

			{	"get_level",							pc_get_level							},
			{	"set_level",							pc_set_level							},

			{	"get_next_exp",							pc_get_next_exp							},
			{	"get_exp",								pc_get_exp								},

			{	"get_job",								pc_get_job								},
			{	"get_race",								pc_get_race								},
			{	"change_sex",							pc_change_sex							},

			{	"gethp",								pc_get_hp								},
			{	"get_hp",								pc_get_hp								},
			{	"getmaxhp",								pc_get_max_hp							},
			{	"get_max_hp",							pc_get_max_hp},

			{	"getsp",								pc_get_sp								},
			{	"get_sp",								pc_get_sp								},
			{	"getmaxsp",								pc_get_max_sp							},
			{	"get_max_sp",							pc_get_max_sp							},
			{	"change_sp",							pc_change_sp							},

			{	"getmoney",								pc_get_money							},
			{	"get_money",							pc_get_money							},
			{	"getgold",								pc_get_money							},
			{	"get_gold",								pc_get_money							},

			{	"get_real_alignment",					pc_get_real_alignment					},
			{	"get_alignment",						pc_get_alignment						},

			{	"getweapon",							pc_get_weapon							},
			{	"get_weapon",							pc_get_weapon							},

			{	"getarmor",								pc_get_armor							},
			{	"get_armor",							pc_get_armor},

			{	"changegold",							pc_change_money							},
			{	"changemoney",							pc_change_money							},
			{	"change_gold",							pc_change_money							},
			{	"change_money",							pc_change_money							},

			{	"changealignment",						pc_change_alignment						},
			{	"change_alignment",						pc_change_alignment						},

			{	"getname",								pc_get_name								},
			{	"get_name",								pc_get_name								},

			{	"get_vid",								pc_get_vid								},

			{	"getplaytime",							pc_get_playtime							},
			{	"get_playtime",							pc_get_playtime							},

			{	"getleadership",						pc_get_leadership						},
			{	"get_leadership",						pc_get_leadership						},

			{	"getqf",								pc_get_quest_flag						},
			{	"setqf",								pc_set_quest_flag						},
			{	"delqf",								pc_del_quest_flag						},

			{	"getf",									pc_get_another_quest_flag				},
			{	"setf",									pc_set_another_quest_flag				},

			{	"get_x",								pc_get_x								},
			{	"get_y",								pc_get_y								},
			{	"getx",									pc_get_x								},
			{	"gety",									pc_get_y								},
			{	"get_local_x",							pc_get_local_x							},
			{	"get_local_y",							pc_get_local_y							},

			{	"getcurrentmapindex",					pc_get_current_map_index				},
			{	"get_map_index",						pc_get_current_map_index				},

			{	"give_exp",								pc_give_exp								},
			{	"give_exp_perc",						pc_give_exp_perc						},
			{	"get_exp_perc",							pc_get_exp_perc							},
			{	"give_exp2",							pc_give_exp2							},

			{	"give_item",							pc_give_item							},
			{	"give_item2",							pc_give_or_drop_item					},
	#ifdef ENABLE_DICE_SYSTEM
			{	"give_item2_with_dice",					pc_give_or_drop_item_with_dice			},
	#endif
			{	"give_item2_select",					pc_give_or_drop_item_and_select			},

			{	"give_gold",							pc_give_gold							},

			{	"count_item",							pc_count_item							},
			{	"countitem",							pc_count_item							},
			{	"remove_item",							pc_remove_item							},
			{	"removeitem",							pc_remove_item							},
			{	"find_first_item",						pc_find_first_item						},

			{	"reset_point",							pc_reset_point							},

			{	"has_guild",							pc_hasguild								},
			{	"hasguild",								pc_hasguild								},
			{	"get_guild",							pc_getguild								},
			{	"getguild",								pc_getguild								},
			{	"isguildmaster",						pc_isguildmaster						},
			{	"is_guild_master",						pc_isguildmaster						},
			{	"destroy_guild",						pc_destroy_guild						},
			{	"remove_from_guild",					pc_remove_from_guild					},

			{	"in_dungeon",							pc_in_dungeon							},

			{	"getempire",							pc_get_empire							},
			{	"get_empire",							pc_get_empire							},

			{	"get_skill_group",						pc_get_skillgroup						},
			{	"set_skill_group",						pc_set_skillgroup						},

			{	"warp",									pc_warp									},
			{	"warp_local",							pc_warp_local							},
			{	"warp_exit",							pc_warp_exit							},
			{	"set_warp_location",					pc_set_warp_location					},
			{	"set_warp_location_local",				pc_set_warp_location_local				},
			{	"get_start_location",					pc_get_start_location},

			{	"has_master_skill",						pc_has_master_skill						},

			{	"set_part",								pc_set_part								},
			{	"get_part",								pc_get_part								},

			{	"is_polymorphed",						pc_is_polymorphed						},
			{	"remove_polymorph",						pc_remove_polymorph						},
			{	"polymorph",							pc_polymorph							},

			{	"is_mount",								pc_is_mount								},
			{	"mount",								pc_mount								},
			{	"mount_bonus",							pc_mount_bonus							},
			{	"unmount",								pc_unmount								},

			{	"warp_to_guild_war_observer_position",	pc_warp_to_guild_war_observer_position	},
			{	"give_item_from_special_item_group",	pc_give_item_from_special_item_group	},
			{	"learn_grand_master_skill",				pc_learn_grand_master_skill				},

			{	"is_skill_book_no_delay",				pc_is_skill_book_no_delay				},
			{	"remove_skill_book_no_delay",			pc_remove_skill_book_no_delay			},

			{	"enough_inventory",						pc_enough_inventory						},
			{	"get_horse_level",						pc_get_horse_level						},
			{	"is_horse_alive",						pc_is_horse_alive						}, 
			{	"revive_horse",							pc_revive_horse							}, 
			{	"have_pos_scroll",						pc_have_pos_scroll						},
			{	"have_map_scroll",						pc_have_map_scroll						},
			{	"get_war_map",							pc_get_war_map							},
			{	"get_equip_refine_level",				pc_get_equip_refine_level				},
			{	"refine_equip",							pc_refine_equip},
			{	"get_skill_level",						pc_get_skill_level						},
			{	"aggregate_monster",					pc_aggregate_monster					},
			{	"forget_my_attacker",					pc_forget_my_attacker					},
			{	"pc_attract_ranger",					pc_attract_ranger						},
			{	"select",								pc_select_vid							},
			{	"get_sex",								pc_get_sex								},
			{	"is_married",							pc_is_married							},
			{	"is_engaged",							pc_is_engaged							},
			{	"is_engaged_or_married",				pc_is_engaged_or_married				},
			{	"is_gm",								pc_is_gm								},
			{	"get_gm_level",							pc_get_gm_level							},
			{	"mining",								pc_mining								},
			{	"ore_refine",							pc_ore_refine							},
			{	"diamond_refine",						pc_diamond_refine						},

			{	"clear_one_skill",						pc_clear_one_skill						},
			{	"clear_skill",							pc_clear_skill							},
			{	"clear_sub_skill",						pc_clear_sub_skill						},
			{	"set_skill_point",						pc_set_skill_point						},
			{	"set_skill_level",						pc_set_skill_level						},
			{	"is_clear_skill_group",					pc_is_clear_skill_group					},

			{	"save_exit_location",					pc_save_exit_location					},
			{	"teleport",								pc_teleport								},

			{	"give_polymorph_book",					pc_give_polymorph_book					},
			{	"upgrade_polymorph_book",				pc_upgrade_polymorph_book				},

			{	"get_premium_remain_sec",				pc_get_premium_remain_sec				},

			{	"send_block_mode",						pc_send_block_mode						},

			{	"change_empire",						pc_change_empire						},
			{	"get_change_empire_count",				pc_get_change_empire_count				},
			{	"set_change_empire_count",				pc_set_change_empire_count				},

			{	"change_name",							pc_change_name							},

			{	"is_dead",								pc_is_dead								},

			{	"reset_status",							pc_reset_status},
			{	"get_ht",								pc_get_ht								},
			{	"set_ht",								pc_set_ht								},
			{	"get_iq",								pc_get_iq								},
			{	"set_iq",								pc_set_iq								},
			{	"get_st",								pc_get_st								},
			{	"set_st",								pc_set_st								},
			{	"get_dx",								pc_get_dx								},
			{	"set_dx",								pc_set_dx								},

			{	"is_near_vid",							pc_is_near_vid							},

			{	"get_socket_items",						pc_get_socket_items						},
			{	"get_empty_inventory_count",			pc_get_empty_inventory_count			},

			{	"get_logoff_interval",					pc_get_logoff_interval					},

			{	"is_riding",							pc_is_riding							},
			{	"get_special_ride_vnum",				pc_get_special_ride_vnum				},

			{	"can_warp",								pc_can_warp								},

			{	"dec_skill_point",						pc_dec_skill_point						},
			{	"get_skill_point",						pc_get_skill_point						},

			{	"get_channel_id",						pc_get_channel_id						},

			{	"give_poly_marble",						pc_give_poly_marble						},
			{	"get_sig_items",						pc_get_sig_items						},

			{	"charge_cash",							pc_charge_cash							},

			{	"get_informer_type",					pc_get_informer_type					},
			{	"get_informer_item",					pc_get_informer_item					},

			{	"give_award",							pc_give_award							},
			{	"give_award_socket",					pc_give_award_socket					},

			{	"get_killee_drop_pct",					pc_get_killee_drop_pct					},

			{	"send_effect",							pc_specific_effect						}, 
			{	"start_duel",							pc_start_duel							},
			{	"stun",									pc_stun_player							},

			{	"reencode_view",						pc_reencode_view						},

			{	"acce_open_absorb",						pc_acce_open_absorb						},
			{	"acce_open_combine",					pc_acce_open_combine					},

			///////////////////////////////////////
			{	"get_mount_vnum",						pc_get_mount_vnum						},
			{	"get_point",							pc_get_point							},
			{	"get_real_point",						pc_get_real_point						},
			{	"disconnect_with_delay",				pc_disconnect_with_delay				},
			{	"get_max_level",						pc_get_max_level						},
			{	"get_ip",								pc_get_ip								},
			{	"kill",									pc_kill									},
			{	"set_coins",							pc_set_coins							},
			{	"get_empire_name",						pc_get_empire_name						},

			{	"set_race",								pc_set_race								},
			{	"del_another_quest_flag",				pc_del_another_quest_flag				},
			{	"pointchange",							pc_pointchange							},
			{	"pullmob",								pc_pullmob								},
			{	"set_level2",							pc_set_level2							},
			{	"set_gm_level",							pc_set_gm_level							},
			{	"if_fire",								pc_if_fire								},
			{	"if_invisible",							pc_if_invisible							},
			{	"if_poison",							pc_if_poison							},

			{	"if_slow",								pc_if_slow								},
			{	"if_stun",								pc_if_stun								},
			{	"sf_fire",								pc_sf_fire								},
			{	"sf_invisible",							pc_sf_invisible							},
			{	"sf_poison",							pc_sf_poison							},
			{	"sf_slow",								pc_sf_slow								},
			{	"sf_stun",								pc_sf_stun								},
			{	"open_shop",							pc_open_shop							},
			{	"sf_kill",								pc_sf_kill								},

			{	"sf_dead",								pc_sf_dead								},
			{	"get_exp_level",						pc_get_exp_level						},
			{	"set_max_health",						pc_set_max_health						},
			{	"dc_delayed",							pc_dc_delayed							},
			{	"dc_direct",							pc_dc_direct							},
			{	"is_trade",								pc_is_trade								},
			{	"is_busy",								pc_is_busy								},
			{	"is_arena",								pc_is_arena								},
			{	"is_arena_observer",					pc_is_arena_observer					},

			{	"equip_slot",							pc_equip_slot							},
			{	"unequip_slot",							pc_unequip_slot							},
			{	"is_available",							pc_is_available							},
			{	"is_pvp",								pc_is_pvp								},
			{	"costume_hide",							pc_costume_hide							},
			{	"set_observer_mode",					pc_set_observer_mode					},
			{	"get_real_x",							pc_get_real_x							},
			{	"get_real_y",							pc_get_real_y							},
			{	"get_quest_data",						pc_get_quest_data						},
				
			{	"find_item_select",						pc_find_item_select						},
			{	"damage",								pc_damage								},
			{	"pickup_item",							pc_pickup_item							},
			{	"get_damage",							pc_get_damage							},
			{	"set_hp",								pc_set_hp								},
			{	"block_exp",							pc_block_exp							},
			{	"unblock_exp",							pc_unblock_exp							},
			{	"if_in_duel",							pc_if_in_duel							},

			{ nullptr,			nullptr			}
		};

		CQuestManager::Instance().AddLuaFunctionTable("pc", pc_functions);
	}
};
