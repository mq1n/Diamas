#pragma once
/*
 * @file	Battleground.h
 * @author	Koray (mq1n@protonmail.com)
 * @version	0.1 / 2019-06-08
 */
#include "../../common/singleton.h"

#include "vid.h"
#include "char.h"
#include "event.h"
#include "sectree.h"
#include "dungeon.h"

#include <memory>
#include <set>
#include <queue>
#include <vector>

// spawn iþleri
// queue iþleri
// queue ve mm seçeneklerini iþlet sýnýrlandýrma fonksiyonlarýnda kullan
// itemshop iþleri

// ---------------------------------------------------- COMMON ---------------------------------------------
#define ENABLE_BATTLEGROUND_FULL_ACTIVE

#define BATTLEGROUND_NORMAL_MAP_INDEX 218

#define BATTLEGROUND_START_TIME_SEC 90
#define BATTLEGROUND_JOIN_COOLDOWN 1000 * 60 * 30 // 30 DK (todo)
#define BATTLEGROUND_ROOM_TIMEOUT_MSEC 1000 * 60 * 10 // 10 DK
#define BATTLEGROUND_TIMEOUT_MSEC 1000 * 60 * 10 // 40 DK (todo)

// Mob vnums
#define MINNION_SOLDIER		301 // Beyaz Yeminli Asker
#define MINNION_ARCHER		302 // Beyaz Yeminli Okçu
#define MINNION_COMMANDER	304 // Beyaz Yeminli Komutan
#define TOWER				2311 // Kýrmýzý Hayalet Aðaç
#define NEXUS_FINAL_BOSS	3890 // Kaptan þrak
#define INHIBITOR			7082 // Alev hayaleti

enum BattlegroundStatus : uint8_t
{
	BG_STATUS_NONE,
	BG_STATUS_FINISH,
	BG_STATUS_OPEN,
	BG_STATUS_STARTED,
	BG_STATUS_CLOSE
};

enum EBattlegroundTeamIDs : uint8_t
{
	BG_TEAM_NONE,
	BG_TEAM_BLUE,
	BG_TEAM_RED
};

enum EBattlegroundLaneIDs : uint8_t
{
	BG_LANE_NONE,
	BG_LANE_LEFT,
	BG_LANE_MID,
	BG_LANE_RIGHT
};

enum EBattlegroundMobIDs : uint8_t
{
	BG_MOB_NONE,
	BG_MOB_MINNION,
	BG_MOB_TOWER,
	BG_MOB_INHIBITOR,
	BG_MOB_NEXUS
#if 0
	BG_MOB_CAMP_1,
	BG_MOB_CAMP_2,
	BG_MOB_CAMP_3,
#endif
};

enum EBattlegroundGameMods : uint8_t
{
	BG_GAME_MODE_NONE,
	BG_GAME_MODE_FUN,
	BG_GAME_MODE_COMPETITIVE
};

enum EBattlegroundJoinMethods : uint8_t
{
	BG_JOIN_METHOD_NONE,
	BG_JOIN_METHOD_POOL,
	BG_JOIN_METHOD_LIST
};
#define BATTLEGROUND_DEFAULT_JOIN_METHOD BG_JOIN_METHOD_LIST

enum EBattlegroundQueueMethods : uint8_t
{
	BG_QUEUE_METHOD_NONE,
	BG_QUEUE_METHOD_BALANCE, // queue sayýsý yüksekse
	BG_QUEUE_METHOD_BLIND	 // default
};

enum EBattlegroundQueueTypes : uint8_t
{
	BG_QUEUE_TYPE_NONE,
	BG_QUEUE_TYPE_LOBBY,
	BG_QUEUE_TYPE_PARTY,
	BG_QUEUE_TYPE_GUILD
};

enum EBattlegroundMatchmakingTypes : uint8_t
{
	BG_MATCHMAKING_NONE,
	BG_MATCHMAKING_SOLO,
	BG_MATCHMAKING_DUO,
	BG_MATCHMAKING_TRIO,
	BG_MATCHMAKING_QUARTET,
	BG_MATCHMAKING_QUINTET	
};

class CBattleground;
typedef struct _BATTLEGROUND_CONTEXT
{
	int32_t nMapIndex;
	uint32_t dwEventVID;
	LPDUNGEON pkDungeon;
	CBattleground* pkBattleground;
} SBattlegroundCtx, *PBattlegroundCtx;

