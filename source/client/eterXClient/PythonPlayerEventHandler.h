#pragma once

#include "../eterGameLib/ActorInstance.h"
#include "../eterGameLib/FlyHandler.h"

#include "PythonNetworkStream.h"
#include "InstanceBase.h"

class CPythonPlayerEventHandler : public CActorInstance::IEventHandler
{
	public:
		static CPythonPlayerEventHandler& GetSingleton();

	public:
		virtual ~CPythonPlayerEventHandler();

		virtual void OnSyncing(const SState& c_rkState);
		virtual void OnWaiting(const SState& c_rkState);
		virtual void OnMoving(const SState& c_rkState);
		virtual void OnMove(const SState& c_rkState);
		virtual void OnStop(const SState& c_rkState);
		virtual void OnWarp(const SState& c_rkState);
		virtual void OnClearAffects();
		virtual void OnSetAffect(uint32_t uAffect);
		virtual void OnResetAffect(uint32_t uAffect);
		virtual void OnAttack(const SState& c_rkState, uint16_t wMotionIndex);
		virtual void OnUseSkill(const SState& c_rkState, uint32_t uMotSkill, uint32_t uArg);
		virtual void OnUpdate();
		virtual void OnChangeShape();
		virtual void OnHit(uint32_t uSkill, CActorInstance& rkActorVictim, BOOL isSendPacket);

		void FlushVictimList();

	protected:
		CPythonPlayerEventHandler();

	protected:
		struct SVictim
		{
			uint32_t	m_dwVID;
			int32_t	m_lPixelX;
			int32_t	m_lPixelY;
		};

	protected:
		std::vector<SVictim> m_kVctkVictim;

		uint32_t m_dwPrevComboIndex;
		uint32_t m_dwNextMovingNotifyTime;
		TPixelPosition m_kPPosPrevWaiting;

	private:
		class CNormalBowAttack_FlyEventHandler_AutoClear : public IFlyEventHandler
		{
			public:
				CNormalBowAttack_FlyEventHandler_AutoClear(): m_pParent(nullptr), m_pInstMain(nullptr), m_pInstTarget(nullptr)
				{}
				virtual ~CNormalBowAttack_FlyEventHandler_AutoClear() {}

				void Set(CPythonPlayerEventHandler * pParent, CInstanceBase * pInstMain, CInstanceBase * pInstTarget);
				void SetTarget(CInstanceBase* pInstTarget);

				virtual void OnSetFlyTarget();
				virtual void OnShoot(uint32_t dwSkillIndex);

				virtual void OnNoTarget() { /*Tracenf("Shoot : target이 없습니다.");*/ }
				virtual void OnExplodingOutOfRange() { /*Tracenf("Shoot : 사정거리가 끝났습니다.");*/ }
				virtual void OnExplodingAtBackground() { /*Tracenf("Shoot : 배경에 맞았습니다.");*/ }
				virtual void OnExplodingAtAnotherTarget(uint32_t dwSkillIndex, uint32_t dwVID);
				virtual void OnExplodingAtTarget(uint32_t dwSkillIndex);

			protected:
				CPythonPlayerEventHandler * m_pParent;
				CInstanceBase * m_pInstMain;
				CInstanceBase * m_pInstTarget;
		} m_NormalBowAttack_FlyEventHandler_AutoClear;

		public:
			IFlyEventHandler * GetNormalBowAttackFlyEventHandler(CInstanceBase* pInstMain, CInstanceBase* pInstTarget)
			{
				m_NormalBowAttack_FlyEventHandler_AutoClear.Set(this,pInstMain,pInstTarget);
				return &m_NormalBowAttack_FlyEventHandler_AutoClear;
			}

			void ChangeFlyTarget(CInstanceBase* pInstTarget)
			{
				m_NormalBowAttack_FlyEventHandler_AutoClear.SetTarget(pInstTarget);
			}
};