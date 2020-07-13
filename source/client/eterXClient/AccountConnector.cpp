#include "StdAfx.h"
#include "AccountConnector.h"
#include "Packet.h"
#include "PythonNetworkStream.h"
#include "PythonApplication.h"
#include "../eterBase/tea.h"

// CHINA_CRYPT_KEY
uint32_t g_adwEncryptKey[4];
uint32_t g_adwDecryptKey[4];
// END_OF_CHINA_CRYPT_KEY

inline const uint8_t* GetSpecialKey()
{
	volatile static uint32_t s_adwKey[1938];

	volatile uint32_t seed = 1491971513;
	for (uint32_t i = 0; i < uint8_t(seed); i++)
	{
		seed ^= 2148941891;
		seed += 3592385981;
		s_adwKey[i] = seed;
	}

	return (const uint8_t*)s_adwKey;
}

void CAccountConnector::SetHandler(PyObject* poHandler)
{
	m_poHandler = poHandler;
}

void CAccountConnector::SetLoginInfo(const char * c_szName, const char * c_szPwd)
{
	m_strID = c_szName;
	m_strPassword = c_szPwd;
}


void CAccountConnector::ClearLoginInfo()
{
	m_strPassword.clear();
}

void CAccountConnector::__BuildSpecialClientKey()
{
	const uint8_t* c_pszKey = GetSpecialKey();
	memcpy(g_adwEncryptKey, c_pszKey + 157, 16);

	for (uint32_t i = 0; i < 4; ++i)
		g_adwEncryptKey[i] = random();

	tea_encrypt((uint32_t*)g_adwDecryptKey, (const uint32_t*)g_adwEncryptKey, (const uint32_t*)(c_pszKey + 37), 16);
	//	TEA_Encrypt((uint32_t *) g_adwDecryptKey, (const uint32_t *) g_adwEncryptKey, (const uint32_t *) (c_pszKey+37), 16);
}

bool CAccountConnector::Connect(const char * c_szAddr, int32_t iPort, const char * c_szAccountAddr, int32_t iAccountPort)
{
#ifndef _IMPROVED_PACKET_ENCRYPTION_
	__BuildClientKey();
#endif

	m_strAddr = c_szAddr;
	m_iPort = iPort;

	__OfflineState_Set();

	// CHINA_CRYPT_KEY
	__BuildSpecialClientKey();
	// END_OF_CHINA_CRYPT_KEY

	return CNetworkStream::Connect(c_szAccountAddr, iAccountPort);
}

void CAccountConnector::Disconnect()
{
	CNetworkStream::Disconnect();
	__OfflineState_Set();
}

void CAccountConnector::Process()
{
	CNetworkStream::Process();

	if (!__StateProcess())
	{
		__OfflineState_Set();
		Disconnect();
	}
}

bool CAccountConnector::__StateProcess()
{
	switch (m_eState)
	{
		case STATE_HANDSHAKE:
			return __HandshakeState_Process();
		case STATE_AUTH:
			return __AuthState_Process();
	}

	return true;
}

bool CAccountConnector::__HandshakeState_Process()
{
	if (!__AnalyzePacket(HEADER_GC_PHASE, sizeof(SPacketGCPhase), &CAccountConnector::__AuthState_RecvPhase))
		return false;

	if (!__AnalyzePacket(HEADER_GC_HANDSHAKE, sizeof(SPacketHandshake), &CAccountConnector::__AuthState_RecvHandshake))
		return false;

	if (!__AnalyzePacket(HEADER_GC_PING, sizeof(SPacketGCPing), &CAccountConnector::__AuthState_RecvPing))
		return false;

#ifdef _IMPROVED_PACKET_ENCRYPTION_
	if (!__AnalyzePacket(HEADER_GC_KEY_AGREEMENT, sizeof(SPacketKeyAgreement), &CAccountConnector::__AuthState_RecvKeyAgreement))
		return false;

	if (!__AnalyzePacket(HEADER_GC_KEY_AGREEMENT_COMPLETED, sizeof(SPacketGCKeyAgreementCompleted), &CAccountConnector::__AuthState_RecvKeyAgreementCompleted))
		return false;
#endif

	return true;
}