#define BATTLEGROUND_MAX_WAVE_COUNT 42
static int gs_iBattlegroundMinnionWaves[BATTLEGROUND_MAX_WAVE_COUNT][4] =
{
	{ 5, 		3, 3, 0}, // 00:05
	{ 30, 		2, 3, 1}, // 00:30
	{ 60, 		3, 3, 0}, // 01:00
	{ 90, 		3, 3, 0}, // 01:30
	{ 120, 		2, 3, 1}, // 02:00
	{ 150, 		3, 3, 0}, // 02:30
	{ 180, 		3, 3, 0}, // 03:00
	{ 210, 		2, 3, 1}, // 03:30
	{ 240, 		3, 3, 0}, // 04:00
	{ 270, 		3, 3, 0}, // 04:30
	{ 300, 		2, 3, 1}, // 05:00
	{ 330, 		3, 3, 0}, // 05:30
	{ 360, 		2, 3, 1}, // 06:00
	{ 390, 		3, 3, 0}, // 06:30
	{ 420, 		2, 3, 1}, // 07:00
	{ 450, 		3, 3, 0}, // 07:30
	{ 480, 		2, 3, 1}, // 08:00
	{ 510, 		3, 3, 0}, // 08:30
	{ 540, 		2, 3, 1}, // 09:00
	{ 570, 		3, 3, 0}, // 09:30
	{ 600, 		2, 3, 1}, // 10:00
	{ 630, 		2, 3, 1}, // 10:30
	{ 660, 		2, 3, 1}, // 11:00
	{ 690, 		2, 3, 1}, // 11:30
	{ 720, 		2, 3, 1}, // 12:00
	{ 750, 		2, 3, 1}, // 12:30
	{ 780, 		2, 3, 1}, // 13:00
	{ 810, 		2, 3, 1}, // 13:30
	{ 840, 		2, 3, 1}, // 14:00
	{ 870, 		2, 3, 1}, // 14:30
	{ 900, 		1, 3, 2}, // 15:00
	{ 930, 		1, 3, 2}, // 15:30
	{ 960, 		1, 3, 2}, // 16:00
	{ 990, 		1, 3, 2}, // 16:30
	{ 1020, 	1, 3, 2}, // 17:00
	{ 1050, 	1, 3, 2}, // 17:30
	{ 1080, 	1, 3, 2}, // 18:00
	{ 1110, 	1, 3, 2}, // 18:30
	{ 1140, 	1, 3, 2}, // 19:00
	{ 1170, 	1, 3, 2}, // 19:30
	{ 1200, 	0, 3, 3}, // 20:00
	{ 1230, 	0, 3, 3} // 20:30 (FINAL WAVE)
};

