#include "StdAfx.h"
#include "../eterLib/ResourceManager.h"
#include "../eterLib/AttributeInstance.h"
#include "../eterBase/Utils.h"
#include "RaceData.h"
#include "RaceMotionData.h"
#include "../eterBase/Filename.h"
#include "../eterXClient/locale_inc.h"

CDynamicPool<CRaceData> CRaceData::ms_kPool;
CDynamicPool<CRaceData::TMotionModeData> CRaceData::ms_MotionModeDataPool;

const std::string& CRaceData::GetSmokeBone()
{
	return m_strSmokeBoneName;
}

uint32_t CRaceData::GetSmokeEffectID(uint32_t eSmoke)
{
	if(eSmoke>=SMOKE_NUM)
	{
		TraceError("CRaceData::GetSmokeEffectID(eSmoke=%d)", eSmoke);
		return 0;
	}

	return m_adwSmokeEffectID[eSmoke];
}

CRaceData::SHair* CRaceData::FindHair(uint32_t eHair)
{
	auto f = m_kMap_dwHairKey_kHair.find(eHair);
	if (m_kMap_dwHairKey_kHair.end()==f)
	{
		if (eHair != 0)
			TraceError("Hair number %d is not exist.",eHair);
		return nullptr;
	}

	return &f->second;
}

void CRaceData::SetHairSkin(uint32_t eHair, uint32_t ePart, const char * c_szModelFileName, const char* c_szSrcFileName, const char* c_szDstFileName)
{
	SSkin kSkin;
	kSkin.m_ePart=ePart;
	kSkin.m_stSrcFileName=c_szSrcFileName;
	kSkin.m_stDstFileName=c_szDstFileName;

	CFileNameHelper::ChangeDosPath(kSkin.m_stSrcFileName);
	m_kMap_dwHairKey_kHair[eHair].m_kVct_kSkin.emplace_back(kSkin);
	m_kMap_dwHairKey_kHair[eHair].m_stModelFileName = c_szModelFileName;
}

CRaceData::SShape* CRaceData::FindShape(uint32_t eShape)
{
	auto f = m_kMap_dwShapeKey_kShape.find(eShape);
	if (m_kMap_dwShapeKey_kShape.end()==f)
		return nullptr;

	return &f->second;
}

void CRaceData::SetShapeModel(uint32_t eShape, const char* c_szModelFileName)
{
	m_kMap_dwShapeKey_kShape[eShape].m_stModelFileName=c_szModelFileName;
}

void CRaceData::AppendShapeSkin(uint32_t eShape, uint32_t ePart, const char* c_szSrcFileName, const char* c_szDstFileName)
{
	SSkin kSkin;
	kSkin.m_ePart=ePart;
	kSkin.m_stSrcFileName=c_szSrcFileName;
	kSkin.m_stDstFileName=c_szDstFileName;

	CFileNameHelper::ChangeDosPath(kSkin.m_stSrcFileName);
	m_kMap_dwShapeKey_kShape[eShape].m_kVct_kSkin.emplace_back(kSkin);
}

CRaceData* CRaceData::New()
{
	return ms_kPool.Alloc();
}

void CRaceData::Delete(CRaceData* pkRaceData)
{
	pkRaceData->Destroy();
	ms_kPool.Free(pkRaceData);
}

void CRaceData::CreateSystem(uint32_t uCapacity, uint32_t uMotModeCapacity)
{
	ms_MotionModeDataPool.Create(uMotModeCapacity);
	ms_kPool.Create(uCapacity);
}

void CRaceData::DestroySystem()
{
	ms_kPool.Destroy();
	ms_MotionModeDataPool.Destroy();
}

BOOL CRaceData::CreateMotionModeIterator(TMotionModeDataIterator & itor)
{
	if (m_pMotionModeDataMap.empty())
		return FALSE;

	itor = m_pMotionModeDataMap.begin();

	return TRUE;
}
BOOL CRaceData::NextMotionModeIterator(TMotionModeDataIterator & itor)
{
	++itor;

	return m_pMotionModeDataMap.end() != itor;
}

