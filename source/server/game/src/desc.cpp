#include "stdafx.h"
#include "config.h"
#include "utils.h"
#include "desc.h"
#include "desc_client.h"
#include "desc_manager.h"
#include "char.h"
#include "protocol.h"
#include "messenger_manager.h"
#include "sectree_manager.h"
#include "p2p.h"
#include "buffer_manager.h"
#include "guild.h"
#include "guild_manager.h"
#include "locale_service.h"
#include "log.h"

extern int32_t max_bytes_written;
extern int32_t current_bytes_written;
extern int32_t total_bytes_written;

static const std::size_t kOutputBufferLimit = 256 * 1024; // 256KiB

DESC::DESC()
{
	Initialize();
}

DESC::~DESC()
{
}

void DESC::Initialize()
{
	m_bDestroyed = false;

	m_pInputProcessor = nullptr;
	m_lpFdw = nullptr;
	m_sock = INVALID_SOCKET;
	m_iPhase = PHASE_CLOSE;
	m_dwHandle = 0;

	m_wPort = 0;
	m_wListenPort = 0;
	m_LastTryToConnectTime = 0;

	m_lpInputBuffer = nullptr;
	m_iMinInputBufferLen = 0;

	m_dwHandshake = 0;
	m_dwHandshakeSentTime = 0;
	m_iHandshakeRetry = 0;
	m_dwClientTime = 0;
	m_bHandshaking = false;

	m_lpBufferedOutputBuffer = nullptr;
	m_lpOutputBuffer = nullptr;

	m_pkPingEvent = nullptr;
	m_lpCharacter = nullptr;
	memset( &m_accountTable, 0, sizeof(m_accountTable) );

	memset( &m_SockAddr, 0, sizeof(m_SockAddr) );

	m_pLogFile = nullptr;
	m_stRelayName.clear();

#ifndef _IMPROVED_PACKET_ENCRYPTION_
	m_bEncrypted = false;
#endif

	m_wP2PPort = 0;
	m_bP2PChannel = 0;

	m_bAdminMode = false;
	m_bPong = true;
	m_bChannelStatusRequested = false;

	m_pkLoginKey = nullptr;
	m_dwLoginKey = 0;

#ifndef _IMPROVED_PACKET_ENCRYPTION_
	memset( m_adwDecryptionKey, 0, sizeof(m_adwDecryptionKey) );
	memset( m_adwEncryptionKey, 0, sizeof(m_adwEncryptionKey) );
#endif

	m_pkDisconnectEvent = nullptr;
}

void DESC::Destroy()
{
	if (m_bDestroyed)
		return;
	m_bDestroyed = true;

	if (m_pkLoginKey)
		m_pkLoginKey->Expire();

	if (GetAccountTable().id)
		DESC_MANAGER::Instance().DisconnectAccount(GetAccountTable().login);

	if (m_pLogFile)
	{
		fclose(m_pLogFile);
		m_pLogFile = nullptr;
	}

	if (m_lpCharacter)
	{
		const char * closeReason = GetCloseReason().c_str();
		m_lpCharacter->Disconnect(*closeReason ? closeReason : "DESC::~DESC");

		m_lpCharacter = nullptr;
	}

	m_closeReason.clear();

	SAFE_BUFFER_DELETE(m_lpOutputBuffer);
	SAFE_BUFFER_DELETE(m_lpInputBuffer);

	event_cancel(&m_pkPingEvent);
	event_cancel(&m_pkDisconnectEvent);

	if (!g_bAuthServer)
	{
		if (m_accountTable.login[0] && m_accountTable.passwd[0])
		{
			TLogoutPacket pack;

			strlcpy(pack.login, m_accountTable.login, sizeof(pack.login));
			strlcpy(pack.passwd, m_accountTable.passwd, sizeof(pack.passwd));

			db_clientdesc->DBPacket(HEADER_GD_LOGOUT, m_dwHandle, &pack, sizeof(TLogoutPacket));
		}
	}

	if (m_sock != INVALID_SOCKET)
	{
		sys_log(0, "SYSTEM: closing socket. DESC #%d", m_sock);
		Log("SYSTEM: closing socket. DESC #%d", m_sock);
		fdwatch_del_fd(m_lpFdw, m_sock);

#ifdef _IMPROVED_PACKET_ENCRYPTION_
		cipher_.CleanUp();
#endif

		socket_close(m_sock);
		m_sock = INVALID_SOCKET;
	}
}

