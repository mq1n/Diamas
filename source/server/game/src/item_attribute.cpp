#include "stdafx.h"
#include "constants.h"
#include "log.h"
#include "item.h"
#include "char.h"
#include "desc.h"
#include "item_manager.h"
#include "config.h"

const int32_t MAX_NORM_ATTR_NUM = ITEM_MANAGER::MAX_NORM_ATTR_NUM;
const int32_t MAX_RARE_ATTR_NUM = ITEM_MANAGER::MAX_RARE_ATTR_NUM;

int32_t CItem::GetAttributeSetIndex()
{
	if (GetType() == ITEM_WEAPON)
	{
		if (GetSubType() == WEAPON_ARROW)
			return -1;

		return ATTRIBUTE_SET_WEAPON;
	}

	if (GetType() == ITEM_ARMOR)
	{
		switch (GetSubType())
		{
			case ARMOR_BODY:
				return ATTRIBUTE_SET_BODY;

			case ARMOR_WRIST:
				return ATTRIBUTE_SET_WRIST;

			case ARMOR_FOOTS:
				return ATTRIBUTE_SET_FOOTS;

			case ARMOR_NECK:
				return ATTRIBUTE_SET_NECK;

			case ARMOR_HEAD:
//			case COSTUME_HAIR: // 코스츔 헤어는 일반 투구 아이템과 동일한 Attribute Set을 이용하여 랜덤속성 붙음 (ARMOR_HEAD == COSTUME_HAIR)
				return ATTRIBUTE_SET_HEAD;

			case ARMOR_SHIELD:
				return ATTRIBUTE_SET_SHIELD;

			case ARMOR_EAR:
				return ATTRIBUTE_SET_EAR;
		}
	}
	else if (GetType() == ITEM_COSTUME)
	{
		switch (GetSubType())
		{
			case COSTUME_BODY: 
#ifdef ENABLE_ITEM_ATTR_COSTUME
				return ATTRIBUTE_SET_COSTUME_BODY;
#else
				return ATTRIBUTE_SET_BODY;
#endif

			case COSTUME_HAIR: 
#ifdef ENABLE_ITEM_ATTR_COSTUME
				return ATTRIBUTE_SET_COSTUME_HAIR;
#else
				return ATTRIBUTE_SET_HEAD;
#endif

#ifdef ENABLE_MOUNT_COSTUME_SYSTEM
			case COSTUME_MOUNT:
				break;
#endif

#ifdef ENABLE_WEAPON_COSTUME_SYSTEM
			case COSTUME_WEAPON:
#ifdef ENABLE_ITEM_ATTR_COSTUME
				return ATTRIBUTE_SET_COSTUME_WEAPON;
#else
				return ATTRIBUTE_SET_WEAPON;
#endif
#endif
		}
	}

	return -1;
}

bool CItem::HasAttr(uint8_t bApply)
{
	for (int32_t i = 0; i < ITEM_APPLY_MAX_NUM; ++i)
		if (m_pProto->aApplies[i].bType == bApply)
			return true;

	for (int32_t i = 0; i < MAX_NORM_ATTR_NUM; ++i)
		if (GetAttributeType(i) == bApply)
			return true;

	return false;
}

bool CItem::HasRareAttr(uint8_t bApply)
{
	for (int32_t i = 0; i < MAX_RARE_ATTR_NUM; ++i)
		if (GetAttributeType(i + 5) == bApply)
			return true;

	return false;
}

void CItem::AddAttribute(uint8_t bApply, int16_t sValue)
{
	if (HasAttr(bApply))
		return;

	int32_t i = GetAttributeCount();

	if (i >= MAX_NORM_ATTR_NUM)
		sys_err("item attribute overflow!");
	else
	{
		if (sValue)
			SetAttribute(i, bApply, sValue);
	}
}

void CItem::AddAttr(uint8_t bApply, uint8_t bLevel)
{
	if (HasAttr(bApply))
		return;

	if (bLevel <= 0)
		return;

	int32_t i = GetAttributeCount();

	if (i == MAX_NORM_ATTR_NUM)
		sys_err("item attribute overflow!");
	else
	{
		const TItemAttrTable & r = g_map_itemAttr[bApply];
		int32_t lVal = r.lValues[MIN(4, bLevel - 1)];

		if (lVal)
			SetAttribute(i, bApply, lVal);
	}
}