BOOL CRaceData::GetMotionKey(uint16_t wMotionModeIndex, uint16_t wMotionIndex, MOTION_KEY * pMotionKey)
{
	TMotionModeData * pMotionModeData;
	if (!GetMotionModeDataPointer(wMotionModeIndex, &pMotionModeData))
		return FALSE;

	if (pMotionModeData->MotionVectorMap.end() == pMotionModeData->MotionVectorMap.find(wMotionIndex))
	{
		uint16_t wGeneralMode=CRaceMotionData::MODE_GENERAL;

		switch(wMotionModeIndex)
		{
			case CRaceMotionData::MODE_HORSE_ONEHAND_SWORD:
			case CRaceMotionData::MODE_HORSE_TWOHAND_SWORD:
			case CRaceMotionData::MODE_HORSE_DUALHAND_SWORD:
			case CRaceMotionData::MODE_HORSE_FAN:
			case CRaceMotionData::MODE_HORSE_BELL:
			case CRaceMotionData::MODE_HORSE_BOW:
#ifdef ENABLE_WOLFMAN_CHARACTER
			case CRaceMotionData::MODE_HORSE_CLAW:
#endif
				wGeneralMode=CRaceMotionData::MODE_HORSE;
				break;

			default:
				wGeneralMode=CRaceMotionData::MODE_GENERAL;
				break;
		}

		TMotionModeData * pMotionModeGeneralData;
		if (!GetMotionModeDataPointer(wGeneralMode, &pMotionModeGeneralData))
			return FALSE;

		if (pMotionModeGeneralData->MotionVectorMap.end() == pMotionModeGeneralData->MotionVectorMap.find(wMotionIndex))
			return FALSE;

		*pMotionKey = MAKE_MOTION_KEY(wGeneralMode, wMotionIndex);
	}
	else
	{
		*pMotionKey = MAKE_MOTION_KEY(wMotionModeIndex, wMotionIndex);
	}

	return TRUE;
}

BOOL CRaceData::GetMotionModeDataPointer(uint16_t wMotionMode, TMotionModeData ** ppMotionModeData)
{
	auto itor = m_pMotionModeDataMap.find(wMotionMode);
	if (itor == m_pMotionModeDataMap.end())
		return FALSE;

	*ppMotionModeData = itor->second;

	return TRUE;
}

BOOL CRaceData::GetModelDataPointer(uint32_t dwModelIndex, const TModelData ** c_ppModelData)
{
	auto itor = m_ModelDataMap.find(dwModelIndex);
	if (m_ModelDataMap.end() == itor)
		return false;

	*c_ppModelData = &itor->second;

	return true;
}

BOOL CRaceData::GetMotionVectorPointer(uint16_t wMotionMode, uint16_t wMotionIndex, TMotionVector ** ppMotionVector)
{
	TMotionModeData * pMotionModeData;
	if (!GetMotionModeDataPointer(wMotionMode, &pMotionModeData))
		return FALSE;

	auto itor = pMotionModeData->MotionVectorMap.find(wMotionIndex);
	if (pMotionModeData->MotionVectorMap.end() == itor)
		return FALSE;

	*ppMotionVector = &itor->second;

	return TRUE;
}

BOOL CRaceData::GetMotionDataPointer(uint16_t wMotionMode, uint16_t wMotionIndex, uint16_t wMotionSubIndex, CRaceMotionData ** c_ppMotionData)
{
	const TMotionVector * c_pMotionVector;
	if (!GetMotionVectorPointer(wMotionMode, wMotionIndex, &c_pMotionVector))
		return FALSE;

	if (wMotionSubIndex >= c_pMotionVector->size())
		return FALSE;

	const TMotion & c_rMotion = c_pMotionVector->at(wMotionSubIndex);

	if (!c_rMotion.pMotionData)
		return FALSE;

	*c_ppMotionData = c_rMotion.pMotionData;

	return TRUE;
}

BOOL CRaceData::GetMotionDataPointer(uint32_t dwMotionKey, CRaceMotionData ** c_ppMotionData)
{
	return GetMotionDataPointer(GET_MOTION_MODE(dwMotionKey), GET_MOTION_INDEX(dwMotionKey), GET_MOTION_SUB_INDEX(dwMotionKey), c_ppMotionData);
}

BOOL CRaceData::GetMotionVectorPointer(uint16_t wMotionMode, uint16_t wMotionIndex, const TMotionVector ** c_ppMotionVector)
{
	TMotionVector * pMotionVector;
	if (!GetMotionVectorPointer(wMotionMode, wMotionIndex, &pMotionVector))
		return FALSE;

	*c_ppMotionVector = pMotionVector;

	return TRUE;
}

uint32_t CRaceData::GetAttachingDataCount()
{
	return m_AttachingDataVector.size();
}