#define BATTLEGROUND_MAX_SPAWN_POSITION_COUNT 48
static int gs_iBattlegroundSpawnPositions[BATTLEGROUND_MAX_SPAWN_POSITION_COUNT][5] = 
{
	// Minnion
	{ BG_TEAM_BLUE, BG_LANE_LEFT, 	BG_MOB_MINNION, 190, 	90 },		// 0
	{ BG_TEAM_BLUE, BG_LANE_MID, 	BG_MOB_MINNION, 160, 	160 },		// 1
	{ BG_TEAM_BLUE, BG_LANE_RIGHT,	BG_MOB_MINNION, 90, 	190 },		// 2
	
	{ BG_TEAM_RED, 	BG_LANE_LEFT,	BG_MOB_MINNION, 800, 	920 },		// 3
	{ BG_TEAM_RED, 	BG_LANE_MID, 	BG_MOB_MINNION, 850, 	850 },		// 4
	{ BG_TEAM_RED, 	BG_LANE_RIGHT, 	BG_MOB_MINNION, 920, 	800 },		// 5

	// Camps
	{ 0, 0, 0, 0, 0},													// 6
	{ 0, 0, 0, 0, 0},													// 7
	{ 0, 0, 0, 0, 0},													// 8
	{ 0, 0, 0, 0, 0},													// 9
	{ 0, 0, 0, 0, 0},													// 10
	{ 0, 0, 0, 0, 0},													// 11
	{ 0, 0, 0, 0, 0},													// 12
	{ 0, 0, 0, 0, 0},													// 13
	{ 0, 0, 0, 0, 0},													// 14
	{ 0, 0, 0, 0, 0},													// 15

	// Crab
	{ 0, 0, 0, 0, 0},													// 16
	{ 0, 0, 0, 0, 0},													// 17

	// Dragon & baron
	{ 0, 0, 0, 0, 0},													// 18
	{ 0, 0, 0, 0, 0},													// 19

	// Towers
	{ BG_TEAM_BLUE, BG_LANE_LEFT, 	BG_MOB_TOWER, 		320, 	95 },	// 20
	{ BG_TEAM_BLUE, BG_LANE_LEFT, 	BG_MOB_TOWER, 		700, 	75 },	// 21

	{ BG_TEAM_BLUE, BG_LANE_MID, 	BG_MOB_TOWER, 		280, 	290 },	// 22
	{ BG_TEAM_BLUE, BG_LANE_MID, 	BG_MOB_TOWER, 		400, 	390 },	// 23

	{ BG_TEAM_BLUE, BG_LANE_RIGHT, 	BG_MOB_TOWER, 		75, 	700 },	// 24
	{ BG_TEAM_BLUE, BG_LANE_RIGHT, 	BG_MOB_TOWER, 		95, 	320 },	// 25

	{ BG_TEAM_RED, BG_LANE_LEFT, 	BG_MOB_TOWER, 		550, 	910 },	// 26
	{ BG_TEAM_RED, BG_LANE_LEFT, 	BG_MOB_TOWER, 		250, 	930 },	// 27

	{ BG_TEAM_RED, BG_LANE_MID, 	BG_MOB_TOWER, 		640, 	644 },	// 28
	{ BG_TEAM_RED, BG_LANE_MID, 	BG_MOB_TOWER, 		800, 	790 },	// 29

	{ BG_TEAM_RED, BG_LANE_RIGHT, 	BG_MOB_TOWER, 		930, 	340 },	// 30
	{ BG_TEAM_RED, BG_LANE_RIGHT, 	BG_MOB_TOWER, 		910, 	720 },	// 31

	// Inhibitor & tower guards
	{ BG_TEAM_BLUE, BG_LANE_LEFT, 	BG_MOB_TOWER, 		150, 	92 },	// 32
	{ BG_TEAM_BLUE, BG_LANE_LEFT, 	BG_MOB_INHIBITOR, 	125, 	92 },	// 33

	{ BG_TEAM_BLUE, BG_LANE_RIGHT, 	BG_MOB_TOWER, 		92, 	150 },	// 34
	{ BG_TEAM_BLUE, BG_LANE_RIGHT, 	BG_MOB_INHIBITOR, 	92, 	125 },	// 35

	{ BG_TEAM_RED, BG_LANE_LEFT, 	BG_MOB_TOWER, 		860, 	920 },	// 36
	{ BG_TEAM_RED, BG_LANE_LEFT, 	BG_MOB_INHIBITOR, 	885, 	920 },	// 37

	{ BG_TEAM_RED, BG_LANE_RIGHT, 	BG_MOB_TOWER, 		920, 	860 },	// 38
	{ BG_TEAM_RED, BG_LANE_RIGHT, 	BG_MOB_INHIBITOR, 	920, 	885 },	// 39

	// Nexus & tower guards
	{ BG_TEAM_BLUE, BG_LANE_NONE, 	BG_MOB_TOWER, 	120, 	120 },		// 40
	{ BG_TEAM_BLUE, BG_LANE_NONE, 	BG_MOB_TOWER, 	120, 	100 },		// 41
	{ BG_TEAM_BLUE, BG_LANE_NONE, 	BG_MOB_TOWER, 	100, 	120 },		// 42
	{ BG_TEAM_BLUE, BG_LANE_NONE, 	BG_MOB_NEXUS, 	100, 	100 },		// 43

	{ BG_TEAM_RED, BG_LANE_NONE, 	BG_MOB_TOWER, 	895, 	895 },		// 44
	{ BG_TEAM_RED, BG_LANE_NONE, 	BG_MOB_TOWER, 	895, 	915 },		// 45
	{ BG_TEAM_RED, BG_LANE_NONE, 	BG_MOB_TOWER, 	915, 	895 },		// 46
	{ BG_TEAM_RED, BG_LANE_NONE, 	BG_MOB_NEXUS, 	915, 	915 },		// 47
};

 // ---------------------------------------------------- TEAM -----------------------------------------------

class CBattlegroundTeam
{
public:
	~CBattlegroundTeam() = default;
	CBattlegroundTeam() = default;
	CBattlegroundTeam(uint8_t nTeamID, CBattleground* pkBattleground, LPSECTREE_MAP pkSectreeMap, int32_t nMapIndex);

