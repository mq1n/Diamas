#include "stdafx.h"
#include "affect.h"
#include "buffer_manager.h"
#include "char.h"
#include "char_manager.h"
#include "config.h"
#include "db.h"
#include "xmas_event.h"
#include "desc.h"
#include "guild.h"
#include "guild_manager.h"
#include "item.h"
#include "locale_service.h"
#include "questlua.h"
#include "quest_manager.h"
#include "sectree_manager.h"

#include <sstream>

#undef sys_err
#ifndef _WIN32
#define sys_err(fmt, args...) quest::CQuestManager::Instance().QuestError(__FUNCTION__, __LINE__, fmt, ##args)
#else
#define sys_err(fmt, ...) quest::CQuestManager::Instance().QuestError(__FUNCTION__, __LINE__, fmt, __VA_ARGS__)
#endif

namespace quest
{

	std::string ScriptToString(const std::string& str)
	{
		lua_State* L = CQuestManager::Instance().GetLuaState();
		int32_t x = lua_gettop(L);

		int32_t errcode = lua_dobuffer(L, ("return "+str).c_str(), str.size()+7, "ScriptToString");

		std::string retstr;
		if (!errcode)
		{
			if (lua_isstring(L,-1))
				retstr = lua_tostring(L, -1);
		}
		else
		{
			sys_err("LUA ScriptRunError (code:%d src:[%s])", errcode, str.c_str());
		}

		lua_settop(L,x);
		return retstr;
	}

	void FSetWarpLocation::operator()(CHARACTER* ch) const
	{
		if (ch->IsPC())
		{
			ch->SetWarpLocation (map_index, x, y);
		}
	}

	void FSetQuestFlag::operator()(CHARACTER* ch) const
	{
		if (!ch->IsPC())
			return;

		PC * pPC = CQuestManager::Instance().GetPCForce(ch->GetPlayerID());
		if (pPC)
			pPC->SetFlag(flagname, value);
	}

	bool FPartyCheckFlagLt::operator()(CHARACTER* ch)
	{
		if (!ch || !ch->IsPC())
			return false;

		bool returnBool = false;

		PC* pPC = CQuestManager::Instance().GetPCForce(ch->GetPlayerID());
		if (pPC)
		{
			int32_t flagValue = pPC->GetFlag(flagname);
			if (flagValue < value)
				returnBool = true;
			else
				returnBool = false;
		}

		return returnBool;
	}

	FPartyChat::FPartyChat(int32_t ChatType, const char* str) : 
		iChatType(ChatType), str(str)
	{
	}

	void FPartyChat::operator()(CHARACTER* ch)
	{
		if (ch)
			ch->ChatPacket(iChatType, "%s", str);
	}

	void FPartyClearReady::operator() (LPCHARACTER ch)
	{
		ch->RemoveAffect(AFFECT_DUNGEON_READY);
	}

	void FSendPacket::operator() (LPENTITY ent)
	{
		if (ent->IsType(ENTITY_CHARACTER))
		{
			LPCHARACTER ch = (LPCHARACTER) ent;

			if (ch && ch->GetDesc()) 
			{
				ch->GetDesc()->Packet(buf.read_peek(), buf.size());
			}
		}
	}

#ifdef ENABLE_NEWSTUFF
	void FSendChatPacket::operator() (LPENTITY ent)
	{
		if (ent->IsType(ENTITY_CHARACTER))
		{
			LPCHARACTER ch = (LPCHARACTER) ent;
			if (ch && ch->GetDesc()) 
				ch->ChatPacket(m_chat_type, "%s", m_text.c_str());
		}
	}
#endif

	void FSendPacketToEmpire::operator() (LPENTITY ent)
	{
		if (ent->IsType(ENTITY_CHARACTER))
		{
			LPCHARACTER ch = (LPCHARACTER) ent;

			if (ch && ch->GetDesc()) 
			{
				if (ch->GetEmpire() == bEmpire)
					ch->GetDesc()->Packet(buf.read_peek(), buf.size());
			}
		}
	}

	void FWarpEmpire::operator() (LPENTITY ent)
	{
		if (ent->IsType(ENTITY_CHARACTER))
		{
			LPCHARACTER ch = (LPCHARACTER) ent;

			if (ch && ch->IsPC() && ch->GetEmpire() == m_bEmpire)
			{
				ch->WarpSet(m_x, m_y, m_lMapIndexTo);
			}
		}
	}

