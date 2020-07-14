#include "StdAfx.h"
#include "InstanceBase.h"
#include "PythonBackground.h"
#include "PythonNonPlayer.h"
#include "PythonPlayer.h"
#include "PythonCharacterManager.h"
#include "AbstractPlayer.h"
#include "AbstractApplication.h"
#include "../eterLib/StateManager.h"
#include "../eterGameLib/ItemManager.h"
#include "PythonSystem.h"
#include "PythonTextTail.h"

BOOL HAIR_COLOR_ENABLE=FALSE;
BOOL USE_ARMOR_SPECULAR=FALSE;
BOOL RIDE_HORSE_ENABLE=TRUE;
const float c_fDefaultRotationSpeed = 1200.0f;
const float c_fDefaultHorseRotationSpeed = 300.0f;

bool IsWall(uint32_t race)
{
	switch (race)
	{
		case 14201:
		case 14202:
		case 14203:
		case 14204:
			return true;
	}
	return false;
}

//////////////////////////////////////////////////////////////////////////////////////


CInstanceBase::SHORSE::SHORSE()
{
	__Initialize();
}

CInstanceBase::SHORSE::~SHORSE()
{
	assert(m_pkActor==nullptr);
}

void CInstanceBase::SHORSE::__Initialize()
{
	m_isMounting=false;
	m_pkActor=nullptr;
}

void CInstanceBase::SHORSE::SetAttackSpeed(uint32_t uAtkSpd)
{
	if (!IsMounting())
		return;

	CActorInstance& rkActor=GetActorRef();
	rkActor.SetAttackSpeed(uAtkSpd/100.0f);	
}

void CInstanceBase::SHORSE::SetMoveSpeed(uint32_t uMovSpd)
{	
	if (!IsMounting())
		return;

	CActorInstance& rkActor=GetActorRef();
	rkActor.SetMoveSpeed(uMovSpd/100.0f);
}

void CInstanceBase::SHORSE::Create(const TPixelPosition& c_rkPPos, uint32_t eRace, uint32_t eHitEffect)
{
	assert(nullptr==m_pkActor && "CInstanceBase::SHORSE::Create - ALREADY MOUNT");

	m_pkActor=new CActorInstance;

	CActorInstance& rkActor=GetActorRef();
	rkActor.SetEventHandler(CActorInstance::IEventHandler::GetEmptyPtr());
	if (!rkActor.SetRace(eRace))
	{
		delete m_pkActor;
		m_pkActor=nullptr;
		return;
	}

	rkActor.SetShape(0);
	rkActor.SetBattleHitEffect(eHitEffect);
	rkActor.SetAlphaValue(0.0f);
	rkActor.BlendAlphaValue(1.0f, 0.5f);
	rkActor.SetMoveSpeed(1.0f);
	rkActor.SetAttackSpeed(1.0f);
	rkActor.SetMotionMode(CRaceMotionData::MODE_GENERAL);
	rkActor.Stop();
	rkActor.RefreshActorInstance();

	rkActor.SetCurPixelPosition(c_rkPPos);

	m_isMounting=true;
}

void CInstanceBase::SHORSE::Destroy()
{
	if (m_pkActor)
	{
		m_pkActor->Destroy();
		delete m_pkActor;	
	}	

	__Initialize();
}

CActorInstance& CInstanceBase::SHORSE::GetActorRef()
{
	assert(nullptr!=m_pkActor && "CInstanceBase::SHORSE::GetActorRef");
	return *m_pkActor;
}

CActorInstance* CInstanceBase::SHORSE::GetActorPtr()
{
	return m_pkActor;
}

enum eMountType
{
	MOUNT_TYPE_NONE = 0,
	MOUNT_TYPE_NORMAL = 1,
	MOUNT_TYPE_COMBAT = 2,
	MOUNT_TYPE_MILITARY = 3
};
eMountType GetMountLevelByVnum(uint32_t dwMountVnum, bool IsNew)
{
	if (!dwMountVnum)
		return MOUNT_TYPE_NONE;

	switch (dwMountVnum)
	{
		// ### YES SKILL
		case 20107: // normal military horse (no guild)
		case 20108: // normal military horse (guild member)
		case 20109: // normal military horse (guild master)
			if (IsNew)
				return MOUNT_TYPE_NONE;
		// Classic
		case 20110: // Classic Boar
		case 20111: // Classic Wolf
		case 20112: // Classic Tiger
		case 20113: // Classic Lion
		case 20114: // White Lion
		// Special Lv2
		case 20115: // Wild Battle Boar
		case 20116: // Fight Wolf
		case 20117: // Storm Tiger
		case 20118: // Battle Lion (bugged)
		case 20205: // Wild Battle Boar (alternative)
		case 20206: // Fight Wolf (alternative)
		case 20207: // Storm Tiger (alternative)
		case 20208: // Battle Lion (bugged) (alternative)
		// Royal Tigers
		case 20120: // blue
		case 20121: // dark red
		case 20122: // gold
		case 20123: // green
		case 20124: // pied
		case 20125: // white
		// Royal mounts (Special Lv3)
		case 20209: // Royal Boar
		case 20210: // Royal Wolf
		case 20211: // Royal Tiger
		case 20212: // Royal Lion
		//
		case 20215: // Rudolph m Lv3 (yes skill, yes atk)
		case 20218: // Rudolph f Lv3 (yes skill, yes atk)
		case 20225: // Dyno Lv3 (yes skill, yes atk)
		case 20230: // Turkey Lv3 (yes skill, yes atk)
			return MOUNT_TYPE_MILITARY;
		// ### NO SKILL YES ATK
		case 20104: // normal combat horse (no guild)
		case 20105: // normal combat horse (guild member)
		case 20106: // normal combat horse (guild master)
			if (IsNew)
				return MOUNT_TYPE_NONE;
		case 20119: // Black Horse (no skill, yes atk)
		case 20214: // Rudolph m Lv2 (no skill, yes atk)
		case 20217: // Rudolph f Lv2 (no skill, yes atk)
		case 20219: // Equus Porphyreus (no skill, yes atk)
		case 20220: // Comet (no skill, yes atk)
		case 20221: // Polar Predator (no skill, yes atk)
		case 20222: // Armoured Panda (no skill, yes atk)
		case 20224: // Dyno Lv2 (no skill, yes atk)
		case 20226: // Nightmare (no skill, yes atk)
		case 20227: // Unicorn (no skill, yes atk)
		case 20229: // Turkey Lv2 (no skill, yes atk)
		case 20231: // Leopard (no skill, yes atk)
		case 20232: // Black Panther (no skill, yes atk)
			return MOUNT_TYPE_COMBAT;
		// ### NO SKILL NO ATK
		case 20101: // normal beginner horse (no guild)
		case 20102: // normal beginner horse (guild member)
		case 20103: // normal beginner horse (guild master)
			if (IsNew)
				return MOUNT_TYPE_NONE;
		case 20213: // Rudolph m Lv1 (no skill, no atk)
		case 20216: // Rudolph f Lv1 (no skill, no atk)
		// Special Lv1
		case 20201: // Boar Lv1 (no skill, no atk)
		case 20202: // Wolf Lv1 (no skill, no atk)
		case 20203: // Tiger Lv1 (no skill, no atk)
		case 20204: // Lion Lv1 (no skill, no atk)
		//
		case 20223: // Dyno Lv1 (no skill, no atk)
		case 20228: // Turkey Lv1 (no skill, no atk)
			return MOUNT_TYPE_NORMAL;
		default:
			return MOUNT_TYPE_NONE;
	}
}

uint32_t CInstanceBase::SHORSE::GetLevel()
{
	if (m_pkActor)
	{
#ifndef ENABLE_NO_MOUNT_CHECK
		return static_cast<uint32_t>(GetMountLevelByVnum(m_pkActor->GetRace(), false));
#else
		return (m_pkActor->GetRace()) ? MOUNT_TYPE_MILITARY : MOUNT_TYPE_NONE;
#endif
	}
	return 0;
}

bool CInstanceBase::SHORSE::IsNewMount()
{
#ifndef ENABLE_NO_MOUNT_CHECK
	if (m_pkActor)
	{
		uint32_t dwMountVnum = m_pkActor->GetRace();
		eMountType mountType = GetMountLevelByVnum(dwMountVnum, true);
		return (mountType != MOUNT_TYPE_NONE) && (mountType != MOUNT_TYPE_NORMAL);
	}
#endif
	return false;
}
bool CInstanceBase::SHORSE::CanUseSkill()
{
	// 마상스킬은 말의 레벨이 3 이상이어야만 함.
	if (IsMounting())
		return 2 < GetLevel();

	return true;
}

bool CInstanceBase::SHORSE::CanAttack()
{
	if (IsMounting())
		if (GetLevel()<=1)
			return false;

	return true;
}
			
bool CInstanceBase::SHORSE::IsMounting()
{
	return m_isMounting;
}

void CInstanceBase::SHORSE::Deform()
{
	if (!IsMounting())
		return;

	CActorInstance& rkActor=GetActorRef();
	rkActor.INSTANCEBASE_Deform();
}

void CInstanceBase::SHORSE::Render()
{
	if (!IsMounting())
		return;

	CActorInstance& rkActor=GetActorRef();
	rkActor.Render();
}

void CInstanceBase::__AttachHorseSaddle()
{
	if (!IsMountingHorse())
		return;
	m_kHorse.m_pkActor->AttachModelInstance(CRaceData::PART_MAIN, "saddle", m_GraphicThingInstance, CRaceData::PART_MAIN);
}

void CInstanceBase::__DetachHorseSaddle()
{
	if (!IsMountingHorse())
		return;
	m_kHorse.m_pkActor->DetachModelInstance(CRaceData::PART_MAIN, m_GraphicThingInstance, CRaceData::PART_MAIN);
}

//////////////////////////////////////////////////////////////////////////////////////

void CInstanceBase::BlockMovement()
{
	m_GraphicThingInstance.BlockMovement();
}

bool CInstanceBase::IsBlockObject(const CGraphicObjectInstance& c_rkBGObj)
{
	return m_GraphicThingInstance.IsBlockObject(c_rkBGObj);
}

bool CInstanceBase::AvoidObject(const CGraphicObjectInstance& c_rkBGObj)
{
	return m_GraphicThingInstance.AvoidObject(c_rkBGObj);
}

///////////////////////////////////////////////////////////////////////////////////

bool __ArmorVnumToShape(int32_t iVnum, uint32_t * pdwShape)
{
	*pdwShape = iVnum;

	/////////////////////////////////////////

	if (0 == iVnum || 1 == iVnum)
		return false;

	if (!USE_ARMOR_SPECULAR)
		return false;

	CItemData * pItemData;
	if (!CItemManager::Instance().GetItemDataPointer(iVnum, &pItemData))
		return false;

	enum
	{
		SHAPE_VALUE_SLOT_INDEX = 3
	};

	*pdwShape = pItemData->GetValue(SHAPE_VALUE_SLOT_INDEX);

	return true;
}

class CActorInstanceBackground : public IBackground
{
	public:
		CActorInstanceBackground() {}
		virtual ~CActorInstanceBackground() {}
		bool IsBlock(int32_t x, int32_t y)
		{
			CPythonBackground& rkBG=CPythonBackground::Instance();
			return rkBG.isAttrOn(x, y, CTerrainImpl::ATTRIBUTE_BLOCK);
		}
};

static CActorInstanceBackground gs_kActorInstBG;

void CInstanceBase::SetLODLimits(uint32_t index, float fLimit)
{

	CGrannyLODController* pLODCtrl = m_GraphicThingInstance.GetLODControllerPointer(index);
	if (pLODCtrl)
	{
		pLODCtrl->SetLODLimits(0.0, fLimit);
	}
}

bool CInstanceBase::LessRenderOrder(CInstanceBase* pkInst)
{
	int32_t nMainAlpha=(__GetAlphaValue() < 1.0f) ? 1 : 0;
	int32_t nTestAlpha=(pkInst->__GetAlphaValue() < 1.0f) ? 1 : 0;
	if (nMainAlpha < nTestAlpha)
		return true;
	if (nMainAlpha > nTestAlpha)
		return false;

	if (GetRace()<pkInst->GetRace())
		return true;
	if (GetRace()>pkInst->GetRace())
		return false;

	if (GetShape()<pkInst->GetShape())
		return true;

	if (GetShape()>pkInst->GetShape())
		return false;

	uint32_t uLeftLODLevel=__LessRenderOrder_GetLODLevel();
	uint32_t uRightLODLevel=pkInst->__LessRenderOrder_GetLODLevel();
	if (uLeftLODLevel<uRightLODLevel)
		return true;
	if (uLeftLODLevel>uRightLODLevel)
		return false;

	if (m_awPart[CRaceData::PART_WEAPON]<pkInst->m_awPart[CRaceData::PART_WEAPON])
		return true;

	return false;
}

uint32_t CInstanceBase::__LessRenderOrder_GetLODLevel()
{
	CGrannyLODController* pLODCtrl=m_GraphicThingInstance.GetLODControllerPointer(0);
	if (!pLODCtrl)
		return 0;

	return pLODCtrl->GetLODLevel();
}

bool CInstanceBase::__Background_GetWaterHeight(const TPixelPosition& c_rkPPos, float* pfHeight)
{
	int32_t lHeight;
	if (!CPythonBackground::Instance().GetWaterHeight(int32_t(c_rkPPos.x), int32_t(c_rkPPos.y), &lHeight))
		return false;

	*pfHeight = float(lHeight);

	return true;
}

bool CInstanceBase::__Background_IsWaterPixelPosition(const TPixelPosition& c_rkPPos)
{
	return CPythonBackground::Instance().isAttrOn(c_rkPPos.x, c_rkPPos.y, CTerrainImpl::ATTRIBUTE_WATER);
}

