#include "StdAfx.h"
#include "ActorInstance.h"
#include "RaceManager.h"
#include "ItemManager.h"
#include "RaceData.h"

#include "../eterLib/ResourceManager.h"
#include "../eterGrnLib/Util.h"
#include "../eterXClient/PythonSystem.h"

uint32_t CActorInstance::GetVirtualID()
{
	return m_dwSelfVID;
}

void CActorInstance::SetVirtualID(uint32_t dwVID)
{
	m_dwSelfVID=dwVID;
}

void CActorInstance::UpdateAttribute()
{
	if (!m_pAttributeInstance)
		return;

	if (!m_bNeedUpdateCollision)
		return;

	m_bNeedUpdateCollision = FALSE;

	const CStaticCollisionDataVector & c_rkVec_ColliData = m_pAttributeInstance->GetObjectPointer()->GetCollisionDataVector();
	UpdateCollisionData(&c_rkVec_ColliData);

	m_pAttributeInstance->RefreshObject(GetTransform());
	UpdateHeightInstance(m_pAttributeInstance);

	//BOOL isHeightData = m_pAttributeInstance->IsEmpty();
}

void CActorInstance::__CreateAttributeInstance(CAttributeData * pData)
{
	m_pAttributeInstance = CAttributeInstance::New();
	m_pAttributeInstance->Clear();
	m_pAttributeInstance->SetObjectPointer(pData);
	if (pData->IsEmpty())
	{
		m_pAttributeInstance->Clear();
		CAttributeInstance::Delete(m_pAttributeInstance);
	}
}

uint32_t CActorInstance::GetRace()
{
	return m_eRace;
}

bool CActorInstance::SetRace(uint32_t eRace)
{
	CRaceData * pRaceData;
	if (!CRaceManager::Instance().GetRaceDataPointer(eRace, &pRaceData))
	{
		m_eRace=0;
		m_pkCurRaceData=nullptr;
		return false;
	}

	m_eRace=eRace;
	m_pkCurRaceData = pRaceData;

	CAttributeData * pAttributeData = pRaceData->GetAttributeDataPtr();
	if (pAttributeData)
		__CreateAttributeInstance(pAttributeData);

	memset(m_adwPartItemID, 0, sizeof(m_adwPartItemID));

	// Setup Graphic ResourceData
	__ClearAttachingEffect();

	CGraphicThingInstance::Clear();

	//NOTE : PC만 Part별로 다 생성하게 해준다.
	if (IsPC() || (IsNPC() && GetRace() < 10))
	{
		CGraphicThingInstance::ReserveModelThing(CRaceData::PART_MAX_NUM);
		CGraphicThingInstance::ReserveModelInstance(CRaceData::PART_MAX_NUM);
	}
	else
	{
		CGraphicThingInstance::ReserveModelThing(1);
		CGraphicThingInstance::ReserveModelInstance(1);
	}


	CRaceData::TMotionModeDataIterator itor;

	if (pRaceData->CreateMotionModeIterator(itor))
	{
		do
		{
			uint16_t wMotionMode = itor->first;
			CRaceData::TMotionModeData * pMotionModeData = itor->second;

			auto itorMotion = pMotionModeData->MotionVectorMap.begin();
			for (; itorMotion != pMotionModeData->MotionVectorMap.end(); ++itorMotion)
			{
				uint16_t wMotionIndex = itorMotion->first;
				const CRaceData::TMotionVector & c_rMotionVector = itorMotion->second;
				CRaceData::TMotionVector::const_iterator it;
				uint32_t i;
				for (i = 0, it = c_rMotionVector.begin(); it != c_rMotionVector.end(); ++i, ++it)
				{
					uint32_t dwMotionKey = MAKE_RANDOM_MOTION_KEY(wMotionMode, wMotionIndex, i);
					CGraphicThingInstance::RegisterMotionThing(dwMotionKey, it->pMotion);
				}
			}
		} while (pRaceData->NextMotionModeIterator(itor));
	}

	return true;
}

