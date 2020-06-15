#include "stdafx.h"
#include "utils.h"
#include "config.h"
#include "quest_manager.h"
#include "char.h"
#include "party.h"
#include "xmas_event.h"
#include "char_manager.h"
#include "shop_manager.h"
#include "guild.h"
#include "sectree_manager.h"
#include "desc.h"

namespace quest
{
	//
	// "npc" lua functions
	//
	int32_t npc_open_shop(lua_State * L)
	{
		int32_t iShopVnum = 0;

		if (lua_gettop(L) == 1)
		{
			if (lua_isnumber(L, 1))
				iShopVnum = (int32_t) lua_tonumber(L, 1);
		}

		if (CQuestManager::instance().GetCurrentNPCCharacterPtr())
			CShopManager::instance().StartShopping(CQuestManager::instance().GetCurrentCharacterPtr(), CQuestManager::instance().GetCurrentNPCCharacterPtr(), iShopVnum);
		return 0;
	}


	int32_t npc_get_name(lua_State* L)
	{
		CQuestManager& q = CQuestManager::instance();
		LPCHARACTER npc = q.GetCurrentNPCCharacterPtr();
		if(npc)
			lua_pushstring(L, npc->GetName());
		else 
			lua_pushstring(L, "");

		return 1;
	}

	int32_t npc_is_pc(lua_State* L)
	{
		CQuestManager& q = CQuestManager::instance();
		LPCHARACTER npc = q.GetCurrentNPCCharacterPtr();
		if (npc && npc->IsPC())
			lua_pushboolean(L, true);
		else
			lua_pushboolean(L, false);

		return 1;
	}

	int32_t npc_get_empire(lua_State * L)
	{
		CQuestManager& q = CQuestManager::instance();
		LPCHARACTER npc = q.GetCurrentNPCCharacterPtr();
		if (npc)
			lua_pushnumber(L, npc->GetEmpire());
		else
			lua_pushnumber(L, 0);
		return 1;
	}

	int32_t npc_get_race(lua_State * L)
	{
		lua_pushnumber(L, CQuestManager::instance().GetCurrentNPCRace());
		return 1;
	}

	int32_t npc_get_guild(lua_State* L)
	{
		CQuestManager& q = CQuestManager::instance();
		LPCHARACTER npc = q.GetCurrentNPCCharacterPtr();
		CGuild* pGuild = nullptr;

		if (npc)
			pGuild = npc->GetGuild();

		lua_pushnumber(L, pGuild ? pGuild->GetID() : 0);
		return 1;
	}

	int32_t npc_get_remain_skill_book_count(lua_State* L)
	{
		LPCHARACTER npc = CQuestManager::instance().GetCurrentNPCCharacterPtr();
		if (!npc || npc->IsPC())
		{
			lua_pushnumber(L, 0);
			return 1;
		}

		lua_pushnumber(L, std::max<int32_t>(0, static_cast<int32_t>(npc->GetPoint(POINT_ATT_GRADE_BONUS))));
		return 1;
	}

	int32_t npc_dec_remain_skill_book_count(lua_State* L)
	{
		LPCHARACTER npc = CQuestManager::instance().GetCurrentNPCCharacterPtr();
		if (!npc || npc->IsPC())
		{
			return 0;
		}

		npc->SetPoint(POINT_ATT_GRADE_BONUS, std::max<int64_t>(0, npc->GetPoint(POINT_ATT_GRADE_BONUS)-1));
		return 0;
	}

	int32_t npc_get_remain_hairdye_count(lua_State* L)
	{
		LPCHARACTER npc = CQuestManager::instance().GetCurrentNPCCharacterPtr();
		if (!npc || npc->IsPC())
		{
			lua_pushnumber(L, 0);
			return 1;
		}

		lua_pushnumber(L, std::max<int32_t>(0, static_cast<int32_t>(npc->GetPoint(POINT_DEF_GRADE_BONUS))));
		return 1;
	}

	int32_t npc_dec_remain_hairdye_count(lua_State* L)
	{
		LPCHARACTER npc = CQuestManager::instance().GetCurrentNPCCharacterPtr();
		if (!npc || npc->IsPC())
		{
			return 0;
		}

		npc->SetPoint(POINT_DEF_GRADE_BONUS, std::max<int64_t>(0, npc->GetPoint(POINT_DEF_GRADE_BONUS)-1));
		return 0;
	}

