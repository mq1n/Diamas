#include "StdAfx.h"
#include "PythonPlayer.h"
#include "PythonTextTail.h"
#include "PythonNetworkStream.h"
#include "InstanceBase.h"
#include "PythonChat.h"
#include "PythonGuild.h"
#include "../eterGameLib/ItemManager.h"

void CPythonPlayer::ClearAffects()
{
	PyCallClassMemberFunc(m_ppyGameWindow, "ClearAffects", Py_BuildValue("()"));
}

void CPythonPlayer::SetAffect(uint32_t uAffect)
{
	PyCallClassMemberFunc(m_ppyGameWindow, "SetAffect", Py_BuildValue("(i)", uAffect));

	/////

	uint32_t dwSkillIndex;
	if (!AffectIndexToSkillIndex(uAffect, &dwSkillIndex))
		return;

	CPythonSkill::TSkillData * pSkillData;
	if (!CPythonSkill::Instance().GetSkillData(dwSkillIndex, &pSkillData))
		return;

	if (!pSkillData->IsToggleSkill())
		return;

	uint32_t dwSkillSlotIndex;
	if (!GetSkillSlotIndex(dwSkillIndex, &dwSkillSlotIndex))
		return;

	__ActivateSkillSlot(dwSkillSlotIndex);
}

void CPythonPlayer::ResetAffect(uint32_t uAffect)
{
	// 2004.07.17.myevan.��ų �ƴ� ����Ʈ�� �� ������� ���� 
	PyCallClassMemberFunc(m_ppyGameWindow, "ResetAffect", Py_BuildValue("(i)", uAffect));

	uint32_t dwSkillIndex;
	if (!AffectIndexToSkillIndex(uAffect, &dwSkillIndex))
		return;

	CPythonSkill::TSkillData * pSkillData;
	if (!CPythonSkill::Instance().GetSkillData(dwSkillIndex, &pSkillData))
		return;

	if (!pSkillData->IsToggleSkill())
		return;

	uint32_t dwSkillSlotIndex;
	if (!GetSkillSlotIndex(dwSkillIndex, &dwSkillSlotIndex))
		return;

	__DeactivateSkillSlot(dwSkillSlotIndex);
}

bool CPythonPlayer::FindSkillSlotIndexBySkillIndex(uint32_t dwSkillIndex, uint32_t * pdwSkillSlotIndex)
{
	for (int32_t i = 0; i < SKILL_MAX_NUM; ++i)
	{
		TSkillInstance & rkSkillInst = m_playerStatus.aSkill[i];
		if (dwSkillIndex == rkSkillInst.dwIndex)
		{
			*pdwSkillSlotIndex = i;
			return true;
		}
	}

	return false;
}

void CPythonPlayer::ChangeCurrentSkillNumberOnly(uint32_t dwSlotIndex)
{
	if (dwSlotIndex >= SKILL_MAX_NUM)
		return;

	TSkillInstance & rkSkillInst = m_playerStatus.aSkill[dwSlotIndex];

	CPythonSkill::TSkillData * pSkillData;
	if (!CPythonSkill::Instance().GetSkillData(rkSkillInst.dwIndex, &pSkillData))
		return;

	if (!pSkillData->IsCanUseSkill())
		return;

	if (!__IsRightButtonSkillMode())
	{
		if (!__IsTarget())
		{
			PyCallClassMemberFunc(m_ppyGameWindow, "OnCannotUseSkill", Py_BuildValue("(is)", GetMainCharacterIndex(), "NEED_TARGET"));
			return;
		}

		ClickSkillSlot(dwSlotIndex);
	}
	else
	{
		m_dwcurSkillSlotIndex = dwSlotIndex;
		PyCallClassMemberFunc(m_ppyGameWindow, "ChangeCurrentSkill", Py_BuildValue("(i)", dwSlotIndex));
	}
}

