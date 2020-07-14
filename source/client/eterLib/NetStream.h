#pragma once

#include "../../common/defines.h"

#ifdef _IMPROVED_PACKET_ENCRYPTION_
#include "../eterBase/cipher.h"
#endif
#include "../eterBase/tea.h"
#include "NetAddress.h"
#include <vector>
#ifndef VC_EXTRALEAN
	#include <winsock.h>
#endif

class CNetworkStream
{
	public:
		CNetworkStream();
		virtual ~CNetworkStream();		

		void SetRecvBufferSize(int32_t recvBufSize);
		void SetSendBufferSize(int32_t sendBufSize);

#ifndef _IMPROVED_PACKET_ENCRYPTION_
		void SetSecurityMode(bool isSecurityMode, const char* c_szTeaKey);
		void SetSecurityMode(bool isSecurityMode, const char* c_szTeaEncryptKey, const char* c_szTeaDecryptKey);
#endif
		bool IsSecurityMode();

		int32_t	GetRecvBufferSize();

		void Clear();
		void ClearRecvBuffer();

		void Process();

		bool Connect(const CNetworkAddress& c_rkNetAddr, int32_t limitSec = 3);
		bool Connect(const char* c_szAddr, int32_t port, int32_t limitSec = 3);
		bool Connect(uint32_t dwAddr, int32_t port, int32_t limitSec = 3);
		void Disconnect();

		bool Peek(int32_t len);
		bool Peek(int32_t len, char* pDestBuf);
		bool Recv(int32_t len);
		bool Recv(int32_t len, char* pDestBuf);
		bool Send(int32_t len, const char* pSrcBuf);

		bool Peek(int32_t len, void* pDestBuf);
		bool Recv(int32_t len, void* pDestBuf);

		bool Send(int32_t len, const void* pSrcBuf);
		bool SendFlush(int32_t len, const void* pSrcBuf);

		bool IsOnline();

	protected:			
		virtual void OnConnectSuccess();				
		virtual void OnConnectFailure();
		virtual void OnRemoteDisconnect();
		virtual void OnDisconnect();		
		virtual bool OnProcess();

		bool __SendInternalBuffer();
		bool __RecvInternalBuffer();

		void __PopSendBuffer();

		int32_t __GetSendBufferSize();

#ifdef _IMPROVED_PACKET_ENCRYPTION_
		size_t Prepare(void* buffer, size_t* length);
		bool Activate(size_t agreed_length, const void* buffer, size_t length);
		void ActivateCipher();
#endif

	private:
		time_t	m_connectLimitTime;

		char*	m_recvTEABuf;
		int32_t		m_recvTEABufInputPos;
		int32_t		m_recvTEABufSize;

		char*	m_recvBuf;
		int32_t		m_recvBufSize;
		int32_t		m_recvBufInputPos;
		int32_t		m_recvBufOutputPos;

		char*	m_sendBuf;
		int32_t		m_sendBufSize;
		int32_t		m_sendBufInputPos;
		int32_t		m_sendBufOutputPos;

		char*	m_sendTEABuf;
		int32_t		m_sendTEABufSize;
		int32_t		m_sendTEABufInputPos;

		bool	m_isOnline;

#ifdef _IMPROVED_PACKET_ENCRYPTION_
		Cipher	m_cipher;
#else
		// Obsolete encryption stuff here
		bool	m_isSecurityMode;
		char	m_szEncryptKey[TEA_KEY_LENGTH]; // Client 에서 보낼 패킷을 Encrypt 할때 사용하는 Key
		char	m_szDecryptKey[TEA_KEY_LENGTH]; // Server 에서 전송된 패킷을 Decrypt 할때 사용하는 Key
#endif

		SOCKET	m_sock;

		CNetworkAddress m_addr;
};
