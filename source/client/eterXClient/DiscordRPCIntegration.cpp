#include "StdAfx.h"
#include "DiscordRPCIntegration.h"
#include "../../common/stl.h"

static std::map <std::string, std::string> gs_MapNames = 
{
	{ "map_a2", "Seungryong Vadisi" },
	{ "map_b2", "Imji Vadisi" },
	{ "map_c2", "Bangsan Vadisi" },
	{ "map_n_snowm_01", "Sohan dagi" },
	{ "metin2_map_a1", "Yongan Bolgesi" },
	{ "metin2_map_a3", "Yayang Bolgesi" },
	{ "metin2_map_b1", "Joan Bolgesi" },
	{ "metin2_map_b3", "Bokjung Bolgesi" },
	{ "metin2_map_c1", "Pyungmoo Bolgesi" },
	{ "metin2_map_c3", "Bakra Bolgesi" },
	{ "metin2_map_deviltower1", "Seytan Kulesi" },
	{ "metin2_map_milgyo", "Tapinak" },
	{ "metin2_map_n_desert_01", "Yongbi Colu" },
	{ "metin2_map_n_flame_01", "Doyyumhwan" },
	{ "metin2_map_spiderdungeon", "Orumcek zindani" },
	{ "metin2_map_spiderdungeon_02", "Orumcek zindani 2.kat" },
	{ "metin2_map_spiderdungeon_03", "Orumcek zindani 3.kat" },
	{ "metin2_map_t1", "Lonca savasi" },
	{ "metin2_map_t2", "Lonca savasi" },
	{ "metin2_map_t3", "Lonca savasi" },
	{ "metin2_map_t4", "Lonca savasi" },
	{ "metin2_map_t5", "Lonca savasi" },
	{ "metin2_map_monkeydungeon", "Maymun zindani 1" },
	{ "metin2_map_monkeydungeon_02", "Maymun zindani 2" },
	{ "metin2_map_monkeydungeon_03", "Maymun zindani 3" },
	{ "metin2_map_wedding_01", "Dugun haritasi" },
	{ "metin2_map_guild_01", "Lonca alani 1" },
	{ "metin2_map_guild_02", "Lonca alani 2" },
	{ "metin2_map_guild_03", "Lonca alani 3" },
	{ "metin2_map_trent", "Lungsam ormani" },
	{ "metin2_map_trent02", "Kizil orman" },
	{ "gm_guild_build", "Etkinlik bolgesi" },
	{ "metin2_map_duel", "Arena" },
	{ "metin2_map_duel2", "Arena 2" },
	{ "season1_metin2_map_WL_01", "Yilan vadisi" },
	{ "season1_metin2_map_nusluck01", "Devler diyari" },
	{ "season1_metin2_map_oxevent", "OX Etkinligi" },
	{ "metin2_map_skipia_dungeon_02", "Surgun magarasi 2.kat" },
	{ "metin2_map_skipia_dungeon_boss", "Kristal oda" },
	{ "metin2_map_devilsCatacomb", "Seytan katakombu" },
	{ "metin2_map_battlefied", "Savas alani" },
	{ "metin2_12zi_stage", "Zodyak" },
	{ "metin2_map_defensewave", "Gemi savunma" },
	{ "metin2_map_defensewave_port", "Gemi savasi rihtimi" },
	{ "mushroom_dungeon", "Mantar tapinagi" },
	{ "water_dungeon", "Su magarasi" },
	{ "demon_dungeon", "Seytan magarasi" },
	{ "moon_cave", "Ay magarasi" },
	{ "nephrite_cave", "Nefrit magarasi" },
	{ "underwater_dungeon", "Denizin dehseti" },
	{ "crystal_dungeon", "Typeous'un Ini" },
	{ "fear_dungeon", "Astrayos'un Ini" },
	{ "owl_dungeon", "Baykus tapinagi" },
	{ "plant_dungeon", "Lanetli bahce" },
	{ "slime_cave", "Balcik magarasi" },
	{ "deep_underwater_cave", "Sualti diyari" },
	{ "plechi_summer2019_dungeon", "Kayip ada" },
	{ "valentine2019_map", "Balayi diyari" },
	{ "easter2019_dungeon", "Paskalya ormani" },
	{ "duratus_dungeon", "Yeralti magarasi" },
	{ "nightmare_dungeon", "nightmare_dungeon" },
	{ "magic_cave_dungeon", "Buyulu magara" },
	{ "devil_tower2", "Lanetli seytan kulesi" },
	{ "plague_dungeon", "Vebalilar diyari" },
	{ "metin2_map_anglar_dungeon_01", "Surgun magarasi 3.kat" },
	{ "summonersrift", "Sihirdar Vadisi" },

	{ "Metin2_map_CapeDragonHead", "Metin2_map_CapeDragonHead" },
	{ "metin2_map_Mt_Thunder", "metin2_map_Mt_Thunder" },
	{ "metin2_map_dawnmistwood", "metin2_map_dawnmistwood" },
	{ "metin2_map_BayBlackSand", "metin2_map_BayBlackSand" },
	{ "mapa_guerra", "mapa_guerra" },
	{ "metin2_map_icerun", "metin2_map_icerun" },
	{ "teodor_glade", "teodor_glade" },
	{ "metin2_map_n_flame_dungeon_01", "metin2_map_n_flame_dungeon_01" },
	{ "metin2_map_n_snow_dungeon_01", "metin2_map_n_snow_dungeon_01" },
	{ "metin2_map_e1", "metin2_map_e1" },
	{ "metin2_map_gemi", "metin2_map_gemi" },
	{ "metin2_map_dawnmist_dungeon_01", "metin2_map_dawnmist_dungeon_01" },
	{ "metin2_map_dawnmist_dungeon_02", "metin2_map_dawnmist_dungeon_02" },
	{ "metin2_map_mt_th_dungeon_01", "metin2_map_mt_th_dungeon_01" },
	{ "metin2_map_peloevent1", "metin2_map_peloevent1" },
	{ "metin2_map_orclabyrinth", "metin2_map_orclabyrinth" },
	{ "metin2_map_n_flame_dragon", "metin2_map_n_flame_dragon" },

	{ "metin2_guild_village_01", "metin2_guild_village_01" },
	{ "metin2_guild_village_02", "metin2_guild_village_02" },
	{ "metin2_guild_village_03", "metin2_guild_village_03" },
	{ "metin2_map_skipia_dungeon_boss2", "metin2_map_skipia_dungeon_boss2" },
	{ "metin2_map_bf", "metin2_map_bf" },
	{ "metin2_map_bf_02", "metin2_map_bf_02" },
	{ "metin2_map_bf_03", "metin2_map_bf_03" },
	{ "season1_metin2_map_sungzi", "season1_metin2_map_sungzi" },
	{ "metin2_map_sungzi_flame_pass01", "metin2_map_sungzi_flame_pass01" },
	{ "metin2_map_sungzi_flame_pass02       ", "metin2_map_sungzi_flame_pass02       " },
	{ "metin2_map_sungzi_flame_pass03       ", "metin2_map_sungzi_flame_pass03       " },
	{ "season1_metin2_map_sungzi_flame_hill_01", "season1_metin2_map_sungzi_flame_hill_01" },
	{ "season1_metin2_map_sungzi_flame_hill_02", "season1_metin2_map_sungzi_flame_hill_02" },
	{ "season1_metin2_map_sungzi_flame_hill_03", "season1_metin2_map_sungzi_flame_hill_03" },
	{ "season1_metin2_map_sungzi_snow", "season1_metin2_map_sungzi_snow" },
	{ "season1_metin2_map_sungzi_snow_pass01 ", "season1_metin2_map_sungzi_snow_pass01 " },
	{ "season1_metin2_map_sungzi_snow_pass02", "season1_metin2_map_sungzi_snow_pass02" },
	{ "season1_metin2_map_sungzi_snow_pass03", "season1_metin2_map_sungzi_snow_pass03" },
	{ "season1_metin2_map_sungzi_desert_01", "season1_metin2_map_sungzi_desert_01" },
	{ "season1_metin2_map_sungzi_desert_hill_01", "season1_metin2_map_sungzi_desert_hill_01" },
	{ "season1_metin2_map_sungzi_desert_hill_02", "season1_metin2_map_sungzi_desert_hill_02" },
	{ "season1_metin2_map_sungzi_desert_hill_03", "season1_metin2_map_sungzi_desert_hill_03" },
	{ "season1_metin2_map_sungzi_milgyo", "season1_metin2_map_sungzi_milgyo" },
	{ "season1_metin2_map_sungzi", "season1_metin2_map_sungzi" },
	{ "season1_metin2_map_sungzi_snow", "season1_metin2_map_sungzi_snow" },
	{ "season1_metin2_map_sungzi_desert_01", "season1_metin2_map_sungzi_desert_01" },
	{ "season1_metin2_map_sungzi_milgyo_pass01", "season1_metin2_map_sungzi_milgyo_pass01" },
	{ "season1_metin2_map_sungzi_milgyo_pass02", "season1_metin2_map_sungzi_milgyo_pass02" },
	{ "season1_metin2_map_sungzi_milgyo_pass03", "season1_metin2_map_sungzi_milgyo_pass03" },
	{ "season1_metin2_map_sungzi_flame_hill_01", "season1_metin2_map_sungzi_flame_hill_01" },
	{ "season1_metin2_map_sungzi_flame_hill_02", "season1_metin2_map_sungzi_flame_hill_02" },
	{ "season1_metin2_map_sungzi_flame_hill_03", "season1_metin2_map_sungzi_flame_hill_03" },
	{ "season1_metin2_map_sungzi_snow_pass01", "season1_metin2_map_sungzi_snow_pass01" },
	{ "season1_metin2_map_sungzi_snow_pass02", "season1_metin2_map_sungzi_snow_pass02" },
	{ "season1_metin2_map_sungzi_snow_pass03", "season1_metin2_map_sungzi_snow_pass03" },
	{ "season1_metin2_map_sungzi_desert_hill_01", "season1_metin2_map_sungzi_desert_hill_01" },
	{ "season1_metin2_map_sungzi_desert_hill_02", "season1_metin2_map_sungzi_desert_hill_02" },
	{ "season1_metin2_map_sungzi_desert_hill_03", "season1_metin2_map_sungzi_desert_hill_03" },
	{ "season1_metin2_map_ew02", "season1_metin2_map_ew02" },
	{ "season2_metin2_map_empirewar01", "season2_metin2_map_empirewar01" },
	{ "season2_metin2_map_empirewar02 ", "season2_metin2_map_empirewar02 " },
	{ "season2_metin2_map_empirewar03 ", "season2_metin2_map_empirewar03 " },
	{ "season2_map_n_snowm_02", "season2_map_n_snowm_02" },
	{ "season2_metin2_map_milgyo_a", "season2_metin2_map_milgyo_a" },
	{ "season2_metin2_map_n_desert_02", "season2_metin2_map_n_desert_02" },
	{ "season2_metin2_map_n_flame_02", "season2_metin2_map_n_flame_02" },
	{ "season2_metin2_map_a2_1", "season2_metin2_map_a2_1" },
	{ "season2_metin2_map_trent_a", "season2_metin2_map_trent_a" },
	{ "season2_metin2_map_trent02_a", "season2_metin2_map_trent02_a" },
	{ "season2_metin2_map_skipia_dungeon_01", "season2_metin2_map_skipia_dungeon_01" },
	{ "metin2_map_skipia_dungeon_02", "metin2_map_skipia_dungeon_02" },
	{ "season2_metin2_map_skipia_dungeon_01", "season2_metin2_map_skipia_dungeon_01" },
	{ "season2_metin2_map_skipia_dungeon_01", "season2_metin2_map_skipia_dungeon_01" },
	{ "season2_metin2_map_skipia_dungeon_01", "season2_metin2_map_skipia_dungeon_01" },
	{ "season2_metin2_map_empirewar01", "season2_metin2_map_empirewar01" },
	{ "season2_metin2_map_empirewar02", "season2_metin2_map_empirewar02" },
	{ "season2_metin2_map_empirewar03", "season2_metin2_map_empirewar03" },
	{ "season1_metin2_map_spiderdungeon_02", "season1_metin2_map_spiderdungeon_02" },
	{ "season1_metin2_map_oxevent", "season1_metin2_map_oxevent" }
};

