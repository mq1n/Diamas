#include "stdafx.h"
#include <sstream>
#include "desc.h"
#include "party.h"
#include "char.h"
#include "questlua.h"
#include "quest_manager.h"
#include "packet.h"
#include "char_manager.h"

#undef sys_err
#ifndef __WIN32__
#define sys_err(fmt, args...) quest::CQuestManager::instance().QuestError(__FUNCTION__, __LINE__, fmt, ##args)
#else
#define sys_err(fmt, ...) quest::CQuestManager::instance().QuestError(__FUNCTION__, __LINE__, fmt, __VA_ARGS__)
#endif

namespace quest
{
	//
	// "party" Lua functions
	//
	int32_t party_clear_ready(lua_State* L)
	{
		auto ch = CQuestManager::instance().GetCurrentCharacterPtr();
		if (!ch) 
		{
			sys_err("Null character pointer triggered at %s:%d", __FILE__, __LINE__);
			return 0;
		}

		auto f = [](CHARACTER* member) {
			member->RemoveAffect(AFFECT_DUNGEON_READY);
		};

		if (ch->GetParty()) {
			ch->GetParty()->ForEachNearMember(f);
		}
		return 0;
	}

	int32_t party_get_max_level(lua_State* L)
	{
		auto ch = CQuestManager::instance().GetCurrentCharacterPtr();
		if (!ch)
		{
			sys_err("Null character pointer triggered at %s:%d", __FILE__, __LINE__);
			return 0;
		}

		if (ch->GetParty())
			lua_pushnumber(L,ch->GetParty()->GetMemberMaxLevel());
		else
			lua_pushnumber(L, 1);

		return 1;
	}

    struct FRunCinematicSender
    {
        std::string data;
		SPacketGCScript pack;

        FRunCinematicSender(const char* str)
        {
            data = "[RUN_CINEMA value;";
            data += str;
            data += "]";

            pack.header = HEADER_GC_SCRIPT;
            pack.skin = CQuestManager::QUEST_SKIN_CINEMATIC;
            //pack.skin = CQuestManager::QUEST_SKIN_NOWINDOW;
			pack.src_size = static_cast<uint16_t>(data.size());
			pack.size = pack.src_size + sizeof(SPacketGCScript);
		}

		void operator()(CHARACTER* ch) const
		{
			if (ch)
			{
				sys_log(0, "CINEMASEND_TRY %s", ch->GetName());

				if (ch->GetDesc()) {
					sys_log(0, "CINEMASEND %s", ch->GetName());
					ch->GetDesc()->BufferedPacket(&pack, sizeof(SPacketGCScript));
					ch->GetDesc()->Packet(data.c_str(), data.size());
				}
			}
		}
    };

	int32_t party_run_cinematic(lua_State* L)
	{
		if (!lua_isstring(L, 1))
			return 0;
		
		auto ch = CQuestManager::instance().GetCurrentCharacterPtr();
		if (!ch)
		{
			sys_err("Null character pointer triggered at %s:%d", __FILE__, __LINE__);
			return 0;
		}

		sys_log(0, "RUN_CINEMA %s", lua_tostring(L, 1));

		if (ch->GetParty())
		{
			FRunCinematicSender f(lua_tostring(L, 1));

			ch->GetParty()->Update();
			ch->GetParty()->ForEachNearMember(f);
		}

		return 0;
	}

	int32_t party_get_min_level(lua_State* L)
	{
		CHARACTER* ch = CQuestManager::instance().GetCurrentCharacterPtr();
		if (!ch) 
		{
			sys_err("Null character pointer triggered at %s:%d", __FILE__, __LINE__);
			return 0;
		}

		if (ch->GetParty())
			lua_pushnumber(L, ch->GetParty()->GetMemberMinLevel());
		else
			lua_pushnumber(L, 1);

		return 1;
	}