EVENTFUNC(ping_event)
{
	DESC::desc_event_info* info = dynamic_cast<DESC::desc_event_info*>( event->info );

	if ( info == nullptr )
	{
		sys_err( "ping_event> <Factor> Null pointer" );
		return 0;
	}

	LPDESC desc = info->desc;

	if (desc->IsAdminMode())
		return (ping_event_second_cycle);

	if (!desc->IsPong())
	{
		sys_log(0, "PING_EVENT: no pong %s", desc->GetHostName());

		desc->SetCloseReason("NO_PONG");
		desc->SetPhase(PHASE_CLOSE);

		return (ping_event_second_cycle);
	}
	else
	{
		SPacketGCPing p;
		desc->Packet(&p, sizeof(p));
		desc->SetPong(false);
	}

	desc->SendHandshake(get_unix_ms_time(), 0);

	return (ping_event_second_cycle);
}

bool DESC::IsPong()
{
	return m_bPong;
}

void DESC::SetPong(bool b)
{
	m_bPong = b;
}

bool DESC::Setup(LPFDWATCH _fdw, socket_t _fd, const struct sockaddr_in & c_rSockAddr, uint32_t _handle, uint32_t _handshake)
{
	m_lpFdw		= _fdw;
	m_sock		= _fd;

	m_stHost		= inet_ntoa(c_rSockAddr.sin_addr);
	m_wPort			= c_rSockAddr.sin_port;
	m_dwHandle		= _handle;

	m_lpOutputBuffer = buffer_new(DEFAULT_PACKET_BUFFER_SIZE * 3); // Default: 2

	m_iMinInputBufferLen = MAX_INPUT_LEN >> 1;
	m_lpInputBuffer = buffer_new(MAX_INPUT_LEN);

	m_SockAddr = c_rSockAddr;

	fdwatch_add_fd(m_lpFdw, m_sock, this, FDW_READ, false);

	// Ping Event 
	desc_event_info* info = AllocEventInfo<desc_event_info>();

	info->desc = this;
	assert(m_pkPingEvent == nullptr);

	m_pkPingEvent = event_create(ping_event, info, ping_event_second_cycle);

#ifndef _IMPROVED_PACKET_ENCRYPTION_
	memcpy(m_adwEncryptionKey, LSS_SECURITY_KEY, sizeof(uint32_t) * 4);
	memcpy(m_adwDecryptionKey, LSS_SECURITY_KEY, sizeof(uint32_t) * 4);
#endif

	// Set Phase to handshake
	SetPhase(PHASE_HANDSHAKE);
	StartHandshake(_handshake);

	sys_log(0, "SYSTEM: new connection from [%s] fd: %d handshake %lu output input_len %lu, ptr %p",
			m_stHost.c_str(), m_sock, m_dwHandshake, buffer_size(m_lpInputBuffer), this);

	Log("SYSTEM: new connection from [%s] fd: %d handshake %u ptr %p", m_stHost.c_str(), m_sock, m_dwHandshake, this);
	return true;
}

