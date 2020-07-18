#include "stdafx.h"
#include <sstream>
#include "constants.h"
#include "char.h"
#include "char_manager.h"
#include "log.h"
#include "quest_manager.h"
#include "questlua.h"
#include "quest_event.h"
#include "config.h"
#include "mining.h"
#include "fishing.h"
#include "priv_manager.h"
#include "utils.h"
#include "p2p.h"
#include "item_manager.h"
#include "mob_manager.h"
#include "start_position.h"
#include "ox_event.h"
#include "regen.h"
#include "cmd.h"
#include "guild.h"
#include "guild_manager.h" 
#include "sectree_manager.h"
#include "db.h"
#include "desc_client.h"
#include "desc_manager.h"

#undef sys_err
#ifndef __WIN32__
#define sys_err(fmt, args...) quest::CQuestManager::Instance().QuestError(__FUNCTION__, __LINE__, fmt, ##args)
#else
#define sys_err(fmt, ...) quest::CQuestManager::Instance().QuestError(__FUNCTION__, __LINE__, fmt, __VA_ARGS__)

#endif

extern ACMD(do_block_chat);

namespace quest
{
	int32_t _get_locale(lua_State* L)
	{
		lua_pushstring(L, g_stLocale.c_str());
		return 1;
	}

	int32_t _number(lua_State* L)
	{
		if (!lua_isnumber(L, 1) || !lua_isnumber(L, 2))
			lua_pushnumber(L, 0);
		else
			lua_pushnumber(L, number((int32_t)lua_tonumber(L, 1), (int32_t)lua_tonumber(L, 2)));
		return 1;
	}

	int32_t _time_to_str(lua_State* L)
	{
		time_t curTime = (time_t)lua_tonumber(L, -1);
		lua_pushstring(L, asctime(gmtime(&curTime)));
		return 1;
	}

	int32_t _say(lua_State* L)
	{
		std::ostringstream s;
		combine_lua_string(L, s);
		CQuestManager::Instance().AddScript(s.str() + "[ENTER]");
		return 0;
	}

	int32_t _flush(lua_State* L)
	{
		CQuestManager::Instance().SendScript();
		return 0;
	}

	int32_t _chat(lua_State* L)
	{
		CHARACTER* ch = CQuestManager::Instance().GetCurrentCharacterPtr();
		if (!ch)
		{
			sys_err("Null character pointer triggered at %s:%d", __FILE__, __LINE__);
			return 0;
		}

		std::ostringstream s;
		combine_lua_string(L, s);

		ch->ChatPacket(CHAT_TYPE_TALKING, "%s", s.str().c_str());
		return 0;
	}

	int32_t _cmdchat(lua_State* L)
	{
		CHARACTER* ch = CQuestManager::Instance().GetCurrentCharacterPtr();
		if (!ch)
		{
			sys_err("Null character pointer triggered at %s:%d", __FILE__, __LINE__);
			return 0;
		}

		std::ostringstream s;
		combine_lua_string(L, s);
		ch->ChatPacket(CHAT_TYPE_COMMAND, "%s", s.str().c_str());
		return 0;
	}

	int32_t _syschat(lua_State* L)
	{
		CHARACTER* ch = CQuestManager::Instance().GetCurrentCharacterPtr();
		if (!ch)
		{
			sys_err("Null character pointer triggered at %s:%d", __FILE__, __LINE__);
			return 0;
		}

		std::ostringstream s;
		combine_lua_string(L, s);
		ch->ChatPacket(CHAT_TYPE_INFO,  "%s", s.str().c_str());
		return 0;
	}

	int32_t _notice(lua_State* L)
	{
		CHARACTER* ch = CQuestManager::Instance().GetCurrentCharacterPtr();
		if (!ch)
		{
			sys_err("Null character pointer triggered at %s:%d", __FILE__, __LINE__);
			return 0;
		}

		std::ostringstream s;
		combine_lua_string(L, s);
		ch->ChatPacket(CHAT_TYPE_NOTICE, "%s", s.str().c_str());
		return 0;
	}

	int32_t _left_image(lua_State* L)
	{
		if (lua_isstring(L, -1))
		{
			std::string s = lua_tostring(L,-1);
			CQuestManager::Instance().AddScript("[LEFTIMAGE src;"+s+"]");
		}
		return 0;
	}

	int32_t _top_image(lua_State* L)
	{
		if (lua_isstring(L, -1))
		{
			std::string s = lua_tostring(L,-1);
			CQuestManager::Instance().AddScript("[TOPIMAGE src;"+s+"]");
		}
		return 0;
	}

	int32_t _set_skin(lua_State* L) // Quest UI style
	{
		if (lua_isnumber(L, -1))
		{
			CQuestManager::Instance().SetSkinStyle((int32_t)rint(lua_tonumber(L,-1)));
		}
		else
		{
			sys_err("QUEST wrong skin index");
		}

		return 0;
	}

	int32_t _set_server_timer(lua_State* L)
	{
		int32_t n = lua_gettop(L);
		if ((n != 2 || !lua_isstring(L, 1) || !lua_isnumber(L, 2)) && 
			(n != 3 || !lua_isstring(L, 1) || !lua_isnumber(L, 2) || !lua_isnumber(L, 3)))
		{
			sys_err("QUEST set_server_timer argument count wrong.");
			return 0;
		}

		const char * name = lua_tostring(L, 1);
		double t = lua_tonumber(L, 2);
		uint32_t arg = 0;

		CQuestManager & q = CQuestManager::Instance();

		if (lua_isnumber(L, 3))
			arg = (uint32_t) lua_tonumber(L, 3);

		int32_t timernpc = q.LoadTimerScript(name);

		LPEVENT event = quest_create_server_timer_event(name, t, timernpc, false, arg);
		q.AddServerTimer(name, arg, event);
		return 0;
	}

	int32_t _set_server_loop_timer(lua_State* L)
	{
		int32_t n = lua_gettop(L);
		if ((n != 2 || !lua_isstring(L, 1) || !lua_isnumber(L, 2)) &&
			(n != 3 || !lua_isstring(L, 1) || !lua_isnumber(L, 2) || !lua_isnumber(L, 3)))
		{
			sys_err("QUEST set_server_timer argument count wrong.");
			return 0;
		}
		const char * name = lua_tostring(L, 1);
		double t = lua_tonumber(L, 2);
		uint32_t arg = 0;
		CQuestManager & q = CQuestManager::Instance();

		if (lua_isnumber(L, 3))
			arg = (uint32_t) lua_tonumber(L, 3);

		int32_t timernpc = q.LoadTimerScript(name);

		LPEVENT event = quest_create_server_timer_event(name, t, timernpc, true, arg);
		q.AddServerTimer(name, arg, event);
		return 0;
	}

	int32_t _clear_server_timer(lua_State* L)
	{
		if (!lua_isstring(L, 1) || !lua_isnumber(L, 2))
		{
			sys_err("QUEST _clear_server_timer argument count wrong.");
			return 0;
		}
		const char * name = lua_tostring(L, 1);
		uint32_t arg = (uint32_t)lua_tonumber(L, 2);

		CQuestManager & q = CQuestManager::Instance();
		q.ClearServerTimer(name, arg);

		return 0;
	}

	int32_t _set_named_loop_timer(lua_State* L)
	{
		CQuestManager & q = CQuestManager::Instance();

		PC* pPC = q.GetCurrentPC();
		if (!pPC)
		{
			sys_err("Null pc pointer triggered at %s:%d", __FILE__, __LINE__);
			return 0;
		}

		CHARACTER* ch = q.GetCurrentCharacterPtr();
		if (!ch)
		{
			sys_err("Null character pointer triggered at %s:%d", __FILE__, __LINE__);
			return 0;
		}

		int32_t n = lua_gettop(L);

		if (n != 2 || !lua_isnumber(L, -1) || !lua_isstring(L, -2))
		{
			sys_err("QUEST _set_named_loop_timer argument count wrong.");
			return 0;
		}	
		double t = lua_tonumber(L, -1);
		const char * name = lua_tostring(L, -2);

		int32_t timernpc = q.LoadTimerScript(name);

		pPC->AddTimer(name, quest_create_timer_event(name, ch->GetPlayerID(), t, timernpc, true));

		return 0;
	}