	int32_t npc_is_quest(lua_State * L)
	{
		CQuestManager& q = CQuestManager::instance();
		LPCHARACTER npc = q.GetCurrentNPCCharacterPtr();

		if (npc)
		{
			const std::string & r_st = q.GetCurrentQuestName();

			if (q.GetQuestIndexByName(r_st) == npc->GetQuestBy())
			{
				lua_pushboolean(L, true);
				return 1;
			}
		}

		lua_pushboolean(L, false);
		return 1;
	}

	int32_t npc_kill(lua_State * L)
	{
		CQuestManager& q = CQuestManager::instance();
		LPCHARACTER ch = q.GetCurrentCharacterPtr();
		LPCHARACTER npc = q.GetCurrentNPCCharacterPtr();

		if (!ch) 
		{
			sys_err("Null character pointer triggered at %s:%d", __FILE__, __LINE__);
			return 0;
		}
		ch->SetQuestNPCID(0);
		if (npc)
		{
			npc->Dead();
		}
		return 0;
	}

	int32_t npc_purge(lua_State * L)
	{
		CQuestManager& q = CQuestManager::instance();
		LPCHARACTER ch = q.GetCurrentCharacterPtr();
		LPCHARACTER npc = q.GetCurrentNPCCharacterPtr();

		if (!ch) 
		{
			sys_err("Null character pointer triggered at %s:%d", __FILE__, __LINE__);
			return 0;
		}
		ch->SetQuestNPCID(0);
		if (npc)
		{
			M2_DESTROY_CHARACTER(npc);
		}
		return 0;
	}

	int32_t npc_is_near(lua_State * L)
	{
		CQuestManager& q = CQuestManager::instance();
		LPCHARACTER ch = q.GetCurrentCharacterPtr();
		LPCHARACTER npc = q.GetCurrentNPCCharacterPtr();

		lua_Number dist = 10;

		if (lua_isnumber(L, 1))
			dist = lua_tonumber(L, 1);

		if (ch == nullptr || npc == nullptr)
		{
			lua_pushboolean(L, false);
		}
		else
		{
			lua_pushboolean(L, (DISTANCE_APPROX(ch->GetX() - npc->GetX(), ch->GetY() - npc->GetY()) < dist * 100));
		}

		return 1;
	}

	int32_t npc_is_near_vid(lua_State* L)
	{
		if (!lua_isnumber(L, 1))
		{
			sys_err("invalid vid");
			lua_pushboolean(L, false);
			return 1;
		}

		CQuestManager& q = CQuestManager::instance();
		LPCHARACTER ch = CHARACTER_MANAGER::instance().Find((uint32_t)lua_tonumber(L, 1));
		LPCHARACTER npc = q.GetCurrentNPCCharacterPtr();

		lua_Number dist = 10;

		if (lua_isnumber(L, 2))
			dist = lua_tonumber(L, 2);

		if (ch == nullptr || npc == nullptr)
		{
			lua_pushboolean(L, false);
		}
		else
		{
			lua_pushboolean(L, DISTANCE_APPROX(ch->GetX() - npc->GetX(), ch->GetY() - npc->GetY()) < dist*100);
		}

		return 1;
	}

	int32_t npc_unlock(lua_State* L)
	{
		CQuestManager& q = CQuestManager::instance();
		LPCHARACTER ch = q.GetCurrentCharacterPtr();
		LPCHARACTER npc = q.GetCurrentNPCCharacterPtr();
		if (!ch)
		{
			sys_err("Null character pointer triggered at %s:%d", __FILE__, __LINE__);
			return 0;
		}

		if ( npc != nullptr )
		{
			if (npc->IsPC())
				return 0;

			if (npc->GetQuestNPCID() == ch->GetPlayerID())
			{
				npc->SetQuestNPCID(0);
			}
		}
		return 0;
	}

	int32_t npc_lock(lua_State* L)
	{
		CQuestManager& q = CQuestManager::instance();
		LPCHARACTER ch = q.GetCurrentCharacterPtr();
		LPCHARACTER npc = q.GetCurrentNPCCharacterPtr();

		if (!ch)
		{
			sys_err("Null character pointer triggered at %s:%d", __FILE__, __LINE__);
			return 0;
		}

		if (!npc || npc->IsPC())
		{
			lua_pushboolean(L, true);
			return 1;
		}

		if (npc->GetQuestNPCID() == 0 || npc->GetQuestNPCID() == ch->GetPlayerID())
		{
			npc->SetQuestNPCID(ch->GetPlayerID());
			lua_pushboolean(L, true);
		}
		else
		{
			lua_pushboolean(L, false);
		}

		return 1;
	}

