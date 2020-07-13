#include "stdafx.h"
#include "questlua.h"
#include "quest_manager.h"
#include "desc_client.h"
#include "char.h"
#include "char_manager.h"
#include "utils.h"
#include "guild.h"
#include "guild_manager.h"

#undef sys_err
#ifndef __WIN32__
#define sys_err(fmt, args...) quest::CQuestManager::Instance().QuestError(__FUNCTION__, __LINE__, fmt, ##args)
#else
#define sys_err(fmt, ...) quest::CQuestManager::Instance().QuestError(__FUNCTION__, __LINE__, fmt, __VA_ARGS__)
#endif

namespace quest
{
	//
	// "guild" Lua functions
	//
	int32_t guild_around_ranking_string(lua_State* L)
	{
		CQuestManager& q = CQuestManager::Instance();
		LPCHARACTER ch = q.GetCurrentCharacterPtr();
		if (!ch)
		{
			sys_err("Null character pointer triggered at %s:%d", __FILE__, __LINE__);
			return 0;
		}

		if (!ch->GetGuild())
		{
			lua_pushstring(L, "");
			return 1;
		}
		
		char szBuf[4096+1];
		CGuildManager::Instance().GetAroundRankString(ch->GetGuild()->GetID(), szBuf, sizeof(szBuf));
		
		lua_pushstring(L, szBuf);
		return 1;
	}

	int32_t guild_high_ranking_string(lua_State* L)
	{
		CQuestManager& q = CQuestManager::Instance();
		LPCHARACTER ch = q.GetCurrentCharacterPtr();
		if (!ch)
		{
			sys_err("Null character pointer triggered at %s:%d", __FILE__, __LINE__);
			return 0;
		}
		uint32_t dwMyGuild = 0;
		if (ch->GetGuild())
			dwMyGuild = ch->GetGuild()->GetID();

		char szBuf[4096+1];
		CGuildManager::Instance().GetHighRankString(dwMyGuild, szBuf, sizeof(szBuf));
		lua_pushstring(L, szBuf);
		return 1;
	}

	int32_t guild_get_ladder_point(lua_State* L)
	{
		CQuestManager& q = CQuestManager::Instance();
		LPCHARACTER ch = q.GetCurrentCharacterPtr();
		if (!ch)
		{
			sys_err("Null character pointer triggered at %s:%d", __FILE__, __LINE__);
			return 0;
		}
		if (!ch->GetGuild())
		{
			lua_pushnumber(L, -1);
		}
		else
		{
			lua_pushnumber(L, ch->GetGuild()->GetLadderPoint());
		}
		return 1;
	}

	int32_t guild_get_rank(lua_State* L)
	{
		CQuestManager& q = CQuestManager::Instance();
		LPCHARACTER ch = q.GetCurrentCharacterPtr();
		if (!ch)
		{
			sys_err("Null character pointer triggered at %s:%d", __FILE__, __LINE__);
			return 0;
		}

		if (!ch->GetGuild())
		{
			lua_pushnumber(L, -1);
		}
		else
		{
			lua_pushnumber(L, CGuildManager::Instance().GetRank(ch->GetGuild()));
		}
		return 1;
	}

	int32_t guild_is_war(lua_State* L)
	{
		if (!lua_isnumber(L, 1))
		{
			sys_err("invalid argument");
			return 0;
		}

		LPCHARACTER ch = CQuestManager::Instance().GetCurrentCharacterPtr();
		if (!ch)
		{
			sys_err("Null character pointer triggered at %s:%d", __FILE__, __LINE__);
			return 0;
		}

		if (ch->GetGuild() && ch->GetGuild()->UnderWar((uint32_t) lua_tonumber(L, 1)))
			lua_pushboolean(L, true);
		else
			lua_pushboolean(L, false);

		return 1;
	}

	int32_t guild_name(lua_State* L)
	{
		if (!lua_isnumber(L, 1))
		{
			sys_err("invalid argument");
			return 0;
		}

		CGuild * pkGuild = CGuildManager::Instance().FindGuild((uint32_t) lua_tonumber(L, 1));

		if (pkGuild)
			lua_pushstring(L, pkGuild->GetName());
		else
			lua_pushstring(L, "");

		return 1;
	}

