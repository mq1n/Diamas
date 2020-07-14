#include "StdAfx.h"
#include "PythonNetworkStream.h"

extern uint32_t g_adwEncryptKey[4];
extern uint32_t g_adwDecryptKey[4];

// Select Character ---------------------------------------------------------------------------
void CPythonNetworkStream::SetSelectPhase()
{
	if ("Select" != m_strPhase)
		m_phaseLeaveFunc.Run();

	Tracen("");
	Tracen("## Network - Select Phase ##");
	Tracen("");

	m_strPhase = "Select";	

#ifndef _IMPROVED_PACKET_ENCRYPTION_
	SetSecurityMode(true, (const char *) g_adwEncryptKey, (const char *) g_adwDecryptKey);
#endif

	m_dwChangingPhaseTime = ELTimer_GetMSec();
	m_phaseProcessFunc.Set(this, &CPythonNetworkStream::SelectPhase);
	m_phaseLeaveFunc.Set(this, &CPythonNetworkStream::__LeaveSelectPhase);

	if (__DirectEnterMode_IsSet())
	{
		PyCallClassMemberFunc(m_poHandler, "SetLoadingPhase", Py_BuildValue("()"));	
	}
	else
	{
		if (IsSelectedEmpire())
			PyCallClassMemberFunc(m_poHandler, "SetSelectCharacterPhase", Py_BuildValue("()"));
		else
			PyCallClassMemberFunc(m_poHandler, "SetSelectEmpirePhase", Py_BuildValue("()"));
	}
}

void CPythonNetworkStream::SelectPhase()
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

		case HEADER_GC_EMPIRE:
			if (__RecvEmpirePacket())
				return;
			break;

	case HEADER_GC_LOGIN_SUCCESS:
		if (__RecvLoginSuccessPacket())
				return;
			break;


		case HEADER_GC_PLAYER_CREATE_SUCCESS:
			if (__RecvPlayerCreateSuccessPacket())
				return;
			break;

		case HEADER_GC_PLAYER_CREATE_FAILURE:
			if (__RecvPlayerCreateFailurePacket())
				return;
			break;

		case HEADER_GC_PLAYER_DELETE_WRONG_SOCIAL_ID:
			if (__RecvPlayerDestroyFailurePacket())
				return;
			break;

		case HEADER_GC_PLAYER_DELETE_SUCCESS:
			if (__RecvPlayerDestroySuccessPacket())
				return;
			break;

		case HEADER_GC_CHANGE_NAME:
			if (__RecvChangeName())
				return;
			break;

		case HEADER_GC_HANDSHAKE:
			RecvHandshakePacket();
			return;

		case HEADER_GC_HANDSHAKE_OK:
			RecvHandshakeOKPacket();
			return;

#ifdef _IMPROVED_PACKET_ENCRYPTION_
		case HEADER_GC_KEY_AGREEMENT:
			RecvKeyAgreementPacket();
			return;

		case HEADER_GC_KEY_AGREEMENT_COMPLETED:
			RecvKeyAgreementCompletedPacket();
			return;
#endif

		case HEADER_GC_PLAYER_POINT_CHANGE:
		{
			SPacketGCPointChange PointChange;
			Recv(sizeof(PointChange), &PointChange);
			return;
		}
			
		case HEADER_GC_CHAT:
			return;

		///////////////////////////////////////////////////////////////////////////////////////////
		case HEADER_GC_PING:
			if (RecvPingPacket())
				return;
			break;
	}

	RecvErrorPacket(header);
}

bool CPythonNetworkStream::SendSelectEmpirePacket(uint32_t dwEmpireID)
{
	SPacketCGEmpire kPacketEmpire;
	kPacketEmpire.bEmpire=dwEmpireID;

	if (!Send(sizeof(kPacketEmpire), &kPacketEmpire))
	{
		Tracen("SendSelectEmpirePacket - Error");
		return false;
	}

	SetEmpireID(dwEmpireID);
	return true;
}

bool CPythonNetworkStream::SendSelectCharacterPacket(uint8_t Index)
{
	SPacketCGSelectCharacter SelectCharacterPacket;
	SelectCharacterPacket.player_index = Index;

	if (!Send(sizeof(SelectCharacterPacket), &SelectCharacterPacket))
	{
		Tracen("SendSelectCharacterPacket - Error");
		return false;
	}

	return true;
}

bool CPythonNetworkStream::SendDestroyCharacterPacket(uint8_t index, const char * szPrivateCode)
{
    SPacketCGDestroyCharacter DestroyCharacterPacket;
	DestroyCharacterPacket.index = index;
	strncpy_s(DestroyCharacterPacket.szPrivateCode, szPrivateCode, PRIVATE_CODE_LENGTH-1);

	if (!Send(sizeof(DestroyCharacterPacket), &DestroyCharacterPacket))
	{
		Tracen("SendDestroyCharacterPacket");
		return false;
	}

	return true;
}

