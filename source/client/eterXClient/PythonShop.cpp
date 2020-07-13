#include "StdAfx.h"
#include "PythonShop.h"
#include "PythonNetworkStream.h"
#include "PythonDynamicModuleNames.h"
#include "../eterSecurity/PythonStackCheck.h"

//BOOL CPythonShop::GetSlotItemID(uint32_t dwSlotPos, uint32_t* pdwItemID)
//{
//	if (!CheckSlotIndex(dwSlotPos))
//		return FALSE;
//	const TShopItemData * itemData;
//	if (!GetItemData(dwSlotPos, &itemData))
//		return FALSE;
//	*pdwItemID=itemData->vnum;
//	return TRUE;
//}
void CPythonShop::SetTabCoinType(uint8_t tabIdx, uint8_t coinType)
{
	if (tabIdx >= m_bTabCount)
	{	
		TraceError("Out of Index. tabIdx(%d) must be less than %d.", tabIdx, SHOP_TAB_COUNT_MAX);
		return;
	}
	m_aShoptabs[tabIdx].coinType = coinType;
}

uint8_t CPythonShop::GetTabCoinType(uint8_t tabIdx)
{
	if (tabIdx >= m_bTabCount)
	{
		TraceError("Out of Index. tabIdx(%d) must be less than %d.", tabIdx, SHOP_TAB_COUNT_MAX);
		return 0xff;
	}
	return m_aShoptabs[tabIdx].coinType;
}

void CPythonShop::SetTabName(uint8_t tabIdx, const char* name)
{
	if (tabIdx >= m_bTabCount)
	{	
		TraceError("Out of Index. tabIdx(%d) must be less than %d.", tabIdx, SHOP_TAB_COUNT_MAX);
		return;
	}
	m_aShoptabs[tabIdx].name = name;
}

const char* CPythonShop::GetTabName(uint8_t tabIdx)
{
	if (tabIdx >= m_bTabCount)
	{
		TraceError("Out of Index. tabIdx(%d) must be less than %d.", tabIdx, SHOP_TAB_COUNT_MAX);
		return nullptr;
	}

	return m_aShoptabs[tabIdx].name.c_str();
}

void CPythonShop::SetItemData(uint32_t dwIndex, const TShopItemData & c_rShopItemData)
{
	uint8_t tabIdx = dwIndex / SHOP_HOST_ITEM_MAX_NUM;
	uint32_t dwSlotPos = dwIndex % SHOP_HOST_ITEM_MAX_NUM;
	
	SetItemData(tabIdx, dwSlotPos, c_rShopItemData);
}

BOOL CPythonShop::GetItemData(uint32_t dwIndex, const TShopItemData ** c_ppItemData)
{
	uint8_t tabIdx = dwIndex / SHOP_HOST_ITEM_MAX_NUM;
	uint32_t dwSlotPos = dwIndex % SHOP_HOST_ITEM_MAX_NUM;

	return GetItemData(tabIdx, dwSlotPos, c_ppItemData);
}

void CPythonShop::SetItemData(uint8_t tabIdx, uint32_t dwSlotPos, const TShopItemData & c_rShopItemData)
{
	if (tabIdx >= SHOP_TAB_COUNT_MAX || dwSlotPos >= SHOP_HOST_ITEM_MAX_NUM)
	{
		TraceError("Out of Index. tabIdx(%d) must be less than %d. dwSlotPos(%d) must be less than %d", tabIdx, SHOP_TAB_COUNT_MAX, dwSlotPos, SHOP_HOST_ITEM_MAX_NUM);
		return;
	}

	m_aShoptabs[tabIdx].items[dwSlotPos] = c_rShopItemData;
}