static uint64_t getUnixTimeStamp()
{
    std::time_t st = std::time(nullptr);
    auto secs = static_cast<std::chrono::seconds>(st).count();
    return static_cast<uint64_t>(secs);
}

CDiscordRPCIntegration::CDiscordRPCIntegration() : 
	m_bSendDiscordPresence(false), m_nDiscordUpdateTimerID(0), m_hWnd(nullptr), m_bInitialized(false)
{
	m_nAppStartTimestamp = getUnixTimeStamp();
}
CDiscordRPCIntegration::~CDiscordRPCIntegration()
{
}

static std::string MapNameFromMapKey(const std::string& mapKey)
{
	auto it = gs_MapNames.find(mapKey);
	if (it == gs_MapNames.end())
	{
		TraceError("Unknown map key: %s", mapKey.c_str());
		return "Diamas_web_logo"; // default name
	}
	return it->second;
}
static std::string NormalizedMapKey(std::string mapKey)
{
	string_replace(mapKey, "/", "_");
	
	auto it = gs_MapNames.find(mapKey);
	if (it == gs_MapNames.end())
	{
		TraceError("Unknown map key: %s", mapKey.c_str());
		return "diamas_logo_yuvarlak"; // default asset
	}

	return mapKey;
}

VOID CALLBACK DiscordUpdateRoutine(HWND hwnd, UINT message, UINT_PTR iCurrTimerID, DWORD dwTime)
{
#ifdef DISCORD_DISABLE_IO_THREAD
	Discord_UpdateConnection();
#endif
	Discord_RunCallbacks();	
}