const float PC_DUST_RANGE = 2000.0f;
const float NPC_DUST_RANGE = 1000.0f;

uint32_t CInstanceBase::ms_dwUpdateCounter=0;
uint32_t CInstanceBase::ms_dwRenderCounter=0;
uint32_t CInstanceBase::ms_dwDeformCounter=0;

CDynamicPool<CInstanceBase> CInstanceBase::ms_kPool;

bool CInstanceBase::__IsInDustRange()
{
	if (!__IsExistMainInstance())
		return false;

	CInstanceBase* pkInstMain=__GetMainInstancePtr();

	float fDistance=NEW_GetDistanceFromDestInstance(*pkInstMain);

	if (IsPC())
	{
		if (fDistance<=PC_DUST_RANGE)
			return true;
	}

	if (fDistance<=NPC_DUST_RANGE)
		return true;

	return false;
}

void CInstanceBase::__EnableSkipCollision()
{
	if (__IsMainInstance())
	{
		TraceError("CInstanceBase::__EnableSkipCollision - You should not skip your own collisions!!");
		return;
	}
	m_GraphicThingInstance.EnableSkipCollision();
}

void CInstanceBase::__DisableSkipCollision()
{
	m_GraphicThingInstance.DisableSkipCollision();
}

bool CInstanceBase::__CanSkipCollision()
{
	return m_GraphicThingInstance.CanSkipCollision();
}

uint32_t CInstanceBase::__GetShadowMapColor(float x, float y)
{
	CPythonBackground& rkBG=CPythonBackground::Instance();
	return rkBG.GetShadowMapColor(x, y);
}

float CInstanceBase::__GetBackgroundHeight(float x, float y)
{
	CPythonBackground& rkBG=CPythonBackground::Instance();
	return rkBG.GetHeight(x, y);
}

BOOL CInstanceBase::IsInvisibility()
{
	//GM invisibility | Stealth | Recently revived invisibility
	if ((IsAffect(AFFECT_INVISIBILITY) || IsAffect(AFFECT_EUNHYEONG) || IsAffect(AFFECT_REVIVE_INVISIBILITY))
#ifdef ENABLE_CANSEEHIDDENTHING_FOR_GM
		 && !__MainCanSeeHiddenThing()
	)
#endif
	{
		return true;
	}

	return false;
}

BOOL CInstanceBase::IsParalysis()
{
	return m_GraphicThingInstance.IsParalysis();
}

BOOL CInstanceBase::IsGameMaster()
{
	if (m_kAffectFlagContainer.IsSet(AFFECT_YMIR))
		return true;
	return false;
}


BOOL CInstanceBase::IsSameEmpire(CInstanceBase& rkInstDst)
{
	if (0 == rkInstDst.m_dwEmpireID)
		return TRUE;

	if (IsGameMaster())
		return TRUE;

	if (rkInstDst.IsGameMaster())
		return TRUE;

	if (rkInstDst.m_dwEmpireID==m_dwEmpireID)
		return TRUE;

	return FALSE;
}

uint32_t CInstanceBase::GetEmpireID()
{
	return m_dwEmpireID;
}

uint32_t CInstanceBase::GetGuildID()
{
	return m_dwGuildID;
}

int32_t CInstanceBase::GetAlignment()
{
	return m_sAlignment;
}

uint32_t CInstanceBase::GetAlignmentGrade()
{
	if (m_sAlignment >= 12000)
		return 0;
	else if (m_sAlignment >= 8000)
		return 1;
	else if (m_sAlignment >= 4000)
		return 2;
	else if (m_sAlignment >= 1000)
		return 3;
	else if (m_sAlignment >= 0)
		return 4;
	else if (m_sAlignment > -4000)
		return 5;
	else if (m_sAlignment > -8000)
		return 6;
	else if (m_sAlignment > -12000)
		return 7;

	return 8;
}

int32_t CInstanceBase::GetAlignmentType()
{
	switch (GetAlignmentGrade())
	{
		case 0:
		case 1:
		case 2:
		case 3:
		{
			return ALIGNMENT_TYPE_WHITE;
		}

		case 5:
		case 6:
		case 7:
		case 8:
		{
			return ALIGNMENT_TYPE_DARK;
		}
	}

	return ALIGNMENT_TYPE_NORMAL;
}

uint8_t CInstanceBase::GetPKMode()
{
	return m_byPKMode;
}

bool CInstanceBase::IsKiller()
{
	return m_isKiller;
}

bool CInstanceBase::IsPartyMember()
{
	return m_isPartyMember;
}

BOOL CInstanceBase::IsInSafe()
{
	const TPixelPosition& c_rkPPosCur=m_GraphicThingInstance.NEW_GetCurPixelPositionRef();
	if (CPythonBackground::Instance().isAttrOn(c_rkPPosCur.x, c_rkPPosCur.y, CTerrainImpl::ATTRIBUTE_BANPK))
		return TRUE;

	return FALSE;
}

float CInstanceBase::CalculateDistanceSq3d(const TPixelPosition& c_rkPPosDst)
{
	const TPixelPosition& c_rkPPosSrc=m_GraphicThingInstance.NEW_GetCurPixelPositionRef();
	return SPixelPosition_CalculateDistanceSq3d(c_rkPPosSrc, c_rkPPosDst);
}

void CInstanceBase::OnSelected()
{
	if (IsStoneDoor())
		return;

	if (IsDead())
		return;

	__AttachSelectEffect();
}

void CInstanceBase::OnUnselected()
{
	__DetachSelectEffect();
}

void CInstanceBase::OnTargeted()
{
	if (IsStoneDoor())
		return;

	if (IsDead())
		return;

	__AttachTargetEffect();
}

void CInstanceBase::OnUntargeted()
{
	__DetachTargetEffect();
}

void CInstanceBase::DestroySystem()
{
	ms_kPool.Clear();
}

void CInstanceBase::CreateSystem(uint32_t uCapacity)
{
	ms_kPool.Create(uCapacity);

	memset(ms_adwCRCAffectEffect, 0, sizeof(ms_adwCRCAffectEffect));

	ms_fDustGap=250.0f;
	ms_fHorseDustGap=500.0f;
}

CInstanceBase* CInstanceBase::New()
{
	return ms_kPool.Alloc();
}

void CInstanceBase::Delete(CInstanceBase* pkInst)
{
	pkInst->Destroy();
	ms_kPool.Free(pkInst);
}

void CInstanceBase::SetMainInstance()
{
	CPythonCharacterManager& rkChrMgr=CPythonCharacterManager::Instance();

	uint32_t dwVID=GetVirtualID();
	rkChrMgr.SetMainInstance(dwVID);

	m_GraphicThingInstance.SetMainInstance();
}

CInstanceBase* CInstanceBase::__GetMainInstancePtr()
{
	CPythonCharacterManager& rkChrMgr=CPythonCharacterManager::Instance();
	return rkChrMgr.GetMainInstancePtr();
}

void CInstanceBase::__ClearMainInstance()
{
	CPythonCharacterManager& rkChrMgr=CPythonCharacterManager::Instance();
	rkChrMgr.ClearMainInstance();
}

/* 실제 플레이어 캐릭터인지 조사.*/
bool CInstanceBase::__IsMainInstance()
{
	if (this==__GetMainInstancePtr())
		return true;

	return false;
}

bool CInstanceBase::__IsExistMainInstance()
{
	if(__GetMainInstancePtr())
		return true;
	return false;
}

bool CInstanceBase::__MainCanSeeHiddenThing()
{
#ifdef ENABLE_CANSEEHIDDENTHING_FOR_GM
	CInstanceBase * pInstance = __GetMainInstancePtr();
	return (pInstance) ? TRUE == pInstance->IsGameMaster() : false;
#else
	return false;
#endif
}

float CInstanceBase::__GetBowRange()
{
	float fRange = 2500.0f - 100.0f;

	if (__IsMainInstance())
	{
		IAbstractPlayer& rPlayer=IAbstractPlayer::GetSingleton();
		fRange += float(rPlayer.GetStatus(POINT_BOW_DISTANCE));
	}

	return fRange;
}

CInstanceBase* CInstanceBase::__FindInstancePtr(uint32_t dwVID)
{
	CPythonCharacterManager& rkChrMgr=CPythonCharacterManager::Instance();
	return rkChrMgr.GetInstancePtr(dwVID);
}

bool CInstanceBase::__FindRaceType(uint32_t dwRace, uint8_t* pbType)
{
	CPythonNonPlayer& rkNonPlayer=CPythonNonPlayer::Instance();
	return rkNonPlayer.GetInstanceType(dwRace, pbType);
}

bool CInstanceBase::Create(const SCreateData& c_rkCreateData)
{
	IAbstractApplication::GetSingleton().SkipRenderBuffering(300);

	SetInstanceType(c_rkCreateData.m_bType);


	if (!SetRace(c_rkCreateData.m_dwRace))
		return false;

	SetVirtualID(c_rkCreateData.m_dwVID);

	if (c_rkCreateData.m_isMain)
		SetMainInstance();

	if (IsGuildWall())
	{
		uint32_t center_x;
		uint32_t center_y;

		c_rkCreateData.m_kAffectFlags.ConvertToPosition(&center_x, &center_y);
		
		float center_z = __GetBackgroundHeight(center_x, center_y);
		NEW_SetPixelPosition(TPixelPosition(float(c_rkCreateData.m_lPosX), float(c_rkCreateData.m_lPosY), center_z));
	}
	else
	{
		SCRIPT_SetPixelPosition(float(c_rkCreateData.m_lPosX), float(c_rkCreateData.m_lPosY));
	}	

	if (0 != c_rkCreateData.m_dwMountVnum)
		MountHorse(c_rkCreateData.m_dwMountVnum);

	SetArmor(c_rkCreateData.m_dwArmor);

	if (IsPC() || (IsNPC() && c_rkCreateData.m_dwHair))
		SetHair(c_rkCreateData.m_dwHair);

	if (IsPC())
	{
		SetWeapon(c_rkCreateData.m_dwWeapon);
#ifdef ENABLE_ACCE_SYSTEM
		SetAcce(c_rkCreateData.m_dwAcce);
#endif
	}

	__Create_SetName(c_rkCreateData);

#if defined(WJ_SHOW_MOB_INFO) && defined(ENABLE_SHOW_MOBLEVEL)
	if (IsEnemy() && CPythonSystem::Instance().IsShowMobLevel())
		m_dwLevel = CPythonNonPlayer::Instance().GetMonsterLevel(GetRace());
	else
		m_dwLevel = c_rkCreateData.m_dwLevel;
#else
	m_dwLevel = c_rkCreateData.m_dwLevel;
#endif

	m_dwGuildID = c_rkCreateData.m_dwGuildID;
	m_dwEmpireID = c_rkCreateData.m_dwEmpireID;

	SetVirtualNumber(c_rkCreateData.m_dwRace);
	SetRotation(c_rkCreateData.m_fRot);

	SetAlignment(c_rkCreateData.m_sAlignment);
	SetPKMode(c_rkCreateData.m_byPKMode);

	SetMoveSpeed(c_rkCreateData.m_dwMovSpd);
	SetAttackSpeed(c_rkCreateData.m_dwAtkSpd);
	
	// NOTE : Dress 를 입고 있으면 Alpha 를 넣지 않는다.
	if (!IsWearingDress())
	{
		// NOTE : 반드시 Affect 셋팅 윗쪽에 있어야 함
		m_GraphicThingInstance.SetAlphaValue(0.0f);
		m_GraphicThingInstance.BlendAlphaValue(1.0f, 0.5f);
	}

	if (!IsGuildWall())
		SetAffectFlagContainer(c_rkCreateData.m_kAffectFlags);

	// NOTE : 반드시 Affect 셋팅 후에 해야 함
	AttachTextTail();
	RefreshTextTail();

	if (c_rkCreateData.m_dwStateFlags & ADD_CHARACTER_STATE_SPAWN) 
	{
		if (IsAffect(AFFECT_SPAWN))
			__AttachEffect(EFFECT_SPAWN_APPEAR);

		if (IsPC())
			Refresh(CRaceMotionData::NAME_WAIT, true);
		else
			Refresh(CRaceMotionData::NAME_SPAWN, false);
	}
	else
		Refresh(CRaceMotionData::NAME_WAIT, true);

	__AttachEmpireEffect(c_rkCreateData.m_dwEmpireID);

	RegisterBoundingSphere();

	if (c_rkCreateData.m_dwStateFlags & ADD_CHARACTER_STATE_DEAD)
		m_GraphicThingInstance.DieEnd();

	SetStateFlags(c_rkCreateData.m_dwStateFlags);

	m_GraphicThingInstance.SetBattleHitEffect(ms_adwCRCAffectEffect[EFFECT_HIT]);

	if (!IsPC())
	{
		uint32_t dwBodyColor = CPythonNonPlayer::Instance().GetMonsterColor(c_rkCreateData.m_dwRace);
		if (0 != dwBodyColor)
		{
			SetModulateRenderMode();
			SetAddColor(dwBodyColor);
		}

#ifdef ENABLE_OBJ_SCALLING
//		const auto table = CPythonNonPlayer::Instance().GetTable(c_rkCreateData.m_dwRace);
//		// NPC scalling according to its value
//		if (table && table->fScale)
//			m_GraphicThingInstance.SetScale(table->fScale);
#endif
	}

	__AttachHorseSaddle();

	// If we receive the guild symbol update its texture if we have one for it.
	if (14200 == GetRace())
	{
		std::string strFileName = GetGuildSymbolFileName(m_dwGuildID);
		if (IsFile(strFileName.c_str()))
			m_GraphicThingInstance.ChangeMaterial(strFileName.c_str());
	}

	//Shining on models
	std::vector<uint32_t>c_vShineRaces;
	c_vShineRaces.push_back(29000);

	if (std::find(c_vShineRaces.begin(), c_vShineRaces.end(), GetRace()) != c_vShineRaces.end())
	{
		SMaterialData kMaterialData;
		kMaterialData.pImage = nullptr;
		kMaterialData.isSpecularEnable = TRUE;
		kMaterialData.fSpecularPower = 0.1f;
		kMaterialData.bSphereMapIndex = 1;

		m_GraphicThingInstance.SetMaterialData(0, nullptr, kMaterialData);
	}
	
#ifdef ENABLE_CANSEEHIDDENTHING_FOR_GM
	if (IsAffect(AFFECT_INVISIBILITY) && __MainCanSeeHiddenThing())
		m_GraphicThingInstance.BlendAlphaValue(0.5f, 0.5f);
#endif

	return true;
}