bool CAccountConnector::__AuthState_Process()
{
	if (!__AnalyzePacket(0, sizeof(uint8_t), &CAccountConnector::__AuthState_RecvEmpty))
		return true;

	if (!__AnalyzePacket(HEADER_GC_PHASE, sizeof(SPacketGCPhase), &CAccountConnector::__AuthState_RecvPhase))
		return false;

	if (!__AnalyzePacket(HEADER_GC_PING, sizeof(SPacketGCPing), &CAccountConnector::__AuthState_RecvPing))
		return false;

	if (!__AnalyzePacket(HEADER_GC_AUTH_SUCCESS, sizeof(SPacketGCAuthSuccess), &CAccountConnector::__AuthState_RecvAuthSuccess))
		return true;

	if (!__AnalyzePacket(HEADER_GC_LOGIN_FAILURE, sizeof(SPacketGCAuthSuccess), &CAccountConnector::__AuthState_RecvAuthFailure))
		return true;

	if (!__AnalyzePacket(HEADER_GC_HANDSHAKE, sizeof(SPacketHandshake), &CAccountConnector::__AuthState_RecvHandshake))
		return false;

#ifdef _IMPROVED_PACKET_ENCRYPTION_
	if (!__AnalyzePacket(HEADER_GC_KEY_AGREEMENT, sizeof(SPacketKeyAgreement), &CAccountConnector::__AuthState_RecvKeyAgreement))
		return false;

	if (!__AnalyzePacket(HEADER_GC_KEY_AGREEMENT_COMPLETED, sizeof(SPacketGCKeyAgreementCompleted), &CAccountConnector::__AuthState_RecvKeyAgreementCompleted))
		return false;
#endif

	return true;
}

bool CAccountConnector::__AuthState_RecvEmpty()
{
	uint8_t byEmpty;
	Recv(sizeof(uint8_t), &byEmpty);
	return true;
}

bool CAccountConnector::__AuthState_RecvPhase()
{
	SPacketGCPhase kPacketPhase;
	if (!Recv(sizeof(kPacketPhase), &kPacketPhase))
		return false;

	CPythonApplication::Instance().SetGameStage(kPacketPhase.stage);

	if (kPacketPhase.phase == PHASE_HANDSHAKE)
	{
		__HandshakeState_Set();
	}
	else if (kPacketPhase.phase == PHASE_AUTH)
	{
#ifndef _IMPROVED_PACKET_ENCRYPTION_
		const char* key = LocaleService_GetSecurityKey();
		SetSecurityMode(true, key);
#endif

		SPacketCGLogin3 LoginPacket;
		strncpy_s(LoginPacket.name, m_strID.c_str(), LOGIN_MAX_LEN);
		strncpy_s(LoginPacket.pwd, m_strPassword.c_str(), PASSWD_MAX_LEN);
		LoginPacket.name[LOGIN_MAX_LEN] = '\0';
		LoginPacket.pwd[PASSWD_MAX_LEN] = '\0';

		LoginPacket.version = CLIENT_VERSION_TIMESTAMP;
		
		// 비밀번호를 메모리에 계속 갖고 있는 문제가 있어서, 사용 즉시 날리는 것으로 변경
		ClearLoginInfo();
		CPythonNetworkStream& rkNetStream=CPythonNetworkStream::Instance();
		rkNetStream.ClearLoginInfo();

		m_strPassword.clear();

		for (uint32_t i = 0; i < 4; ++i)
			LoginPacket.adwClientKey[i] = g_adwEncryptKey[i];

		if (!Send(sizeof(LoginPacket), &LoginPacket))
		{
			Tracen(" CAccountConnector::__AuthState_RecvPhase - SendLogin3 Error");
			return false;
		}

		__AuthState_Set();
	}

	return true;
}

bool CAccountConnector::__AuthState_RecvHandshake()
{
	SPacketHandshake kPacketHandshake;
	if (!Recv(sizeof(kPacketHandshake), &kPacketHandshake))
		return false;

	// HandShake
	{
		Tracenf("HANDSHAKE RECV %u %d", kPacketHandshake.dwTime, kPacketHandshake.lDelta);

		ELTimer_SetServerMSec(kPacketHandshake.dwTime+ kPacketHandshake.lDelta);

		//uint32_t dwBaseServerTime = kPacketHandshake.dwTime+ kPacketHandshake.lDelta;
		//uint32_t dwBaseClientTime = ELTimer_GetMSec();

		kPacketHandshake.dwTime = kPacketHandshake.dwTime + kPacketHandshake.lDelta + kPacketHandshake.lDelta;
		kPacketHandshake.lDelta = 0;

		Tracenf("HANDSHAKE SEND %u", kPacketHandshake.dwTime);

		if (!Send(sizeof(kPacketHandshake), &kPacketHandshake))
		{
			Tracen(" CAccountConnector::__AuthState_RecvHandshake - SendHandshake Error");
			return false;
		}
	}

	return true;
}

bool CAccountConnector::__AuthState_RecvPing()
{
	SPacketGCPing kPacketPing;
	if (!Recv(sizeof(kPacketPing), &kPacketPing))
		return false;

	__AuthState_SendPong();

	return true;
}

bool CAccountConnector::__AuthState_SendPong()
{
	SPacketCGPong kPacketPong;
	if (!Send(sizeof(kPacketPong), &kPacketPong))
		return false;
		
	return true;
}

