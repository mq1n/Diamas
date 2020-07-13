#pragma once
#include "../../../common/stl.h"
#include "../../../common/defines.h"

#ifdef M2_USE_POOL
#include "pool.h"
#endif

#include "packet.h"
#include "vid.h"
#include <unordered_map>

class CDungeon;
class CHARACTER;
class CharacterVectorInteractor;
class CharacterSetSnapshot;

class CHARACTER_MANAGER : public CSingleton<CHARACTER_MANAGER>
{
	public:
		typedef std::unordered_map<std::string, LPCHARACTER> NAME_MAP;

		CHARACTER_MANAGER();
		virtual ~CHARACTER_MANAGER();

		void                    Destroy();

		void			GracefulShutdown();	// 정상적 셧다운할 때 사용. PC를 모두 저장시키고 Destroy 한다.

		uint32_t			AllocVID();

		LPCHARACTER             CreateCharacter(std::string stName, uint32_t dwPID = 0);
#ifndef DEBUG_ALLOC
		void DestroyCharacter(LPCHARACTER ch);
#else
		void DestroyCharacter(LPCHARACTER ch, const char* file, size_t line);
#endif

		void			Update(int32_t iPulse);

		LPCHARACTER		SpawnMob(uint32_t dwVnum, int32_t lMapIndex, int32_t x, int32_t y, int32_t z, bool bSpawnMotion = false, int32_t iRot = -1, bool bShow = true, bool bBattleground = false);
		LPCHARACTER		SpawnMobRange(uint32_t dwVnum, int32_t lMapIndex, int32_t sx, int32_t sy, int32_t ex, int32_t ey, bool bIsException=false, bool bSpawnMotion = false , bool bAggressive = false);
		LPCHARACTER		SpawnGroup(uint32_t dwVnum, int32_t lMapIndex, int32_t sx, int32_t sy, int32_t ex, int32_t ey, LPREGEN pkRegen = nullptr, bool bAggressive_ = false, LPDUNGEON pDungeon = nullptr, bool bRandom = false);
		bool			SpawnGroupGroup(uint32_t dwVnum, int32_t lMapIndex, int32_t sx, int32_t sy, int32_t ex, int32_t ey, LPREGEN pkRegen = nullptr, bool bAggressive_ = false, LPDUNGEON pDungeon = nullptr);
		bool			SpawnMoveGroup(uint32_t dwVnum, int32_t lMapIndex, int32_t sx, int32_t sy, int32_t ex, int32_t ey, int32_t tx, int32_t ty, LPREGEN pkRegen = nullptr, bool bAggressive_ = false);
		LPCHARACTER		SpawnMobRandomPosition(uint32_t dwVnum, int32_t lMapIndex);

		void			SelectStone(LPCHARACTER pkChrStone);

		NAME_MAP &		GetPCMap() { return m_map_pkPCChr; }

		LPCHARACTER		Find(uint32_t dwVID);
		LPCHARACTER		Find(const VID & vid);
		LPCHARACTER		FindPC(std::string stName);
		LPCHARACTER		FindByPID(uint32_t dwPID);

		bool			AddToStateList(LPCHARACTER ch);
		void			RemoveFromStateList(LPCHARACTER ch);

		// DelayedSave: 어떠한 루틴 내에서 저장을 해야 할 짓을 많이 하면 저장
		// 쿼리가 너무 많아지므로 "저장을 한다" 라고 표시만 해두고 잠깐
		// (예: 1 frame) 후에 저장시킨다.
		void                    DelayedSave(LPCHARACTER ch);
		bool                    FlushDelayedSave(LPCHARACTER ch); // Delayed 리스트에 있다면 지우고 저장한다. 끊김 처리시 사용 됨.
		void			ProcessDelayedSave();

		template<class Func>	Func for_each_pc(Func f);

		void			KillLog(uint32_t dwVnum);

		void			RegisterRaceNum(uint32_t dwVnum);
		void			RegisterRaceNumMap(LPCHARACTER ch);
		void			UnregisterRaceNumMap(LPCHARACTER ch);
		CharacterSetSnapshot	GetCharactersByRaceNum(uint32_t dwRaceNum);
		int32_t					CountCharactersByRaceNum(uint32_t dwRaceNum);

		LPCHARACTER		FindSpecifyPC(uint32_t uiJobFlag, int32_t lMapIndex, LPCHARACTER except=nullptr, int32_t iMinLevel = 1, int32_t iMaxLevel = PLAYER_MAX_LEVEL_CONST);