	int32_t _get_server_timer_arg(lua_State* L)
	{
		lua_pushnumber(L, CQuestManager::Instance().GetServerTimerArg());
		return 1;
	}

	int32_t _set_timer(lua_State* L)
	{
		CHARACTER* ch = CQuestManager::Instance().GetCurrentCharacterPtr();
		if (!ch)
		{
			sys_err("Null character pointer triggered at %s:%d", __FILE__, __LINE__);
			return 0;
		}

		if (lua_gettop(L) != 1 || !lua_isnumber(L, -1))
		{
			sys_err("QUEST invalid argument.");
			return 0;
		}
		double t = lua_tonumber(L, -1);

		CQuestManager& q = CQuestManager::Instance();
		quest_create_timer_event("", ch->GetPlayerID(), t);

		return 0;
	}

	int32_t _set_named_timer(lua_State* L)
	{
		CQuestManager & q = CQuestManager::Instance();

		PC* pPC = q.GetCurrentPC();
		if (!pPC)
		{
			sys_err("Null pc pointer triggered at %s:%d", __FILE__, __LINE__);
			return 0;
		}

		CHARACTER* ch = q.GetCurrentCharacterPtr();
		if (!ch)
		{
			sys_err("Null character pointer triggered at %s:%d", __FILE__, __LINE__);
			return 0;
		}

		int32_t n = lua_gettop(L);

		if (n != 2 || !lua_isnumber(L, -1) || !lua_isstring(L, -2))
		{
			sys_err("QUEST _set_named_timer argument count wrong.");
			return 0;
		}
		double t = lua_tonumber(L, -1);
		const char * name = lua_tostring(L,-2);

		int32_t timernpc = q.LoadTimerScript(name);
		pPC->AddTimer(name, quest_create_timer_event(name, ch->GetPlayerID(), t, timernpc));

		return 0;
	}

	int32_t _timer(lua_State * L)
	{
		if (lua_gettop(L) == 1)
			return _set_timer(L);
		
		return _set_named_timer(L);
	}

	int32_t _clear_named_timer(lua_State* L)
	{
		PC* pPC = CQuestManager::Instance().GetCurrentPC();
		if (!pPC)
		{
			sys_err("Null pc pointer triggered at %s:%d", __FILE__, __LINE__);
			return 0;
		}

		int32_t n = lua_gettop(L);

		if (n != 1 || !lua_isstring(L, -1))
		{
			sys_err("QUEST _clear_named_timer argument count wrong.");
			return 0;
		}
		
		pPC->RemoveTimer(lua_tostring(L, -1));
		return 0;
	}

	int32_t _getnpcid(lua_State * L)
	{
		int32_t n = lua_gettop(L);

		if (n != 1 || !lua_isstring(L, -1))
		{
			sys_err("QUEST _getnpcid argument count wrong.");
			return 0;
		}
		const char * name = lua_tostring(L, -1);
		CQuestManager & q = CQuestManager::Instance();
		lua_pushnumber(L, q.FindNPCIDByName(name));
		return 1;
	}

	int32_t _is_test_server(lua_State * L)
	{
		lua_pushboolean(L, g_bIsTestServer);
		return 1;
	}

	int32_t _raw_script(lua_State* L)
	{
		if ( g_bIsTestServer )
			sys_log ( 0, "_raw_script : %s ", lua_tostring(L,-1));
		if (lua_isstring(L, -1))
			CQuestManager::Instance().AddScript(lua_tostring(L,-1));
		else
			sys_err("QUEST wrong argument: questname: %s", CQuestManager::Instance().GetCurrentQuestName().c_str());

		return 0;
	}

	int32_t _char_log(lua_State * L)
	{
		CHARACTER* ch = CQuestManager::Instance().GetCurrentCharacterPtr();
		if (!ch)
		{
			sys_err("Null character pointer triggered at %s:%d", __FILE__, __LINE__);
			return 0;
		}

		uint32_t what = 0;
		const char* how = "";
		const char* hint = "";

		if (lua_isnumber(L, 1)) what = (uint32_t)lua_tonumber(L, 1);
		if (lua_isstring(L, 2)) how = lua_tostring(L, 2);
		if (lua_tostring(L, 3)) hint = lua_tostring(L, 3);

		LogManager::Instance().CharLog(ch, what, how, hint);
		return 0;
	}
	
	int32_t _item_log(lua_State* L)
	{
		CHARACTER* ch = CQuestManager::Instance().GetCurrentCharacterPtr();
		if (!ch)
		{
			sys_err("Null character pointer triggered at %s:%d", __FILE__, __LINE__);
			return 0;
		}

		uint32_t dwItemID = 0;
		const char* how = "";
		const char* hint = "";

		if ( lua_isnumber(L, 1) ) dwItemID = (uint32_t)lua_tonumber(L, 1);
		if ( lua_isstring(L, 2) ) how = lua_tostring(L, 2);
		if ( lua_tostring(L, 3) ) hint = lua_tostring(L, 3);

		CItem * item = ITEM_MANAGER::Instance().Find(dwItemID);
		if (item)
			LogManager::Instance().ItemLog(ch, item, how, hint);

		return 0;
	}

	int32_t _syslog(lua_State* L)
	{
		if (!lua_isnumber(L, 1) || !lua_isstring(L, 2))
			return 0;

		if (lua_tonumber(L, 1) >= 1)
		{
			if (!g_bIsTestServer)
				return 0;
		}

		PC* pc = CQuestManager::Instance().GetCurrentPC();

		if (!pc)
			return 0;

		LPCHARACTER ch = CQuestManager::Instance().GetCurrentCharacterPtr();

		if (!ch)
			return 0;

		sys_log(0, "QUEST: quest: %s player: %s : %s", pc->GetCurrentQuestName().c_str(), ch->GetName(), lua_tostring(L, 2));

		if (g_bIsTestServer)
			ch->ChatPacket(CHAT_TYPE_INFO,  "QUEST_SYSLOG %s", lua_tostring(L, 2));

		return 0;
	}

	int32_t _syserr(lua_State* L)
	{
		if (!lua_isstring(L, 1))
			return 0;

		PC* pc = CQuestManager::Instance().GetCurrentPC();

		if (!pc)
			return 0;

		LPCHARACTER ch = CQuestManager::Instance().GetCurrentCharacterPtr();

		if (!ch)
			return 0;

		sys_err("QUEST: quest: %s player: %s : %s", pc->GetCurrentQuestName().c_str(), ch->GetName(), lua_tostring(L, 1));
		ch->ChatPacket(CHAT_TYPE_INFO, "QUEST_SYSERR %s", lua_tostring(L, 1));
		return 0;
	}
	
	// LUA_ADD_BGM_INFO
	int32_t _set_bgm_volume_enable(lua_State* L)
	{
		CHARACTER_SetBGMVolumeEnable();

		return 0;
	}

	int32_t _add_bgm_info(lua_State* L)
	{
		if (!lua_isnumber(L, 1) || !lua_isstring(L, 2))
			return 0;

		int32_t mapIndex		= (int32_t)lua_tonumber(L, 1);

		const char*	bgmName	= lua_tostring(L, 2);
		if (!bgmName)
			return 0;

		float bgmVol = lua_isnumber(L, 3) ? static_cast<float>(lua_tonumber(L, 3)) : (1/5.0f)*0.1f;

		CHARACTER_AddBGMInfo(mapIndex, bgmName, bgmVol);

		return 0;
	}
	// END_OF_LUA_ADD_BGM_INFO	