bool CAccountConnector::__AuthState_RecvAuthSuccess()
{
	SPacketGCAuthSuccess kAuthSuccessPacket;
	if (!Recv(sizeof(kAuthSuccessPacket), &kAuthSuccessPacket))
		return false;

	if (!kAuthSuccessPacket.bResult)
	{
		if (m_poHandler)
			PyCallClassMemberFunc(m_poHandler, "OnLoginFailure", Py_BuildValue("(s)", "BESAMEKEY"));
	}
	else
	{
		CPythonNetworkStream & rkNet = CPythonNetworkStream::Instance();
		rkNet.SetLoginKey(kAuthSuccessPacket.dwLoginKey);
		rkNet.Connect(m_strAddr.c_str(), m_iPort);
	}

	Disconnect();
	__OfflineState_Set();

	return true;
}

bool CAccountConnector::__AuthState_RecvAuthFailure()
{
	SPacketGCLoginFailure packet_failure;
	if (!Recv(sizeof(packet_failure), &packet_failure))
		return false;

	if (m_poHandler)
		PyCallClassMemberFunc(m_poHandler, "OnLoginFailure", Py_BuildValue("(s)", packet_failure.szStatus));

//	__OfflineState_Set();

	return true;
}

#ifdef _IMPROVED_PACKET_ENCRYPTION_
bool CAccountConnector::__AuthState_RecvKeyAgreement()
{
	SPacketKeyAgreement packet;
	if (!Recv(sizeof(packet), &packet))
		return false;

	Tracenf("KEY_AGREEMENT RECV %u", packet.wDataLength);

	SPacketKeyAgreement packetToSend;
	size_t dataLength = SPacketKeyAgreement::MAX_DATA_LEN;
	size_t agreedLength = Prepare(packetToSend.data, &dataLength);
	if (agreedLength == 0)
	{
		// 초기화 실패
		Disconnect();
		return false;
	}
	assert(dataLength <= SPacketKeyAgreement::MAX_DATA_LEN);

	if (Activate(packet.wAgreedLength, packet.data, packet.wDataLength))
	{
		// Key agreement 성공, 응답 전송
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

bool CAccountConnector::__AuthState_RecvKeyAgreementCompleted()
{
	SPacketGCKeyAgreementCompleted packet;
	if (!Recv(sizeof(packet), &packet))
		return false;

	Tracenf("KEY_AGREEMENT_COMPLETED RECV");

	ActivateCipher();

	return true;
}
#endif // _IMPROVED_PACKET_ENCRYPTION_

bool CAccountConnector::__AnalyzePacket(uint32_t uHeader, uint32_t uPacketSize, bool (CAccountConnector::*pfnDispatchPacket)())
{
	uint8_t bHeader;
	if (!Peek(sizeof(bHeader), &bHeader))
		return true;

	if (bHeader!=uHeader)
		return true;

	if (!Peek(uPacketSize))
		return true;

	return (this->*pfnDispatchPacket)();
}

bool CAccountConnector::__AnalyzeVarSizePacket(uint32_t uHeader, bool (CAccountConnector::*pfnDispatchPacket)(int32_t))
{
	uint8_t bHeader;
	if (!Peek(sizeof(bHeader), &bHeader))
		return true;

	if (bHeader!=uHeader)
		return true;

	SDynamicSizePacketHeader dynamicHeader;

	if (!Peek(sizeof(dynamicHeader), &dynamicHeader))
		return true;

	if (!Peek(dynamicHeader.size))
		return true;

	return (this->*pfnDispatchPacket)(dynamicHeader.size);
}


void CAccountConnector::__OfflineState_Set()
{
	__Inialize();
}

void CAccountConnector::__HandshakeState_Set()
{
	m_eState=STATE_HANDSHAKE;
}

void CAccountConnector::__AuthState_Set()
{
	m_eState=STATE_AUTH;
}

void CAccountConnector::OnConnectFailure()
{
	if (m_poHandler)
		PyCallClassMemberFunc(m_poHandler, "OnConnectFailure", Py_BuildValue("()"));

	__OfflineState_Set();
}

void CAccountConnector::OnConnectSuccess()
{
	m_eState = STATE_HANDSHAKE;
}

void CAccountConnector::OnRemoteDisconnect()
{
	__OfflineState_Set();
}

void CAccountConnector::OnDisconnect()
{
	__OfflineState_Set();
}

#ifndef _IMPROVED_PACKET_ENCRYPTION_
void CAccountConnector::__BuildClientKey()
{
	for (uint32_t i = 0; i < 4; ++i)
		g_adwEncryptKey[i] = random();

	const uint8_t * c_pszKey = (const uint8_t *) "JyTxtHljHJlVJHorRM301vf@4fvj10-v";
	tea_encrypt((uint32_t *) g_adwDecryptKey, (const uint32_t *) g_adwEncryptKey, (const uint32_t *) c_pszKey, 16);
}
#endif

void CAccountConnector::__Inialize()
{
	m_eState=STATE_OFFLINE;
}

CAccountConnector::CAccountConnector()
{
	m_poHandler = nullptr;
	m_strAddr.clear();
	m_iPort = 0;

	SetLoginInfo("", "");
	SetRecvBufferSize(1024 * 128);
	SetSendBufferSize(2048);
	__Inialize();
}

CAccountConnector::~CAccountConnector()
{
	__OfflineState_Set();
}