int32_t DESC::ProcessInput()
{
	if (!m_lpInputBuffer)
	{
		net_err("DESC::ProcessInput : nil input buffer");
		return -1;
	}

	buffer_adjust_size(m_lpInputBuffer, m_iMinInputBufferLen);
	auto bytes_read = socket_read(m_sock, (char *) buffer_write_peek(m_lpInputBuffer), buffer_has_space(m_lpInputBuffer));

	if (bytes_read < 0)
		return -1;
	else if (bytes_read == 0)
		return 0;

	buffer_write_proceed(m_lpInputBuffer, bytes_read);

	if (!m_pInputProcessor)
		net_err("no input processor");
#ifdef _IMPROVED_PACKET_ENCRYPTION_
	else
	{
		if (cipher_.activated()) {
			cipher_.Decrypt(const_cast<void*>(buffer_read_peek(m_lpInputBuffer)), buffer_size(m_lpInputBuffer));
		}

		int32_t iBytesProceed = 0;

		// false가 리턴 되면 다른 phase로 바뀐 것이므로 다시 프로세스로 돌입한다!
		while (!m_pInputProcessor->Process(this, buffer_read_peek(m_lpInputBuffer), buffer_size(m_lpInputBuffer), iBytesProceed))
		{
			buffer_read_proceed(m_lpInputBuffer, iBytesProceed);
			iBytesProceed = 0;
		}

		buffer_read_proceed(m_lpInputBuffer, iBytesProceed);
	}
#else
	else if (!m_bEncrypted)
	{
		int32_t iBytesProceed = 0;

		// false가 리턴 되면 다른 phase로 바뀐 것이므로 다시 프로세스로 돌입한다!
		while (!m_pInputProcessor->Process(this, buffer_read_peek(m_lpInputBuffer), buffer_size(m_lpInputBuffer), iBytesProceed))
		{
			buffer_read_proceed(m_lpInputBuffer, iBytesProceed);
			iBytesProceed = 0;
		}

		buffer_read_proceed(m_lpInputBuffer, iBytesProceed);
	}
	else
	{
		int32_t iSizeBuffer = buffer_size(m_lpInputBuffer);

		// 8바이트 단위로만 처리한다. 8바이트 단위에 부족하면 잘못된 암호화 버퍼를 복호화
		// 할 가능성이 있으므로 짤라서 처리하기로 한다.
		if (iSizeBuffer & 7) // & 7은 % 8과 같다. 2의 승수에서만 가능
			iSizeBuffer -= iSizeBuffer & 7;

		if (iSizeBuffer > 0)
		{
			TEMP_BUFFER	tempbuf (8192 + 4); //Size greater than 8192 so that its on a different buffer pool than normal buffers
			LPBUFFER lpBufferDecrypt = tempbuf.getptr();
			buffer_adjust_size(lpBufferDecrypt, iSizeBuffer);

			int32_t iSizeAfter = TEA_Decrypt((uint32_t *) buffer_write_peek(lpBufferDecrypt),
					(uint32_t *) buffer_read_peek(m_lpInputBuffer),
					GetDecryptionKey(),
					iSizeBuffer);

			buffer_write_proceed(lpBufferDecrypt, iSizeAfter);

			int32_t iBytesProceed = 0;

			// false가 리턴 되면 다른 phase로 바뀐 것이므로 다시 프로세스로 돌입한다!
			while (!m_pInputProcessor->Process(this, buffer_read_peek(lpBufferDecrypt), buffer_size(lpBufferDecrypt), iBytesProceed))
			{
				if (iBytesProceed > iSizeBuffer)
				{
					buffer_read_proceed(m_lpInputBuffer, iSizeBuffer);
					iSizeBuffer = 0;
					iBytesProceed = 0;
					break;
				}

				buffer_read_proceed(m_lpInputBuffer, iBytesProceed);
				iSizeBuffer -= iBytesProceed;

				buffer_read_proceed(lpBufferDecrypt, iBytesProceed);
				iBytesProceed = 0;
			}

			buffer_read_proceed(m_lpInputBuffer, iBytesProceed);
		}
	}
#endif // _IMPROVED_PACKET_ENCRYPTION_

	return (bytes_read);
}