void CActorInstance::SetHair(uint32_t eHair, float fSpecular)
{
	m_eHair = eHair;

	CRaceData * pRaceData;

	if (!CRaceManager::Instance().GetRaceDataPointer(m_eRace, &pRaceData))
		return;

	CRaceData::SHair* pkHair=pRaceData->FindHair(eHair);
	if (pkHair)
	{
		if (!pkHair->m_stModelFileName.empty())
		{
			CGraphicThing* pkHairThing = CResourceManager::Instance().GetResourcePointer<CGraphicThing>(pkHair->m_stModelFileName);
			RegisterModelThing(CRaceData::PART_HAIR, pkHairThing);
			SetModelInstance(CRaceData::PART_HAIR, CRaceData::PART_HAIR, 0, CRaceData::PART_MAIN);
		}

		const std::vector<CRaceData::SSkin>& c_rkVct_kSkin = pkHair->m_kVct_kSkin;
		std::vector<CRaceData::SSkin>::const_iterator i;
		for (i = c_rkVct_kSkin.begin(); i != c_rkVct_kSkin.end(); ++i)
		{
			const CRaceData::SSkin& c_rkSkinItem = *i;

			CResource* pkRes = CResourceManager::Instance().GetResourcePointer<CResource>(c_rkSkinItem.m_stDstFileName);

			if (pkRes)
			{
				if (fSpecular > 0.0f)
				{
					SMaterialData kMaterialData;
					kMaterialData.pImage = static_cast<CGraphicImage*>(pkRes);
					kMaterialData.isSpecularEnable = TRUE;
					kMaterialData.fSpecularPower = fSpecular;
					kMaterialData.bSphereMapIndex = 0;
	 				SetMaterialData(CRaceData::PART_HAIR, c_rkSkinItem.m_stSrcFileName.c_str(), kMaterialData);
				}
				else
				{
					SetMaterialImagePointer(CRaceData::PART_HAIR, c_rkSkinItem.m_stSrcFileName.c_str(), static_cast<CGraphicImage*>(pkRes));
	 				SetMaterialImagePointer(c_rkSkinItem.m_ePart, c_rkSkinItem.m_stSrcFileName.c_str(), static_cast<CGraphicImage*>(pkRes));
				}
			}
		}
	}
}

void CActorInstance::SetShape(uint32_t eShape, float fSpecular)
{
//	if ((IsNPC() || IsPoly() || IsEnemy()) && CPythonSystem::Instance().IsShowSpecular())
//	{
//		fSpecular = 1.0;
//	}

	m_eShape = eShape;

	CRaceData * pRaceData;
	if (!CRaceManager::Instance().GetRaceDataPointer(m_eRace, &pRaceData))
		return;

	CRaceData::SShape* pkShape=pRaceData->FindShape(eShape);
	if (pkShape)
	{
		CResourceManager& rkResMgr=CResourceManager::Instance();
		
		if (pkShape->m_stModelFileName.empty())
		{
			CGraphicThing* pModelThing = pRaceData->GetBaseModelThing();
			RegisterModelThing(0, pModelThing);
		}
		else
		{
			CGraphicThing* pModelThing = rkResMgr.GetResourcePointer<CGraphicThing>(pkShape->m_stModelFileName);
			RegisterModelThing(0, pModelThing);
		}		

		{
			std::string stLODModelFileName;

			char szLODModelFileNameEnd[256];
			for (uint32_t uLODIndex=1; uLODIndex<=3; ++uLODIndex)
			{
				sprintf_s(szLODModelFileNameEnd, "_lod_%.2d.gr2", uLODIndex);
				stLODModelFileName = CFileNameHelper::NoExtension(pkShape->m_stModelFileName) + szLODModelFileNameEnd;
				if (!rkResMgr.IsFileExist(stLODModelFileName.c_str()))
					break;
				
				CGraphicThing* pLODModelThing = rkResMgr.GetResourcePointer<CGraphicThing>(stLODModelFileName);
				if (!pLODModelThing)
					break;

				RegisterLODThing(0, pLODModelThing);
			}
		}

		SetModelInstance(0, 0, 0);

		const std::vector<CRaceData::SSkin>& c_rkVct_kSkin = pkShape->m_kVct_kSkin;
		std::vector<CRaceData::SSkin>::const_iterator i;
		for (i = c_rkVct_kSkin.begin(); i != c_rkVct_kSkin.end(); ++i)
		{
			const CRaceData::SSkin& c_rkSkinItem = *i;

			CResource* pkRes = CResourceManager::Instance().GetResourcePointer<CResource>(c_rkSkinItem.m_stDstFileName);

			if (pkRes)
			{
				if (fSpecular > 0.0f)
				{
					SMaterialData kMaterialData;
					kMaterialData.pImage = static_cast<CGraphicImage*>(pkRes);
					kMaterialData.isSpecularEnable = TRUE;
					kMaterialData.fSpecularPower = fSpecular;
					kMaterialData.bSphereMapIndex = 0;
	 				SetMaterialData(c_rkSkinItem.m_ePart, c_rkSkinItem.m_stSrcFileName.c_str(), kMaterialData);
				}
				else
				{
	 				SetMaterialImagePointer(c_rkSkinItem.m_ePart, c_rkSkinItem.m_stSrcFileName.c_str(), static_cast<CGraphicImage*>(pkRes));
				}
			}
		}
	}
	else
	{
		if (pRaceData->IsTree())
		{
			__CreateTree(pRaceData->GetTreeFileName());
		}
		else
		{
			CGraphicThing* pModelThing = pRaceData->GetBaseModelThing();
			RegisterModelThing(0, pModelThing);

			CGraphicThing* pLODModelThing = pRaceData->GetLODModelThing();

			bool canLOD = true;
			if (pModelThing && pLODModelThing) {
				if (pModelThing->GetTextureCount() == pLODModelThing->GetTextureCount()) {
					for (int32_t i = 0; i < pModelThing->GetTextureCount(); i++) {
						if (strcmp(pModelThing->GetTexturePath(i), pLODModelThing->GetTexturePath(i)) != 0)
							canLOD = false;
					}
				} else {
					canLOD = false;
				}
			}

			if(canLOD)
				RegisterLODThing(0, pLODModelThing);

			SetModelInstance(0, 0, 0);
		}
	}

	// Attaching Objects
	for (uint32_t i = 0; i < pRaceData->GetAttachingDataCount(); ++i)
	{
		const NRaceData::TAttachingData * c_pAttachingData;
		if (!pRaceData->GetAttachingDataPointer(i, &c_pAttachingData))
			continue;

		switch (c_pAttachingData->dwType)
		{
			case NRaceData::ATTACHING_DATA_TYPE_EFFECT:
				if (c_pAttachingData->isAttaching)
					AttachEffectByName(0, c_pAttachingData->strAttachingBoneName.c_str(), c_pAttachingData->pEffectData->strFileName.c_str());
				else
					AttachEffectByName(0, nullptr, c_pAttachingData->pEffectData->strFileName.c_str());
				break;
		}
	}
}

