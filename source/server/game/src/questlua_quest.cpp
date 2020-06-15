#include "stdafx.h"
#include "questlua.h"
#include "quest_manager.h"
#include "char.h"

#undef sys_err
#ifndef __WIN32__
#define sys_err(fmt, args...) quest::CQuestManager::instance().QuestError(__FUNCTION__, __LINE__, fmt, ##args)
#else
#define sys_err(fmt, ...) quest::CQuestManager::instance().QuestError(__FUNCTION__, __LINE__, fmt, __VA_ARGS__)
#endif

namespace quest
{
	//
	// "quest" Lua functions
	//
	int32_t quest_start(lua_State* L)
	{
		PC* pPC = CQuestManager::instance().GetCurrentPC();
		if (!pPC)
		{
			sys_err("Null pc pointer triggered at %s:%d", __FILE__, __LINE__);
			return 0;
		}

		//q.GetPC(q.GetCurrentCharacterPtr()->GetPlayerID())->SetCurrentQuestStartFlag();
		pPC->SetCurrentQuestStartFlag();
		return 0;
	}

	int32_t quest_done(lua_State* L)
	{
		PC* pPC = CQuestManager::instance().GetCurrentPC();
		if (!pPC)
		{
			sys_err("Null pc pointer triggered at %s:%d", __FILE__, __LINE__);
			return 0;
		}

		pPC->SetCurrentQuestDoneFlag();
		//q.GetPC(q.GetCurrentCharacterPtr()->GetPlayerID())->SetCurrentQuestDoneFlag();
		return 0;
	}

	int32_t quest_set_title(lua_State* L)
	{
		PC* pPC = CQuestManager::instance().GetCurrentPC();
		if (!pPC)
		{
			sys_err("Null pc pointer triggered at %s:%d", __FILE__, __LINE__);
			return 0;
		}

		//q.GetPC(q.GetCurrentCharacterPtr()->GetPlayerID())->SetCurrentQuestTitle(lua_tostring(L,-1));
		if (lua_isstring(L,-1))
			pPC->SetCurrentQuestTitle(lua_tostring(L,-1));

		return 0;
	}

	int32_t quest_set_another_title(lua_State* L)
	{
		PC* pPC = CQuestManager::instance().GetCurrentPC();
		if (!pPC)
		{
			sys_err("Null pc pointer triggered at %s:%d", __FILE__, __LINE__);
			return 0;
		}

		if (lua_isstring(L,1) && lua_isstring(L,2))
			pPC->SetQuestTitle(lua_tostring(L,1),lua_tostring(L,2));

		return 0;
	}

	int32_t quest_set_clock_name(lua_State* L)
	{
		PC* pPC = CQuestManager::instance().GetCurrentPC();
		if (!pPC)
		{
			sys_err("Null pc pointer triggered at %s:%d", __FILE__, __LINE__);
			return 0;
		}

		//q.GetPC(q.GetCurrentCharacterPtr()->GetPlayerID())->SetCurrentQuestClockName(lua_tostring(L,-1));
		if (lua_isstring(L,-1))
			pPC->SetCurrentQuestClockName(lua_tostring(L,-1));

		return 0;
	}

	int32_t quest_set_clock_value(lua_State* L)
	{
		PC* pPC = CQuestManager::instance().GetCurrentPC();
		if (!pPC)
		{
			sys_err("Null pc pointer triggered at %s:%d", __FILE__, __LINE__);
			return 0;
		}

		//q.GetPC(q.GetCurrentCharacterPtr()->GetPlayerID())->SetCurrentQuestClockValue((int32_t)rint(lua_tonumber(L,-1)));
		if (lua_isnumber(L,-1))
			pPC->SetCurrentQuestClockValue((int32_t)rint(lua_tonumber(L,-1)));

		return 0;
	}

	int32_t quest_set_counter_name(lua_State* L)
	{
		PC* pPC = CQuestManager::instance().GetCurrentPC();
		if (!pPC)
		{
			sys_err("Null pc pointer triggered at %s:%d", __FILE__, __LINE__);
			return 0;
		}

		//q.GetPC(q.GetCurrentCharacterPtr()->GetPlayerID())->SetCurrentQuestCounterName(lua_tostring(L,-1));
		if (lua_isstring(L,-1))
			pPC->SetCurrentQuestCounterName(lua_tostring(L,-1));

		return 0;
	}