int32_t DESC::ProcessOutput()
{
	if (buffer_size(m_lpOutputBuffer) <= 0)
		return 0;

	int32_t buffer_left = fdwatch_get_buffer_size(m_lpFdw, m_sock);

	if (buffer_left <= 0)
		return 0;

	int32_t bytes_to_write = MIN(buffer_left, buffer_size(m_lpOutputBuffer));

	if (bytes_to_write == 0)
		return 0;

	int32_t result = socket_write(m_sock, (const char *) buffer_read_peek(m_lpOutputBuffer), bytes_to_write);

	if (result == 0)
	{
		//sys_log(0, "%d bytes written to %s first %u", bytes_to_write, GetHostName(), *(uint8_t *) buffer_read_peek(m_lpOutputBuffer));
		//Log("%d bytes written", bytes_to_write);
		max_bytes_written = MAX(bytes_to_write, max_bytes_written);

		total_bytes_written += bytes_to_write;
		current_bytes_written += bytes_to_write;

		buffer_read_proceed(m_lpOutputBuffer, bytes_to_write);

		if (buffer_size(m_lpOutputBuffer) != 0)
			fdwatch_add_fd(m_lpFdw, m_sock, this, FDW_WRITE, true);
	}

	return (result);
}

void DESC::BufferedPacket(const void * c_pvData, int32_t iSize)
{
	if (m_iPhase == PHASE_CLOSE)
		return;

	if (!m_lpBufferedOutputBuffer)
		m_lpBufferedOutputBuffer = buffer_new(MAX(1024, iSize));

	buffer_write(m_lpBufferedOutputBuffer, c_pvData, iSize);
}

void DESC::Packet(const void * c_pvData, int32_t iSize)
{
	assert(iSize > 0);

	if (m_iPhase == PHASE_CLOSE) // 끊는 상태면 보내지 않는다.
		return;

	if (g_bIsTestServer)
	{
		std::string stName = GetCharacter() ? GetCharacter()->GetName() : GetHostName();
		sys_log(0, "SENT HEADER : %u to %s  (size %d) ", *(static_cast<const uint8_t*>(c_pvData)), stName.c_str(), iSize);
	}

	if (m_stRelayName.length() != 0)
	{
		// Relay 패킷은 암호화하지 않는다.
		TPacketGGRelay p;

		p.bHeader = HEADER_GG_RELAY;
		strlcpy(p.szName, m_stRelayName.c_str(), sizeof(p.szName));
		p.lSize = iSize;

		if (!packet_encode(m_lpOutputBuffer, &p, sizeof(p)))
		{
			m_iPhase = PHASE_CLOSE;
			return;
		}

		m_stRelayName.clear();

		if (!packet_encode(m_lpOutputBuffer, c_pvData, iSize))
		{
			m_iPhase = PHASE_CLOSE;
			return;
		}
	}
	else
	{
		if (m_lpBufferedOutputBuffer)
		{
			buffer_write(m_lpBufferedOutputBuffer, c_pvData, iSize);

			c_pvData = buffer_read_peek(m_lpBufferedOutputBuffer);
			iSize = buffer_size(m_lpBufferedOutputBuffer);
		}

#ifdef _IMPROVED_PACKET_ENCRYPTION_
		void* buf = buffer_write_peek(m_lpOutputBuffer);
	
		if (packet_encode(m_lpOutputBuffer, c_pvData, iSize))
		{
			if (cipher_.activated())
			{
				cipher_.Encrypt(buf, iSize);
			}
		}
		else
		{
			m_iPhase = PHASE_CLOSE;
		}
#else
		const int32_t padding = m_bEncrypted ? 8 : 0;
		if (buffer_has_space(m_lpOutputBuffer) < iSize + padding)
		{
			// Restrict non-admin sockets to |kOutputBufferLimit| bytes.
			if (!m_bAdminMode && buffer_size(m_lpOutputBuffer) > kOutputBufferLimit)
			{
				net_err("Output buffer full. Size (%u) at(%u) iSize(%d) limit(%u)",
				        m_lpOutputBuffer->mem_size,
				        m_lpOutputBuffer->write_point_pos, iSize, kOutputBufferLimit);
				m_iPhase = PHASE_CLOSE;
			}
			else
			{
				buffer_adjust_size(m_lpOutputBuffer, iSize + padding);
			}
		}

		if (!m_bEncrypted && m_iPhase != PHASE_CLOSE)
		{
			if (!packet_encode(m_lpOutputBuffer, c_pvData, iSize))
				m_iPhase = PHASE_CLOSE;
		}
		else if (m_iPhase != PHASE_CLOSE)
		{
			uint32_t * pdwWritePoint = (uint32_t *) buffer_write_peek(m_lpOutputBuffer);

			// TODO: Check whether we can delay encryption.
			// This could save us a few bytes for small packets.
			// (It's probably not worth it though).
			if (packet_encode(m_lpOutputBuffer, c_pvData, iSize))
			{
				int32_t iSize2 = TEA_Encrypt(pdwWritePoint, pdwWritePoint, GetEncryptionKey(), iSize);
				if (iSize2 > iSize)
					buffer_write_proceed(m_lpOutputBuffer, iSize2 - iSize);
			}
			else
			{
				m_iPhase = PHASE_CLOSE;
			}
		}
#endif

		SAFE_BUFFER_DELETE(m_lpBufferedOutputBuffer);
	}

	//sys_log(0, "%d bytes written (first byte %d)", iSize, *(uint8_t *) c_pvData);
	if (m_iPhase != PHASE_CLOSE)
		fdwatch_add_fd(m_lpFdw, m_sock, this, FDW_WRITE, true);
}

