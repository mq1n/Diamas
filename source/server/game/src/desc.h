#ifndef __INC_METIN_II_GAME_DESC_H__
#define __INC_METIN_II_GAME_DESC_H__

#include "constants.h"
#include "input.h"
#ifdef _IMPROVED_PACKET_ENCRYPTION_
#include "cipher.h"
#endif

#define MAX_ALLOW_USER                  4096
//#define MAX_INPUT_LEN			2048
#define MAX_INPUT_LEN			65536

#define HANDSHAKE_RETRY_LIMIT		32

class CInputProcessor;

enum EDescType
{
	DESC_TYPE_ACCEPTOR,
	DESC_TYPE_CONNECTOR
};

class CLoginKey
{
	public:
		CLoginKey(uint32_t dwKey, LPDESC pkDesc) : m_dwKey(dwKey), m_pkDesc(pkDesc)
		{
			m_dwExpireTime = 0;
		}

		void Expire()
		{
			m_dwExpireTime = get_dword_time();
			m_pkDesc = nullptr;
		}

		operator uint32_t() const
		{
			return m_dwKey;
		}

		uint32_t   m_dwKey;
		uint32_t   m_dwExpireTime;
		LPDESC  m_pkDesc;
};

class DESC
{
	public:
		EVENTINFO(desc_event_info)
		{
			LPDESC desc;

			desc_event_info() 
			: desc(0)
			{
			}
		};

	public:
		DESC();
		virtual ~DESC();

		virtual uint8_t		GetType() { return DESC_TYPE_ACCEPTOR; }
		virtual void		Destroy();
		virtual void		SetPhase(int32_t _phase);

		void			FlushOutput();

		bool			Setup(LPFDWATCH _fdw, socket_t _fd, const struct sockaddr_in & c_rSockAddr, uint32_t _handle, uint32_t _handshake);

		socket_t		GetSocket() const	{ return m_sock; }
		const char *	GetHostName()		{ return m_stHost.c_str(); }
		uint16_t			GetPort()	{ return m_wPort; }

		void			SetListenPort(uint16_t w) { m_wListenPort = w; }
		uint16_t		GetListenPort() { return m_wListenPort; }

		void			SetP2P(const char * h, uint16_t w, uint8_t b) { m_stP2PHost = h; m_wP2PPort = w; m_bP2PChannel = b; }
		const char *	GetP2PHost()		{ return m_stP2PHost.c_str();	}
		uint16_t			GetP2PPort() const		{ return m_wP2PPort; }
		uint8_t			GetP2PChannel() const	{ return m_bP2PChannel;	}

		void			BufferedPacket(const void * c_pvData, int32_t iSize);
		void			Packet(const void * c_pvData, int32_t iSize);
		void			LargePacket(const void * c_pvData, int32_t iSize);

		int32_t			ProcessInput();		// returns -1 if error
		int32_t			ProcessOutput();	// returns -1 if error

		CInputProcessor	*	GetInputProcessor()	{ return m_pInputProcessor; }

		uint32_t			GetHandle() const	{ return m_dwHandle; }
		LPBUFFER		GetOutputBuffer()	{ return m_lpOutputBuffer; }

		void			BindAccountTable(TAccountTable * pTable);
		TAccountTable &		GetAccountTable()	{ return m_accountTable; }

		void			BindCharacter(LPCHARACTER ch);
		LPCHARACTER		GetCharacter()		{ return m_lpCharacter; }

		bool			IsPhase(int32_t phase) const	{ return m_iPhase == phase ? true : false; }
		int32_t			GetPhase() { return m_iPhase; };

		const struct sockaddr_in & GetAddr()		{ return m_SockAddr;	}

		void			Log(const char * format, ...);

		// 핸드쉐이크 (시간 동기화)
		void			StartHandshake(uint32_t _dw);
		void			SendHandshake(uint32_t dwCurTime, int32_t lNewDelta);
		bool			HandshakeProcess(uint32_t dwTime, int32_t lDelta, bool bInfiniteRetry=false);
		bool			IsHandshaking();

