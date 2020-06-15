#include "StdAfx.h"
#include <FileSystemIncl.hpp>
#include "PythonNonPlayer.h"
#include "InstanceBase.h"
#include "PythonCharacterManager.h"
#include "../eterBase/lzo.h"

bool CPythonNonPlayer::LoadNonPlayerData(const char * c_szFileName)
{
	static uint32_t s_adwMobProtoKey[4] =
	{   
		4813894,
		18955,
		552631,
		6822045
	};

	Tracef("CPythonNonPlayer::LoadNonPlayerData: %s, sizeof(TMobTable)=%u\n", c_szFileName, sizeof(TMobTable));

	CFile file;
	if (!FileSystemManager::Instance().OpenFile(c_szFileName, file))
		return false;

	uint32_t dwFourCC, dwElements, dwDataSize;

	file.Read(&dwFourCC, sizeof(uint32_t));

	if (dwFourCC != MAKEFOURCC('M', 'M', 'P', 'T'))
	{
		TraceError("CPythonNonPlayer::LoadNonPlayerData: invalid Mob proto type %s", c_szFileName);
		return false;
	}

	file.Read(&dwElements, sizeof(uint32_t));
	file.Read(&dwDataSize, sizeof(uint32_t));

	std::vector<uint8_t> pbData(dwDataSize);
	file.Read(pbData.data(), dwDataSize);
	/////

	CLZObject zObj;

	if (!CLZO::Instance().Decompress(zObj, pbData.data(), s_adwMobProtoKey))
		return false;

	uint32_t structSize = zObj.GetSize() / dwElements;
	uint32_t structDiff = zObj.GetSize() % dwElements;
#ifdef ENABLE_PROTOSTRUCT_AUTODETECT
	if (structDiff != 0 && !TMobTableAll::IsValidStruct(structSize))
#else
	if ((zObj.GetSize() % sizeof(TMobTable)) != 0)
#endif
	{
		TraceError("CPythonNonPlayer::LoadNonPlayerData: invalid size %u check data format. structSize %u, structDiff %u", zObj.GetSize(),
				   structSize, structDiff);
		return false;
	}

	for (uint32_t i = 0; i < dwElements; ++i)
	{
#ifdef ENABLE_PROTOSTRUCT_AUTODETECT
		TMobTable t = {0};
		TMobTableAll::Process(zObj.GetBuffer(), structSize, i, t);
#else
		CPythonNonPlayer::TMobTable & t = *((CPythonNonPlayer::TMobTable *) zObj.GetBuffer() + i);
#endif
		m_NonPlayerDataMap.emplace(t.dwVnum, t);
	}

	return true;
}

bool CPythonNonPlayer::GetName(uint32_t dwVnum, const char ** c_pszName)
{
	const TMobTable * p = GetTable(dwVnum);

	if (!p)
		return false;

	*c_pszName = p->szLocaleName;

	return true;
}

bool CPythonNonPlayer::GetInstanceType(uint32_t dwVnum, uint8_t* pbType)
{
	const TMobTable * p = GetTable(dwVnum);

	// dwVnum를 찾을 수 없으면 플레이어 캐릭터로 간주 한다. 문제성 코드 -_- [cronan]
	if (!p)
		return false;

	*pbType=p->bType;
	
	return true;
}

const CPythonNonPlayer::TMobTable * CPythonNonPlayer::GetTable(uint32_t dwVnum)
{
	auto itor = m_NonPlayerDataMap.find(dwVnum);
	if (itor == m_NonPlayerDataMap.end())
		return nullptr;

	return &itor->second;
}

uint8_t CPythonNonPlayer::GetEventType(uint32_t dwVnum)
{
	const TMobTable * p = GetTable(dwVnum);

	if (!p)
	{
		//Tracef("CPythonNonPlayer::GetEventType - Failed to find virtual number\n");
		return ON_CLICK_EVENT_NONE;
	}

	return p->bOnClickType;
}

#if defined(WJ_SHOW_MOB_INFO) && defined(ENABLE_SHOW_MOBLEVEL)
uint32_t CPythonNonPlayer::GetMonsterLevel(uint32_t dwVnum)
{
	const CPythonNonPlayer::TMobTable * c_pTable = GetTable(dwVnum);
	if (!c_pTable)
		return 0;

	return c_pTable->bLevel;
}
#endif

#if defined(WJ_SHOW_MOB_INFO) && defined(ENABLE_SHOW_MOBAIFLAG)
bool CPythonNonPlayer::IsAggressive(uint32_t dwVnum)
{
	const CPythonNonPlayer::TMobTable * c_pTable = GetTable(dwVnum);
	if (!c_pTable)
		return 0;

	return (IS_SET(c_pTable->dwAIFlag, AIFLAG_AGGRESSIVE));
}
#endif

uint8_t CPythonNonPlayer::GetEventTypeByVID(uint32_t dwVID)
{
	CInstanceBase * pInstance = CPythonCharacterManager::Instance().GetInstancePtr(dwVID);

	if (nullptr == pInstance)
	{
		//Tracef("CPythonNonPlayer::GetEventTypeByVID - There is no Virtual Number\n");
		return ON_CLICK_EVENT_NONE;
	}

	uint32_t dwVnum = pInstance->GetVirtualNumber();
	return GetEventType(dwVnum);
}

