#include "stdafx.h"
#include "constants.h"
#include "config.h"
#include "p2p.h"
#include "desc_p2p.h"
#include "char.h"
#include "char_manager.h"
#include "sectree_manager.h"
#include "guild_manager.h"
#include "party.h"
#include "messenger_manager.h"
#include "marriage.h"
#include "utils.h"
#include "locale_service.h"
#include "map_location.h"

P2P_MANAGER::P2P_MANAGER()
{
	m_pkInputProcessor = nullptr;
	m_iHandleCount = 0;

	memset(m_aiEmpireUserCount, 0, sizeof(m_aiEmpireUserCount));
}

P2P_MANAGER::~P2P_MANAGER()
{
}

void P2P_MANAGER::Boot(LPDESC d)
{
	CHARACTER_MANAGER::NAME_MAP & map = CHARACTER_MANAGER::Instance().GetPCMap();
	CHARACTER_MANAGER::NAME_MAP::iterator it = map.begin();

	TPacketGGLogin p;

	while (it != map.end())
	{
		LPCHARACTER ch = it->second;
		it++;

		p.bHeader = HEADER_GG_LOGIN;
		strlcpy(p.szName, ch->GetName(), sizeof(p.szName));
		p.dwPID = ch->GetPlayerID();
		p.bEmpire = ch->GetEmpire();
		p.lMapIndex = SECTREE_MANAGER::Instance().GetMapIndex(ch->GetX(), ch->GetY());
		p.bChannel = g_bChannel;
		p.iLevel = ch->GetLevel();

		d->Packet(&p, sizeof(p));
	}
}

void P2P_MANAGER::FlushOutput()
{
	std::unordered_set<LPDESC>::iterator it = m_set_pkPeers.begin();

	while (it != m_set_pkPeers.end())
	{
		LPDESC pkDesc = *it++;
		pkDesc->FlushOutput();
	}
}

void P2P_MANAGER::RegisterAcceptor(LPDESC d)
{
	sys_log(0, "P2P Acceptor opened (host %s)", d->GetHostName());
	m_set_pkPeers.insert(d);
	Boot(d);
}

void P2P_MANAGER::UnregisterAcceptor(LPDESC d)
{
	sys_log(0, "P2P Acceptor closed (host %s)", d->GetHostName());
	EraseUserByDesc(d);
	m_set_pkPeers.erase(d);
}

void P2P_MANAGER::RegisterConnector(LPDESC d)
{
	sys_log(0, "P2P Connector opened (host %s)", d->GetHostName());
	m_set_pkPeers.insert(d);
	Boot(d);

	TPacketGGSetup p;
	p.bHeader = HEADER_GG_SETUP;
	p.wPort = p2p_port;
	p.wListenPort = mother_port;
	p.bChannel = g_bChannel;
	d->Packet(&p, sizeof(p));
}

void P2P_MANAGER::UnregisterConnector(LPDESC d)
{
	std::unordered_set<LPDESC>::iterator it = m_set_pkPeers.find(d);

	if (it != m_set_pkPeers.end())
	{
		sys_log(0, "P2P Connector closed (host %s)", d->GetHostName());
		EraseUserByDesc(d);
		m_set_pkPeers.erase(it);
	}
}

void P2P_MANAGER::EraseUserByDesc(LPDESC d)
{
	TCCIMap::iterator it = m_map_pkCCI.begin();

	while (it != m_map_pkCCI.end())
	{
		CCI * pkCCI = it->second;
		it++;

		if (pkCCI->pkDesc == d)
			Logout(pkCCI);
	}
}

void P2P_MANAGER::Send(const void * c_pvData, int32_t iSize, LPDESC except)
{
	std::unordered_set<LPDESC>::iterator it = m_set_pkPeers.begin();

	while (it != m_set_pkPeers.end())
	{
		LPDESC pkDesc = *it++;

		if (except == pkDesc)
			continue;

		pkDesc->Packet(c_pvData, iSize);
	}
}


bool P2P_MANAGER::Send(const void * c_pvData, int32_t iSize, uint16_t wPort, bool bP2PPort)
{
	auto it = m_set_pkPeers.begin();

	while (it != m_set_pkPeers.end())
	{
		LPDESC pkDesc = *it++;

		if ((bP2PPort && pkDesc->GetP2PPort() == wPort) || (!bP2PPort && pkDesc->GetListenPort() == wPort)) {
			sys_log(0, "P2P_Send: send to port %u", wPort);
			pkDesc->Packet(c_pvData, iSize);
			return true;
		}
		else
			sys_log(0, "P2P_Send: wrong port %u [p2p %u, listen %u] [searched %u]", pkDesc->GetPort(), pkDesc->GetP2PPort(), pkDesc->GetListenPort(), wPort);
	}

	return false;
}

void P2P_MANAGER::SendByPID(uint32_t dwPID, const void * c_pvData, int32_t iSize)
{
	CCI* pkCCI = FindByPID(dwPID);
	if (!pkCCI)
		return;

	if (iSize <= 0)
		return;

	pkCCI->pkDesc->SetRelay(pkCCI->szName);
	pkCCI->pkDesc->Packet(c_pvData, iSize);
}

