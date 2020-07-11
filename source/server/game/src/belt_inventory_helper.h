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
			0,			// ��Ʈ+0
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

	// ���� ��Ʈ ������ ��������, � ������ �̿��� �� �ִ��� ����
	static const TGradeUnit* GetAvailableRuleTableByGrade()
	{
		/**
			��Ʈ�� �� +0 ~ +9 ������ ���� �� ������, ������ ���� 7�ܰ� ������� ���еǾ� �κ��丮�� Ȱ�� ȭ �ȴ�.
			��Ʈ ������ ���� ��� ������ ���� �Ʒ� �׸��� ����. ���� ��� >= Ȱ������ ����̸� ��� ����.
			(��, ���� ������ 0�̸� ������ ��� �Ұ�, ��ȣ ���� ���ڴ� ���)
			
				2(1)  4(2)  6(4)  8(6)
				5(3)  5(3)  6(4)  8(6)
				7(5)  7(5)  7(5)  8(6)
				9(7)  9(7)  9(7)  9(7)

			��Ʈ �κ��丮�� ũ��� 4x4 (16ĭ)
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
		// ��ȹ �� �ٲ�.. �Ƴ�...
		//const TGradeUnit beltGrade = GetBeltGradeByRefineLevel(beltLevel);		
		const TGradeUnit* ruleTable = GetAvailableRuleTableByGrade();

		return ruleTable[cell] <= beltGrade;
	}

	/// pc�� ��Ʈ �κ��丮�� �������� �ϳ��� �����ϴ� �� �˻��ϴ� �Լ�.
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

	/// pc�� ��Ʈ �κ��丮�� �������� �ϳ��� �����ϴ� �� �˻��ϴ� �Լ�.
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

	/// item�� ��Ʈ �κ��丮�� �� �� �ִ� Ÿ������ �˻��ϴ� �Լ�. (�� ��Ģ�� ��ȹ�ڰ� ������)
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