	int32_t guild_level(lua_State* L)
	{
		if (!lua_isnumber(L, 1))
		{
			sys_err("invalid argument");
			return 0;
		}

		CGuild * pkGuild = CGuildManager::Instance().FindGuild((uint32_t) lua_tonumber(L, 1));

		if (pkGuild)
			lua_pushnumber(L, pkGuild->GetLevel());
		else
			lua_pushnumber(L, 0);

		return 1;
	}

	int32_t guild_war_enter(lua_State* L)
	{
		if (!lua_isnumber(L, 1))
		{
			sys_err("invalid argument");
			return 0;
		}

		CQuestManager& q = CQuestManager::Instance();
		LPCHARACTER ch = q.GetCurrentCharacterPtr();
		if (!ch)
		{
			sys_err("Null character pointer triggered at %s:%d", __FILE__, __LINE__);
			return 0;
		}

		if (ch->GetGuild())
			ch->GetGuild()->GuildWarEntryAccept((uint32_t) lua_tonumber(L, 1), ch);

		return 0;
	}

	int32_t guild_get_any_war(lua_State* L)
	{
		LPCHARACTER ch = CQuestManager::Instance().GetCurrentCharacterPtr();
		if (!ch)
		{
			sys_err("Null character pointer triggered at %s:%d", __FILE__, __LINE__);
			return 0;
		}

		if (ch->GetGuild())
			lua_pushnumber(L, ch->GetGuild()->UnderAnyWar());
		else
			lua_pushnumber(L, 0);

		return 1;
	}

	int32_t guild_get_name(lua_State * L)
	{
		if (!lua_isnumber(L, 1))
		{
			lua_pushstring(L,  "");
			return 1;
		}

		CGuild * pkGuild = CGuildManager::Instance().FindGuild((uint32_t) lua_tonumber(L, 1));

		if (pkGuild)
			lua_pushstring(L, pkGuild->GetName());
		else
			lua_pushstring(L, "");

		return 1;
	}

	int32_t guild_get_warp_war_list(lua_State* L)
	{
		FBuildLuaGuildWarList f(L);
		CGuildManager::Instance().for_each_war(f);

		return 1;
	}

	int32_t guild_get_member_count(lua_State* L)
	{
		LPCHARACTER ch = CQuestManager::Instance().GetCurrentCharacterPtr();
		
		if ( ch == nullptr )
		{
			lua_pushnumber(L, 0);
			return 1;
		}

		CGuild* pGuild = ch->GetGuild();

		if ( pGuild == nullptr )
		{
			lua_pushnumber(L, 0);
			return 1;
		}

		lua_pushnumber(L, pGuild->GetMemberCount());

		return 1;
	}

	int32_t guild_change_master(lua_State* L)
	{
		// 리턴값
		//	0 : 입력한 이름이 잘못되었음 ( 문자열이 아님 )
		//	1 : 길드장이 아님
		//	2 : 지정한 이름의 길드원이 없음
		//	3 : 요청 성공
		//	4 : 길드가 없음

		LPCHARACTER ch = CQuestManager::Instance().GetCurrentCharacterPtr();
		if (!ch)
		{
			sys_err("Null character pointer triggered at %s:%d", __FILE__, __LINE__);
			return 0;
		}

		CGuild* pGuild = ch->GetGuild();
		if (pGuild)
		{
			if (pGuild->GetMasterPID() == ch->GetPlayerID())
			{
				if (!lua_isstring(L, 1))
				{
					lua_pushnumber(L, 0);
				}
				else
				{
					bool ret = pGuild->ChangeMasterTo(pGuild->GetMemberPID(lua_tostring(L, 1)));

					lua_pushnumber(L, ret == false ? 2 : 3 );
				}
			}
			else
			{
				lua_pushnumber(L, 1);
			}
		}
		else
		{
			lua_pushnumber(L, 4);
		}

		return 1;
	}

