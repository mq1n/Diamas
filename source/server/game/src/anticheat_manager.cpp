#include "stdafx.h"
#include "anticheat_manager.h"
#include "db.h"
#include "char.h"
#include "desc.h"
#include "config.h"

bool CAnticheatManager::Initialize()
{
	return ReloadCheatBlacklists();
}

void CAnticheatManager::Release()
{
}


bool CAnticheatManager::CreateClientHandle(LPCHARACTER ch)
{
	if (!ch || !ch->GetDesc())
		return false;

	if (SendBlacklistPacket(ch) == false)
		return false;

	ch->StartAnticheatCommunication();

	return true;
}

void CAnticheatManager::DeleteClientHandle(uint32_t dwPlayerID)
{
}


bool CAnticheatManager::SendBlacklistPacket(LPCHARACTER ch)
{
	return true; // FIXME: Unknown packet header: 48, last: 76 76
	/*
	bool bRet = false;

	for (size_t i = 0; i < m_vSerializedBlacklists.size(); i++)
	{
		std::string strCurrList = m_vSerializedBlacklists[i];
		if (strCurrList.size() <= 0)
			continue;

		TPacketGCCheatBlacklist cheatBlacklistPacket;
		cheatBlacklistPacket.header = HEADER_GC_CHEAT_BLACKLIST;
		strcpy(cheatBlacklistPacket.content, strCurrList.c_str());

        if (ch->GetDesc())
        {
            ch->GetDesc()->Packet(&cheatBlacklistPacket, sizeof(cheatBlacklistPacket));
            bRet = true;
        }
	}

	return bRet;
	*/
}

bool CAnticheatManager::SendCheckPacket(LPCHARACTER ch)
{
	return true;
}


bool CAnticheatManager::ReloadCheatBlacklists()
{
	if (m_vSerializedBlacklists.size() > 0)
		m_vSerializedBlacklists.clear();

	if (__ParseCheatBlacklist(NET_SCAN_ID_PROCESS, "cheat_blacklist_1") == false)
		return false;

	return true;
}

bool CAnticheatManager::__ParseCheatBlacklist(uint32_t uiScanID, const std::string & szTableName)
{
#if 0
	std::unique_ptr<SQLMsg> pMsg(DBManager::Instance().DirectQuery("SELECT * FROM anticheat.%s", szTableName.c_str()));
	if (pMsg->Get()->uiNumRows == 0 || pMsg->uiSQLErrno != 0 || !pMsg->Get()->pSQLResult)
	{
		sys_err("Anticheat blacklist parse query fail! Table: %s Error: %u", szTableName.c_str(), pMsg->uiSQLErrno);
		return true;
	}

	boost::property_tree::ptree pRootTree;
	int32_t iFieldCount = 0;

	switch (uiScanID)
	{
		case NET_SCAN_ID_PROCESS:
		{
			MYSQL_ROW mRow;

			while ((mRow = mysql_fetch_row(pMsg->Get()->pSQLResult)))
			{
				const char* c_szObjectID = mRow[0];
				const char* c_szProcessHash = mRow[1];

				if (g_bIsTestServer)
					sys_log(0, "Field ID: %u | ID: %s Hash: %s", iFieldCount, c_szObjectID, c_szProcessHash);

				boost::property_tree::ptree pNode;
				pNode.put("Md5Hash", c_szProcessHash);
				pRootTree.add_child(c_szObjectID, pNode);

				iFieldCount++;
			}
		} break;

		default:
			sys_err("Unknown scan ID: %u", uiScanID);
			break;
	}

	if (iFieldCount == 0)
		return false;

	char szScanID[4];
	char szRowCount[4];
	char szFieldCount[4];

	sprintf(szScanID, "%u", uiScanID);
	sprintf(szRowCount, "%u", pMsg->Get()->uiAffectedRows);
	sprintf(szFieldCount, "%d", iFieldCount);

	boost::property_tree::ptree pHeaderNode;
	pHeaderNode.put("ScanID", szScanID);
	pHeaderNode.put("RowCount", szRowCount);
	pHeaderNode.put("FieldCount", szFieldCount);
	pRootTree.add_child("Header", pHeaderNode);

	std::ostringstream ossBuf;
	boost::property_tree::write_json(ossBuf, pRootTree);

	m_vSerializedBlacklists.push_back(ossBuf.str().c_str());

	sys_log(0, "Cheat blacklist: %u parsed: %s", uiScanID, ossBuf.str().c_str());
#endif
	return true;
}

