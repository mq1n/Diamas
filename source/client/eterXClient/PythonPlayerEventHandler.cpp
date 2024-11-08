#include "StdAfx.h"
#include "PythonPlayerEventHandler.h"
#include "PythonPlayer.h"
#include "PythonCharacterManager.h"
#include "PythonNetworkStream.h"

CPythonPlayerEventHandler& CPythonPlayerEventHandler::GetSingleton()
{
	static CPythonPlayerEventHandler s_kPlayerEventHandler;
	return s_kPlayerEventHandler;
}

CPythonPlayerEventHandler::~CPythonPlayerEventHandler() = default;

void CPythonPlayerEventHandler::OnClearAffects()
{
	CPythonPlayer::Instance().ClearAffects();
}

void CPythonPlayerEventHandler::OnSetAffect(uint32_t uAffect)
{
	CPythonPlayer::Instance().SetAffect(uAffect);
}

void CPythonPlayerEventHandler::OnResetAffect(uint32_t uAffect)
{
	CPythonPlayer::Instance().ResetAffect(uAffect);
}

void CPythonPlayerEventHandler::OnSyncing(const SState& c_rkState)
{
	const TPixelPosition& c_rkPPosCurSyncing=c_rkState.kPPosSelf;
	m_kPPosPrevWaiting=c_rkPPosCurSyncing;
}

//This function indicates that a player made a tiny step and stopped afterwards.
//=> No delay rarely happens.
void CPythonPlayerEventHandler::OnWaiting(const SState& c_rkState)
{
	uint32_t dwCurTime = ELTimer_GetMSec();
	m_dwNextMovingNotifyTime = dwCurTime;  // We want to instantly get the moving state because we might do bigger steps now.

	const TPixelPosition& c_rkPPosCurWaiting=c_rkState.kPPosSelf;
	float dx=m_kPPosPrevWaiting.x-c_rkPPosCurWaiting.x;
	float dy=m_kPPosPrevWaiting.y-c_rkPPosCurWaiting.y;
	float len=sqrt(dx*dx+dy*dy);

	if (len<1.0f)
		return;

	m_kPPosPrevWaiting=c_rkPPosCurWaiting;

	CPythonNetworkStream& rkNetStream=CPythonNetworkStream::Instance();
	rkNetStream.SendCharacterStatePacket(c_rkState.kPPosSelf, c_rkState.fAdvRotSelf, FUNC_WAIT, 0);

	//Trace("waiting\n");
}

//This function indicates that a player is moving.
//It keeps getting called, so we need to delay it.
void CPythonPlayerEventHandler::OnMoving(const SState& c_rkState)
{
	uint32_t dwCurTime=ELTimer_GetMSec();		
	if (m_dwNextMovingNotifyTime>dwCurTime)
		return;
	
	m_dwNextMovingNotifyTime = dwCurTime + 175; // If we keep moving delaying the stream of packets is fine!

	CPythonNetworkStream& rkNetStream=CPythonNetworkStream::Instance();
	rkNetStream.SendCharacterStatePacket(c_rkState.kPPosSelf, c_rkState.fAdvRotSelf, FUNC_MOVE, 0);

//	Trace("moving\n");
}

//This function indicates that a player started to move after a wait.
//Only called once after a wait state.
void CPythonPlayerEventHandler::OnMove(const SState& c_rkState)
{
	uint32_t dwCurTime=ELTimer_GetMSec();
	m_dwNextMovingNotifyTime=dwCurTime + 100;

	CPythonNetworkStream& rkNetStream=CPythonNetworkStream::Instance();
	rkNetStream.SendCharacterStatePacket(c_rkState.kPPosSelf, c_rkState.fAdvRotSelf, FUNC_MOVE, 0);

//	Trace("move\n");
}

//This function indicates that a player stopped completely.
void CPythonPlayerEventHandler::OnStop(const SState& c_rkState)
{
	CPythonNetworkStream& rkNetStream=CPythonNetworkStream::Instance();
	rkNetStream.SendCharacterStatePacket(c_rkState.kPPosSelf, c_rkState.fAdvRotSelf, FUNC_WAIT, 0);

//	Trace("stop\n");
}

void CPythonPlayerEventHandler::OnWarp(const SState& c_rkState)
{
	CPythonNetworkStream& rkNetStream=CPythonNetworkStream::Instance();
	rkNetStream.SendCharacterStatePacket(c_rkState.kPPosSelf, c_rkState.fAdvRotSelf, FUNC_WAIT, 0);
}

void CPythonPlayerEventHandler::OnAttack(const SState& c_rkState, uint16_t wMotionIndex)
{
//	Tracef("CPythonPlayerEventHandler::OnAttack [%d]\n", wMotionIndex);
	assert(wMotionIndex < 255);

	CPythonNetworkStream& rkNetStream=CPythonNetworkStream::Instance();
	rkNetStream.SendCharacterStatePacket(c_rkState.kPPosSelf, c_rkState.fAdvRotSelf, FUNC_COMBO, wMotionIndex);
}

void CPythonPlayerEventHandler::OnUseSkill(const SState& c_rkState, uint32_t uMotSkill, uint32_t uArg)
{
	CPythonNetworkStream& rkNetStream=CPythonNetworkStream::Instance();
	rkNetStream.SendCharacterStatePacket(c_rkState.kPPosSelf, c_rkState.fAdvRotSelf, FUNC_SKILL|uMotSkill, uArg);
}

void CPythonPlayerEventHandler::OnUpdate()
{
}

void CPythonPlayerEventHandler::OnChangeShape()
{
	CPythonPlayer::Instance().NEW_Stop();
}