void DESC::LargePacket(const void * c_pvData, int32_t iSize)
{
	buffer_adjust_size(m_lpOutputBuffer, iSize);
	sys_log(0, "LargePacket Size %d/%lu", iSize, buffer_size(m_lpOutputBuffer));

	Packet(c_pvData, iSize);
}

void DESC::SetPhase(int32_t _phase)
{
	m_iPhase = _phase;

	SPacketGCPhase pack;
	pack.phase = _phase;
	pack.stage = game_stage;
	Packet(&pack, sizeof(pack));

//	sys_log(0, "phase %d", _phase);

	switch (m_iPhase)
	{
		case PHASE_CLOSE:
			// 메신저가 캐릭터단위가 되면서 삭제
			//MessengerManager::Instance().Logout(GetAccountTable().login);
			m_pInputProcessor = &m_inputClose;
			break;

		case PHASE_HANDSHAKE:
			m_pInputProcessor = &m_inputHandshake;
			break;

		case PHASE_SELECT:
			// 메신저가 캐릭터단위가 되면서 삭제
			//MessengerManager::Instance().Logout(GetAccountTable().login); // 의도적으로 break 안검
		case PHASE_LOGIN:
		case PHASE_LOADING:
#ifndef _IMPROVED_PACKET_ENCRYPTION_
			m_bEncrypted = true;
#endif
			m_pInputProcessor = &m_inputLogin;
			break;

		case PHASE_GAME:
		case PHASE_DEAD:
#ifndef _IMPROVED_PACKET_ENCRYPTION_
			m_bEncrypted = true;
#endif
			m_pInputProcessor = &m_inputMain;
			break;

		case PHASE_AUTH:
#ifndef _IMPROVED_PACKET_ENCRYPTION_
			m_bEncrypted = true;
#endif
			m_pInputProcessor = &m_inputAuth;
			sys_log(0, "AUTH_PHASE %p", this);
			break;
	}
}

void DESC::BindAccountTable(TAccountTable * pAccountTable)
{
	assert(pAccountTable != nullptr);
	memcpy(&m_accountTable, pAccountTable, sizeof(TAccountTable));
	DESC_MANAGER::Instance().ConnectAccount(m_accountTable.login, this);
}

void DESC::Log(const char * format, ...)
{
	if (!m_pLogFile)
		return;

	va_list args;

	time_t ct = get_unix_time();
	struct tm tm = *localtime(&ct);

	fprintf(m_pLogFile,
			"%02d %02d %02d:%02d:%02d | ",
			tm.tm_mon + 1,
			tm.tm_mday,
			tm.tm_hour,
			tm.tm_min,
			tm.tm_sec);

	va_start(args, format);
	vfprintf(m_pLogFile, format, args);
	va_end(args);

	fputs("\n", m_pLogFile);

	fflush(m_pLogFile);
}

