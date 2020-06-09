#include "stdafx.h" 
#include "constants.h"
#include "config.h"
#include "input.h"
#include "desc_client.h"
#include "desc_manager.h"
#include "protocol.h"
#include "matrix_card.h"
#include "passpod.h"
#include "locale_service.h"
#include "db.h"

#ifndef __WIN32__
	#include "limit_time.h"
#endif

#include "utils.h"


bool FN_IS_VALID_LOGIN_STRING(const char *str)
{
	const char*	tmp;

	if (!str || !*str)
		return false;

	if (strlen(str) < 2)
		return false;

	for (tmp = str; *tmp; ++tmp)
	{
		// ���ĺ��� ���ڸ� ���
		if (isdigit(*tmp) || isalpha(*tmp))
			continue;

#ifdef ENABLE_ACCOUNT_W_SPECIALCHARS
		
		switch (*tmp)
		{
			case ' ':
			case '_':
			case '-':
			case '.':
			case '!':
			case '@':
			case '#':
			case '$':
			case '%':
			case '^':
			case '&':
			case '*':
			case '(':
			case ')':
				continue;
		}
#endif
		return false;
	}

	return true;
}

bool Login_IsInChannelService(const char* c_login)
{
	if (c_login[0] == '[')
		return true;
	return false;
}

CInputAuth::CInputAuth()
{
}

void CInputAuth::Login(LPDESC d, const char * c_pData)
{
	TPacketCGLogin3 * pinfo = (TPacketCGLogin3 *) c_pData;

	if (!g_bAuthServer)
	{
		sys_err ("CInputAuth class is not for game server. IP %s might be a hacker.", 
			inet_ntoa(d->GetAddr().sin_addr));
		d->DelayedDisconnect(5);
		return;
	}

	// string ���Ἲ�� ���� ����
	char login[LOGIN_MAX_LEN + 1];
	trim_and_lower(pinfo->login, login, sizeof(login));

	char passwd[PASSWD_MAX_LEN + 1];
	strlcpy(passwd, pinfo->passwd, sizeof(passwd));

	sys_log(0, "InputAuth::Login : %s(%d) desc %p",
			login, strlen(login), get_pointer(d));

	// check login string
	if (false == FN_IS_VALID_LOGIN_STRING(login))
	{
		sys_log(0, "InputAuth::Login : IS_NOT_VALID_LOGIN_STRING(%s) desc %p",
				login, get_pointer(d));
		LoginFailure(d, "NOID");
		return;
	}

	if (g_bNoMoreClient)
	{
		TPacketGCLoginFailure failurePacket;

		failurePacket.header = HEADER_GC_LOGIN_FAILURE;
		strlcpy(failurePacket.szStatus, "SHUTDOWN", sizeof(failurePacket.szStatus));

		d->Packet(&failurePacket, sizeof(failurePacket));
		return;
	}

	if (DESC_MANAGER::instance().FindByLoginName(login))
	{
		LoginFailure(d, "ALREADY");
		return;
	}

	DWORD dwKey = DESC_MANAGER::instance().CreateLoginKey(d);
	DWORD dwPanamaKey = dwKey ^ pinfo->adwClientKey[0] ^ pinfo->adwClientKey[1] ^ pinfo->adwClientKey[2] ^ pinfo->adwClientKey[3];
	d->SetPanamaKey(dwPanamaKey);

	sys_log(0, "InputAuth::Login : key %u:0x%x login %s", dwKey, dwPanamaKey, login);

	TPacketCGLogin3 * p = M2_NEW TPacketCGLogin3;
	memcpy(p, pinfo, sizeof(TPacketCGLogin3));

	char szPasswd[PASSWD_MAX_LEN * 2 + 1];
	DBManager::instance().EscapeString(szPasswd, sizeof(szPasswd), passwd, strlen(passwd));

	char szLogin[LOGIN_MAX_LEN * 2 + 1];
	DBManager::instance().EscapeString(szLogin, sizeof(szLogin), login, strlen(login));

	// CHANNEL_SERVICE_LOGIN
	if (Login_IsInChannelService(szLogin))
	{
		sys_log(0, "ChannelServiceLogin [%s]", szLogin);

		DBManager::instance().ReturnQuery(QID_AUTH_LOGIN, dwKey, p,
				"SELECT '%s',password,securitycode,social_id,id,status,availDt - NOW() > 0,"
				"UNIX_TIMESTAMP(silver_expire),"
				"UNIX_TIMESTAMP(gold_expire),"
				"UNIX_TIMESTAMP(safebox_expire),"
				"UNIX_TIMESTAMP(autoloot_expire),"
				"UNIX_TIMESTAMP(fish_mind_expire),"
				"UNIX_TIMESTAMP(marriage_fast_expire),"
				"UNIX_TIMESTAMP(money_drop_rate_expire),"
				"UNIX_TIMESTAMP(create_time)"
				" FROM account WHERE login='%s'",

				szPasswd, szLogin);
	}
	// END_OF_CHANNEL_SERVICE_LOGIN
	else
	{
		DBManager::instance().ReturnQuery(QID_AUTH_LOGIN, dwKey, p, 
				"SELECT PASSWORD('%s'),password,securitycode,social_id,id,status,availDt - NOW() > 0,"
				"UNIX_TIMESTAMP(silver_expire),"
				"UNIX_TIMESTAMP(gold_expire),"
				"UNIX_TIMESTAMP(safebox_expire),"
				"UNIX_TIMESTAMP(autoloot_expire),"
				"UNIX_TIMESTAMP(fish_mind_expire),"
				"UNIX_TIMESTAMP(marriage_fast_expire),"
				"UNIX_TIMESTAMP(money_drop_rate_expire),"
				"UNIX_TIMESTAMP(create_time)"
				" FROM account WHERE login='%s'",
				szPasswd, szLogin);
	}
}

