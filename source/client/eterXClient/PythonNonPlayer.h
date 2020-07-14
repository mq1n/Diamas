#pragma once
#include "StdAfx.h"
#include <list>

class CPythonNonPlayer : public CSingleton<CPythonNonPlayer>
{
	public:
		using TMobTableList = std::list<TMobTable*>;
		using TNonPlayerDataMap = std::map<uint32_t, TMobTable>;

	public:
		CPythonNonPlayer();
		virtual ~CPythonNonPlayer();

		void Clear();
		void Destroy();

		bool				LoadNonPlayerData(const char * c_szFileName);

		const TMobTable *	GetTable(uint32_t dwVnum);
		bool				GetName(uint32_t dwVnum, const char ** c_pszName);
		bool				GetInstanceType(uint32_t dwVnum, uint8_t* pbType);
		uint8_t				GetEventType(uint32_t dwVnum);
		uint8_t				GetEventTypeByVID(uint32_t dwVID);
		uint32_t				GetMonsterColor(uint32_t dwVnum);
		const char*			GetMonsterName(uint32_t dwVnum);

#if defined(WJ_SHOW_MOB_INFO) && defined(ENABLE_SHOW_MOBLEVEL)
		uint32_t				GetMonsterLevel(uint32_t dwVnum);
#endif

#if defined(WJ_SHOW_MOB_INFO) && defined(ENABLE_SHOW_MOBAIFLAG)
		bool				IsAggressive(uint32_t dwVnum);
#endif

		// Function for outer
		void				GetMatchableMobList(int32_t iLevel, int32_t iInterval, TMobTableList * pMobTableList);
		
		uint32_t				GetMonsterMaxHP(uint32_t dwVnum);
		uint32_t				GetMonsterRaceFlag(uint32_t dwVnum);
		uint32_t				GetMonsterDamage1(uint32_t dwVnum);
		uint32_t				GetMonsterDamage2(uint32_t dwVnum);
		uint32_t				GetMonsterExp(uint32_t dwVnum);
		float				GetMonsterDamageMultiply(uint32_t dwVnum);
		uint32_t				GetMonsterST(uint32_t dwVnum);
		uint32_t				GetMonsterDX(uint32_t dwVnum);
		bool				IsMonsterStone(uint32_t dwVnum);
		uint8_t				GetMobRegenCycle(uint32_t dwVnum);
		uint8_t				GetMobRegenPercent(uint32_t dwVnum);
		uint32_t				GetMobGoldMin(uint32_t dwVnum);
		uint32_t				GetMobGoldMax(uint32_t dwVnum);
		
	protected:
		TNonPlayerDataMap	m_NonPlayerDataMap;
};

