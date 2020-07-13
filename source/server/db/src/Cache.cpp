#include "stdafx.h"
#include "Cache.h"
#include "../../common/tables.h"
#include "QID.h"
#include "ClientManager.h"
#include "Main.h"

extern int32_t g_iPlayerCacheFlushSeconds;
extern int32_t g_iItemCacheFlushSeconds;
extern bool g_test_server;
// MYSHOP_PRICE_LIST
extern int32_t g_iItemPriceListTableCacheFlushSeconds;
const int32_t CItemPriceListTableCache::s_nMinFlushSec = 1800;
// END_OF_MYSHOP_PRICE_LIST
//
extern int32_t g_iActivityCacheFlushSeconds;

CItemCache::CItemCache()
{
	m_expireTime = MIN(1800, g_iItemCacheFlushSeconds);
}

CItemCache::~CItemCache()
= default;

// �̰� �̻��ѵ�...
// Delete�� ������, Cache�� �����ؾ� �ϴ°� �ƴѰ�???
// �ٵ� Cache�� �����ϴ� �κ��� ����.
// �� ã�� �ǰ�?
// �̷��� �س�����, ��� �ð��� �� ������ �������� ��� ����...
// �̹� ����� �������ε�... Ȯ�λ��??????
// fixme
// by rtsummit
void CItemCache::Delete()
{
	if (m_data.vnum == 0)
		return;

	//char szQuery[QUERY_MAX_LEN];
	//szQuery[QUERY_MAX_LEN] = '\0';
	if (g_test_server)
		sys_log(0, "ItemCache::Delete : DELETE %u", m_data.id);

	m_data.vnum = 0;
	m_bNeedQuery = true;
	m_lastUpdateTime = time(nullptr);
	OnFlush();
	
	//m_bNeedQuery = false;
	//m_lastUpdateTime = time(0) - m_expireTime; // �ٷ� Ÿ�Ӿƿ� �ǵ��� ����.
}

