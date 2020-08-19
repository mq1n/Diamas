#include "stdafx.h"
#include "questlua.h"
#include "quest_manager.h"
#include "desc_client.h"
#include "char.h"
#include "item_manager.h"
#include "item.h"
#include "cmd.h"
#include "desc_manager.h"
#include "party.h"

#undef sys_err
#ifndef __WIN32__
#define sys_err(fmt, args...) quest::CQuestManager::Instance().QuestError(__FUNCTION__, __LINE__, fmt, ##args)
#else
#define sys_err(fmt, ...) quest::CQuestManager::Instance().QuestError(__FUNCTION__, __LINE__, fmt, __VA_ARGS__)
#endif

extern ACMD(do_in_game_mall);

namespace quest
{
	int32_t game_set_event_flag(lua_State* L)
	{
		CQuestManager & q = CQuestManager::Instance();

		if (lua_isstring(L,1) && lua_isnumber(L, 2))
			q.RequestSetEventFlag(lua_tostring(L,1), (int32_t)lua_tonumber(L,2));

		return 0;
	}

	int32_t game_get_event_flag(lua_State* L)
	{
		CQuestManager& q = CQuestManager::Instance();

		if (lua_isstring(L,1))
			lua_pushnumber(L, q.GetEventFlag(lua_tostring(L,1)));
		else
			lua_pushnumber(L, 0);

		return 1;
	}

	int32_t game_request_make_guild(lua_State* L)
	{
		CHARACTER* ch = CQuestManager::Instance().GetCurrentCharacterPtr();
		if (!ch) 
		{
			sys_err("Null character pointer triggered at %s:%d", __FILE__, __LINE__);
			return 0;
		}

		LPDESC d = ch->GetDesc();
		if (d)
		{
			uint8_t header = HEADER_GC_REQUEST_MAKE_GUILD;
			d->Packet(&header, 1);
		}
		return 0;
	}

	int32_t game_get_safebox_level(lua_State* L)
	{
		CHARACTER* ch = CQuestManager::Instance().GetCurrentCharacterPtr();
		if (!ch)
		{
			sys_err("Null character pointer triggered at %s:%d", __FILE__, __LINE__);
			return 0;
		}

		lua_pushnumber(L, ch->GetSafeboxSize()/SAFEBOX_PAGE_SIZE);
		return 1;
	}

	int32_t game_set_safebox_level(lua_State* L)
	{
		CHARACTER* ch = CQuestManager::Instance().GetCurrentCharacterPtr();
		if (!ch)
		{
			sys_err("Null character pointer triggered at %s:%d", __FILE__, __LINE__);
			return 0;
		}

		LPDESC d = ch->GetDesc();
		if (!d)
		{
			sys_err("Null desc pointer triggered at %s:%d", __FILE__, __LINE__);
			return 0;
		}

		if (!lua_isnumber(L, -1))
		{
			sys_err("wrong arg");
			return 0;
		}

		TSafeboxChangeSizePacket p;
		p.dwID = d->GetAccountTable().id;
		p.bSize = (int32_t)lua_tonumber(L,-1);
		db_clientdesc->DBPacket(HEADER_GD_SAFEBOX_CHANGE_SIZE,  d->GetHandle(), &p, sizeof(p));

		ch->SetSafeboxSize(SAFEBOX_PAGE_SIZE * (int32_t)lua_tonumber(L,-1));
		return 0;
	}

	int32_t game_open_safebox(lua_State* /*L*/)
	{
		CHARACTER* ch = CQuestManager::Instance().GetCurrentCharacterPtr();
		if (!ch)
		{
			sys_err("Null character pointer triggered at %s:%d", __FILE__, __LINE__);
			return 0;
		}

		ch->SetSafeboxOpenPosition();
		ch->ChatPacket(CHAT_TYPE_COMMAND, "ShowMeSafeboxPassword");
		return 0;
	}

	int32_t game_open_mall(lua_State* /*L*/)
	{
		CHARACTER* ch = CQuestManager::Instance().GetCurrentCharacterPtr();
		if (!ch)
		{
			sys_err("Null character pointer triggered at %s:%d", __FILE__, __LINE__);
			return 0;
		}

		ch->SetSafeboxOpenPosition();
		ch->ChatPacket(CHAT_TYPE_COMMAND, "ShowMeMallPassword");
		return 0;
	}