BOOL CRaceData::GetAttachingDataPointer(uint32_t dwIndex, const NRaceData::TAttachingData ** c_ppAttachingData)
{
	if (dwIndex >= GetAttachingDataCount())
		return FALSE;

	*c_ppAttachingData = &m_AttachingDataVector[dwIndex];

	return TRUE;
}

BOOL CRaceData::GetCollisionDataPointer(uint32_t dwIndex, const NRaceData::TAttachingData ** c_ppAttachingData)
{
	if (dwIndex >= GetAttachingDataCount())
		return FALSE;

	if (NRaceData::ATTACHING_DATA_TYPE_COLLISION_DATA != m_AttachingDataVector[dwIndex].dwType)
		return FALSE;

	*c_ppAttachingData = &m_AttachingDataVector[dwIndex];

	return TRUE;
}

BOOL CRaceData::GetBodyCollisionDataPointer(const NRaceData::TAttachingData ** c_ppAttachingData)
{
	for (uint32_t i = 0; i < m_AttachingDataVector.size(); ++i)
	{
		const NRaceData::TAttachingData * pAttachingData = &m_AttachingDataVector[i];

		if (NRaceData::ATTACHING_DATA_TYPE_COLLISION_DATA == pAttachingData->dwType)
		if (NRaceData::COLLISION_TYPE_BODY == pAttachingData->pCollisionData->iCollisionType)
		{
			*c_ppAttachingData = pAttachingData;
			return true;
		}
	}

	return FALSE;
}

void CRaceData::SetRace(uint32_t dwRaceIndex)
{
	m_dwRaceIndex = dwRaceIndex;
}

void CRaceData::RegisterAttachingBoneName(uint32_t dwPartIndex, const char * c_szBoneName)
{
	m_AttachingBoneNameMap.emplace(dwPartIndex, c_szBoneName);
}

void CRaceData::ChangeAttachingBoneName(uint32_t dwPartIndex, const char * c_szBoneName)
{
	TAttachingBoneNameMap::iterator it = m_AttachingBoneNameMap.find(dwPartIndex);
	if (it == m_AttachingBoneNameMap.end())
		return;

	//m_AttachingBoneNameMap[dwPartIndex] = c_szBoneName; //bad behavior possiblity
	it->second = c_szBoneName;
}

void CRaceData::RegisterMotionMode(uint16_t wMotionModeIndex)
{
	TMotionModeData * pMotionModeData = ms_MotionModeDataPool.Alloc();
	pMotionModeData->wMotionModeIndex = wMotionModeIndex;
	pMotionModeData->MotionVectorMap.clear();
	m_pMotionModeDataMap.emplace(wMotionModeIndex, pMotionModeData);
}

CGraphicThing* CRaceData::NEW_RegisterMotion(CRaceMotionData* pkMotionData, uint16_t wMotionModeIndex, uint16_t wMotionIndex, const char * c_szFileName, uint8_t byPercentage)
{	
	CGraphicThing* pMotionThing = CResourceManager::Instance().GetResourcePointer<CGraphicThing>(c_szFileName);

	TMotionModeData * pMotionModeData;
	if (!GetMotionModeDataPointer(wMotionModeIndex, &pMotionModeData))
	{
		AssertLog("Failed getting motion mode data!");
		return nullptr;
	}

	TMotion	kMotion;
	kMotion.byPercentage = byPercentage;
	kMotion.pMotion		 = pMotionThing;
	kMotion.pMotionData	 = pkMotionData;
	pMotionModeData->MotionVectorMap[wMotionIndex].emplace_back(kMotion);

	return pMotionThing;
}

CGraphicThing* CRaceData::RegisterMotionData(uint16_t wMotionMode, uint16_t wMotionIndex, const char * c_szFileName, uint8_t byPercentage)
{
	CRaceMotionData * pRaceMotionData = CRaceMotionData::New();
	if (!pRaceMotionData->LoadMotionData(c_szFileName))
	{
		TraceError("CRaceData::RegisterMotionData - LoadMotionData(c_szFileName=%s) ERROR", c_szFileName);
		CRaceMotionData::Delete(pRaceMotionData);
		pRaceMotionData = nullptr;
		return nullptr;
	}

	pRaceMotionData->SetName(wMotionIndex);

	/////

	// FIXME : 이미 GR2가 로드 되어 있을 경우에는 어떻게 해야 하는가?
	//         현재는 똑같은 것이 하나 더 추가 되어 버린다. - [levites]

	return NEW_RegisterMotion(pRaceMotionData, wMotionMode, wMotionIndex, pRaceMotionData->GetMotionFileName(), byPercentage);

	//TMotion	Motion;
	//Motion.byPercentage	= byPercentage;
	//Motion.pMotion = (CGraphicThing *)CResourceManager::Instance().GetResourcePointer(pRaceMotionData->GetMotionFileName());
	//Motion.pMotionData = pRaceMotionData;
	//__OLD_RegisterMotion(wMotionMode, wMotionIndex, Motion);
	//return true;
}