	int32_t quest_set_counter_value(lua_State* L)
	{
		PC* pPC = CQuestManager::instance().GetCurrentPC();
		if (!pPC)
		{
			sys_err("Null pc pointer triggered at %s:%d", __FILE__, __LINE__);
			return 0;
		}

		//q.GetPC(q.GetCurrentCharacterPtr()->GetPlayerID())->SetCurrentQuestCounterValue((int32_t)rint(lua_tonumber(L,-1)));
		if (lua_isnumber(L,-1))
			pPC->SetCurrentQuestCounterValue((int32_t)rint(lua_tonumber(L,-1)));

		return 0;
	}

	int32_t quest_set_icon_file(lua_State* L)
	{
		PC* pPC = CQuestManager::instance().GetCurrentPC();
		if (!pPC)
		{
			sys_err("Null pc pointer triggered at %s:%d", __FILE__, __LINE__);
			return 0;
		}

		//q.GetPC(q.GetCurrentCharacterPtr()->GetPlayerID())->SetCurrentQuestCounterValue((int32_t)rint(lua_tonumber(L,-1)));
		if (lua_isstring(L,-1))
			pPC->SetCurrentQuestIconFile(lua_tostring(L,-1));

		return 0;
	}

	int32_t quest_setstate(lua_State* L)
	{
		if (!lua_isstring(L, -1))
		{
			sys_err("state name is empty");
			return 0;
		}

		CQuestManager& q = CQuestManager::instance();
		QuestState * pqs = q.GetCurrentState();
		PC* pPC = q.GetCurrentPC();
		if (!pPC)
		{
			sys_err("Null pc pointer triggered at %s:%d", __FILE__, __LINE__);
			return 0;
		}

		if (!pqs || L != pqs->co) 
		{
			luaL_error(L, "running thread != current thread???");
			sys_err("running thread != current thread??? %s:%d", __FILE__, __LINE__);
			return 0;
		}

		std::string stCurrentState = lua_tostring(L,-1);
		if ( g_bIsTestServer )
			sys_log ( 0 ,"questlua->setstate( %s, %s )", pPC->GetCurrentQuestName().c_str(), stCurrentState.c_str() );

		pqs->st = q.GetQuestStateIndex(pPC->GetCurrentQuestName(), stCurrentState);
		pPC->SetCurrentQuestStateName(stCurrentState );

		return 0;
	}

	int32_t quest_coroutine_yield(lua_State * L)
	{
		CQuestManager& q = CQuestManager::instance();
		// other_pc_block 내부에서는 yield가 일어나서는 안된다. 절대로.
		if (q.IsInOtherPCBlock())
		{
			sys_err("[FATAL ERROR] Yield occur in other_pc_block. Quest: '%s'", (q.GetCurrentQuestName().size() > 0) ? q.GetCurrentQuestName().c_str() : "NULL");

			PC* pPC = q.GetOtherPCBlockRootPC();
			if (!pPC)
			{
				sys_err("[FATAL ERROR] RootPC is nullptr");
				return 0;
			}

			QuestState* pQS = pPC->GetRunningQuestState();
			if (!pQS)
			{
				sys_err("[FATAL ERROR] Quest state is nullptr PC Quest '%s'", (pPC->GetCurrentQuestName().size() > 0) ? pPC->GetCurrentQuestName().c_str() : "NULL");
				return 0;
			}

			std::string questStateName = q.GetQuestStateName(pPC->GetCurrentQuestName(), pQS->st);
			if (!questStateName.size())
			{
				sys_err("[FATAL ERROR] Unknown quest state name State Quest '%s' State: %d", (pQS->questName.size() > 0) ? pQS->questName.c_str() : "NULL", pQS->st);
				return 0;
			}
			
			sys_err("[FATAL ERROR] Current Quest(%s). State(%s)", pPC->GetCurrentQuestName().c_str(), questStateName.c_str());
			return 0;
		}
		return lua_yield(L, lua_gettop(L));
	}

	int32_t quest_no_send(lua_State* L)
	{
		CQuestManager& q = CQuestManager::instance();
		q.SetNoSend();
		return 0;
	}

	int32_t quest_get_current_quest_index(lua_State* L)
	{
		CQuestManager& q = CQuestManager::instance();
		PC* pPC = q.GetCurrentPC();
		if (!pPC)
		{
			sys_err("Null pc pointer triggered at %s:%d", __FILE__, __LINE__);
			return 0;
		}

		int32_t idx = q.GetQuestIndexByName(pPC->GetCurrentQuestName());
		lua_pushnumber(L, idx);
		return 1;
	}