void CItem::PutAttributeWithLevel(uint8_t bLevel)
{
	int32_t iAttributeSet = GetAttributeSetIndex();

	if (iAttributeSet < 0)
		return;

	if (bLevel > ITEM_ATTRIBUTE_MAX_LEVEL)
		return;

	std::vector<int32_t> avail;

	int32_t total = 0;

	// 붙일 수 있는 속성 배열을 구축
	for (int32_t i = 0; i < MAX_APPLY_NUM; ++i)
	{
		const TItemAttrTable & r = g_map_itemAttr[i];

		if (r.bMaxLevelBySet[iAttributeSet] && !HasAttr(i))
		{
			avail.push_back(i);
			total += r.dwProb;
		}
	}

	// 구축된 배열로 확률 계산을 통해 붙일 속성 선정
	uint32_t prob = number(1, total);
	int32_t attr_idx = APPLY_NONE;

	for (uint32_t i = 0; i < avail.size(); ++i)
	{
		const TItemAttrTable & r = g_map_itemAttr[avail[i]];

		if (prob <= r.dwProb)
		{
			attr_idx = avail[i];
			break;
		}

		prob -= r.dwProb;
	}

	if (!attr_idx)
	{
		sys_err("Cannot put item attribute %d %d", iAttributeSet, bLevel);
		return;
	}

	const TItemAttrTable & r = g_map_itemAttr[attr_idx];

	// 종류별 속성 레벨 최대값 제한
	if (bLevel > r.bMaxLevelBySet[iAttributeSet])
		bLevel = r.bMaxLevelBySet[iAttributeSet];

	AddAttr(attr_idx, bLevel);
}

void CItem::PutAttribute(const int32_t * aiAttrPercentTable)
{
	int32_t iAttrLevelPercent = number(1, 100);
	int32_t i;

	for (i = 0; i < ITEM_ATTRIBUTE_MAX_LEVEL; ++i)
	{
		if (iAttrLevelPercent <= aiAttrPercentTable[i])
			break;

		iAttrLevelPercent -= aiAttrPercentTable[i];
	}

	PutAttributeWithLevel(i + 1);
}

void CItem::ChangeAttribute(const int32_t* aiChangeProb)
{
	int32_t iAttributeCount = GetAttributeCount();

	ClearAttribute();

	if (iAttributeCount == 0)
		return;

	SItemTable_Server const * pProto = GetProto();

	if (pProto && pProto->sAddonType)
	{
		ApplyAddon(pProto->sAddonType);
	}

	static const int32_t tmpChangeProb[ITEM_ATTRIBUTE_MAX_LEVEL] =
	{
		0, 10, 40, 35, 15,
	};

	for (int32_t i = GetAttributeCount(); i < iAttributeCount; ++i)
	{
		if (aiChangeProb == nullptr)
		{
			PutAttribute(tmpChangeProb);
		}
		else
		{
			PutAttribute(aiChangeProb);
		}
	}
}

void CItem::AddAttribute()
{
	static const int32_t aiItemAddAttributePercent[ITEM_ATTRIBUTE_MAX_LEVEL] = 
	{
		40, 50, 10, 0, 0
	};

	if (GetAttributeCount() < MAX_NORM_ATTR_NUM)
		PutAttribute(aiItemAddAttributePercent);
}

void CItem::ClearAttribute()
{
	for (int32_t i = 0; i < MAX_NORM_ATTR_NUM; ++i)
	{
		m_aAttr[i].bType = 0;
		m_aAttr[i].sValue = 0;
	}
}

int32_t CItem::GetAttributeCount()
{
	int32_t i;

	for (i = 0; i < MAX_NORM_ATTR_NUM; ++i)
	{
		if (GetAttributeType(i) == 0)
			break;
	}

	return i;
}

int32_t CItem::FindAttribute(uint8_t bType)
{
	for (int32_t i = 0; i < MAX_NORM_ATTR_NUM; ++i)
	{
		if (GetAttributeType(i) == bType)
			return i;
	}

	return -1;
}

bool CItem::RemoveAttributeAt(int32_t index)
{
	if (GetAttributeCount() <= index)
		return false;

	for (int32_t i = index; i < MAX_NORM_ATTR_NUM - 1; ++i)
	{
		SetAttribute(i, GetAttributeType(i + 1), GetAttributeValue(i + 1));
	}

	SetAttribute(MAX_NORM_ATTR_NUM - 1, APPLY_NONE, 0);
	return true;
}

bool CItem::RemoveAttributeType(uint8_t bType)
{
	int32_t index = FindAttribute(bType);
	return index != -1 && RemoveAttributeType(index);
}

void CItem::SetAttributes(const TPlayerItemAttribute* c_pAttribute)
{
	memcpy(m_aAttr, c_pAttribute, sizeof(m_aAttr));
	Save();
}

void CItem::SetAttribute(int32_t i, uint8_t bType, int16_t sValue)
{
	assert(i < MAX_NORM_ATTR_NUM);

	m_aAttr[i].bType = bType;
	m_aAttr[i].sValue = sValue;
	UpdatePacket();
	Save();
}

void CItem::SetForceAttribute(int32_t i, uint8_t bType, int16_t sValue)
{
	assert(i < ITEM_ATTRIBUTE_MAX_NUM);

	m_aAttr[i].bType = bType;
	m_aAttr[i].sValue = sValue;
	UpdatePacket();
	Save();
}


void CItem::CopyAttributeTo(LPITEM pItem)
{
	pItem->SetAttributes(m_aAttr);
}

int32_t CItem::GetRareAttrCount()
{
	int32_t ret = 0;

	for (uint32_t dwIdx = ITEM_ATTRIBUTE_RARE_START; dwIdx < ITEM_ATTRIBUTE_RARE_END; dwIdx++)
	{
		if (m_aAttr[dwIdx].bType != 0)
			ret++;
	}

	return ret;
}