	FBuildLuaGuildWarList::FBuildLuaGuildWarList(lua_State* lua_state) :
		L(lua_state), m_count(1)
	{
		lua_newtable(lua_state);
	}

	void FBuildLuaGuildWarList::operator() (uint32_t g1, uint32_t g2)
	{
		CGuild* g = CGuildManager::Instance().FindGuild(g1);

		if (!g)
			return;

		if (g->GetGuildWarType(g2) == GUILD_WAR_TYPE_FIELD)
			return;

		if (g->GetGuildWarState(g2) != GUILD_WAR_ON_WAR)
			return;

		lua_newtable(L);
		lua_pushnumber(L, g1);
		lua_rawseti(L, -2, 1);
		lua_pushnumber(L, g2);
		lua_rawseti(L, -2, 2);
		lua_rawseti(L, -2, m_count++);
	}

	bool IsScriptTrue(const char* code, int32_t size)
	{
		if (size==0)
			return true;

		lua_State* L = CQuestManager::Instance().GetLuaState();
		int32_t x = lua_gettop(L);
		int32_t errcode = lua_dobuffer(L, code, size, "IsScriptTrue");
		int32_t bStart = lua_toboolean(L, -1);
		if (errcode)
		{
			char buf[100];
			snprintf(buf, sizeof(buf), "LUA ScriptRunError (code:%%d src:[%%%ds])", size);
			sys_err(buf, errcode, code);
		}
		lua_settop(L,x);
		return bStart != 0;
	}

	void combine_lua_string(lua_State* L, std::ostringstream& s)
	{
		char buf[32];

		int32_t n = lua_gettop(L);
		int32_t i;

		for (i = 1; i <= n; ++i)
		{
			if (lua_isstring(L,i))
			{
				s << lua_tostring(L, i);
			}
			else if (lua_isnumber(L, i))
			{
				snprintf(buf, sizeof(buf), "%.14g\n", lua_tonumber(L,i));
				s << buf;
			}
		}
	}

	// 
	// "member" Lua functions
	//
	int32_t member_chat(lua_State* L)
	{
		std::ostringstream s;
		combine_lua_string(L, s);

		CQuestManager::Instance().GetCurrentPartyMember()->ChatPacket(CHAT_TYPE_TALKING, "%s", s.str().c_str());

		return 0;
	}

	int32_t member_clear_ready(lua_State* L)
	{
		CHARACTER* ch = CQuestManager::Instance().GetCurrentPartyMember();
		if (ch)
			ch->RemoveAffect(AFFECT_DUNGEON_READY);
		return 0;
	}

	int32_t member_set_ready(lua_State* L)
	{
		CHARACTER* ch = CQuestManager::Instance().GetCurrentPartyMember();
		if (ch)
			ch->AddAffect(AFFECT_DUNGEON_READY, POINT_NONE, 0, AFF_DUNGEON_READY, 65535, 0, true);
		return 0;
	}

	int32_t mob_spawn(lua_State* L)
	{
		if (!lua_isnumber(L, 1) || !lua_isnumber(L, 2) || !lua_isnumber(L, 3) || !lua_isnumber(L, 4))
		{
			sys_err("invalid argument");
			return 0;
		}

		uint32_t mob_vnum = (uint32_t)lua_tonumber(L, 1);
		int32_t local_x = (int32_t) lua_tonumber(L, 2)*100;
		int32_t local_y = (int32_t) lua_tonumber(L, 3)*100;
		float radius = (float) lua_tonumber(L, 4)*100;
		bool bAggressive = lua_toboolean(L, 5);
		uint32_t count = (lua_isnumber(L, 6))?(uint32_t) lua_tonumber(L, 6):1;
		bool noReward = lua_toboolean(L, 7);
		const char* mobName = lua_tostring(L, 8);

		if (count == 0)
			count = 1;
		else if (count > 10)
		{
			sys_err("count bigger than 10");
			count = 10;
		}

		LPCHARACTER ch = CQuestManager::Instance().GetCurrentCharacterPtr();
		LPSECTREE_MAP pMap = SECTREE_MANAGER::Instance().GetMap(ch->GetMapIndex());
		if (!ch || !pMap)
			return 0;

		uint32_t dwQuestIdx = CQuestManager::Instance().GetCurrentPC()->GetCurrentQuestIndex();

		bool ret = false;
		LPCHARACTER mob = nullptr;

		while (count--)
		{
			for (int32_t loop = 0; loop < 8; ++loop)
			{
				double angle = number(0, 999) * M_PI * 2 / 1000;
				double r = number(0, 999) * radius / 1000;

				int32_t x = local_x + pMap->m_setting.iBaseX + (int32_t)(r * cos(angle));
				int32_t y = local_y + pMap->m_setting.iBaseY + (int32_t)(r * sin(angle));

				mob = CHARACTER_MANAGER::Instance().SpawnMob(mob_vnum, ch->GetMapIndex(), x, y, 0);

				if (mob)
					break;
			}

			if (mob)
			{
				if (bAggressive)
					mob->SetAggressive();

				if (noReward)
					mob->SetNoRewardFlag();

				mob->SetQuestBy(dwQuestIdx);

				if (mobName)
				{
					mob->SetName(mobName);
				}

				if (!ret)
				{
					ret = true;
					lua_pushnumber(L, (uint32_t) mob->GetVID());
				}
			}
		}

		if (!ret)
			lua_pushnumber(L, 0);

		return 1;
	}