void DESC::StartHandshake(uint32_t _handshake)
{
	// Handshake
	m_dwHandshake = _handshake;

	SendHandshake(get_unix_ms_time(), 0);

	m_iHandshakeRetry = 0;
}

void DESC::SendHandshake(uint32_t dwCurTime, int32_t lNewDelta)
{
	SPacketHandshake pack;
	pack.dwHandshake	= m_dwHandshake;
	pack.dwTime			= dwCurTime;
	pack.lDelta			= lNewDelta;

	Packet(&pack, sizeof(pack));

	m_dwHandshakeSentTime = dwCurTime;
	m_bHandshaking = true;
}

bool DESC::HandshakeProcess(uint32_t dwTime, int32_t lDelta, bool bInfiniteRetry)
{
	uint32_t dwCurTime = get_unix_ms_time();

	if (lDelta < 0)
	{
		net_err("Desc::HandshakeProcess : value error (lDelta %ld, ip %s)", lDelta, m_stHost.c_str());
		return false;
	}

	int32_t bias = (int32_t) (dwCurTime - (dwTime + lDelta));

	if (bias >= -50 && bias <= 50) // Default lower limit: 0
	{
		if (bInfiniteRetry)
		{
			uint8_t bHeader = HEADER_GC_HANDSHAKE_OK;
			Packet(&bHeader, sizeof(uint8_t));
		}

		if (GetCharacter())
			sys_log(0, "Handshake: client_time %u server_time %u name: %s", m_dwClientTime, dwCurTime, GetCharacter()->GetName());
		else
			sys_log(0, "Handshake: client_time %lu server_time %lu, delta %ld", m_dwClientTime, dwCurTime, lDelta);

		m_dwClientTime = dwCurTime;
		m_bHandshaking = false;
		return true; 
	}

	int32_t lNewDelta = (int32_t) (dwCurTime - dwTime) / 2;

	if (lNewDelta < 0)
	{
		sys_log(0, "Handshake: lower than zero %d", lNewDelta);
		lNewDelta = (dwCurTime - m_dwHandshakeSentTime) / 2;
	}

	sys_log(1, "Handshake: ServerTime %lu dwTime %lu lDelta %lu SentTime %lu lNewDelta %lu", dwCurTime, dwTime, lDelta, m_dwHandshakeSentTime, lNewDelta);

	if (!bInfiniteRetry)
	{
		if (++m_iHandshakeRetry > HANDSHAKE_RETRY_LIMIT)
		{
			net_err("handshake retry limit reached! (limit %d character %s)",
					HANDSHAKE_RETRY_LIMIT, GetCharacter() ? GetCharacter()->GetName() : "!NO CHARACTER!");
			SetPhase(PHASE_CLOSE);
			return false;
		}
	}

	SendHandshake(dwCurTime, lNewDelta);
	return false;
}

bool DESC::IsHandshaking()
{
	return m_bHandshaking;
}

uint32_t DESC::GetClientTime()
{
	return m_dwClientTime;
}

#ifdef _IMPROVED_PACKET_ENCRYPTION_
void DESC::SendKeyAgreement()
{
	SPacketKeyAgreement packet;

	size_t data_length = SPacketKeyAgreement::MAX_DATA_LEN;
	size_t agreed_length = cipher_.Prepare(packet.data, &data_length);
	if (agreed_length == 0) {
		// Initialization failure
		SetCloseReason("KEY_AGR");
		SetPhase(PHASE_CLOSE);
		return;
	}
	assert(data_length <= SPacketKeyAgreement::MAX_DATA_LEN);

	packet.wAgreedLength = (uint16_t)agreed_length;
	packet.wDataLength = (uint16_t)data_length;

	Packet(&packet, sizeof(packet));
}

void DESC::SendKeyAgreementCompleted()
{
	SPacketGCKeyAgreementCompleted packet;
	Packet(&packet, sizeof(packet));
}

