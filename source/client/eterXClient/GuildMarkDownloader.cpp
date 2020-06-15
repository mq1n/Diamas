#include "StdAfx.h"
#include "GuildMarkDownloader.h"
#include "PythonCharacterManager.h"
#include "PythonApplication.h"
#include "Packet.h"

// MARK_BUG_FIX
struct SMarkIndex
{
	uint16_t guild_id;
	uint16_t mark_id;
};

// END_OFMARK_BUG_FIX

CGuildMarkDownloader::CGuildMarkDownloader()
{
	SetRecvBufferSize(640*1024);
	SetSendBufferSize(1024);
	__Initialize();
}

CGuildMarkDownloader::~CGuildMarkDownloader()
{
	__OfflineState_Set();
}

bool CGuildMarkDownloader::Connect(const CNetworkAddress& c_rkNetAddr, uint32_t dwHandle, uint32_t dwRandomKey)
{
	__OfflineState_Set();

	m_dwHandle=dwHandle;
	m_dwRandomKey=dwRandomKey;
	m_dwTodo=TODO_RECV_MARK;
	return CNetworkStream::Connect(c_rkNetAddr);
}

bool CGuildMarkDownloader::ConnectToRecvSymbol(const CNetworkAddress& c_rkNetAddr, uint32_t dwHandle, uint32_t dwRandomKey, const std::set<uint32_t> & c_rkSet_dwGuildID)
{
	__OfflineState_Set();

	m_dwHandle=dwHandle;
	m_dwRandomKey=dwRandomKey;
	m_dwTodo=TODO_RECV_SYMBOL;
	m_kSet_dwGuildID = c_rkSet_dwGuildID;
	return CNetworkStream::Connect(c_rkNetAddr);
}

void CGuildMarkDownloader::Process()
{
	CNetworkStream::Process();

	if (!__StateProcess())
	{
		__OfflineState_Set();
		Disconnect();
	}
}

void CGuildMarkDownloader::OnConnectFailure()
{
	__OfflineState_Set();
}

void CGuildMarkDownloader::OnConnectSuccess()
{
	__LoginState_Set();
}

void CGuildMarkDownloader::OnRemoteDisconnect()
{
	__OfflineState_Set();
}

void CGuildMarkDownloader::OnDisconnect()
{
	__OfflineState_Set();
}

void CGuildMarkDownloader::__Initialize()
{
	m_eState=STATE_OFFLINE;
	m_pkMarkMgr=nullptr;
	m_currentRequestingImageIndex=0;
	m_dwBlockIndex=0;
	m_dwBlockDataPos=0;
	m_dwBlockDataSize=0;
	m_dwSymbolLastGuildID = 0;

	m_dwHandle=0;
	m_dwRandomKey=0;
	m_dwTodo=TODO_RECV_NONE;
	m_kSet_dwGuildID.clear();
}

bool CGuildMarkDownloader::__StateProcess()
{
	switch (m_eState)
	{
		case STATE_LOGIN:
			return __LoginState_Process();
		case STATE_COMPLETE:
			return false;
	}

	return true;
}

void CGuildMarkDownloader::__OfflineState_Set()
{
	__Initialize();
}

void CGuildMarkDownloader::__CompleteMarkState_Set()
{
	m_eState = STATE_COMPLETE;
	CPythonCharacterManager::Instance().RefreshAllGuildMark();
}

void CGuildMarkDownloader::__CompleteSymbolState_Set()
{
	m_eState = STATE_COMPLETE;
	CPythonCharacterManager::Instance().RefreshGuildSymbols(m_kSet_dwGuildID);
}

void CGuildMarkDownloader::__LoginState_Set()
{
	m_eState = STATE_LOGIN;
}

bool CGuildMarkDownloader::__LoginState_Process()
{
	uint8_t header;

	if (!Peek(sizeof(uint8_t), &header))
		return true;

	if (IsSecurityMode())
	{
		if (0 == header)
		{
			if (!Recv(sizeof(header), &header))
				return false;
			
			return true;
		}
	}
	
	uint32_t needPacketSize = __GetPacketSize(header);

	if (!needPacketSize)
		return false;

	if (!Peek(needPacketSize))
		return true;

	__DispatchPacket(header);
	return true;
}