	int32_t mob_spawn_group(lua_State* L)
	{
		if (!lua_isnumber(L, 1) || !lua_isnumber(L, 2) || !lua_isnumber(L, 3) || !lua_isnumber(L, 4) || !lua_isnumber(L, 6))
		{
			sys_err("invalid argument");

			lua_pushnumber(L, 0);
			return 1;
		}

		uint32_t group_vnum = (uint32_t)lua_tonumber(L, 1);
		int32_t local_x = (int32_t) lua_tonumber(L, 2) * 100;
		int32_t local_y = (int32_t) lua_tonumber(L, 3) * 100;
		double radius = (double)lua_tonumber(L, 4) * 100;
		bool bAggressive = lua_toboolean(L, 5);
		uint32_t count = (uint32_t) lua_tonumber(L, 6);

		if (count == 0)
		{
			count = 1;
		}
		else if (count > 10)
		{
			sys_err("count bigger than 10");
			count = 10;
		}

		CHARACTER* ch = CQuestManager::Instance().GetCurrentCharacterPtr();
		LPSECTREE_MAP pMap = SECTREE_MANAGER::Instance().GetMap(ch->GetMapIndex());
		if (!ch || !pMap) 
		{
			lua_pushnumber(L, 0);
			return 1;
		}
		uint32_t dwQuestIdx = CQuestManager::Instance().GetCurrentPC()->GetCurrentQuestIndex();

		bool ret = false;
		LPCHARACTER mob = nullptr;

		while (count--)
		{
			for (int32_t loop = 0; loop < 8; ++loop)
			{
				double angle = number(0, 999) * M_PI * 2 / 1000;
				double r = number(0, 999) * radius / 1000;

				int32_t x = local_x + pMap->m_setting.iBaseX + (int32_t)(r * cos(angle));
				int32_t y = local_y + pMap->m_setting.iBaseY + (int32_t)(r * sin(angle));

				mob = CHARACTER_MANAGER::Instance().SpawnGroup(group_vnum, ch->GetMapIndex(), x, y, x, y, nullptr, bAggressive);

				if (mob)
					break;
			}

			if (mob)
			{
				mob->SetQuestBy(dwQuestIdx);

				if (!ret)
				{
					ret = true;
					lua_pushnumber(L, (uint32_t) mob->GetVID());
				}
			}
		}

		if (!ret)
			lua_pushnumber(L, 0);

		return 1;
	}

	//
	// global Lua functions
	//
	//
	// Registers Lua function table
	//
	void CQuestManager::AddLuaFunctionTable(const char * c_pszName, luaL_reg * preg, bool bCheckIfExists)
	{
#ifdef ENABLE_NEWSTUFF
		bool bIsExists = false;
		if (bCheckIfExists)
		{
			int32_t x = lua_gettop(L);
			lua_getglobal(L, c_pszName);
			if (!lua_istable(L, -1))
			{
				lua_settop(L, x);
				bIsExists = true;
			}
		}
		if (!bIsExists)
			lua_newtable(L);
#else
		lua_newtable(L);
#endif

		while ((preg->name))
		{
			lua_pushstring(L, preg->name);
			lua_pushcfunction(L, preg->func);
			lua_rawset(L, -3);
			preg++;
		}

		lua_setglobal(L, c_pszName);
	}

