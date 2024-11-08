#pragma once
#include "quest.h"
#include "buffer_manager.h"

extern int32_t g_bIsTestServer;

namespace quest
{
	extern void RegisterPCFunctionTable();
	extern void RegisterNPCFunctionTable();
	extern void RegisterTargetFunctionTable();
	extern void RegisterAffectFunctionTable();
	extern void RegisterBuildingFunctionTable();
	extern void RegisterMarriageFunctionTable();
	extern void RegisterITEMFunctionTable();
	extern void RegisterDungeonFunctionTable();
	extern void RegisterQuestFunctionTable();
	extern void RegisterPartyFunctionTable();
	extern void RegisterHorseFunctionTable();
	extern void RegisterPetFunctionTable();
	extern void RegisterGuildFunctionTable();
	extern void RegisterGameFunctionTable();
	extern void RegisterArenaFunctionTable();
	extern void RegisterGlobalFunctionTable(lua_State* L);
	extern void RegisterForkedFunctionTable();
	extern void RegisterOXEventFunctionTable();
	extern void RegisterDanceEventFunctionTable();
	extern void RegisterDragonSoulFunctionTable();

	extern void combine_lua_string(lua_State* L, std::ostringstream &s);
	
	struct FSetWarpLocation
	{
		int32_t map_index; 
		int32_t x;
		int32_t y;

		FSetWarpLocation (int32_t _map_index, int32_t _x, int32_t _y) :
			map_index (_map_index), x (_x), y (_y)
		{}
		void operator () (LPCHARACTER ch) const;
	};

	struct FSetQuestFlag
	{
		std::string flagname;
		int32_t value;

		void operator () (LPCHARACTER ch) const;
	};

	struct FPartyCheckFlagLt
	{
		std::string flagname;
		int32_t value;

		bool operator () (LPCHARACTER ch);
	};

	struct FPartyChat
	{
		int32_t iChatType;
		const char* str;

		FPartyChat(int32_t ChatType, const char* str);
		void operator() (LPCHARACTER ch);
	};

	struct FPartyClearReady
	{
		void operator() (LPCHARACTER ch);
	};

	struct FSendPacket
	{
		TEMP_BUFFER buf;

		void operator() (LPENTITY ent);
	};

#ifdef ENABLE_NEWSTUFF
	struct FSendChatPacket
	{
		uint32_t m_chat_type;
		std::string m_text;

		FSendChatPacket(uint32_t chat_type, std::string text) : m_chat_type(chat_type), m_text(text) {}
		void operator() (LPENTITY ent);
	};
#endif

	struct FSendPacketToEmpire
	{
		TEMP_BUFFER buf;
		uint8_t bEmpire;

		void operator() (LPENTITY ent);
	};

	struct FWarpEmpire
	{
		uint8_t m_bEmpire;
		int32_t m_lMapIndexTo;
		int32_t m_x;
		int32_t m_y;

		void operator() (LPENTITY ent);
	};

	EVENTINFO(warp_all_to_map_my_empire_event_info)
	{
		uint8_t 	m_bEmpire;
		int32_t	m_lMapIndexFrom;
		int32_t 	m_lMapIndexTo;
		int32_t 	m_x;
		int32_t	m_y;

		warp_all_to_map_my_empire_event_info() 
		: m_bEmpire( 0 )
		, m_lMapIndexFrom( 0 )
		, m_lMapIndexTo( 0 )
		, m_x( 0 )
		, m_y( 0 )
		{
		}
	};

	EVENTFUNC(warp_all_to_map_my_empire_event);

	struct FBuildLuaGuildWarList
	{
		lua_State * L;
		int32_t m_count;

		FBuildLuaGuildWarList(lua_State * L);
		void operator() (uint32_t g1, uint32_t g2);
	};
}