	// LUA_ADD_GOTO_INFO
	int32_t _add_goto_info(lua_State* L)
	{
		if (!lua_isstring(L, 1) || !lua_isnumber(L, 2) || !lua_isnumber(L, 3))
		{
			return 0;
		}
		const char* name	= lua_tostring(L, 1);
		int32_t empire 	= (int32_t)lua_tonumber(L, 2);
		int32_t mapIndex 	= (int32_t)lua_tonumber(L, 3);
		int32_t x 		= (int32_t)lua_tonumber(L, 4);
		int32_t y 		= (int32_t)lua_tonumber(L, 5);

		if (!name)
			return 0;

		CHARACTER_AddGotoInfo(name, empire, mapIndex, x, y);
		return 0;
	}
	// END_OF_LUA_ADD_GOTO_INFO

	// REFINE_PICK
	int32_t _refine_pick(lua_State* L)
	{
		if (!lua_isnumber(L, 1))
		{
			return 0;
		}
		uint16_t bCell = (uint16_t) lua_tonumber(L,-1);

		LPCHARACTER ch = CQuestManager::Instance().GetCurrentCharacterPtr();
		if (!ch)
		{
			return 0;
		}

		CItem * item = ch->GetInventoryItem(bCell);
		if (!item)
		{
			return 0;
		}

		int32_t ret = mining::RealRefinePick(ch, item);

		lua_pushnumber(L, ret);
		return 1;
	}
	// END_OF_REFINE_PICK

	int32_t _fish_real_refine_rod(lua_State* L)
	{
		if (!lua_isnumber(L, 1))
		{
			return 0;
		}
		uint16_t bCell = (uint16_t) lua_tonumber(L,-1);

		LPCHARACTER ch = CQuestManager::Instance().GetCurrentCharacterPtr();
		if (!ch)
		{
			return 0;
		}

		CItem * item = ch->GetInventoryItem(bCell);
		if (!item)
		{
			return 0;
		}

		int32_t ret = fishing::RealRefineRod(ch, item);
		lua_pushnumber(L, ret);
		return 1;
	}

	int32_t _give_char_privilege(lua_State* L)
	{
		if (!lua_isnumber(L, 1) || !lua_isnumber(L, 2))
		{
			return 0;
		}
		int32_t type = (int32_t)lua_tonumber(L, 1);
		int32_t value = (int32_t)lua_tonumber(L, 2);

		LPCHARACTER ch = CQuestManager::Instance().GetCurrentCharacterPtr();
		if (!ch)
		{
			return 0;
		}
		
		int32_t pid = ch->GetPlayerID();

		if (MAX_PRIV_NUM <= type)
		{
			sys_err("PRIV_MANAGER: _give_char_privilege: wrong empire priv type(%u)", type);
			return 0;
		}

		CPrivManager::Instance().RequestGiveCharacterPriv(pid, type, value);

		return 0;
	}

	int32_t _give_empire_privilege(lua_State* L)
	{
		if (!lua_isnumber(L, 1) || !lua_isnumber(L, 2) || !lua_isnumber(L, 3) || !lua_isnumber(L, 4))
		{
			return 0;
		}
		int32_t empire = (int32_t)lua_tonumber(L,1);
		int32_t type = (int32_t)lua_tonumber(L, 2);
		int32_t value = (int32_t)lua_tonumber(L, 3);
		int32_t time = (int32_t) lua_tonumber(L,4);

		LPCHARACTER ch = CQuestManager::Instance().GetCurrentCharacterPtr();
		if (!ch)
		{
			return 0;
		}

		if (MAX_PRIV_NUM <= type)
		{
			sys_err("PRIV_MANAGER: _give_empire_privilege: wrong empire priv type(%u)", type);
			return 0;
		}

		sys_log(0, "_give_empire_privileage(empire=%d, type=%d, value=%d, time=%d), by quest, %s", empire, type, value, time, ch->GetName());

		CPrivManager::Instance().RequestGiveEmpirePriv(empire, type, value, time);
		return 0;
	}

	int32_t _give_guild_privilege(lua_State* L)
	{
		if (!lua_isnumber(L, 1) || !lua_isnumber(L, 2) || !lua_isnumber(L, 3) || !lua_isnumber(L, 4))
		{
			return 0;
		}
		int32_t guild_id = (int32_t)lua_tonumber(L,1);
		int32_t type = (int32_t)lua_tonumber(L, 2);
		int32_t value = (int32_t)lua_tonumber(L, 3);
		int32_t time = (int32_t)lua_tonumber( L, 4 );

		if (MAX_PRIV_NUM <= type)
		{
			sys_err("PRIV_MANAGER: _give_guild_privilege: wrong empire priv type(%u)", type);
			return 0;
		}

		sys_log(0, "_give_guild_privileage(empire=%d, type=%d, value=%d, time=%d)", guild_id, type, value, time);

		CPrivManager::Instance().RequestGiveGuildPriv(guild_id,type,value,time);

		return 0;
	}

	int32_t _get_empire_privilege_string(lua_State* L)
	{
		if (!lua_isnumber(L, 1))
		{
			return 0;
		}
		int32_t empire = (int32_t) lua_tonumber(L, 1);

		std::ostringstream os;
		bool found = false;

		for (int32_t type = PRIV_NONE + 1; type < MAX_PRIV_NUM; ++type)
		{
			auto pkPrivEmpireData = CPrivManager::Instance().GetPrivByEmpireEx(empire, type);

			if (pkPrivEmpireData && pkPrivEmpireData->m_value)
			{
				if (found)
					os << ", ";

				os << LC_TEXT(c_apszPrivNames[type]) << " : " << 
					pkPrivEmpireData->m_value << "%" << " (" <<
					((pkPrivEmpireData->m_end_time_sec-get_global_time())/3600.0f) << " hours)" << '\n';
				found = true;
			}
		}

		if (!found)
			os << "None!" << '\n';

		lua_pushstring(L, os.str().c_str());
		return 1;
	}

	int32_t _get_empire_privilege(lua_State* L)
	{
		if (!lua_isnumber(L,1) || !lua_isnumber(L,2))
		{
			lua_pushnumber(L,0);
			return 1;
		}
		int32_t empire = (int32_t)lua_tonumber(L,1);
		int32_t type = (int32_t)lua_tonumber(L,2);
		int32_t value = CPrivManager::Instance().GetPrivByEmpire(empire,type);
		lua_pushnumber(L, value);
		return 1;
	}

	int32_t _get_guild_privilege_string(lua_State* L)
	{
		if (!lua_isnumber(L, 1))
			return 0;

		int32_t guild = (int32_t) lua_tonumber(L,1);
		std::ostringstream os;
		bool found = false;

		for (int32_t type = PRIV_NONE+1; type < MAX_PRIV_NUM; ++type)
		{
			auto pPrivGuildData = CPrivManager::Instance().GetPrivByGuildEx( guild, type );

			if (pPrivGuildData && pPrivGuildData->value)
			{
				if (found)
					os << ", ";

				os << LC_TEXT(c_apszPrivNames[type]) << " : " << pPrivGuildData->value << "%"
					<< " (" << ((pPrivGuildData->end_time_sec - get_global_time()) / 3600.0f) << " hours)" <<  '\n';

				found = true;
			}
		}

		if (!found)
			os << "None!" <<  '\n';

		lua_pushstring(L, os.str().c_str());
		return 1;
	}

