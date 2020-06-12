#include "stdafx.h"
#include "pythoncharactermanager.h"
#include "PythonBackground.h"
#include "PythonNonPlayer.h"
#include "AbstractPlayer.h"
#include "packet.h"
#include "NetworkActorManager.h"
#include "PythonNetworkStream.h"

#include "../eterLib/Camera.h"

///////////////////////////////////////////////////////////////////////////////////////////////////
// Frame Process

int32_t CHAR_STAGE_VIEW_BOUND = 200*100;

struct FCharacterManagerCharacterInstanceUpdate
{
	inline void operator () (const std::pair<uint32_t,CInstanceBase *>& cr_Pair)
	{
		cr_Pair.second->Update();
	}
};

void CPythonCharacterManager::AdjustCollisionWithOtherObjects(CActorInstance* pInst )
{
	if( !pInst->IsPC() )
		return;

	CPythonCharacterManager& rkChrMgr=CPythonCharacterManager::Instance();
	for(CPythonCharacterManager::CharacterIterator i = rkChrMgr.CharacterInstanceBegin(); i!=rkChrMgr.CharacterInstanceEnd();++i)
	{
		CInstanceBase*  pkInstEach=*i;
		CActorInstance* rkActorEach=pkInstEach->GetGraphicThingInstancePtr();

		if (rkActorEach==pInst)
			continue;

		if( rkActorEach->IsPC() || rkActorEach->IsNPC() || rkActorEach->IsEnemy() )
			continue;

		if(pInst->TestPhysicsBlendingCollision(*rkActorEach) )
		{
			// NOTE : 일단 기존위치로 원복
			// TODO : 향후 조금더 잘 처리한다면 physic movement거리를 steping해서 iteration처리해야 함.
			TPixelPosition curPos;
			pInst->GetPixelPosition(&curPos);
			pInst->SetBlendingPosition(curPos);
			//Tracef("!!!!!! Collision Adjusted\n"); 
			break;
		}
	}
}


void CPythonCharacterManager::EnableSortRendering(bool isEnable)
{
}

void CPythonCharacterManager::InsertPVPKey(uint32_t dwVIDSrc, uint32_t dwVIDDst)
{
	CInstanceBase::InsertPVPKey(dwVIDSrc, dwVIDDst);

	CInstanceBase* pkInstSrc=GetInstancePtr(dwVIDSrc);
	if (pkInstSrc)
		pkInstSrc->RefreshTextTail();

	CInstanceBase* pkInstDst=GetInstancePtr(dwVIDDst);
	if (pkInstDst)
		pkInstDst->RefreshTextTail();
}

void CPythonCharacterManager::RemovePVPKey(uint32_t dwVIDSrc, uint32_t dwVIDDst)
{
	CInstanceBase::RemovePVPKey(dwVIDSrc, dwVIDDst);

	CInstanceBase* pkInstSrc=GetInstancePtr(dwVIDSrc);
	if (pkInstSrc)
		pkInstSrc->RefreshTextTail();

	CInstanceBase* pkInstDst=GetInstancePtr(dwVIDDst);
	if (pkInstDst)
		pkInstDst->RefreshTextTail();
}

void CPythonCharacterManager::ChangeGVG(uint32_t dwSrcGuildID, uint32_t dwDstGuildID)
{
	TCharacterInstanceMap::iterator itor;
	for (itor = m_kAliveInstMap.begin(); itor != m_kAliveInstMap.end(); ++itor)
	{
		CInstanceBase * pInstance = itor->second;

		uint32_t dwInstanceGuildID = pInstance->GetGuildID();
		if (dwSrcGuildID == dwInstanceGuildID || dwDstGuildID == dwInstanceGuildID)
		{
			pInstance->RefreshTextTail();
		}
	}
}

void CPythonCharacterManager::ClearMainInstance()
{
	m_pkInstMain=nullptr;
}

bool CPythonCharacterManager::SetMainInstance(uint32_t dwVID)
{
	m_pkInstMain=GetInstancePtr(dwVID);

	if (!m_pkInstMain)
		return false;

	return true;
}