void CPythonPlayer::ClickSkillSlot(uint32_t dwSlotIndex)
{
	if (dwSlotIndex >= SKILL_MAX_NUM)
		return;

	TSkillInstance & rkSkillInst = m_playerStatus.aSkill[dwSlotIndex];

	CPythonSkill::TSkillData * pSkillData;
	if (!CPythonSkill::Instance().GetSkillData(rkSkillInst.dwIndex, &pSkillData))
		return;

	if (CPythonSkill::SKILL_TYPE_GUILD == pSkillData->byType)
	{
		UseGuildSkill(dwSlotIndex);
		return;
	}

	if (!pSkillData->IsCanUseSkill())
		return;

	if (pSkillData->IsStandingSkill())
	{
		if (pSkillData->IsToggleSkill())
		{
			if (IsSkillActive(dwSlotIndex))
			{
				CInstanceBase * pkInstMain = NEW_GetMainActorPtr();
				if (!pkInstMain)
					return;
				if (pkInstMain->IsUsingSkill())
					return;

				CPythonNetworkStream::Instance().SendUseSkillPacket(rkSkillInst.dwIndex);
				return;
			}
		}

		__UseSkill(dwSlotIndex);
		return;
	}

	if (m_dwcurSkillSlotIndex == dwSlotIndex)
	{
		__UseSkill(m_dwcurSkillSlotIndex);
		return;
	}

	if (!__IsRightButtonSkillMode())
	{
		__UseSkill(dwSlotIndex);
	}
	else
	{
		m_dwcurSkillSlotIndex = dwSlotIndex;
		PyCallClassMemberFunc(m_ppyGameWindow, "ChangeCurrentSkill", Py_BuildValue("(i)", dwSlotIndex));
	}
}

bool CPythonPlayer::__CheckSkillUsable(uint32_t dwSlotIndex)
{
	CInstanceBase * pkInstMain = NEW_GetMainActorPtr();
	if (!pkInstMain)
		return false;

	if (dwSlotIndex >= SKILL_MAX_NUM)
		return false;

	TSkillInstance & rkSkillInst = m_playerStatus.aSkill[dwSlotIndex];

	CPythonSkill::TSkillData * pSkillData;
	if (!CPythonSkill::Instance().GetSkillData(rkSkillInst.dwIndex, &pSkillData))
		return false;

	if (pkInstMain->IsMountingHorse())
	{
		if (!pSkillData->IsHorseSkill())
		{
			PyCallClassMemberFunc(m_ppyGameWindow, "OnCannotUseSkill", Py_BuildValue("(is)", GetMainCharacterIndex(), "NOT_HORSE_SKILL"));
			return false;
		}
	}

	if (pSkillData->IsHorseSkill())
	{
		if (!pkInstMain->IsMountingHorse())
		{
			PyCallClassMemberFunc(m_ppyGameWindow, "OnCannotUseSkill", Py_BuildValue("(is)", GetMainCharacterIndex(), "HAVE_TO_RIDE"));
			return false;
		}
	}

	// 2004.07.26.levites - �������뿡�� ���� ���ϵ��� ����
	// NOTE : ���� ��ų�� �������뿡�� ������� ���մϴ� - [levites]
	if (pSkillData->IsAttackSkill())
	{
		if (pkInstMain->IsInSafe())
		{
			PyCallClassMemberFunc(m_ppyGameWindow, "OnCannotUseSkill", Py_BuildValue("(is)", GetMainCharacterIndex(), "IN_SAFE"));

			return false;
		}
	}	

	// NOTE : �нú� ��ų�� ������� ���մϴ� - [levites]
	if (!pSkillData->IsCanUseSkill())
		return false;
//	if (CPythonSkill::SKILL_TYPE_PASSIVE == pSkillData->byType)
//		return false;

	// NOTE : [Only Assassin] ���� �ִ��� üũ �մϴ�.
	if (pSkillData->IsNeedEmptyBottle())
	{
		if (!__HasItem(27995))
		{
			PyCallClassMemberFunc(m_ppyGameWindow, "OnCannotUseSkill", Py_BuildValue("(is)", GetMainCharacterIndex(), "NEED_EMPTY_BOTTLE"));
			return false;
		}
	}

	// NOTE : [Only Assassin] ������ �ִ��� üũ �մϴ�.
	if (pSkillData->IsNeedPoisonBottle())
	{
		if (!__HasItem(27996))
		{
			PyCallClassMemberFunc(m_ppyGameWindow, "OnCannotUseSkill", Py_BuildValue("(is)", GetMainCharacterIndex(), "NEED_POISON_BOTTLE"));
			return false;
		}
	}

	// NOTE : ���� ���϶��� ��ų�� ������� ���մϴ�.
	if (pkInstMain->IsFishingMode())
	{
		PyCallClassMemberFunc(m_ppyGameWindow, "OnCannotUseSkill", Py_BuildValue("(is)", GetMainCharacterIndex(), "REMOVE_FISHING_ROD"));
		return false;
	}

	// NOTE : ���� üũ
	if (m_sysIsLevelLimit)
	{
		if (rkSkillInst.iLevel <= 0)
		{
			PyCallClassMemberFunc(m_ppyGameWindow, "OnCannotUseSkill", Py_BuildValue("(is)", GetMainCharacterIndex(), "NOT_YET_LEARN"));
			return false;
		}
	}

	// NOTE : ��� �ִ� ���� üũ
	if (!pSkillData->CanUseWeaponType(pkInstMain->GetWeaponType()))
	{
		PyCallClassMemberFunc(m_ppyGameWindow, "OnCannotUseSkill", Py_BuildValue("(is)", GetMainCharacterIndex(), "NOT_MATCHABLE_WEAPON"));
		return false;
	}

	if (!pSkillData->IsHorseSkill()) // HORSE ��ų �߿� ȭ���� ���� �ʴ� ��ų�� �ֱ� ������
	{
		if (__CheckShortArrow(rkSkillInst, *pSkillData))
			return false;

		// NOTE : Ȱ�� �ʿ��� ��� ȭ�� ���� üũ
		if (pSkillData->IsNeedBow())
		{
			if (!__HasEnoughArrow())
				return false;
		}
	}

	if (__CheckDashAffect(*pkInstMain))
	{
		if (!pSkillData->IsChargeSkill())
		{
			if (__CheckRestSkillCoolTime(dwSlotIndex))
			{
				PyCallClassMemberFunc(m_ppyGameWindow, "OnCannotUseSkill", Py_BuildValue("(is)", GetMainCharacterIndex(), "WAIT_COOLTIME"));
				return false;
			}
		}
	}
	else
	{
		if (__CheckRestSkillCoolTime(dwSlotIndex))
		{
			PyCallClassMemberFunc(m_ppyGameWindow, "OnCannotUseSkill", Py_BuildValue("(is)", GetMainCharacterIndex(), "WAIT_COOLTIME"));
			return false;
		}

		if (__CheckShortLife(rkSkillInst, *pSkillData))
		{
			PyCallClassMemberFunc(m_ppyGameWindow, "OnCannotUseSkill", Py_BuildValue("(is)", GetMainCharacterIndex(), "NOT_ENOUGH_HP"));
			return false;
		}

		if (__CheckShortMana(rkSkillInst, *pSkillData))
		{
			PyCallClassMemberFunc(m_ppyGameWindow, "OnCannotUseSkill", Py_BuildValue("(is)", GetMainCharacterIndex(), "NOT_ENOUGH_SP"));
			return false;
		}
	}

	return true;
}