void CRaceData::OLD_RegisterMotion(uint16_t wMotionModeIndex, uint16_t wMotionIndex, const char * c_szFileName, uint8_t byPercentage)
{
	CGraphicThing* pThing = CResourceManager::Instance().GetResourcePointer<CGraphicThing>(c_szFileName);

	TMotion	Motion;
	Motion.byPercentage	= byPercentage;
	Motion.pMotion		= pThing;
	Motion.pMotionData	= nullptr;
	__OLD_RegisterMotion(wMotionModeIndex, wMotionIndex, Motion);
}

void CRaceData::__OLD_RegisterMotion(uint16_t wMotionMode, uint16_t wMotionIndex, const TMotion & rMotion)
{
	TMotionModeData * pMotionModeData;
	if (!GetMotionModeDataPointer(wMotionMode, &pMotionModeData))
	{
		AssertLog("Failed getting motion mode data!");
		return;
	}

	auto itor = pMotionModeData->MotionVectorMap.find(wMotionIndex);
	if (pMotionModeData->MotionVectorMap.end() == itor)
	{
		TMotionVector MotionVector;
		MotionVector.emplace_back(rMotion);

		pMotionModeData->MotionVectorMap.emplace(wMotionIndex, MotionVector);
	}
	else
	{
		TMotionVector & rMotionVector = itor->second;
		rMotionVector.emplace_back(rMotion);
	}
}


bool CRaceData::SetMotionRandomWeight(uint16_t wMotionModeIndex, uint16_t wMotionIndex, uint16_t wMotionSubIndex, uint8_t byPercentage)
{
	TMotionModeData * pMotionModeData;

	if (!GetMotionModeDataPointer(wMotionModeIndex, &pMotionModeData))
	{
		//AssertLog("Failed getting motion mode data!");
		return false;
	}

	auto itor = pMotionModeData->MotionVectorMap.find(wMotionIndex);

	if (pMotionModeData->MotionVectorMap.end() != itor)
	{
		TMotionVector & rMotionVector = itor->second;
		if (wMotionSubIndex < rMotionVector.size())
			rMotionVector[wMotionSubIndex].byPercentage = byPercentage;
		else
			return false;
	}
	else
	{
		TraceError("CRaceData::SetMotionRandomWeight(wMotionModeIndex=%d, wMotionIndex=%d, wMotionSubIndex=%d, byPercentage=%d) - Find Motion(wMotionIndex=%d) FAILED",
			wMotionModeIndex, wMotionIndex, wMotionSubIndex, byPercentage, wMotionModeIndex);

		return false;
	}

	return true;
}

void CRaceData::RegisterNormalAttack(uint16_t wMotionModeIndex, uint16_t wMotionIndex)
{
	m_NormalAttackIndexMap.emplace(wMotionModeIndex, wMotionIndex);
}
BOOL CRaceData::GetNormalAttackIndex(uint16_t wMotionModeIndex, uint16_t * pwMotionIndex)
{
	auto itor = m_NormalAttackIndexMap.find(wMotionModeIndex);

	if (m_NormalAttackIndexMap.end() == itor)
		return FALSE;

	*pwMotionIndex = itor->second;

	return TRUE;
}

void CRaceData::ReserveComboAttack(uint16_t wMotionModeIndex, uint16_t wComboType, uint32_t dwComboCount)
{
	TComboData ComboData;
	ComboData.ComboIndexVector.clear();
	ComboData.ComboIndexVector.resize(dwComboCount);
	m_ComboAttackDataMap.emplace(MAKE_COMBO_KEY(wMotionModeIndex, wComboType), ComboData);
}

void CRaceData::RegisterComboAttack(uint16_t wMotionModeIndex, uint16_t wComboType, uint32_t dwComboIndex, uint16_t wMotionIndex)
{
	auto itor = m_ComboAttackDataMap.find(MAKE_COMBO_KEY(wMotionModeIndex, wComboType));
	if (m_ComboAttackDataMap.end() == itor)
		return;

	TComboIndexVector & rComboIndexVector = itor->second.ComboIndexVector;
	if (dwComboIndex >= rComboIndexVector.size())
	{
		AssertLog("CRaceData::RegisterCombo - Strange combo index!");
		return;
	}

	rComboIndexVector[dwComboIndex] = wMotionIndex;
}

