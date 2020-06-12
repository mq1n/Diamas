#include "StdAfx.h"
#include "../eterLib/ResourceManager.h"

#include "ItemData.h"

CDynamicPool<CItemData>		CItemData::ms_kPool;

CItemData* CItemData::New()
{
	return ms_kPool.Alloc();
}

void CItemData::Delete(CItemData* pkItemData)
{
	pkItemData->Clear();
	ms_kPool.Free(pkItemData);
}

void CItemData::DestroySystem()
{
	ms_kPool.Destroy();	
}

CGraphicThing * CItemData::GetModelThing()
{
	return m_pModelThing;
}

CGraphicThing * CItemData::GetSubModelThing()
{
	if (m_pSubModelThing)
		return m_pSubModelThing;
	else
		return m_pModelThing;
}

CGraphicThing * CItemData::GetDropModelThing()
{
	return m_pDropModelThing;
}

CGraphicSubImage * CItemData::GetIconImage(const std::string& stIconFileName)
{
	bool reload = false; 
	if (!stIconFileName.empty())
	{
		if (m_pIconImage)
		{
			const auto& stRefResourceName = m_pIconImage->GetFileNameString();
			reload = stRefResourceName != stIconFileName;
		}
		__SetIconImage(stIconFileName.c_str(), reload);
	}
	else
	{
		if (!m_strIconFileName.empty())
		{
			if (m_pIconImage)
			{
				const auto& stRefResourceName = m_pIconImage->GetFileNameString();
				reload = stRefResourceName != m_strIconFileName;
			}
			__SetIconImage(m_strIconFileName.c_str(), reload);
		}
	}
	return m_pIconImage;
}


uint32_t CItemData::GetLODModelThingCount()
{
	return m_pLODModelThingVector.size();
}

BOOL CItemData::GetLODModelThingPointer(uint32_t dwIndex, CGraphicThing ** ppModelThing)
{
	if (dwIndex >= m_pLODModelThingVector.size())
		return FALSE;

	*ppModelThing = m_pLODModelThingVector[dwIndex];

	return TRUE;
}

uint32_t CItemData::GetAttachingDataCount()
{
	return m_AttachingDataVector.size();
}

BOOL CItemData::GetCollisionDataPointer(uint32_t dwIndex, const NRaceData::TAttachingData ** c_ppAttachingData)
{
	if (dwIndex >= GetAttachingDataCount())
		return FALSE;

	if (NRaceData::ATTACHING_DATA_TYPE_COLLISION_DATA != m_AttachingDataVector[dwIndex].dwType)
		return FALSE;
	
	*c_ppAttachingData = &m_AttachingDataVector[dwIndex];
	return TRUE;
}

BOOL CItemData::GetAttachingDataPointer(uint32_t dwIndex, const NRaceData::TAttachingData ** c_ppAttachingData)
{
	if (dwIndex >= GetAttachingDataCount())
		return FALSE;
	
	*c_ppAttachingData = &m_AttachingDataVector[dwIndex];
	return TRUE;
}

void CItemData::SetSummary(const std::string& c_rstSumm)
{
	m_strSummary=c_rstSumm;
}

void CItemData::SetDescription(const std::string& c_rstDesc)
{
	m_strDescription=c_rstDesc;
}

void CItemData::SetDefaultItemData(const char * c_szIconFileName, const char * c_szModelFileName)
{
	if(c_szModelFileName)
	{
		m_strModelFileName = c_szModelFileName;
		m_strDropModelFileName = c_szModelFileName;
	}
	else
	{
		m_strModelFileName.clear();
		m_strDropModelFileName = "d:/ymir work/item/etc/item_bag.gr2";
	}
	m_strIconFileName = c_szIconFileName;

	m_strSubModelFileName.clear();
	m_strDescription.clear();
	m_strSummary.clear();
	memset(m_ItemTable.alSockets, 0, sizeof(m_ItemTable.alSockets));

	__LoadFiles();
}