	int32_t _get_guildid_byname( lua_State* L )
	{
		if ( !lua_isstring( L, 1 ) ) 
		{
			sys_err( "_get_guildid_byname() - invalud argument" );

			lua_pushnumber( L, 0 );
			return 1;
		}
		const char* pszGuildName = lua_tostring( L, 1 );
		CGuild* pFindGuild = CGuildManager::Instance().FindGuildByName( pszGuildName );
		if ( pFindGuild )
			lua_pushnumber( L, pFindGuild->GetID() );
		else
			lua_pushnumber( L, 0 );

		return 1;
	}

	int32_t _get_guild_privilege(lua_State* L)
	{
		if (!lua_isnumber(L,1) || !lua_isnumber(L,2))
		{
			lua_pushnumber(L,0);
			return 1;
		}
		int32_t guild = (int32_t)lua_tonumber(L,1);
		int32_t type = (int32_t)lua_tonumber(L,2);
		int32_t value = CPrivManager::Instance().GetPrivByGuild(guild,type);
		lua_pushnumber(L, value);
		return 1;
	}

	int32_t _item_name(lua_State* L)
	{
		if (!lua_isnumber(L, 1))
		{
			lua_pushstring(L, "");
			return 1;
		}
		uint32_t dwVnum = (uint32_t)lua_tonumber(L,1);

		SItemTable_Server* pTable = ITEM_MANAGER::Instance().GetTable(dwVnum);
		if (pTable)
			lua_pushstring(L, pTable->szLocaleName);
		else
			lua_pushstring(L, "");

		return 1;
	}

	int32_t _mob_name(lua_State* L)
	{
		if (!lua_isnumber(L, 1))
		{
			lua_pushstring(L, "");
			return 1;
		}
		uint32_t dwVnum = (uint32_t) lua_tonumber(L, 1);

		const CMob * pkMob = CMobManager::Instance().Get(dwVnum);
		if (pkMob)
			lua_pushstring(L, pkMob->m_table.szLocaleName);
		else
			lua_pushstring(L, "");

		return 1;
	}

	int32_t _mob_vnum(lua_State* L)
	{
		if (lua_isstring(L,1))
		{
			lua_pushnumber(L, 0);
			return 1;
		}	
		const char* str = lua_tostring(L, 1);

		const CMob* pkMob = CMobManager::Instance().Get(str, false);
		if (pkMob)
			lua_pushnumber(L, pkMob->m_table.dwVnum);
		else
			lua_pushnumber(L, 0);

		return 1;
	}

	int32_t _get_global_time(lua_State* L)
	{
		lua_pushnumber(L, get_global_time());
		return 1;
	}
	

	int32_t _get_channel_id(lua_State* L)
	{
		lua_pushnumber(L, g_bChannel);

		return 1;
	}

	int32_t _do_command(lua_State* L)
	{
		if (!lua_isstring(L, 1))
			return 0;

		const char * str = lua_tostring(L, 1);
		size_t len = strlen(str);

		LPCHARACTER ch = CQuestManager::Instance().GetCurrentCharacterPtr();
		::interpret_command(ch, str, len);
		return 0;
	}

	int32_t _find_pc(lua_State* L)
	{
		if (!lua_isstring(L, 1))
		{
			sys_err("invalid argument");
			lua_pushnumber(L, 0);
			return 1;
		}

		const char * name = lua_tostring(L, 1);
		LPCHARACTER tch = CHARACTER_MANAGER::Instance().FindPC(name);
		lua_pushnumber(L, tch ? tch->GetVID() : 0);
		return 1;
	}

	int32_t _find_pc_cond(lua_State* L)
	{
		if (!lua_isnumber(L, 1) || !lua_isnumber(L, 2) || !lua_isnumber(L, 3))
		{
			sys_err("invalid argument");
			lua_pushnumber(L, 0);
			return 1;
		}

		int32_t iMinLev = (int32_t) lua_tonumber(L, 1);
		int32_t iMaxLev = (int32_t) lua_tonumber(L, 2);
		uint32_t uiJobFlag = (uint32_t) lua_tonumber(L, 3);

		LPCHARACTER ch = CQuestManager::Instance().GetCurrentCharacterPtr();
		if (!ch)
		{
			sys_err("null ch ptr");
			lua_pushnumber(L, 0);
			return 1;
		}

		if (g_bIsTestServer)
		{
			sys_log(0, "find_pc_cond map=%d, job=%d, level=%d~%d", ch->GetMapIndex(), uiJobFlag, iMinLev, iMaxLev);
		}

		auto tch = CHARACTER_MANAGER::Instance().FindSpecifyPC(uiJobFlag, ch->GetMapIndex(), ch, iMinLev, iMaxLev);

		lua_pushnumber(L, tch ? tch->GetVID() : 0);
		return 1;
	}

	int32_t _find_npc_by_vnum(lua_State* L)
	{
		if (!lua_isnumber(L, 1))
		{
			sys_err("invalid argument");

			lua_pushnumber(L, 0);
			return 1;
		}
		auto race = static_cast<uint32_t>(lua_tonumber(L, 1));

		const auto mapIndex = CQuestManager::Instance().GetCurrentCharacterPtr()->GetMapIndex();

		const auto chars = CHARACTER_MANAGER::Instance().GetCharactersByRaceNum(race);
		for (auto ch : chars) 
		{
			if (ch && ch->GetMapIndex() == mapIndex) 
			{
				lua_pushnumber(L, ch->GetVID());
				return 1;
			}
		}

		lua_pushnumber(L, 0);
		return 1;
	}

	// 새로운 state를 만든다.
	int32_t _set_quest_state(lua_State* L)
	{
		if (!lua_isstring(L, 1) || !lua_isstring(L, 2))
			return 0;

		CQuestManager& q = CQuestManager::Instance();
		QuestState * pqs = q.GetCurrentState();

		if (!pqs || L != pqs->co)
		{
			luaL_error(L, "running thread != current thread???");
			sys_err("running thread != current thread???");
			return 0;
		}

		PC* pPC = q.GetCurrentPC();
		if (pPC)
		{
			const std::string stQuestName(lua_tostring(L, 1));
			const std::string stStateName(lua_tostring(L, 2));

			if ( g_bIsTestServer )
				sys_log(0,"set_state %s %s ", stQuestName.c_str(), stStateName.c_str() );
			if (pPC->GetCurrentQuestName() == stQuestName)
			{
				pqs->st = q.GetQuestStateIndex(pPC->GetCurrentQuestName(), lua_tostring(L, -1));
				pPC->SetCurrentQuestStateName(lua_tostring(L,-1));
			}
			else
			{
				pPC->SetQuestState(stQuestName, stStateName);
			}
		}
		return 0;
	}

	int32_t _get_quest_state(lua_State* L)
	{
		if (!lua_isstring(L, 1) )
			return 0;

		CQuestManager& q = CQuestManager::Instance();
		PC* pPC = q.GetCurrentPC();
		
		if (pPC)
		{
			std::string stQuestName	= lua_tostring(L, 1);
			stQuestName += ".__status";

			int32_t nRet = pPC->GetFlag( stQuestName.c_str() ); 

			lua_pushnumber(L, nRet );

			if ( g_bIsTestServer )
				sys_log(0,"Get_quest_state name %s value %d", stQuestName.c_str(), nRet );
		}
		else
		{
			if ( g_bIsTestServer )
				sys_log(0,"PC == 0 ");

			lua_pushnumber(L, 0);
		}
		return 1;
	}

	int32_t _notice_all( lua_State* L )
	{
		std::ostringstream s;
		combine_lua_string(L, s);

		TPacketGGNotice p;
		p.bHeader = HEADER_GG_NOTICE;
		p.lSize = s.str().length() + 1;

		TEMP_BUFFER buf;
		buf.write(&p, sizeof(p));
		buf.write(s.str().c_str(), p.lSize);

		P2P_MANAGER::Instance().Send(buf.read_peek(), buf.size()); // HEADER_GG_NOTICE

		SendNotice(s.str().c_str());
		return 1;	
	}