	int32_t party_leave_party(lua_State* L)
	{
		CHARACTER* ch = CQuestManager::instance().GetCurrentCharacterPtr();
		if (!ch)
		{
			sys_err("Null character pointer triggered at %s:%d", __FILE__, __LINE__);
			return 0;
		}

		LPPARTY pParty = ch->GetParty();
		if (pParty)
		{
			if (pParty->GetMemberCount() == 2)
				CPartyManager::instance().DeleteParty(pParty);
			else
				pParty->Quit(ch->GetPlayerID());
		}

		lua_pushboolean(L, ch->GetParty() == nullptr);
		return 1;
	}

	int32_t party_delete_party(lua_State* L)
	{
		CHARACTER* ch = CQuestManager::instance().GetCurrentCharacterPtr();
		if (!ch) 
		{
			sys_err("Null character pointer triggered at %s:%d", __FILE__, __LINE__);
			return 0;
		}

		if (ch->GetParty() && ch->GetParty()->GetLeaderPID() == ch->GetPlayerID())
			CPartyManager::instance().DeleteParty(ch->GetParty());

		lua_pushboolean(L, ch->GetParty() == nullptr);
		return 1;
	}

	struct FCinematicSender 
	{
		const char* str;
		SPacketGCScript packet_script;
		int32_t len;

		FCinematicSender(const char* str) : 
			str(str)
		{
			len = strlen(str);

			packet_script.header = HEADER_GC_SCRIPT;
			packet_script.skin = CQuestManager::QUEST_SKIN_CINEMATIC;
			packet_script.src_size = len;
			packet_script.size = packet_script.src_size + sizeof(SPacketGCScript);
		}

		void operator()(CHARACTER* ch) const
		{
			if (ch)
			{
				sys_log(0, "CINEMASEND_TRY %s", ch->GetName());

				if (ch->GetDesc()) {
					sys_log(0, "CINEMASEND %s", ch->GetName());
					ch->GetDesc()->BufferedPacket(&packet_script,
						sizeof(SPacketGCScript));
					ch->GetDesc()->Packet(str, len);
				}
			}
		}
	};

	int32_t party_show_cinematic(lua_State* L)
	{
		if (!lua_isstring(L, 1))
			return 0;

		auto ch = CQuestManager::instance().GetCurrentCharacterPtr();
		if (!ch)
		{
			sys_err("Null character pointer triggered at %s:%d", __FILE__, __LINE__);
			return 0;
		}

		sys_log(0, "CINEMA %s", lua_tostring(L, 1));

		if (ch->GetParty())
		{
			FCinematicSender f(lua_tostring(L, 1));

			ch->GetParty()->Update();
			ch->GetParty()->ForEachNearMember(f);
		}
		return 0;
	}

	int32_t party_get_members_count(lua_State* L)
	{
		CHARACTER* ch = CQuestManager::instance().GetCurrentCharacterPtr();
		if (!ch) 
		{
			sys_err("Null character pointer triggered at %s:%d", __FILE__, __LINE__);
			return 0;
		}

		if (ch->GetParty())
			lua_pushnumber(L, ch->GetParty()->GetMemberCount());
		else
			lua_pushnumber(L, 0);

		return 1;
	}

	int32_t party_get_near_count(lua_State* L)
	{
		auto ch = CQuestManager::instance().GetCurrentCharacterPtr();
		if (!ch)
		{
			sys_err("Null character pointer triggered at %s:%d", __FILE__, __LINE__);
			return 0;
		}

		if (ch->GetParty())
			lua_pushnumber(L, ch->GetParty()->GetNearMemberCount());
		else
			lua_pushnumber(L, 0);

		return 1;
	}

	int32_t party_syschat(lua_State* L)
	{
		auto ch = CQuestManager::instance().GetCurrentCharacterPtr();
		if (!ch)
		{
			sys_err("Null character pointer triggered at %s:%d", __FILE__, __LINE__);
			return 0;
		}

		auto pParty = ch->GetParty();
		if (pParty) 
		{
			std::ostringstream s;
			combine_lua_string(L, s);

			FPartyChat f(CHAT_TYPE_INFO, s.str().c_str());

			pParty->ForEachOnlineMember(f);
		}

		return 0;
	}

