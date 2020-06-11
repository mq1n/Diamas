#pragma once

#include <map>

class CNetworkPacketHeaderMap
{
	public:
		typedef struct SPacketType
		{
			SPacketType(int32_t iSize = 0, bool bFlag = false)
			{
				iPacketSize = iSize;
				isDynamicSizePacket = bFlag;
			}

			int32_t iPacketSize;
			bool isDynamicSizePacket;
		} TPacketType;

	public:
		CNetworkPacketHeaderMap();
		virtual ~CNetworkPacketHeaderMap();

		void Set(int32_t header, TPacketType & rPacketType);
		bool Get(int32_t header, TPacketType * pPacketType);

	protected:
		std::map<int32_t, TPacketType> m_headerMap;
};