CInstanceBase* CPythonCharacterManager::GetMainInstancePtr()
{
	return m_pkInstMain;
}

void CPythonCharacterManager::GetInfo(std::string* pstInfo)
{
	pstInfo->append("Actor: ");

	CInstanceBase::GetInfo(pstInfo);

	char szInfo[256];
	sprintf_s(szInfo, "Container - Live %u, Dead %u", m_kAliveInstMap.size(), m_kDeadInstList.size());
	pstInfo->append(szInfo);
}


bool CPythonCharacterManager::IsCacheMode()
{
	static bool s_isOldCacheMode=false;

	bool isCacheMode=s_isOldCacheMode;
	if (s_isOldCacheMode)
	{
		if (m_kAliveInstMap.size()<30)
			isCacheMode=false;
	}
	else
	{
		if (m_kAliveInstMap.size()>40)
			isCacheMode=true;
	}
	s_isOldCacheMode=isCacheMode;

	return isCacheMode;
}

void CPythonCharacterManager::Update()
{
#ifdef __PERFORMANCE_CHECKER__
	uint32_t t1=timeGetTime();
#endif
	CInstanceBase::ResetPerformanceCounter();

	CInstanceBase* pkInstMain=GetMainInstancePtr();
#ifdef __PERFORMANCE_CHECKER__
	uint32_t t2=timeGetTime();
	uint32_t dwDeadInstCount=0;
	uint32_t dwForceVisibleInstCount=0;
#endif

	TCharacterInstanceMap::iterator i=m_kAliveInstMap.begin(); 
	while (m_kAliveInstMap.end()!=i)
	{
		TCharacterInstanceMap::iterator c=i++;

		CInstanceBase* pkInstEach=c->second;
		pkInstEach->Update();

		if (pkInstMain)
		{
#ifdef __PERFORMANCE_CHECKER__
			if (pkInstEach->IsForceVisible())
			{
				dwForceVisibleInstCount++;
				// continue;
			}
#endif

			int32_t nDistance = int32_t(pkInstEach->NEW_GetDistanceFromDestInstance(*pkInstMain));
			if (nDistance > CHAR_STAGE_VIEW_BOUND + 10)
			{
				__DeleteBlendOutInstance(pkInstEach);
				m_kAliveInstMap.erase(c);
#ifdef __PERFORMANCE_CHECKER__
				dwDeadInstCount++;
#endif
			}
		}
	}
#ifdef __PERFORMANCE_CHECKER__
	uint32_t t3=timeGetTime();
#endif
	UpdateTransform();
#ifdef __PERFORMANCE_CHECKER__
	uint32_t t4=timeGetTime();
#endif

	UpdateDeleting();
#ifdef __PERFORMANCE_CHECKER__
	uint32_t t5=timeGetTime();
#endif

	__NEW_Pick();
#ifdef __PERFORMANCE_CHECKER__
	uint32_t t6=timeGetTime();
#endif

#ifdef __PERFORMANCE_CHECKER__
	{
		static FILE* fp=fopen("perf_chrmgr_update.txt", "w");

		if (t6-t1>1)
		{
			fprintf(fp, "CU.Total %d (Time %d, Alive %d, Dead %d)\n", 
				t6-t1, ELTimer_GetMSec(),
				m_kAliveInstMap.size(),
				m_kDeadInstList.size());
			fprintf(fp, "CU.Counter %d\n", t2-t1);
			fprintf(fp, "CU.ForEach %d\n", t3-t2);
			fprintf(fp, "CU.Trans %d\n", t4-t3);
			fprintf(fp, "CU.Del %d\n", t5-t4);
			fprintf(fp, "CU.Pick %d\n", t6-t5);
			fprintf(fp, "CU.AI %d\n", m_kAliveInstMap.size());
			fprintf(fp, "CU.DI %d\n", dwDeadInstCount);
			fprintf(fp, "CU.FVI %d\n", dwForceVisibleInstCount);
			fprintf(fp, "-------------------------------- \n");
			fflush(fp);
		}
	}
#endif
}