	void CQuestManager::BuildStateIndexToName(const char* questName)
	{
		int32_t x = lua_gettop(L);
		lua_getglobal(L, questName);

		if (lua_isnil(L,-1))
		{
			sys_err("QUEST wrong quest state file for quest %s",questName);

			CHARACTER* ch = GetCurrentCharacterPtr();
			if (ch)
				sys_err("QUEST wrong quest state file from %s", ch->GetName());

			CHARACTER* npc = GetCurrentNPCCharacterPtr();
			if (npc)
				sys_err("QUEST wrong quest state file for %s", npc->GetName());

			lua_settop(L,x);
			return;
		}

		for (lua_pushnil(L); lua_next(L, -2);)
		{
			if (lua_isstring(L, -2) && lua_isnumber(L, -1))
			{
				lua_pushvalue(L, -2);
				lua_rawset(L, -4);
			}
			else
			{
				lua_pop(L, 1);
			}
		}

		lua_settop(L, x);
	}

	/**
	 * @version 05/06/08	Bang2ni - __get_guildid_byname 스크립트 함수 등록
	 */
	bool CQuestManager::InitializeLua()
	{
		L = lua_open();

		luaopen_base(L);
		luaopen_table(L);
		luaopen_string(L);
		luaopen_math(L);
		//TEMP
		luaopen_io(L);
		luaopen_debug(L);

		RegisterAffectFunctionTable();
		RegisterBuildingFunctionTable();
		RegisterDungeonFunctionTable();
		RegisterGameFunctionTable();
		RegisterGuildFunctionTable();
		RegisterHorseFunctionTable();
#ifdef __PET_SYSTEM__
		RegisterPetFunctionTable();
#endif
		RegisterITEMFunctionTable();
		RegisterMarriageFunctionTable();
		RegisterNPCFunctionTable();
		RegisterPartyFunctionTable();
		RegisterPCFunctionTable();
		RegisterQuestFunctionTable();
		RegisterTargetFunctionTable();
		RegisterArenaFunctionTable();
		RegisterForkedFunctionTable();
		RegisterOXEventFunctionTable();
		RegisterDanceEventFunctionTable();
		RegisterDragonSoulFunctionTable();

		{
			luaL_reg member_functions[] = 
			{
				{ "chat",			member_chat		},
				{ "set_ready",			member_set_ready	},
				{ "clear_ready",		member_clear_ready	},
				{ nullptr,				nullptr			}
			};

			AddLuaFunctionTable("member", member_functions);
		}

		{
			luaL_reg mob_functions[] =
			{
				{ "spawn",			mob_spawn		},
				{ "spawn_group",		mob_spawn_group		},
				{ nullptr,				nullptr			}
			};

			AddLuaFunctionTable("mob", mob_functions);
		}

		//
		// global namespace functions
		//
		RegisterGlobalFunctionTable(L);

		// LUA_INIT_ERROR_MESSAGE
		{
			char settingsFileName[256];
			snprintf(settingsFileName, sizeof(settingsFileName), "%s/settings.lua", LocaleService_GetBasePath().c_str());

			int32_t settingsLoadingResult = lua_dofile(L, settingsFileName);
			sys_log(0, "LoadSettings(%s), returns %d", settingsFileName, settingsLoadingResult);
			if (settingsLoadingResult != 0)
			{
				sys_err("LOAD_SETTINS_FAILURE(%s)", settingsFileName);
				return false;
			}
		}

		{
			char questlibFileName[256];
			snprintf(questlibFileName, sizeof(questlibFileName), "%s/questlib.lua", LocaleService_GetQuestPath().c_str());

			int32_t questlibLoadingResult = lua_dofile(L, questlibFileName);
			sys_log(0, "LoadQuestlib(%s), returns %d", questlibFileName, questlibLoadingResult);
			if (questlibLoadingResult != 0)
			{
				sys_err("LOAD_QUESTLIB_FAILURE(%s)", questlibFileName);
				return false;
			}
		}



		char translateFileName[256];
		snprintf(translateFileName, sizeof(translateFileName), "%s/translate.lua", LocaleService_GetBasePath().c_str());

		int32_t translateLoadingResult = lua_dofile(L, translateFileName);
		sys_log(0, "LoadTranslate(%s), returns %d", translateFileName, translateLoadingResult);

		if (translateLoadingResult != 0) 
		{
			sys_err("LOAD_TRANSLATE_ERROR(%s)", translateFileName);
			return false;
		}



		char questLocaleFileName[256];
		snprintf(questLocaleFileName, sizeof(questLocaleFileName), "%s/locale.lua", g_stQuestDir.c_str());

		int32_t questLocaleLoadingResult = lua_dofile(L, questLocaleFileName);
		sys_log(0, "LoadQuestLocale(%s), returns %d", questLocaleFileName, questLocaleLoadingResult);

		if (questLocaleLoadingResult != 0) 
		{
			sys_err("LoadQuestLocale(%s) FAILURE", questLocaleFileName);
			return false;
		}



		for (const auto& stQuestObjectDir : g_setQuestObjectDir)
		{
			std::ostringstream oss;
			oss << stQuestObjectDir << "/state/";

			auto iQuestIdx = 0;

			if (std::filesystem::exists(oss.str()))
			{
				for (const auto& entry : std::filesystem::directory_iterator(oss.str()))
				{
					if (entry.is_regular_file())
					{
						const auto& filename = entry.path().filename().string();
						RegisterQuest(filename, ++iQuestIdx);

						auto ret = lua_dofile(L, (stQuestObjectDir + "/state/" + filename).c_str());

						sys_log(0, "QUEST: loading %s, returns %d", (stQuestObjectDir + "/state/" + filename).c_str(), ret);

						BuildStateIndexToName(filename.c_str());
					}
				}
			}
		}

		lua_setgcthreshold(L, 0);

		lua_newtable(L);
		lua_setglobal(L, "__codecache");
		return true;
	}