	int32_t guild_change_master_with_limit(lua_State* L)
	{
		// 인자
		//  arg0 : 새 길드장 이름
		//  arg1 : 새 길드장 레벨 제한
		//  arg2 : resign_limit 제한 시간
		//  arg3 : be_other_leader 제한 시간
		//  arg4 : be_other_member 제한 시간
		//  arg5 : 캐시템인가 아닌가
		//
		// 리턴값
		//	0 : 입력한 이름이 잘못되었음 ( 문자열이 아님 )
		//	1 : 길드장이 아님
		//	2 : 지정한 이름의 길드원이 없음
		//	3 : 요청 성공
		//	4 : 길드가 없음
		//	5 : 지정한 이름이 온라인이 아님
		//	6 : 지정한 캐릭터 레벨이 기준레벨보다 낮음
		//	7 : 새 길드장이 be_other_leader 제한에 걸림

		LPCHARACTER ch = CQuestManager::Instance().GetCurrentCharacterPtr();
		if (!ch)
		{
			sys_err("Null character pointer triggered at %s:%d", __FILE__, __LINE__);
			return 0;
		}

		if (!lua_isnumber(L, 2))
		{
			sys_err("wrong arg");
			return 0;
		}

		CGuild* pGuild = ch->GetGuild();
		if (pGuild)
		{
			if (pGuild->GetMasterPID() == ch->GetPlayerID())
			{
				if (!lua_isstring(L, 1))
				{
					lua_pushnumber(L, 0);
				}
				else
				{
					LPCHARACTER pNewMaster = CHARACTER_MANAGER::Instance().FindPC( lua_tostring(L,1) );
					if (pNewMaster)
					{
						if (pNewMaster->GetLevel() < lua_tonumber(L, 2))
						{
							lua_pushnumber(L, 6);
						}
						else
						{
							int32_t nBeOtherLeader = pNewMaster->GetQuestFlag("change_guild_master.be_other_leader");
							CQuestManager::Instance().GetPC( ch->GetPlayerID() );

							if ( lua_toboolean(L, 6) ) 
								nBeOtherLeader = 0;

							if ( nBeOtherLeader > get_global_time() )
							{
								lua_pushnumber(L, 7);
							}
							else
							{
								bool ret = pGuild->ChangeMasterTo(pGuild->GetMemberPID(lua_tostring(L, 1)));

								if ( ret == false )
								{
									lua_pushnumber(L, 2);
								}
								else
								{
									lua_pushnumber(L, 3);

									pNewMaster->SetQuestFlag("change_guild_master.be_other_leader", 0);
									pNewMaster->SetQuestFlag("change_guild_master.be_other_member", 0);
									pNewMaster->SetQuestFlag("change_guild_master.resign_limit", (int32_t)lua_tonumber(L, 3));

									ch->SetQuestFlag("change_guild_master.be_other_leader", (int32_t)lua_tonumber(L, 4));
									ch->SetQuestFlag("change_guild_master.be_other_member", (int32_t)lua_tonumber(L, 5));
									ch->SetQuestFlag("change_guild_master.resign_limit", 0);
								}
							}
						}
					}
					else
					{
						lua_pushnumber(L, 5);
					}
				}
			}
			else
			{
				lua_pushnumber(L, 1);
			}
		}
		else
		{
			lua_pushnumber(L, 4);
		}

		return 1;
	}


	int32_t guild_get_id(lua_State* L)
	{
		CHARACTER* ch = CQuestManager::Instance().GetCurrentCharacterPtr();
		if (!ch) 
		{
			sys_err("Null character pointer triggered at %s:%d", __FILE__, __LINE__);
			return 0;
		}

		CGuild* pGuild = ch->GetGuild();

		lua_pushnumber(L, (pGuild != nullptr) ? pGuild->GetID() : 0);
		return 1;
	}

	int32_t guild_get_sp(lua_State* L)
	{
		CHARACTER* ch = CQuestManager::Instance().GetCurrentCharacterPtr();
		if (!ch)
		{
			sys_err("Null character pointer triggered at %s:%d", __FILE__, __LINE__);
			return 0;
		}

		CGuild* pGuild = ch->GetGuild();

		lua_pushnumber(L, (pGuild != nullptr) ? pGuild->GetSP() : 0);
		return 1;
	}

	int32_t guild_get_maxsp(lua_State* L)
	{
		CHARACTER* ch = CQuestManager::Instance().GetCurrentCharacterPtr();
		if (!ch) 
		{
			sys_err("Null character pointer triggered at %s:%d", __FILE__, __LINE__);
			return 0;
		}

		CGuild* pGuild = ch->GetGuild();

		lua_pushnumber(L, (pGuild != nullptr) ? pGuild->GetMaxSP() : 0);
		return 1;
	}

	int32_t guild_get_money(lua_State* L)
	{
		CHARACTER* ch = CQuestManager::Instance().GetCurrentCharacterPtr();
		if (!ch) 
		{
			sys_err("Null character pointer triggered at %s:%d", __FILE__, __LINE__);
			return 0;
		}

		CGuild* pGuild = ch->GetGuild();

		lua_pushnumber(L, (pGuild != nullptr) ? static_cast<int32_t>(pGuild->GetGuildMoney()) : 0);
		return 1;
	}