	int32_t npc_get_leader_vid(lua_State* L)
	{
		CQuestManager& q = CQuestManager::instance();
		LPCHARACTER npc = q.GetCurrentNPCCharacterPtr();

		if (!npc)
		{
			sys_err("Null npc pointer triggered at %s:%d", __FILE__, __LINE__);
			return 0;
		}

		LPPARTY party = npc->GetParty();
		if (!party)
		{
			sys_err("Null party pointer triggered at %s:%d", __FILE__, __LINE__);
			return 0;
		}

		LPCHARACTER leader = party->GetLeader();

		if (leader)
			lua_pushnumber(L, leader->GetVID());
		else
			lua_pushnumber(L, 0);

		return 1;
	}

	int32_t npc_get_vid(lua_State* L)
	{
		CQuestManager& q = CQuestManager::instance();
		LPCHARACTER npc = q.GetCurrentNPCCharacterPtr();

		if (!npc)
		{
			lua_pushnumber(L, 0);
			return 1;
		}
		
		lua_pushnumber(L, npc->GetVID());
		return 1;
	}

	int32_t npc_get_vid_attack_mul(lua_State* L)
	{
		if (!lua_isnumber(L, 1))
		{
			sys_err("wrong arg");
			return 0;
		}
		uint32_t vid = (uint32_t)lua_tonumber(L, 1);

		LPCHARACTER targetChar = CHARACTER_MANAGER::instance().Find(vid);

		if (targetChar)
			lua_pushnumber(L, targetChar->GetAttMul());
		else
			lua_pushnumber(L, 0);

		return 1;
	}
	
	int32_t npc_set_vid_attack_mul(lua_State* L)
	{
		if (!lua_isnumber(L, 1) || !lua_isnumber(L, 2))
		{
			sys_err("wrong arg");
			return 0;
		}
		uint32_t vid = (uint32_t)lua_tonumber(L, 1);
		float attack_mul = (float)lua_tonumber(L, 2);

		LPCHARACTER targetChar = CHARACTER_MANAGER::Instance().Find(vid);

		if (targetChar)
			targetChar->SetAttMul(attack_mul);

		return 0;
	}

	int32_t npc_get_vid_damage_mul(lua_State* L)
	{
		if (!lua_isnumber(L, 1))
		{
			sys_err("wrong arg");
			return 0;
		}
		uint32_t vid = (uint32_t)lua_tonumber(L, 1);

		LPCHARACTER targetChar = CHARACTER_MANAGER::Instance().Find(vid);

		if (targetChar)
			lua_pushnumber(L, targetChar->GetDamMul());
		else
			lua_pushnumber(L, 0);


		return 1;
	}
	
	int32_t npc_set_vid_damage_mul(lua_State* L)
	{
		if (!lua_isnumber(L, 1) || !lua_isnumber(L, 2))
		{
			sys_err("wrong arg");
			return 0;
		}
		uint32_t vid = (uint32_t)lua_tonumber(L, 1);
		float damage_mul = (float)lua_tonumber(L, 2);

		LPCHARACTER targetChar = CHARACTER_MANAGER::instance().Find(vid);

		if (targetChar)
			targetChar->SetDamMul(damage_mul);

		return 0;
	}

	int32_t npc_get_level(lua_State* L)
	{
		if (!lua_isnumber(L, 1))
		{
			sys_err("wrong arg");
			return 0;
		}

		uint32_t vid = (uint32_t)lua_tonumber(L, 1);
		if (vid)
		{
			LPCHARACTER ch = CHARACTER_MANAGER::Instance().Find((uint32_t)lua_tonumber(L, 1));

			if (!ch) 
			{
				lua_pushnumber(L, 0);
				return 1;
			}

			lua_pushnumber(L,ch->GetLevel());
		}
		else
		{
			CQuestManager& q = CQuestManager::instance();
			LPCHARACTER npc = q.GetCurrentNPCCharacterPtr();

			if (!npc) 
			{
				lua_pushnumber(L, 0);
				return 1;
			}

			lua_pushnumber(L,npc->GetLevel());
		}

		return 1;
	}

	int32_t npc_send_effect(lua_State* L)
	{
		if (!lua_isnumber(L, 1) || !lua_isstring(L, 2))
		{
			sys_err("wrong arg");
			return 0;
		}
		auto vid = (uint32_t)lua_tonumber(L, 1);
		std::string fname = lua_tostring(L, 2);

		LPCHARACTER npc = nullptr;
		if (vid)
		{
			npc = CHARACTER_MANAGER::Instance().Find((uint32_t)vid);
		}
		else
		{
			CQuestManager& q = CQuestManager::instance();
			npc = q.GetCurrentNPCCharacterPtr();
		}

		if (fname.size() > 0 && npc)
			npc->SpecificEffectPacket(fname.c_str());

		return 0;
	}

