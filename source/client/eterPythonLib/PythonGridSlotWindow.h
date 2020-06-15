#pragma once

#include "PythonSlotWindow.h"

namespace UI 
{
	class CGridSlotWindow : public CSlotWindow
	{
		public:
			static uint32_t Type();

		public:
			CGridSlotWindow(PyObject * ppyObject);
			virtual ~CGridSlotWindow();

			void Destroy();

			void ArrangeGridSlot(uint32_t dwStartIndex, uint32_t dwxCount, uint32_t dwyCount, int32_t ixSlotSize, int32_t iySlotSize, int32_t ixTemporarySize, int32_t iyTemporarySize);

		protected:
			void __Initialize();

			bool GetPickedSlotPointer(TSlot ** ppSlot);
			bool GetPickedSlotList(int32_t iWidth, int32_t iHeight, std::list<TSlot*> * pSlotPointerList);
			bool GetGridSlotPointer(int32_t ix, int32_t iy, TSlot ** ppSlot);
			bool GetSlotPointerByNumber (uint32_t dwSlotNumber, TSlot** ppSlot);
			bool GetPickedGridSlotPosition(int32_t ixLocal, int32_t iyLocal, int32_t * pix, int32_t * piy);
			bool CheckMoving(uint32_t dwSlotNumber, uint32_t dwItemIndex, const std::list<TSlot*> & c_rSlotList);
			bool CheckSwapping (uint32_t dwSlotNumber, uint32_t dwItemIndex, const std::list<TSlot*>& c_rSlotList);

			bool OnIsType(uint32_t dwType);

			void OnRefreshSlot();
			void OnRenderPickingSlot();

		protected:
			uint32_t m_dwxCount;
			uint32_t m_dwyCount;

			std::vector<TSlot *> m_SlotVector;
	};
};
