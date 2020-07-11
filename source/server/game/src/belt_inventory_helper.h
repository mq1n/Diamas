#pragma once
#include "char.h"
#include "item.h"

class CBeltInventoryHelper
{
public:
	typedef uint8_t	TGradeUnit;

	static TGradeUnit GetBeltGradeByRefineLevel(uint32_t level)
	{
		static TGradeUnit beltGradeByLevelTable[] = 
		{
			0,			// 벨트+0
			1,			// +1
			1,			// +2
			2,			// +3
			2,			// +4,
			3,			// +5
			4,			// +6,
			5,			// +7,
			6,			// +8,
			7,			// +9
		};

		if (level >= _countof(beltGradeByLevelTable))
		{
			sys_err("CBeltInventoryHelper::GetBeltGradeByRefineLevel - Overflow level (%u)", level);
			return 0;
		}

		return beltGradeByLevelTable[level];
	}

	// 현재 벨트 레벨을 기준으로, 어떤 셀들을 이용할 수 있는지 리턴
	static const TGradeUnit* GetAvailableRuleTableByGrade()
	{
		/**
			벨트는 총 +0 ~ +9 레벨을 가질 수 있으며, 레벨에 따라 7단계 등급으로 구분되어 인벤토리가 활성 화 된다.
			벨트 레벨에 따른 사용 가능한 셀은 아래 그림과 같음. 현재 등급 >= 활성가능 등급이면 사용 가능.
			(단, 현재 레벨이 0이면 무조건 사용 불가, 괄호 안의 숫자는 등급)
			
				2(1)  4(2)  6(4)  8(6)
				5(3)  5(3)  6(4)  8(6)
				7(5)  7(5)  7(5)  8(6)
				9(7)  9(7)  9(7)  9(7)

			벨트 인벤토리의 크기는 4x4 (16칸)
		*/

		static TGradeUnit availableRuleByGrade[BELT_INVENTORY_SLOT_COUNT] = {
			1, 2, 4, 6,
			3, 3, 4, 6,
			5, 5, 5, 6,
			7, 7, 7, 7
		};

		return availableRuleByGrade;
	}

	static bool IsAvailableCell(uint16_t cell, int32_t beltGrade /*int32_t beltLevel*/)
	{
		// 기획 또 바뀜.. 아놔...
		//const TGradeUnit beltGrade = GetBeltGradeByRefineLevel(beltLevel);		
		const TGradeUnit* ruleTable = GetAvailableRuleTableByGrade();

		return ruleTable[cell] <= beltGrade;
	}

	/// pc의 벨트 인벤토리에 아이템이 하나라도 존재하는 지 검사하는 함수.
	static bool IsExistItemInBeltInventory(LPCHARACTER pc)
	{
		for (uint16_t i = BELT_INVENTORY_SLOT_START; i < BELT_INVENTORY_SLOT_END; ++i)
		{
			LPITEM beltInventoryItem = pc->GetInventoryItem(i);

			if (nullptr != beltInventoryItem)
				return true;
		}

		return false;
	}
	static int32_t GetItemCount(LPCHARACTER pc)
	{
		uint32_t count = 0;
		for (uint16_t i = BELT_INVENTORY_SLOT_START; i < BELT_INVENTORY_SLOT_END; ++i)
		{
			LPITEM beltInventoryItem = pc->GetInventoryItem(i);
			if (beltInventoryItem)
				count++;
		}
		return count;
	}

	/// pc의 벨트 인벤토리에 아이템이 하나라도 존재하는 지 검사하는 함수.
	static bool ClearBelt(LPCHARACTER pc)
	{
		int32_t EmptySlotCount = pc->GetEmptyInventoryCount(1);
		int32_t BeltItemCount = GetItemCount(pc);

		if (EmptySlotCount - 1 < BeltItemCount)
		{
			pc->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("You have not enough empty belt inventory space"));
			return false;
		}

		for (uint16_t i = BELT_INVENTORY_SLOT_START; i < BELT_INVENTORY_SLOT_END; ++i)
		{
			LPITEM beltInventoryItem = pc->GetInventoryItem(i);

			if (beltInventoryItem)
			{
				int32_t NewPos = pc->GetEmptyInventory(1);
				if (NewPos != -1)
				{
					beltInventoryItem->RemoveFromCharacter();
					beltInventoryItem->AddToCharacter(pc, TItemPos(INVENTORY, NewPos));
				}
				else
				{
					pc->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("You have not enough empty belt inventory space"));
					return false;
				}
			}
		}
		return true;
	}

	/// item이 벨트 인벤토리에 들어갈 수 있는 타입인지 검사하는 함수. (이 규칙은 기획자가 결정함)
	static bool CanMoveIntoBeltInventory(LPITEM item)
	{
		/*
		if (item->GetType() == ITEM_LOTTERY && item->GetSubType() == LOTTERY_TICKET)
			return true;

		if (item->GetType() == ITEM_BLEND)
			return true;
		*/

		if (item->GetType() == ITEM_USE)
		{
			switch (item->GetSubType())
			{
			case USE_POTION:
			case USE_POTION_NODELAY:
			case USE_ABILITY_UP:
			case USE_AFFECT:
			case USE_BAIT:
			case USE_TALISMAN:
				return true;
				break;
			}
		}

		return false;
	}

};