bool CPythonPlayer::__CheckShortArrow(TSkillInstance & rkSkillInst, CPythonSkill::TSkillData& rkSkillData)
{
	if (!rkSkillData.IsNeedBow())
		return false;

	if (__HasEnoughArrow())
		return false;
	
	return true;
}

bool CPythonPlayer::__CheckShortMana(TSkillInstance& rkSkillInst, CPythonSkill::TSkillData& rkSkillData)
{
	extern const uint32_t c_iSkillIndex_Summon;
	if (c_iSkillIndex_Summon == rkSkillInst.dwIndex)
		return false;

	int32_t iNeedSP = rkSkillData.GetNeedSP(rkSkillInst.fcurEfficientPercentage);
	int32_t icurSP = GetStatus(POINT_SP);

	// NOTE : ToggleSkill �� �ƴѵ� �Ҹ� SP �� 0 �̴�.
	if (!rkSkillData.IsToggleSkill())
	{
		if (iNeedSP == 0)
		{
			CPythonChat::Instance().AppendChat(CHAT_TYPE_INFO, "!!! Find strange game data. Please reinstall metin2.");
			return true;
		}
	}

	if (rkSkillData.CanUseIfNotEnough())
	{
		if (icurSP <= 0)
			return true;
	}
	else
	{
		if (-1 != iNeedSP)
			if (iNeedSP > icurSP)
				return true;
	}

	return false;
}

bool CPythonPlayer::__CheckShortLife(TSkillInstance& rkSkillInst, CPythonSkill::TSkillData& rkSkillData)
{
	if (!rkSkillData.IsUseHPSkill())
		return false;
	
	uint32_t dwNeedHP = rkSkillData.GetNeedSP(rkSkillInst.fcurEfficientPercentage);
	if (dwNeedHP <= GetStatus(POINT_HP))
		return false;

	return true;
}

bool CPythonPlayer::__CheckRestSkillCoolTime(uint32_t dwSlotIndex)
{
	if (!m_sysIsCoolTime)
		return false;

	if (dwSlotIndex >= SKILL_MAX_NUM)
		return false;

	float fElapsedTime = CTimer::Instance().GetCurrentSecond() - m_playerStatus.aSkill[dwSlotIndex].fLastUsedTime;
	if (fElapsedTime >= m_playerStatus.aSkill[dwSlotIndex].fCoolTime)
		return false;	

	return true;
}

bool CPythonPlayer::__CheckDashAffect(CInstanceBase& rkInstMain)
{
	return rkInstMain.IsAffect(CInstanceBase::AFFECT_DASH);
}