void CItemCache::OnFlush()
{
	if (m_data.vnum == 0) // vnum�� 0�̸� �����϶�� ǥ�õ� ���̴�.
	{
		char szQuery[ASQL_QUERY_MAX_LEN];
		snprintf(szQuery, sizeof(szQuery), "DELETE FROM item WHERE id=%u", m_data.id);
		CDBManager::Instance().ReturnQuery(szQuery, QID_ITEM_DESTROY, 0, nullptr);

		if (g_test_server)
			sys_log(0, "ItemCache::Flush : DELETE %u %s", m_data.id, szQuery);
	}
	else
	{
		int32_t alSockets[ITEM_SOCKET_MAX_NUM];
		TPlayerItemAttribute aAttr[ITEM_ATTRIBUTE_MAX_NUM];
		bool isSocket = false, isAttr = false;

		memset(&alSockets, 0, sizeof(int32_t) * ITEM_SOCKET_MAX_NUM);
		memset(&aAttr, 0, sizeof(TPlayerItemAttribute) * ITEM_ATTRIBUTE_MAX_NUM);

		TPlayerItem * p = &m_data;

		if (memcmp(alSockets, p->alSockets, sizeof(int32_t) * ITEM_SOCKET_MAX_NUM))
			isSocket = true;

		if (memcmp(aAttr, p->aAttr, sizeof(TPlayerItemAttribute) * ITEM_ATTRIBUTE_MAX_NUM))
			isAttr = true;

		char szColumns[ASQL_QUERY_MAX_LEN];
		char szValues[ASQL_QUERY_MAX_LEN];
		char szUpdate[ASQL_QUERY_MAX_LEN];

		int32_t iLen = snprintf(szColumns, sizeof(szColumns), "id, owner_id, `window`, pos, count, vnum, is_gm_owner");

		int32_t iValueLen = snprintf(szValues, sizeof(szValues), "%u, %u, %d, %d, %u, %u, %u",
				p->id, p->owner, p->window, p->pos, p->count, p->vnum, p->is_gm_owner);

		int32_t iUpdateLen = snprintf(szUpdate, sizeof(szUpdate), "owner_id=%u, window=%d, pos=%d, count=%u, vnum=%u, is_gm_owner=%u",
				p->owner, p->window, p->pos, p->count, p->vnum, p->is_gm_owner);

		if (isSocket)
		{
			iLen += snprintf(szColumns + iLen, sizeof(szColumns) - iLen, ", socket0, socket1, socket2");
			iValueLen += snprintf(szValues + iValueLen, sizeof(szValues) - iValueLen,
					", %u, %u, %u", p->alSockets[0], p->alSockets[1], p->alSockets[2]);
			iUpdateLen += snprintf(szUpdate + iUpdateLen, sizeof(szUpdate) - iUpdateLen,
					", socket0=%u, socket1=%u, socket2=%u", p->alSockets[0], p->alSockets[1], p->alSockets[2]);
		}

		if (isAttr)
		{
			iLen += snprintf(szColumns + iLen, sizeof(szColumns) - iLen, 
					", attrtype0, attrvalue0, attrtype1, attrvalue1, attrtype2, attrvalue2, attrtype3, attrvalue3"
					", attrtype4, attrvalue4, attrtype5, attrvalue5, attrtype6, attrvalue6");

			iValueLen += snprintf(szValues + iValueLen, sizeof(szValues) - iValueLen,
					", %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d",
					p->aAttr[0].bType, p->aAttr[0].sValue,
					p->aAttr[1].bType, p->aAttr[1].sValue,
					p->aAttr[2].bType, p->aAttr[2].sValue,
					p->aAttr[3].bType, p->aAttr[3].sValue,
					p->aAttr[4].bType, p->aAttr[4].sValue,
					p->aAttr[5].bType, p->aAttr[5].sValue,
					p->aAttr[6].bType, p->aAttr[6].sValue);

			iUpdateLen += snprintf(szUpdate + iUpdateLen, sizeof(szUpdate) - iUpdateLen,
					", attrtype0=%d, attrvalue0=%d"
					", attrtype1=%d, attrvalue1=%d"
					", attrtype2=%d, attrvalue2=%d"
					", attrtype3=%d, attrvalue3=%d"
					", attrtype4=%d, attrvalue4=%d"
					", attrtype5=%d, attrvalue5=%d"
					", attrtype6=%d, attrvalue6=%d",
					p->aAttr[0].bType, p->aAttr[0].sValue,
					p->aAttr[1].bType, p->aAttr[1].sValue,
					p->aAttr[2].bType, p->aAttr[2].sValue,
					p->aAttr[3].bType, p->aAttr[3].sValue,
					p->aAttr[4].bType, p->aAttr[4].sValue,
					p->aAttr[5].bType, p->aAttr[5].sValue,
					p->aAttr[6].bType, p->aAttr[6].sValue);
		}

		char szItemQuery[ASQL_QUERY_MAX_LEN + ASQL_QUERY_MAX_LEN];
		snprintf(szItemQuery, sizeof(szItemQuery), "REPLACE INTO item (%s) VALUES(%s)", szColumns, szValues);

		if (g_test_server)	
			sys_log(0, "ItemCache::Flush :REPLACE  (%s)", szItemQuery);

		CDBManager::Instance().ReturnQuery(szItemQuery, QID_ITEM_SAVE, 0, nullptr);
	}

	m_bNeedQuery = false;
}

//
// CPlayerTableCache
//
CPlayerTableCache::CPlayerTableCache()
{
	m_expireTime = MIN(1800, g_iPlayerCacheFlushSeconds);
}

CPlayerTableCache::~CPlayerTableCache()
= default;

void CPlayerTableCache::OnFlush()
{
	if (g_test_server)
		sys_log(0, "PlayerTableCache::Flush : %s", m_data.name);

	char szQuery[ASQL_QUERY_MAX_LEN];
	CreatePlayerSaveQuery(szQuery, sizeof(szQuery), &m_data);
	CDBManager::Instance().ReturnQuery(szQuery, QID_PLAYER_SAVE, 0, nullptr);
}

// MYSHOP_PRICE_LIST
//
// CItemPriceListTableCache class implementation
//


CItemPriceListTableCache::CItemPriceListTableCache()
{
	m_expireTime = MIN(s_nMinFlushSec, g_iItemPriceListTableCacheFlushSeconds);
}