	EVENTINFO(warp_all_to_village_event_info)
	{
		uint32_t dwWarpMapIndex;

		warp_all_to_village_event_info() : 
			dwWarpMapIndex( 0 )
		{
		}
	};

	struct FWarpAllToVillage
	{
		FWarpAllToVillage() = default;

		void operator()(LPENTITY ent)
		{
			if (ent->IsType(ENTITY_CHARACTER))
			{
				LPCHARACTER ch = (CHARACTER *) ent;
				if (ch && ch->IsPC())
				{
					uint8_t bEmpire =  ch->GetEmpire();
					if ( bEmpire == 0 )
					{
						sys_err( "Unkonwn Empire %s %d ", ch->GetName(), ch->GetPlayerID() );
						return;
					}

					ch->WarpSet( g_start_position[bEmpire][0], g_start_position[bEmpire][1] );
				}
			}
		}
	};

	EVENTFUNC(warp_all_to_village_event)
	{
		warp_all_to_village_event_info * info = dynamic_cast<warp_all_to_village_event_info *>(event->info);

		if ( info == nullptr )
		{
			sys_err( "warp_all_to_village_event> <Factor> Null pointer" );
			return 0;
		}

		LPSECTREE_MAP pSecMap = SECTREE_MANAGER::Instance().GetMap( info->dwWarpMapIndex );

		if (pSecMap)
		{
			FWarpAllToVillage f;
			pSecMap->for_each( f );
		}

		return 0;
	}

	int32_t _warp_all_to_village( lua_State * L )
	{
		if (!lua_isnumber(L, 1) || !lua_isnumber(L, 2))
			return 0;

		int32_t iMapIndex 	= static_cast<int32_t>(lua_tonumber(L, 1));
		int32_t iSec		= static_cast<int32_t>(lua_tonumber(L, 2));
		
		warp_all_to_village_event_info* info = AllocEventInfo<warp_all_to_village_event_info>();

		info->dwWarpMapIndex = iMapIndex;

		event_create(warp_all_to_village_event, info, PASSES_PER_SEC(iSec));

		SendNoticeMap(LC_TEXT("잠시후 모두 마을로 이동됩니다."), iMapIndex, false);

		return 0;
	}

	int32_t _warp_to_village( lua_State * L )
	{
		LPCHARACTER ch = CQuestManager::Instance().GetCurrentCharacterPtr();
	
		if (ch)
		{
			uint8_t bEmpire = ch->GetEmpire();	
			ch->WarpSet( g_start_position[bEmpire][0], g_start_position[bEmpire][1] );
		}

		return 0;
	}

	int32_t _say_in_map( lua_State * L )
	{
		if (!lua_isnumber(L, 1) || !lua_isstring(L, 2))
			return 0;

		int32_t iMapIndex 		= static_cast<int32_t>(lua_tonumber( L, 1 ));
		std::string Script(lua_tostring( L, 2 ));

		Script += "[ENTER]";
		Script += "[DONE]";
		
		SPacketGCScript packet_script;
		packet_script.skin = CQuestManager::QUEST_SKIN_NORMAL;
		packet_script.src_size = static_cast<uint16_t>(Script.size());
		packet_script.size = packet_script.src_size + sizeof(SPacketGCScript);

		FSendPacket f;
		f.buf.write(&packet_script, sizeof(SPacketGCScript));
		f.buf.write(&Script[0], Script.size());

		LPSECTREE_MAP pSecMap = SECTREE_MANAGER::Instance().GetMap( iMapIndex );

		if ( pSecMap )
		{
			pSecMap->for_each( f );
		}

		return 0;
	}

	struct FKillSectree2
	{
		void operator () (LPENTITY ent)
		{
			if (ent->IsType(ENTITY_CHARACTER))
			{
				LPCHARACTER ch = (LPCHARACTER) ent;

				if (ch && !ch->IsPC() && !ch->IsPet())
					ch->Dead();
			}
		}
	};

	int32_t _kill_all_in_map ( lua_State * L )
	{
		if (!lua_isnumber(L, 1))
			return 0;

		LPSECTREE_MAP pSecMap = SECTREE_MANAGER::Instance().GetMap( (int32_t)lua_tonumber(L, 1) );
		if (pSecMap)
		{
			FKillSectree2 f;
			pSecMap->for_each( f );
		}

		return 0;
	}

	//주의: 몹 리젠이 안되는 맵에서만 사용
	int32_t _regen_in_map( lua_State * L )
	{
		if (!lua_isnumber(L, 1) || !lua_isstring(L, 2))
			return 0;

		int32_t iMapIndex = static_cast<int32_t>(lua_tonumber(L, 1));
		std::string szFilename(lua_tostring(L, 2));

		LPSECTREE_MAP pkMap = SECTREE_MANAGER::Instance().GetMap(iMapIndex);
		if (pkMap)
		{
			regen_load_in_file( szFilename.c_str(), iMapIndex, pkMap->m_setting.iBaseX ,pkMap->m_setting.iBaseY );
		}

		return 0;
	}

	int32_t _add_ox_quiz(lua_State* L)
	{
		if (!lua_isnumber(L, 1) || !lua_isstring(L, 2) || !lua_isboolean(L, 3))
		{
			lua_pushnumber(L, 0);
			return 1;
		}
		int32_t level = (int32_t)lua_tonumber(L, 1);
		const char* quiz = lua_tostring(L, 2);
		bool answer = lua_toboolean(L, 3);

		if (!COXEventManager::Instance().AddQuiz(level, quiz, answer))
		{
			sys_log(0, "OXEVENT : Cannot add quiz. %d %s %d", level, quiz, answer);
		}

		lua_pushnumber(L, 1);
		return 1;
	}

	EVENTFUNC(warp_all_to_map_my_empire_event)
	{
		warp_all_to_map_my_empire_event_info * info = dynamic_cast<warp_all_to_map_my_empire_event_info *>(event->info);

		if ( info == nullptr )
		{
			sys_err( "warp_all_to_map_my_empire_event> <Factor> Null pointer" );
			return 0;
		}

		LPSECTREE_MAP pSecMap = SECTREE_MANAGER::Instance().GetMap( info->m_lMapIndexFrom );

		if (pSecMap)
		{
			FWarpEmpire f;

			f.m_lMapIndexTo = info->m_lMapIndexTo;
			f.m_x			= info->m_x;
			f.m_y			= info->m_y;
			f.m_bEmpire		= info->m_bEmpire;

			pSecMap->for_each(f);
		}

		return 0;
	}

	int32_t _block_chat(lua_State* L)
	{
		LPCHARACTER pChar = CQuestManager::Instance().GetCurrentCharacterPtr();
		if (pChar)
		{
			if (!lua_isstring(L, 1) || !lua_isstring(L, 2))
			{
				lua_pushboolean(L, false);
				return 1;
			}

			std::string strName(lua_tostring(L, 1));
			std::string strTime(lua_tostring(L, 2));

			std::string strArg = strName + " " + strTime;

			do_block_chat(pChar, const_cast<char*>(strArg.c_str()), 0, 0);

			lua_pushboolean(L, true);
			return 1;
		}

		lua_pushboolean(L, false);
		return 1;
	}