void P2P_MANAGER::Login(LPDESC d, const TPacketGGLogin * p)
{
	CCI* pkCCI = Find(p->szName);

	bool UpdateP2P = false;

	if (nullptr == pkCCI)
	{
		UpdateP2P = true;
		pkCCI = M2_NEW CCI;

		strlcpy(pkCCI->szName, p->szName, sizeof(pkCCI->szName));

		pkCCI->dwPID = p->dwPID;
		pkCCI->bEmpire = p->bEmpire;

		if (p->bChannel == g_bChannel)
		{
			if (pkCCI->bEmpire < EMPIRE_MAX_NUM)
			{
				++m_aiEmpireUserCount[pkCCI->bEmpire];
			}
			else
			{
				sys_err("LOGIN_EMPIRE_ERROR: %d >= MAX(%d)", pkCCI->bEmpire, EMPIRE_MAX_NUM);
			}
		}

		m_map_pkCCI.insert(std::make_pair(pkCCI->szName, pkCCI));
		m_map_dwPID_pkCCI.insert(std::make_pair(pkCCI->dwPID, pkCCI));
	}

	pkCCI->lMapIndex = p->lMapIndex;
	pkCCI->pkDesc = d;
	pkCCI->bChannel = p->bChannel;
	pkCCI->iLevel = p->iLevel;
	sys_log(0, "P2P: Login %s", pkCCI->szName);

	CGuildManager::Instance().P2PLoginMember(pkCCI->dwPID);
	CPartyManager::Instance().P2PLogin(pkCCI->dwPID, pkCCI->szName);

	// CCI가 생성시에만 메신저를 업데이트하면 된다.
	if (UpdateP2P) 
	{
		std::string name(pkCCI->szName);
	    MessengerManager::Instance().P2PLogin(name);
	}
}

void P2P_MANAGER::Logout(CCI * pkCCI)
{
	if (pkCCI->bChannel == g_bChannel)
	{
		if (pkCCI->bEmpire < EMPIRE_MAX_NUM)
		{
			--m_aiEmpireUserCount[pkCCI->bEmpire];
			if (m_aiEmpireUserCount[pkCCI->bEmpire] < 0)
			{
				sys_err("m_aiEmpireUserCount[%d] < 0", pkCCI->bEmpire);
			}
		}
		else
		{
			sys_err("LOGOUT_EMPIRE_ERROR: %d >= MAX(%d)", pkCCI->bEmpire, EMPIRE_MAX_NUM);
		}
	}

	std::string name(pkCCI->szName);

	CGuildManager::Instance().P2PLogoutMember(pkCCI->dwPID);
	CPartyManager::Instance().P2PLogout(pkCCI->dwPID);
	MessengerManager::Instance().P2PLogout(name);
	marriage::CManager::Instance().Logout(pkCCI->dwPID);

	m_map_pkCCI.erase(name);
	m_map_dwPID_pkCCI.erase(pkCCI->dwPID);
	M2_DELETE(pkCCI);
}

void P2P_MANAGER::Logout(const char * c_pszName)
{
	CCI * pkCCI = Find(c_pszName);

	if (!pkCCI)
		return;

	Logout(pkCCI);
	sys_log(0, "P2P: Logout %s", c_pszName);
}

CCI * P2P_MANAGER::FindByPID(uint32_t pid)
{
	TPIDCCIMap::iterator it = m_map_dwPID_pkCCI.find(pid);
	if (it == m_map_dwPID_pkCCI.end())
		return nullptr;
	return it->second;
}

CCI * P2P_MANAGER::Find(const char * c_pszName)
{
	TCCIMap::const_iterator it;

	it = m_map_pkCCI.find(c_pszName);

	if (it == m_map_pkCCI.end())
		return nullptr;

	return it->second;
}

int32_t P2P_MANAGER::GetCount()
{
	//return m_map_pkCCI.size();
	return m_aiEmpireUserCount[1] + m_aiEmpireUserCount[2] + m_aiEmpireUserCount[3];
}

int32_t P2P_MANAGER::GetEmpireUserCount(int32_t idx)
{
	assert(idx < EMPIRE_MAX_NUM);
	return m_aiEmpireUserCount[idx];
}


int32_t P2P_MANAGER::GetDescCount()
{
	return m_set_pkPeers.size();
}

void P2P_MANAGER::GetP2PHostNames(std::string& hostNames)
{
	std::unordered_set<LPDESC>::iterator it = m_set_pkPeers.begin();

	std::ostringstream oss(std::ostringstream::out);

	while (it != m_set_pkPeers.end())
	{
		LPDESC pkDesc = *it++;

		oss << pkDesc->GetP2PHost() << " " << pkDesc->GetP2PPort() << "\n";

	}
	hostNames += oss.str();
}

LPDESC P2P_MANAGER::GetP2PDescByMapIndex(int32_t lMapIndex)
{
	uint16_t wPort = CMapLocation::Instance().GetPort(lMapIndex);
	if (wPort == 0 || wPort == mother_port)
		return nullptr;

	for (auto it = m_set_pkPeers.begin(); it != m_set_pkPeers.end(); ++it)
	{
		LPDESC pkDesc = *it;
		if (pkDesc->GetListenPort() == wPort)
			return pkDesc;

		sys_log(0, "listenPort %hu != wPort %hu", pkDesc->GetListenPort(), wPort);
	}

	sys_err("cannot find peer by port %hu", wPort);

	return nullptr;
}