const char*	CPythonNonPlayer::GetMonsterName(uint32_t dwVnum)
{	
	const TMobTable * c_pTable = GetTable(dwVnum);
	if (!c_pTable)
		return "";

	return c_pTable->szLocaleName;
}

uint32_t CPythonNonPlayer::GetMonsterColor(uint32_t dwVnum)
{
	const TMobTable * c_pTable = GetTable(dwVnum);
	if (!c_pTable)
		return 0;

	return c_pTable->dwMonsterColor;
}
void CPythonNonPlayer::GetMatchableMobList(int32_t iLevel, int32_t iInterval, TMobTableList * pMobTableList)
{
/*
	pMobTableList->clear();

	TNonPlayerDataMap::iterator itor = m_NonPlayerDataMap.begin();
	for (; itor != m_NonPlayerDataMap.end(); ++itor)
	{
		TMobTable * pMobTable = itor->second;

		int32_t iLowerLevelLimit = iLevel-iInterval;
		int32_t iUpperLevelLimit = iLevel+iInterval;

		if ((pMobTable->abLevelRange[0] >= iLowerLevelLimit && pMobTable->abLevelRange[0] <= iUpperLevelLimit) ||
			(pMobTable->abLevelRange[1] >= iLowerLevelLimit && pMobTable->abLevelRange[1] <= iUpperLevelLimit))
		{
			pMobTableList->push_back(pMobTable);
		}
	}
*/
}

uint32_t CPythonNonPlayer::GetMonsterMaxHP(uint32_t dwVnum)
{
	const TMobTable * c_pTable = GetTable(dwVnum);
	if (!c_pTable)
	{
		return 0;
	}

	return c_pTable->dwMaxHP;
}

uint32_t CPythonNonPlayer::GetMonsterRaceFlag(uint32_t dwVnum)
{
	const TMobTable * c_pTable = GetTable(dwVnum);
	if (!c_pTable)
	{
		return 0;
	}

	return c_pTable->dwRaceFlag;
}

uint32_t CPythonNonPlayer::GetMonsterDamage1(uint32_t dwVnum)
{
	const TMobTable * c_pTable = GetTable(dwVnum);
	if (!c_pTable)
	{
		return 0;
	}

	return c_pTable->dwDamageRange[0];
}

uint32_t CPythonNonPlayer::GetMonsterDamage2(uint32_t dwVnum)
{
	const TMobTable * c_pTable = GetTable(dwVnum);
	if (!c_pTable)
	{
		return 0;
	}

	return c_pTable->dwDamageRange[1];
}

uint32_t CPythonNonPlayer::GetMonsterExp(uint32_t dwVnum)
{
	const TMobTable * c_pTable = GetTable(dwVnum);
	if (!c_pTable)
	{
		return 0;
	}

	return c_pTable->dwExp;
}

float CPythonNonPlayer::GetMonsterDamageMultiply(uint32_t dwVnum)
{
	const TMobTable * c_pTable = GetTable(dwVnum);
	if (!c_pTable)
	{
		return 0.f;
	}

	return c_pTable->fDamMultiply;
}

uint32_t CPythonNonPlayer::GetMonsterST(uint32_t dwVnum)
{
	const TMobTable * c_pTable = GetTable(dwVnum);
	if (!c_pTable)
	{
		return 0;
	}

	return c_pTable->bStr;
}

uint32_t CPythonNonPlayer::GetMonsterDX(uint32_t dwVnum)
{
	const TMobTable * c_pTable = GetTable(dwVnum);
	if (!c_pTable)
	{
		return 0;
	}

	return c_pTable->bDex;
}

bool CPythonNonPlayer::IsMonsterStone(uint32_t dwVnum)
{
	const TMobTable * c_pTable = GetTable(dwVnum);
	if (!c_pTable)
		return false;

	return c_pTable->bType == 2;
}

uint8_t CPythonNonPlayer::GetMobRegenCycle(uint32_t dwVnum)
{
	const TMobTable* c_pTable = GetTable(dwVnum);
	if (!c_pTable)
		return 0;

	return c_pTable->bRegenCycle;
}

uint8_t CPythonNonPlayer::GetMobRegenPercent(uint32_t dwVnum)
{
	const TMobTable* c_pTable = GetTable(dwVnum);
	if (!c_pTable)
		return 0;

	return c_pTable->bRegenPercent;
}

uint32_t CPythonNonPlayer::GetMobGoldMin(uint32_t dwVnum)
{
	const TMobTable* c_pTable = GetTable(dwVnum);
	if (!c_pTable)
		return 0;

	return c_pTable->dwGoldMin;
}

uint32_t CPythonNonPlayer::GetMobGoldMax(uint32_t dwVnum)
{
	const TMobTable* c_pTable = GetTable(dwVnum);
	if (!c_pTable)
		return 0;

	return c_pTable->dwGoldMax;
}


void CPythonNonPlayer::Clear()
{
}

void CPythonNonPlayer::Destroy()
{
	m_NonPlayerDataMap.clear();
}

CPythonNonPlayer::CPythonNonPlayer()
{
	Clear();
}

CPythonNonPlayer::~CPythonNonPlayer()
{
	Destroy();
}