// MARK_BUG_FIX
uint32_t CGuildMarkDownloader::__GetPacketSize(uint32_t header) const
{
	switch (header)
	{
		case HEADER_GC_PHASE:
			return sizeof(TPacketGCPhase);
		case HEADER_GC_HANDSHAKE:
			return sizeof(TPacketGCHandshake);
		case HEADER_GC_PING:
			return sizeof(TPacketGCPing);
		case HEADER_GC_MARK_IDXLIST:
			return sizeof(TPacketGCMarkIDXList);
		case HEADER_GC_MARK_BLOCK:
			return sizeof(TPacketGCMarkBlock);
		case HEADER_GC_GUILD_SYMBOL_DATA:
			return sizeof(TPacketGCGuildSymbolData);
#ifdef _IMPROVED_PACKET_ENCRYPTION_
		case HEADER_GC_KEY_AGREEMENT:
			return sizeof(TPacketKeyAgreement);
		case HEADER_GC_KEY_AGREEMENT_COMPLETED:
			return sizeof(TPacketKeyAgreementCompleted);
#endif
	}
	return 0;
}

bool CGuildMarkDownloader::__DispatchPacket(uint32_t header)
{
	switch (header)
	{
		case HEADER_GC_PHASE:
			return __LoginState_RecvPhase();
		case HEADER_GC_HANDSHAKE:
			return __LoginState_RecvHandshake();
		case HEADER_GC_PING:
			return __LoginState_RecvPing();
		case HEADER_GC_MARK_IDXLIST:
			return __LoginState_RecvMarkIndex();
		case HEADER_GC_MARK_BLOCK:
			return __LoginState_RecvMarkBlock();
		case HEADER_GC_GUILD_SYMBOL_DATA:
			return __LoginState_RecvSymbolData();
#ifdef _IMPROVED_PACKET_ENCRYPTION_
		case HEADER_GC_KEY_AGREEMENT:
			return __LoginState_RecvKeyAgreement();
		case HEADER_GC_KEY_AGREEMENT_COMPLETED:
			return __LoginState_RecvKeyAgreementCompleted();
#endif
	}
	return false;	
}
// END_OF_MARK_BUG_FIX

bool CGuildMarkDownloader::__LoginState_RecvHandshake()
{
	TPacketGCHandshake kPacketHandshake;
	if (!Recv(sizeof(kPacketHandshake), &kPacketHandshake))
		return false;

	TPacketCGMarkLogin kPacketMarkLogin;

	kPacketMarkLogin.header = HEADER_CG_MARK_LOGIN;
	kPacketMarkLogin.handle = m_dwHandle;
	kPacketMarkLogin.random_key = m_dwRandomKey;

	if (!Send(sizeof(kPacketMarkLogin), &kPacketMarkLogin))
		return false;

	return true;
}

bool CGuildMarkDownloader::__LoginState_RecvPing()
{
	TPacketGCPing kPacketPing;

	if (!Recv(sizeof(kPacketPing), &kPacketPing))
		return false;

	TPacketCGPong kPacketPong;
	kPacketPong.bHeader = HEADER_CG_PONG;

	if (!Send(sizeof(TPacketCGPong), &kPacketPong))
		return false;

	return true;
}

bool CGuildMarkDownloader::__LoginState_RecvPhase()
{
	TPacketGCPhase kPacketPhase;

	if (!Recv(sizeof(kPacketPhase), &kPacketPhase))
		return false;

	if (kPacketPhase.phase == PHASE_LOGIN)
	{
#ifndef _IMPROVED_PACKET_ENCRYPTION_
		const char* key = LocaleService_GetSecurityKey();
		SetSecurityMode(true, key);
#endif

		switch (m_dwTodo)
		{
			case TODO_RECV_NONE:
			{
				assert(!"CGuildMarkDownloader::__LoginState_RecvPhase - Todo type is none");
				break;
			}
			case TODO_RECV_MARK:
			{
				// MARK_BUG_FIX
				if (!__SendMarkIDXList())
					return false;
				// END_OF_MARK_BUG_FIX
				break;
			}
			case TODO_RECV_SYMBOL:
			{
				if (!__SendSymbolCRCList())
					return false;
				break;
			}
		}
	}

	CPythonApplication::Instance().SetGameStage(kPacketPhase.stage);
	return true;
}			 

// MARK_BUG_FIX
bool CGuildMarkDownloader::__SendMarkIDXList()
{
	TPacketCGMarkIDXList kPacketMarkIDXList;
	kPacketMarkIDXList.header = HEADER_CG_MARK_IDXLIST;
	if (!Send(sizeof(kPacketMarkIDXList), &kPacketMarkIDXList))
		return false;

	return true;
}