	int32_t quest_begin_other_pc_block(lua_State* L)
	{
		if (!lua_isnumber(L, -1))
		{
			sys_err("wrong arg");
			return 0;
		}
		uint32_t pid = (uint32_t)lua_tonumber(L, -1);

		CQuestManager& q = CQuestManager::instance();
		q.BeginOtherPCBlock(pid);
		return 0;
	}

	int32_t quest_end_other_pc_block(lua_State* L)
	{
		CQuestManager& q = CQuestManager::instance();
		q.EndOtherPCBlock();
		return 0;
	}

	int32_t quest_get_state_index(lua_State* L)
	{
		if (!lua_isstring(L, 1) || !lua_isstring(L, 2))
		{
			sys_err("wrong get state index flag");
		}

		lua_pushnumber(L, CQuestManager::Instance().GetQuestStateIndex(lua_tostring(L, 1), lua_tostring(L, 2)));
		return 1;
	}

	int32_t quest_get_command(lua_State* L)
	{
		CQuestManager& q = CQuestManager::instance();
		PC* pPC = q.GetCurrentPC();
		if (!pPC)
		{
			sys_err("Null pc pointer triggered at %s:%d", __FILE__, __LINE__);
			return 0;
		}

		lua_pushstring(L, pPC->GetCommand().c_str());
		return 1;
	}


	int32_t quest_start_other_quest(lua_State* L)
	{
		if (!lua_isstring(L, 1))
		{
			lua_pushboolean(L, false);
			return 1;
		}
		
		CQuestManager& q = CQuestManager::instance();
		LPCHARACTER ch = q.GetCurrentCharacterPtr();		
		if (!ch )
		{
			sys_err("no character instance found");

			lua_pushboolean(L, false);		
			return 1;
		}
		
		uint32_t qidx = q.GetQuestIndexByName(lua_tostring(L, 1));	
		if (qidx == 0)
		{
			lua_pushboolean(L, false);
			return 1;
		}
		
		//setting qf to verify its triggered by a quest
		ch->SetQuestFlag("quest_call.called_by_quest", 1);
		q.QuestButton(ch->GetPlayerID(), qidx);
		ch->SetQuestFlag("quest_call.called_by_quest", 0);
		
		lua_pushboolean(L, true);		
		return 1;
	}
	
	int32_t quest_get_current_quest_name(lua_State* L)
	{
		PC* pc = CQuestManager::instance().GetCurrentPC();

		lua_pushstring(L, pc ? pc->GetCurrentQuestName().c_str() : "");
		return 1;
	}
	
	void RegisterQuestFunctionTable()
	{
		luaL_reg quest_functions[] = 
		{
			{ "setstate",				quest_setstate				},
			{ "set_state",				quest_setstate				},
			{ "yield",					quest_coroutine_yield		},
			{ "set_title",				quest_set_title				},
			{ "set_title2",				quest_set_another_title		},
			{ "set_clock_name",			quest_set_clock_name		},
			{ "set_clock_value",		quest_set_clock_value		},
			{ "set_counter_name",		quest_set_counter_name		},
			{ "set_counter_value",		quest_set_counter_value		},
			{ "set_icon",				quest_set_icon_file			},
			{ "start",					quest_start					},
			{ "done",					quest_done					},
			{ "getcurrentquestindex",	quest_get_current_quest_index	},
			{ "no_send",				quest_no_send				},
			// begin_other_pc_block(pid), end_other_pc_block 사이를 other_pc_block이라고 하자.
			// other_pc_block에서는 current_pc가 pid로 변경된다.
			//						끝나면 다시 원래의 current_pc로 돌아간다.
			/*		이런 것을 위해 만듬.
					for i, pid in next, pids, nil do
						q.begin_other_pc_block(pid)
						if pc.count_item(PASS_TICKET) < 1 then
							table.insert(criminalNames, pc.get_name())
							canPass = false
						end
						q.end_other_pc_block()
					end
			*/
			// 주의 : other_pc_block 내부에서는 절대로 yield가 일어나서는 안된다.(ex. wait, select, input, ...)
			{ "begin_other_pc_block",	quest_begin_other_pc_block	}, 
			{ "end_other_pc_block",		quest_end_other_pc_block	},

			{ "get_command",			quest_get_command				},
			{ "get_command",			quest_get_command				},
			{ "start_other_quest",		quest_start_other_quest			},
			{ "get_state_index",		quest_get_state_index			},

			{ "get_current_quest_name", quest_get_current_quest_name	},

			{ nullptr,						nullptr						}
		};

		CQuestManager::instance().AddLuaFunctionTable("q", quest_functions);
	}
}