void CPythonPlayer::__UseCurrentSkill()
{
	__UseSkill(m_dwcurSkillSlotIndex);
}

uint32_t CPythonPlayer::__GetSkillTargetRange(CPythonSkill::TSkillData& rkSkillData)
{
	return rkSkillData.GetTargetRange() + GetStatus(POINT_BOW_DISTANCE)*100;
}

bool CPythonPlayer::__ProcessEnemySkillTargetRange(CInstanceBase& rkInstMain, CInstanceBase& rkInstTarget, CPythonSkill::TSkillData& rkSkillData, uint32_t dwSkillSlotIndex)
{
	uint32_t dwSkillTargetRange=__GetSkillTargetRange(rkSkillData);
	auto fSkillTargetRange = float(dwSkillTargetRange);
	if (fSkillTargetRange <= 0.0f)
		return true;

	// #0000806: [M2EU] ���濡�� ����(���ѱ�) źȯ�� ��ų ��� �ȵ�	
	float fTargetDistance = rkInstMain.GetDistance(&rkInstTarget);

	extern bool IS_HUGE_RACE(uint32_t vnum);
	if (IS_HUGE_RACE(rkInstTarget.GetRace()))
	{
		fTargetDistance -= 200.0f; // TEMP: �ϴ� �ϵ� �ڵ� ó��. ���������δ� �ٿ�� ���Ǿ ����ؾ���
	}

	if (fTargetDistance >= fSkillTargetRange)
	{
		if (rkSkillData.IsChargeSkill())
		{
			if (!__IsReservedUseSkill(dwSkillSlotIndex))
				__SendUseSkill(dwSkillSlotIndex, 0);
		}

		__ReserveUseSkill(rkInstTarget.GetVirtualID(), dwSkillSlotIndex, dwSkillTargetRange);

		return false;
	}

	// 2004.07.05.myevan. �ý�ź�� ���� �ʿ� ����. ����ϱ��� ���� �ִ°� üũ
	TPixelPosition kPPosTarget;
	rkInstTarget.NEW_GetPixelPosition(&kPPosTarget);

	IBackground& rkBG=IBackground::Instance();
	if (rkBG.IsBlock(kPPosTarget.x, kPPosTarget.y))
	{
		PyCallClassMemberFunc(m_ppyGameWindow, "OnCannotUseSkill", Py_BuildValue("(is)", GetMainCharacterIndex(), "CANNOT_APPROACH"));
		return false;
	}

	return true;
}

bool CPythonPlayer::__CanUseSkill()
{
	CInstanceBase* pkInstMain=NEW_GetMainActorPtr();
	if (!pkInstMain)
		return false;

	if (IsObserverMode())
		return false;

	// Fix me
	// ������Ʈ. �¸���ų���� 20 �̸��� ���, ��� ����Ʈ�� Ÿ�� �¸� ���� ��ų �� ������ ���ϵ��� �ϵ� �ڵ�... 
	// ���߿� �ð� ���� can use skill üũ�� �������� ������...
	if (pkInstMain->IsMountingHorse() && (GetSkillGrade(109) < 1 && GetSkillLevel(109) < 20))
		return false;

	return pkInstMain->CanUseSkill();
}