	bool 		IsTeamID(uint8_t nTeamID) const;
	uint8_t 	GetTeamID() const;

	void 		Enter(uint32_t dwPlayerID);
	
	bool 		IsAttender(uint32_t dwPlayerID) const;
	uint32_t 	GetAttenderCount() const;

	bool		IsOwnMob(uint32_t dwVID) const;
	bool		IsOwnMinnion(uint32_t dwVID) const;

	LPCHARACTER	GetNexus();

	LPCHARACTER Spawn(uint8_t nMobType, uint8_t nLaneID, uint32_t dwVnum, int32_t iX, int32_t iY, int32_t iDir, bool bSpawnMotion, bool bShow = true);

private:
	uint8_t 		m_nTeamID;
	int32_t 		m_nMapIndex;
	LPSECTREE_MAP	m_pkSectreeMap;
	CBattleground*	m_pkBattleground;

	std::vector <uint32_t /* dwPlayerID */> m_vTeamMembers;
	std::map	<LPCHARACTER /* pkMob */, uint8_t /* nMobType */>	m_pkMapSpawnedMobs;
};

// ------------------------------------------------- MIDDLEWARE --------------------------------------------

class CBattleground
{
public:
	CBattleground(int32_t nMapIndex, uint8_t nGameMode, uint8_t nGameType, uint8_t nQueueType);
	~CBattleground() = default;

	bool 		Initialize();
	void 		Destroy();

	bool 		Enter(LPCHARACTER pkChar, uint8_t nTeamID);
	bool 		EnterSpectator(LPCHARACTER pkChar) const;
	void 		Logout(LPCHARACTER pkChar);

	void 		SpawnMinnions(uint32_t dwPassedSec);
	void		SpawnFirstWaveMobs();
	
	bool 		Start();
	void 		Close();
	bool 		IsStarted() const;

	void 		SendNotice(const char* c_szFormat, ...);
	void		ExitAllMembers();
	void		SendPacket(const void* data, int32_t size);
	void		GoToHome(LPCHARACTER pkChar);
	LPCHARACTER Spawn(uint8_t nTeamID, uint8_t nMobType, uint8_t nLaneID, uint32_t dwVnum, int32_t iX, int32_t iY, int32_t iDir, bool bSpawnMotion, LPCHARACTER pkProtegeMob);

	bool		IsAttender(uint32_t dwPlayerID) const;
	uint32_t 	GetAttenderCount(uint8_t nTeamID) const;
	uint8_t		GetAttenderTeamID(uint32_t dwPlayerID) const;

	uint8_t		GetGameMode() const;
	uint8_t		GetGameType() const;
	uint8_t		GetQueueType() const;
	uint32_t	GetSetupTime() const;
	uint32_t	GetStartTime() const;
	uint32_t 	GetVID() const;
	int32_t 	GetMapIndex() const;
	uint32_t 	GetSpectatorCount() const;
	LPCHARACTER	GetOurNexus(uint8_t nTeamID) const;
	LPCHARACTER	GetEnemyNexus(uint8_t nTeamID) const;

	uint8_t		GetBattlegroundMobTeamID(uint32_t dwVID) const;
	uint8_t		GetBattlegroundAttenderTeamID(uint32_t dwPlayerID) const;

	void		SetNextMinnionWaveIndex();
	uint8_t		GetMinnionWaveIndex() const;

	void		SetLeaderName(const std::string& strName);
	std::string	GetLeaderName() const;
		
	void		SetParticipantCount(uint8_t nParticipantCount);
	uint8_t		GetParticipantCount() const;

	void		SetParticipantLimit(uint8_t nParticipantLimit);
	uint8_t		GetParticipantLimit() const;

	void 		SetRoomID(uint32_t dwRoomID);
	uint32_t 	GetRoomID() const;

	void		SetDungeon(LPDUNGEON pkDungeon);
	LPDUNGEON	GetDungeon() const;

	LPSECTREE_MAP &	GetMapSectree();
	GPOS 		NormalizePosition(GPOS pos);

protected:
	void __CreateVID();

	bool __SwitchToSpectator(LPCHARACTER pkChar) const;
	void __ProcessPlayer(LPCHARACTER pkChar);

	void __SpawnRandomPos(LPCHARACTER pkChar);

private:
	uint32_t	m_dwSetupTimestamp;
	uint32_t 	m_dwBattlegroundVID;
	int32_t		m_nMapIndex;
	bool		m_bStarted;
	uint8_t		m_nMinnionWaveIndex;
	uint32_t	m_dwRoomID;
	uint32_t	m_dwStartTime;