void CActorInstance::ChangeMaterial(const char * c_szFileName)
{
	CRaceData * pRaceData;
	if (!CRaceManager::Instance().GetRaceDataPointer(m_eRace, &pRaceData))
		return;

	CRaceData::SShape* pkShape=pRaceData->FindShape(m_eShape);
	if (!pkShape)
		return;

	const std::vector<CRaceData::SSkin>& c_rkVct_kSkin = pkShape->m_kVct_kSkin;
	if (c_rkVct_kSkin.empty())
		return;

	auto i = c_rkVct_kSkin.begin();
	const CRaceData::SSkin& c_rkSkinItem = *i;

	std::string dstFileName = c_szFileName;

	CGraphicImage* pkRes = CResourceManager::Instance().GetResourcePointer<CGraphicImage>(dstFileName);
	if (!pkRes)
		return;

	SetMaterialImagePointer(c_rkSkinItem.m_ePart, c_rkSkinItem.m_stSrcFileName.c_str(), pkRes);
}
/*
void CActorInstance::SetPart(uint32_t dwPartIndex, uint32_t dwItemID)
{
	if (dwPartIndex>=CRaceData::PART_MAX_NUM)
		return;

	if (!m_pkCurRaceData)
	{
		assert(m_pkCurRaceData);
		return;
	}

	CItemData * pItemData;
	if (!CItemManager::Instance().GetItemDataPointer(dwItemID, &pItemData))
		return;

	RegisterModelThing(dwPartIndex, pItemData->GetModelThing());
	for (uint32_t i = 0; i < pItemData->GetLODModelThingCount(); ++i)
	{
		CGraphicThing * pThing;
		if (!pItemData->GetLODModelThingPointer(i, &pThing))
			continue;

		RegisterLODThing(dwPartIndex, pThing);
	}
	SetModelInstance(dwPartIndex, dwPartIndex, 0);

	m_adwPartItemID[dwPartIndex] = dwItemID;
}
*/

uint32_t CActorInstance::GetPartItemID(uint32_t dwPartIndex)
{
	if (dwPartIndex>=CRaceData::PART_MAX_NUM)
	{
		TraceError("CActorInstance::GetPartIndex(dwPartIndex=%d/CRaceData::PART_MAX_NUM=%d)", dwPartIndex, CRaceData::PART_MAX_NUM);
		return 0;
	}
	
	return m_adwPartItemID[dwPartIndex];
}

void CActorInstance::SetSpecularInfo(bool bEnable, int32_t iPart, float fAlpha)
{
	CRaceData * pkRaceData;
	if (!CRaceManager::Instance().GetRaceDataPointer(m_eRace, &pkRaceData))
		return;

	CRaceData::SShape * pkShape = pkRaceData->FindShape(m_eShape);
	if (pkShape->m_kVct_kSkin.empty())
		return;

	std::string filename = pkShape->m_kVct_kSkin[0].m_stSrcFileName;
	CFileNameHelper::ChangeDosPath(filename);

	CGraphicThingInstance::SetSpecularInfo(iPart, filename.c_str(), bEnable, fAlpha);
}

void CActorInstance::SetSpecularInfoForce(bool bEnable, int32_t iPart, float fAlpha)
{
	CGraphicThingInstance::SetSpecularInfo(iPart, nullptr, bEnable, fAlpha);
}