bool CGuildMarkDownloader::__LoginState_RecvMarkIndex()
{
	TPacketGCMarkIDXList kPacketMarkIndex;

	if (!Peek(sizeof(kPacketMarkIndex), &kPacketMarkIndex))
		return false;

	//uint32_t bufSize = sizeof(uint16_t) * 2 * kPacketMarkIndex.count;

	if (!Peek(kPacketMarkIndex.bufSize))
		return false;

	Recv(sizeof(kPacketMarkIndex));

	uint16_t guildID, markID;

	for (uint32_t i = 0; i < kPacketMarkIndex.count; ++i)
	{
		Recv(sizeof(uint16_t), &guildID);
		Recv(sizeof(uint16_t), &markID);

		// 길드ID -> 마크ID 인덱스 등록
		CGuildMarkManager::Instance().AddMarkIDByGuildID(guildID, markID);
	}

	// 모든 마크 이미지 파일을 로드한다. (파일이 없으면 만들어짐)
	CGuildMarkManager::Instance().LoadMarkImages();

	m_currentRequestingImageIndex = 0;
	__SendMarkCRCList();
	return true;
}

bool CGuildMarkDownloader::__SendMarkCRCList()
{
	TPacketCGMarkCRCList kPacketMarkCRCList;

	if (!CGuildMarkManager::Instance().GetBlockCRCList(m_currentRequestingImageIndex, kPacketMarkCRCList.crclist))
		__CompleteMarkState_Set();
	else
	{
		kPacketMarkCRCList.header = HEADER_CG_MARK_CRCLIST;
		kPacketMarkCRCList.imgIdx = m_currentRequestingImageIndex;
		++m_currentRequestingImageIndex;

		if (!Send(sizeof(kPacketMarkCRCList), &kPacketMarkCRCList))
			return false;
	}
	return true;
}

bool CGuildMarkDownloader::__LoginState_RecvMarkBlock()
{
	TPacketGCMarkBlock kPacket;

	if (!Peek(sizeof(kPacket), &kPacket))
		return false;

	if (!Peek(kPacket.bufSize))
		return false;

	Recv(sizeof(kPacket));

	uint8_t posBlock;
	uint32_t compSize;
	char compBuf[SGuildMarkBlock::MAX_COMP_SIZE];

	for (uint32_t i = 0; i < kPacket.count; ++i)
	{
		Recv(sizeof(uint8_t), &posBlock);
		Recv(sizeof(uint32_t), &compSize);

		if (compSize > SGuildMarkBlock::MAX_COMP_SIZE)
		{
			TraceError("RecvMarkBlock: data corrupted");
			Recv(compSize);
		}
		else
		{
			Recv(compSize, compBuf);
			// 압축된 이미지를 실제로 저장한다. CRC등 여러가지 정보가 함께 빌드된다.
			CGuildMarkManager::Instance().SaveBlockFromCompressedData(kPacket.imgIdx, posBlock, (const uint8_t *) compBuf, compSize);
		}
	}

	if (kPacket.count > 0)
	{
		// 마크 이미지 저장
		CGuildMarkManager::Instance().SaveMarkImage(kPacket.imgIdx);

		// 리소스 리로딩 (재접속을 안해도 본인것은 잘 보이게 함)
		std::string imagePath;

		if (CGuildMarkManager::Instance().GetMarkImageFilename(kPacket.imgIdx, imagePath))
		{
			CResource * pResource = CResourceManager::Instance().GetResourcePointer(imagePath.c_str());
			if (pResource->IsType(CGraphicImage::Type()))
			{
				CGraphicImage* pkGrpImg=static_cast<CGraphicImage*>(pResource);
				pkGrpImg->Reload();
			}
		}
	}

	// 더 요청할 것이 있으면 요청하고 아니면 이미지를 저장하고 종료
	if (m_currentRequestingImageIndex < CGuildMarkManager::Instance().GetMarkImageCount())
		__SendMarkCRCList();
	else
		__CompleteMarkState_Set();

	return true;
}
// END_OF_MARK_BUG_FIX

#ifdef _IMPROVED_PACKET_ENCRYPTION_
bool CGuildMarkDownloader::__LoginState_RecvKeyAgreement()
{
	TPacketKeyAgreement packet;
	if (!Recv(sizeof(packet), &packet))
		return false;

	Tracenf("KEY_AGREEMENT RECV %u", packet.wDataLength);

	TPacketKeyAgreement packetToSend;
	size_t dataLength = TPacketKeyAgreement::MAX_DATA_LEN;
	size_t agreedLength = Prepare(packetToSend.data, &dataLength);
	if (agreedLength == 0)
	{
		// 초기화 실패
		Disconnect();
		return false;
	}
	assert(dataLength <= TPacketKeyAgreement::MAX_DATA_LEN);

	if (Activate(packet.wAgreedLength, packet.data, packet.wDataLength))
	{
		// Key agreement 성공, 응답 전송
		packetToSend.bHeader = HEADER_CG_KEY_AGREEMENT;
		packetToSend.wAgreedLength = static_cast<uint16_t>(agreedLength);
		packetToSend.wDataLength = static_cast<uint16_t>(dataLength);

		if (!Send(sizeof(packetToSend), &packetToSend))
		{
			Tracen(" CAccountConnector::__AuthState_RecvKeyAgreement - SendKeyAgreement Error");
			return false;
		}
		Tracenf("KEY_AGREEMENT SEND %u", packetToSend.wDataLength);
	}
	else
	{
		// 키 협상 실패
		Disconnect();
		return false;
	}
	return true;
}