	int32_t party_is_leader(lua_State* L)
	{
		auto ch = CQuestManager::instance().GetCurrentCharacterPtr();
		if (!ch)
		{
			sys_err("Null character pointer triggered at %s:%d", __FILE__, __LINE__);
			return 0;
		}

		if (ch->GetParty() && ch->GetParty()->GetLeaderPID() == ch->GetPlayerID())
			lua_pushboolean(L, true);
		else
			lua_pushboolean(L, false);

		return 1;
	}

	int32_t party_is_party(lua_State* L)
	{
		auto ch = CQuestManager::instance().GetCurrentCharacterPtr();
		if (!ch)
		{
			sys_err("Null character pointer triggered at %s:%d", __FILE__, __LINE__);
			return 0;
		}

		lua_pushboolean(L, ch->GetParty() ? true : false);
		return 1;
	}

	int32_t party_get_leader_pid(lua_State* L)
	{
		auto ch = CQuestManager::instance().GetCurrentCharacterPtr();
		if (!ch)
		{
			sys_err("Null character pointer triggered at %s:%d", __FILE__, __LINE__);
			return 0;
		}

		if (ch->GetParty()) {
			lua_pushnumber(L, ch->GetParty()->GetLeaderPID());
		} else {
			lua_pushnumber(L, -1);
		}
		return 1;
	}
	int32_t party_chat(lua_State* L)
	{
		auto ch = CQuestManager::instance().GetCurrentCharacterPtr();
		if (!ch)
		{
			sys_err("Null character pointer triggered at %s:%d", __FILE__, __LINE__);
			return 0;
		}

		auto pParty = ch->GetParty();
		if (pParty)
		{
			std::ostringstream s;
			combine_lua_string(L, s);

			FPartyChat f(CHAT_TYPE_TALKING, s.str().c_str());

			pParty->ForEachOnlineMember(f);
		}

		return 0;
	}

	int32_t party_is_map_member_flag_lt(lua_State* L)
	{
		PC* pPC = CQuestManager::instance().GetCurrentPC();
		if (!pPC)
		{
			sys_err("Null pc pointer triggered at %s:%d", __FILE__, __LINE__);
			return 0;
		}

		auto ch = CQuestManager::instance().GetCurrentCharacterPtr();
		if (!ch)
		{
			sys_err("Null character pointer triggered at %s:%d", __FILE__, __LINE__);
			return 0;
		}

		if (!lua_isstring(L, 1) || !lua_isnumber(L, 2))
		{
			lua_pushnumber(L, 0);
			return 1;
		}
		auto sz = lua_tostring(L, 1);

		auto pParty = ch->GetParty();
		if (pParty) 
		{
			FPartyCheckFlagLt f;
			f.flagname = pPC->GetCurrentQuestName() + "." + sz;
			f.value = static_cast<int32_t>(rint(lua_tonumber(L, 2)));

			auto returnBool = pParty->ForEachOnMapMemberBool(f, ch->GetMapIndex());
			lua_pushboolean(L, returnBool);
		}

		return 1;
	}

	int32_t party_set_flag(lua_State* L)
	{
		auto ch = CQuestManager::instance().GetCurrentCharacterPtr();
		if (!ch)
		{
			sys_err("Null character pointer triggered at %s:%d", __FILE__, __LINE__);
			return 0;
		}

		if (ch->GetParty() && lua_isstring(L, 1) && lua_isnumber(L, 2))
		{
			ch->GetParty()->SetFlag(lua_tostring(L, 1), static_cast<int32_t>(lua_tonumber(L, 2)));
		}

		return 0;
	}

	int32_t party_get_flag(lua_State* L)
	{
		auto ch = CQuestManager::instance().GetCurrentCharacterPtr();
		if (!ch)
		{
			sys_err("Null character pointer triggered at %s:%d", __FILE__, __LINE__);
			return 0;
		}

		if (!ch->GetParty() || !lua_isstring(L, 1))
			lua_pushnumber(L, 0);
		else
			lua_pushnumber(L, ch->GetParty()->GetFlag(lua_tostring(L, 1)));

		return 1;
	}

