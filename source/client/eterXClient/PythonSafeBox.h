#pragma once

class CPythonSafeBox : public CSingleton<CPythonSafeBox>
{
	public:
		enum
		{
			SAFEBOX_SLOT_X_COUNT = 5,
			SAFEBOX_SLOT_Y_COUNT = 9,
			SAFEBOX_PAGE_SIZE = SAFEBOX_SLOT_X_COUNT * SAFEBOX_SLOT_Y_COUNT
		};
		using TItemInstanceVector = std::vector<TItemData>;

	public:
		CPythonSafeBox();
		virtual ~CPythonSafeBox();

		void OpenSafeBox(int32_t iSize);
		void SetItemData(uint32_t dwSlotIndex, const TItemData & rItemData);
		void DelItemData(uint32_t dwSlotIndex);

		void SetMoney(uint32_t dwMoney);
		uint32_t GetMoney();
		
		BOOL GetSlotItemID(uint32_t dwSlotIndex, uint32_t* pdwItemID);

		int32_t GetCurrentSafeBoxSize();
		BOOL GetItemDataPtr(uint32_t dwSlotIndex, TItemData ** ppInstance);

		// MALL
		void OpenMall(int32_t iSize);
		void SetMallItemData(uint32_t dwSlotIndex, const TItemData & rItemData);
		void DelMallItemData(uint32_t dwSlotIndex);
		BOOL GetMallItemDataPtr(uint32_t dwSlotIndex, TItemData ** ppInstance);
		BOOL GetSlotMallItemID(uint32_t dwSlotIndex, uint32_t * pdwItemID);
		uint32_t GetMallSize();

	protected:
		TItemInstanceVector m_ItemInstanceVector;
		TItemInstanceVector m_MallItemInstanceVector;
		uint32_t m_dwMoney;
};