		void			SetMobItemRate(int32_t value)	{ m_iMobItemRate = value; }
		void			SetMobDamageRate(int32_t value)	{ m_iMobDamageRate = value; }
		void			SetMobGoldAmountRate(int32_t value)	{ m_iMobGoldAmountRate = value; }
		void			SetMobGoldDropRate(int32_t value)	{ m_iMobGoldDropRate = value; }
		void			SetMobExpRate(int32_t value)	{ m_iMobExpRate = value; }

		void			SetMobItemRatePremium(int32_t value)	{ m_iMobItemRatePremium = value; }
		void			SetMobGoldAmountRatePremium(int32_t value)	{ m_iMobGoldAmountRatePremium = value; }
		void			SetMobGoldDropRatePremium(int32_t value)	{ m_iMobGoldDropRatePremium = value; }
		void			SetMobExpRatePremium(int32_t value)		{ m_iMobExpRatePremium = value; }

		void			SetUserDamageRatePremium(int32_t value)	{ m_iUserDamageRatePremium = value; }
		void			SetUserDamageRate(int32_t value ) { m_iUserDamageRate = value; }
		int32_t			GetMobItemRate(LPCHARACTER ch) const;
		int32_t			GetMobDamageRate(LPCHARACTER ch) const;
		int32_t			GetMobGoldAmountRate(LPCHARACTER ch) const;
		int32_t			GetMobGoldDropRate(LPCHARACTER ch) const;
		int32_t			GetMobExpRate(LPCHARACTER ch) const;

		int32_t			GetUserDamageRate(LPCHARACTER ch) const;
		void		SendScriptToMap(int32_t lMapIndex, const std::string & s); 

		bool			BeginPendingDestroy();
		void			FlushPendingDestroy();

	private:
		int32_t					m_iMobItemRate;
		int32_t					m_iMobDamageRate;
		int32_t					m_iMobGoldAmountRate;
		int32_t					m_iMobGoldDropRate;
		int32_t					m_iMobExpRate;

		int32_t					m_iMobItemRatePremium;
		int32_t					m_iMobGoldAmountRatePremium;
		int32_t					m_iMobGoldDropRatePremium;
		int32_t					m_iMobExpRatePremium;

		int32_t					m_iUserDamageRate;
		int32_t					m_iUserDamageRatePremium;
		int32_t					m_iVIDCount;

		std::unordered_map<uint32_t, LPCHARACTER> m_map_pkChrByVID;
		std::unordered_map<uint32_t, LPCHARACTER> m_map_pkChrByPID;
		NAME_MAP			m_map_pkPCChr;
		std::unordered_set <LPCHARACTER>			m_map_pkBGMob;

		char				dummy1[1024];	// memory barrier
		CHARACTER_SET		m_set_pkChrState;	// FSM이 돌아가고 있는 놈들
		CHARACTER_SET		m_set_pkChrForDelayedSave;

		LPCHARACTER			m_pkChrSelectedStone;

		std::map<uint32_t, uint32_t> m_map_dwMobKillCount;

		std::set<uint32_t>		m_set_dwRegisteredRaceNum;
		std::map<uint32_t, CHARACTER_SET> m_map_pkChrByRaceNum;

		bool				m_bUsePendingDestroy;
		CHARACTER_SET		m_set_pkChrPendingDestroy;

#ifdef M2_USE_POOL
		ObjectPool<CHARACTER> pool_;
#endif
};

	template<class Func>	
Func CHARACTER_MANAGER::for_each_pc(Func f)
{
	std::unordered_map<uint32_t, LPCHARACTER>::iterator it;

	for (it = m_map_pkChrByPID.begin(); it != m_map_pkChrByPID.end(); ++it)
		f(it->second);

	return f;
}

class CharacterSnapshotGuard
{
public:
	CharacterSnapshotGuard();
	~CharacterSnapshotGuard();

private:
	bool m_hasPendingOwnership;
};

class CharacterSetSnapshot
{
public:
	CharacterSetSnapshot();
	CharacterSetSnapshot(const std::unordered_set<CHARACTER*>& chars);

	std::unordered_set<CHARACTER*>::const_iterator begin() const;
	std::unordered_set<CHARACTER*>::const_iterator end() const;
	bool empty() const
	{
		return !m_chars || m_chars->empty();
	}

private:
	CharacterSnapshotGuard m_guard;
	const std::unordered_set<CHARACTER*>* m_chars;
};

#ifndef DEBUG_ALLOC
#define M2_DESTROY_CHARACTER(ptr) CHARACTER_MANAGER::Instance().DestroyCharacter(ptr)
#else
#define M2_DESTROY_CHARACTER(ptr) CHARACTER_MANAGER::Instance().DestroyCharacter(ptr, __FILE__, __LINE__)
#endif