BOOL CPythonShop::GetItemData(uint8_t tabIdx, uint32_t dwSlotPos, const TShopItemData ** c_ppItemData)
{
	if (tabIdx >= SHOP_TAB_COUNT_MAX || dwSlotPos >= SHOP_HOST_ITEM_MAX_NUM)
	{
		TraceError("Out of Index. tabIdx(%d) must be less than %d. dwSlotPos(%d) must be less than %d", tabIdx, SHOP_TAB_COUNT_MAX, dwSlotPos, SHOP_HOST_ITEM_MAX_NUM);
		return FALSE;
	}

	*c_ppItemData = &m_aShoptabs[tabIdx].items[dwSlotPos];

	return TRUE;
}
//
//BOOL CPythonShop::CheckSlotIndex(uint32_t dwSlotPos)
//{
//	if (dwSlotPos >= SHOP_HOST_ITEM_MAX_NUM * SHOP_TAB_COUNT_MAX)
//		return FALSE;
//
//	return TRUE;
//}

void CPythonShop::ClearPrivateShopStock()
{
	m_PrivateShopItemStock.clear();
}
void CPythonShop::AddPrivateShopItemStock(TItemPos ItemPos, uint8_t dwDisplayPos, uint32_t dwPrice)
{
	DelPrivateShopItemStock(ItemPos);

	TShopItemTable SellingItem;
	SellingItem.vnum = 0;
	SellingItem.count = 0;
	SellingItem.pos = ItemPos;
	SellingItem.price = dwPrice;
	SellingItem.display_pos = dwDisplayPos;
	m_PrivateShopItemStock.emplace(ItemPos, SellingItem);
}
void CPythonShop::DelPrivateShopItemStock(TItemPos ItemPos)
{
	if (m_PrivateShopItemStock.end() == m_PrivateShopItemStock.find(ItemPos))
		return;

	m_PrivateShopItemStock.erase(ItemPos);
}
int32_t CPythonShop::GetPrivateShopItemPrice(TItemPos ItemPos)
{
	auto itor = m_PrivateShopItemStock.find(ItemPos);

	if (m_PrivateShopItemStock.end() == itor)
		return 0;

	TShopItemTable & rShopItemTable = itor->second;
	return rShopItemTable.price;
}
struct ItemStockSortFunc
{
	bool operator() (TShopItemTable & rkLeft, TShopItemTable & rkRight)
	{
		return rkLeft.display_pos < rkRight.display_pos;
	}
};
void CPythonShop::BuildPrivateShop(const char * c_szName)
{
	std::vector<TShopItemTable> ItemStock;
	ItemStock.reserve(m_PrivateShopItemStock.size());

	TPrivateShopItemStock::iterator itor = m_PrivateShopItemStock.begin();
	for (; itor != m_PrivateShopItemStock.end(); ++itor)
	{
		ItemStock.push_back(itor->second);
	}

	std::sort(ItemStock.begin(), ItemStock.end(), ItemStockSortFunc());

	CPythonNetworkStream::Instance().SendBuildPrivateShopPacket(c_szName, ItemStock);
}

void CPythonShop::Open(BOOL isPrivateShop, BOOL isMainPrivateShop)
{
	m_isShoping = TRUE;
	m_isPrivateShop = isPrivateShop;
	m_isMainPlayerPrivateShop = isMainPrivateShop;
}

void CPythonShop::Close()
{
	m_isShoping = FALSE;
	m_isPrivateShop = FALSE;
	m_isMainPlayerPrivateShop = FALSE;
}

BOOL CPythonShop::IsOpen()
{
	return m_isShoping;
}

BOOL CPythonShop::IsPrivateShop()
{
	return m_isPrivateShop;
}

BOOL CPythonShop::IsMainPlayerPrivateShop()
{
	return m_isMainPlayerPrivateShop;
}

void CPythonShop::Clear()
{
	m_isShoping = FALSE;
	m_isPrivateShop = FALSE;
	m_isMainPlayerPrivateShop = FALSE;
	ClearPrivateShopStock();
	m_bTabCount = 1;

	for (int32_t i = 0; i < SHOP_TAB_COUNT_MAX; i++)
	{
		m_aShoptabs[i].coinType = SHOP_COIN_TYPE_GOLD;
		m_aShoptabs[i].name = "";

		memset (m_aShoptabs[i].items, 0, sizeof(TShopItemData) * SHOP_HOST_ITEM_MAX_NUM);
	}
}