	int32_t _spawn_mob(lua_State* L)
	{
		if (!lua_isnumber(L, 1) || !lua_isnumber(L, 2) || !lua_isboolean(L, 3))
		{
			lua_pushnumber(L, 0);
			return 1;
		}

		auto dwVnum		 = static_cast<uint32_t>(lua_tonumber(L, 1));
		auto count		 = MINMAX(1, static_cast<int32_t>(lua_tonumber(L, 2)), 10);
		auto isAggresive = static_cast<bool>(lua_toboolean(L, 3));
	
		size_t SpawnCount = 0;

		auto pMonster = CMobManager::Instance().Get(dwVnum);
		if (pMonster)
		{
			auto pChar = CQuestManager::Instance().GetCurrentCharacterPtr();

			for (int32_t i = 0; i < count; ++i)
			{
				auto pSpawnMonster = CHARACTER_MANAGER::Instance().SpawnMobRange(
					dwVnum,
					pChar->GetMapIndex(),
					pChar->GetX() - number(200, 750),
					pChar->GetY() - number(200, 750),
					pChar->GetX() + number(200, 750),
					pChar->GetY() + number(200, 750),
					true,
					pMonster->m_table.bType == CHAR_TYPE_STONE,
					isAggresive
				);

				if (pSpawnMonster)
				{
					++SpawnCount;
				}
			}

			sys_log(0, "QUEST Spawn Monstster: VNUM(%u) COUNT(%u) isAggresive(%b)", dwVnum, SpawnCount, isAggresive);
		}

		lua_pushnumber(L, SpawnCount);

		return 1;
	}

	int32_t _notice_in_map( lua_State* L )
	{
		if (!lua_isstring(L, 1) || !lua_isboolean(L, 2))
		{
			sys_err("wrong arg");
			return 0;
		}

		LPCHARACTER pChar = CQuestManager::Instance().GetCurrentCharacterPtr();
		if (pChar)
		{
			SendNoticeMap( lua_tostring(L,1), pChar->GetMapIndex(), lua_toboolean(L,2) );
		}

		return 0;
	}

	int32_t _get_locale_base_path( lua_State* L )
	{
		lua_pushstring( L, LocaleService_GetBasePath().c_str() );

		return 1;
	}

	struct FQuestFlagArea
	{
		int32_t x1, y1, x2, y2;
		const char* sz;
		int32_t val;

		FQuestFlagArea(int32_t a, int32_t b, int32_t c, int32_t d, const char* p, int32_t value) : 
			x1(a), y1(b), x2(c), y2(d), sz(p), val(value)
		{
		}

		void operator () (LPENTITY ent)
		{
			if (ent && ent->IsType(ENTITY_CHARACTER))
			{
				LPCHARACTER pChar = static_cast<CHARACTER *>(ent);

				if (pChar->IsPC())
				{
					if (x1 <= pChar->GetX() && pChar->GetX() <= x2 && y1 <= pChar->GetY() && pChar->GetY() <= y2)
					{
						CQuestManager& q = CQuestManager::Instance();
						PC* pPC = q.GetPC(pChar->GetPlayerID());
						if (pPC)
						{
							pPC->SetFlag(sz, val);
						}
					}
				}
			}
		}
	};


	int32_t _set_quest_flag_in_area(lua_State* L)
	{
		if (!lua_isstring(L, 1))
		{
			sys_err("wrong arg");
			return 0;
		}

		const char* sz = (const char*)lua_tostring(L, 1);
		int32_t value = (int32_t)lua_tonumber(L, 2);
		int32_t x1 = (int32_t)lua_tonumber(L, 3);
		int32_t y1 = (int32_t)lua_tonumber(L, 4);
		int32_t x2 = (int32_t)lua_tonumber(L, 5);
		int32_t y2 = (int32_t)lua_tonumber(L, 6);

		const int32_t mapIndex = SECTREE_MANAGER::Instance().GetMapIndex(x1, y1);
		if (!mapIndex)
		{
			sys_err("_purge_area: cannot get a map index with (%u, %u)", x1, y1);
			return 0;
		}

		LPSECTREE_MAP pSectree = SECTREE_MANAGER::Instance().GetMap(mapIndex);
		if (pSectree)
		{
			FQuestFlagArea func(x1, y1, x2, y2, sz, value);
			pSectree->for_each(func);
		}

		return 0;
	}
	struct FPurgeArea
	{
		int32_t x1, y1, x2, y2;
		LPCHARACTER ExceptChar;

		FPurgeArea(int32_t a, int32_t b, int32_t c, int32_t d, LPCHARACTER p) :
			x1(a), y1(b), x2(c), y2(d),
			ExceptChar(p)
		{
		}

		void operator () (LPENTITY ent)
		{
			if (ent && ent->IsType(ENTITY_CHARACTER))
			{
				LPCHARACTER pChar = static_cast<CHARACTER *>(ent);

				if (pChar == ExceptChar)
					return;
					
				if (!pChar->IsPet() && (pChar->IsMonster() || pChar->IsStone()))
				{
					if (x1 <= pChar->GetX() && pChar->GetX() <= x2 && y1 <= pChar->GetY() && pChar->GetY() <= y2)
					{
						M2_DESTROY_CHARACTER(pChar);
					}
				}
			}
		}
	};

	int32_t _purge_area( lua_State* L )
	{
		int32_t x1 = (int32_t)lua_tonumber(L, 1);
		int32_t y1 = (int32_t)lua_tonumber(L, 2);
		int32_t x2 = (int32_t)lua_tonumber(L, 3);
		int32_t y2 = (int32_t)lua_tonumber(L, 4);

		const int32_t mapIndex = SECTREE_MANAGER::Instance().GetMapIndex( x1, y1 );
		if (!mapIndex)
		{
			sys_err("_purge_area: cannot get a map index with (%u, %u)", x1, y1);
			return 0;
		}

		LPSECTREE_MAP pSectree = SECTREE_MANAGER::Instance().GetMap(mapIndex);
		if (pSectree)
		{
			FPurgeArea func(x1, y1, x2, y2, CQuestManager::Instance().GetCurrentNPCCharacterPtr());
			pSectree->for_each(func);
		}

		return 0;
	}

	struct FWarpAllInAreaToArea
	{
		int32_t from_x1, from_y1, from_x2, from_y2;
		int32_t to_x1, to_y1, to_x2, to_y2;
		size_t warpCount;

		FWarpAllInAreaToArea(int32_t a, int32_t b, int32_t c, int32_t d, int32_t e, int32_t f, int32_t g, int32_t h) : 
			from_x1(a), from_y1(b), from_x2(c), from_y2(d),
			to_x1(e), to_y1(f), to_x2(g), to_y2(h),
			warpCount(0)
		{
		}

		void operator () (LPENTITY ent)
		{
			if (ent && ent->IsType(ENTITY_CHARACTER))
			{
				LPCHARACTER pChar = static_cast<CHARACTER *>(ent);
				if (pChar->IsPC())
				{
					if (from_x1 <= pChar->GetX() && pChar->GetX() <= from_x2 && from_y1 <= pChar->GetY() && pChar->GetY() <= from_y2)
					{
						++warpCount;

						pChar->WarpSet( number(to_x1, to_x2), number(to_y1, to_y2) );
					}
				}
			}
		}
	};

	int32_t _warp_all_in_area_to_area( lua_State* L )
	{
		int32_t from_x1 = (int32_t)lua_tonumber(L, 1);
		int32_t from_y1 = (int32_t)lua_tonumber(L, 2);
		int32_t from_x2 = (int32_t)lua_tonumber(L, 3);
		int32_t from_y2 = (int32_t)lua_tonumber(L, 4);

		int32_t to_x1 = (int32_t)lua_tonumber(L, 5);
		int32_t to_y1 = (int32_t)lua_tonumber(L, 6);
		int32_t to_x2 = (int32_t)lua_tonumber(L, 7);
		int32_t to_y2 = (int32_t)lua_tonumber(L, 8);

		const int32_t mapIndex = SECTREE_MANAGER::Instance().GetMapIndex(from_x1, from_y1);
		if (!mapIndex)
		{
			sys_err("_warp_all_in_area_to_area: cannot get a map index with (%u, %u)", from_x1, from_y1);

			lua_pushnumber(L, 0);
			return 1;
		}

		LPSECTREE_MAP pSectree = SECTREE_MANAGER::Instance().GetMap(mapIndex);
		if (pSectree)
		{
			FWarpAllInAreaToArea func(from_x1, from_y1, from_x2, from_y2, to_x1, to_y1, to_x2, to_y2);
			pSectree->for_each(func);

			sys_log(0, "_warp_all_in_area_to_area: %u character warp", func.warpCount);

			lua_pushnumber(L, func.warpCount);
			return 1;
		}

		sys_err("_warp_all_in_area_to_area: no sectree");
		
		lua_pushnumber(L, 0);
		return 1;
	}