bool CGuildMarkDownloader::__LoginState_RecvKeyAgreementCompleted()
{
	TPacketKeyAgreementCompleted packet;
	if (!Recv(sizeof(packet), &packet))
		return false;

	Tracenf("KEY_AGREEMENT_COMPLETED RECV");

	ActivateCipher();

	return true;
}
#endif // _IMPROVED_PACKET_ENCRYPTION_

bool CGuildMarkDownloader::__SendSymbolCRCList()
{
	if (m_kSet_dwGuildID.empty())
	{
		m_dwSymbolLastGuildID = 0;
		return true;
	}
	m_dwSymbolLastGuildID = *--m_kSet_dwGuildID.end();

	for (auto guildID : m_kSet_dwGuildID)
	{
		TPacketCGSymbolCRC kSymbolCRCPacket;
		kSymbolCRCPacket.header = HEADER_CG_GUILD_SYMBOL_CRC;
		kSymbolCRCPacket.dwGuildID = guildID;
		kSymbolCRCPacket.isLastEntry = (m_dwSymbolLastGuildID == guildID) ? 1 : 0;

		std::string strFileName = GetGuildSymbolFileName(guildID);
		kSymbolCRCPacket.dwCRC = GetFileCRC32(strFileName.c_str());
		kSymbolCRCPacket.dwSize = GetFileSize(strFileName.c_str());
#ifdef _DEBUG
		printf("__SendSymbolCRCList [GuildID:%d / CRC:%u]\n", guildID, kSymbolCRCPacket.dwCRC);
#endif

		if (!Send(sizeof(kSymbolCRCPacket), &kSymbolCRCPacket)) {
			TraceError("__SendSymbolCRCList:: failed to send TPacketCGSymbolCRC packet data: (gid: %d, crc: %d, size: %d)", kSymbolCRCPacket.dwGuildID, kSymbolCRCPacket.dwCRC, kSymbolCRCPacket.dwSize);
			return false;
		}
	}

	return true;
}

bool CGuildMarkDownloader::__LoginState_RecvSymbolData()
{
	//Lets peek the size of the packet and find out if it is too big for our receive buffer
	TPacketGCBlankDynamic packet;
	if (!Peek(sizeof(TPacketGCBlankDynamic), &packet))
		return false;

#ifdef _DEBUG
	printf("__LoginState_RecvSymbolData [%d/%d]\n", GetRecvBufferSize(), packet.size);
#endif
	if (packet.size > GetRecvBufferSize())
		return true;

	//////////////////////////////////////////////////////////////

	TPacketGCGuildSymbolData kPacketSymbolData;
	if (!Recv(sizeof(kPacketSymbolData), &kPacketSymbolData))
		return false;

	uint32_t dwGuildID = kPacketSymbolData.guild_id;

	// We set the complete state as early as possible to avoid further errors
	if (m_dwSymbolLastGuildID == dwGuildID)
		__CompleteSymbolState_Set();

	// The symbol was the same or there was no symbol.
	if (kPacketSymbolData.size == 0)
		return false;

	uint16_t wDataSize = kPacketSymbolData.size - sizeof(kPacketSymbolData);

	std::vector<uint8_t> pbyBuf(wDataSize);
	if (!Recv(wDataSize, pbyBuf.data()))
		return false;

	MyCreateDirectory(g_strGuildSymbolPathName.c_str());

	std::string strFileName = GetGuildSymbolFileName(dwGuildID);

	msl::file_ptr fPtr(strFileName.c_str(), "wb");
	if (!fPtr)
		return false;
	fwrite(pbyBuf.data(), wDataSize, 1, fPtr.get());

	// Let's reload the file in the game
	CResource * pResource = CResourceManager::Instance().GetResourcePointer(strFileName.c_str());
	if (pResource->IsType(CGraphicImage::Type()))
	{
		CGraphicImage* pkGrpImg = static_cast<CGraphicImage*>(pResource);
		pkGrpImg->Reload();
	}

#ifdef _DEBUG
	printf("__LoginState_RecvSymbolData(filename:%s, datasize:%d, guildid:%d)\n", strFileName.c_str(), wDataSize, dwGuildID);
#endif

	return true;
}