int CInputAuth::Analyze(LPDESC d, BYTE bHeader, const char * c_pData)
{

	if (!g_bAuthServer)
	{
		sys_err ("CInputAuth class is not for game server. IP %s might be a hacker.", 
			inet_ntoa(d->GetAddr().sin_addr));
		d->DelayedDisconnect(5);
		return 0;
	}

	int iExtraLen = 0;

	if (test_server)
		sys_log(0, " InputAuth Analyze Header[%d] ", bHeader);

	switch (bHeader)
	{
		case HEADER_CG_PONG:
			Pong(d);
			break;

		case HEADER_CG_LOGIN3:
			Login(d, c_pData);
			break;

		case HEADER_CG_PASSPOD_ANSWER:
			PasspodAnswer(d, c_pData);
			break;

		case HEADER_CG_HANDSHAKE:
			break;

		default:
			sys_err("This phase does not handle this header %d (0x%x)(phase: AUTH)", bHeader, bHeader);
			break;
	}

	return iExtraLen;
}

void CInputAuth::PasspodAnswer(LPDESC d, const char * c_pData)
{

	if (!g_bAuthServer)
	{
		sys_err ("CInputAuth class is not for game server. IP %s might be a hacker.", 
			inet_ntoa(d->GetAddr().sin_addr));
		d->DelayedDisconnect(5);		
		return;
	}

	TPacketCGPasspod * packet = (TPacketCGPasspod*)c_pData;

	RequestConfirmPasspod Confirm;

	memcpy(Confirm.passpod, packet->szAnswer, MAX_PASSPOD + 1);
	memcpy(Confirm.login, d->GetAccountTable().login, LOGIN_MAX_LEN + 1);
	

	if (!d->GetAccountTable().id)
	{
		sys_err("HEADER_CG_PASSPOD_ANSWER received to desc with no account table binded");
		return;
	}   

	int ret_code = 1;
	sys_log(0, "Passpod start %s %s", d->GetAccountTable().login, packet->szAnswer);
	ret_code = CPasspod::instance().ConfirmPasspod(d->GetAccountTable().login, packet->szAnswer);
	
	if (ret_code != 0)
	{
		sys_log(0, "PASSPOD: wrong answer: %s ret_code %d", d->GetAccountTable().login, ret_code);
	
		LoginFailure(d, ERR_MESSAGE[ret_code]);

		if (!d->CheckMatrixTryCount())
		{
			LoginFailure(d, "QUIT");
			d->SetPhase(PHASE_CLOSE);
		}
	}
	else
	{
		sys_log(0, "PASSPOD: success: %s", d->GetAccountTable().login);
		DBManager::instance().SendAuthLogin(d);
	}
//	g_PasspodDesc->DBPacket(HEADER_GP_CONFIRM_PASSPOD,  0, &Confirm, sizeof(Confirm));

//	sys_log(0, "PASSPOD %s %d", Confirm.login, Confirm.passpod);	
}