	int32_t _get_special_item_group( lua_State* L )
	{
		if (!lua_isnumber (L, 1))
		{
			sys_err("invalid argument");
			return 0;
		}

		const CSpecialItemGroup* pItemGroup = ITEM_MANAGER::Instance().GetSpecialItemGroup((uint32_t)lua_tonumber(L, 1));

		if (!pItemGroup)
		{
			sys_err("cannot find special item group %d", (uint32_t)lua_tonumber(L, 1));
			return 0;
		}

		int32_t count = pItemGroup->GetGroupSize();
		
		for (int32_t i = 0; i < count; i++)
		{
			lua_pushnumber(L, (int32_t)pItemGroup->GetVnum(i));
			lua_pushnumber(L, (int32_t)pItemGroup->GetCount(i));
		}

		return count*2;
	}

	int32_t _mysql_update_query(lua_State* L) 
	{
		SQLMsg * msg = DBManager::Instance().DirectQuery(lua_tostring(L, 1));
		if (msg) 
		{
			lua_pushnumber(L, 1);
		}
		else
		{
			sys_err("MySQL Query failed!");
			lua_pushnil(L);
		}

		M2_DELETE(msg);
		return 1;
	}

	struct FMobCounter
	{
		int32_t nCount;
		uint32_t iSpecificVnum;

		FMobCounter(uint32_t specificVnum) 
		{
			iSpecificVnum = specificVnum;
			nCount = 0;
		}

		void operator () (LPENTITY ent)
		{
			if (ent->IsType(ENTITY_CHARACTER))
			{
				LPCHARACTER pChar = static_cast<CHARACTER *>(ent);

				if (iSpecificVnum) 
				{
					if (pChar && pChar->GetRaceNum() == iSpecificVnum)
						nCount++;

					return;
				}
			}
		}
	};

	int32_t _find_boss_by_vnum(lua_State* L)
	{
		if (!lua_isnumber(L, 1))
		{
			sys_err("invalid argument");

			lua_pushnumber(L, 0);
			return 1;
		}
		uint32_t boss = (uint32_t)lua_tonumber(L, 1);

		LPCHARACTER ch = CQuestManager::Instance().GetCurrentCharacterPtr();
		if (!ch)
		{
			sys_err("null ch ptr");

			lua_pushnumber(L, 0);
			return 1;
		}

		LPSECTREE_MAP pSectree = SECTREE_MANAGER::Instance().GetMap(ch->GetMapIndex());
		if (!pSectree)
		{
			lua_pushnumber(L, 0);
			return 1;
		}

		FMobCounter f(boss);
		pSectree->for_each(f);

		if (boss)
		{
			lua_pushnumber(L, f.nCount);
			return 1;
		}

		lua_pushnumber(L, 0);
		return 1;
	}

	int32_t _purge_vid(lua_State* L)
	{
		if (!lua_isnumber(L, 1))
		{
			sys_err("_purge_vid: invalid vid");
			return 0;
		}
		uint32_t vid = (uint32_t)lua_tonumber(L, 1);

		CHARACTER* ch = CQuestManager::Instance().GetCurrentNPCCharacterPtr();
		CHARACTER* npc = CHARACTER_MANAGER::Instance().Find(vid);
		if (!npc || !ch)
			return 0;

		if ((npc->IsMonster() || npc->IsStone()) &&
			!(npc->IsPet() || npc == ch))
		{
			M2_DESTROY_CHARACTER(npc);
		}

		return 0;
	}

	int32_t _spawn_mob_in_map(lua_State* L)
	{
		if (!lua_isnumber(L, 1) || !lua_isnumber(L, 2) || !lua_isboolean(L, 3) ||
			!lua_isnumber(L, 4) || !lua_isnumber(L, 5) || !lua_isnumber(L, 6))
		{
			lua_pushnumber(L, 0);
			return 1;
		}

		auto dwVnum			= static_cast<uint32_t>(lua_tonumber(L, 1));
		auto count			= MINMAX(1, static_cast<int32_t>(lua_tonumber(L, 2)), 10);
		auto isAggressive	= static_cast<bool>(lua_toboolean(L, 3));
		auto iMapIndex		= static_cast<int32_t>(lua_tonumber(L, 4));
		auto iMapX			= static_cast<int32_t>(lua_tonumber(L, 5));
		auto iMapY			= static_cast<int32_t>(lua_tonumber(L, 6));

		sys_log(0, "QUEST _spawn_mob_in_map: VNUM(%u) COUNT(%d) isAggressive(%b) MapIndex(%d) MapX(%d) MapY(%d)", dwVnum, count, isAggressive, iMapIndex, iMapX, iMapY);

		GPOS pos;
		if (!SECTREE_MANAGER::Instance().GetMapBasePositionByMapIndex(iMapIndex, pos))
		{
			sys_err("QUEST _spawn_mob_in_map: cannot find base position in this map %d", iMapIndex);
			lua_pushnumber(L, 0);
			return 1;
		}

		size_t SpawnCount = 0;

		auto pMonster = CMobManager::Instance().Get(dwVnum);
		if (pMonster)
		{
			for (int32_t i = 0; i < count; ++i)
			{
				auto pSpawnMonster = CHARACTER_MANAGER::Instance().SpawnMobRange(
					dwVnum,
					iMapIndex,
					pos.x - number(200, 750) + (iMapX * 100),
					pos.y - number(200, 750) + (iMapY * 100),
					pos.x + number(200, 750) + (iMapX * 100),
					pos.y + number(200, 750) + (iMapY * 100),
					true,
					pMonster->m_table.bType == CHAR_TYPE_STONE,
					isAggressive
				);

				if (pSpawnMonster)
				{
					++SpawnCount;
				}
			}

			sys_log(0, "QUEST Spawn Monster: VNUM(%u) COUNT(%u) isAggressive(%b)", dwVnum, SpawnCount, isAggressive);
		}

		lua_pushnumber(L, SpawnCount);
		return 1;
	}

	int32_t _list_guild(lua_State* L)
	{
		std::ostringstream s;
		combine_lua_string(L, s);

		auto descset = DESC_MANAGER::Instance().GetClientSet();

		auto it = descset.begin();
		while (it != descset.end())
		{
			LPDESC d = *(it++);

			if (!d || !d->GetCharacter())
				continue;

			CGuild * g = d->GetCharacter()->GetGuild();
			if (!g)
				continue;

			if (g->UnderAnyWar())
				continue;

			uint32_t gm_pid = g->GetMasterPID();

			if (gm_pid == d->GetCharacter()->GetPlayerID())
			{
				d->GetCharacter()->ChatPacket(CHAT_TYPE_COMMAND, "guild %s %s %d", g->GetName(), d->GetCharacter()->GetName(), g->GetID());
			}
		}
		return 0;
	}