	uint8_t 	m_nGameMode;
	uint8_t 	m_nGameType;
	uint8_t 	m_nQueueType;
	std::string m_strLeaderName;
	uint8_t 	m_nParticipantCount;
	uint8_t 	m_nParticipantLimit;

	std::unique_ptr <CBattlegroundTeam>		m_upTeamBlue;
	std::unique_ptr <CBattlegroundTeam>		m_upTeamRed;
	std::vector		<uint32_t /* dwPID */>	m_vSpecatators;
	std::list		<LPCHARACTER>			m_pkLstSpawnedMobs;
	std::list		<LPCHARACTER>			m_pkLstAttenders;

	LPSECTREE_MAP	m_pkSectreeMap;
	LPSECTREE		m_pkSectree;
	LPDUNGEON		m_pkDungeon;

	LPEVENT m_pkBattlegroundManagmentEvent;
	LPEVENT m_pkMinnionSpawnEvent;
};

// -------------------------------------------------- QUEUE ---------------------------------------------

typedef struct _BATTLEGROUND_QUEUE_CONTEXT
{

} SBattlegroundQueCtx, *PBattlegroundQueCtx;

class CBattlegroundQueue // todo
{
public:
	CBattlegroundQueue();
	~CBattlegroundQueue() = default;

	void Initialize();
	void Destroy();

	bool JoinToQueue(uint32_t dwPlayerID, uint8_t nGameMode, uint8_t nQueueType, uint8_t nMatchMakingType);
	void RemoveFromQueue(uint32_t dwPlayerID); // call from CHARACTER::Logout
	bool CanJoinToQueue(uint32_t dwPlayerID) const;
	bool InQueue(uint32_t dwPlayerID) const;

	uint32_t GetQueueSize() const;

	void ProcessQueue();

private:
	std::list <uint32_t /* dwPlayerID */> m_pkLstAttenders;
};

class CBattlegroundRoom
{
public:
	~CBattlegroundRoom() = default;
	CBattlegroundRoom(uint8_t nGameMode, uint8_t nGameType, uint8_t nQueueType);

	bool		StartBattleground();
	void		DestroyRoom(); // todo call when current battleground is completed
	
	void		SetRoomID(uint32_t dwRoomID);
	uint32_t	GetRoomID() const;

	bool		JoinToRoom(uint32_t dwPlayerID, uint8_t nTeamID);
	void		LeaveFromRoom(uint32_t dwPlayerID); // todo call from logout
	bool		IsAttender(uint32_t dwPlayerID) const;
	auto		GetAttenderList() const { return m_pkMapAttenders; };
	uint32_t	GetAttenderCount() const;

	bool		KickPlayer(const std::string& strName);

	void		SetLeaderPlayerID(uint32_t dwLeaderPlayerID);
	uint32_t	GetLeaderPlayerID() const;
	std::string	GetLeaderName() const;

	uint8_t		GetGameMode() const;
	uint8_t		GetGameType() const;
	uint8_t		GetQueueType() const;
	uint8_t		GetRoomLimit() const;
	uint32_t	GetSetupTime() const;
	bool		IsInGame() const;

	void		SetAttenderState(uint32_t dwPlayerID, bool bNewState);
	bool		GetAttenderState(uint32_t dwPlayerID) const;

	void		SendRoomDetails(LPCHARACTER pkChar);

	void		SetTeamID(uint32_t dwPlayerID, uint8_t nTeamID);
	uint8_t		GetTeamID(uint32_t dwPlayerID) const;

private:
	uint32_t 	m_dwSetupTime;
	uint32_t 	m_dwRoomID;
	bool		m_bInGame;
	bool		m_bIsLocked;

	std::map <uint32_t /* dwPlayerID */, uint8_t /* nTeamID */> 	m_pkMapAttenders;
	std::map <uint32_t /* dwPlayerID */, bool /* bState */> 		m_pkMapAttenderStates;
	std::map <uint32_t /* dwPlayerID */, std::string /* stName */> 	m_pkMapAttenderNames;

	uint32_t 	m_dwLeaderPlayerID;
	std::string m_strLeaderName;
	uint8_t 	m_nGameMode;
	uint8_t 	m_nGameType;
	uint8_t 	m_nQueueType;

	std::set <std::string> m_pkSetKickedPlayers;

	CBattleground* 	m_pkBattleground;
	LPDUNGEON		m_pkDungeon;
	int32_t 		m_nBattlegroundMapIndex;
	uint32_t 		m_dwBattlegroundVID;
};