bool DESC::FinishHandshake(size_t agreed_length, const void* buffer, size_t length)
{
	return cipher_.Activate(false, agreed_length, buffer, length);
}

bool DESC::IsCipherPrepared()
{
	return cipher_.IsKeyPrepared();
}
#endif // #ifdef _IMPROVED_PACKET_ENCRYPTION_

void DESC::SetRelay(const char * c_pszName)
{
	m_stRelayName = c_pszName;
}

void DESC::BindCharacter(LPCHARACTER ch)
{
	m_lpCharacter = ch;
}

void DESC::FlushOutput()
{
	if (m_sock == INVALID_SOCKET) {
		return;
	}

	if (buffer_size(m_lpOutputBuffer) <= 0)
		return;

	struct timeval sleep_tv, now_tv, start_tv;
	int32_t event_triggered = false;

	gettimeofday(&start_tv, nullptr);

	_socket_block(m_sock);
	sys_log(0, "FLUSH START %lu", buffer_size(m_lpOutputBuffer));

	while (buffer_size(m_lpOutputBuffer) > 0)
	{
		gettimeofday(&now_tv, nullptr);

		int32_t iSecondsPassed = now_tv.tv_sec - start_tv.tv_sec;

		if (iSecondsPassed > 10)
		{
			if (!event_triggered || iSecondsPassed > 20)
			{
				SetPhase(PHASE_CLOSE);
				break;
			}
		}

		sleep_tv.tv_sec = 0;
		sleep_tv.tv_usec = 10000;

		int32_t num_events = fdwatch(m_lpFdw, &sleep_tv);

		if (num_events < 0)
		{
			net_err("num_events < 0 : %d", num_events);
			break;
		}

		int32_t event_idx;

		for (event_idx = 0; event_idx < num_events; ++event_idx)
		{
			LPDESC d2 = (LPDESC) fdwatch_get_client_data(m_lpFdw, event_idx);

			if (d2 != this)
				continue;

			switch (fdwatch_check_event(m_lpFdw, m_sock, event_idx))
			{
				case FDW_WRITE:
					event_triggered = true;

					if (ProcessOutput() < 0)
					{
						net_err("Cannot flush output buffer");
						SetPhase(PHASE_CLOSE);
					}
					break;

				case FDW_EOF:
					SetPhase(PHASE_CLOSE);
					break;
			}
		}

		if (IsPhase(PHASE_CLOSE))
			break;
	}

	if (buffer_size(m_lpOutputBuffer) == 0)
		sys_log(0, "FLUSH SUCCESS");
	else
		sys_log(0, "FLUSH FAIL");

	usleep(250000);
}

EVENTFUNC(disconnect_event)
{
	DESC::desc_event_info* info = dynamic_cast<DESC::desc_event_info*>( event->info );

	if ( info == nullptr )
	{
		sys_err( "disconnect_event> <Factor> Null pointer" );
		return 0;
	}

	LPDESC d = info->desc;

	d->m_pkDisconnectEvent = nullptr;
	d->SetCloseReason("DELAYED_DC");
	d->SetPhase(PHASE_CLOSE);
	return 0;
}

bool DESC::DelayedDisconnect(int32_t iSec)
{
	if (m_pkDisconnectEvent != nullptr) {
		return false;
	}

	desc_event_info* info = AllocEventInfo<desc_event_info>();
	info->desc = this;

	m_pkDisconnectEvent = event_create(disconnect_event, info, PASSES_PER_SEC(iSec));
	return true;
}

void DESC::DisconnectOfSameLogin()
{
	if (GetCharacter())
	{
		if (m_pkDisconnectEvent)
			return;

		GetCharacter()->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("다른 컴퓨터에서 로그인 하여 접속을 종료 합니다."));
		DelayedDisconnect(5);
	}
	else
	{
		SetPhase(PHASE_CLOSE);
	}
}

void DESC::SetAdminMode()
{
	m_bAdminMode = true;
}