void CDiscordRPCIntegration::__InitializeDiscordAPIImpl()
{
	DiscordEventHandlers handlers{};

    handlers.ready = [](const::DiscordRpcUser* connectedUser) {
		Tracenf("Discord: connected to user %s#%s - %s\n",
           connectedUser->username, connectedUser->discriminator, connectedUser->userId
		);
    };
    handlers.disconnected = [](int errorCode, const char* message) {
		Tracenf("Discord: disconnected (%d: %s)", errorCode, message);
    };
    handlers.errored = [](int errorCode, const char* message) {
		TraceError("Discord: error (%d: %s)", errorCode, message);
    };
    handlers.joinGame = [](const char* joinSecret) {
		Tracenf("Discord: join (%s)", joinSecret);
    };
    handlers.spectateGame = [](const char* spectateSecret) {
		Tracenf("Discord: spectate (%s)", spectateSecret);
    };
    handlers.joinRequest = [](const::DiscordRpcUser* request) {
		Tracenf("Discord: join request from %s#%s - %s",
			request->username, request->discriminator, request->userId
		);
    };

    Discord_Initialize(DISCORD_APP_CLIENT_ID, &handlers, 1, nullptr);

	m_nDiscordUpdateTimerID = SetTimer(m_hWnd, 0, 15000, DiscordUpdateRoutine);	
	m_bInitialized = true;
}
void CDiscordRPCIntegration::UpdateDiscordState(uint8_t stateType, uint8_t channelID, const std::string& mapKey, const std::string& name)
{
	if (!m_bInitialized)
		return;

    DiscordRichPresence discordPresence{};

	switch (stateType)
	{
		case DISCORD_STATE_CLEAR:
		{
			Discord_ClearPresence();
			return;
		} break;
		case DISCORD_STATE_INITIALIZE:
		{
			discordPresence.state = "Giris yapiyor";
    		discordPresence.startTimestamp = m_nAppStartTimestamp;

			discordPresence.largeImageKey = "diamas_logo_yuvarlak";
			discordPresence.largeImageText = "diamas_web_addr";
		} break;
		case DISCORD_STATE_SELECT:
		{
			discordPresence.state = "Karakter seciyor";
    		discordPresence.startTimestamp = m_nAppStartTimestamp;

			discordPresence.largeImageKey = "diamas_logo_yuvarlak";
			discordPresence.largeImageText = "diamas_web_addr";
		} break;
		case DISCORD_STATE_LOAD:
		{
			discordPresence.state = "Oyun yukleniyor";
    		discordPresence.startTimestamp = m_nAppStartTimestamp;

			discordPresence.largeImageKey = "diamas_logo_yuvarlak";
			discordPresence.largeImageText = "diamas_web_addr";
		} break;
		case DISCORD_STATE_GAME:
		{
			char szDetails[128];
			sprintf_s(szDetails, "%s [CH%d]", name.c_str(), channelID);

			char szNormalizedMapKey[128];
			sprintf_s(szNormalizedMapKey, "%s", NormalizedMapKey(mapKey).c_str());

			char szMapName[100];
			sprintf_s(szMapName, "%s", MapNameFromMapKey(szNormalizedMapKey).c_str());

			char szState[128];
			sprintf_s(szState, "Oyunda: %s", szMapName);

			discordPresence.state = szState;
			discordPresence.details = szDetails;
    		discordPresence.startTimestamp = m_nAppStartTimestamp;
			
			discordPresence.largeImageKey =  szNormalizedMapKey;
			discordPresence.largeImageText = szMapName;

  			discordPresence.smallImageKey = "diamas_logo_yuvarlak";
			discordPresence.smallImageText = "diamas_web_addr";
		} break;
		default:
			TraceError("Unknown discord state: %u", stateType);
			break;
	}

//    discordPresence.partyId = "party1234";
//    discordPresence.partySize = 1;
//    discordPresence.partyMax = 6;
//    discordPresence.matchSecret = "xyzzy";
//    discordPresence.joinSecret = "join";
//    discordPresence.spectateSecret = "look";

    discordPresence.instance = 0;
    Discord_UpdatePresence(&discordPresence);
}

void CDiscordRPCIntegration::InitializeDiscordAPI(HWND hWnd)
{
	m_hWnd = hWnd;

	__InitializeDiscordAPIImpl();
	UpdateDiscordState(DISCORD_STATE_INITIALIZE);
}
void CDiscordRPCIntegration::ReleaseDiscordAPI()
{
	if (!m_bInitialized)
		return;

	if (m_nDiscordUpdateTimerID)
	{
		KillTimer(m_hWnd, m_nDiscordUpdateTimerID);
		m_nDiscordUpdateTimerID = 0;
	}
	UpdateDiscordState(DISCORD_STATE_CLEAR);
    Discord_Shutdown();

	m_bInitialized = false;
}