void CItemData::__LoadFiles()
{
	// Model File Name
	if (!m_strModelFileName.empty())
		m_pModelThing = (CGraphicThing *)CResourceManager::Instance().GetResourcePointer(m_strModelFileName.c_str());

	if (!m_strSubModelFileName.empty())
		m_pSubModelThing = (CGraphicThing *)CResourceManager::Instance().GetResourcePointer(m_strSubModelFileName.c_str());

	if (!m_strDropModelFileName.empty())
		m_pDropModelThing = (CGraphicThing *)CResourceManager::Instance().GetResourcePointer(m_strDropModelFileName.c_str());


	if (!m_strLODModelFileNameVector.empty())
	{
		m_pLODModelThingVector.clear();
		m_pLODModelThingVector.resize(m_strLODModelFileNameVector.size());

		for (uint32_t i = 0; i < m_strLODModelFileNameVector.size(); ++i)
		{
			const std::string & c_rstrLODModelFileName = m_strLODModelFileNameVector[i];
			m_pLODModelThingVector[i] = (CGraphicThing *)CResourceManager::Instance().GetResourcePointer(c_rstrLODModelFileName.c_str());
		}
	}
}

#define ENABLE_LOAD_ALTER_ITEMICON
void CItemData::__SetIconImage(const char * c_szFileName, bool renew)
{
	if (!CResourceManager::Instance().IsFileExist(c_szFileName))
	{
		TraceError("CItemData::__SetIconImage - %s does not exist",c_szFileName);
		m_pIconImage = nullptr;
#ifdef ENABLE_LOAD_ALTER_ITEMICON
		static const char* c_szAlterIconImage = "icon/item/27995.tga";
		if (CResourceManager::Instance().IsFileExist(c_szAlterIconImage))
			m_pIconImage = (CGraphicSubImage *)CResourceManager::Instance().GetResourcePointer(c_szAlterIconImage);
#endif
	}
	else if (m_pIconImage == nullptr || renew)
		m_pIconImage = (CGraphicSubImage *)CResourceManager::Instance().GetResourcePointer(c_szFileName);
}

void CItemData::SetItemTableData(TItemTable * pItemTable)
{
	memcpy(&m_ItemTable, pItemTable, sizeof(TItemTable));
}

#ifdef ENABLE_ACCE_SYSTEM
void CItemData::SetItemScale(const std::string strJob, const std::string strSex, const std::string strScaleX, const std::string strScaleY, const std::string strScaleZ, const std::string strPositionX, const std::string strPositionY, const std::string strPositionZ)
{
	uint32_t dwPos=0;
	if (strJob == "JOB_WARRIOR")
		dwPos = NRaceData::JOB_WARRIOR;
	else if (strJob == "JOB_ASSASSIN")
		dwPos = NRaceData::JOB_ASSASSIN;
	else if (strJob == "JOB_SURA")
		dwPos = NRaceData::JOB_SURA;
	else if (strJob == "JOB_SHAMAN")
		dwPos = NRaceData::JOB_SHAMAN;
#ifdef ENABLE_WOLFMAN_CHARACTER
	else
		dwPos = NRaceData::JOB_WOLFMAN;
#endif

	dwPos += 1;
	if (strSex == "F")
		dwPos += 5;

	m_ScaleTable.tInfo[dwPos].fScaleX = float(atof(strScaleX.c_str()) / 100.0f);
	m_ScaleTable.tInfo[dwPos].fScaleY = float(atof(strScaleY.c_str()) / 100.0f);
	m_ScaleTable.tInfo[dwPos].fScaleZ = float(atof(strScaleZ.c_str()) / 100.0f);
	m_ScaleTable.tInfo[dwPos].fPositionX = float(atof(strPositionX.c_str()) * 100.0f);
	m_ScaleTable.tInfo[dwPos].fPositionY = float(atof(strPositionY.c_str()) * 100.0f);
	m_ScaleTable.tInfo[dwPos].fPositionZ = float(atof(strPositionZ.c_str()) * 100.0f);
}

bool CItemData::GetItemScale(uint32_t dwPos, float & fScaleX, float & fScaleY, float & fScaleZ, float & fPositionX, float & fPositionY, float & fPositionZ)
{
	fScaleX = m_ScaleTable.tInfo[dwPos].fScaleX;
	fScaleY = m_ScaleTable.tInfo[dwPos].fScaleY;
	fScaleZ = m_ScaleTable.tInfo[dwPos].fScaleZ;
	fPositionX = m_ScaleTable.tInfo[dwPos].fPositionX;
	fPositionY = m_ScaleTable.tInfo[dwPos].fPositionY;
	fPositionZ = m_ScaleTable.tInfo[dwPos].fPositionZ;
	return true;
}
#endif