bool DESC::IsAdminMode()
{
	return m_bAdminMode;
}

void DESC::SendLoginSuccessPacket()
{
	TAccountTable & rTable = GetAccountTable();

	SPacketGCLoginSuccess p;
	p.handle     = GetHandle();
	p.random_key = DESC_MANAGER::Instance().MakeRandomKey(GetHandle()); // FOR MARK
	memcpy(p.akSimplePlayerInformation, rTable.players, sizeof(rTable.players));

	for (int32_t i = 0; i < PLAYER_PER_ACCOUNT; ++i)
	{   
#ifdef ENABLE_NEWSTUFF
		if (!g_stProxyIP.empty())
			rTable.players[i].lAddr=inet_addr(g_stProxyIP.c_str());
#endif
		CGuild* g = CGuildManager::Instance().GetLinkedGuild(rTable.players[i].dwID);

		if (g)
		{   
			p.guild_id[i] = g->GetID();
			strlcpy(p.guild_name[i], g->GetName(), sizeof(p.guild_name[i]));
		}   
		else
		{
			p.guild_id[i] = 0;
			p.guild_name[i][0] = '\0';
		}
	}

	Packet(&p, sizeof(SPacketGCLoginSuccess));
}

void DESC::SetLoginKey(uint32_t dwKey)
{
	m_dwLoginKey = dwKey;
}

void DESC::SetLoginKey(CLoginKey * pkKey)
{
	m_pkLoginKey = pkKey;
	sys_log(0, "SetLoginKey %u", m_pkLoginKey->m_dwKey);
}

uint32_t DESC::GetLoginKey()
{
	if (m_pkLoginKey)
		return m_pkLoginKey->m_dwKey;

	return m_dwLoginKey;
}

const uint8_t* GetSpecialKey()
{   
	static bool bGenerated = false;
	static uint32_t s_adwKey[1938]; 

	if (!bGenerated) 
	{
		bGenerated = true;
		uint32_t seed = 1491971513; 

		for (uint32_t i = 0; i < uint8_t(seed); ++i)
		{
			seed ^= 2148941891ul;
			seed += 3592385981ul;

			s_adwKey[i] = seed;
		}
	}

	return (const uint8_t*)s_adwKey;
}

#ifndef _IMPROVED_PACKET_ENCRYPTION_
void DESC::SetSecurityKey(const uint32_t * c_pdwKey)
{
	const auto c_pszKey = GetSpecialKey() + 37;

	memcpy(&m_adwDecryptionKey, c_pdwKey, 16);
	TEA_Encrypt(&m_adwEncryptionKey[0], &m_adwDecryptionKey[0], (const uint32_t *) c_pszKey, 16);

	sys_log(0, "SetSecurityKey decrypt %u %u %u %u encrypt %u %u %u %u", 
			m_adwDecryptionKey[0], m_adwDecryptionKey[1], m_adwDecryptionKey[2], m_adwDecryptionKey[3],
			m_adwEncryptionKey[0], m_adwEncryptionKey[1], m_adwEncryptionKey[2], m_adwEncryptionKey[3]);
}
#endif // _IMPROVED_PACKET_ENCRYPTION_

uint8_t DESC::GetEmpire()
{
	return m_accountTable.bEmpire;
}

void DESC::ChatPacket(uint8_t type, const char * format, ...)
{
	char chatbuf[CHAT_MAX_LEN + 1];
	va_list args;

	va_start(args, format);
	int32_t len = vsnprintf(chatbuf, sizeof(chatbuf), format, args);
	va_end(args);

	SPacketGCChat pack_chat;
	pack_chat.size      = sizeof(SPacketGCChat) + len;
	pack_chat.type      = type;
	pack_chat.dwVID        = 0;
	pack_chat.bEmpire   = GetEmpire();

	TEMP_BUFFER buf;
	buf.write(&pack_chat, sizeof(SPacketGCChat));
	buf.write(chatbuf, len);

	Packet(buf.read_peek(), buf.size());
}