		uint32_t			GetHandshake() const	{ return m_dwHandshake; }
		uint32_t			GetClientTime();

#ifdef _IMPROVED_PACKET_ENCRYPTION_
		void SendKeyAgreement();
		void SendKeyAgreementCompleted();
		bool FinishHandshake(size_t agreed_length, const void* buffer, size_t length);
		bool IsCipherPrepared();
#else
		// Obsolete encryption stuff here
		void			SetSecurityKey(const uint32_t * c_pdwKey);
		const uint32_t *	GetEncryptionKey() const { return &m_adwEncryptionKey[0]; }
		const uint32_t *	GetDecryptionKey() const { return &m_adwDecryptionKey[0]; }
#endif

		// 제국
		uint8_t			GetEmpire();

		// for p2p
		void			SetRelay(const char * c_pszName);
		bool			DelayedDisconnect(int32_t iSec);
		void			DisconnectOfSameLogin();

		void			SetAdminMode();
		bool			IsAdminMode();		// Handshake 에서 어드민 명령을 쓸수있나?

		void			SetPong(bool b);
		bool			IsPong();

		void			SendLoginSuccessPacket();
		//void			SendServerStatePacket(int32_t nIndex);

		void			SetLoginKey(uint32_t dwKey);
		void			SetLoginKey(CLoginKey * pkKey);
		uint32_t			GetLoginKey();

		bool			isChannelStatusRequested() const { return m_bChannelStatusRequested; }
		void			SetChannelStatusRequested(bool bChannelStatusRequested) { m_bChannelStatusRequested = bChannelStatusRequested; }

		void			SetCloseReason(const std::string &reason) { m_closeReason = reason; }
		std::string		GetCloseReason() const { return m_closeReason; }

	protected:
		void			Initialize();

	protected:
		CInputProcessor *	m_pInputProcessor;
		CInputClose		m_inputClose;
		CInputHandshake	m_inputHandshake;
		CInputLogin		m_inputLogin;
		CInputMain		m_inputMain;
		CInputAuth		m_inputAuth;


		LPFDWATCH		m_lpFdw;
		socket_t		m_sock;
		int32_t				m_iPhase;
		uint32_t			m_dwHandle;

		std::string		m_stHost;
		uint16_t			m_wPort;
		uint16_t			m_wListenPort;
		time_t			m_LastTryToConnectTime;

		LPBUFFER		m_lpInputBuffer;
		int32_t				m_iMinInputBufferLen;
	
		uint32_t			m_dwHandshake;
		uint32_t			m_dwHandshakeSentTime;
		int32_t				m_iHandshakeRetry;
		uint32_t			m_dwClientTime;
		bool			m_bHandshaking;

		LPBUFFER		m_lpBufferedOutputBuffer;
		LPBUFFER		m_lpOutputBuffer;

		LPEVENT			m_pkPingEvent;
		LPCHARACTER		m_lpCharacter;
		TAccountTable		m_accountTable;

		struct sockaddr_in	m_SockAddr;

		FILE *			m_pLogFile;
		std::string		m_stRelayName;

		std::string		m_stP2PHost;
		uint16_t			m_wP2PPort;
		uint8_t			m_bP2PChannel;

		bool			m_bAdminMode; // Handshake 에서 어드민 명령을 쓸수있나?
		bool			m_bPong;

		CLoginKey *		m_pkLoginKey;
		uint32_t			m_dwLoginKey;

		std::string		m_Login;

		bool			m_bDestroyed;
		bool			m_bChannelStatusRequested;

		std::string		m_closeReason;

#ifdef _IMPROVED_PACKET_ENCRYPTION_
		Cipher cipher_;
#else
		// Obsolete encryption stuff here
		bool			m_bEncrypted;
		uint32_t			m_adwDecryptionKey[4];
		uint32_t			m_adwEncryptionKey[4];
#endif

	public:
		LPEVENT			m_pkDisconnectEvent;

		void SetLogin( const std::string & login ) { m_Login = login; }
		void SetLogin( const char * login ) { m_Login = login; }
		const std::string& GetLogin() { return m_Login; }

		void RawPacket(const void * c_pvData, int32_t iSize);
		void ChatPacket(uint8_t type, const char * format, ...);
};

#endif
