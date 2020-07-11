#include "StdAfx.h"
#include "PythonNetworkStream.h"
#include "Packet.h"
#include "AccountConnector.h"

// Login ---------------------------------------------------------------------------
void CPythonNetworkStream::LoginPhase()
{
	TPacketHeader header;
	if (!CheckPacket(&header))
		return;

#if defined(_DEBUG) && defined(ENABLE_PRINT_RECV_PACKET_DEBUG)
	Tracenf("RECV HEADER : %u , phase %s ", header, m_strPhase.c_str());
#endif

	switch (header)
	{
		case HEADER_GC_PHASE:
			if (RecvPhasePacket())
				return;
			break;

	case HEADER_GC_LOGIN_SUCCESS:
		if (__RecvLoginSuccessPacket())
				return;
			break;


		case HEADER_GC_LOGIN_FAILURE:
			if (__RecvLoginFailurePacket())
				return;		
			break;

		case HEADER_GC_EMPIRE:
			if (__RecvEmpirePacket())
				return;
			break;

		case HEADER_GC_LOGIN_KEY:
			if (__RecvLoginKeyPacket())
				return;
			break;

		case HEADER_GC_PING:
			if (RecvPingPacket())
				return;
			break;

		default:
			if (RecvDefaultPacket(header))
				return;
			break;
	}

	RecvErrorPacket(header);
}

void CPythonNetworkStream::SetLoginPhase()
{
	const char* key = LocaleService_GetSecurityKey();
#ifndef _IMPROVED_PACKET_ENCRYPTION_
	SetSecurityMode(true, key);
#endif

	if ("Login" != m_strPhase)
		m_phaseLeaveFunc.Run();

	Tracen("");
	Tracen("## Network - Login Phase ##");
	Tracen("");

	m_strPhase = "Login";	

	m_phaseProcessFunc.Set(this, &CPythonNetworkStream::LoginPhase);
	m_phaseLeaveFunc.Set(this, &CPythonNetworkStream::__LeaveLoginPhase);

	m_dwChangingPhaseTime = ELTimer_GetMSec();

	if (__DirectEnterMode_IsSet())
	{
		if (0 != m_dwLoginKey)
			SendLoginPacketNew(m_stID.c_str(), m_stPassword.c_str());
		else
			SendLoginPacket(m_stID.c_str(), m_stPassword.c_str());

		// 비밀번호를 메모리에 계속 갖고 있는 문제가 있어서, 사용 즉시 날리는 것으로 변경
		ClearLoginInfo();
		CAccountConnector & rkAccountConnector = CAccountConnector::Instance();
		rkAccountConnector.ClearLoginInfo();
	}
	else
	{
		if (0 != m_dwLoginKey)
			SendLoginPacketNew(m_stID.c_str(), m_stPassword.c_str());
		else
			SendLoginPacket(m_stID.c_str(), m_stPassword.c_str());

		// 비밀번호를 메모리에 계속 갖고 있는 문제가 있어서, 사용 즉시 날리는 것으로 변경
		ClearLoginInfo();
		CAccountConnector & rkAccountConnector = CAccountConnector::Instance();
		rkAccountConnector.ClearLoginInfo();

		PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_LOGIN], "OnLoginStart", Py_BuildValue("()"));

		__ClearSelectCharacterData();
	}
}

bool CPythonNetworkStream::__RecvEmpirePacket()
{
	SPacketGCEmpire kPacketEmpire;
	if (!Recv(sizeof(kPacketEmpire), &kPacketEmpire))
		return false;

	m_dwEmpireID=kPacketEmpire.bEmpire;
	return true;
}

bool CPythonNetworkStream::__RecvLoginSuccessPacket()
{
	SPacketGCLoginSuccess kPacketLoginSuccess;

	if (!Recv(sizeof(kPacketLoginSuccess), &kPacketLoginSuccess))
		return false;

	for (int32_t i = 0; i < PLAYER_PER_ACCOUNT; ++i)
	{
		m_akSimplePlayerInfo[i]=kPacketLoginSuccess.akSimplePlayerInformation[i];
		m_adwGuildID[i]=kPacketLoginSuccess.guild_id[i];
		m_astrGuildName[i]=kPacketLoginSuccess.guild_name[i];
	}

	m_kMarkAuth.m_dwHandle=kPacketLoginSuccess.handle;
	m_kMarkAuth.m_dwRandomKey=kPacketLoginSuccess.random_key;	

	if (__DirectEnterMode_IsSet())
	{
	}
	else
	{
		PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_SELECT], "Refresh", Py_BuildValue("()"));		
	}

	return true;
}


void CPythonNetworkStream::OnConnectFailure()
{
	if (__DirectEnterMode_IsSet())
		ClosePhase();
	else
		PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_LOGIN], "OnConnectFailure", Py_BuildValue("()"));
}


bool CPythonNetworkStream::__RecvLoginFailurePacket()
{
	SPacketGCLoginFailure packet_failure;
	if (!Recv(sizeof(SPacketGCLoginFailure), &packet_failure))
		return false;

	PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_LOGIN], "OnLoginFailure", Py_BuildValue("(s)", packet_failure.szStatus));
#ifdef _DEBUG
	Tracef(" RecvLoginFailurePacket : [%s]\n", packet_failure.szStatus);
#endif
	return true;
}

bool CPythonNetworkStream::SendLoginPacket(const char* c_szName, const char* c_szPassword)
{
	SPacketCGLogin LoginPacket;
	strncpy_s(LoginPacket.login, c_szName, sizeof(LoginPacket.login) - 1);
	strncpy_s(LoginPacket.passwd, c_szPassword, sizeof(LoginPacket.passwd) - 1);

	LoginPacket.login[LOGIN_MAX_LEN]='\0';
	LoginPacket.passwd[PASSWD_MAX_LEN]='\0';

	if (!Send(sizeof(LoginPacket), &LoginPacket))
	{
		Tracen("SendLogin Error");
		return false;
	}

	return true;
}

bool CPythonNetworkStream::SendLoginPacketNew(const char * c_szName, const char * c_szPassword)
{
	SPacketCGLogin2 LoginPacket;
	LoginPacket.login_key = m_dwLoginKey;

	strncpy_s(LoginPacket.name, c_szName, sizeof(LoginPacket.name) - 1);
	LoginPacket.name[LOGIN_MAX_LEN]='\0';

	extern uint32_t g_adwEncryptKey[4];
	extern uint32_t g_adwDecryptKey[4];
	for (uint32_t i = 0; i < 4; ++i)
		LoginPacket.adwClientKey[i] = g_adwEncryptKey[i];

	if (!Send(sizeof(LoginPacket), &LoginPacket))
	{
		Tracen("SendLogin Error");
		return false;
	}

	__SendInternalBuffer();

#ifndef _IMPROVED_PACKET_ENCRYPTION_
	SetSecurityMode(true, (const char *) g_adwEncryptKey, (const char *) g_adwDecryptKey);
#endif
	return true;
}

bool CPythonNetworkStream::__RecvLoginKeyPacket()
{
	SPacketGCLoginKey kLoginKeyPacket;
	if (!Recv(sizeof(SPacketGCLoginKey), &kLoginKeyPacket))
		return false;

	m_dwLoginKey = kLoginKeyPacket.dwLoginKey;
	m_isWaitLoginKey = FALSE;

	return true;
}