	int32_t party_set_quest_flag(lua_State* L)
	{
		PC* pPC = CQuestManager::instance().GetCurrentPC();
		if (!pPC)
		{
			sys_err("Null pc pointer triggered at %s:%d", __FILE__, __LINE__);
			return 0;
		}

		auto ch = CQuestManager::instance().GetCurrentCharacterPtr();
		if (!ch)
		{
			sys_err("Null character pointer triggered at %s:%d", __FILE__, __LINE__);
			return 0;
		}

		if (!lua_isstring(L, 1) || !lua_isnumber(L, 2))
		{
			lua_pushnumber(L, 0);
			return 1;
		}
		auto flagname = pPC->GetCurrentQuestName() + "." + lua_tostring(L, 1);
		auto value = static_cast<int32_t>(rint(lua_tonumber(L, 2)));

		auto FSetQuestFlag = [&](CHARACTER* member) 
		{
			if (!member || !member->IsPC())
				return;

			auto pPC = CQuestManager::instance().GetPCForce(member->GetPlayerID());
			if (pPC)
				pPC->SetFlag(flagname, value);
		};

		if (ch->GetParty())
			ch->GetParty()->ForEachOnlineMember(FSetQuestFlag);
		else
			FSetQuestFlag(ch);

		return 0;
	}

	int32_t party_is_in_dungeon (lua_State* L)
	{
		auto ch = CQuestManager::instance().GetCurrentCharacterPtr();
		if (!ch)
		{
			sys_err("Null character pointer triggered at %s:%d", __FILE__, __LINE__);
			return 0;
		}

		auto pParty = ch->GetParty();
		if (pParty) 
		{
			lua_pushboolean (L, pParty->GetDungeon() ? true : false);
			return 1;
		}
		lua_pushboolean (L, false);
		return 1;
	}

	int32_t party_give_buff(lua_State* L)
	{
		auto ch = CQuestManager::instance().GetCurrentCharacterPtr();
		if (!ch)
		{
			sys_err("Null character pointer triggered at %s:%d", __FILE__, __LINE__);
			return 0;
		}

		if (!lua_isnumber(L, 1) || !lua_isnumber(L, 2) || !lua_isnumber(L, 3) || !lua_isnumber(L, 4) || !lua_isnumber(L, 5) || !lua_isnumber(L, 6) ||
			!lua_isboolean(L, 7) || !lua_isboolean(L, 8))
		{
			lua_pushboolean(L, false);
			return 1;
		}

		auto dwType = static_cast<uint32_t>(lua_tonumber(L, 1));
		auto bApplyOn = static_cast<uint8_t>(lua_tonumber(L, 2));
		auto lApplyValue = static_cast<int32_t>(lua_tonumber(L, 3));
		auto dwFlag = static_cast<uint32_t>(lua_tonumber(L, 4));
		auto lDuration = static_cast<int32_t>(lua_tonumber(L, 5));
		auto lSPCost = static_cast<int32_t>(lua_tonumber(L, 6));
		auto bOverride = lua_toboolean(L, 7) != 0;
		auto IsCube = lua_toboolean(L, 8) != 0;

		auto FGiveBuff = [&](CHARACTER* member) 
		{
			if (member)
				member->AddAffect(dwType, bApplyOn, lApplyValue, dwFlag, lDuration, lSPCost, bOverride, IsCube);
		};

		if (ch->GetParty())
			ch->GetParty()->ForEachOnMapMember(FGiveBuff, ch->GetMapIndex());
		else
			FGiveBuff(ch);

		lua_pushboolean (L, true);
		return 1;
	}

	struct FPartyPIDCollector
	{
		std::vector <uint32_t> vecPIDs;
		FPartyPIDCollector() {}

		void operator()(CHARACTER* ch)
		{ 
			if (ch)
				vecPIDs.push_back(ch->GetPlayerID()); 
		}
	};

