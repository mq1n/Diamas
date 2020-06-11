#pragma once
#include "../eterLib/NetStream.h"
#include "MarkImage.h"
#include <il/il.h>

class CGuildMarkUploader : public CNetworkStream, public CSingleton<CGuildMarkUploader>
{
	public:
		enum
		{
			ERROR_NONE,
			ERROR_CONNECT,
			ERROR_LOAD,
			ERROR_WIDTH,
			ERROR_HEIGHT
		};

		enum
		{
			SEND_TYPE_MARK,
			SEND_TYPE_SYMBOL
		};

	public:
		CGuildMarkUploader();
		virtual ~CGuildMarkUploader();

		void Disconnect();
		bool Connect(const CNetworkAddress& c_rkNetAddr, uint32_t dwHandle, uint32_t dwRandomKey, uint32_t dwGuildID, const char* c_szFileName, uint32_t* peError);
		bool ConnectToSendSymbol(const CNetworkAddress& c_rkNetAddr, uint32_t dwHandle, uint32_t dwRandomKey, uint32_t dwGuildID, const char* c_szFileName, uint32_t* peError);

		void Process();

	private:
		enum
		{
			STATE_OFFLINE,
			STATE_LOGIN,
			STATE_COMPLETE
		};

	private:
		void OnConnectFailure();
		void OnConnectSuccess();
		void OnRemoteDisconnect();
		void OnDisconnect();

		bool __Load(const char* c_szFileName, uint32_t* peError);
		bool __LoadSymbol(const char* c_szFileName, uint32_t* peError);

		void __Inialize();
		bool __StateProcess();

		void __OfflineState_Set();
		void __CompleteState_Set();

		void __LoginState_Set();
		bool __LoginState_Process();
		bool __LoginState_RecvPhase();
		bool __LoginState_RecvHandshake();
		bool __LoginState_RecvPing();
#ifdef _IMPROVED_PACKET_ENCRYPTION_
		bool __LoginState_RecvKeyAgreement();
		bool __LoginState_RecvKeyAgreementCompleted();
#endif

		bool __AnalyzePacket(uint32_t uHeader, uint32_t uPacketSize, bool (CGuildMarkUploader::*pfnDispatchPacket)());

		bool __SendMarkPacket();
		bool __SendSymbolPacket();

	private:
		uint32_t m_eState;

		uint32_t m_dwSendType;
		uint32_t m_dwHandle;
		uint32_t m_dwRandomKey;
		uint32_t m_dwGuildID;

		SGuildMark m_kMark;

		uint32_t m_dwSymbolCRC32;
		std::vector<uint8_t> m_pbySymbolBuf;
};
