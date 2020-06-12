#ifndef __INC_METIN_II_GAME_PACKET_HEADER_INFO_H__
#define __INC_METIN_II_GAME_PACKET_HEADER_INFO_H__

#include "packet.h"

typedef struct SPacketElement
{
	int32_t		iSize;
	std::string	stName;
	int32_t		iCalled;
	uint32_t	dwLoad;
} TPacketElement;

class CPacketInfo
{
	public:
		CPacketInfo();
		virtual ~CPacketInfo();

		void Set(int32_t header, int32_t size, const char * c_pszName);
		bool Get(int32_t header, int32_t * size, const char ** c_ppszName);

		void Start();
		void End();

		void Log(const char * c_pszFileName);

	private:
		TPacketElement * GetElement(int32_t header);

	protected:
		std::map<int32_t, TPacketElement *> m_pPacketMap;
		TPacketElement * m_pCurrentPacket;
		uint32_t m_dwStartTime;
};

class CPacketInfoCG : public CPacketInfo
{
	public:
		CPacketInfoCG();
		virtual ~CPacketInfoCG();
};

// PacketInfo P2P 
class CPacketInfoGG : public CPacketInfo
{
	public:
		CPacketInfoGG();
		virtual ~CPacketInfoGG();
};

#endif