void CInstanceBase::__Create_SetName(const SCreateData& c_rkCreateData)
{
	if (IsGoto())
	{
		SetNameString("", 0);
		return;
	}
	if (IsWarp())
	{
		__Create_SetWarpName(c_rkCreateData);
		return;
	}
#if defined(WJ_SHOW_MOB_INFO) && defined(ENABLE_SHOW_MOBAIFLAG)
	if (IsEnemy() && CPythonSystem::Instance().IsShowMobAIFlag() && CPythonNonPlayer::Instance().IsAggressive(GetRace()))
	{
		std::string strName = c_rkCreateData.m_stName;
		strName += "*";
		SetNameString(strName.c_str(), strName.length());
	}
	else
		SetNameString(c_rkCreateData.m_stName.c_str(), c_rkCreateData.m_stName.length());
#else
	SetNameString(c_rkCreateData.m_stName.c_str(), c_rkCreateData.m_stName.length());
#endif
}

void CInstanceBase::__Create_SetWarpName(const SCreateData& c_rkCreateData)
{
	const char * c_szName;
	if (CPythonNonPlayer::Instance().GetName(c_rkCreateData.m_dwRace, &c_szName))
	{
		std::string strName = c_szName;
//		int32_t iFindingPos = strName.find_first_of(' ', 0);
//		if (iFindingPos > 0)
//			strName.resize(iFindingPos);
		SetNameString(strName.c_str(), strName.length());
	}
	else
	{
		SetNameString(c_rkCreateData.m_stName.c_str(), c_rkCreateData.m_stName.length());
	}
}

void CInstanceBase::SetNameString(const char* c_szName, int32_t len)
{
	m_stName.assign(c_szName, len);
}


bool CInstanceBase::SetRace(uint32_t eRace)
{
	m_dwRace = eRace;

	if (!m_GraphicThingInstance.SetRace(eRace))
		return false;

	if (!__FindRaceType(m_dwRace, &m_eRaceType))
		m_eRaceType=CActorInstance::TYPE_PC;

	return true;
}

BOOL CInstanceBase::__IsChangableWeapon(int32_t iWeaponID)
{	
	// 드레스 입고 있을때는 부케외의 장비는 나오지 않게..
	if (IsWearingDress())
	{
		const int32_t c_iBouquets[] =
		{
			50201,	// Bouquet for Assassin
			50202,	// Bouquet for Shaman
			50203,
			50204,
			0, // #0000545: [M2CN] 웨딩 드레스와 장비 착용 문제
		};

		for (int32_t i = 0; c_iBouquets[i] != 0; ++i)
			if (iWeaponID == c_iBouquets[i])
				return true;

		return false;
	}
	return true;
}

BOOL CInstanceBase::IsWearingDress()
{
	const int32_t c_iWeddingDressShape = 201;
	return c_iWeddingDressShape == m_eShape;
}

BOOL CInstanceBase::IsHoldingPickAxe()
{
	const int32_t c_iPickAxeStart = 29101;
	const int32_t c_iPickAxeEnd = 29110;
	return m_awPart[CRaceData::PART_WEAPON] >= c_iPickAxeStart && m_awPart[CRaceData::PART_WEAPON] <= c_iPickAxeEnd;
}

BOOL CInstanceBase::IsNewMount()
{
	return m_kHorse.IsNewMount();
}

BOOL CInstanceBase::IsMountingHorse()
{
	return m_kHorse.IsMounting();
}

void CInstanceBase::MountHorse(uint32_t eRace)
{
	m_kHorse.Destroy();
	m_kHorse.Create(m_GraphicThingInstance.NEW_GetCurPixelPositionRef(), eRace, ms_adwCRCAffectEffect[EFFECT_HIT]);

	SetMotionMode(CRaceMotionData::MODE_HORSE);	
	SetRotationSpeed(c_fDefaultHorseRotationSpeed);

	m_GraphicThingInstance.MountHorse(m_kHorse.GetActorPtr());
	m_GraphicThingInstance.Stop();
	m_GraphicThingInstance.RefreshActorInstance();
}

void CInstanceBase::DismountHorse()
{
	m_kHorse.Destroy();
}

void CInstanceBase::GetInfo(std::string* pstInfo)
{
	char szInfo[256];
	sprintf_s(szInfo, "Inst - UC %u, RC %u Pool - %u ",
		ms_dwUpdateCounter, 
		ms_dwRenderCounter,
		ms_kPool.GetCapacity()
	);

	pstInfo->append(szInfo);
}

void CInstanceBase::ResetPerformanceCounter()
{
	ms_dwUpdateCounter=0;
	ms_dwRenderCounter=0;
	ms_dwDeformCounter=0;
}

bool CInstanceBase::NEW_IsLastPixelPosition()
{
	return m_GraphicThingInstance.IsPushing();
}

const TPixelPosition& CInstanceBase::NEW_GetLastPixelPositionRef()
{
	return m_GraphicThingInstance.NEW_GetLastPixelPositionRef();
}

void CInstanceBase::NEW_SetDstPixelPositionZ(float z)
{
	m_GraphicThingInstance.NEW_SetDstPixelPositionZ(z);
}

void CInstanceBase::NEW_SetDstPixelPosition(const TPixelPosition& c_rkPPosDst)
{
	m_GraphicThingInstance.NEW_SetDstPixelPosition(c_rkPPosDst);
}

void CInstanceBase::NEW_SetSrcPixelPosition(const TPixelPosition& c_rkPPosSrc)
{
	m_GraphicThingInstance.NEW_SetSrcPixelPosition(c_rkPPosSrc);
}

const TPixelPosition& CInstanceBase::NEW_GetCurPixelPositionRef()
{
	return m_GraphicThingInstance.NEW_GetCurPixelPositionRef();	
}

const TPixelPosition& CInstanceBase::NEW_GetDstPixelPositionRef()
{
	return m_GraphicThingInstance.NEW_GetDstPixelPositionRef();
}

const TPixelPosition& CInstanceBase::NEW_GetSrcPixelPositionRef()
{
	return m_GraphicThingInstance.NEW_GetSrcPixelPositionRef();
}

/////////////////////////////////////////////////////////////////////////////////////////////////
void CInstanceBase::OnSyncing()
{
	m_GraphicThingInstance.__OnSyncing();
}

void CInstanceBase::OnWaiting()
{
	m_GraphicThingInstance.__OnWaiting();
}

void CInstanceBase::OnMoving()
{
	m_GraphicThingInstance.__OnMoving();
}

void CInstanceBase::ChangeGuild(uint32_t dwGuildID)
{
	m_dwGuildID=dwGuildID;

	DetachTextTail();
	AttachTextTail();
	RefreshTextTail();
}

uint32_t CInstanceBase::GetPart(CRaceData::EParts part)
{
	assert(part >= 0 && part < CRaceData::PART_MAX_NUM);
	return m_awPart[part];
}

uint32_t CInstanceBase::GetShape()
{
	return m_eShape;
}

bool CInstanceBase::CanAct()
{
	return m_GraphicThingInstance.CanAct();
}

bool CInstanceBase::CanMove()
{
	return m_GraphicThingInstance.CanMove();
}

bool CInstanceBase::CanUseSkill()
{
	if (IsPoly())
		return false;

	if (IsWearingDress())
		return false;

	if (IsHoldingPickAxe())
		return false;

	if (!m_kHorse.CanUseSkill())
		return false;

	if (!m_GraphicThingInstance.CanUseSkill())
		return false;

	return true;
}

bool CInstanceBase::CanAttack()
{
	if (!m_kHorse.CanAttack())
		return false;

	if (IsWearingDress())
		return false;

	if (IsHoldingPickAxe())
		return false;
	
	return m_GraphicThingInstance.CanAttack();
}



bool CInstanceBase::CanFishing()
{
	return m_GraphicThingInstance.CanFishing();
}


BOOL CInstanceBase::IsBowMode()
{
	return m_GraphicThingInstance.IsBowMode();
}

BOOL CInstanceBase::IsHandMode()
{
	return m_GraphicThingInstance.IsHandMode();
}

BOOL CInstanceBase::IsFishingMode()
{
	if (CRaceMotionData::MODE_FISHING == m_GraphicThingInstance.GetMotionMode())
		return true;

	return false;
}

BOOL CInstanceBase::IsFishing()
{
	return m_GraphicThingInstance.IsFishing();
}

BOOL CInstanceBase::IsDead()
{
	return m_GraphicThingInstance.IsDead();
}

BOOL CInstanceBase::IsStun()
{
	return m_GraphicThingInstance.IsStun();
}

BOOL CInstanceBase::IsSleep()
{
	return m_GraphicThingInstance.IsSleep();
}


BOOL CInstanceBase::__IsSyncing()
{
	return m_GraphicThingInstance.__IsSyncing();
}



void CInstanceBase::NEW_SetOwner(uint32_t dwVIDOwner)
{
	m_GraphicThingInstance.SetOwner(dwVIDOwner);
}

float CInstanceBase::GetLocalTime()
{
	return m_GraphicThingInstance.GetLocalTime();
}

uint32_t ELTimer_GetServerFrameMSec();

void CInstanceBase::PushTCPStateExpanded(uint32_t dwCmdTime, const TPixelPosition& c_rkPPosDst, float fDstRot, uint32_t eFunc, uint32_t uArg, uint32_t uTargetVID)
{
	SCommand kCmdNew;
	kCmdNew.m_kPPosDst = c_rkPPosDst;
	kCmdNew.m_dwChkTime = dwCmdTime+100;
	kCmdNew.m_dwCmdTime = dwCmdTime;
	kCmdNew.m_fDstRot = fDstRot;
	kCmdNew.m_eFunc = eFunc;
	kCmdNew.m_uArg = uArg;
	kCmdNew.m_uTargetVID = uTargetVID;
	m_kQue_kCmdNew.emplace_back(kCmdNew);
}

void CInstanceBase::PushTCPState(uint32_t dwCmdTime, const TPixelPosition& c_rkPPosDst, float fDstRot, uint32_t eFunc, uint32_t uArg)
{	
	if (__IsMainInstance())
	{
		//assert(!"CInstanceBase::PushTCPState 플레이어 자신에게 이동패킷은 오면 안된다!");
		TraceError("CInstanceBase::PushTCPState You can't send move packets to yourself!");
		return;
	}

	int32_t nNetworkGap=ELTimer_GetServerFrameMSec()-dwCmdTime;
	
	m_nAverageNetworkGap=(m_nAverageNetworkGap*70+nNetworkGap*30)/100;
	
	/*
	if (m_dwBaseCmdTime == 0)
	{
		m_dwBaseChkTime = ELTimer_GetFrameMSec()-nNetworkGap;
		m_dwBaseCmdTime = dwCmdTime;

		Tracenf("VID[%d] 네트웍갭 [%d]", GetVirtualID(), nNetworkGap);
	}
	*/

	//m_dwBaseChkTime-m_dwBaseCmdTime+ELTimer_GetServerMSec();

	SCommand kCmdNew;
	kCmdNew.m_kPPosDst = c_rkPPosDst;
	kCmdNew.m_dwChkTime = dwCmdTime+m_nAverageNetworkGap;//m_dwBaseChkTime + (dwCmdTime - m_dwBaseCmdTime);// + nNetworkGap;
	kCmdNew.m_dwCmdTime = dwCmdTime;
	kCmdNew.m_fDstRot = fDstRot;
	kCmdNew.m_eFunc = eFunc;
	kCmdNew.m_uArg = uArg;
	m_kQue_kCmdNew.emplace_back(kCmdNew);

	//int32_t nApplyGap=kCmdNew.m_dwChkTime-ELTimer_GetServerFrameMSec();

	//if (nApplyGap<-500 || nApplyGap>500)
	//	Tracenf("VID[%d] NAME[%s] 네트웍갭 [cur:%d ave:%d] 작동시간 (%d)", GetVirtualID(), GetNameString(), nNetworkGap, m_nAverageNetworkGap, nApplyGap);
}

/*
CInstanceBase::TStateQueue::iterator CInstanceBase::FindSameState(TStateQueue& rkQuekStt, uint32_t dwCmdTime, uint32_t eFunc, uint32_t uArg)
{
	TStateQueue::iterator i=rkQuekStt.begin();
	while (rkQuekStt.end()!=i)
	{
		SState& rkSttEach=*i;
		if (rkSttEach.m_dwCmdTime==dwCmdTime)
			if (rkSttEach.m_eFunc==eFunc)
				if (rkSttEach.m_uArg==uArg)
					break;
		++i;
	}

	return i;
}
*/

BOOL CInstanceBase::__CanProcessNetworkStatePacket()
{
	if (m_GraphicThingInstance.IsDead())
		return FALSE;
	if (m_GraphicThingInstance.IsKnockDown())
		return FALSE;
	if (m_GraphicThingInstance.IsUsingSkill())
		if (!m_GraphicThingInstance.CanCancelSkill())
			return FALSE;

	return TRUE;
}