bool CPythonPlayer::__UseSkill(uint32_t dwSlotIndex)
{
	// PrivateShop
	if (IsOpenPrivateShop())
		return true;

	if (!__CanUseSkill())
		return false;

	if (dwSlotIndex >= SKILL_MAX_NUM)
	{
		Tracenf("CPythonPlayer::__UseSkill(dwSlotIndex=%d) It's not available skill slot number", dwSlotIndex);
		return false;
	}

	TSkillInstance & rkSkillInst = m_playerStatus.aSkill[dwSlotIndex];

	if (__CheckSpecialSkill(rkSkillInst.dwIndex))
		return true;

	CPythonSkill::TSkillData * pSkillData;
	if (!CPythonSkill::Instance().GetSkillData(rkSkillInst.dwIndex, &pSkillData))
	{
		Tracenf("CPythonPlayer::__UseSkill(dwSlotIndex=%d) There is no skill data", dwSlotIndex);
		return false;
	}

	if (pSkillData->IsToggleSkill())
	{
		if (IsSkillActive(dwSlotIndex))
		{
			CPythonNetworkStream::Instance().SendUseSkillPacket(rkSkillInst.dwIndex, dwSlotIndex);
			return false;
		}
	}

	if (!__CheckSkillUsable(dwSlotIndex))
		return false;

	CInstanceBase * pkInstMain = NEW_GetMainActorPtr();
	if (!pkInstMain)
	{
		Tracenf("CPythonPlayer::__UseSkill(dwSlotIndex=%d) There is no main player", dwSlotIndex);
		return false;
	}

	if (pkInstMain->IsUsingSkill())
		return false;

	CInstanceBase * pkInstTarget = nullptr;

	// NOTE : Ÿ���� �ʿ��� ���
	if (pSkillData->IsNeedTarget() ||
		pSkillData->CanChangeDirection() ||
		pSkillData->IsAutoSearchTarget())
	{
		if (pSkillData->IsNeedCorpse())
			pkInstTarget=__GetDeadTargetInstancePtr();
		else
			pkInstTarget=__GetAliveTargetInstancePtr();

		// ���� Ÿ���� ������..
		if (!pkInstTarget)
		{
			// ������Ʈ�ϰ�..
			__ChangeTargetToPickedInstance();

			// �ٽ� ����.
			if (pSkillData->IsNeedCorpse())
				pkInstTarget=__GetDeadTargetInstancePtr();
			else
				pkInstTarget=__GetAliveTargetInstancePtr();
		}

		if (pkInstTarget)
		{
			if (pSkillData->IsOnlyForAlliance())
			{
				if (pkInstMain == pkInstTarget)
				{
					if (!pSkillData->CanUseForMe())
					{
						PyCallClassMemberFunc(m_ppyGameWindow, "OnCannotUseSkill", Py_BuildValue("(is)", GetMainCharacterIndex(), "CANNOT_USE_SELF"));
						return false;
					}
				}
				else if (!pkInstMain->IsAttackableInstance(*pkInstTarget) && pkInstTarget->IsPC())
				{
					uint32_t dwSkillRange = __GetSkillTargetRange(*pSkillData);

					if (dwSkillRange > 0)
					{
						float fDistance=pkInstMain->GetDistance(pkInstTarget);
						if (fDistance>=float(dwSkillRange))
						{
							__ReserveUseSkill(pkInstTarget->GetVirtualID(), dwSlotIndex, dwSkillRange);
							return false;
						}
					}
				}
				else
				{
					if (pSkillData->CanUseForMe())
					{
						pkInstTarget = pkInstMain;
						Tracef(" [ALERT] Used on you, and the enemy target is changed to yourself\n");
					}
					else
					{
						PyCallClassMemberFunc(m_ppyGameWindow, "OnCannotUseSkill", Py_BuildValue("(is)", GetMainCharacterIndex(), "ONLY_FOR_ALLIANCE"));
						return false;
					}
				}
			}
			else
			{
				if (pkInstTarget->IsInSafe())
				{
					PyCallClassMemberFunc(m_ppyGameWindow, "OnCannotUseSkill", Py_BuildValue("(is)", GetMainCharacterIndex(), "CANNOT_ATTACK_ENEMY_IN_SAFE_AREA"));
					return false;
				}

				if (pkInstMain->IsAttackableInstance(*pkInstTarget))
				{
					if (!__ProcessEnemySkillTargetRange(*pkInstMain, *pkInstTarget, *pSkillData, dwSlotIndex))
						return false;
				}
				else
				{
					PyCallClassMemberFunc(m_ppyGameWindow, "OnCannotUseSkill", Py_BuildValue("(is)", GetMainCharacterIndex(), "CANNOT_ATTACK"));
					return false;
				}
			}

			pkInstMain->SetFlyTargetInstance(*pkInstTarget);

			if (pkInstMain != pkInstTarget)
			{
				if (pkInstMain->IsFlyTargetObject())
					pkInstMain->NEW_LookAtFlyTarget();
				else
					pkInstMain->NEW_LookAtDestInstance(*pkInstTarget);
			}
		}
		else
		{
			if (pSkillData->IsAutoSearchTarget())
			{
				if (pkInstMain->NEW_GetFrontInstance(&pkInstTarget, 2000.0f))
				{
					SetTarget(pkInstTarget->GetVirtualID());
					if (!__ProcessEnemySkillTargetRange(*pkInstMain, *pkInstTarget, *pSkillData, dwSlotIndex))
						return false;
				}
				else
				{
					PyCallClassMemberFunc(m_ppyGameWindow, "OnCannotUseSkill", Py_BuildValue("(is)", GetMainCharacterIndex(), "NEED_TARGET"));
					return false;
				}
			}

			if (pSkillData->CanUseForMe())
			{
				pkInstTarget = pkInstMain;
				pkInstMain->SetFlyTargetInstance(*pkInstMain);
				Tracef(" [ALERT] The target is set to yourself\n");
			}
			else if (pSkillData->IsNeedCorpse())
			{
				PyCallClassMemberFunc(m_ppyGameWindow, "OnCannotUseSkill", Py_BuildValue("(is)", GetMainCharacterIndex(), "ONLY_FOR_CORPSE"));
				return false;
			}
			else
			{
				PyCallClassMemberFunc(m_ppyGameWindow, "OnCannotUseSkill", Py_BuildValue("(is)", GetMainCharacterIndex(), "NEED_TARGET"));
				return false;
			}
		}
	}

	if (pSkillData->CanChangeDirection())
	{
		uint32_t dwPickedActorID;
		TPixelPosition kPPosPickedGround;

		if (pkInstTarget && pkInstTarget != pkInstMain)
			pkInstMain->NEW_LookAtDestInstance(*pkInstTarget);
		else if (__GetPickedActorID(&dwPickedActorID))
		{
			CInstanceBase * pkInstVictim = NEW_FindActorPtr(dwPickedActorID);
			if (pkInstVictim)
				pkInstMain->NEW_LookAtDestInstance(*pkInstVictim);
		}
		else if (__GetPickedGroundPos(&kPPosPickedGround))
			pkInstMain->NEW_LookAtDestPixelPosition(kPPosPickedGround);
		else
			Tracenf("CPythonPlayer::__UseSkill(%d) - The screen direction settings should be as standard", dwSlotIndex);
	}

	// ���ݼ� ó��
	uint32_t dwTargetMaxCount = pSkillData->GetTargetCount(rkSkillInst.fcurEfficientPercentage);
	uint32_t dwRange = __GetSkillTargetRange(*pSkillData);
	if (dwTargetMaxCount>0 && pkInstTarget)
	{
		uint32_t dwTargetCount=1;
		std::vector<CInstanceBase*> kVct_pkInstTarget;

		if (pSkillData->IsFanRange())
		{
			if (pkInstMain->NEW_GetInstanceVectorInFanRange(float(dwRange), *pkInstTarget, &kVct_pkInstTarget))
			{
				for (auto pkInstEach : kVct_pkInstTarget)
				{
					if (dwTargetCount >= dwTargetMaxCount)
						break;

					if (pkInstTarget != pkInstEach && !pkInstEach->IsDead())
					{
						pkInstMain->AddFlyTargetInstance(*pkInstEach);
						CPythonNetworkStream::Instance().SendAddFlyTargetingPacket(
							pkInstEach->GetVirtualID(), pkInstEach->GetGraphicThingInstanceRef().OnGetFlyTargetPosition());

						dwTargetCount++;
					}
				}
			}
		}
		else if (pSkillData->IsCircleRange())
		{
			if (pkInstMain->NEW_GetInstanceVectorInCircleRange(float(dwRange), &kVct_pkInstTarget))
			{
				for (auto pkInstEach : kVct_pkInstTarget)
				{
					if (dwTargetCount >= dwTargetMaxCount)
						break;

					if (pkInstTarget != pkInstEach && !pkInstEach->IsDead())
					{
						pkInstMain->AddFlyTargetInstance(*pkInstEach);
						CPythonNetworkStream::Instance().SendAddFlyTargetingPacket(
							pkInstEach->GetVirtualID(), pkInstEach->GetGraphicThingInstanceRef().OnGetFlyTargetPosition());

						dwTargetCount++;
					}
				}
			}
		}

		if (dwTargetCount<dwTargetMaxCount)
		{
			while (dwTargetCount<dwTargetMaxCount)
			{
				TPixelPosition kPPosDst;
				pkInstMain->NEW_GetRandomPositionInFanRange(*pkInstTarget, &kPPosDst);

				kPPosDst.x=kPPosDst.x;
				kPPosDst.y=-kPPosDst.y;

				pkInstMain->AddFlyTargetPosition(kPPosDst);
				CPythonNetworkStream::Instance().SendAddFlyTargetingPacket(0, kPPosDst);

				dwTargetCount++;
			}
		}
	}

	/////
	// NOTE : �ָ��� ���� Ŭ���س��� ��ų�� ���� ��ų�� ���� �ٷ� ���� �����ϴ� ������ �����ϱ� ���� �ڵ� - [levites]
	__ClearReservedAction();
	/////

	if (!pSkillData->IsNoMotion())
	{
		uint32_t dwMotionIndex = pSkillData->GetSkillMotionIndex(rkSkillInst.iGrade);
		uint32_t dwLoopCount = pSkillData->GetMotionLoopCount(rkSkillInst.fcurEfficientPercentage);
		if (!pkInstMain->NEW_UseSkill(rkSkillInst.dwIndex, dwMotionIndex, dwLoopCount, pSkillData->IsMovingSkill() ? true : false))
		{
			Tracenf("CPythonPlayer::__UseSkill(%d) - pkInstMain->NEW_UseSkill - ERROR", dwSlotIndex);
			return false;
		}
	}

	uint32_t dwTargetVID=pkInstTarget ? pkInstTarget->GetVirtualID() : 0;

	__SendUseSkill(dwSlotIndex, dwTargetVID);
	return true;
}