	int32_t SQL_Table__PlayerQuery(lua_State* L)
	{
		if (!lua_isstring(L, 1))
		{
			sys_err("wrong arg");
			return 0;
		}
		std::string str = lua_tostring(L, 1);

		SQLMsg* msg = DBManager::Instance().DirectQuery(str.c_str());
		if (!msg)
		{
			sys_err("SQL_Table__PlayerQuery fail!");
			return 0;
		}	
		if (msg->uiSQLErrno)
		{
			sys_err("SQL_Table__PlayerQuery fail! error: %u", msg->uiSQLErrno);

			delete msg;
			return 0;
		}

		auto res = msg->Get()->pSQLResult;
		if (!res)
		{
			sys_err("null sql result ptr!");
	
			delete msg;
			return 0;
		}

		//Fetch field list
		uint32_t rowcount = 1;

		std::set<MYSQL_FIELD*> fieldList;
		while (MYSQL_FIELD *f = mysql_fetch_field(res)) 
		{
			fieldList.insert(f);
		}

		MYSQL_ROW row;
		while ((row = mysql_fetch_row(res)))
		{
			lua_newtable(L);

			int32_t i = 0;
			for (auto iter = fieldList.begin(); iter != fieldList.end(); ++iter)
			{
				MYSQL_FIELD *f = *iter;
				lua_pushstring(L, f->name);
				lua_pushstring(L, row[i]);
				lua_settable(L, -3); //stack: [arg] table table2
				i++;
			}

			lua_pushnumber(L, rowcount);
			lua_insert(L, -2); //stack: [arg] table rowcount table2
			lua_settable(L, -3);
			rowcount++;
		}

		M2_DELETE(msg);
		return 1;
	}

	int32_t _mysql_escape_string(lua_State* L)
	{
		if (!lua_isstring(L, 1))
			return 0;

		char szQuery[1024] = { 0 };
		DBManager::Instance().EscapeString(szQuery, sizeof(szQuery), lua_tostring(L, 1), strlen(lua_tostring(L, 1)));

		lua_pushstring(L, szQuery);
		return 1;
	}

	int32_t _say_reward(lua_State* L)
	{
		std::ostringstream s;
		combine_lua_string(L, s);

		CQuestManager::Instance().AddScript(s.str() + "[ENTER]");
		return 0;
	}

	int32_t _get_current_time_ms(lua_State* L)
	{
		lua_pushnumber(L, get_dword_time());
		return 1;
	}

	int32_t _get_current_time_s(lua_State* L)
	{
		uint32_t result = static_cast<uint32_t>((get_dword_time() / 1000) + 0.5);

		lua_pushnumber(L, result);
		return 1;
	}

	int32_t _check_time(lua_State* L)
	{
		if (!lua_isnumber(L, 1) || !lua_isnumber(L, 2))
		{
			lua_pushnumber(L, 0);
			return 1;
		}
		uint32_t timex = (uint32_t)lua_tonumber(L, 1);
		uint32_t timey = (uint32_t)lua_tonumber(L, 2);

		lua_pushnumber(L, MIN(0, (timey - timex)));
		return 1;
	}

	void RegisterGlobalFunctionTable(lua_State* L)
	{
		extern int32_t quest_setstate(lua_State* L);

		luaL_reg global_functions[] =
		{
			{	"sys_err",					_syserr					},
			{	"sys_log",					_syslog					},
			{	"char_log",					_char_log				},
			{	"item_log",					_item_log				},
			{	"set_state",				quest_setstate			},
			{	"set_skin",					_set_skin				},
			{	"setskin",					_set_skin				},
			{	"time_to_str",				_time_to_str			},
			{	"say",						_say					},
			{	"flush",					_flush					},
			{	"chat",						_chat					},
			{	"cmdchat",					_cmdchat				},
			{	"syschat",					_syschat				},
			{	"get_locale",				_get_locale				},
			{	"setleftimage",				_left_image				},
			{	"settopimage",				_top_image				},
			{	"server_timer",				_set_server_timer		},
			{	"clear_server_timer",		_clear_server_timer		},
			{	"server_loop_timer",		_set_server_loop_timer	},
			{	"get_server_timer_arg",		_get_server_timer_arg	},
			{	"timer",					_timer					},
			{	"loop_timer",				_set_named_loop_timer	},
			{	"cleartimer",				_clear_named_timer		},
			{	"getnpcid",					_getnpcid				},
			{	"is_test_server",			_is_test_server			},
			{	"raw_script",				_raw_script				},
			{	"number",					_number	   				},

			// LUA_ADD_BGM_INFO
			{	"set_bgm_volume_enable",	_set_bgm_volume_enable	},
			{	"add_bgm_info",				_add_bgm_info			},
			// END_OF_LUA_ADD_BGM_INFO

			// LUA_ADD_GOTO_INFO
			{	"add_goto_info",			_add_goto_info			},
			// END_OF_LUA_ADD_GOTO_INFO

			// REFINE_PICK
			{	"__refine_pick",			_refine_pick			},
			// END_OF_REFINE_PICK

			{	"add_ox_quiz",					_add_ox_quiz					},
			{	"__fish_real_refine_rod",		_fish_real_refine_rod			}, // XXX
			{	"__give_char_priv",				_give_char_privilege			},
			{	"__give_empire_priv",			_give_empire_privilege			},
			{	"__give_guild_priv",			_give_guild_privilege			},
			{	"__get_empire_priv_string",		_get_empire_privilege_string	},
			{	"__get_empire_priv",			_get_empire_privilege			},
			{	"__get_guild_priv_string",		_get_guild_privilege_string		},
			{	"__get_guildid_byname",			_get_guildid_byname				},
			{	"__get_guild_priv",				_get_guild_privilege			},
			{	"item_name",					_item_name						},
			{	"mob_name",						_mob_name						},
			{	"mob_vnum",						_mob_vnum						},
			{	"get_time",						_get_global_time				},
			{	"get_global_time",				_get_global_time				},
			{	"get_channel_id",				_get_channel_id					},
			{	"command",						_do_command						},
			{	"find_pc_cond",					_find_pc_cond					},
			{	"find_pc_by_name",				_find_pc						},
			{	"find_npc_by_vnum",				_find_npc_by_vnum				},
			{	"set_quest_state",				_set_quest_state				},
			{	"get_quest_state",				_get_quest_state				},
			{	"notice",						_notice							},
			{	"notice_all",					_notice_all						},
			{	"notice_in_map",				_notice_in_map					},
			{	"warp_all_to_village",			_warp_all_to_village			},
			{	"warp_to_village",				_warp_to_village				},	
			{	"say_in_map",					_say_in_map						},	
			{	"kill_all_in_map",				_kill_all_in_map				},
			{	"regen_in_map",					_regen_in_map					},
			{	"block_chat",					_block_chat						},
			{	"spawn_mob",					_spawn_mob						},
			{	"get_locale_base_path",			_get_locale_base_path			},
			{	"purge_area",					_purge_area						},
			{	"warp_all_in_area_to_area",		_warp_all_in_area_to_area		},

			{	"mysql_update",					_mysql_update_query				},

			{	"get_special_item_group",		_get_special_item_group			},
			{   "set_quest_flag_in_area",		_set_quest_flag_in_area			},
			{   "find_boss_by_vnum",			_find_boss_by_vnum				},

			{	"purge_vid",					_purge_vid						},
			{	"spawn_mob_in_map",				_spawn_mob_in_map				},
			{	"guild_list",           		_list_guild             		},
			{	"mysql_player_query",			SQL_Table__PlayerQuery			},
			{	"mysql_escape_string",			_mysql_escape_string			},
			{	"say_reward",					_say_reward						},

			{  "get_current_time_ms",			_get_current_time_ms			},
			{  "get_current_time_s",			_get_current_time_s				},
			{  "check_time",					_check_time						},

			{	nullptr,	nullptr	}
		};
	
		int32_t i = 0;

		while (global_functions[i].name != nullptr)
		{
			lua_register(L, global_functions[i].name, global_functions[i].func);
			++i;
		}
	}
}