const CItemData::TItemTable* CItemData::GetTable() const
{
	return &m_ItemTable;
}

uint32_t CItemData::GetIndex() const
{
	return m_ItemTable.dwVnum;
}

const char * CItemData::GetName() const
{
	return m_ItemTable.szLocaleName;
}

const char * CItemData::GetDescription() const
{
	return m_strDescription.c_str();
}

const char * CItemData::GetSummary() const
{
	return m_strSummary.c_str();
}


uint8_t CItemData::GetType() const
{
	return m_ItemTable.bType;
}

uint8_t CItemData::GetSubType() const
{
	return m_ItemTable.bSubType;
}

#define DEF_STR(x) #x

const char* CItemData::GetUseTypeString() const
{
	if (GetType() != CItemData::ITEM_TYPE_USE)
		return "NOT_USE_TYPE";

	switch (GetSubType())
	{
		case USE_TUNING:
			return DEF_STR(USE_TUNING);				
		case USE_DETACHMENT:
			return DEF_STR(USE_DETACHMENT);							
		case USE_CLEAN_SOCKET:
			return DEF_STR(USE_CLEAN_SOCKET);
		case USE_CHANGE_ATTRIBUTE:
			return DEF_STR(USE_CHANGE_ATTRIBUTE);
		case USE_ADD_ATTRIBUTE:
			return DEF_STR(USE_ADD_ATTRIBUTE);		
		case USE_ADD_ATTRIBUTE2:
			return DEF_STR(USE_ADD_ATTRIBUTE2);		
		case USE_ADD_ACCESSORY_SOCKET:
			return DEF_STR(USE_ADD_ACCESSORY_SOCKET);		
		case USE_PUT_INTO_ACCESSORY_SOCKET:
			return DEF_STR(USE_PUT_INTO_ACCESSORY_SOCKET);
		case USE_PUT_INTO_BELT_SOCKET:
			return DEF_STR(USE_PUT_INTO_BELT_SOCKET);
		case USE_PUT_INTO_RING_SOCKET:
			return DEF_STR(USE_PUT_INTO_RING_SOCKET);
#ifdef ENABLE_USE_COSTUME_ATTR
		case USE_CHANGE_COSTUME_ATTR:
			return DEF_STR(USE_CHANGE_COSTUME_ATTR);
		case USE_RESET_COSTUME_ATTR:
			return DEF_STR(USE_RESET_COSTUME_ATTR);
#endif
	}
	return "USE_UNKNOWN_TYPE";
}


uint32_t CItemData::GetWeaponType() const
{
#ifdef ENABLE_WEAPON_COSTUME_SYSTEM
	if (GetType()==CItemData::ITEM_TYPE_COSTUME && GetSubType()==CItemData::COSTUME_WEAPON)
		return GetValue(3);
#endif
	return m_ItemTable.bSubType;
}

uint8_t CItemData::GetSize() const
{
	return m_ItemTable.bSize;
}

BOOL CItemData::IsAntiFlag(uint32_t dwFlag) const
{
	return (dwFlag & m_ItemTable.dwAntiFlags) != 0;
}

BOOL CItemData::IsFlag(uint32_t dwFlag) const
{
	return (dwFlag & m_ItemTable.dwFlags) != 0;
}

BOOL CItemData::IsWearableFlag(uint32_t dwFlag) const
{
	return (dwFlag & m_ItemTable.dwWearFlags) != 0;
}

BOOL CItemData::HasNextGrade() const
{
	return 0 != m_ItemTable.dwRefinedVnum;
}

uint32_t CItemData::GetFlags() const
{
	return m_ItemTable.dwFlags;
}

uint32_t CItemData::GetAntiFlags() const
{
	return m_ItemTable.dwAntiFlags;
}

uint32_t CItemData::GetWearFlags() const
{
	return m_ItemTable.dwWearFlags;
}

uint32_t CItemData::GetIBuyItemPrice() const
{
	return m_ItemTable.dwIBuyItemPrice;
}