void CPythonPlayer::__SendUseSkill(uint32_t dwSkillSlotIndex, uint32_t dwTargetVID)
{
	TSkillInstance & rkSkillInst = m_playerStatus.aSkill[dwSkillSlotIndex];

	CPythonNetworkStream& rkNetStream=CPythonNetworkStream::Instance();
	rkNetStream.SendUseSkillPacket(rkSkillInst.dwIndex, dwTargetVID);

	__RunCoolTime(dwSkillSlotIndex);
}

uint8_t CPythonPlayer::__GetSkillType(uint32_t dwSkillSlotIndex)
{
	TSkillInstance & rkSkillInst = m_playerStatus.aSkill[dwSkillSlotIndex];

	CPythonSkill::TSkillData * pkSkillData;
	CPythonSkill& rkPythonSkill = CPythonSkill::Instance();
	if (!rkPythonSkill.GetSkillData(rkSkillInst.dwIndex, &pkSkillData))
	{
		//TraceError("CPythonPlayer::__GetSkillType(dwSkillSlotIndex=%d) - NOT CHECK", dwSkillSlotIndex);
		return 0;
	}
	return pkSkillData->GetType();
}

void CPythonPlayer::__RunCoolTime(uint32_t dwSkillSlotIndex)
{
	TSkillInstance & rkSkillInst = m_playerStatus.aSkill[dwSkillSlotIndex];

	CPythonSkill::TSkillData * pkSkillData;
	if (!CPythonSkill::Instance().GetSkillData(rkSkillInst.dwIndex, &pkSkillData))
	{
		TraceError("CPythonPlayer::__SendUseSkill(dwSkillSlotIndex=%d) - NOT CHECK", dwSkillSlotIndex);
		return;
	}

	CPythonSkill::TSkillData& rkSkillData=*pkSkillData;

	rkSkillInst.fCoolTime = rkSkillData.GetSkillCoolTime(rkSkillInst.fcurEfficientPercentage);
	rkSkillInst.fLastUsedTime = CTimer::Instance().GetCurrentSecond();

	int32_t iSpd = 100 - GetStatus(POINT_CASTING_SPEED);
	if (iSpd > 0)
		iSpd = 100 + iSpd;
	else if (iSpd < 0)
		iSpd = 10000 / (100 - iSpd);
	else
		iSpd = 100;

	rkSkillInst.fCoolTime = rkSkillInst.fCoolTime * iSpd / 100;

	PyCallClassMemberFunc(m_ppyGameWindow, "RunUseSkillEvent", Py_BuildValue("(if)", dwSkillSlotIndex, rkSkillInst.fCoolTime));
}

