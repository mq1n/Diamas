#ifndef __HEADER_INVENTORY__
#define __HEADER_INVENTORY__

#include "typedef.h"

class CInventory
{
private:
	LPCHARACTER m_owner;

public:
	enum class EStatus {
		ERROR_UNDEFINED = -1,
		SUCCESS = 0,
		ERROR_NO_SPACE = 1,
	};

public:
	CInventory(LPCHARACTER owner) { m_owner = owner; };

	/**
	* Add
	* @desc	Add an item to the inventory
	* @args	LPITEM &item	Item to add. Passed by reference, might be modified inside the function
	*						if the item is stackable and it is, indeed, stacked.
	* @returns	EStatus	Code with the results of the function.
	*/
	EStatus Add(LPITEM &item);

	/**
	* GetItemAtPos
	* @args	uint16_t wCell	The cell we would like to get the item for
	* @returns	LPITEM	The item in said cell, or nullptr if the cell is out of bounds or empty
	*/
	LPITEM GetItemAtPos(const uint16_t& wCell) const;

	/**
	* FindSameItem
	* @desc	Returns the first item in the inventory that matches our item (vnum & sockets)
	* @args	LPITEM item	The item we will try to find a match for.
	* @args	int32_t fromPos [optional]	The position to start searching from
	* @returns	LPITEM	The match item, if found
	*/
	LPITEM FindSameItem(LPITEM item, int32_t fromPos, int32_t untilPos) const;

#ifdef __ENABLE_EXTEND_INVEN_SYSTEM__
	static int CheckExtendAvailableSlots3(LPCHARACTER pc);
	static int CheckExtendAvailableSlots4(LPCHARACTER pc);
	static int CheckExtendAvailableSlots5(LPCHARACTER pc);
	static bool IsAvailableCell(uint16_t cell, short ExtendSlots);
	static int GetExtendItemNeedCount(LPCHARACTER pc);
#endif

private:
	/**
	* TryStack
	* @desc	Attempts to stack a given item into an already existing item on the player's inventory.
	* @args	int32_t fromInventoryPos	The inventory cell we will search from.
	* @args	int32_t untilInventoryPos	The last inventory cell searched.
	* @args	LPITEM item	The item we're trying to stack.
	* @args	LPITEM match	Passed by reference, it will take the value of the *last* item stacked.
	* @args int32_t bCount	Passed by reference, it will take the value of the final count of the original
	*					item (the one we were trying to stack). If zero, the item was fully stacked.
	*/
	void TryStack(int32_t fromInventoryPos, int32_t untilInventoryPos, LPITEM item, LPITEM &match, uint8_t &bCount);
};

#endif