void CPythonPlayerEventHandler::OnHit(uint32_t uSkill, CActorInstance& rkActorVictim, BOOL isSendPacket)
{
	uint32_t dwVIDVictim=rkActorVictim.GetVirtualID();

	// Update Target
	CPythonPlayer::Instance().SetTarget(dwVIDVictim, FALSE);
	// Update Target

	if (isSendPacket)
	{
		CPythonNetworkStream& rkStream=CPythonNetworkStream::Instance();
		rkStream.SendAttackPacket(uSkill, dwVIDVictim);
	}

	if (!rkActorVictim.IsPushing())
		return;

	// 거대 몬스터 밀림 제외
	extern bool IS_HUGE_RACE(uint32_t vnum);
	if (IS_HUGE_RACE(rkActorVictim.GetRace()))
		return;


	CPythonCharacterManager::Instance().AdjustCollisionWithOtherObjects(&rkActorVictim);

	const TPixelPosition& kPPosLast=rkActorVictim.NEW_GetLastPixelPositionRef();

	SVictim kVictim;
	kVictim.m_dwVID=dwVIDVictim;
	kVictim.m_lPixelX=int32_t(kPPosLast.x);
	kVictim.m_lPixelY=int32_t(kPPosLast.y);

	rkActorVictim.TEMP_Push(kVictim.m_lPixelX, kVictim.m_lPixelY);

	m_kVctkVictim.emplace_back(kVictim);
}

void CPythonPlayerEventHandler::FlushVictimList()
{
	if (m_kVctkVictim.empty())
		return;

	// #0000682: [M2EU] 대진각 스킬 사용시 튕김 
	//uint32_t SYNC_POSITION_COUNT_LIMIT = 16;
	uint32_t uiVictimCount = m_kVctkVictim.size();

	CPythonNetworkStream& rkStream=CPythonNetworkStream::Instance();

	SPacketCGSyncPosition kPacketSyncPos;
	kPacketSyncPos.wSize=sizeof(kPacketSyncPos)+sizeof(SPacketSyncPositionElement) * uiVictimCount;

	rkStream.Send(sizeof(kPacketSyncPos), &kPacketSyncPos);

	for (uint32_t i = 0; i < uiVictimCount; ++i)
	{
		const SVictim& rkVictim =  m_kVctkVictim[i];
		rkStream.SendSyncPositionElementPacket(rkVictim.m_dwVID, rkVictim.m_lPixelX, rkVictim.m_lPixelY);		
	}

	m_kVctkVictim.clear();
}

CPythonPlayerEventHandler::CPythonPlayerEventHandler()
{
	m_dwPrevComboIndex=0;
	m_dwNextMovingNotifyTime=0;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////

void CPythonPlayerEventHandler::CNormalBowAttack_FlyEventHandler_AutoClear::OnSetFlyTarget()
{
	SState s;
	m_pInstMain->NEW_GetPixelPosition(&s.kPPosSelf);
	s.fAdvRotSelf=m_pInstMain->GetGraphicThingInstancePtr()->GetTargetRotation();

	CInstanceBase* targetInstance = m_pInstMain->GetFlyTargetInstance();
	if (!targetInstance)
		targetInstance = m_pInstTarget ? m_pInstTarget : m_pInstMain;

	CPythonNetworkStream& rpns=CPythonNetworkStream::Instance();
	rpns.SendFlyTargetingPacket(targetInstance->GetVirtualID(), targetInstance->GetGraphicThingInstancePtr()->OnGetFlyTargetPosition());
}
void CPythonPlayerEventHandler::CNormalBowAttack_FlyEventHandler_AutoClear::OnShoot(uint32_t dwSkillIndex)
{
	CPythonNetworkStream& rpns=CPythonNetworkStream::Instance();
	rpns.SendShootPacket(dwSkillIndex);
}

void CPythonPlayerEventHandler::CNormalBowAttack_FlyEventHandler_AutoClear::Set(CPythonPlayerEventHandler * pParent, CInstanceBase * pInstMain, CInstanceBase * pInstTarget)
{
	m_pParent=(pParent);
	m_pInstMain=(pInstMain);
	m_pInstTarget=(pInstTarget);
}

void CPythonPlayerEventHandler::CNormalBowAttack_FlyEventHandler_AutoClear::SetTarget(CInstanceBase* pInstTarget)
{
	m_pInstTarget = pInstTarget;
}

void CPythonPlayerEventHandler::CNormalBowAttack_FlyEventHandler_AutoClear::OnExplodingAtAnotherTarget(uint32_t dwSkillIndex, uint32_t dwVID)
{
	return;

	Tracef("Shoot : Hitting another target : %d, %d\n", dwSkillIndex, dwVID);

	CPythonNetworkStream& rkStream=CPythonNetworkStream::Instance();
	rkStream.SendAttackPacket(dwSkillIndex, dwVID);

	CPythonCharacterManager& rkChrMgr = CPythonCharacterManager::Instance();
	CInstanceBase * pInstance = rkChrMgr.GetInstancePtr(dwVID);
	if (pInstance)
	{
		pInstance->GetGraphicThingInstanceRef().OnShootDamage();
	}
}
void CPythonPlayerEventHandler::CNormalBowAttack_FlyEventHandler_AutoClear::OnExplodingAtTarget(uint32_t dwSkillIndex)
{
//	Tracef("Shoot : 원하는 target에 맞았습니다 : %d, %d\n", dwSkillIndex, m_pInstTarget->GetVirtualID());
//	CPythonNetworkStream& rkStream=CPythonNetworkStream::Instance();
//	rkStream.SendAttackPacket(dwSkillIndex, m_pInstTarget->GetVirtualID());
}