bool CPythonPlayer::__HasEnoughArrow()
{
	CItemData * pItemData;
	if (CItemManager::Instance().GetItemDataPointer(GetItemIndex(TItemPos(INVENTORY, c_Equipment_Arrow)), &pItemData))
	{
		if (ITEM_TYPE_WEAPON == pItemData->GetType())
		{
			if (WEAPON_ARROW == pItemData->GetSubType())
				return true;
		}
	}

	PyCallClassMemberFunc(m_ppyGameWindow, "OnCannotShotError", Py_BuildValue("(is)", GetMainCharacterIndex(), "EMPTY_ARROW"));
	return false;
}

bool CPythonPlayer::__HasItem(uint32_t dwItemID)
{
	for (int32_t i = 0; i < c_Inventory_Count; ++i)
	{
		if (dwItemID == GetItemIndex(TItemPos (INVENTORY, i)))
			return true;
	}
	for (int32_t i = 0; i < c_DragonSoul_Inventory_Count; ++i)
	{
		if (dwItemID == GetItemIndex(TItemPos (DRAGON_SOUL_INVENTORY, i)))
			return true;
	}

	return false;
}

extern const uint32_t c_iSkillIndex_Tongsol;
extern const uint32_t c_iSkillIndex_Fishing;
extern const uint32_t c_iSkillIndex_Mining;
extern const uint32_t c_iSkillIndex_Making;
extern const uint32_t c_iSkillIndex_Combo;
extern const uint32_t c_iSkillIndex_Language1;
extern const uint32_t c_iSkillIndex_Language2;
extern const uint32_t c_iSkillIndex_Language3;
extern const uint32_t c_iSkillIndex_Polymorph;