BOOL CInstanceBase::__IsEnableTCPProcess(uint32_t eCurFunc)
{
	if (m_GraphicThingInstance.IsActEmotion())
		return FALSE;

	if (!m_bEnableTCPState)
	{
		if (FUNC_EMOTION != eCurFunc)
			return FALSE;
	}

	return TRUE;
}

void CInstanceBase::StateProcess()
{	
	while (true)
	{
		if (m_kQue_kCmdNew.empty())
			return;	

		uint32_t dwDstChkTime = m_kQue_kCmdNew.front().m_dwChkTime;
		uint32_t dwCurChkTime = ELTimer_GetServerFrameMSec();	

		if (dwCurChkTime < dwDstChkTime)
			return;

		SCommand kCmdTop = m_kQue_kCmdNew.front();
		m_kQue_kCmdNew.pop_front();	

		TPixelPosition kPPosDst = kCmdTop.m_kPPosDst;
		//uint32_t dwCmdTime = kCmdTop.m_dwCmdTime;	
		float fRotDst = kCmdTop.m_fDstRot;
		uint32_t eFunc = kCmdTop.m_eFunc;
		uint32_t uArg = kCmdTop.m_uArg;
		uint32_t uVID = GetVirtualID();	
		uint32_t uTargetVID = kCmdTop.m_uTargetVID;

		TPixelPosition kPPosCur;
		NEW_GetPixelPosition(&kPPosCur);

		/*
		if (IsPC())
			Tracenf("%d cmd: vid=%d[%s] func=%d arg=%d  curPos=(%f, %f) dstPos=(%f, %f) rot=%f (time %d)", 
			ELTimer_GetMSec(),
			uVID, m_stName.c_str(), eFunc, uArg, 
			kPPosCur.x, kPPosCur.y,
			kPPosDst.x, kPPosDst.y, fRotDst, dwCmdTime-m_dwBaseCmdTime);
		*/

		TPixelPosition kPPosDir = kPPosDst - kPPosCur;
		float fDirLen = (float)sqrt(kPPosDir.x * kPPosDir.x + kPPosDir.y * kPPosDir.y);

		//Tracenf("거리 %f", fDirLen);

		if (!__CanProcessNetworkStatePacket())
		{
			Lognf(0, "vid=%d Skip State as unable to process IsDead=%d, IsKnockDown=%d", uVID, m_GraphicThingInstance.IsDead(), m_GraphicThingInstance.IsKnockDown());
			return;
		}

		if (!__IsEnableTCPProcess(eFunc))
			return;

		switch (eFunc)
		{
			case FUNC_WAIT:
			{
				//Tracenf("%s (%f, %f) -> (%f, %f) 남은거리 %f", GetNameString(), kPPosCur.x, kPPosCur.y, kPPosDst.x, kPPosDst.y, fDirLen);
				if (fDirLen > 1.0f)
				{
					//NEW_GetSrcPixelPositionRef() = kPPosCur;
					//NEW_GetDstPixelPositionRef() = kPPosDst;
					NEW_SetSrcPixelPosition(kPPosCur);
					NEW_SetDstPixelPosition(kPPosDst);

					__EnableSkipCollision();

					m_fDstRot = fRotDst;
					m_isGoing = TRUE;

					m_kMovAfterFunc.eFunc = FUNC_WAIT;

					if (!IsWalking())
						StartWalking();

					//Tracen("목표정지");
				}
				else
				{
					//Tracen("현재 정지");

					m_isGoing = FALSE;

					if (!IsWaiting())
						EndWalking();

					SCRIPT_SetPixelPosition(kPPosDst.x, kPPosDst.y);
					SetAdvancingRotation(fRotDst);
					SetRotation(fRotDst);
				}
				break;
			}

			case FUNC_MOVE:
			{
				if (fDirLen > 0.0f)
				{
					NEW_SetSrcPixelPosition(kPPosCur);
					NEW_SetDstPixelPosition(kPPosDst);
					m_fDstRot = fRotDst;
					m_isGoing = TRUE;
					__EnableSkipCollision();
					//m_isSyncMov = TRUE;

					m_kMovAfterFunc.eFunc = FUNC_MOVE;

					if (!IsWalking())
					{
						//Tracen("걷고 있지 않아 걷기 시작");
						StartWalking();
					}
					else
					{
						//Tracen("이미 걷는중 ");
					}
				}
				else
				{
					//If there was no advancement, update only rotation
					m_fDstRot = fRotDst;

					m_isGoing = FALSE;
					if (!IsWaiting())
						EndWalking();
				}
				break;
			}

			case FUNC_COMBO:
			{
				if (fDirLen >= 50.0f)
				{
					NEW_SetSrcPixelPosition(kPPosCur);
					NEW_SetDstPixelPosition(kPPosDst);
					m_fDstRot=fRotDst;
					m_isGoing = TRUE;
					__EnableSkipCollision();

					m_kMovAfterFunc.eFunc = FUNC_COMBO;
					m_kMovAfterFunc.uArg = uArg;

					if (!IsWalking())
						StartWalking();
				}
				else
				{
					//Tracen("대기 공격 정지");

					m_isGoing = FALSE;

					if (IsWalking())
						EndWalking();

					SCRIPT_SetPixelPosition(kPPosDst.x, kPPosDst.y);
					RunComboAttack(fRotDst, uArg);
				}
				break;
			}

			case FUNC_ATTACK:
			{
				if (fDirLen>=50.0f)
				{
					//NEW_GetSrcPixelPositionRef() = kPPosCur;
					//NEW_GetDstPixelPositionRef() = kPPosDst;
					NEW_SetSrcPixelPosition(kPPosCur);
					NEW_SetDstPixelPosition(kPPosDst);
					m_fDstRot = fRotDst;
					m_isGoing = TRUE;
					__EnableSkipCollision();
					//m_isSyncMov = TRUE;

					m_kMovAfterFunc.eFunc = FUNC_ATTACK;

					if (!IsWalking())
						StartWalking();

					//Tracen("너무 멀어서 이동 후 공격");
				}
				else
				{
					//Tracen("노말 공격 정지");

					m_isGoing = FALSE;

					if (IsWalking())
						EndWalking();

					SCRIPT_SetPixelPosition(kPPosDst.x, kPPosDst.y);
					BlendRotation(fRotDst);

					RunNormalAttack(fRotDst);

					//Tracen("가깝기 때문에 워프 공격");
				}
				break;
			}

			case FUNC_MOB_SKILL:
			{
				if (fDirLen >= 50.0f)
				{
					NEW_SetSrcPixelPosition(kPPosCur);
					NEW_SetDstPixelPosition(kPPosDst);
					m_fDstRot = fRotDst;
					m_isGoing = TRUE;
					__EnableSkipCollision();

					m_kMovAfterFunc.eFunc = FUNC_MOB_SKILL;
					m_kMovAfterFunc.uArg = uArg;

					if (!IsWalking())
						StartWalking();
				}
				else
				{
					m_isGoing = FALSE;

					if (IsWalking())
						EndWalking();

					SCRIPT_SetPixelPosition(kPPosDst.x, kPPosDst.y);
					BlendRotation(fRotDst);

					m_GraphicThingInstance.InterceptOnceMotion(CRaceMotionData::NAME_SPECIAL_1 + uArg);
				}
				break;
			}

			case FUNC_EMOTION:
			{
				if (fDirLen>100.0f)
				{
					NEW_SetSrcPixelPosition(kPPosCur);
					NEW_SetDstPixelPosition(kPPosDst);
					m_fDstRot = fRotDst;
					m_isGoing = TRUE;

					if (__IsMainInstance())
						__EnableSkipCollision();

					m_kMovAfterFunc.eFunc = FUNC_EMOTION;
					m_kMovAfterFunc.uArg = uArg;
					m_kMovAfterFunc.uArgExpanded = uTargetVID;
					m_kMovAfterFunc.kPosDst = kPPosDst;

					if (!IsWalking())
						StartWalking();
				}
				else
				{
					__ProcessFunctionEmotion(uArg, uTargetVID, kPPosDst);
				}
				break;
			}

			default:
			{
				if (eFunc & FUNC_SKILL)
				{
					if (fDirLen >= 50.0f)
					{
						//NEW_GetSrcPixelPositionRef() = kPPosCur;
						//NEW_GetDstPixelPositionRef() = kPPosDst;
						NEW_SetSrcPixelPosition(kPPosCur);
						NEW_SetDstPixelPosition(kPPosDst);
						m_fDstRot = fRotDst;
						m_isGoing = TRUE;
						//m_isSyncMov = TRUE;
						__EnableSkipCollision();

						m_kMovAfterFunc.eFunc = eFunc;
						m_kMovAfterFunc.uArg = uArg;

						if (!IsWalking())
							StartWalking();

						//Tracen("너무 멀어서 이동 후 공격");
					}
					else
					{
						//Tracen("스킬 정지");

						m_isGoing = FALSE;

						if (IsWalking())
							EndWalking();

						SCRIPT_SetPixelPosition(kPPosDst.x, kPPosDst.y);
						SetAdvancingRotation(fRotDst);
						SetRotation(fRotDst);

						NEW_UseSkill(0, eFunc & 0x7f, uArg&0x0f, (uArg>>4) ? true : false);
						//Tracen("가깝기 때문에 워프 공격");
					}
				}
				break;
			}
		}
	}
}


