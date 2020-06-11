#include "StdAfx.h"
#include "ActorInstance.h"

void CActorInstance::__OnSyncing()
{
	IEventHandler& rkEventHandler=__GetEventHandlerRef();

	IEventHandler::SState kState;
	kState.kPPosSelf=NEW_GetCurPixelPositionRef();
	kState.fAdvRotSelf=GetAdvancingRotation();
	rkEventHandler.OnSyncing(kState);
}

void CActorInstance::__OnWaiting()
{
	assert(!IsPushing());

	IEventHandler& rkEventHandler=__GetEventHandlerRef();

	IEventHandler::SState kState;
	kState.kPPosSelf=NEW_GetCurPixelPositionRef();
	kState.fAdvRotSelf=GetAdvancingRotation();
	rkEventHandler.OnWaiting(kState);
}

void CActorInstance::__OnMoving()
{
	assert(!IsPushing());

	IEventHandler& rkEventHandler=__GetEventHandlerRef();

	const TPixelPosition& c_rkPPosCur=NEW_GetCurPixelPositionRef();
	const TPixelPosition& c_rkPPosDst=NEW_GetDstPixelPositionRef();

	TPixelPosition kPPosDir=c_rkPPosDst-c_rkPPosCur;
	float distance=sqrt(kPPosDir.x*kPPosDir.x+kPPosDir.y*kPPosDir.y);

	IEventHandler::SState kState;

	if (distance>1000.0f)
	{
		D3DXVec3Normalize(&kPPosDir, &kPPosDir);
		D3DXVec3Scale(&kPPosDir, &kPPosDir, 1000.0f);
		D3DXVec3Add(&kState.kPPosSelf, &kPPosDir, &c_rkPPosCur);
	}
	else
	{
		kState.kPPosSelf=c_rkPPosDst;
	}
	kState.fAdvRotSelf=GetAdvancingRotation();
	rkEventHandler.OnMoving(kState);
}


void CActorInstance::__OnMove()
{
	IEventHandler& rkEventHandler=__GetEventHandlerRef();

	IEventHandler::SState kState;
	kState.kPPosSelf=NEW_GetCurPixelPositionRef();
	kState.fAdvRotSelf=GetAdvancingRotation();
	rkEventHandler.OnMove(kState);
}

void CActorInstance::__OnStop()
{
	IEventHandler& rkEventHandler=__GetEventHandlerRef();

	IEventHandler::SState kState;
	kState.kPPosSelf=NEW_GetCurPixelPositionRef();
	kState.fAdvRotSelf=GetAdvancingRotation();
	rkEventHandler.OnStop(kState);
}

void CActorInstance::__OnWarp()
{
	IEventHandler& rkEventHandler=__GetEventHandlerRef();

	IEventHandler::SState kState;
	kState.kPPosSelf=NEW_GetCurPixelPositionRef();
	kState.fAdvRotSelf=GetAdvancingRotation();
	rkEventHandler.OnWarp(kState);
}

void CActorInstance::__OnAttack(uint16_t wMotionIndex)
{
	IEventHandler& rkEventHandler=__GetEventHandlerRef();

	IEventHandler::SState kState;
	kState.kPPosSelf=NEW_GetCurPixelPositionRef();
	kState.fAdvRotSelf=GetTargetRotation();
	rkEventHandler.OnAttack(kState, wMotionIndex);
}

void CActorInstance::__OnUseSkill(uint32_t uMotSkill, uint32_t uLoopCount, bool isMovingSkill)
{
	IEventHandler& rkEventHandler=__GetEventHandlerRef();

	IEventHandler::SState kState;
	kState.kPPosSelf=NEW_GetCurPixelPositionRef();
	kState.fAdvRotSelf=GetAdvancingRotation();

	uint32_t uArg=uLoopCount;
	if (isMovingSkill)
		uArg|=1<<4;

	rkEventHandler.OnUseSkill(kState, uMotSkill, uArg);
}

void CActorInstance::__OnHit(uint32_t uSkill, CActorInstance& rkActorVictm, BOOL isSendPacket)
{
	IEventHandler& rkEventHandler=__GetEventHandlerRef();
	rkEventHandler.OnHit(uSkill, rkActorVictm, isSendPacket);
}

void CActorInstance::__OnClearAffects()
{
	IEventHandler& rkEventHandler=__GetEventHandlerRef();
	rkEventHandler.OnClearAffects();
}

void CActorInstance::__OnSetAffect(uint32_t uAffect)
{
	IEventHandler& rkEventHandler=__GetEventHandlerRef();
	rkEventHandler.OnSetAffect(uAffect);
}

void CActorInstance::__OnResetAffect(uint32_t uAffect)
{
	IEventHandler& rkEventHandler=__GetEventHandlerRef();
	rkEventHandler.OnResetAffect(uAffect);
}


CActorInstance::IEventHandler& CActorInstance::__GetEventHandlerRef()
{
	assert(m_pkEventHandler!=nullptr && "CActorInstance::GetEventHandlerRef");
	return *m_pkEventHandler;
}

CActorInstance::IEventHandler* CActorInstance::__GetEventHandlerPtr()
{
	return m_pkEventHandler;
}

void CActorInstance::SetEventHandler(IEventHandler* pkEventHandler)
{
	m_pkEventHandler=pkEventHandler;
}

CActorInstance::IEventHandler* CActorInstance::IEventHandler::GetEmptyPtr()
{
	static class CEmptyEventHandler : public IEventHandler
	{
		public:
			CEmptyEventHandler() {}
			virtual ~CEmptyEventHandler() {}

			virtual void OnSyncing(const SState& c_rkState) {}
			virtual void OnWaiting(const SState& c_rkState) {}
			virtual void OnMoving(const SState& c_rkState) {}
			virtual void OnMove(const SState& c_rkState) {}
			virtual void OnStop(const SState& c_rkState) {}
			virtual void OnWarp(const SState& c_rkState) {}

			virtual void OnClearAffects() {}
			virtual void OnSetAffect(uint32_t uAffect) {}
			virtual void OnResetAffect(uint32_t uAffect) {}

			virtual void OnAttack(const SState& c_rkState, uint16_t wMotionIndex) {}
			virtual void OnUseSkill(const SState& c_rkState, uint32_t uMotSkill, uint32_t uMotLoopCount) {}

			virtual void OnHit(uint32_t uMotAttack, CActorInstance& rkActorVictim, BOOL isSendPacket) {}

			virtual void OnChangeShape() {}

	} s_kEmptyEventHandler;

	return &s_kEmptyEventHandler;
}