void CPythonPlayer::UseGuildSkill(uint32_t dwSkillSlotIndex)
{
	CInstanceBase* pkInstMain = NEW_GetMainActorPtr();
	if (!pkInstMain)
		return;
	if (!pkInstMain->CanUseSkill())
		return;

	///////////////////////////////////////////////////////////////////////////////////////////////

	if (dwSkillSlotIndex >= SKILL_MAX_NUM)
	{
		Tracenf("CPythonPlayer::UseGuildSkill(dwSkillSlotIndex=%d) It's not available skill slot number", dwSkillSlotIndex);
		return;
	}

	TSkillInstance & rkSkillInst = m_playerStatus.aSkill[dwSkillSlotIndex];

	uint32_t dwSkillIndex = rkSkillInst.dwIndex;

	CPythonSkill::TSkillData * pSkillData;
	if (!CPythonSkill::Instance().GetSkillData(dwSkillIndex, &pSkillData))
		return;

	if (__CheckRestSkillCoolTime(dwSkillSlotIndex))
	{
		PyCallClassMemberFunc(m_ppyGameWindow, "OnCannotUseSkill", Py_BuildValue("(is)", GetMainCharacterIndex(), "WAIT_COOLTIME"));
		return;
	}

	if (pSkillData->IsOnlyForGuildWar())
	{
		if (!CPythonGuild::Instance().IsDoingGuildWar())
		{
			PyCallClassMemberFunc(m_ppyGameWindow, "OnCannotUseSkill", Py_BuildValue("(is)", GetMainCharacterIndex(), "ONLY_FOR_GUILD_WAR"));
			return;
		}
	}

	uint32_t dwMotionIndex = pSkillData->GetSkillMotionIndex();
	if (!pkInstMain->NEW_UseSkill(dwSkillIndex, dwMotionIndex, 1, false))
		Tracenf("CPythonPlayer::UseGuildSkill(%d) - pkInstMain->NEW_UseSkill - ERROR", dwSkillIndex);

	///////////////////////////////////////////////////////////////////////////////////////////////

	CPythonNetworkStream::Instance().SendGuildUseSkillPacket(dwSkillIndex, 0);
	__RunCoolTime(dwSkillSlotIndex);
}

void CPythonPlayer::SetComboSkillFlag(BOOL bFlag)
{
	uint32_t dwSlotIndex;
	if (!GetSkillSlotIndex(c_iSkillIndex_Combo, &dwSlotIndex))
	{
		Tracef("CPythonPlayer::SetComboSkillFlag(killIndex=%d) - Can't Find Slot Index\n", c_iSkillIndex_Combo);
		return;
	}

	int32_t iLevel = GetSkillLevel(dwSlotIndex);
	if (iLevel <= 0)
	{
		Tracef("CPythonPlayer::SetComboSkillFlag(skillIndex=%d, skillLevel=%d) - Invalid Combo Skill Level\n", c_iSkillIndex_Combo, iLevel);
		return;
	}

	iLevel = std::min(iLevel, 2);

	CInstanceBase* pkInstMain = NEW_GetMainActorPtr();
	if (!pkInstMain)
		return;

	if (bFlag)
	{
		pkInstMain->SetComboType(iLevel);
		__ActivateSkillSlot(dwSlotIndex);
	}
	else
	{
		pkInstMain->SetComboType(0);
		__DeactivateSkillSlot(dwSlotIndex);
	}
}

bool CPythonPlayer::__CheckSpecialSkill(uint32_t dwSkillIndex)
{
	CInstanceBase* pkInstMain = NEW_GetMainActorPtr();
	if (!pkInstMain)
		return false;

	// Fishing
	if (c_iSkillIndex_Fishing == dwSkillIndex)
	{
		if (pkInstMain->IsFishingMode())
			NEW_Fishing();
		else
			PyCallClassMemberFunc(m_ppyGameWindow, "OnCannotUseSkill", Py_BuildValue("(is)", GetMainCharacterIndex(), "EQUIP_FISHING_ROD"));
		return true;
	}

	// Combo
	if (c_iSkillIndex_Combo == dwSkillIndex)
	{
		uint32_t dwSlotIndex;
		if (!GetSkillSlotIndex(dwSkillIndex, &dwSlotIndex))
			return false;

		int32_t iLevel = GetSkillLevel(dwSlotIndex);
		if (iLevel > 0)
			CPythonNetworkStream::Instance().SendUseSkillPacket(dwSkillIndex);
		else
			PyCallClassMemberFunc(m_ppyGameWindow, "OnCannotUseSkill", Py_BuildValue("(is)", GetMainCharacterIndex(), "NOT_YET_LEARN"));

		return true;
	}

	return false;
}
