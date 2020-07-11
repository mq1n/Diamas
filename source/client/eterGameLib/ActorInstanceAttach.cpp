#include "StdAfx.h"
#include "../eterEffectLib/EffectManager.h"
#include "ActorInstance.h"
#include "ItemData.h"
#include "ItemManager.h"
#include "RaceData.h"
#include "WeaponTrace.h"
#include "../eterXClient/locale_inc.h"

BOOL USE_WEAPON_SPECULAR = TRUE;

uint32_t CActorInstance::AttachSmokeEffect(uint32_t eSmoke)
{
	if (!m_pkCurRaceData)
		return 0;

	uint32_t dwSmokeEffectID=m_pkCurRaceData->GetSmokeEffectID(eSmoke);

	return AttachEffectByID(0, m_pkCurRaceData->GetSmokeBone().c_str(), dwSmokeEffectID); 
}

bool CActorInstance::__IsLeftHandWeapon(uint32_t type)
{
	if (WEAPON_DAGGER == type || (WEAPON_FAN == type && __IsMountingHorse()))
		return true;
	else if (WEAPON_BOW == type)
		return true;
#ifdef ENABLE_WOLFMAN_CHARACTER
	else if (WEAPON_CLAW == type)
		return true;
#endif
	else
		return false;
}

bool CActorInstance::__IsRightHandWeapon(uint32_t type)
{
	if (WEAPON_DAGGER == type || (WEAPON_FAN == type && __IsMountingHorse()))
		return true;
	else if (WEAPON_BOW == type)
		return false;
#ifdef ENABLE_WOLFMAN_CHARACTER
	else if (WEAPON_CLAW == type)
		return true;
#endif
	else 
		return true;
}

bool CActorInstance::__IsWeaponTrace(uint32_t weaponType)
{
	switch(weaponType)
	{
	case WEAPON_BELL:
	case WEAPON_FAN:
	case WEAPON_BOW:
		return false;
	default:
		return true;

	}
}

void CActorInstance::AttachWeapon(uint32_t dwItemIndex,uint32_t dwParentPartIndex, uint32_t dwPartIndex)
{
	if (dwPartIndex>=CRaceData::PART_MAX_NUM)
		return;
	
	m_adwPartItemID[dwPartIndex]=dwItemIndex;

	CItemData * pItemData;
	if (!CItemManager::Instance().GetItemDataPointer(dwItemIndex, &pItemData))
	{
		RegisterModelThing(dwPartIndex, nullptr);
		SetModelInstance(dwPartIndex, dwPartIndex, 0);

		RegisterModelThing(CRaceData::PART_WEAPON_LEFT, nullptr);
		SetModelInstance(CRaceData::PART_WEAPON_LEFT, CRaceData::PART_WEAPON_LEFT, 0);

		RefreshActorInstance();
		return;
	}

	__DestroyWeaponTrace();
	//양손무기(자객 이도류) 왼손,오른손 모두에 장착.
	if (__IsRightHandWeapon(pItemData->GetWeaponType()))
		AttachWeapon(dwParentPartIndex, CRaceData::PART_WEAPON, pItemData);
	if (__IsLeftHandWeapon(pItemData->GetWeaponType()))
		AttachWeapon(dwParentPartIndex, CRaceData::PART_WEAPON_LEFT, pItemData);
}

BOOL CActorInstance::GetAttachingBoneName(uint32_t dwPartIndex, const char ** c_pszBoneName)
{
	return m_pkCurRaceData->GetAttachingBoneName(dwPartIndex, c_pszBoneName);
}

