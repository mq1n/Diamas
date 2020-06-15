#include "StdAfx.h"
#include "NetAddress.h"

#ifndef VC_EXTRALEAN

bool CNetworkAddress::GetHostName(char* szName, int32_t size)
{
	if (gethostname(szName, size)==SOCKET_ERROR)
		return false;
	return true;
}

CNetworkAddress::CNetworkAddress()
{
	Clear();
}

CNetworkAddress::~CNetworkAddress() = default;

CNetworkAddress::operator const SOCKADDR_IN&() const
{
	return m_sockAddrIn;
}

void CNetworkAddress::Clear()
{
	memset(&m_sockAddrIn, 0, sizeof(m_sockAddrIn));
	m_sockAddrIn.sin_family=AF_INET;
}

bool CNetworkAddress::IsIP(const char* c_szAddr)
{
	if (c_szAddr[0]<'0' || c_szAddr[0]>'9')
		return false;
	return true;
}

bool CNetworkAddress::Set(const char* c_szAddr, int32_t port)
{
	if (IsIP(c_szAddr))
	{
		SetIP(c_szAddr);
	}
	else
	{
		if (!SetDNS(c_szAddr))
			return false;
	}

	SetPort(port);
	return true;
}

void CNetworkAddress::SetLocalIP()
{
	SetIP((uint32_t)0x00000000 /* INADDR_ANY */);
}

void CNetworkAddress::SetIP(uint32_t ip)
{
	m_sockAddrIn.sin_addr.s_addr=htonl(ip);
}

void CNetworkAddress::SetIP(const char* c_szIP)
{
	m_sockAddrIn.sin_addr.s_addr=inet_addr(c_szIP);
}

bool CNetworkAddress::SetDNS(const char* c_szDNS)
{
	HOSTENT* pHostent=gethostbyname(c_szDNS);
	if (!pHostent)
		return false;
	memcpy(&m_sockAddrIn.sin_addr, pHostent->h_addr, sizeof(m_sockAddrIn.sin_addr));
	return true;
}

void CNetworkAddress::SetPort(int32_t port)
{
	m_sockAddrIn.sin_port = htons(port);
}

int32_t CNetworkAddress::GetSize()
{
	return sizeof(m_sockAddrIn);
}

uint32_t CNetworkAddress::GetIP()
{
	return ntohl(m_sockAddrIn.sin_addr.s_addr);
}

void CNetworkAddress::GetIP(char* szIP, int32_t len)
{
	uint8_t IPs[4];
	*((uint32_t*)IPs)=m_sockAddrIn.sin_addr.s_addr;

	_snprintf_s(szIP, len, _TRUNCATE, "%d.%d.%d.%d", IPs[0], IPs[1], IPs[2], IPs[3]);
}
			
int32_t CNetworkAddress::GetPort()
{
	return ntohs(m_sockAddrIn.sin_port);
}

#endif