void CPythonCharacterManager::ShowPointEffect(uint32_t ePoint, uint32_t dwVID)
{
	CInstanceBase * pkInstSel = (dwVID == 0xffffffff) ? GetMainInstancePtr() : GetInstancePtr(dwVID);

	if (!pkInstSel)
		return;

	switch (ePoint)
	{
		case POINT_LEVEL:
			pkInstSel->LevelUp();
			break;
		case POINT_LEVEL_STEP:
			pkInstSel->SkillUp();
			break;
	}
}

bool CPythonCharacterManager::RegisterPointEffect(uint32_t ePoint, const char* c_szFileName)
{
	if (ePoint>=POINT_MAX_NUM)
		return false;

	CEffectManager& rkEftMgr=CEffectManager::Instance();
	rkEftMgr.RegisterEffect2(c_szFileName, &m_adwPointEffect[ePoint]);

	return true;
}

void CPythonCharacterManager::UpdateTransform()
{
#ifdef __PERFORMANCE_CHECKER__
	uint32_t t1=timeGetTime();
	uint32_t t2=timeGetTime();
#endif

	CInstanceBase * pMainInstance = GetMainInstancePtr();
	if (pMainInstance)
	{
		CPythonBackground& rkBG=CPythonBackground::Instance();
		for (TCharacterInstanceMap::iterator i = m_kAliveInstMap.begin(); i != m_kAliveInstMap.end(); ++i)
		{
			CInstanceBase * pSrcInstance = i->second;

			pSrcInstance->CheckAdvancing();

			// 2004.08.02.myevan.IsAttacked 일 경우 죽었을때도 체크하므로, 
			// 실질적으로 거리가 변경되는 IsPushing일때만 체크하도록 한다
			if (pSrcInstance->IsPushing())
				rkBG.CheckAdvancing(pSrcInstance);
		}
#ifdef __PERFORMANCE_CHECKER__
		t2=timeGetTime();
#endif

		rkBG.CheckAdvancing(m_pkInstMain);
	}

#ifdef __PERFORMANCE_CHECKER__
	uint32_t t3=timeGetTime();
#endif

	{
		for (TCharacterInstanceMap::iterator itor = m_kAliveInstMap.begin(); itor != m_kAliveInstMap.end(); ++itor)
		{
			CInstanceBase * pInstance = itor->second;
			pInstance->Transform();
		}
	}

#ifdef __PERFORMANCE_CHECKER__
	uint32_t t4=timeGetTime();
#endif

#ifdef __PERFORMANCE_CHECKER__
	{
		static FILE* fp=fopen("perf_chrmgr_updatetransform.txt", "w");

		if (t4-t1>5)
		{
			fprintf(fp, "CUT.Total %d (Time %f, Alive %d, Dead %d)\n", 
				t4-t1, ELTimer_GetMSec()/1000.0f,
				m_kAliveInstMap.size(),
				m_kDeadInstList.size());
			fprintf(fp, "CUT.ChkAdvInst %d\n", t2-t1);
			fprintf(fp, "CUT.ChkAdvBG %d\n", t3-t2);
			fprintf(fp, "CUT.Trans %d\n", t4-t3);

			fprintf(fp, "-------------------------------- \n");
			fflush(fp);
		}

		fflush(fp);
	}
#endif
}
void CPythonCharacterManager::UpdateDeleting()
{
	TCharacterInstanceList::iterator itor = m_kDeadInstList.begin();
	for (; itor != m_kDeadInstList.end();)
	{
		CInstanceBase * pInstance = *itor;

		if (pInstance->UpdateDeleting())
		{
			++itor;
		}
		else
		{
			CInstanceBase::Delete(pInstance);
			itor = m_kDeadInstList.erase(itor);
		}
	}
}

void CPythonCharacterManager::Deform()
{
	std::for_each(m_kAliveInstMap.begin(), m_kAliveInstMap.end(), [](const std::pair<uint32_t, CInstanceBase *>& cr_Pair) { cr_Pair.second->Deform(); });
	std::for_each(m_kDeadInstList.begin(), m_kDeadInstList.end(), [](CInstanceBase * pInstance) { pInstance->Deform(); });
}