	int32_t party_get_member_pids(lua_State *L)
	{
		auto ch = CQuestManager::instance().GetCurrentCharacterPtr();
		if (!ch)
		{
			sys_err("Null character pointer triggered at %s:%d", __FILE__, __LINE__);
			return 0;
		}

		auto pParty = ch->GetParty();
		if (!pParty) 
		{
			return 0;
		}

		FPartyPIDCollector f;
		pParty->ForEachOnMapMember(f, ch->GetMapIndex());

		for (auto& vecPID : f.vecPIDs) 
		{
			lua_pushnumber(L, vecPID);
		}
		return f.vecPIDs.size();
	}

	struct FPartyVIDCollector 
	{
		std::vector<uint32_t> vecVIDs;
		FPartyVIDCollector() {}

		void operator()(CHARACTER* ch) 
		{ 
			if (ch)
				vecVIDs.push_back(ch->GetVID());
		}
	};

	int32_t party_get_member_vids(lua_State* L)
	{
		auto ch = CQuestManager::instance().GetCurrentCharacterPtr();
		if (!ch)
		{
			sys_err("Null character pointer triggered at %s:%d", __FILE__, __LINE__);
			return 0;
		}

		auto pParty = ch->GetParty();
		if (!pParty)
		{
			return 0;
		}

		FPartyVIDCollector f;
		pParty->ForEachOnMapMember(f, ch->GetMapIndex());

		for (auto& vecVID : f.vecVIDs) 
		{
			lua_pushnumber(L, vecVID);
		}
		return f.vecVIDs.size();
	}


	int32_t party_remove_player(lua_State* L)
	{
		CHARACTER* ch = nullptr;
		if (lua_isnumber(L, 1)) 
		{
			auto pid = static_cast<uint32_t>(lua_tonumber(L, 1));
			ch = CHARACTER_MANAGER::Instance().FindByPID(pid);
		} 
		else
		{
			ch = CQuestManager::instance().GetCurrentCharacterPtr();
		}

		if (!ch)
			return 0;

		auto pParty = ch->GetParty();
		if (!pParty)
			return 0;

		if (pParty->GetMemberCount() == 2 || pParty->GetLeaderPID() == ch->GetPlayerID())
		{
			CPartyManager::instance().DeleteParty(pParty);
		}
		else 
		{
			ch->ChatPacket(CHAT_TYPE_INFO, "<파티> 파티에서 추방당하셨습니다.");
			pParty->Quit(ch->GetPlayerID());
		}

		return 1;
	}

	void RegisterPartyFunctionTable()
	{
		luaL_reg party_functions[] = 
		{
			{ "is_leader",		party_is_leader		},
			{ "is_party",		party_is_party		},
			{ "get_leader_pid",	party_get_leader_pid},
			{ "setf",			party_set_flag		},
			{ "getf",			party_get_flag		},
			{ "setqf",			party_set_quest_flag},
			{ "chat",			party_chat			},
			{ "syschat",		party_syschat		},
			{ "get_near_count",	party_get_near_count},
			{ "show_cinematic",	party_show_cinematic},
			{ "run_cinematic",	party_run_cinematic	},
			{ "get_max_level",	party_get_max_level	},
			{ "clear_ready",	party_clear_ready	},
			{ "is_in_dungeon",	party_is_in_dungeon	},
			{ "give_buff",		party_give_buff		},
			{ "is_map_member_flag_lt",	party_is_map_member_flag_lt	},
			{ "get_member_pids",		party_get_member_pids	}, // 파티원들의 pid를 return
			{ "get_member_vids",		party_get_member_vids		},
			{ "remove_player",			party_remove_player			},
			{ "get_min_level",			party_get_min_level			},
			{ "leave_party",			party_leave_party			},
			{ "delete_party",			party_delete_party			},
			{ "get_members_count",		party_get_members_count		},

			{ nullptr,				nullptr				}
		};

		CQuestManager::instance().AddLuaFunctionTable("party", party_functions);
	}
}