#define AUTODETECT_LYCAN_RODNPICK_BONE
void CActorInstance::AttachWeapon(uint32_t dwParentPartIndex, uint32_t dwPartIndex, CItemData * pItemData)
{
//	assert(m_pkCurRaceData);
	if (!pItemData)
		return;

	const char * szBoneName;
#if defined(ENABLE_WOLFMAN_CHARACTER) && defined(AUTODETECT_LYCAN_RODNPICK_BONE)
	if (GetRace() == 8 && (pItemData->GetType() == ITEM_TYPE_ROD || pItemData->GetType() == ITEM_TYPE_PICK))
		szBoneName = "equip_right";
	else if (!GetAttachingBoneName(dwPartIndex, &szBoneName))
		return;
#else
	if (!GetAttachingBoneName(dwPartIndex, &szBoneName))
		return;
#endif

	// NOTE : (이도류처리)단도일 경우 형태가 다른 것으로 얻는다. 없을 경우 디폴트를 리턴
	if (CRaceData::PART_WEAPON_LEFT == dwPartIndex)
		RegisterModelThing(dwPartIndex, pItemData->GetSubModelThing());
	else
		RegisterModelThing(dwPartIndex, pItemData->GetModelThing());

	for (uint32_t i = 0; i < pItemData->GetLODModelThingCount(); ++i)
	{
		CGraphicThing * pThing;

		if (!pItemData->GetLODModelThingPointer(i, &pThing))
			continue;

		RegisterLODThing(dwPartIndex, pThing);
	}

	SetModelInstance(dwPartIndex, dwPartIndex, 0);
	AttachModelInstance(dwParentPartIndex, szBoneName, dwPartIndex);

	// 20041208.myevan.무기스펙큘러(값옷은 SetShape에서 직접 해준다.)
	if (USE_WEAPON_SPECULAR)
	{
		SMaterialData kMaterialData;
		kMaterialData.pImage = nullptr;
		kMaterialData.isSpecularEnable = TRUE;
		kMaterialData.fSpecularPower = pItemData->GetSpecularPowerf();
		kMaterialData.bSphereMapIndex = 1;
		SetMaterialData(dwPartIndex, nullptr, kMaterialData);
	}

	// Weapon Trace
	if (__IsWeaponTrace(pItemData->GetWeaponType()))
	{
		CWeaponTrace * pWeaponTrace = CWeaponTrace::New();		
		pWeaponTrace->SetWeaponInstance(this, dwPartIndex, szBoneName);
		m_WeaponTraceVector.emplace_back(pWeaponTrace);
	}
}

void  CActorInstance::DettachEffect(uint32_t dwEID)
{
	auto i = m_AttachingEffectList.begin();

	while (i != m_AttachingEffectList.end())
	{
		TAttachingEffect & rkAttEft = (*i);

		if (rkAttEft.dwEffectIndex == dwEID)
		{
			i = m_AttachingEffectList.erase(i);
			CEffectManager::Instance().DestroyEffectInstance(dwEID);
		}
		else
			++i;
	}
}

uint32_t CActorInstance::AttachEffectByName(uint32_t dwParentPartIndex, const char * c_pszBoneName, const char * c_pszEffectName)
{
	std::string str;
	uint32_t dwCRC;
	StringPath(c_pszEffectName, str);
	dwCRC = GetCaseCRC32(str.c_str(), str.length());

	return AttachEffectByID(dwParentPartIndex, c_pszBoneName, dwCRC);
}

uint32_t CActorInstance::AttachEffectByID(uint32_t dwParentPartIndex, const char * c_pszBoneName, uint32_t dwEffectID, const D3DXVECTOR3 * c_pv3Position)
{
	TAttachingEffect ae;
	ae.iLifeType = EFFECT_LIFE_INFINITE;
	ae.dwEndTime = 0;
	ae.dwModelIndex = dwParentPartIndex;
	ae.dwEffectIndex = CEffectManager::Instance().GetEmptyIndex();
	ae.isAttaching = TRUE;
	if (c_pv3Position)
		D3DXMatrixTranslation(&ae.matTranslation, c_pv3Position->x, c_pv3Position->y, c_pv3Position->z);
	else
		D3DXMatrixIdentity(&ae.matTranslation);
	CEffectManager& rkEftMgr=CEffectManager::Instance();
	rkEftMgr.CreateEffectInstance(ae.dwEffectIndex, dwEffectID);

	if (c_pszBoneName)
	{
		int32_t iBoneIndex;
		
		if (!FindBoneIndex(dwParentPartIndex,c_pszBoneName, &iBoneIndex))
		{
			ae.iBoneIndex = -1;
			//Tracef("Cannot get Bone Index\n");
			//assert(false && "Cannot get Bone Index");
		}
		else
		{
			ae.iBoneIndex = iBoneIndex;
		}
	}
	else
	{
		ae.iBoneIndex = -1;
	}

	m_AttachingEffectList.emplace_back(ae);

	return ae.dwEffectIndex;
}