bool CPythonCharacterManager::OLD_GetPickedInstanceVID(uint32_t* pdwPickedActorID)
{
	if (!m_pkInstPick)
		return false;
		
	*pdwPickedActorID=m_pkInstPick->GetVirtualID();
	return true;
}

CInstanceBase * CPythonCharacterManager::OLD_GetPickedInstancePtr()
{
	return m_pkInstPick;
}

D3DXVECTOR2 & CPythonCharacterManager::OLD_GetPickedInstPosReference()
{
	return m_v2PickedInstProjPos;
}

bool CPythonCharacterManager::IsRegisteredVID(uint32_t dwVID)
{
	if (m_kAliveInstMap.end()==m_kAliveInstMap.find(dwVID))
		return false;

	return true;
}

bool CPythonCharacterManager::IsAliveVID(uint32_t dwVID)
{
	return m_kAliveInstMap.find(dwVID)!=m_kAliveInstMap.end();
}

bool CPythonCharacterManager::IsDeadVID(uint32_t dwVID)
{
	for (TCharacterInstanceList::iterator f=m_kDeadInstList.begin(); f!=m_kDeadInstList.end(); ++f)
	{
		if ((*f)->GetVirtualID()==dwVID)
			return true;
	}

	return false;
}


void CPythonCharacterManager::__RenderSortedAliveActorList()
{
	static std::vector<CInstanceBase*> s_kVct_pkInstAliveSort;
	s_kVct_pkInstAliveSort.clear();

	TCharacterInstanceMap& rkMap_pkInstAlive=m_kAliveInstMap;
	TCharacterInstanceMap::iterator i;
	for (i=rkMap_pkInstAlive.begin(); i!=rkMap_pkInstAlive.end(); ++i)
		s_kVct_pkInstAliveSort.push_back(i->second);

	std::sort(s_kVct_pkInstAliveSort.begin(), s_kVct_pkInstAliveSort.end(), [](CInstanceBase* pkLeft, CInstanceBase* pkRight){return pkLeft->LessRenderOrder(pkRight);});
	std::for_each(s_kVct_pkInstAliveSort.begin(), s_kVct_pkInstAliveSort.end(), [](CInstanceBase* pInstance){pInstance->Render(); pInstance->RenderTrace(); });
}

void CPythonCharacterManager::__RenderSortedDeadActorList()
{
	static std::vector<CInstanceBase*> s_kVct_pkInstDeadSort;
	s_kVct_pkInstDeadSort.clear();

	TCharacterInstanceList& rkLst_pkInstDead=m_kDeadInstList;
	TCharacterInstanceList::iterator i;
	for (i=rkLst_pkInstDead.begin(); i!=rkLst_pkInstDead.end(); ++i)
		s_kVct_pkInstDeadSort.push_back(*i);

	std::sort(s_kVct_pkInstDeadSort.begin(), s_kVct_pkInstDeadSort.end(), [](CInstanceBase* pkLeft, CInstanceBase* pkRight){return pkLeft->LessRenderOrder(pkRight); });
	std::for_each(s_kVct_pkInstDeadSort.begin(), s_kVct_pkInstDeadSort.end(), [](CInstanceBase* pInstance){pInstance->Render();});

}

void CPythonCharacterManager::Render()
{
	STATEMANAGER.SetTexture(0, nullptr);	
	STATEMANAGER.SetTextureStageState(0, D3DTSS_COLORARG1,	D3DTA_TEXTURE);
	STATEMANAGER.SetTextureStageState(0, D3DTSS_COLORARG2,	D3DTA_CURRENT);
	STATEMANAGER.SetTextureStageState(0, D3DTSS_COLOROP,	D3DTOP_MODULATE);
	STATEMANAGER.SetTextureStageState(0, D3DTSS_ALPHAARG1,	D3DTA_TEXTURE);
	STATEMANAGER.SetTextureStageState(0, D3DTSS_ALPHAOP,	D3DTOP_SELECTARG1);

	STATEMANAGER.SetTexture(1, nullptr);
	STATEMANAGER.SetTextureStageState(1, D3DTSS_COLOROP,	D3DTOP_DISABLE);
	STATEMANAGER.SetTextureStageState(1, D3DTSS_ALPHAOP,	D3DTOP_DISABLE);


	__RenderSortedAliveActorList();
	__RenderSortedDeadActorList();

	CInstanceBase * pkPickedInst = OLD_GetPickedInstancePtr();
	if (pkPickedInst)
	{
		const D3DXVECTOR3 & c_rv3Position = pkPickedInst->GetGraphicThingInstanceRef().GetPosition();
		CPythonGraphic::Instance().ProjectPosition(c_rv3Position.x, c_rv3Position.y, c_rv3Position.z, &m_v2PickedInstProjPos.x, &m_v2PickedInstProjPos.y);
	}
}

