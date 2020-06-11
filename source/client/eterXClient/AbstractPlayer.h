#pragma once

#include "AbstractSingleton.h"

class CInstanceBase;

class IAbstractPlayer : public TAbstractSingleton<IAbstractPlayer>
{
	public:
		IAbstractPlayer() {}
		virtual ~IAbstractPlayer() {}

		virtual uint32_t	GetMainCharacterIndex() = 0;
		virtual void	SetMainCharacterIndex(int32_t iIndex) = 0;
		virtual bool	IsMainCharacterIndex(uint32_t dwIndex) = 0;

		virtual int32_t		GetStatus(uint32_t dwType) = 0;

		virtual const char *	GetName() = 0;

		virtual void	SetRace(uint32_t dwRace) = 0;

		virtual void	StartStaminaConsume(uint32_t dwConsumePerSec, uint32_t dwCurrentStamina) = 0;
		virtual void	StopStaminaConsume(uint32_t dwCurrentStamina) = 0;		

		virtual bool	IsPartyMemberByVID(uint32_t dwVID) = 0;		
		virtual bool	PartyMemberVIDToPID(uint32_t dwVID, uint32_t * pdwPID) = 0;
		virtual bool	IsSamePartyMember(uint32_t dwVID1, uint32_t dwVID2) = 0;

		virtual void	SetItemData(TItemPos itemPos, const TItemData & c_rkItemInst) = 0;
		virtual void	SetItemCount(TItemPos itemPos, uint8_t byCount) = 0;
		virtual void	SetItemMetinSocket(TItemPos itemPos, uint32_t dwMetinSocketIndex, uint32_t dwMetinNumber) = 0;
		virtual void	SetItemAttribute(TItemPos itemPos, uint32_t dwAttrIndex, uint8_t byType, int16_t sValue) = 0;

		virtual uint32_t	GetItemIndex(TItemPos itemPos) = 0;
		virtual uint32_t	GetItemFlags(TItemPos itemPos) = 0;
		virtual uint32_t	GetItemCount(TItemPos itemPos) = 0;

		virtual bool	IsEquipItemInSlot(TItemPos itemPos) = 0;

		virtual void	AddQuickSlot(int32_t QuickslotIndex, char IconType, char IconPosition) = 0;
		virtual void	DeleteQuickSlot(int32_t QuickslotIndex) = 0;
		virtual void	MoveQuickSlot(int32_t Source, int32_t Target) = 0;

		virtual void	SetWeaponPower(uint32_t dwMinPower, uint32_t dwMaxPower, uint32_t dwMinMagicPower, uint32_t dwMaxMagicPower, uint32_t dwAddPower) = 0;

		virtual void	SetTarget(uint32_t dwVID, BOOL bForceChange = TRUE) = 0;
		virtual void	NotifyCharacterUpdate(uint32_t dwVID) = 0;		
		virtual void	NotifyCharacterDead(uint32_t dwVID) = 0;
		virtual void	NotifyDeletingCharacterInstance(uint32_t dwVID) = 0;
		virtual void	NotifyChangePKMode() = 0;
				
		virtual void	SetObserverMode(bool isEnable) = 0;
		virtual void	SetComboSkillFlag(BOOL bFlag) = 0;

		virtual void	StartEmotionProcess() = 0;
		virtual void	EndEmotionProcess() = 0;

		virtual CInstanceBase* NEW_GetMainActorPtr() = 0;
};