CPythonShop::CPythonShop()
{
	Clear();
}

CPythonShop::~CPythonShop(void)
{
}

PyObject * shopOpen(PyObject * poSelf, PyObject * poArgs)
{
	BOOL isPrivateShop = FALSE;
	PyTuple_GetInteger(poArgs, 0, &isPrivateShop);
	BOOL isMainPrivateShop = FALSE;
	PyTuple_GetInteger(poArgs, 1, &isMainPrivateShop);

	CPythonShop& rkShop=CPythonShop::Instance();
	rkShop.Open(isPrivateShop, isMainPrivateShop);
	return Py_BuildNone();
}

PyObject * shopClose(PyObject * poSelf, PyObject * poArgs)
{
	CPythonShop& rkShop=CPythonShop::Instance();
	rkShop.Close();
	return Py_BuildNone();
}

PyObject * shopIsOpen(PyObject * poSelf, PyObject * poArgs)
{
	CPythonShop& rkShop=CPythonShop::Instance();
	return Py_BuildValue("i", rkShop.IsOpen());
}

PyObject * shopIsPrviateShop(PyObject * poSelf, PyObject * poArgs)
{
	CPythonShop& rkShop=CPythonShop::Instance();
	return Py_BuildValue("i", rkShop.IsPrivateShop());
}

PyObject * shopIsMainPlayerPrivateShop(PyObject * poSelf, PyObject * poArgs)
{
	CPythonShop& rkShop=CPythonShop::Instance();
	return Py_BuildValue("i", rkShop.IsMainPlayerPrivateShop());
}

PyObject * shopGetItemID(PyObject * poSelf, PyObject * poArgs)
{
	CPythonStackController::Instance().CheckStackReference(CHEAT_TYPE_shop_GetItemID, PY_REF_FILE, PY_REF_FUNC);

	int32_t nPos;
	if (!PyTuple_GetInteger(poArgs, 0, &nPos))
		return Py_BuildException();

	const TShopItemData * c_pItemData;
	if (CPythonShop::Instance().GetItemData(nPos, &c_pItemData))
		return Py_BuildValue("i", c_pItemData->vnum);

	return Py_BuildValue("i", 0);
}

PyObject * shopGetItemCount(PyObject * poSelf, PyObject * poArgs)
{
	CPythonStackController::Instance().CheckStackReference(CHEAT_TYPE_shop_GetItemCount, PY_REF_FILE, PY_REF_FUNC);

	int32_t iIndex;
	if (!PyTuple_GetInteger(poArgs, 0, &iIndex))
		return Py_BuildException();

	const TShopItemData * c_pItemData;
	if (CPythonShop::Instance().GetItemData(iIndex, &c_pItemData))
		return Py_BuildValue("i", c_pItemData->count);

	return Py_BuildValue("i", 0);
}

PyObject * shopGetItemPrice(PyObject * poSelf, PyObject * poArgs)
{
	CPythonStackController::Instance().CheckStackReference(CHEAT_TYPE_shop_GetItemPrice, PY_REF_FILE, PY_REF_FUNC);

	int32_t iIndex;
	if (!PyTuple_GetInteger(poArgs, 0, &iIndex))
		return Py_BuildException();

	const TShopItemData * c_pItemData;
	if (CPythonShop::Instance().GetItemData(iIndex, &c_pItemData))
		return Py_BuildValue("i", c_pItemData->price);

	return Py_BuildValue("i", 0);
}

PyObject * shopGetItemMetinSocket(PyObject * poSelf, PyObject * poArgs)
{
	CPythonStackController::Instance().CheckStackReference(CHEAT_TYPE_shop_GetItemMetinSocket, PY_REF_FILE, PY_REF_FUNC);

	int32_t iIndex;
	if (!PyTuple_GetInteger(poArgs, 0, &iIndex))
		return Py_BuildException();
	int32_t iMetinSocketIndex;
	if (!PyTuple_GetInteger(poArgs, 1, &iMetinSocketIndex))
		return Py_BuildException();

	const TShopItemData * c_pItemData;
	if (CPythonShop::Instance().GetItemData(iIndex, &c_pItemData))
		return Py_BuildValue("i", c_pItemData->alSockets[iMetinSocketIndex]);

	return Py_BuildValue("i", 0);
}