// ------------------------------------------------- MANAGER --------------------------------------------

class CBattlegroundManager : public singleton <CBattlegroundManager>
{
public:
	CBattlegroundManager();
	~CBattlegroundManager() = default;

	// Manager
	void Initialize();
	void Destroy();

	void SetStarted(bool bStatus);
	bool IsStarted() const;

	void BlockCreateBattleground();
	bool IsBattlegroundCreateBlocked() const;

	// Battleground
	SBattlegroundCtx* CreateBattleground(uint8_t nGameMode, uint8_t nGameType, uint8_t nQueueType);
	bool DeleteBattleground(uint32_t dwEventVID);
	bool HasBattleground() const;

	bool StartEvent(uint32_t dwVID) const;
	bool CloseEvent(uint32_t dwVID) const;
	
	bool IsEventVID(uint32_t dwVID) const;
	bool IsEventStarted(uint32_t dwVID) const;
	bool IsEventMap(int32_t nMapIndex) const;

	CBattleground* FindBattleground(uint32_t dwVID) const;
	CBattleground* FindBattlegroundByAttender(uint32_t dwPlayerID) const;
	CBattleground* FindBattlegroundByMapIndex(int32_t nMapIndex) const;

	void SearchBattleground(LPCHARACTER ch, uint8_t nGameMode, uint8_t nGameType, uint8_t nQueueType);

	// Battleground Room
	uint32_t CreateBattlegroundRoom(LPCHARACTER pkLeaderChar, uint8_t nGameMode, uint8_t nGameType, uint8_t nQueueType);
	void DeleteBattlegroundRoom(uint32_t dwRoomID);

	CBattlegroundRoom* FindBattlegroundRoom(uint32_t dwRoomID) const;
	CBattlegroundRoom* FindBattlegroundRoomByLeader(uint32_t dwLeaderPID) const;
	CBattlegroundRoom* FindBattlegroundRoomByAttender(uint32_t dwPlayerID) const;

	void CheckRoomTimeouts();

	// Battleground queue
	bool JoinBattlegroundQueue(uint32_t dwPlayerID, uint8_t nGameMode, uint8_t nGameType, uint8_t nQueueType) const;
	void LeaveFromBattlegroundQueue(uint32_t dwPlayerID);

	// Character callbacks
	void OnRevive(LPCHARACTER pkChar, int32_t nDeadTime);
	void OnLogin(LPCHARACTER pkChar);
	void OnLogout(LPCHARACTER pkChar);
	void OnAffectLoad(LPCHARACTER pkChar);
	void OnKill(LPCHARACTER pkKiller, LPCHARACTER pkTarget);
	int32_t OnDamage(LPCHARACTER pkAttacker, LPCHARACTER pkVictim, int32_t nOldDamage); // return: calculated new damage
	void OnSwitchIdle(LPCHARACTER pkChar);
	uint16_t GetMobAttackRange(uint16_t wRaceNum);

	// Event timer callbacks
	void OnQueueTick();
	void OnEventTimerTick();

	// Utilities
	bool IsLimitedItem(uint32_t dwVnum) const;
	void WarpToBattlegroundMap(LPCHARACTER pkChar, uint32_t dwVID, uint8_t nTeamID);

	// Setter & Getter
	void SetBattlegroundJoinType(uint8_t nJoinType);
	uint8_t GetBattlegroundJoinType() const;

	void SaveBattlegroundRequiredItems(uint32_t dwReqItem1Vnum, uint32_t dwReqItem2Vnum, uint32_t dwReqItem3Vnum);
	std::array <uint32_t, 3> GetBattlegroundRequiredItems() const;

private:
	bool									m_bStarted;
	bool									m_bBlockCreateBattleground;
	std::set <CBattleground*> 				m_pkSetBattlegroundContainer;
	std::unique_ptr <CBattlegroundQueue>	m_upBattlegroundQueue;
	LPEVENT									m_pkBattlegroundQueueWorker;
	LPEVENT									m_pkBattlegroundEventTimer;
	uint8_t									m_nBattlegroundJoinType;
	std::array <uint32_t, 3>				m_arrBattlegroundRequiredItems;
	uint32_t								m_dwLastRoomID;
	std::set <CBattlegroundRoom*>			m_pkSetBattlegroundRoomContainer;
	LPEVENT									m_pkBattlegroundRoomTimeoutCheck;
};