void CPythonCharacterManager::RenderShadowMainInstance()
{
	CInstanceBase* pkInstMain=GetMainInstancePtr();
	if (pkInstMain)
		pkInstMain->RenderToShadowMap();
}

struct FCharacterManagerCharacterInstanceRenderToShadowMap
{
	inline void operator () (const std::pair<uint32_t,CInstanceBase *>& cr_Pair)
	{
		cr_Pair.second->RenderToShadowMap();
	}
};

void CPythonCharacterManager::RenderShadowAllInstances()
{
	std::for_each(m_kAliveInstMap.begin(), m_kAliveInstMap.end(), FCharacterManagerCharacterInstanceRenderToShadowMap());
}

struct FCharacterManagerCharacterInstanceRenderCollision
{
	inline void operator () (const std::pair<uint32_t,CInstanceBase *>& cr_Pair)
	{
		cr_Pair.second->RenderCollision();
	}
};

void CPythonCharacterManager::RenderCollision()
{
 	std::for_each(m_kAliveInstMap.begin(), m_kAliveInstMap.end(), FCharacterManagerCharacterInstanceRenderCollision());
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// Managing Process

CInstanceBase * CPythonCharacterManager::CreateInstance(const CInstanceBase::SCreateData& c_rkCreateData)
{
	CInstanceBase * pCharacterInstance = RegisterInstance(c_rkCreateData.m_dwVID);
	if (!pCharacterInstance)
	{
		TraceError("CPythonCharacterManager::CreateInstance: VID[%d] - ALREADY EXIST\n", c_rkCreateData.m_dwVID); // @fixme010
		return nullptr;
	}

	if (!pCharacterInstance->Create(c_rkCreateData))
	{
		TraceError("CPythonCharacterManager::CreateInstance VID[%d] Race[%d]", c_rkCreateData.m_dwVID, c_rkCreateData.m_dwRace);
		DeleteInstance(c_rkCreateData.m_dwVID);
		return nullptr;
	}

	if (c_rkCreateData.m_isMain)
		SelectInstance(c_rkCreateData.m_dwVID);

	return (pCharacterInstance);
}

CInstanceBase * CPythonCharacterManager::RegisterInstance(uint32_t VirtualID)
{
	TCharacterInstanceMap::iterator itor = m_kAliveInstMap.find(VirtualID);

	if (m_kAliveInstMap.end() != itor)
	{
		return nullptr;
	}

	CInstanceBase * pCharacterInstance = CInstanceBase::New();
	m_kAliveInstMap.insert(TCharacterInstanceMap::value_type(VirtualID, pCharacterInstance));

	return (pCharacterInstance);
}

void CPythonCharacterManager::DeleteInstance(uint32_t dwDelVID)
{
	TCharacterInstanceMap::iterator itor = m_kAliveInstMap.find(dwDelVID);

	if (m_kAliveInstMap.end() == itor)
	{
		Tracef("DeleteCharacterInstance: no vid by %d\n", dwDelVID);
		return;
	}

	CInstanceBase * pkInstDel = itor->second;

	if (pkInstDel == m_pkInstBind)
		m_pkInstBind = nullptr;

	if (pkInstDel == m_pkInstMain)
		m_pkInstMain = nullptr;

	if (pkInstDel == m_pkInstPick)
		m_pkInstPick = nullptr;

	CInstanceBase::Delete(pkInstDel);

	m_kAliveInstMap.erase(itor);
}

void CPythonCharacterManager::__DeleteBlendOutInstance(CInstanceBase* pkInstDel)
{
	pkInstDel->DeleteBlendOut();
	m_kDeadInstList.push_back(pkInstDel);	

	IAbstractPlayer& rkPlayer=IAbstractPlayer::GetSingleton();
	rkPlayer.NotifyCharacterDead(pkInstDel->GetVirtualID());

	if (m_pkInstMain && m_pkInstMain->IsMiningVID(pkInstDel->GetVirtualID())) {
		m_pkInstMain->CancelMining();
	}
}

void CPythonCharacterManager::DeleteInstanceByFade(uint32_t dwVID)
{
	TCharacterInstanceMap::iterator f = m_kAliveInstMap.find(dwVID);
	if (m_kAliveInstMap.end() == f)
		return;
	
	__DeleteBlendOutInstance(f->second);
	m_kAliveInstMap.erase(f);	
}

void CPythonCharacterManager::SelectInstance(uint32_t VirtualID)
{
	TCharacterInstanceMap::iterator itor = m_kAliveInstMap.find(VirtualID);

	if (m_kAliveInstMap.end() == itor)
	{
		Tracef("SelectCharacterInstance: no vid by %d\n", VirtualID);
		return;
	}

	m_pkInstBind = itor->second;
}

CInstanceBase * CPythonCharacterManager::GetInstancePtr(uint32_t VirtualID)
{
	TCharacterInstanceMap::iterator itor = m_kAliveInstMap.find(VirtualID);

	if (m_kAliveInstMap.end() == itor)
		return nullptr;

	return itor->second;
}

CInstanceBase * CPythonCharacterManager::GetInstancePtrByName(const char *name)
{
	TCharacterInstanceMap::iterator itor;

	for (itor = m_kAliveInstMap.begin(); itor != m_kAliveInstMap.end(); ++itor)
	{
		CInstanceBase * pInstance = itor->second;

		if (!strcmp(pInstance->GetNameString(), name))
			return pInstance;
	}

	return nullptr;
}

CInstanceBase * CPythonCharacterManager::GetSelectedInstancePtr()
{
	return m_pkInstBind;
}

CInstanceBase* CPythonCharacterManager::FindClickableInstancePtr()
{
	return nullptr;
}

void CPythonCharacterManager::__UpdateSortPickedActorList()
{
	__UpdatePickedActorList();
	__SortPickedActorList();
}

void CPythonCharacterManager::__UpdatePickedActorList()
{
	m_kVct_pkInstPicked.clear();

	TCharacterInstanceMap::iterator i;
	for (i=m_kAliveInstMap.begin(); i!=m_kAliveInstMap.end(); ++i)
	{
		CInstanceBase* pkInstEach=i->second;
		// 2004.07.17.levites.isShow를 ViewFrustumCheck로 변경
		if (pkInstEach->CanPickInstance())
		{
			if (pkInstEach->IsDead())
			{
				if (pkInstEach->IntersectBoundingBox())
					m_kVct_pkInstPicked.push_back(pkInstEach);
			}
			else
			{
				if (pkInstEach->IntersectDefendingSphere())
					m_kVct_pkInstPicked.push_back(pkInstEach);
			}
		}
	}
}

struct CInstanceBase_SLessCameraDistance
{
	TPixelPosition m_kPPosEye;

	bool operator() (CInstanceBase* pkInstLeft, CInstanceBase* pkInstRight)
	{
		int32_t nLeftDeadPoint=pkInstLeft->IsDead();
		int32_t nRightDeadPoint=pkInstRight->IsDead();

		if (nLeftDeadPoint<nRightDeadPoint)
			return true;

		if (pkInstLeft->CalculateDistanceSq3d(m_kPPosEye)<pkInstRight->CalculateDistanceSq3d(m_kPPosEye))
			return true;

		return false;
	}
};

void CPythonCharacterManager::__SortPickedActorList()
{
	CCamera * pCamera = CCameraManager::Instance().GetCurrentCamera();
	const D3DXVECTOR3& c_rv3EyePos=pCamera->GetEye();

	CInstanceBase_SLessCameraDistance kLess;
	kLess.m_kPPosEye=TPixelPosition(+c_rv3EyePos.x, -c_rv3EyePos.y, +c_rv3EyePos.z);

	std::sort(m_kVct_pkInstPicked.begin(), m_kVct_pkInstPicked.end(), kLess);
}

void CPythonCharacterManager::__NEW_Pick()
{
	__UpdateSortPickedActorList();

	CInstanceBase* pkInstMain=GetMainInstancePtr();

#ifdef __MOVIE_MODE
	if (pkInstMain)
		if (pkInstMain->IsMovieMode())
		{
			if (m_pkInstPick)
				m_pkInstPick->OnUnselected();
			return;
		}
#endif

	// 정밀한 체크
	{
		std::vector<CInstanceBase*>::iterator f;
		for (f=m_kVct_pkInstPicked.begin(); f!=m_kVct_pkInstPicked.end(); ++f)
		{
			CInstanceBase* pkInstEach=*f;
			if (pkInstEach!=pkInstMain && pkInstEach->IntersectBoundingBox())
			{
				if (m_pkInstPick)
					if (m_pkInstPick!=pkInstEach)
						m_pkInstPick->OnUnselected();

				if (pkInstEach->CanPickInstance())
				{
					m_pkInstPick = pkInstEach;
					m_pkInstPick->OnSelected();
					return;
				}
			}
		}
	}

	// 못찾겠으면 걍 순서대로
	{
		std::vector<CInstanceBase*>::iterator f;
		for (f=m_kVct_pkInstPicked.begin(); f!=m_kVct_pkInstPicked.end(); ++f)
		{
			CInstanceBase* pkInstEach=*f;
			if (pkInstEach!=pkInstMain)
			{
				if (m_pkInstPick)
					if (m_pkInstPick!=pkInstEach)
						m_pkInstPick->OnUnselected();

				if (pkInstEach->CanPickInstance())
				{
					m_pkInstPick = pkInstEach;
					m_pkInstPick->OnSelected();
					return;
				}
			}
		}
	}

	if (pkInstMain)
	if (pkInstMain->CanPickInstance())
	if (m_kVct_pkInstPicked.end() != std::find(m_kVct_pkInstPicked.begin(), m_kVct_pkInstPicked.end(), pkInstMain))
	{
		if (m_pkInstPick)
			if (m_pkInstPick!=pkInstMain)
				m_pkInstPick->OnUnselected();			

		m_pkInstPick = pkInstMain;
		m_pkInstPick->OnSelected();
		return;
	}

	if (m_pkInstPick)
	{
		m_pkInstPick->OnUnselected();	
		m_pkInstPick=nullptr;
	}
}

void CPythonCharacterManager::__OLD_Pick()
{
	for (TCharacterInstanceMap::iterator itor = m_kAliveInstMap.begin(); itor != m_kAliveInstMap.end(); ++itor)
	{
		CInstanceBase * pkInstEach = itor->second;

		if (pkInstEach == m_pkInstMain)
			continue;

		if (pkInstEach->IntersectDefendingSphere())
		{
			if (m_pkInstPick)
				if (m_pkInstPick!=pkInstEach)
					m_pkInstPick->OnUnselected();	

			m_pkInstPick = pkInstEach;
			m_pkInstPick->OnSelected();

			return;
		}
	}

	if (m_pkInstPick)
	{
		m_pkInstPick->OnUnselected();	
		m_pkInstPick=nullptr;
	}
}

int32_t CPythonCharacterManager::PickAll()
{
	for (TCharacterInstanceMap::iterator itor = m_kAliveInstMap.begin(); itor != m_kAliveInstMap.end(); ++itor)
	{
		CInstanceBase * pInstance = itor->second;

		if (pInstance->IntersectDefendingSphere())
			return pInstance->GetVirtualID();
	}

	return -1;
}

CInstanceBase * CPythonCharacterManager::GetCloseInstance(CInstanceBase * pInstance)
{
	float fMinDistance = 10000.0f;
	CInstanceBase * pCloseInstance = nullptr;

	TCharacterInstanceMap::iterator itor = m_kAliveInstMap.begin();
	for (; itor != m_kAliveInstMap.end(); ++itor)
	{
		CInstanceBase * pTargetInstance = itor->second;

		if (pTargetInstance == pInstance)
			continue;

		uint32_t dwVirtualNumber = pTargetInstance->GetVirtualNumber();
		if (CPythonNonPlayer::ON_CLICK_EVENT_BATTLE != CPythonNonPlayer::Instance().GetEventType(dwVirtualNumber))
			continue;

		float fDistance = pInstance->GetDistance(pTargetInstance);
		if (fDistance < fMinDistance)
		{
			fMinDistance = fDistance;
			pCloseInstance = pTargetInstance;
		}
	}

	return pCloseInstance;
}

void CPythonCharacterManager::RefreshAllPCTextTail()
{
	CPythonCharacterManager::CharacterIterator itor = CharacterInstanceBegin();
	CPythonCharacterManager::CharacterIterator itorEnd = CharacterInstanceEnd();
	for (; itor != itorEnd; ++itor)
	{
		CInstanceBase * pInstance = *itor;
		if (!pInstance->IsPC())
			continue;

		pInstance->RefreshTextTail();
	}
}

void CPythonCharacterManager::RefreshAllGuildMark()
{
	CPythonCharacterManager::CharacterIterator itor = CharacterInstanceBegin();
	CPythonCharacterManager::CharacterIterator itorEnd = CharacterInstanceEnd();
	for (; itor != itorEnd; ++itor)
	{
		CInstanceBase * pInstance = *itor;
		if (!pInstance->IsPC())
			continue;

		pInstance->ChangeGuild(pInstance->GetGuildID());
		pInstance->RefreshTextTail();
	}
}

void CPythonCharacterManager::RefreshGuildSymbols(std::set<uint32_t> guildIDSet)
{
	CPythonCharacterManager::CharacterIterator itor = CharacterInstanceBegin();
	CPythonCharacterManager::CharacterIterator itorEnd = CharacterInstanceEnd();
	for (; itor != itorEnd; ++itor)
	{
		CInstanceBase * pInstance = *itor;

		// Guild Symbol
		if (pInstance->GetRace() != 14200)
			continue;

		uint32_t gid = pInstance->GetGuildID();
		bool isInList = (guildIDSet.find(gid) != guildIDSet.end());

		// We found a symbol but it was not part of this update.
		if (!isInList)
			continue;

		std::string strFileName = GetGuildSymbolFileName(gid);
		CActorInstance * guildSymbolModel = pInstance->GetGraphicThingInstancePtr();
		if (!guildSymbolModel)
			continue;

		if (IsFile(strFileName.c_str()))
			guildSymbolModel->ChangeMaterial(strFileName.c_str());
		else
			guildSymbolModel->ChangeMaterial("d:/ymir work/guild/object/guild_symbol/guild_symbol01.dds"); // This makes it possible to delete symbols.
	}
}
void CPythonCharacterManager::DeleteAllInstances()
{
	DestroyAliveInstanceMap();
	DestroyDeadInstanceList();
}


void CPythonCharacterManager::DestroyAliveInstanceMap()
{
	for (TCharacterInstanceMap::iterator i = m_kAliveInstMap.begin(); i != m_kAliveInstMap.end(); ++i)
		CInstanceBase::Delete(i->second);

	m_kAliveInstMap.clear();
}

void CPythonCharacterManager::DestroyDeadInstanceList()
{
	std::for_each(m_kDeadInstList.begin(), m_kDeadInstList.end(), CInstanceBase::Delete);
	m_kDeadInstList.clear();
}

void CPythonCharacterManager::Destroy()
{
	DeleteAllInstances();

	CInstanceBase::DestroySystem();

	__Initialize();
}

void CPythonCharacterManager::__Initialize()
{
	memset(m_adwPointEffect, 0, sizeof(m_adwPointEffect));
	m_pkInstMain = nullptr;
	m_pkInstBind = nullptr;
	m_pkInstPick = nullptr;
	m_v2PickedInstProjPos = D3DXVECTOR2(0.0f, 0.0f);
}


CPythonCharacterManager::CPythonCharacterManager()
{
	__Initialize();
}

CPythonCharacterManager::~CPythonCharacterManager()
{
	Destroy();
}