PyObject * shopGetItemAttribute(PyObject * poSelf, PyObject * poArgs)
{
	int32_t iIndex;
	if (!PyTuple_GetInteger(poArgs, 0, &iIndex))
		return Py_BuildException();
	int32_t iAttrSlotIndex;
	if (!PyTuple_GetInteger(poArgs, 1, &iAttrSlotIndex))
		return Py_BuildException();

	if (iAttrSlotIndex >= 0 && iAttrSlotIndex < ITEM_ATTRIBUTE_SLOT_MAX_NUM)
	{
		const TShopItemData * c_pItemData;
		if (CPythonShop::Instance().GetItemData(iIndex, &c_pItemData))
			return Py_BuildValue("ii", c_pItemData->aAttr[iAttrSlotIndex].bType, c_pItemData->aAttr[iAttrSlotIndex].sValue);
	}

	return Py_BuildValue("ii", 0, 0);
}

PyObject * shopClearPrivateShopStock(PyObject * poSelf, PyObject * poArgs)
{
	CPythonStackController::Instance().CheckStackReference(CHEAT_TYPE_shop_ClearPrivateShopStock, PY_REF_FILE, PY_REF_FUNC);

	CPythonShop::Instance().ClearPrivateShopStock();
	return Py_BuildNone();
}
PyObject * shopAddPrivateShopItemStock(PyObject * poSelf, PyObject * poArgs)
{
	CPythonStackController::Instance().CheckStackReference(CHEAT_TYPE_shop_AddPrivateShopItemStock, PY_REF_FILE, PY_REF_FUNC);

	uint8_t bItemWindowType;
	if (!PyTuple_GetInteger(poArgs, 0, &bItemWindowType))
		return Py_BuildException();
	uint16_t wItemSlotIndex;
	if (!PyTuple_GetInteger(poArgs, 1, &wItemSlotIndex))
		return Py_BuildException();
	int32_t iDisplaySlotIndex;
	if (!PyTuple_GetInteger(poArgs, 2, &iDisplaySlotIndex))
		return Py_BuildException();
	int32_t iPrice;
	if (!PyTuple_GetInteger(poArgs, 3, &iPrice))
		return Py_BuildException();

	CPythonShop::Instance().AddPrivateShopItemStock(TItemPos(bItemWindowType, wItemSlotIndex), iDisplaySlotIndex, iPrice);
	return Py_BuildNone();
}
PyObject * shopDelPrivateShopItemStock(PyObject * poSelf, PyObject * poArgs)
{
	uint8_t bItemWindowType;
	if (!PyTuple_GetInteger(poArgs, 0, &bItemWindowType))
		return Py_BuildException();
	uint16_t wItemSlotIndex;
	if (!PyTuple_GetInteger(poArgs, 1, &wItemSlotIndex))
		return Py_BuildException();

	CPythonShop::Instance().DelPrivateShopItemStock(TItemPos(bItemWindowType, wItemSlotIndex));
	return Py_BuildNone();
}
PyObject * shopGetPrivateShopItemPrice(PyObject * poSelf, PyObject * poArgs)
{
	uint8_t bItemWindowType;
	if (!PyTuple_GetInteger(poArgs, 0, &bItemWindowType))
		return Py_BuildException();
	uint16_t wItemSlotIndex;
	if (!PyTuple_GetInteger(poArgs, 1, &wItemSlotIndex))
		return Py_BuildException();

	int32_t iValue = CPythonShop::Instance().GetPrivateShopItemPrice(TItemPos(bItemWindowType, wItemSlotIndex));
	return Py_BuildValue("i", iValue);
}
PyObject * shopBuildPrivateShop(PyObject * poSelf, PyObject * poArgs)
{
	CPythonStackController::Instance().CheckStackReference(CHEAT_TYPE_shop_BuildPrivateShop, PY_REF_FILE, PY_REF_FUNC);

	char * szName;
	if (!PyTuple_GetString(poArgs, 0, &szName))
		return Py_BuildException();

	CPythonShop::Instance().BuildPrivateShop(szName);
	return Py_BuildNone();
}