void CActorInstance::RefreshActorInstance()
{
	if (!m_pkCurRaceData)
	{
		TraceError("void CActorInstance::RefreshActorInstance() - m_pkCurRaceData=nullptr");
		return;
	}

	// This is Temporary place before making the weapon detection system
	// Setup Collison Detection Data
	m_BodyPointInstanceList.clear();
	//m_AttackingPointInstanceList.clear();
	m_DefendingPointInstanceList.clear();

	// Base
	for (uint32_t i = 0; i < m_pkCurRaceData->GetAttachingDataCount(); ++i)
	{
		const NRaceData::TAttachingData * c_pAttachingData;

		if (!m_pkCurRaceData->GetAttachingDataPointer(i, &c_pAttachingData))
			continue;

		switch (c_pAttachingData->dwType)
		{
			case NRaceData::ATTACHING_DATA_TYPE_COLLISION_DATA:
			{
				const NRaceData::TCollisionData * c_pCollisionData = c_pAttachingData->pCollisionData;

				TCollisionPointInstance PointInstance;
				if (NRaceData::COLLISION_TYPE_ATTACKING == c_pCollisionData->iCollisionType)
					continue;

				if (!CreateCollisionInstancePiece(CRaceData::PART_MAIN, c_pAttachingData, &PointInstance))
					continue;

				switch (c_pCollisionData->iCollisionType)
				{
					case NRaceData::COLLISION_TYPE_ATTACKING:
						//m_AttackingPointInstanceList.push_back(PointInstance);
						break;
					case NRaceData::COLLISION_TYPE_DEFENDING:
						m_DefendingPointInstanceList.emplace_back(PointInstance);
						break;
					case NRaceData::COLLISION_TYPE_BODY:
						m_BodyPointInstanceList.emplace_back(PointInstance);
						break;
				}
			}
			break;

			case NRaceData::ATTACHING_DATA_TYPE_EFFECT:
//				if (!m_bEffectInitialized)
//				{
//					uint32_t dwCRC;
//					StringPath(c_pAttachingData->pEffectData->strFileName);
//					dwCRC = GetCaseCRC32(c_pAttachingData->pEffectData->strFileName.c_str(),c_pAttachingData->pEffectData->strFileName.length());
//
//					TAttachingEffect ae;
//					ae.iLifeType = EFFECT_LIFE_INFINITE;
//					ae.dwEndTime = 0;
//					ae.dwModelIndex = 0;
//					ae.dwEffectIndex = CEffectManager::Instance().GetEmptyIndex();
//					ae.isAttaching = TRUE;
//					CEffectManager::Instance().CreateEffectInstance(ae.dwEffectIndex, dwCRC);
//
//					if (c_pAttachingData->isAttaching)
//					{
//						int32_t iBoneIndex;
//						if (!FindBoneIndex(0,c_pAttachingData->strAttachingBoneName.c_str(), &iBoneIndex))
//						{
//							Tracef("Cannot get Bone Index\n");
//							assert(false/*Cannot get Bone Index*/);
//						}
//
//						ae.iBoneIndex = iBoneIndex;
//					}
//					else
//					{
//						ae.iBoneIndex = -1;
//					}
//
//					m_AttachingEffectList.push_back(ae);
//				}

				if (c_pAttachingData->isAttaching)
					AttachEffectByName(0, c_pAttachingData->strAttachingBoneName.c_str(), c_pAttachingData->pEffectData->strFileName.c_str());
				else
					AttachEffectByName(0, nullptr, c_pAttachingData->pEffectData->strFileName.c_str());
				break;

			case NRaceData::ATTACHING_DATA_TYPE_OBJECT:
				break;

			default:
				assert(false/*NOT_IMPLEMENTED*/);
				break;
		}
	}

	for (uint32_t j = 0; j < CRaceData::PART_MAX_NUM; ++j)
	{
		if (0 == m_adwPartItemID[j])
			continue;

		CItemData * pItemData;
		if (!CItemManager::Instance().GetItemDataPointer(m_adwPartItemID[j], &pItemData))
			return;

		for (uint32_t k = 0; k < pItemData->GetAttachingDataCount(); ++k)
		{
			const NRaceData::TAttachingData * c_pAttachingData;

			if (!pItemData->GetAttachingDataPointer(k, &c_pAttachingData))
				continue;

			switch(c_pAttachingData->dwType)
			{
				case NRaceData::ATTACHING_DATA_TYPE_COLLISION_DATA:
					{
						const NRaceData::TCollisionData * c_pCollisionData = c_pAttachingData->pCollisionData;
						
						// FIXME : 첫번째 인자는 Part의 번호다.
						//         Base는 무조건 0인가? - [levites]
						TCollisionPointInstance PointInstance;
						if (NRaceData::COLLISION_TYPE_ATTACKING == c_pCollisionData->iCollisionType)
							continue;
						if (!CreateCollisionInstancePiece(j, c_pAttachingData, &PointInstance))
							continue;
						
						switch (c_pCollisionData->iCollisionType)
						{
						case NRaceData::COLLISION_TYPE_ATTACKING:
							//m_AttackingPointInstanceList.push_back(PointInstance);
							break;
						case NRaceData::COLLISION_TYPE_DEFENDING:
					m_DefendingPointInstanceList.emplace_back(PointInstance);
							break;
						case NRaceData::COLLISION_TYPE_BODY:
					m_BodyPointInstanceList.emplace_back(PointInstance);
							break;
						}
					}
					break;

				case NRaceData::ATTACHING_DATA_TYPE_EFFECT:
					if (!m_bEffectInitialized)
					{
						uint32_t dwCRC;
						StringPath(c_pAttachingData->pEffectData->strFileName);
						dwCRC = GetCaseCRC32(c_pAttachingData->pEffectData->strFileName.c_str(),
											 c_pAttachingData->pEffectData->strFileName.length());

						TAttachingEffect ae;
						ae.iLifeType = EFFECT_LIFE_INFINITE;
						ae.dwEndTime = 0;
						ae.dwModelIndex = j;
						ae.dwEffectIndex = CEffectManager::Instance().GetEmptyIndex();
						ae.isAttaching = TRUE;
						CEffectManager::Instance().CreateEffectInstance(ae.dwEffectIndex, dwCRC);

						int32_t iBoneIndex;
						if (!FindBoneIndex(j,c_pAttachingData->strAttachingBoneName.c_str(), &iBoneIndex))
						{
							Tracef("Cannot get Bone Index\n");
							assert(false/*Cannot get Bone Index*/);
						}
						Tracef("Creating %p %d %d\n", this, j,k);

						ae.iBoneIndex = iBoneIndex;

					m_AttachingEffectList.emplace_back(ae);
					}
					break;

				case NRaceData::ATTACHING_DATA_TYPE_OBJECT:
					break;

				default:
					assert(false/*NOT_IMPLEMENTED*/);
					break;
			}
		}
	}

	m_bEffectInitialized = true;
}