	int32_t game_drop_item(lua_State* L)
	{
		CHARACTER* ch = CQuestManager::Instance().GetCurrentCharacterPtr();
		if (!ch)
		{
			sys_err("Null character pointer triggered at %s:%d", __FILE__, __LINE__);
			return 0;
		}

		if (!lua_isnumber(L, 1) || !lua_isnumber(L, 2))
		{
			sys_err("wrong arg");
			return 0;
		}
		uint32_t item_vnum = (uint32_t) lua_tonumber(L, 1);
		int32_t count = (int32_t) lua_tonumber(L, 2);
		int32_t x = ch->GetX();
		int32_t y = ch->GetY();

		LPITEM item = ITEM_MANAGER::Instance().CreateItem(item_vnum, count);

		if (!item)
		{
			sys_err("cannot create item vnum %d count %d", item_vnum, count);
			return 0;
		}

		GPOS pos;
		pos.x = x + number(-200, 200);
		pos.y = y + number(-200, 200);

		item->AddToGround(ch->GetMapIndex(), pos);
		item->StartDestroyEvent();

		return 0;
	}

	int32_t game_drop_item_with_ownership(lua_State* L)
	{
		LPCHARACTER ch = CQuestManager::Instance().GetCurrentCharacterPtr();
		if (!ch)
		{
			sys_err("Null character pointer triggered at %s:%d", __FILE__, __LINE__);
			return 0;
		}

		LPITEM item = nullptr;
		switch (lua_gettop(L))
		{
		case 1:
			item = ITEM_MANAGER::Instance().CreateItem((uint32_t) lua_tonumber(L, 1));
			break;
		case 2:
		case 3:
			item = ITEM_MANAGER::Instance().CreateItem((uint32_t) lua_tonumber(L, 1), (int32_t) lua_tonumber(L, 2));
			break;
		default:
			return 0;
		}

		if (!item)
		{
			return 0;
		}

		if (lua_isnumber(L, 3))
		{
			int32_t sec = (int32_t) lua_tonumber(L, 3);
			if (sec <= 0)
			{
				item->SetOwnership( ch );
			}
			else
			{
				item->SetOwnership( ch, sec );
			}
		}
		else
			item->SetOwnership( ch );

		GPOS pos;
		pos.x = ch->GetX() + number(-200, 200);
		pos.y = ch->GetY() + number(-200, 200);

		item->AddToGround(ch->GetMapIndex(), pos);
		item->StartDestroyEvent();

		return 0;
	}

	int32_t game_web_mall(lua_State* L)
	{
		LPCHARACTER ch = CQuestManager::Instance().GetCurrentCharacterPtr();
		if (ch)
			do_in_game_mall(ch, const_cast<char*>(""), 0, 0);

		return 0;
	}

#ifdef ENABLE_DICE_SYSTEM
	int32_t game_drop_item_with_ownership_and_dice(lua_State* L)
	{
		CItem * item = nullptr;
		switch (lua_gettop(L))
		{
		case 1:
			item = ITEM_MANAGER::Instance().CreateItem((uint32_t)lua_tonumber(L, 1));
			break;
		case 2:
		case 3:
			item = ITEM_MANAGER::Instance().CreateItem((uint32_t)lua_tonumber(L, 1), (int32_t)lua_tonumber(L, 2));
			break;
		default:
			return 0;
		}

		if (item == nullptr)
		{
			return 0;
		}

		LPCHARACTER ch = CQuestManager::Instance().GetCurrentCharacterPtr();
		if (ch->GetParty())
		{
			FPartyDropDiceRoll f(item, ch);
			f.Process(nullptr);
		}

		if (lua_isnumber(L, 3))
		{
			int32_t sec = (int32_t)lua_tonumber(L, 3);
			if (sec <= 0)
			{
				item->SetOwnership(ch);
			}
			else
			{
				item->SetOwnership(ch, sec);
			}
		}
		else
			item->SetOwnership(ch);

		GPOS pos;
		pos.x = ch->GetX() + number(-200, 200);
		pos.y = ch->GetY() + number(-200, 200);

		item->AddToGround(ch->GetMapIndex(), pos);
		item->StartDestroyEvent();

		return 0;
	}
#endif

	
	int32_t game_drop_item_and_select(lua_State* L)
	{
		CHARACTER* ch = CQuestManager::Instance().GetCurrentCharacterPtr();
		if (!ch)
			return 0;

		CItem* item = nullptr;
		bool bHasOwnership = false;
		int32_t iOwnershipTime = 0;

		switch (lua_gettop(L))
		{
			case 1:
				if (!lua_isnumber(L, 1))
				{
					sys_err("Invalid arguments..");
					return 0;
				}
				item = ITEM_MANAGER::Instance().CreateItem((uint32_t)lua_tonumber(L, 1));
				break;

			case 2:
			case 3:
			case 4:
				if (!lua_isnumber(L, 1) || !lua_isnumber(L, 2))
				{
					sys_err("Invalid arguments..");
					return 0;
				}
				item = ITEM_MANAGER::Instance().CreateItem((uint32_t)lua_tonumber(L, 1), (int32_t)lua_tonumber(L, 2));
				bHasOwnership = lua_isboolean(L, 3) ? (bool)lua_toboolean(L, 3) : false;
				iOwnershipTime = lua_isnumber(L, 4) ? (int32_t)lua_tonumber(L, 4) : 250;
				break;

			default:
				sys_err("Invalid arguments..");
				return 0;
		}

		if (!item)
		{
			sys_err("Cannot created item, error occurred.");
			return 0;
		}

		// SELECT_ITEM
		CQuestManager::Instance().SetCurrentItem(item);
		// END_OF_SELECT_ITEM

		if (bHasOwnership)
			item->SetOwnership(ch, iOwnershipTime);

		GPOS pos;
		pos.x = ch->GetX() + number(-100, 100);
		pos.y = ch->GetY() + number(-100, 100);

		item->AddToGround(ch->GetMapIndex(), pos);
		item->StartDestroyEvent();

		return 0;
	}