	int32_t npc_get_local_x(lua_State* L)
	{
		CQuestManager& q = CQuestManager::instance();
		LPCHARACTER npc = q.GetCurrentNPCCharacterPtr();
		if (!npc)
			return 0;

		LPSECTREE_MAP pMap = SECTREE_MANAGER::instance().GetMap(npc->GetMapIndex());
		
		if (pMap)
			lua_pushnumber(L, (npc->GetX() - pMap->m_setting.iBaseX) / 100);
		else
			lua_pushnumber(L, npc->GetX() / 100);

		return 1;
	}

	int32_t npc_get_local_y(lua_State* L)
	{
		CQuestManager& q = CQuestManager::instance();
		LPCHARACTER npc = q.GetCurrentNPCCharacterPtr();
		if (!npc)
			return 0;

		LPSECTREE_MAP pMap = SECTREE_MANAGER::instance().GetMap(npc->GetMapIndex());

		if (pMap)
			lua_pushnumber(L, (npc->GetY() - pMap->m_setting.iBaseY) / 100);
		else
			lua_pushnumber(L, npc->GetY() / 100);

		return 1;
	}

	int32_t npc_select(lua_State* L)
	{
		if (!lua_isnumber(L, 1))
		{
			lua_pushnumber(L, 0);
			return 1;
		}
		uint32_t vid = (uint32_t)lua_tonumber(L, 1);

		CHARACTER* pc = CQuestManager::instance().GetCurrentCharacterPtr();
		CHARACTER* ch = CHARACTER_MANAGER::instance().Find(vid);
		if (ch && pc)
		{
			pc->SetQuestNPCID(vid);
			lua_pushnumber(L, 1);
			return 1;
		}
	
		lua_pushnumber(L, 0);
		return 1;
	}

	int32_t npc_get_rank(lua_State* L)
	{
		if (!CQuestManager::instance().GetCurrentNPCCharacterPtr())
			return 0;

		lua_pushnumber(L, CQuestManager::instance().GetCurrentNPCCharacterPtr()->GetMobRank());
		return 1;
	}

	int32_t npc_get_type(lua_State* L)
	{
		if (!CQuestManager::instance().GetCurrentNPCCharacterPtr())
			return 0;

		lua_pushnumber(L, CQuestManager::instance().GetCurrentNPCCharacterPtr()->GetMobTable().bType);
		return 1;
	}

	int32_t npc_is_metin(lua_State* L)
	{
		if (!CQuestManager::instance().GetCurrentNPCCharacterPtr())
			return 0;

		lua_pushboolean(L, CQuestManager::instance().GetCurrentNPCCharacterPtr()->IsStone());
		return 1;
	}

	int32_t npc_is_boss(lua_State* L)
	{
		if (!CQuestManager::instance().GetCurrentNPCCharacterPtr())
			return 0;

		lua_pushboolean(L, CQuestManager::instance().GetCurrentNPCCharacterPtr()->GetMobRank() == MOB_RANK_BOSS);
		return 1;
	}

	int32_t npc_get_ip(lua_State* L)
	{
		CHARACTER* npc = CQuestManager::instance().GetCurrentNPCCharacterPtr();

		if (npc && npc->IsPC() && npc->GetDesc())
			lua_pushstring(L, npc->GetDesc()->GetHostName());
		else
			lua_pushstring(L, "");

		return 1;
	}

	int32_t npc_get_job(lua_State* L)
	{
		CHARACTER* npc = CQuestManager::instance().GetCurrentNPCCharacterPtr();
		if (npc && npc->IsPC())
			lua_pushnumber(L, npc->GetJob());
		else
			lua_pushnumber(L, -1);
		return 1;
	}

	int32_t npc_get_exp(lua_State* L)
	{
		if (!CQuestManager::instance().GetCurrentNPCCharacterPtr())
			return 0;

		lua_pushnumber(L, CQuestManager::instance().GetCurrentNPCCharacterPtr()->GetMobTable().dwExp);
		return 1;
	}

	int32_t npc_get_pid(lua_State* L)
	{
		CQuestManager & q = CQuestManager::instance();
		CHARACTER* npc = q.GetCurrentNPCCharacterPtr();
		if (!npc)
			return 0;

		lua_pushnumber(L, npc->GetPlayerID());
		return 1;
	}