void CActorInstance::SetWeaponTraceTexture(const char * szTextureName)
{
	std::vector<CWeaponTrace*>::iterator it;
	for (it = m_WeaponTraceVector.begin(); it != m_WeaponTraceVector.end(); ++it)
		(*it)->SetTexture(szTextureName);
}

void CActorInstance::UseTextureWeaponTrace()
{
	for (const auto & weaponTrace : m_WeaponTraceVector)
		weaponTrace->UseTexture();
}

void CActorInstance::UseAlphaWeaponTrace()
{
	for (const auto & weaponTrace : m_WeaponTraceVector)
		weaponTrace->UseAlpha();
}

void CActorInstance::UpdateAttachingInstances()
{
	CEffectManager& rkEftMgr=CEffectManager::Instance();

	std::list<TAttachingEffect>::iterator it;
	uint32_t dwCurrentTime = CTimer::Instance().GetCurrentMillisecond();
	for (it = m_AttachingEffectList.begin(); it!= m_AttachingEffectList.end();)
	{
		if (EFFECT_LIFE_WITH_MOTION == it->iLifeType)
		{
			++it;
			continue;
		}

		if ((EFFECT_LIFE_NORMAL == it->iLifeType && it->dwEndTime < dwCurrentTime) ||
			!rkEftMgr.IsAliveEffect(it->dwEffectIndex))
		{
			rkEftMgr.DestroyEffectInstance(it->dwEffectIndex);
			it = m_AttachingEffectList.erase(it);
		}
		else
		{
			if (it->isAttaching)
			{
				rkEftMgr.SelectEffectInstance(it->dwEffectIndex);

				if (it->iBoneIndex == -1)
				{
					D3DXMATRIX matTransform;
					matTransform = it->matTranslation;
					matTransform *= m_worldMatrix;
					rkEftMgr.SetEffectInstanceGlobalMatrix(matTransform);
				}
				else
				{
					D3DXMATRIX * pBoneMat;
					if (GetBoneMatrix(it->dwModelIndex, it->iBoneIndex, &pBoneMat))
					{
						D3DXMATRIX matTransform;
						matTransform = *pBoneMat;
						matTransform *= it->matTranslation;
						matTransform *= m_worldMatrix;
						rkEftMgr.SetEffectInstanceGlobalMatrix(matTransform);
					}
					else
					{						
						//TraceError("GetBoneMatrix(modelIndex(%d), boneIndex(%d)).NOT_FOUND_BONE", 
						//	it->dwModelIndex, it->iBoneIndex);
					}
				}
			}

			++it;
		}
	}
}