	void CQuestManager::GotoSelectState(QuestState& qs)
	{
		lua_checkstack(qs.co, 1);

		//int32_t n = lua_gettop(L);
		int32_t n = luaL_getn(qs.co, -1);
		qs.args = n;
		//cout << "select here (1-" << qs.args << ")" << endl;
		//

		std::ostringstream os;
		os << "[QUESTION ";

		for (int32_t i=1; i<=n; i++)
		{
			lua_rawgeti(qs.co,-1,i);
			if (lua_isstring(qs.co,-1))
			{
				//printf("%d\t%s\n",i,lua_tostring(qs.co,-1));
				if (i != 1)
					os << "|";
				os << i << ";" << lua_tostring(qs.co,-1);
			}
			else
			{
				sys_err("SELECT wrong data %s", lua_typename(qs.co, -1));
				sys_err("here %s", qs.questName.c_str());
			}
			lua_pop(qs.co,1);
		}
		os << "]";


		AddScript(os.str());
		qs.suspend_state = SUSPEND_STATE_SELECT;
		if ( g_bIsTestServer )
			sys_log( 0, "%s", m_strScript.c_str() );
		SendScript();
	}

	EVENTINFO(confirm_timeout_event_info)
	{
		uint32_t dwWaitPID;
		uint32_t dwReplyPID;

		confirm_timeout_event_info() :
			dwWaitPID(0) , dwReplyPID(0)
		{
		}
	};

	EVENTFUNC(confirm_timeout_event)
	{
		confirm_timeout_event_info * info = dynamic_cast<confirm_timeout_event_info *>(event->info);

		if ( info == nullptr )
		{
			sys_err( "confirm_timeout_event> <Factor> Null pointer" );
			return 0;
		}

		LPCHARACTER chWait = CHARACTER_MANAGER::Instance().FindByPID(info->dwWaitPID);
		LPCHARACTER chReply = nullptr; //CHARACTER_MANAGER::info().FindByPID(info->dwReplyPID);

		if (chReply) {
			// 시간 지나면 알아서 닫힘
		}

		if (chWait) {
			CQuestManager::Instance().Confirm(info->dwWaitPID, CONFIRM_TIMEOUT);
		}

		return 0;
	}