	int32_t guild_get_max_member(lua_State* L)
	{
		CHARACTER* ch = CQuestManager::Instance().GetCurrentCharacterPtr();
		if (!ch) 
		{
			sys_err("Null character pointer triggered at %s:%d", __FILE__, __LINE__);
			return 0;
		}

		CGuild* pGuild = ch->GetGuild();

		lua_pushnumber(L, (pGuild != nullptr) ? pGuild->GetMaxMemberCount() : 0);
		return 1;
	}

	int32_t guild_get_total_member_level(lua_State* L)
	{
		CHARACTER* ch = CQuestManager::Instance().GetCurrentCharacterPtr();
		if (!ch)
		{
			sys_err("Null character pointer triggered at %s:%d", __FILE__, __LINE__);
			return 0;
		}

		CGuild* pGuild = ch->GetGuild();

		lua_pushnumber(L, (pGuild != nullptr) ? pGuild->GetTotalLevel() : 0);
		return 1;
	}

	int32_t guild_has_land(lua_State* L)
	{
		CHARACTER* ch = CQuestManager::Instance().GetCurrentCharacterPtr();
		if (!ch)
		{
			sys_err("Null character pointer triggered at %s:%d", __FILE__, __LINE__);
			return 0;
		}

		CGuild* pGuild = ch->GetGuild();

		lua_pushboolean(L, (pGuild != nullptr) ? pGuild->HasLand() : false);
		return 1;
	}

	int32_t guild_get_win_count(lua_State* L)
	{
		CHARACTER* ch = CQuestManager::Instance().GetCurrentCharacterPtr();
		if (!ch) 
		{
			sys_err("Null character pointer triggered at %s:%d", __FILE__, __LINE__);
			return 0;
		}

		CGuild* pGuild = ch->GetGuild();

		lua_pushnumber(L, (pGuild != nullptr) ? pGuild->GetGuildWarWinCount() : 0);
		return 1;
	}

	int32_t guild_get_draw_count(lua_State* L)
	{
		CHARACTER* ch = CQuestManager::Instance().GetCurrentCharacterPtr();
		if (!ch) 
		{
			sys_err("Null character pointer triggered at %s:%d", __FILE__, __LINE__);
			return 0;
		}

		CGuild* pGuild = ch->GetGuild();

		lua_pushnumber(L, (pGuild != nullptr) ? pGuild->GetGuildWarDrawCount() : 0);
		return 1;
	}

	int32_t guild_get_loss_count(lua_State* L)
	{
		CHARACTER* ch = CQuestManager::Instance().GetCurrentCharacterPtr();
		if (!ch)
		{
			sys_err("Null character pointer triggered at %s:%d", __FILE__, __LINE__);
			return 0;
		}

		CGuild* pGuild = ch->GetGuild();

		lua_pushnumber(L, (pGuild != nullptr) ? pGuild->GetGuildWarLossCount() : 0);
		return 1;
	}

	int32_t guild_add_comment(lua_State* L)
	{
		if (!lua_isstring(L, 1))
			return 0;

		CHARACTER* ch = CQuestManager::Instance().GetCurrentCharacterPtr();
		if (!ch) 
		{
			sys_err("Null character pointer triggered at %s:%d", __FILE__, __LINE__);
			return 0;
		}

		CGuild* pGuild = ch->GetGuild();
		if (pGuild)
			pGuild->AddComment(ch, std::string(lua_tostring(L, 1)));

		return 0;
	}

	int32_t guild_set_ladder_point(lua_State* L)
	{
		if (!lua_isnumber(L, 1))
			return 0;

		CHARACTER* ch = CQuestManager::Instance().GetCurrentCharacterPtr();
		if (!ch)
		{
			sys_err("Null character pointer triggered at %s:%d", __FILE__, __LINE__);
			return 0;
		}

		CGuild* pGuild = ch->GetGuild();
		if (pGuild)
			pGuild->ChangeLadderPoint((int32_t)lua_tonumber(L, 1));

		return 0;
	}

	int32_t guild_get_exp_level(lua_State* L)
	{
		lua_pushnumber(L, guild_exp_table2[MINMAX(0, (int32_t)lua_tonumber(L, 1), GUILD_MAX_LEVEL)]);
		return 1;
	}