void CItemPriceListTableCache::UpdateList(const TItemPriceListTable* pUpdateList)
{
	//
	// �̹� ĳ�̵� �����۰� �ߺ��� �������� ã�� �ߺ����� �ʴ� ���� ������ tmpvec �� �ִ´�.
	//

	std::vector<TItemPriceInfo> tmpvec;

	for (uint32_t idx = 0; idx < m_data.byCount; ++idx)
	{
		const TItemPriceInfo* pos = pUpdateList->aPriceInfo;
		for (; pos != pUpdateList->aPriceInfo + pUpdateList->byCount && m_data.aPriceInfo[idx].dwVnum != pos->dwVnum; ++pos)
			;

		if (pos == pUpdateList->aPriceInfo + pUpdateList->byCount)
			tmpvec.push_back(m_data.aPriceInfo[idx]);
	}

	//
	// pUpdateList �� m_data �� �����ϰ� ���� ������ tmpvec �� �տ��� ���� ���� ��ŭ �����Ѵ�.
	// 

	if (pUpdateList->byCount > SHOP_PRICELIST_MAX_NUM)
	{
		sys_err("Count overflow!");
		return;
	}

	m_data.byCount = pUpdateList->byCount;

	memcpy(m_data.aPriceInfo, pUpdateList->aPriceInfo, sizeof(TItemPriceInfo) * pUpdateList->byCount);

	int32_t nDeletedNum;	// ������ ���������� ����

	if (pUpdateList->byCount < SHOP_PRICELIST_MAX_NUM && tmpvec.size() > 0)
	{
		size_t sizeAddOldDataSize = SHOP_PRICELIST_MAX_NUM - pUpdateList->byCount;

		if (tmpvec.size() < sizeAddOldDataSize)
			sizeAddOldDataSize = tmpvec.size();
		if (tmpvec.size() != 0)
		{
			memcpy(m_data.aPriceInfo + pUpdateList->byCount, &tmpvec[0], sizeof(TItemPriceInfo) * sizeAddOldDataSize);
			m_data.byCount += static_cast<uint8_t>(sizeAddOldDataSize);
		}
		nDeletedNum = static_cast<int32_t>(tmpvec.size() - sizeAddOldDataSize);
	}
	else
		nDeletedNum = static_cast<int32_t>(tmpvec.size());

	m_bNeedQuery = true;

	sys_log(0, 
			"ItemPriceListTableCache::UpdateList : OwnerID[%u] Update [%u] Items, Delete [%u] Items, Total [%u] Items", 
			m_data.dwOwnerID, pUpdateList->byCount, nDeletedNum, m_data.byCount);
}

void CItemPriceListTableCache::OnFlush()
{
	char szQuery[ASQL_QUERY_MAX_LEN];

	//
	// �� ĳ���� �����ڿ� ���� ������ DB �� ����� ������ ���������� ��� �����Ѵ�.
	//

	snprintf(szQuery, sizeof(szQuery), "DELETE FROM myshop_pricelist WHERE owner_id = %u", m_data.dwOwnerID);
	CDBManager::Instance().ReturnQuery(szQuery, QID_ITEMPRICE_DESTROY, 0, nullptr);

	//
	// ĳ���� ������ ��� DB �� ����.
	//

	for (int32_t idx = 0; idx < m_data.byCount; ++idx)
	{
		snprintf(szQuery, sizeof(szQuery),
				"REPLACE myshop_pricelist(owner_id, item_vnum, price) VALUES(%u, %u, %u)", 
				m_data.dwOwnerID, m_data.aPriceInfo[idx].dwVnum, m_data.aPriceInfo[idx].dwPrice);
		CDBManager::Instance().ReturnQuery(szQuery, QID_ITEMPRICE_SAVE, 0, nullptr);
	}

	sys_log(0, "ItemPriceListTableCache::Flush : OwnerID[%u] Update [%u]Items", m_data.dwOwnerID, m_data.byCount);
	
	m_bNeedQuery = false;
}

CItemPriceListTableCache::~CItemPriceListTableCache()
{
}
// END_OF_MYSHOP_PRICE_LIST

//
// CActivityCache
//
CActivityCache::CActivityCache()
{
	m_expireTime = MIN(600, g_iActivityCacheFlushSeconds);
}

CActivityCache::~CActivityCache()
= default;

void CActivityCache::OnFlush()
{
	if (g_test_server)
		sys_log(0, "ActivityCache::Flush: %u", m_data.pid);

	//Save the main shop info
	char szQuery[ASQL_QUERY_MAX_LEN];
	snprintf(szQuery, sizeof(szQuery),
		"INSERT INTO activity (pid, today_pve, today_pvp, today_gk, today_other, total, last_update) "
		"VALUES (%u, %d, %d, %d, %d, %d, %u) "
		"ON DUPLICATE KEY UPDATE today_pve = VALUES(today_pve), today_pvp = VALUES(today_pvp), today_gk = VALUES(today_gk), "
		"today_other = VALUES(today_other), total = VALUES(total), last_update = VALUES(last_update)",
		m_data.pid,
		m_data.today.pve,
		m_data.today.pvp,
		m_data.today.gk,
		m_data.today.other,
		m_data.total,
		m_data.lastUpdate);

	CDBManager::Instance().AsyncQuery(szQuery);
}