void CInstanceBase::MovementProcess()
{
	TPixelPosition kPPosCur;
	NEW_GetPixelPosition(&kPPosCur);

	// 렌더링 좌표계이므로 y를 -화해서 더한다.

	TPixelPosition kPPosNext;
	{
		const D3DXVECTOR3 & c_rkV3Mov = m_GraphicThingInstance.GetMovementVectorRef();

		kPPosNext.x = kPPosCur.x + (+c_rkV3Mov.x);
		kPPosNext.y = kPPosCur.y + (-c_rkV3Mov.y);
		kPPosNext.z = kPPosCur.z + (+c_rkV3Mov.z);
	}

	TPixelPosition kPPosDeltaSC = kPPosCur - NEW_GetSrcPixelPositionRef();
	TPixelPosition kPPosDeltaSN = kPPosNext - NEW_GetSrcPixelPositionRef();
	TPixelPosition kPPosDeltaSD = NEW_GetDstPixelPositionRef() - NEW_GetSrcPixelPositionRef();

	float fCurLen = sqrtf(kPPosDeltaSC.x * kPPosDeltaSC.x + kPPosDeltaSC.y * kPPosDeltaSC.y);
	float fNextLen = sqrtf(kPPosDeltaSN.x * kPPosDeltaSN.x + kPPosDeltaSN.y * kPPosDeltaSN.y);
	float fTotalLen = sqrtf(kPPosDeltaSD.x * kPPosDeltaSD.x + kPPosDeltaSD.y * kPPosDeltaSD.y);
	float fRestLen = fTotalLen - fCurLen;

	if (__IsMainInstance())
	{
		if (m_isGoing && IsWalking())
		{
			float fDstRot = NEW_GetAdvancingRotationFromPixelPosition(kPPosCur, NEW_GetDstPixelPositionRef());
			SetAdvancingRotation(fDstRot);

			if (fRestLen<=0.0)
			{
				if (IsWalking())
					EndWalking();

				//Tracen("목표 도달 정지");

				m_isGoing = FALSE;

				BlockMovement();

				if (FUNC_EMOTION == m_kMovAfterFunc.eFunc)
				{
					uint32_t dwMotionNumber = m_kMovAfterFunc.uArg;
					uint32_t dwTargetVID = m_kMovAfterFunc.uArgExpanded;
					__ProcessFunctionEmotion(dwMotionNumber, dwTargetVID, m_kMovAfterFunc.kPosDst);
					m_kMovAfterFunc.eFunc = FUNC_WAIT;
					return;
				}
			}
		}
	}
	else
	{
		if (m_isGoing && IsWalking())
		{
			float fDstRot = NEW_GetAdvancingRotationFromPixelPosition(NEW_GetSrcPixelPositionRef(), NEW_GetDstPixelPositionRef());

			SetAdvancingRotation(fDstRot);

			// 만약 렌턴시가 늦어 너무 많이 이동했다면..
			if (fRestLen < -100.0f)
			{
				NEW_SetSrcPixelPosition(kPPosCur);

				float fDstRot = NEW_GetAdvancingRotationFromPixelPosition(kPPosCur, NEW_GetDstPixelPositionRef());
				SetAdvancingRotation(fDstRot);
				//Tracenf("VID %d 오버 방향설정 (%f, %f) %f rest %f", GetVirtualID(), kPPosCur.x, kPPosCur.y, fDstRot, fRestLen);			

				// 이동중이라면 다음번에 멈추게 한다
				if (FUNC_MOVE == m_kMovAfterFunc.eFunc)
					m_kMovAfterFunc.eFunc = FUNC_WAIT;
			}
			// 도착했다면...
			else if (fCurLen <= fTotalLen && fTotalLen <= fNextLen)
			{
				if (m_GraphicThingInstance.IsDead() || m_GraphicThingInstance.IsKnockDown())
				{
					__DisableSkipCollision();

					//Tracen("사망 상태라 동작 스킵");

					m_isGoing = FALSE;

					//Tracen("행동 불능 상태라 이후 동작 스킵");
				}
				else
				{
					switch (m_kMovAfterFunc.eFunc)
					{
						case FUNC_ATTACK:
						{
							if (IsWalking())
								EndWalking();

							__DisableSkipCollision();
							m_isGoing = FALSE;

							BlockMovement();
							SCRIPT_SetPixelPosition(NEW_GetDstPixelPositionRef().x, NEW_GetDstPixelPositionRef().y);
							SetAdvancingRotation(m_fDstRot);
							SetRotation(m_fDstRot);

							RunNormalAttack(m_fDstRot);
							break;
						}

						case FUNC_COMBO:
						{
							if (IsWalking())
								EndWalking();

							__DisableSkipCollision();
							m_isGoing = FALSE;

							BlockMovement();
							SCRIPT_SetPixelPosition(NEW_GetDstPixelPositionRef().x, NEW_GetDstPixelPositionRef().y);
							RunComboAttack(m_fDstRot, m_kMovAfterFunc.uArg);
							break;
						}

						case FUNC_EMOTION:
						{
							m_isGoing = FALSE;
							m_kMovAfterFunc.eFunc = FUNC_WAIT;
							__DisableSkipCollision();
							BlockMovement();

							uint32_t dwMotionNumber = m_kMovAfterFunc.uArg;
							uint32_t dwTargetVID = m_kMovAfterFunc.uArgExpanded;
							__ProcessFunctionEmotion(dwMotionNumber, dwTargetVID, m_kMovAfterFunc.kPosDst);
							break;
						}

						case FUNC_MOVE:
						{
							break;
						}

						case FUNC_MOB_SKILL:
						{
							if (IsWalking())
								EndWalking();

							__DisableSkipCollision();
							m_isGoing = FALSE;

							BlockMovement();
							SCRIPT_SetPixelPosition(NEW_GetDstPixelPositionRef().x, NEW_GetDstPixelPositionRef().y);
							SetAdvancingRotation(m_fDstRot);
							SetRotation(m_fDstRot);

							m_GraphicThingInstance.InterceptOnceMotion(CRaceMotionData::NAME_SPECIAL_1 + m_kMovAfterFunc.uArg);
							break;
						}

						default:
						{
							if (m_kMovAfterFunc.eFunc & FUNC_SKILL)
							{
								SetAdvancingRotation(m_fDstRot);
								BlendRotation(m_fDstRot);
								NEW_UseSkill(0, m_kMovAfterFunc.eFunc & 0x7f, m_kMovAfterFunc.uArg&0x0f, (m_kMovAfterFunc.uArg>>4) ? true : false);
							}
							else
							{
								//Tracenf("VID %d 스킬 공격 (%f, %f) rot %f", GetVirtualID(), NEW_GetDstPixelPositionRef().x, NEW_GetDstPixelPositionRef().y, m_fDstRot);

								__DisableSkipCollision();
								m_isGoing = FALSE;

								BlockMovement();
								SCRIPT_SetPixelPosition(NEW_GetDstPixelPositionRef().x, NEW_GetDstPixelPositionRef().y);
								SetAdvancingRotation(m_fDstRot);
								BlendRotation(m_fDstRot);
								if (!IsWaiting())
									EndWalking();

								//Tracenf("VID %d 정지 (%f, %f) rot %f IsWalking %d", GetVirtualID(), NEW_GetDstPixelPositionRef().x, NEW_GetDstPixelPositionRef().y, m_fDstRot, IsWalking());
							}
							break;
						}
					}

				}
			}

		}
	}

	if (IsWalking() || m_GraphicThingInstance.IsUsingMovingSkill())
	{
		float fRotation = m_GraphicThingInstance.GetRotation();
		float fAdvancingRotation = m_GraphicThingInstance.GetAdvancingRotation();
		int32_t iDirection = GetRotatingDirection(fRotation, fAdvancingRotation);

		if (DEGREE_DIRECTION_SAME != m_iRotatingDirection)
		{
			if (DEGREE_DIRECTION_LEFT == iDirection)
				fRotation = fmodf(fRotation + m_fRotSpd * m_GraphicThingInstance.GetSecondElapsed(), 360.0f);
			else if (DEGREE_DIRECTION_RIGHT == iDirection)
				fRotation = fmodf(fRotation - m_fRotSpd * m_GraphicThingInstance.GetSecondElapsed() + 360.0f, 360.0f);

			if (m_iRotatingDirection != GetRotatingDirection(fRotation, fAdvancingRotation))
			{
				m_iRotatingDirection = DEGREE_DIRECTION_SAME;
				fRotation = fAdvancingRotation;
			}

			m_GraphicThingInstance.SetRotation(fRotation);
		}

		if (__IsInDustRange())
		{ 
			float fDustDistance = NEW_GetDistanceFromDestPixelPosition(m_kPPosDust);
			if (IsMountingHorse())
			{
				if (fDustDistance > ms_fHorseDustGap)
				{
					NEW_GetPixelPosition(&m_kPPosDust);
					__AttachEffect(EFFECT_HORSE_DUST);
				}
			}
			else
			{
				if (fDustDistance > ms_fDustGap)
				{
					NEW_GetPixelPosition(&m_kPPosDust);
					__AttachEffect(EFFECT_DUST);
				}
			}
		}
	}
}