BOOL CRaceData::GetComboDataPointer(uint16_t wMotionModeIndex, uint16_t wComboType, TComboData ** ppComboData)
{
	auto itor = m_ComboAttackDataMap.find(MAKE_COMBO_KEY(wMotionModeIndex, wComboType));

	if (m_ComboAttackDataMap.end() == itor)
		return FALSE;

	*ppComboData = &itor->second;
	return TRUE;
}

const char * CRaceData::GetBaseModelFileName() const
{
	return m_strBaseModelFileName.c_str();
}

const char * CRaceData::GetAttributeFileName() const
{
	return m_strAttributeFileName.c_str();
}

const char* CRaceData::GetMotionListFileName() const
{
	return m_strMotionListFileName.c_str();
}

CGraphicThing * CRaceData::GetBaseModelThing()
{
	if (!m_pBaseModelThing)
		m_pBaseModelThing = CResourceManager::Instance().GetResourcePointer<CGraphicThing>(m_strBaseModelFileName);

	return m_pBaseModelThing;
}

CGraphicThing * CRaceData::GetLODModelThing()
{
	if (!m_pLODModelThing)
	{
		std::string strLODFileName = CFileNameHelper::NoExtension(m_strBaseModelFileName) + "_lod_01.gr2";
		if (CResourceManager::Instance().IsFileExist(strLODFileName.c_str()))
			m_pLODModelThing = CResourceManager::Instance().GetResourcePointer<CGraphicThing>(strLODFileName);
	}

	return m_pLODModelThing;
}

CAttributeData * CRaceData::GetAttributeDataPtr()
{
	if (m_strAttributeFileName.empty())
		return nullptr;

	if (!CResourceManager::Instance().IsFileExist(m_strAttributeFileName.c_str()))
		return nullptr;

	return CResourceManager::Instance().GetResourcePointer<CAttributeData>(m_strAttributeFileName);
}

BOOL CRaceData::GetAttachingBoneName(uint32_t dwPartIndex, const char ** c_pszBoneName)
{
	auto itor = m_AttachingBoneNameMap.find(dwPartIndex);
	if (itor == m_AttachingBoneNameMap.end())
		return FALSE;

	const std::string & c_rstrBoneName = itor->second;

	*c_pszBoneName = c_rstrBoneName.c_str();

	return TRUE;
}

BOOL CRaceData::IsTree()
{
	return !m_strTreeFileName.empty();
}

const char * CRaceData::GetTreeFileName()
{
	return m_strTreeFileName.c_str();
}

void CRaceData::Destroy()
{
	m_kMap_dwHairKey_kHair.clear();
	m_kMap_dwShapeKey_kShape.clear();

	m_strBaseModelFileName.clear();
	m_strTreeFileName.clear();
	m_strAttributeFileName.clear();
	m_strMotionListFileName = "motlist.txt";

	m_AttachingBoneNameMap.clear();
	m_ModelDataMap.clear();

	m_NormalAttackIndexMap.clear();
	m_ComboAttackDataMap.clear();

	auto itorMode = m_pMotionModeDataMap.begin();
	for (; itorMode != m_pMotionModeDataMap.end(); ++itorMode)
	{
		TMotionModeData * pMotionModeData = itorMode->second;

		auto itorMotion = pMotionModeData->MotionVectorMap.begin();
		for (; itorMotion != pMotionModeData->MotionVectorMap.end(); ++itorMotion)
		{
			TMotionVector & rMotionVector = itorMotion->second;
			for (auto & i : rMotionVector)
				CRaceMotionData::Delete(i.pMotionData);
		}

		ms_MotionModeDataPool.Free(pMotionModeData);
	}
	m_pMotionModeDataMap.clear();

	__Initialize();
}

void CRaceData::__Initialize()
{
	m_strMotionListFileName = "motlist.txt";

	m_pBaseModelThing = nullptr;
	m_pLODModelThing = nullptr;

	m_dwRaceIndex = 0;

	memset(m_adwSmokeEffectID, 0, sizeof(m_adwSmokeEffectID));
}

CRaceData::CRaceData()
{
	__Initialize();
}
CRaceData::~CRaceData()
{
	Destroy();
}