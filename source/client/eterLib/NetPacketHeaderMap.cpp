#include "StdAfx.h"
#include "NetPacketHeaderMap.h"

void CNetworkPacketHeaderMap::Set(int32_t header, TPacketType & rPacketType)
{
	m_headerMap[header] = rPacketType;
}
bool CNetworkPacketHeaderMap::Get(int32_t header, TPacketType * pPacketType)
{
	auto f = m_headerMap.find(header);
	
	if (m_headerMap.end()==f)
		return false;
	
	*pPacketType = f->second;

	return true;
}

CNetworkPacketHeaderMap::CNetworkPacketHeaderMap() = default;

CNetworkPacketHeaderMap::~CNetworkPacketHeaderMap() = default;