bool CPythonNetworkStream::SendCreateCharacterPacket(uint8_t index, const char *name, uint8_t job, uint8_t shape, uint8_t byCON, uint8_t byINT, uint8_t bySTR, uint8_t byDEX)
{
	SPacketCGCreateCharacter createCharacterPacket;
	createCharacterPacket.index = index;
	strncpy_s(createCharacterPacket.name, name, CHARACTER_NAME_MAX_LEN);
	createCharacterPacket.job = job;
	createCharacterPacket.shape = shape;
	createCharacterPacket.CON = byCON;
	createCharacterPacket.IQ = byINT;
	createCharacterPacket.STR = bySTR;
	createCharacterPacket.DEX = byDEX;

	if (!Send(sizeof(createCharacterPacket), &createCharacterPacket))
	{
		Tracen("Failed to SendCreateCharacterPacket");
		return false;
	}

	return true;
}

bool CPythonNetworkStream::SendChangeNamePacket(uint8_t index, const char *name)
{
	SPacketCGChangeName ChangeNamePacket;
	ChangeNamePacket.index = index;
	strncpy_s(ChangeNamePacket.name, name, CHARACTER_NAME_MAX_LEN);

	if (!Send(sizeof(SPacketCGChangeName), &ChangeNamePacket))
	{
		Tracen("Failed to SendChangeNamePacket");
		return false;
	}

	return true;
}

bool CPythonNetworkStream::__RecvPlayerCreateSuccessPacket()
{
	SPacketGCPlayerCreateSuccess kCreateSuccessPacket;

	if (!Recv(sizeof(kCreateSuccessPacket), &kCreateSuccessPacket))
		return false;

	if (kCreateSuccessPacket.bAccountCharacterSlot >= PLAYER_PER_ACCOUNT)
	{
		TraceError("CPythonNetworkStream::RecvPlayerCreateSuccessPacket - OUT OF RANGE SLOT(%d) > PLATER_PER_ACCOUNT(%d)",
				   kCreateSuccessPacket.bAccountCharacterSlot, PLAYER_PER_ACCOUNT);
		return true;
	}

	m_akSimplePlayerInfo[kCreateSuccessPacket.bAccountCharacterSlot]=kCreateSuccessPacket.kSimplePlayerInfomation;
	PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_CREATE], "OnCreateSuccess", Py_BuildValue("()"));
	return true;
}

bool CPythonNetworkStream::__RecvPlayerCreateFailurePacket()
{
	SPacketGCCreateFailure packet;

	if (!Recv(sizeof(SPacketGCCreateFailure), &packet))
		return false;

	PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_CREATE], "OnCreateFailure", Py_BuildValue("(i)", packet.bType));
	PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_SELECT], "OnCreateFailure", Py_BuildValue("(i)", packet.bType));
	return true;
}

bool CPythonNetworkStream::__RecvPlayerDestroySuccessPacket()
{
	TPacketGCDestroyCharacterSuccess packet;
	if (!Recv(sizeof(TPacketGCDestroyCharacterSuccess), &packet))
		return false;

	memset(&m_akSimplePlayerInfo[packet.account_index], 0, sizeof(m_akSimplePlayerInfo[packet.account_index]));
	m_adwGuildID[packet.account_index] = 0;
	m_astrGuildName[packet.account_index].clear();

	PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_SELECT], "OnDeleteSuccess", Py_BuildValue("(i)", packet.account_index));
	return true;
}

bool CPythonNetworkStream::__RecvPlayerDestroyFailurePacket()
{
	SPacketGCPlayerDeleteWrongSocialID packet;
	if (!Recv(sizeof(packet), &packet))
		return false;

	PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_SELECT], "OnDeleteFailure", Py_BuildValue("()"));
	return true;
}

bool CPythonNetworkStream::__RecvChangeName()
{
	SPacketGCChangeName ChangeNamePacket;
	if (!Recv(sizeof(SPacketGCChangeName), &ChangeNamePacket))
		return false;

	for (int32_t i = 0; i < PLAYER_PER_ACCOUNT; ++i)
	{
		if (ChangeNamePacket.pid == m_akSimplePlayerInfo[i].dwID)
		{
			m_akSimplePlayerInfo[i].bChangeName = FALSE;
			strncpy_s(m_akSimplePlayerInfo[i].szName, ChangeNamePacket.name, CHARACTER_NAME_MAX_LEN);

			PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_SELECT], "OnChangeName", Py_BuildValue("(is)", i, ChangeNamePacket.name));
			return true;
		}
	}

	PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_SELECT], "OnCreateFailure", Py_BuildValue("(i)", 100));
	return true;
}