bool CItem::ChangeRareAttribute()
{
	if (GetRareAttrCount() == 0)
		return false;

	int32_t cnt = GetRareAttrCount();

	for (int32_t i = 0; i < cnt; ++i)
	{
		m_aAttr[i + ITEM_ATTRIBUTE_RARE_START].bType = 0;
		m_aAttr[i + ITEM_ATTRIBUTE_RARE_START].sValue = 0;
	}

	for (int32_t i = 0; i < cnt; ++i)
	{
		AddRareAttribute();
	}

	return true;
}

bool CItem::AddRareAttribute()
{
	int32_t count = GetRareAttrCount();

	if (count >= ITEM_ATTRIBUTE_RARE_NUM)
		return false;

	int32_t pos = count + ITEM_ATTRIBUTE_RARE_START;
	TPlayerItemAttribute & attr = m_aAttr[pos];

	int32_t nAttrSet = GetAttributeSetIndex();
	std::vector<int32_t> avail;

	for (int32_t i = 0; i < MAX_APPLY_NUM; ++i)
	{
		const TItemAttrTable & r = g_map_itemRare[i];

		if (r.dwApplyIndex != 0 && r.bMaxLevelBySet[nAttrSet] > 0 && HasRareAttr(i) != true)
		{
			avail.push_back(i);
		}
	}

	const TItemAttrTable& r = g_map_itemRare[avail[number(0, avail.size() - 1)]];
	int32_t nAttrLevel = 5;

	if (nAttrLevel > r.bMaxLevelBySet[nAttrSet])
		nAttrLevel = r.bMaxLevelBySet[nAttrSet];

	attr.bType = r.dwApplyIndex;
	attr.sValue = r.lValues[nAttrLevel - 1];

	UpdatePacket();

	Save();

	return true;
}

void CItem::AddRareAttribute2(const int32_t * aiAttrPercentTable)
{
	static const int32_t aiItemAddAttributePercent[ITEM_ATTRIBUTE_MAX_LEVEL] =
	{
		40, 50, 10, 0, 0
	};
	if (aiAttrPercentTable == nullptr)
		aiAttrPercentTable = aiItemAddAttributePercent;

	if (GetRareAttrCount() < MAX_RARE_ATTR_NUM)
		PutRareAttribute(aiAttrPercentTable);
}

void CItem::PutRareAttribute(const int32_t * aiAttrPercentTable)
{
	int32_t iAttrLevelPercent = number(1, 100);
	int32_t i;

	for (i = 0; i < ITEM_ATTRIBUTE_MAX_LEVEL; ++i)
	{
		if (iAttrLevelPercent <= aiAttrPercentTable[i])
			break;

		iAttrLevelPercent -= aiAttrPercentTable[i];
	}

	PutRareAttributeWithLevel(i + 1);
}

void CItem::PutRareAttributeWithLevel(uint8_t bLevel)
{
	int32_t iAttributeSet = GetAttributeSetIndex();
	if (iAttributeSet < 0)
		return;

	if (bLevel > ITEM_ATTRIBUTE_MAX_LEVEL)
		return;

	std::vector<int32_t> avail;

	int32_t total = 0;

	
	for (int32_t i = 0; i < MAX_APPLY_NUM; ++i)
	{
		const TItemAttrTable & r = g_map_itemRare[i];

		if (r.bMaxLevelBySet[iAttributeSet] && !HasRareAttr(i))
		{
			avail.push_back(i);
			total += r.dwProb;
		}
	}

	
	uint32_t prob = number(1, total);
	int32_t attr_idx = APPLY_NONE;

	for (uint32_t i = 0; i < avail.size(); ++i)
	{
		const TItemAttrTable & r = g_map_itemRare[avail[i]];

		if (prob <= r.dwProb)
		{
			attr_idx = avail[i];
			break;
		}

		prob -= r.dwProb;
	}

	if (!attr_idx)
	{
		sys_err("Cannot put item rare attribute %d %d", iAttributeSet, bLevel);
		return;
	}

	const TItemAttrTable & r = g_map_itemRare[attr_idx];

	
	if (bLevel > r.bMaxLevelBySet[iAttributeSet])
		bLevel = r.bMaxLevelBySet[iAttributeSet];

	AddRareAttr(attr_idx, bLevel);
}

void CItem::AddRareAttr(uint8_t bApply, uint8_t bLevel)
{
	if (HasRareAttr(bApply))
		return;

	if (bLevel <= 0)
		return;

	int32_t i = ITEM_ATTRIBUTE_RARE_START + GetRareAttrCount();

	if (i == ITEM_ATTRIBUTE_RARE_END)
		sys_err("item rare attribute overflow!");
	else
	{
		const TItemAttrTable & r = g_map_itemRare[bApply];
		int32_t lVal = r.lValues[MIN(4, bLevel - 1)];

		if (lVal)
			SetForceAttribute(i, bApply, lVal);
	}
}