	void CQuestManager::GotoConfirmState(QuestState & qs)
	{
		qs.suspend_state = SUSPEND_STATE_CONFIRM;
		uint32_t dwVID = (uint32_t) lua_tonumber(qs.co, -3);
		const char* szMsg = lua_tostring(qs.co, -2);
		int32_t iTimeout = (int32_t) lua_tonumber(qs.co, -1);

		sys_log(0, "GotoConfirmState vid %u msg '%s', timeout %d", dwVID, szMsg, iTimeout);

		// 1. 상대방에게 확인창 띄움
		// 2. 나에게 확인 기다린다고 표시하는 창 띄움
		// 3. 타임아웃 설정 (타임아웃 되면 상대방 창 닫고 나에게도 창 닫으라고 보냄)

		// 1
		// 상대방이 없는 경우는 그냥 상대방에게 보내지 않는다. 타임아웃에 의해서 넘어가게됨
		CHARACTER* ch = CHARACTER_MANAGER::Instance().Find(dwVID);
		if (ch && ch->IsPC())
		{
			ch->ConfirmWithMsg(szMsg, iTimeout, GetCurrentCharacterPtr()->GetPlayerID());
		}

		// 2
		GetCurrentPC()->SetConfirmWait((ch && ch->IsPC())?ch->GetPlayerID():0);
		std::ostringstream os;
		os << "[CONFIRM_WAIT timeout;" << iTimeout << "]";
		AddScript(os.str());
		SendScript();

		// 3
		confirm_timeout_event_info* info = AllocEventInfo<confirm_timeout_event_info>();

		info->dwWaitPID = GetCurrentCharacterPtr()->GetPlayerID();
		info->dwReplyPID = (ch && ch->IsPC()) ? ch->GetPlayerID() : 0;

		event_create(confirm_timeout_event, info, PASSES_PER_SEC(iTimeout));
	}

	void CQuestManager::GotoSelectItemState(QuestState& qs)
	{
		qs.suspend_state = SUSPEND_STATE_SELECT_ITEM;
		AddScript("[SELECT_ITEM]");
		SendScript();
	}

	void CQuestManager::GotoInputState(QuestState & qs)
	{
		qs.suspend_state = SUSPEND_STATE_INPUT;
		AddScript("[INPUT]");
		SendScript();

		// 시간 제한을 검
		//event_create(input_timeout_event, dwEI, PASSES_PER_SEC(iTimeout));
	}

	void CQuestManager::GotoPauseState(QuestState & qs)
	{
		qs.suspend_state = SUSPEND_STATE_PAUSE;
		AddScript("[NEXT]");
		SendScript();
	}

	void CQuestManager::GotoEndState(QuestState & qs)
	{
		AddScript("[DONE]");
		SendScript();
	}

	//
	// * OpenState
	//
	// The beginning of script
	// 

	QuestState CQuestManager::OpenState(const std::string& quest_name, int32_t state_index)
	{
		QuestState qs;
		qs.args = 0;
		qs.st = state_index;
		qs.questName = quest_name;
		qs.co = lua_newthread(L);
		qs.ico = lua_ref(L, 1 /*qs.co*/);

		CHARACTER* ch = CQuestManager::Instance().GetCurrentCharacterPtr();
		if (ch)
			qs._item = ch->GetQuestItemPtr();

		return qs;
	}

	//
	// * RunState
	// 
	// decides script to wait for user input, or finish
	// 
	bool CQuestManager::RunState(QuestState & qs)
	{
		ClearError();

		m_CurrentRunningState = &qs;
		int32_t ret = lua_resume(qs.co, qs.args);

		if (ret == 0)
		{
			if (lua_gettop(qs.co) == 0)
			{
				// end of quest
				GotoEndState(qs);
				return false;
			}

			if (!strcmp(lua_tostring(qs.co, 1), "select")) {
				GotoSelectState(qs);
				return true;
			}

			if (!strcmp(lua_tostring(qs.co, 1), "wait")) {
				GotoPauseState(qs);
				return true;
			}

			if (!strcmp(lua_tostring(qs.co, 1), "input")) {
				GotoInputState(qs);
				return true;
			}

			if (!strcmp(lua_tostring(qs.co, 1), "confirm")) {
				GotoConfirmState(qs);
				return true;
			}

			if (!strcmp(lua_tostring(qs.co, 1), "select_item")) {
				GotoSelectItemState(qs);
				return true;
			}
		} 
		else 
		{
			sys_err("LUA_ERROR: %s WITH ERRORCODE %d", lua_tostring(qs.co, 1), ret);
			sys_err("LUA_STATE: index %d ref %d", qs.st, qs.ico);
		}

		WriteRunningStateToSyserr();
		SetError();

		GotoEndState(qs);
		return false;
	}

	//
	// * CloseState
	//
	// makes script end
	//
	void CQuestManager::CloseState(QuestState& qs)
	{
		if (qs._item)
			qs._item = nullptr;

		if (qs.co)
		{
			//cerr << "ICO "<<qs.ico <<endl;
			lua_unref(L, qs.ico);
			qs.co = nullptr;
		}
	}
}