PyObject * shopGetTabCount(PyObject * poSelf, PyObject * poArgs)
{
	return Py_BuildValue("i", CPythonShop::Instance().GetTabCount());
}

PyObject * shopGetTabName(PyObject * poSelf, PyObject * poArgs)
{
	uint8_t bTabIdx;
	if (!PyTuple_GetInteger(poArgs, 0, &bTabIdx))
		return Py_BuildException();

	return Py_BuildValue("s", CPythonShop::Instance().GetTabName(bTabIdx));
}

PyObject * shopGetTabCoinType(PyObject * poSelf, PyObject * poArgs)
{
	uint8_t bTabIdx;
	if (!PyTuple_GetInteger(poArgs, 0, &bTabIdx))
		return Py_BuildException();

	return Py_BuildValue("i", CPythonShop::Instance().GetTabCoinType(bTabIdx));
}

PyObject * shopSetTabCoinType(PyObject * poSelf, PyObject * poArgs)
{
	uint8_t bTabIdx, type;
	if (!PyTuple_GetInteger(poArgs, 0, &bTabIdx))
		if (!PyTuple_GetInteger(poArgs, 0, &type))
			return Py_BuildException();

	CPythonShop::Instance().SetTabCoinType(bTabIdx, type);
	return Py_BuildNone();
}
void initshop()
{
	static PyMethodDef s_methods[] =
	{
		// Shop
		{ "Open",						shopOpen,						METH_VARARGS },
		{ "Close",						shopClose,						METH_VARARGS },
		{ "IsOpen",						shopIsOpen,						METH_VARARGS },
		{ "IsPrivateShop",				shopIsPrviateShop,				METH_VARARGS },
		{ "IsMainPlayerPrivateShop",	shopIsMainPlayerPrivateShop,	METH_VARARGS },
		{ "GetItemID",					shopGetItemID,					METH_VARARGS },
		{ "GetItemCount",				shopGetItemCount,				METH_VARARGS },
		{ "GetItemPrice",				shopGetItemPrice,				METH_VARARGS },
		{ "GetItemMetinSocket",			shopGetItemMetinSocket,			METH_VARARGS },
		{ "GetItemAttribute",			shopGetItemAttribute,			METH_VARARGS },
		{ "GetTabCount",				shopGetTabCount,				METH_VARARGS },
		{ "GetTabName",					shopGetTabName,					METH_VARARGS },
		{ "GetTabCoinType",				shopGetTabCoinType,				METH_VARARGS },
		{ "SetTabCoinType",				shopSetTabCoinType,				METH_VARARGS },

		// Private Shop
		{ "ClearPrivateShopStock",		shopClearPrivateShopStock,		METH_VARARGS },
		{ "AddPrivateShopItemStock",	shopAddPrivateShopItemStock,	METH_VARARGS },
		{ "DelPrivateShopItemStock",	shopDelPrivateShopItemStock,	METH_VARARGS },
		{ "GetPrivateShopItemPrice",	shopGetPrivateShopItemPrice,	METH_VARARGS },
		{ "BuildPrivateShop",			shopBuildPrivateShop,			METH_VARARGS },
		{ nullptr,							nullptr,							0 },
	};

	PyObject* poModule = Py_InitModule(CPythonDynamicModule::Instance().GetModule(SHOP_MODULE).c_str(), s_methods);

	PyModule_AddIntConstant(poModule, "SHOP_SLOT_COUNT", SHOP_HOST_ITEM_MAX_NUM);
	PyModule_AddIntConstant(poModule, "SHOP_COIN_TYPE_GOLD", SHOP_COIN_TYPE_GOLD);
	PyModule_AddIntConstant(poModule, "SHOP_COIN_TYPE_SECONDARY_COIN", SHOP_COIN_TYPE_SECONDARY_COIN);
}