	int32_t game_drop_map(lua_State* L)
	{
		CHARACTER* ch = CQuestManager::Instance().GetCurrentCharacterPtr();
		if (!ch)
			return 0;

		CItem* item = nullptr;
		if (lua_isnumber(L, 1))
			item = ITEM_MANAGER::Instance().CreateItem((uint32_t)lua_tonumber(L, 1));

		if (!item)
			return 0;

		uint32_t kordx = 0;
		if (lua_isnumber(L, 2))
			kordx = ((uint32_t)lua_tonumber(L, 2));

		uint32_t kordy = 0;
		if (lua_isnumber(L, 3))
			kordy = ((uint32_t)lua_tonumber(L, 3));

		if (!kordx || !kordy)
			return 0;

		GPOS pos;
		pos.x = kordx;
		pos.y = kordy;

		item->AddToGround(ch->GetMapIndex(), pos);
		item->StartDestroyEvent();
		item->SetOwnership(ch);

		return 0;
	}

	int32_t game_create_discord_lobby(lua_State* L)
	{
		const auto ch = CQuestManager::Instance().GetCurrentCharacterPtr();
		if (!ch)
		{
			sys_err("cannot get character");
			return 0;
		}		

		ch->CreateDiscordLobby();
		return 0;
	}
	int32_t game_join_discord_lobby(lua_State* L)
	{	
		if (!lua_isnumber(L, 1) || !lua_isstring(L, 2)) 
		{
			sys_err("Invalid Argument");
			return 0;
		}
		const auto lobby = static_cast<int64_t>(rint(lua_tonumber(L, 1)));
		const auto secret = std::string(lua_tostring(L, 2));

		if (!lobby || secret.empty())
		{
			sys_err("Sanity failed");
			return 0;		
		}

		const auto ch = CQuestManager::Instance().GetCurrentCharacterPtr();
		if (!ch)
		{
			sys_err("cannot get character");
			return 0;
		}		

		ch->JoinDiscordLobby(lobby, secret);
		return 0;
	}
	int32_t game_get_discord_lobby(lua_State* L)
	{	
		const auto ch = CQuestManager::Instance().GetCurrentCharacterPtr();
		if (!ch)
		{
			sys_err("cannot get character");
			lua_pushnumber(L, 0);
			lua_pushstring(L, "");
			return 2;
		}		

		lua_pushnumber(L, ch->GetDiscordLobbyID());
		lua_pushstring(L, ch->GetDiscordLobbySecret().c_str());
		return 2;
	}

	void RegisterGameFunctionTable()
	{
		luaL_reg game_functions[] = 
		{
			{ "create_discord_lobby",		game_create_discord_lobby		},
			{ "join_discord_lobby",			game_join_discord_lobby			},
			{ "get_discord_lobby",			game_get_discord_lobby			},

			{ "get_safebox_level",			game_get_safebox_level			},
			{ "request_make_guild",			game_request_make_guild			},
			{ "set_safebox_level",			game_set_safebox_level			},
			{ "open_safebox",				game_open_safebox				},
			{ "open_mall",					game_open_mall					},
			{ "get_event_flag",				game_get_event_flag				},
			{ "set_event_flag",				game_set_event_flag				},
			{ "drop_item",					game_drop_item					},
			{ "drop_item_with_ownership",	game_drop_item_with_ownership	},
#ifdef ENABLE_DICE_SYSTEM
			{ "drop_item_with_ownership_and_dice",	game_drop_item_with_ownership_and_dice },
#endif
			{ "open_web_mall",				game_web_mall					},
			{ "drop_item_and_select",       game_drop_item_and_select       },
			{ "drop_item_in_map",			game_drop_map					},

			{ nullptr,					nullptr				}
		};

		CQuestManager::Instance().AddLuaFunctionTable("game", game_functions);
	}
}