void CInstanceBase::__ProcessFunctionEmotion(uint32_t dwMotionNumber, uint32_t dwTargetVID, const TPixelPosition & c_rkPosDst)
{
	if (IsWalking())
		EndWalkingWithoutBlending();

	__EnableChangingTCPState();
	SCRIPT_SetPixelPosition(c_rkPosDst.x, c_rkPosDst.y);

	CInstanceBase * pTargetInstance = CPythonCharacterManager::Instance().GetInstancePtr(dwTargetVID);
	if (pTargetInstance)
	{
		pTargetInstance->__EnableChangingTCPState();

		if (pTargetInstance->IsWalking())
			pTargetInstance->EndWalkingWithoutBlending();

		uint16_t wMotionNumber1 = HIWORD(dwMotionNumber);
		uint16_t wMotionNumber2 = LOWORD(dwMotionNumber);

		int32_t src_job = RaceToJob(GetRace());
		int32_t dst_job = RaceToJob(pTargetInstance->GetRace());

		NEW_LookAtDestInstance(*pTargetInstance);
		m_GraphicThingInstance.InterceptOnceMotion(wMotionNumber1 + dst_job);
		m_GraphicThingInstance.SetRotation(m_GraphicThingInstance.GetTargetRotation());
		m_GraphicThingInstance.SetAdvancingRotation(m_GraphicThingInstance.GetTargetRotation());

		pTargetInstance->NEW_LookAtDestInstance(*this);
		pTargetInstance->m_GraphicThingInstance.InterceptOnceMotion(wMotionNumber2 + src_job);
		pTargetInstance->m_GraphicThingInstance.SetRotation(pTargetInstance->m_GraphicThingInstance.GetTargetRotation());
		pTargetInstance->m_GraphicThingInstance.SetAdvancingRotation(pTargetInstance->m_GraphicThingInstance.GetTargetRotation());

		if (pTargetInstance->__IsMainInstance())
		{
			IAbstractPlayer & rPlayer=IAbstractPlayer::GetSingleton();
			rPlayer.EndEmotionProcess();
		}
	}

	if (__IsMainInstance())
	{
		IAbstractPlayer & rPlayer=IAbstractPlayer::GetSingleton();
		rPlayer.EndEmotionProcess();
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// Update & Deform & Render

int32_t g_iAccumulationTime = 0;

void CInstanceBase::Update()
{
	++ms_dwUpdateCounter;	

	StateProcess();
	m_GraphicThingInstance.PhysicsProcess();
	m_GraphicThingInstance.RotationProcess();
	m_GraphicThingInstance.ComboProcess();
	m_GraphicThingInstance.AccumulationMovement();

	if (m_GraphicThingInstance.IsMovement())
	{
		TPixelPosition kPPosCur;
		NEW_GetPixelPosition(&kPPosCur);

		uint32_t dwCurTime=ELTimer_GetFrameMSec();
		//if (m_dwNextUpdateHeightTime<dwCurTime)
		{
			m_dwNextUpdateHeightTime=dwCurTime;
			kPPosCur.z = __GetBackgroundHeight(kPPosCur.x, kPPosCur.y);
			NEW_SetPixelPosition(kPPosCur);
		}

		// SetMaterialColor
		{
			uint32_t dwMtrlColor=__GetShadowMapColor(kPPosCur.x, kPPosCur.y);
			m_GraphicThingInstance.SetMaterialColor(dwMtrlColor);
		}
	}

	m_GraphicThingInstance.UpdateAdvancingPointInstance();

	AttackProcess();
	MovementProcess();

	m_GraphicThingInstance.MotionProcess(IsPC());
	if (IsMountingHorse())
		m_kHorse.m_pkActor->HORSE_MotionProcess(FALSE);

	__ComboProcess();	
	
	ProcessDamage();

}

void CInstanceBase::Transform()
{
	if (__IsSyncing())
	{
		//OnSyncing();
	}
	else
	{
		if (IsWalking() || m_GraphicThingInstance.IsUsingMovingSkill())
		{
			const D3DXVECTOR3& c_rv3Movment=m_GraphicThingInstance.GetMovementVectorRef();

			float len=(c_rv3Movment.x*c_rv3Movment.x)+(c_rv3Movment.y*c_rv3Movment.y);
			if (len>1.0f)
				OnMoving();
			else
				OnWaiting();	
		}	
	}

	m_GraphicThingInstance.INSTANCEBASE_Transform();
}


void CInstanceBase::Deform()
{
	// 2004.07.17.levites.isShow를 ViewFrustumCheck로 변경
	if (!__CanRender())
		return;

	++ms_dwDeformCounter;

	m_GraphicThingInstance.INSTANCEBASE_Deform();

	m_kHorse.Deform();
}

void CInstanceBase::RenderTrace()
{
	if (!__CanRender())
		return;

	m_GraphicThingInstance.RenderTrace();
}




void CInstanceBase::Render()
{
	// 2004.07.17.levites.isShow를 ViewFrustumCheck로 변경
	if (!__CanRender())
		return;

	++ms_dwRenderCounter;

	m_kHorse.Render();
	m_GraphicThingInstance.Render();	
	
	if (CActorInstance::IsDirLine())
	{	
		if (NEW_GetDstPixelPositionRef().x != 0.0f)
		{
			static CScreen s_kScreen;

			STATEMANAGER.SetTextureStageState(0, D3DTSS_COLORARG1,	D3DTA_DIFFUSE);
			STATEMANAGER.SetTextureStageState(0, D3DTSS_COLOROP,	D3DTOP_SELECTARG1);
			STATEMANAGER.SetTextureStageState(0, D3DTSS_ALPHAOP,	D3DTOP_DISABLE);	
			STATEMANAGER.SaveRenderState(D3DRS_ZENABLE, FALSE);
			STATEMANAGER.SetRenderState(D3DRS_FOGENABLE, FALSE);
			STATEMANAGER.SetRenderState(D3DRS_LIGHTING, FALSE);
			
			TPixelPosition px;
			m_GraphicThingInstance.GetPixelPosition(&px);
			D3DXVECTOR3 kD3DVt3Cur(px.x, px.y, px.z);
			//D3DXVECTOR3 kD3DVt3Cur(NEW_GetSrcPixelPositionRef().x, -NEW_GetSrcPixelPositionRef().y, NEW_GetSrcPixelPositionRef().z);
			D3DXVECTOR3 kD3DVt3Dest(NEW_GetDstPixelPositionRef().x, -NEW_GetDstPixelPositionRef().y, NEW_GetDstPixelPositionRef().z);

			//printf("%s %f\n", GetNameString(), kD3DVt3Cur.y - kD3DVt3Dest.y);
			//float fdx = NEW_GetDstPixelPositionRef().x - NEW_GetSrcPixelPositionRef().x;
			//float fdy = NEW_GetDstPixelPositionRef().y - NEW_GetSrcPixelPositionRef().y;

			s_kScreen.SetDiffuseColor(0.0f, 0.0f, 1.0f);
			s_kScreen.RenderLine3d(kD3DVt3Cur.x, kD3DVt3Cur.y, px.z, kD3DVt3Dest.x, kD3DVt3Dest.y, px.z);
			STATEMANAGER.RestoreRenderState(D3DRS_ZENABLE);
			STATEMANAGER.SetRenderState(D3DRS_FOGENABLE, TRUE);
			STATEMANAGER.SetRenderState(D3DRS_LIGHTING, TRUE);
		}
	}	
}

void CInstanceBase::RenderToShadowMap()
{
	if (IsDoor())
		return;

	if (IsBuilding())
		return;

	if (!__CanRender())
		return;

	if (!__IsExistMainInstance())
		return;

	CInstanceBase* pkInstMain=__GetMainInstancePtr();

	const float SHADOW_APPLY_DISTANCE = 2500.0f;

	float fDistance=NEW_GetDistanceFromDestInstance(*pkInstMain);
	if (fDistance>=SHADOW_APPLY_DISTANCE)
		return;

	m_GraphicThingInstance.RenderToShadowMap();	
}

void CInstanceBase::RenderCollision()
{
	m_GraphicThingInstance.RenderCollisionData();
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// Setting & Getting Data

void CInstanceBase::SetVirtualID(uint32_t dwVirtualID)
{
	m_GraphicThingInstance.SetVirtualID(dwVirtualID);		
}

void CInstanceBase::SetVirtualNumber(uint32_t dwVirtualNumber)
{
	m_dwVirtualNumber = dwVirtualNumber;
}

void CInstanceBase::SetInstanceType(int32_t iInstanceType)
{
	m_GraphicThingInstance.SetActorType(iInstanceType);
}

void CInstanceBase::SetAlignment(int16_t sAlignment)
{
	m_sAlignment = sAlignment;
	RefreshTextTailTitle();
}

void CInstanceBase::SetPKMode(uint8_t byPKMode)
{
	if (m_byPKMode == byPKMode)
		return;

	m_byPKMode = byPKMode;

	if (__IsMainInstance())
	{
		IAbstractPlayer& rPlayer=IAbstractPlayer::GetSingleton();
		rPlayer.NotifyChangePKMode();
	}	
}

void CInstanceBase::SetKiller(bool bFlag)
{
	if (m_isKiller == bFlag)
		return;

	m_isKiller = bFlag;
	RefreshTextTail();
}

void CInstanceBase::SetPartyMemberFlag(bool bFlag)
{
	m_isPartyMember = bFlag;
}

void CInstanceBase::SetStateFlags(uint32_t dwStateFlags)
{
	if (dwStateFlags & ADD_CHARACTER_STATE_KILLER)
		SetKiller(TRUE);
	else
		SetKiller(FALSE);

	if (dwStateFlags & ADD_CHARACTER_STATE_PARTY)
		SetPartyMemberFlag(TRUE);
	else
		SetPartyMemberFlag(FALSE);
}

void CInstanceBase::SetComboType(uint32_t uComboType)
{
	m_GraphicThingInstance.SetComboType(uComboType);
}

const char * CInstanceBase::GetNameString()
{
	return m_stName.c_str();
}

#ifdef ENABLE_LEVEL_IN_TRADE
uint32_t CInstanceBase::GetLevel()
{
	return m_dwLevel;
}
#endif

#ifdef ENABLE_TEXT_LEVEL_REFRESH
void CInstanceBase::SetLevel(uint32_t dwLevel)
{
	m_dwLevel = dwLevel;
}
#endif

#if defined(WJ_SHOW_MOB_INFO) && defined(ENABLE_SHOW_MOBAIFLAG)
void CInstanceBase::MobInfoAiFlagRefresh()
{
	// set
	std::string strName = CPythonNonPlayer::Instance().GetMonsterName(GetRace());
	if (CPythonSystem::Instance().IsShowMobAIFlag() && CPythonNonPlayer::Instance().IsAggressive(GetRace()))
		strName += "*";
	SetNameString(strName.c_str(), strName.length());
	// refresh
	DetachTextTail();
	AttachTextTail();
	RefreshTextTail();
}
#endif
#if defined(WJ_SHOW_MOB_INFO) && defined(ENABLE_SHOW_MOBLEVEL)
void CInstanceBase::MobInfoLevelRefresh()
{
	// set
	if (CPythonSystem::Instance().IsShowMobLevel())
		m_dwLevel = CPythonNonPlayer::Instance().GetMonsterLevel(GetRace());
	else
		m_dwLevel = 0;
	// refresh
	if (m_dwLevel)
		UpdateTextTailLevel(m_dwLevel);
	else
		CPythonTextTail::Instance().DetachLevel(GetVirtualID());
}
#endif

uint32_t CInstanceBase::GetRace()
{
	return m_dwRace;
}


bool CInstanceBase::IsConflictAlignmentInstance(CInstanceBase& rkInstVictim)
{
	if (PK_MODE_PROTECT == rkInstVictim.GetPKMode())
		return false;

	switch (GetAlignmentType())
	{
		case ALIGNMENT_TYPE_NORMAL:
		case ALIGNMENT_TYPE_WHITE:
			if (ALIGNMENT_TYPE_DARK == rkInstVictim.GetAlignmentType())
				return true;
			break;
		case ALIGNMENT_TYPE_DARK:
			if (GetAlignmentType() != rkInstVictim.GetAlignmentType())
				return true;
			break;
	}

	return false;
}

void CInstanceBase::SetDuelMode(uint32_t type)
{
	m_dwDuelMode = type;
}

uint32_t CInstanceBase::GetDuelMode()
{
	return m_dwDuelMode;
}

bool CInstanceBase::IsAttackableInstance(CInstanceBase& rkInstVictim)
{	
	if (__IsMainInstance())
	{		
		CPythonPlayer& rkPlayer=CPythonPlayer::Instance();
		if(rkPlayer.IsObserverMode())
			return false;
	}

	if (GetVirtualID() == rkInstVictim.GetVirtualID())
		return false;

	if (IsStone())
	{
		if (rkInstVictim.IsPC())
			return true;
	}
	else if (IsPC())
	{
		if (rkInstVictim.IsStone())
			return true;

		if (rkInstVictim.IsPC())
		{
			// The other person is invincible.
			if (rkInstVictim.IsAffect(AFFECT_REVIVE_INVISIBILITY))
				return false;

			// In Primal Map players can only attack each other if the event started
			if (CPythonBackground::Instance().IsPrimalMap())
				return CPythonBackground::Instance().IsPrimalMapAttackable();

			if (GetDuelMode())
			{
				switch(GetDuelMode())
				{
				case DUEL_CANNOTATTACK:
					return false;
				case DUEL_START:
					if(__FindDUELKey(GetVirtualID(),rkInstVictim.GetVirtualID()))
						return true;
					return false;
				}
			}
			if (PK_MODE_GUILD == GetPKMode())
				if (GetGuildID() == rkInstVictim.GetGuildID())
					return false;

			if (rkInstVictim.IsKiller())
				if (!IAbstractPlayer::GetSingleton().IsSamePartyMember(GetVirtualID(), rkInstVictim.GetVirtualID()))
					return true;

			if (PK_MODE_PROTECT != GetPKMode())
			{
				if (PK_MODE_FREE == GetPKMode())
				{
					if (PK_MODE_PROTECT != rkInstVictim.GetPKMode())
						if (!IAbstractPlayer::GetSingleton().IsSamePartyMember(GetVirtualID(), rkInstVictim.GetVirtualID()))
							return true;
				}
				if (PK_MODE_GUILD == GetPKMode())
				{
					if (PK_MODE_PROTECT != rkInstVictim.GetPKMode())
						if (!IAbstractPlayer::GetSingleton().IsSamePartyMember(GetVirtualID(), rkInstVictim.GetVirtualID()))
							if (GetGuildID() != rkInstVictim.GetGuildID())
								return true;
				}
			}

			if (IsSameEmpire(rkInstVictim))
			{
				if (IsPVPInstance(rkInstVictim))
					return true;

				if (PK_MODE_REVENGE == GetPKMode())
					if (!IAbstractPlayer::GetSingleton().IsSamePartyMember(GetVirtualID(), rkInstVictim.GetVirtualID()))
						if (IsConflictAlignmentInstance(rkInstVictim))
							return true;
			}
			else
			{
				return PK_MODE_PROTECT != GetPKMode() && PK_MODE_PROTECT != rkInstVictim.GetPKMode();
			}
		}

		if (rkInstVictim.IsEnemy())
			return true;

		if (rkInstVictim.IsWoodenDoor())
			return true;
	}
	else if (IsEnemy())
	{
		if (rkInstVictim.IsPC())
			return true;

		if (rkInstVictim.IsBuilding())
			return true;
		
	}
	else if (IsPoly())
	{
		if (rkInstVictim.IsPC())
			return true;

		if (rkInstVictim.IsEnemy())
			return true;
	}
	return false;
}

bool CInstanceBase::IsTargetableInstance(CInstanceBase& rkInstVictim, bool bIgnoreViewFrustrum)
{
	return rkInstVictim.CanPickInstance(bIgnoreViewFrustrum);
}

// 2004. 07. 07. [levites] - 스킬 사용중 타겟이 바뀌는 문제 해결을 위한 코드
bool CInstanceBase::CanChangeTarget()
{
	return m_GraphicThingInstance.CanChangeTarget();
}

// 2004.07.17.levites.isShow를 ViewFrustumCheck로 변경
bool CInstanceBase::CanPickInstance(bool bIgnoreViewFrustrum /*= false*/)
{
	if (!bIgnoreViewFrustrum && !__IsInViewFrustum())
		return false;

	// Prevents clicking on client objects and guild buildings
	if (IsObject() || IsBuilding())
		return false;

	if (IsDoor())
	{
		if (IsDead())
			return false;
	}

	if (IsPC())
	{
		if ((IsAffect(AFFECT_EUNHYEONG) ||
		   IsAffect(AFFECT_REVIVE_INVISIBILITY) ||
		   IsAffect(AFFECT_INVISIBILITY))
#ifdef ENABLE_CANSEEHIDDENTHING_FOR_GM	
			&& !__MainCanSeeHiddenThing())
#endif
		{
			return false;
		}


	}

	if (IsDead()) {
		if (CPythonBackground::Instance().IsHazardousMap() && IsPC())
			return true;

		return false;
	}

	return true;
}

bool CInstanceBase::CanViewTargetHP(CInstanceBase& rkInstVictim)
{
	if (rkInstVictim.IsStone())
		return true;
	if (rkInstVictim.IsWoodenDoor())
		return true;
	if (rkInstVictim.IsEnemy())
		return true;

	return false;
}

BOOL CInstanceBase::IsPoly()
{
	return m_GraphicThingInstance.IsPoly();
}

BOOL CInstanceBase::IsPC()
{
	return m_GraphicThingInstance.IsPC();
}

BOOL CInstanceBase::IsNPC()
{
	return m_GraphicThingInstance.IsNPC();
}

BOOL CInstanceBase::IsEnemy()
{
	return m_GraphicThingInstance.IsEnemy();
}

BOOL CInstanceBase::IsStone()
{
	return m_GraphicThingInstance.IsStone();
}


BOOL CInstanceBase::IsGuildWall()	//IsBuilding 길드건물전체 IsGuildWall은 담장벽만(문은 제외)
{
	return IsWall(m_dwRace);		
}


BOOL CInstanceBase::IsResource()
{
	switch (m_dwVirtualNumber)
	{
		case 20047:
		case 20048:
		case 20049:
		case 20050:
		case 20051:
		case 20052:
		case 20053:
		case 20054:
		case 20055:
		case 20056:
		case 20057:
		case 20058:
		case 20059:
		case 30301:
		case 30302:
		case 30303:
		case 30304:
		case 30305:
		case 30306:
			return TRUE;
	}

	return FALSE;
}

BOOL CInstanceBase::IsWarp()
{
	return m_GraphicThingInstance.IsWarp();
}

BOOL CInstanceBase::IsGoto()
{
	return m_GraphicThingInstance.IsGoto();
}

BOOL CInstanceBase::IsObject()
{
	return m_GraphicThingInstance.IsObject();
}

BOOL CInstanceBase::IsShop()
{
	return m_GraphicThingInstance.IsShop();
}

BOOL CInstanceBase::IsBuilding()
{
	return m_GraphicThingInstance.IsBuilding();
}

BOOL CInstanceBase::IsDoor()
{
	return m_GraphicThingInstance.IsDoor();
}

BOOL CInstanceBase::IsWoodenDoor()
{
	if (m_GraphicThingInstance.IsDoor())
	{
		int32_t vnum = GetVirtualNumber();
		if (vnum == 13000) // 나무문
			return true;
		if (vnum >= 30111 && vnum <= 30119) // 사귀문
			return true;
		return false;
	}
	return false;
}

BOOL CInstanceBase::IsStoneDoor()
{
	return m_GraphicThingInstance.IsDoor() && 13001 == GetVirtualNumber();
}

BOOL CInstanceBase::IsFlag()
{
	if (GetRace() == 20035)
		return TRUE;
	if (GetRace() == 20036)
		return TRUE;
	if (GetRace() == 20037)
		return TRUE;

	return FALSE;
}

BOOL CInstanceBase::IsForceVisible()
{
	if (IsAffect(AFFECT_SHOW_ALWAYS))
		return TRUE;

	if (IsObject() || IsBuilding() || IsDoor() )
		return TRUE;

	return FALSE;
}

int32_t CInstanceBase::GetInstanceType()
{
	return m_GraphicThingInstance.GetActorType();
}

uint32_t CInstanceBase::GetVirtualID()
{
	return m_GraphicThingInstance.GetVirtualID();
}

uint32_t CInstanceBase::GetVirtualNumber()
{
	return m_dwVirtualNumber;
}

// 2004.07.17.levites.isShow를 ViewFrustumCheck로 변경
bool CInstanceBase::__IsInViewFrustum()
{
	return m_GraphicThingInstance.isShow();
}

bool CInstanceBase::__CanRender()
{
	if (!__IsInViewFrustum())
		return false;

#ifdef ENABLE_CANSEEHIDDENTHING_FOR_GM
	if (IsAffect(AFFECT_INVISIBILITY) && !__MainCanSeeHiddenThing())
		return false;
#else
	if (IsAffect(AFFECT_INVISIBILITY))
		return false;
#endif

	return true;
}


///////////////////////////////////////////////////////////////////////////////////////////////////
// Graphic Control

bool CInstanceBase::IntersectBoundingBox()
{
	float u, v, t;
	return m_GraphicThingInstance.Intersect(&u, &v, &t);
}

bool CInstanceBase::IntersectDefendingSphere()
{
	return m_GraphicThingInstance.IntersectDefendingSphere();
}

float CInstanceBase::GetDistance(CInstanceBase * pkTargetInst)
{
	TPixelPosition TargetPixelPosition;
	pkTargetInst->m_GraphicThingInstance.GetPixelPosition(&TargetPixelPosition);
	return GetDistance(TargetPixelPosition);
}

float CInstanceBase::GetDistance(const TPixelPosition & c_rPixelPosition)
{
	TPixelPosition PixelPosition;
	m_GraphicThingInstance.GetPixelPosition(&PixelPosition);

	float fdx = PixelPosition.x - c_rPixelPosition.x;
	float fdy = PixelPosition.y - c_rPixelPosition.y;

	return sqrtf((fdx*fdx) + (fdy*fdy));
}

CActorInstance& CInstanceBase::GetGraphicThingInstanceRef()
{
	return m_GraphicThingInstance;
}

CActorInstance* CInstanceBase::GetGraphicThingInstancePtr()
{
	return &m_GraphicThingInstance;
}

void CInstanceBase::RefreshActorInstance()
{
	m_GraphicThingInstance.RefreshActorInstance();
}

void CInstanceBase::Refresh(uint32_t dwMotIndex, bool isLoop)
{
	RefreshState(dwMotIndex, isLoop);
}

void CInstanceBase::RestoreRenderMode()
{
	m_GraphicThingInstance.RestoreRenderMode();
}

void CInstanceBase::SetAddRenderMode()
{
	m_GraphicThingInstance.SetAddRenderMode();
}

void CInstanceBase::SetModulateRenderMode()
{
	m_GraphicThingInstance.SetModulateRenderMode();
}

void CInstanceBase::SetRenderMode(int32_t iRenderMode)
{
	m_GraphicThingInstance.SetRenderMode(iRenderMode);
}

void CInstanceBase::SetAddColor(const D3DXCOLOR & c_rColor)
{
	m_GraphicThingInstance.SetAddColor(c_rColor);
}

void CInstanceBase::__SetBlendRenderingMode()
{
	m_GraphicThingInstance.SetBlendRenderMode();
}

void CInstanceBase::__SetAlphaValue(float fAlpha)
{
	m_GraphicThingInstance.SetAlphaValue(fAlpha);
}

float CInstanceBase::__GetAlphaValue()
{
	return m_GraphicThingInstance.GetAlphaValue();
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// Part

void CInstanceBase::SetHair(uint32_t eHair)
{
	if (!HAIR_COLOR_ENABLE)
		return;

	if (!IsPC() && !IsNPC())
		return;
	//CItemData * pItemData;
	float fSpecularPower = 0.0f;
	/*if (CItemManager::Instance().GetItemDataPointer(__GetHairVnum(eHair), &pItemData))
		fSpecularPower=pItemData->GetSpecularPowerf();*/
	
	m_awPart[CRaceData::PART_HAIR] = eHair;
	m_GraphicThingInstance.SetHair(eHair, fSpecularPower);
}

void CInstanceBase::ChangeHair(uint32_t eHair)
{
	if (!HAIR_COLOR_ENABLE)
		return;

	if (IsPC()==false)
		return;

	if (GetPart(CRaceData::PART_HAIR)==eHair)
		return;

	SetHair(eHair);

	//int32_t type = m_GraphicThingInstance.GetMotionMode();

	RefreshState(CRaceMotionData::NAME_WAIT, true);
	//RefreshState(type, true);
}

void CInstanceBase::SetArmor(uint32_t dwArmor)
{
	uint32_t dwShape;
	if (__ArmorVnumToShape(dwArmor, &dwShape))
	{
		CItemData * pItemData;
		if (CItemManager::Instance().GetItemDataPointer(dwArmor, &pItemData))
		{
			float fSpecularPower=pItemData->GetSpecularPowerf();
			SetShape(dwShape, fSpecularPower);
			__GetRefinedEffect(pItemData);
			return;
		}
		__ClearArmorRefineEffect();
	}

	SetShape(dwArmor);
}


#ifdef ENABLE_ACCE_SYSTEM
void CInstanceBase::SetAcce(uint32_t dwAcce)
{
	if (!IsPC())
		return;

	if (IsPoly())
		return;

	dwAcce += 85000;
	ClearAcceEffect();

		
	float fSpecular = 65.0f;
	if (dwAcce > 86000)
	{
		dwAcce -= 1000;
		fSpecular += 35;

		m_dwAcceEffect = EFFECT_REFINED + EFFECT_ACCE;
		__EffectContainer_AttachEffect(m_dwAcceEffect);
	}

	fSpecular /= 100.0f;
	


	m_awPart[CRaceData::PART_ACCE] = dwAcce;

	CItemData * pItemData = nullptr;
	CItemManager::Instance().GetItemDataPointer(dwAcce, &pItemData);

	m_GraphicThingInstance.AttachAcce(pItemData, fSpecular);
#ifdef ENABLE_OBJ_SCALLING
	uint32_t dwRace = GetRace(), dwPos = RaceToJob(dwRace), dwSex = RaceToSex(dwRace);
	dwPos += 1;
	if (dwSex == 0)
		dwPos += 5;

	float fScaleX, fScaleY, fScaleZ, fPositionX, fPositionY, fPositionZ;
	if (pItemData && pItemData->GetItemScale(dwPos, fScaleX, fScaleY, fScaleZ, fPositionX, fPositionY, fPositionZ))
	{
		m_GraphicThingInstance.SetScale(fScaleX, fScaleY, fScaleZ, true);
		if (m_kHorse.IsMounting())
			fPositionZ += 10.0f;

		m_GraphicThingInstance.SetScalePosition(fPositionX, fPositionY, fPositionZ);
	}
#endif
}

void CInstanceBase::ChangeAcce(uint32_t dwAcce)
{
	if (!IsPC())
		return;

	SetAcce(dwAcce);
}

void CInstanceBase::ClearAcceEffect()
{
	if (!m_dwAcceEffect)
		return;

	__EffectContainer_DetachEffect(m_dwAcceEffect);
	m_dwAcceEffect = 0;
}
#endif

void CInstanceBase::SetShape(uint32_t eShape, float fSpecular)
{
	if (IsPoly())
		m_GraphicThingInstance.SetShape(0);
	else
		m_GraphicThingInstance.SetShape(eShape, fSpecular);

	m_eShape = eShape;
}


uint32_t CInstanceBase::GetWeaponType()
{
	uint32_t dwWeapon = GetPart(CRaceData::PART_WEAPON);
	CItemData * pItemData;
	if (!CItemManager::Instance().GetItemDataPointer(dwWeapon, &pItemData))
		return WEAPON_NONE;

	return pItemData->GetWeaponType();
}

/*
void CInstanceBase::SetParts(const uint16_t * c_pParts)
{
	if (IsPoly())
		return;

	if (__IsShapeAnimalWear())
		return;

	uint32_t eWeapon=c_pParts[CRaceData::PART_WEAPON];

	if (__IsChangableWeapon(eWeapon) == false)
			eWeapon = 0;

	if (eWeapon != m_GraphicThingInstance.GetPartItemID(CRaceData::PART_WEAPON))
	{
		m_GraphicThingInstance.AttachPart(CRaceData::PART_MAIN, CRaceData::PART_WEAPON, eWeapon);
		m_awPart[CRaceData::PART_WEAPON] = eWeapon;
	}

	__AttachHorseSaddle();
}
*/

void CInstanceBase::__ClearWeaponRefineEffect()
{
	if (m_swordRefineEffectRight)
	{
		__DetachEffect(m_swordRefineEffectRight);
		m_swordRefineEffectRight = 0;
	}
	if (m_swordRefineEffectLeft)
	{
		__DetachEffect(m_swordRefineEffectLeft);
		m_swordRefineEffectLeft = 0;
	}
}

void CInstanceBase::__ClearArmorRefineEffect()
{
	if (m_armorRefineEffect)
	{
		__DetachEffect(m_armorRefineEffect);
		m_armorRefineEffect = 0;
	}
}

uint32_t CInstanceBase::__GetRefinedEffect(CItemData* pItem)
{
#ifdef ENABLE_SIMPLE_REFINED_EFFECT_CHECK
	uint32_t refine = pItem->GetRefine();
#else
	uint32_t refine = std::max<uint32_t>(pItem->GetRefine() + pItem->GetSocketCount(),ITEM_SOCKET_MAX_NUM) - ITEM_SOCKET_MAX_NUM;
#endif
	switch (pItem->GetType())
	{
	case ITEM_TYPE_WEAPON:
		__ClearWeaponRefineEffect();		
		if (refine < 7)	//현재 제련도 7 이상만 이펙트가 있습니다.
			return 0;
		switch(pItem->GetSubType())
		{
		case WEAPON_DAGGER:
			m_swordRefineEffectRight = EFFECT_REFINED+EFFECT_SMALLSWORD_REFINED7+refine-7;
			m_swordRefineEffectLeft = EFFECT_REFINED+EFFECT_SMALLSWORD_REFINED7_LEFT+refine-7;
			break;
		case WEAPON_FAN:
			m_swordRefineEffectRight = EFFECT_REFINED+EFFECT_FANBELL_REFINED7+refine-7;
			break;
		case WEAPON_ARROW:
		case WEAPON_BELL:
			m_swordRefineEffectRight = EFFECT_REFINED+EFFECT_SMALLSWORD_REFINED7+refine-7;
			break;
		case WEAPON_BOW:
			m_swordRefineEffectRight = EFFECT_REFINED+EFFECT_BOW_REFINED7+refine-7;
			break;
#ifdef ENABLE_WOLFMAN_CHARACTER
		case WEAPON_CLAW:
			m_swordRefineEffectRight = EFFECT_REFINED + EFFECT_SMALLSWORD_REFINED7 + refine - 7;
			m_swordRefineEffectLeft = EFFECT_REFINED + EFFECT_SMALLSWORD_REFINED7_LEFT + refine - 7;
			break;
#endif
		default:
			m_swordRefineEffectRight = EFFECT_REFINED+EFFECT_SWORD_REFINED7+refine-7;
		}
		if (m_swordRefineEffectRight)
			m_swordRefineEffectRight = __AttachEffect(m_swordRefineEffectRight);
		if (m_swordRefineEffectLeft)
			m_swordRefineEffectLeft = __AttachEffect(m_swordRefineEffectLeft);
		break;
	case ITEM_TYPE_ARMOR:
		__ClearArmorRefineEffect();

		// 갑옷 특화 이펙트
		if (pItem->GetSubType() == ARMOR_BODY)
		{
			uint32_t vnum = pItem->GetIndex();

			if ((12010 <= vnum && vnum <= 12049)
#ifdef ENABLE_WOLFMAN_CHARACTER
				|| (21080 <= vnum && vnum <= 21089)
#endif
			)
			{
				__AttachEffect(EFFECT_REFINED+EFFECT_BODYARMOR_SPECIAL);
				__AttachEffect(EFFECT_REFINED+EFFECT_BODYARMOR_SPECIAL2);
			}
#ifdef ENABLE_LVL115_ARMOR_EFFECT
			else if (20760 <= vnum && vnum <= 20959)
				__AttachEffect(EFFECT_REFINED+EFFECT_BODYARMOR_SPECIAL3);
#endif //ENABLE_LVL115_ARMOR_EFFECT
		}

		if (refine < 7)	//현재 제련도 7 이상만 이펙트가 있습니다.
			return 0;

		if (pItem->GetSubType() == ARMOR_BODY)
		{
			m_armorRefineEffect = EFFECT_REFINED+EFFECT_BODYARMOR_REFINED7+refine-7;
			__AttachEffect(m_armorRefineEffect);
		}
		break;
	case ITEM_TYPE_COSTUME:
#ifdef ENABLE_WEAPON_COSTUME_SYSTEM
		if (pItem->GetSubType() == COSTUME_WEAPON)
		{
			__ClearWeaponRefineEffect();

#ifdef USE_WEAPON_COSTUME_WITH_EFFECT
			switch(pItem->GetValue(3))
			{
				case WEAPON_DAGGER:
					m_swordRefineEffectRight = EFFECT_REFINED+EFFECT_SMALLSWORD_REFINED9;
					m_swordRefineEffectLeft = EFFECT_REFINED+EFFECT_SMALLSWORD_REFINED9_LEFT;
					break;
				case WEAPON_FAN:
					m_swordRefineEffectRight = EFFECT_REFINED+EFFECT_FANBELL_REFINED9;
					break;
				case WEAPON_ARROW:
				case WEAPON_BELL:
					m_swordRefineEffectRight = EFFECT_REFINED+EFFECT_SMALLSWORD_REFINED9;
					break;
				case WEAPON_BOW:
					m_swordRefineEffectRight = EFFECT_REFINED+EFFECT_BOW_REFINED9;
					break;
#ifdef ENABLE_WOLFMAN_CHARACTER
				case WEAPON_CLAW:
					m_swordRefineEffectRight = EFFECT_REFINED + EFFECT_SMALLSWORD_REFINED9;
					m_swordRefineEffectLeft = EFFECT_REFINED + EFFECT_SMALLSWORD_REFINED9_LEFT;
					break;
#endif
				default:
					m_swordRefineEffectRight = EFFECT_REFINED+EFFECT_SWORD_REFINED9;
			}
			if (m_swordRefineEffectRight)
				m_swordRefineEffectRight = __AttachEffect(m_swordRefineEffectRight);
			if (m_swordRefineEffectLeft)
				m_swordRefineEffectLeft = __AttachEffect(m_swordRefineEffectLeft);
#endif //USE_WEAPON_COSTUME_WITH_EFFECT
			break;
		}
#endif
#ifdef USE_BODY_COSTUME_WITH_EFFECT
		if (pItem->GetSubType() == COSTUME_BODY)
		{
			__ClearArmorRefineEffect();

			// lvl80 armor's effect (blue smoke+bubbles)
			// {
				// __AttachEffect(EFFECT_REFINED+EFFECT_BODYARMOR_SPECIAL);
				// __AttachEffect(EFFECT_REFINED+EFFECT_BODYARMOR_SPECIAL2);
			// }

			// lvl105 armor's effect (sparkles)
			// {
				// __AttachEffect(EFFECT_REFINED+EFFECT_BODYARMOR_SPECIAL3);
			// }

			m_armorRefineEffect = EFFECT_REFINED+EFFECT_BODYARMOR_REFINED9;
			__AttachEffect(m_armorRefineEffect);
			break;
		}
#endif //USE_BODY_COSTUME_WITH_EFFECT
		break;
	}
	return 0;
}

bool CInstanceBase::SetWeapon(uint32_t eWeapon)
{
	if (IsPoly())
		return false;
	
	if (__IsShapeAnimalWear())
		return false;
	
	if (__IsChangableWeapon(eWeapon) == false)
		eWeapon = 0;

	if (CPythonBackground::Instance().IsPrimalMap())
	{
		uint32_t weaponType = WEAPON_NONE;
		CItemData * pItemData;
		if (CItemManager::Instance().GetItemDataPointer(eWeapon, &pItemData))
			weaponType = pItemData->GetWeaponType();

		switch (weaponType)
		{
		case WEAPON_SWORD:
			eWeapon = 19;
			break;
		case WEAPON_DAGGER:
			eWeapon = 1009;
			break;
		case WEAPON_BOW:
			eWeapon = 2009;
			break;
		case WEAPON_TWO_HANDED:
			eWeapon = 3009;
			break;
		case WEAPON_BELL:
			eWeapon = 5009;
			break;
		case WEAPON_FAN:
			eWeapon = 7009;
			break;
		}
	}
	m_GraphicThingInstance.AttachWeapon(eWeapon);
	m_awPart[CRaceData::PART_WEAPON] = eWeapon;
	
	//Weapon Effect
	CItemData * pItemData;
	if (CItemManager::Instance().GetItemDataPointer(eWeapon, &pItemData))
		__GetRefinedEffect(pItemData);
	else
		__ClearWeaponRefineEffect();

	return true;
}

void CInstanceBase::ChangeWeapon(uint32_t eWeapon)
{
	if (CPythonBackground::Instance().IsPrimalMap())
	{
		uint32_t weaponType = WEAPON_NONE;
		CItemData * pItemData;
		if (CItemManager::Instance().GetItemDataPointer(eWeapon, &pItemData))
			weaponType = pItemData->GetWeaponType();

		switch (weaponType)
		{
		case WEAPON_SWORD:
			eWeapon = 19;
			break;
		case WEAPON_DAGGER:
			eWeapon = 1009;
			break;
		case WEAPON_BOW:
			eWeapon = 2009;
			break;
		case WEAPON_TWO_HANDED:
			eWeapon = 3009;
			break;
		case WEAPON_BELL:
			eWeapon = 5009;
			break;
		case WEAPON_FAN:
			eWeapon = 7009;
			break;
		}
	}

	if (eWeapon == m_GraphicThingInstance.GetPartItemID(CRaceData::PART_WEAPON))
		return;

	if (SetWeapon(eWeapon))
		RefreshState(CRaceMotionData::NAME_WAIT, true);
}

bool CInstanceBase::ChangeArmor(uint32_t dwArmor)
{
	uint32_t eShape;
	__ArmorVnumToShape(dwArmor, &eShape);

	if (GetShape()==eShape)
		return false;

	CAffectFlagContainer kAffectFlagContainer;
	kAffectFlagContainer.CopyInstance(m_kAffectFlagContainer);

	uint32_t dwVID = GetVirtualID();
	uint32_t dwRace = GetRace();
	uint32_t eHair = GetPart(CRaceData::PART_HAIR);
#ifdef ENABLE_ACCE_SYSTEM
	uint32_t dwAcce = GetPart(CRaceData::PART_ACCE);
#endif
	uint32_t eWeapon = GetPart(CRaceData::PART_WEAPON);
	float fRot = GetRotation();
	float fAdvRot = GetAdvancingRotation();

	if (IsWalking())
		EndWalking();

	//////////////////////////////////////////////////////
	__ClearAffects();
	//////////////////////////////////////////////////////

	if (!SetRace(dwRace))
	{
		TraceError("CPythonCharacterManager::ChangeArmor - SetRace VID[%d] Race[%d] ERROR", dwVID, dwRace);
		return false;
	}

	SetArmor(dwArmor);
	SetHair(eHair);
	SetWeapon(eWeapon);
#ifdef ENABLE_ACCE_SYSTEM
	SetAcce(dwAcce);
#endif
	SetRotation(fRot);
	SetAdvancingRotation(fAdvRot);

	__AttachHorseSaddle();

	RefreshState(CRaceMotionData::NAME_WAIT, true);

	if (m_kHorse.IsMounting()) {
		m_GraphicThingInstance.m_nextAllowedMovement = CTimer::Instance().GetCurrentMillisecond() + 250;
	}

	/////////////////////////////////////////////////
	SetAffectFlagContainer(kAffectFlagContainer);
	/////////////////////////////////////////////////

	CActorInstance::IEventHandler& rkEventHandler=GetEventHandlerRef();
	rkEventHandler.OnChangeShape();

	return true;
}

bool CInstanceBase::__IsShapeAnimalWear()
{
	if (100 == GetShape() ||
		101 == GetShape() ||
		102 == GetShape() ||
		103 == GetShape())
		return true;

	return false;
}

uint32_t CInstanceBase::__GetRaceType()
{
	return m_eRaceType;
}


void CInstanceBase::RefreshState(uint32_t dwMotIndex, bool isLoop)
{
	uint32_t dwPartItemID = m_GraphicThingInstance.GetPartItemID(CRaceData::PART_WEAPON);

	uint8_t byItemType = 0xff;
	uint8_t bySubType = 0xff;

	CItemManager & rkItemMgr = CItemManager::Instance();
	CItemData * pItemData;
	
	if (rkItemMgr.GetItemDataPointer(dwPartItemID, &pItemData))
	{
		byItemType = pItemData->GetType();
		bySubType = pItemData->GetWeaponType();
	}

	if (IsPoly())
		SetMotionMode(CRaceMotionData::MODE_GENERAL);
	else if (IsWearingDress())
		SetMotionMode(CRaceMotionData::MODE_WEDDING_DRESS);
	else if (IsHoldingPickAxe())
	{
		if (m_kHorse.IsMounting())
			SetMotionMode(CRaceMotionData::MODE_HORSE);
		else
			SetMotionMode(CRaceMotionData::MODE_GENERAL);
	}
	else if (ITEM_TYPE_ROD == byItemType)
	{
		if (m_kHorse.IsMounting())
			SetMotionMode(CRaceMotionData::MODE_HORSE);
		else
			SetMotionMode(CRaceMotionData::MODE_FISHING);
	}
	else if (m_kHorse.IsMounting())
	{
		switch (bySubType)
		{
			case WEAPON_SWORD:
				SetMotionMode(CRaceMotionData::MODE_HORSE_ONEHAND_SWORD);
				break;

			case WEAPON_TWO_HANDED:
				SetMotionMode(CRaceMotionData::MODE_HORSE_TWOHAND_SWORD); // Only Warrior
				break;

			case WEAPON_DAGGER:
				SetMotionMode(CRaceMotionData::MODE_HORSE_DUALHAND_SWORD); // Only Assassin
				break;

			case WEAPON_FAN:
				SetMotionMode(CRaceMotionData::MODE_HORSE_FAN); // Only Shaman
				break;

			case WEAPON_BELL:
				SetMotionMode(CRaceMotionData::MODE_HORSE_BELL); // Only Shaman
				break;

			case WEAPON_BOW:
				SetMotionMode(CRaceMotionData::MODE_HORSE_BOW); // Only Shaman
				break;
#ifdef ENABLE_WOLFMAN_CHARACTER
			case WEAPON_CLAW:
				SetMotionMode(CRaceMotionData::MODE_HORSE_CLAW); // Only Wolfman
				break;
#endif
			default:
				SetMotionMode(CRaceMotionData::MODE_HORSE);
				break;
		}
	}
	else
	{
		switch (bySubType)
		{
			case WEAPON_SWORD:
				SetMotionMode(CRaceMotionData::MODE_ONEHAND_SWORD);
				break;

			case WEAPON_TWO_HANDED:
				SetMotionMode(CRaceMotionData::MODE_TWOHAND_SWORD); // Only Warrior
				break;

			case WEAPON_DAGGER:
				SetMotionMode(CRaceMotionData::MODE_DUALHAND_SWORD); // Only Assassin
				break;

			case WEAPON_BOW:
				SetMotionMode(CRaceMotionData::MODE_BOW); // Only Assassin
				break;

			case WEAPON_FAN:
				SetMotionMode(CRaceMotionData::MODE_FAN); // Only Shaman
				break;

			case WEAPON_BELL:
				SetMotionMode(CRaceMotionData::MODE_BELL); // Only Shaman
				break;
#ifdef ENABLE_WOLFMAN_CHARACTER
			case WEAPON_CLAW:
				SetMotionMode(CRaceMotionData::MODE_CLAW); // Only Wolfman
				break;
#endif
			case WEAPON_ARROW:
			default:
				SetMotionMode(CRaceMotionData::MODE_GENERAL);
				break;
		}
	}

	if (isLoop)
		m_GraphicThingInstance.InterceptLoopMotion(dwMotIndex);
	else
		m_GraphicThingInstance.InterceptOnceMotion(dwMotIndex);

	RefreshActorInstance();
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// Device

void CInstanceBase::RegisterBoundingSphere()
{
	// Stone 일 경우 DeforomNoSkin 을 하면
	// 낙하하는 애니메이션 같은 경우 애니메이션이
	// 바운드 박스에 영향을 미쳐 컬링이 제대로 이루어지지 않는다.
	if (!IsStone())
		m_GraphicThingInstance.DeformNoSkin();

	m_GraphicThingInstance.RegisterBoundingSphere();
}

bool CInstanceBase::CreateDeviceObjects()
{
	return m_GraphicThingInstance.CreateDeviceObjects();
}

void CInstanceBase::DestroyDeviceObjects()
{
	m_GraphicThingInstance.DestroyDeviceObjects();
}

void CInstanceBase::Destroy()
{	
	DetachTextTail();
	
	DismountHorse();

	m_kQue_kCmdNew.clear();
	
	__EffectContainer_Destroy();
	__StoneSmoke_Destroy();

	if (__IsMainInstance())
		__ClearMainInstance();	
	
	m_GraphicThingInstance.Destroy();
	
	__Initialize();
}

void CInstanceBase::__InitializeRotationSpeed()
{
	SetRotationSpeed(c_fDefaultRotationSpeed);
}

void CInstanceBase::__Warrior_Initialize()
{
	m_kWarrior.m_dwGeomgyeongEffect=0;
}

void CInstanceBase::__Initialize()
{
	__Warrior_Initialize();
	__StoneSmoke_Inialize();
	__EffectContainer_Initialize();
	__InitializeRotationSpeed();

	SetEventHandler(CActorInstance::IEventHandler::GetEmptyPtr());

	m_kAffectFlagContainer.Clear();

	m_dwLevel = 0;
	m_dwGuildID = 0;
	m_dwEmpireID = 0;

	m_eType = 0;
	m_eRaceType = 0;
	m_eShape = 0;
	m_dwRace = 0;
	m_dwVirtualNumber = 0;

	m_dwBaseCmdTime=0;
	m_dwBaseChkTime=0;
	m_dwSkipTime=0;

	m_GraphicThingInstance.Initialize();

	m_dwAdvActorVID=0;
	m_dwLastDmgActorVID=0;

	m_nAverageNetworkGap=0;
	m_dwNextUpdateHeightTime=0;

	// Moving by keyboard
	m_iRotatingDirection = DEGREE_DIRECTION_SAME;

	// Moving by mouse	
	m_isTextTail = FALSE;
	m_isGoing = FALSE;
	NEW_SetSrcPixelPosition(TPixelPosition(0, 0, 0));
	NEW_SetDstPixelPosition(TPixelPosition(0, 0, 0));

	m_kPPosDust = TPixelPosition(0, 0, 0);


	m_kQue_kCmdNew.clear();

	m_dwLastComboIndex = 0;

	m_swordRefineEffectRight = 0;
	m_swordRefineEffectLeft = 0;
	m_armorRefineEffect = 0;
#ifdef ENABLE_ACCE_SYSTEM
	m_dwAcceEffect = 0;
#endif

	m_sAlignment = 0;
	m_byPKMode = 0;
	m_isKiller = false;
	m_isPartyMember = false;

	m_bEnableTCPState = TRUE;

	m_stName.clear();

	memset(m_awPart, 0, sizeof(m_awPart));
	memset(m_adwCRCAffectEffect, 0, sizeof(m_adwCRCAffectEffect));
	//memset(m_adwCRCEmoticonEffect, 0, sizeof(m_adwCRCEmoticonEffect));
	memset(&m_kMovAfterFunc, 0, sizeof(m_kMovAfterFunc));

	m_bDamageEffectType = false;
	m_dwDuelMode = DUEL_NONE;
	m_dwEmoticonTime = 0;

	mp_flyTargetInstance = nullptr;
	m_IsAlwaysRender = false;
}

CInstanceBase::CInstanceBase()
{
	__Initialize();
}

CInstanceBase::~CInstanceBase()
{
	Destroy();
}


void CInstanceBase::GetBoundBox(D3DXVECTOR3 * vtMin, D3DXVECTOR3 * vtMax)
{
	m_GraphicThingInstance.GetBoundBox(vtMin, vtMax);
}

bool CInstanceBase::IsMiningVID(uint32_t vid)
{
	return m_dwMiningVID == vid && GetGraphicThingInstancePtr()->IsMining();
}

void CInstanceBase::StartMining(uint32_t vid)
{
	m_dwMiningVID = vid;
}

void CInstanceBase::CancelMining()
{
	GetGraphicThingInstancePtr()->InterceptLoopMotion(CRaceMotionData::NAME_WAIT);
}

bool CInstanceBase::IsAlwaysRender() const
{
	return m_IsAlwaysRender;
}

void CInstanceBase::SetAlwaysRender(bool val)
{
	m_IsAlwaysRender = val;
}