	int32_t npc_show_effect_on_target(lua_State* L)
	{
		CQuestManager& q = CQuestManager::instance();
		CHARACTER* ch = q.GetCurrentCharacterPtr();
		CHARACTER* tch = q.GetCurrentNPCCharacterPtr();

		if (!tch || !ch || ch->GetVID() == tch->GetVID())
			return 0;

		if (lua_isstring(L, 1))
			tch->SpecificEffectPacket(lua_tostring(L, 2));

		return 0;
	}

	int32_t npc_get_vnum(lua_State* L)
	{
		CQuestManager& q = CQuestManager::instance();
		CHARACTER* npc = q.GetCurrentNPCCharacterPtr();
		if (!npc)
			return 0;

		lua_pushnumber(L, npc->GetRaceNum());
		return 1;
	}

	int32_t npc_is_available(lua_State* L)
	{
		CQuestManager& q = CQuestManager::instance();
		CHARACTER* npc = q.GetCurrentNPCCharacterPtr();

		lua_pushboolean(L, npc != nullptr);
		return 1;
	}

	int32_t npc_do_move(lua_State* L)
	{
		if (!lua_isnumber(L, 1) || !lua_isnumber(L, 2))
		{
			sys_err("npc_do_move >> invalid coordinates");
			return 0;
		}

		CHARACTER* pc = CQuestManager::instance().GetCurrentCharacterPtr();
		CHARACTER* npc = CQuestManager::instance().GetCurrentNPCCharacterPtr();
		if (!pc || !npc)
			return 0;

		LPSECTREE sectree = SECTREE_MANAGER::instance().Get(pc->GetMapIndex(), (uint32_t)lua_tonumber(L, 1), (uint32_t)lua_tonumber(L, 2));
		if (!sectree)
		{
			sys_err("npc_do_move >> wrong coordinates");
			return 0;
		}

		uint8_t FUNC_MOVE = 1;
		if (npc->Goto((int32_t)lua_tonumber(L, 1), (int32_t)lua_tonumber(L, 2)))
			npc->SendMovePacket(FUNC_MOVE, 0, 0, 0, 0);

		return 0;
	}


	void RegisterNPCFunctionTable()
	{
		luaL_reg npc_functions[] = 
		{
			{ "getrace",			npc_get_race			},
			{ "get_race",			npc_get_race			},
			{ "open_shop",			npc_open_shop			},
			{ "get_empire",			npc_get_empire			},
			{ "is_pc",				npc_is_pc			},
			{ "is_quest",			npc_is_quest			},
			{ "kill",				npc_kill			},
			{ "purge",				npc_purge			},
			{ "is_near",			npc_is_near			},
			{ "is_near_vid",			npc_is_near_vid			},
			{ "lock",				npc_lock			},
			{ "unlock",				npc_unlock			},
			{ "get_guild",			npc_get_guild			},
			{ "get_leader_vid",		npc_get_leader_vid	},
			{ "get_vid",			npc_get_vid	},
			{ "get_vid_attack_mul",		npc_get_vid_attack_mul	},
			{ "set_vid_attack_mul",		npc_set_vid_attack_mul	},
			{ "get_vid_damage_mul",		npc_get_vid_damage_mul	},
			{ "set_vid_damage_mul",		npc_set_vid_damage_mul	},

			// X-mas santa special
			{ "get_remain_skill_book_count",	npc_get_remain_skill_book_count },
			{ "dec_remain_skill_book_count",	npc_dec_remain_skill_book_count },
			{ "get_remain_hairdye_count",	npc_get_remain_hairdye_count	},
			{ "dec_remain_hairdye_count",	npc_dec_remain_hairdye_count	},

			{ "get_level",						npc_get_level						},
			{ "send_effect",					npc_send_effect						},
			{ "get_local_x",					npc_get_local_x						},
			{ "get_local_y",					npc_get_local_y						},
			{ "select",							npc_select							},
			{ "get_type",						npc_get_type						},
			{ "get_rank",						npc_get_rank						},
			{ "is_metin",						npc_is_metin						},
			{ "is_boss",						npc_is_boss							},
			{ "get_ip",							npc_get_ip							},
			{ "get_job",						npc_get_job							},
			{ "get_pid",						npc_get_pid							},
			{ "get_exp",						npc_get_exp							},
			{ "get_vnum",						npc_get_vnum						},
			{ "show_effect_on_target",			npc_show_effect_on_target			},
			{ "get_name",						npc_get_name						},
			{ "is_available",					npc_is_available					},
			{ "do_move",						npc_do_move							},

			{ nullptr,				nullptr			    	}
		};

		CQuestManager::instance().AddLuaFunctionTable("npc", npc_functions);
	}
};