	int32_t guild_offer_exp(lua_State* L)
	{
		CHARACTER* ch = CQuestManager::Instance().GetCurrentCharacterPtr();
		if (!ch)
		{
			sys_err("Null character pointer triggered at %s:%d", __FILE__, __LINE__);
			return 0;
		}

		CGuild* pGuild = ch->GetGuild();
		if (!pGuild)
		{
			lua_pushboolean(L, false);
			return 1;
		}

		uint32_t offer = (uint32_t)lua_tonumber(L, 1);

		if (pGuild->GetLevel() >= GUILD_MAX_LEVEL)
		{
			lua_pushboolean(L, false);
		}
		else
		{
			offer /= 100;
			offer *= 100;

			if (pGuild->OfferExp(ch, offer))
			{
				lua_pushboolean(L, true);
			}
			else
			{
				lua_pushboolean(L, false);
			}
		}
		return 1;
	}

	int32_t guild_give_exp(lua_State* L)
	{
		if (!lua_isnumber(L, 1))
			return 0;

		CHARACTER* ch = CQuestManager::Instance().GetCurrentCharacterPtr();
		if (!ch) 
		{
			sys_err("Null character pointer triggered at %s:%d", __FILE__, __LINE__);
			return 0;
		}

		CGuild* pGuild = ch->GetGuild();
		if (!pGuild)
			return 0;

		pGuild->GuildPointChange(POINT_EXP, (int32_t)lua_tonumber(L, 1) / 100, true);
		return 0;
	}

	int32_t guild_force_war(lua_State* L)
	{
		if (!lua_isstring(L, 1) || !lua_isstring(L, 2))
			return 0;

		CHARACTER* lpGuildOwn1 = CHARACTER_MANAGER::Instance().FindPC(lua_tostring(L, 1));
		CHARACTER* lpGuildOwn2 = CHARACTER_MANAGER::Instance().FindPC(lua_tostring(L, 2));
		if (lpGuildOwn1 || lpGuildOwn2)
			return 0;

		CGuild* lpGuild1 = lpGuildOwn1->GetGuild();
		CGuild* lpGuild2 = lpGuildOwn2->GetGuild();
		if (lpGuild1 && lpGuild2) 
		{
			lpGuild1->StartWar(lpGuild2->GetID());
			lpGuild2->StartWar(lpGuild1->GetID());

			char szNoticeBuf[500];
			sprintf(szNoticeBuf, "%s ve %s arasinda savas basladi", lpGuild1->GetName(), lpGuild2->GetName());
			SendNotice(szNoticeBuf);
		}
		else 
		{
			sys_err("Forcewar error guild pointers are null");
		}

		return 0;
	}


	void RegisterGuildFunctionTable()
	{
		luaL_reg guild_functions[] =
		{
			{ "get_rank",					guild_get_rank					},
			{ "get_ladder_point",			guild_get_ladder_point			},
			{ "high_ranking_string",		guild_high_ranking_string		},
			{ "around_ranking_string",		guild_around_ranking_string		},
			{ "name",						guild_name						},
			{ "level",						guild_level						},
			{ "is_war",						guild_is_war					},
			{ "war_enter",					guild_war_enter					},
			{ "get_any_war",				guild_get_any_war				},
			{ "get_name",					guild_get_name					},
			{ "get_warp_war_list",			guild_get_warp_war_list			},
			{ "get_member_count",			guild_get_member_count			},
			{ "change_master",				guild_change_master				},
			{ "change_master_with_limit",	guild_change_master_with_limit	},

			{ "get_id",						guild_get_id					},
			{ "get_sp",						guild_get_sp					},
			{ "get_maxsp",					guild_get_maxsp					},
			{ "get_money",					guild_get_money					},
			{ "get_max_member",				guild_get_max_member			},
			{ "get_total_member_level",		guild_get_total_member_level	},
			{ "has_land",					guild_has_land					},
			{ "get_win_count",				guild_get_win_count				},
			{ "get_draw_count",				guild_get_draw_count			},
			{ "get_loss_count",				guild_get_loss_count			},
			{ "add_comment",				guild_add_comment				},
			{ "set_ladder_point",			guild_set_ladder_point			},
			{ "get_exp_level",              guild_get_exp_level				},
			{ "offer_exp",                  guild_offer_exp					},
			{ "give_exp",               	guild_give_exp					},
			{ "force_war",               	guild_force_war					},

			{ nullptr,						nullptr						}
		};

		CQuestManager::Instance().AddLuaFunctionTable("guild", guild_functions);
	}
}