void CActorInstance::SetActiveAllAttachingEffect()
{
	std::list<TAttachingEffect>::iterator it;
	for (it = m_AttachingEffectList.begin(); it != m_AttachingEffectList.end(); ++it)
	{
		CEffectManager::Instance().ActiveEffectInstance(it->dwEffectIndex);
	}
}

void CActorInstance::SetDeactiveAllAttachingEffect()
{
	std::list<TAttachingEffect>::iterator it;
	for (it = m_AttachingEffectList.begin(); it != m_AttachingEffectList.end(); ++it)
	{
		CEffectManager::Instance().DeactiveEffectInstance(it->dwEffectIndex);
	}
}

void CActorInstance::ShowAllAttachingEffect()
{
	std::list<TAttachingEffect>::iterator it;
	for (it = m_AttachingEffectList.begin(); it != m_AttachingEffectList.end(); ++it)
	{
		CEffectManager::Instance().SelectEffectInstance(it->dwEffectIndex);
		CEffectManager::Instance().ShowEffect();
	}
}

void CActorInstance::HideAllAttachingEffect()
{
	std::list<TAttachingEffect>::iterator it;
	for(it = m_AttachingEffectList.begin(); it!= m_AttachingEffectList.end();++it)
	{
		CEffectManager::Instance().SelectEffectInstance(it->dwEffectIndex);
		CEffectManager::Instance().HideEffect();
	}
}

void CActorInstance::__ClearAttachingEffect()
{
	m_bEffectInitialized = false;

	std::list<TAttachingEffect>::iterator it;
	for(it = m_AttachingEffectList.begin(); it!= m_AttachingEffectList.end();++it)
		CEffectManager::Instance().DestroyEffectInstance(it->dwEffectIndex);
	m_AttachingEffectList.clear();
}

#ifdef ENABLE_ACCE_SYSTEM
void CActorInstance::AttachAcce(CItemData * pItemData, float fSpecular)
{
	if (!pItemData)
	{
		RegisterModelThing(CRaceData::PART_ACCE, nullptr);
		SetModelInstance(CRaceData::PART_ACCE, CRaceData::PART_ACCE, 0);
		RefreshActorInstance();
		return;
	}

	RegisterModelThing(CRaceData::PART_ACCE, pItemData->GetModelThing());
	SetModelInstance(CRaceData::PART_ACCE, CRaceData::PART_ACCE, 0);
	AttachModelInstance(CRaceData::PART_MAIN, "Bip01 Spine2", CRaceData::PART_ACCE);

	if (fSpecular > 0.0f)
	{
		SMaterialData kMaterialData;
		kMaterialData.pImage = nullptr;
		kMaterialData.isSpecularEnable = TRUE;
		kMaterialData.fSpecularPower = fSpecular;
		kMaterialData.bSphereMapIndex = 1;
		SetMaterialData(CRaceData::PART_ACCE, nullptr, kMaterialData);
	}
}
#endif