uint32_t CItemData::GetISellItemPrice() const
{
	return m_ItemTable.dwISellItemPrice;
}

int32_t CItemData::GetLevelLimit() const
{
	for(int32_t lv = 0; lv < ITEM_LIMIT_MAX_NUM; lv++)
	{
		if(m_ItemTable.aLimits[lv].bType == LIMIT_LEVEL)
			return m_ItemTable.aLimits[lv].lValue;
	}
	return 0;
}


BOOL CItemData::GetLimit(uint8_t byIndex, TItemLimit * pItemLimit) const
{
	if (byIndex >= ITEM_LIMIT_MAX_NUM)
	{
		assert(byIndex < ITEM_LIMIT_MAX_NUM);
		return FALSE;
	}

	*pItemLimit = m_ItemTable.aLimits[byIndex];

	return TRUE;
}

BOOL CItemData::GetApply(uint8_t byIndex, TItemApply * pItemApply) const
{
	if (byIndex >= ITEM_APPLY_MAX_NUM)
	{
		assert(byIndex < ITEM_APPLY_MAX_NUM);
		return FALSE;
	}

	*pItemApply = m_ItemTable.aApplies[byIndex];
	return TRUE;
}

int32_t CItemData::GetValue(uint8_t byIndex) const
{
	if (byIndex >= ITEM_VALUES_MAX_NUM)
	{
		assert(byIndex < ITEM_VALUES_MAX_NUM);
		return 0;
	}

	return m_ItemTable.alValues[byIndex];
}

int32_t CItemData::SetSocket(uint8_t byIndex,uint32_t value)
{
	if (byIndex >= ITEM_SOCKET_MAX_NUM)
	{
		assert(byIndex < ITEM_SOCKET_MAX_NUM);
		return -1;
	}

	return m_ItemTable.alSockets[byIndex] = value;
}

int32_t CItemData::GetSocket(uint8_t byIndex) const
{
	if (byIndex >= ITEM_SOCKET_MAX_NUM)
	{
		assert(byIndex < ITEM_SOCKET_MAX_NUM);
		return -1;
	}

	return m_ItemTable.alSockets[byIndex];
}

//서버와 동일 서버 함수 변경시 같이 변경!!(이후에 합친다)
//SocketCount = 1 이면 초급무기
//SocketCount = 2 이면 중급무기
//SocketCount = 3 이면 고급무기
int32_t CItemData::GetSocketCount() const		
{
	return m_ItemTable.bGainSocketPct;
}

uint32_t CItemData::GetIconNumber() const
{
	return m_ItemTable.dwVnum;
//!@#
//	return m_ItemTable.dwIconNumber;
}

uint32_t CItemData::GetSpecularPoweru() const
{
	return m_ItemTable.bSpecular;
}

float CItemData::GetSpecularPowerf() const
{
	uint32_t uSpecularPower=GetSpecularPoweru();

	return float(uSpecularPower) / 100.0f;	
}

//refine 값은 아이템번호 끝자리와 일치한다-_-(테이블이용으로 바꿀 예정)
uint32_t CItemData::GetRefine() const
{
	return GetIndex()%10;
}

BOOL CItemData::IsEquipment() const
{
	switch (GetType())
	{
		case ITEM_TYPE_WEAPON:
		case ITEM_TYPE_ARMOR:
			return TRUE;
	}

	return FALSE;
}

void CItemData::Clear()
{
	m_strSummary.clear();
	m_strModelFileName.clear();
	m_strSubModelFileName.clear();
	m_strDropModelFileName.clear();
	m_strIconFileName.clear();
	m_strLODModelFileNameVector.clear();

	m_pModelThing = nullptr;
	m_pSubModelThing = nullptr;
	m_pDropModelThing = nullptr;
	m_pIconImage = nullptr;
	m_pLODModelThingVector.clear();

	memset(&m_ItemTable, 0, sizeof(m_ItemTable));
#ifdef ENABLE_ACCE_SYSTEM
	memset(&m_ScaleTable, 0, sizeof(m_ScaleTable));
#endif
}

CItemData::CItemData()
{
	Clear();
}

CItemData::~CItemData()
{
}
