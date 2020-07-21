#include "StdAfx.h"
#include "NetStream.h"
#ifndef _IMPROVED_PACKET_ENCRYPTION_
#include "../eterBase/tea.h"
#endif

#ifndef _IMPROVED_PACKET_ENCRYPTION_
void CNetworkStream::SetSecurityMode(bool isSecurityMode, const char* c_szTeaKey)
{
	m_isSecurityMode = isSecurityMode;
	memcpy(m_szEncryptKey, c_szTeaKey, TEA_KEY_LENGTH);
	memcpy(m_szDecryptKey, c_szTeaKey, TEA_KEY_LENGTH);
}

void CNetworkStream::SetSecurityMode(bool isSecurityMode, const char* c_szTeaEncryptKey, const char* c_szTeaDecryptKey)
{
	m_isSecurityMode = isSecurityMode;
	memcpy(m_szEncryptKey, c_szTeaEncryptKey, TEA_KEY_LENGTH);
	memcpy(m_szDecryptKey, c_szTeaDecryptKey, TEA_KEY_LENGTH);
}
#endif // _IMPROVED_PACKET_ENCRYPTION_

bool CNetworkStream::IsSecurityMode()
{
#ifdef _IMPROVED_PACKET_ENCRYPTION_
	return m_cipher.activated();
#else
	return m_isSecurityMode;
#endif
}

void CNetworkStream::SetRecvBufferSize(int32_t recvBufSize)
{
	if (m_recvBuf)
	{
		if (m_recvBufSize>recvBufSize)
			return;

		delete [] m_recvBuf;
		delete [] m_recvTEABuf;
	}
	m_recvBufSize = recvBufSize;
	m_recvBuf = new char[m_recvBufSize];	
	m_recvTEABufSize = ((m_recvBufSize>>3)+1)<<3;
	m_recvTEABuf = new char[m_recvTEABufSize];
}

void CNetworkStream::SetSendBufferSize(int32_t sendBufSize)
{
	if (m_sendBuf)
	{
		if (m_sendBufSize > sendBufSize)
			return;

		delete [] m_sendBuf;
		delete [] m_sendTEABuf;
	}

	m_sendBufSize = sendBufSize;
	m_sendBuf = new char[m_sendBufSize];
	m_sendTEABufSize = ((m_sendBufSize>>3)+1)<<3;
	m_sendTEABuf = new char[m_sendTEABufSize];
}

bool CNetworkStream::__RecvInternalBuffer()
{
	if (m_recvBufOutputPos>0)
	{
		int32_t recvBufDataSize = m_recvBufInputPos - m_recvBufOutputPos;
		if (recvBufDataSize>0)
		{
			memmove(m_recvBuf, m_recvBuf + m_recvBufOutputPos, recvBufDataSize);
		}
		
		m_recvBufInputPos -= m_recvBufOutputPos;
		m_recvBufOutputPos = 0;
	}

#ifdef _IMPROVED_PACKET_ENCRYPTION_
	int32_t restSize = m_recvBufSize - m_recvBufInputPos;
	if (restSize>0)
	{		
		int32_t recvSize = recv(m_sock, m_recvBuf + m_recvBufInputPos, m_recvBufSize - m_recvBufInputPos, 0);	
		//Tracenf("RECV %d %d(%d, %d)", recvSize, restSize, m_recvTEABufSize - m_recvTEABufInputPos, m_recvBufSize - m_recvBufInputPos);

		if (recvSize < 0)
		{
			int32_t error = WSAGetLastError();

			if (error != WSAEWOULDBLOCK)
			{
				return false;
			}
		}
		else if (recvSize == 0)
		{
			return false;
		}

		if (IsSecurityMode()) {
			m_cipher.Decrypt(m_recvBuf + m_recvBufInputPos, recvSize);
		}

		m_recvBufInputPos += recvSize;
	}
#else
	if (IsSecurityMode())
	{		
		int32_t restSize = std::min(m_recvTEABufSize - m_recvTEABufInputPos, m_recvBufSize - m_recvBufInputPos);
 
		if (restSize > 0)
		{
			int32_t recvSize = recv(m_sock, m_recvTEABuf + m_recvTEABufInputPos, restSize, 0);	
			//Tracenf("RECV %d %d(%d, %d)", recvSize, restSize, m_recvTEABufSize - m_recvTEABufInputPos, m_recvBufSize - m_recvBufInputPos);

			if (recvSize < 0)
			{
				int32_t error = WSAGetLastError();

				if (error != WSAEWOULDBLOCK)
				{
					return false;
				}
			}
			else if (recvSize == 0)
			{
				return false;
			}

			m_recvTEABufInputPos += recvSize;

			int32_t decodeSize = m_recvTEABufInputPos;

			if (decodeSize >= 8)
			{
				decodeSize >>= 3;
				decodeSize <<= 3;

				/*int32_t decodeDstSize = tea_decrypt((uint32_t *) (m_recvBuf + m_recvBufInputPos),
												 (uint32_t *) m_recvTEABuf,
												 (const uint32_t *) m_szDecryptKey,
												 decodeSize);
												 */
				int32_t decodeDstSize = tea_decrypt((uint32_t *) (m_recvBuf + m_recvBufInputPos),
												(uint32_t *) m_recvTEABuf,
												(const uint32_t *) m_szDecryptKey,
												decodeSize);

				m_recvBufInputPos += decodeDstSize;

				if (m_recvTEABufInputPos>decodeSize)
					memmove(m_recvTEABuf, m_recvTEABuf+decodeSize, m_recvTEABufInputPos-decodeSize);

				m_recvTEABufInputPos -= decodeSize;
				
				
				//Tracenf("!!!!!! decrypt decodeSrcSize %d -> decodeDstSize %d (recvOutputPos %d, recvInputPos %d, teaInputPos %d)", 
				//		decodeSize, decodeDstSize, m_recvBufOutputPos, m_recvBufInputPos, m_recvTEABufInputPos);
			}
		}
	}
	else
	{
		int32_t restSize = m_recvBufSize - m_recvBufInputPos;
		if (restSize>0)
		{		
			int32_t recvSize = recv(m_sock, m_recvBuf + m_recvBufInputPos, m_recvBufSize - m_recvBufInputPos, 0);	
			//Tracenf("RECV %d %d(%d, %d)", recvSize, restSize, m_recvTEABufSize - m_recvTEABufInputPos, m_recvBufSize - m_recvBufInputPos);

			if (recvSize < 0)
			{
				int32_t error = WSAGetLastError();

				if (error != WSAEWOULDBLOCK)
				{
					return false;
				}
			}
			else if (recvSize == 0)
			{
				return false;
			}

			m_recvBufInputPos += recvSize;
		}
	}
#endif // _IMPROVED_PACKET_ENCRYPTION_
		
	//Tracef("recvSize: %d input pos %d output pos %d\n", recvSize, m_recvBufInputPos, m_recvBufOutputPos);			

	return true;
}


bool CNetworkStream::__SendInternalBuffer()
{
#ifdef _IMPROVED_PACKET_ENCRYPTION_
	int32_t dataSize=__GetSendBufferSize();
	if (dataSize<=0)
		return true;

	if (IsSecurityMode()) {
		m_cipher.Encrypt(m_sendBuf + m_sendBufOutputPos, dataSize);
	}

	int32_t sendSize = send(m_sock, m_sendBuf+m_sendBufOutputPos, dataSize, 0);	
	if (sendSize < 0)
		return false;

	m_sendBufOutputPos+=sendSize;

	__PopSendBuffer();
#else
	if (IsSecurityMode())
	{
		int32_t encodeSize=__GetSendBufferSize();
		if (encodeSize<=0)
			return true;

		m_sendTEABufInputPos += tea_encrypt((uint32_t *) (m_sendTEABuf + m_sendTEABufInputPos),
												 (uint32_t *) (m_sendBuf + m_sendBufOutputPos),
												 (const uint32_t *) m_szEncryptKey,
												 encodeSize);
		m_sendBufOutputPos += encodeSize;

		if (m_sendTEABufInputPos>0)
		{	
			int32_t sendSize = send(m_sock, m_sendTEABuf, m_sendTEABufInputPos, 0);	
			if (sendSize < 0)
				return false;

			if (m_sendTEABufInputPos>sendSize)
				memmove(m_sendTEABuf, m_sendTEABuf+sendSize, m_sendTEABufInputPos-sendSize);

			m_sendTEABufInputPos-=sendSize;			
		}

		__PopSendBuffer();
	}
	else
	{
		int32_t dataSize=__GetSendBufferSize();
		if (dataSize<=0)
			return true;

		int32_t sendSize = send(m_sock, m_sendBuf+m_sendBufOutputPos, dataSize, 0);	
		if (sendSize < 0)
			return false;

		m_sendBufOutputPos+=sendSize;

		__PopSendBuffer();
	}
#endif // _IMPROVED_PACKET_ENCRYPTION_

	return true;
}

void CNetworkStream::__PopSendBuffer()
{
	if (m_sendBufOutputPos<=0)
		return;
		
	int32_t sendBufDataSize = m_sendBufInputPos - m_sendBufOutputPos;

	if (sendBufDataSize>0)
	{
		memmove(m_sendBuf, m_sendBuf+m_sendBufOutputPos, sendBufDataSize);
	}

	m_sendBufInputPos = sendBufDataSize;
	m_sendBufOutputPos = 0;	
}

#pragma warning(push)
#pragma warning(disable:4127)
void CNetworkStream::Process()
{
	if (m_sock == INVALID_SOCKET)
		return;

	fd_set fdsRecv, fdsSend, fdsExcept;
	FD_ZERO(&fdsRecv);
	FD_ZERO(&fdsSend);
	FD_ZERO(&fdsExcept);

	FD_SET(m_sock, &fdsRecv);
	FD_SET(m_sock, &fdsSend);
	FD_SET(m_sock, &fdsExcept);

	TIMEVAL delay;
	delay.tv_sec = 0;
	delay.tv_usec = 0;
	
	if (select(0, &fdsRecv, &fdsSend, &fdsExcept, &delay) == SOCKET_ERROR)
		return;

	if (FD_ISSET(m_sock, &fdsExcept))
	{
		TraceError("select: Connecting to %s:%d failed",
		           inet_ntoa(m_addr.GetAddr()), m_addr.GetPort());
		Clear();
		OnConnectFailure();
		return;
	}

	if (!m_isOnline)
	{
		if (FD_ISSET(m_sock, &fdsSend))
		{
			m_isOnline = true;
			OnConnectSuccess();
		}
		else if (time(nullptr) > m_connectLimitTime)
		{
			TraceError("Connecting to %s:%d timed out",
			           inet_ntoa(m_addr.GetAddr()), m_addr.GetPort());
			Clear();
			OnConnectFailure();
		}

		return;
	}

	if (FD_ISSET(m_sock, &fdsSend) && (m_sendBufInputPos > m_sendBufOutputPos))
	{
		if (!__SendInternalBuffer())
		{
			int32_t error = WSAGetLastError();

			if (error != WSAEWOULDBLOCK)
			{
				OnRemoteDisconnect();
				Clear();
				return;
			}
		}
	}

	if (FD_ISSET(m_sock, &fdsRecv))
	{
		if (!__RecvInternalBuffer())
		{
			OnRemoteDisconnect();
			Clear();
			return;
		}
	}

	if (!OnProcess())
	{
		OnRemoteDisconnect();
		Clear();
	}
}
#pragma warning(pop)

void CNetworkStream::Disconnect()
{
	if (m_sock == INVALID_SOCKET)
		return;

	//OnDisconnect();

	Clear();
}

void CNetworkStream::Clear()
{
	if (m_sock == INVALID_SOCKET)
		return;

#ifdef _IMPROVED_PACKET_ENCRYPTION_
	m_cipher.CleanUp();
#endif

	closesocket(m_sock);
	m_sock = INVALID_SOCKET;

#ifndef _IMPROVED_PACKET_ENCRYPTION_
	memset(m_szEncryptKey, 0, sizeof(m_szEncryptKey));
	memset(m_szDecryptKey, 0, sizeof(m_szDecryptKey));

	m_isSecurityMode = false;
#endif

	m_isOnline = false;
	m_connectLimitTime = 0;

	m_recvTEABufInputPos = 0;
	m_sendTEABufInputPos = 0;

	m_recvBufInputPos = 0;	
	m_recvBufOutputPos = 0;
	
	m_sendBufInputPos = 0;	
	m_sendBufOutputPos = 0;
}

bool CNetworkStream::Connect(const CNetworkAddress& c_rkNetAddr, int32_t limitSec)
{
	Clear();

	m_addr = c_rkNetAddr;
	
	m_sock = socket(AF_INET, SOCK_STREAM, 0);

	if (m_sock == INVALID_SOCKET) 
	{
		Clear();
		OnConnectFailure();
		return false;
	}

	DWORD arg = 1;
	ioctlsocket(m_sock, FIONBIO, &arg);	// Non-blocking mode

	if (connect(m_sock, reinterpret_cast<PSOCKADDR>(&m_addr), m_addr.GetSize()) == SOCKET_ERROR)
	{
		const int32_t error = WSAGetLastError();

		if (error != WSAEWOULDBLOCK)
		{
			Sleep(1000);
			Clear();
			m_sock = socket(AF_INET, SOCK_STREAM, 0);
			if (m_sock == INVALID_SOCKET)
			{
				Clear();
				OnConnectFailure();
				return false;

			}

			ioctlsocket(m_sock, FIONBIO, &arg);
			if (connect(m_sock, reinterpret_cast<PSOCKADDR>(&m_addr), m_addr.GetSize()) == SOCKET_ERROR)
			{
				Tracen("error != WSAEWOULDBLOCK");
				Clear();
				OnConnectFailure();
				return false;
			}
		}
	}

	m_connectLimitTime = time(nullptr) + limitSec;
	return true;	
}

bool CNetworkStream::Connect(uint32_t dwAddr, int32_t port, int32_t limitSec)
{
	char szAddr[256];
	{
		uint8_t ip[4];
		ip[0] = dwAddr & 0xff;
		dwAddr >>= 8;
		ip[1] = dwAddr & 0xff;
		dwAddr >>= 8;
		ip[2] = dwAddr & 0xff;
		dwAddr >>= 8;
		ip[3] = dwAddr & 0xff;
		dwAddr >>= 8;

		sprintf_s(szAddr, "%u.%u.%u.%u", ip[0], ip[1], ip[2], ip[3]);
	}

	return Connect(szAddr, port, limitSec);
}

bool CNetworkStream::Connect(const char* c_szAddr, int32_t port, int32_t /*limitSec*/)
{
	Tracenf("Connect to %s:%d", c_szAddr, port);

	CNetworkAddress kNetAddr;
	kNetAddr.Set(c_szAddr, port);

	return Connect(kNetAddr);
}

void CNetworkStream::ClearRecvBuffer()
{
	m_recvBufOutputPos = m_recvBufInputPos = 0;	 
}

int32_t CNetworkStream::GetRecvBufferSize()
{
	return m_recvBufInputPos - m_recvBufOutputPos;
}

bool CNetworkStream::Peek(int32_t size)
{
	if (GetRecvBufferSize() < size)
		return false;

	return true;
}

bool CNetworkStream::Peek(int32_t size, char * pDestBuf)
{
	if (GetRecvBufferSize() < size)
		return false;

	memcpy(pDestBuf, m_recvBuf + m_recvBufOutputPos, size);
	return true;
}


#ifdef _PACKETDUMP
const char * GetSendHeaderName(uint8_t header)
{
	static bool b = false;
	static std::string stringList[UCHAR_MAX+1];
	if (b==false)
	{
		for (uint32_t i = 0; i < UCHAR_MAX+1; i++)
		{
			char buf[20];
			sprintf_s(buf, "UNKNOWN[%d]", i);
			stringList[i] = buf;
		}
		stringList[HEADER_CG_LOGIN] = "HEADER_CG_LOGIN";
		stringList[HEADER_CG_ATTACK] = "HEADER_CG_ATTACK";
		stringList[HEADER_CG_CHAT] = "HEADER_CG_CHAT";
		stringList[HEADER_CG_PLAYER_CREATE] = "HEADER_CG_PLAYER_CREATE";
		stringList[HEADER_CG_PLAYER_DESTROY] = "HEADER_CG_PLAYER_DESTROY";
		stringList[HEADER_CG_PLAYER_SELECT] = "HEADER_CG_PLAYER_SELECT";
		stringList[HEADER_CG_CHARACTER_MOVE] = "HEADER_CG_CHARACTER_MOVE";
		stringList[HEADER_CG_SYNC_POSITION] = "HEADER_CG_SYNC_POSITION";
		stringList[HEADER_CG_ENTERGAME] = "HEADER_CG_ENTERGAME";
		stringList[HEADER_CG_ITEM_USE] = "HEADER_CG_ITEM_USE";
		stringList[HEADER_CG_ITEM_MOVE] = "HEADER_CG_ITEM_MOVE";
		stringList[HEADER_CG_ITEM_PICKUP] = "HEADER_CG_ITEM_PICKUP";
		stringList[HEADER_CG_QUICKSLOT_ADD] = "HEADER_CG_QUICKSLOT_ADD";
		stringList[HEADER_CG_QUICKSLOT_DEL] = "HEADER_CG_QUICKSLOT_DEL";
		stringList[HEADER_CG_QUICKSLOT_SWAP] = "HEADER_CG_QUICKSLOT_SWAP";
		stringList[HEADER_CG_WHISPER] = "HEADER_CG_WHISPER";
		stringList[HEADER_CG_ITEM_DROP2] = "HEADER_CG_ITEM_DROP2";
		stringList[HEADER_CG_ON_CLICK] = "HEADER_CG_ON_CLICK";
		stringList[HEADER_CG_EXCHANGE] = "HEADER_CG_EXCHANGE";
		stringList[HEADER_CG_CHARACTER_POSITION] = "HEADER_CG_CHARACTER_POSITION";
		stringList[HEADER_CG_SCRIPT_ANSWER] = "HEADER_CG_SCRIPT_ANSWER";
		stringList[HEADER_CG_QUEST_INPUT_STRING] = "HEADER_CG_QUEST_INPUT_STRING";
		stringList[HEADER_CG_QUEST_CONFIRM] = "HEADER_CG_QUEST_CONFIRM";
		stringList[HEADER_CG_SHOP] = "HEADER_CG_SHOP";
		stringList[HEADER_CG_FLY_TARGETING] = "HEADER_CG_FLY_TARGETING";
		stringList[HEADER_CG_USE_SKILL] = "HEADER_CG_USE_SKILL";
		stringList[HEADER_CG_ADD_FLY_TARGETING] = "HEADER_CG_ADD_FLY_TARGETING";
		stringList[HEADER_CG_SHOOT] = "HEADER_CG_SHOOT";
		stringList[HEADER_CG_MYSHOP] = "HEADER_CG_MYSHOP";
		stringList[HEADER_CG_ITEM_USE_TO_ITEM] = "HEADER_CG_ITEM_USE_TO_ITEM";
		stringList[HEADER_CG_TARGET] = "HEADER_CG_TARGET";
		stringList[HEADER_CG_SCRIPT_BUTTON] = "HEADER_CG_SCRIPT_BUTTON";
		stringList[HEADER_CG_MESSENGER] = "HEADER_CG_MESSENGER";
		stringList[HEADER_CG_MALL_CHECKOUT] = "HEADER_CG_MALL_CHECKOUT";
		stringList[HEADER_CG_SAFEBOX_CHECKIN] = "HEADER_CG_SAFEBOX_CHECKIN";
		stringList[HEADER_CG_SAFEBOX_CHECKOUT] = "HEADER_CG_SAFEBOX_CHECKOUT";
		stringList[HEADER_CG_PARTY_INVITE] = "HEADER_CG_PARTY_INVITE";
		stringList[HEADER_CG_PARTY_INVITE_ANSWER] = "HEADER_CG_PARTY_INVITE_ANSWER";
		stringList[HEADER_CG_PARTY_REMOVE] = "HEADER_CG_PARTY_REMOVE";
		stringList[HEADER_CG_PARTY_SET_STATE] = "HEADER_CG_PARTY_SET_STATE";
		stringList[HEADER_CG_PARTY_USE_SKILL] = "HEADER_CG_PARTY_USE_SKILL";
		stringList[HEADER_CG_SAFEBOX_ITEM_MOVE] = "HEADER_CG_SAFEBOX_ITEM_MOVE";
		stringList[HEADER_CG_PARTY_PARAMETER] = "HEADER_CG_PARTY_PARAMETER";
		stringList[HEADER_CG_GUILD] = "HEADER_CG_GUILD";
		stringList[HEADER_CG_ANSWER_MAKE_GUILD] = "HEADER_CG_ANSWER_MAKE_GUILD";
		stringList[HEADER_CG_FISHING] = "HEADER_CG_FISHING";
		stringList[HEADER_CG_GIVE_ITEM] = "HEADER_CG_GIVE_ITEM";
		stringList[HEADER_CG_EMPIRE] = "HEADER_CG_EMPIRE";
		stringList[HEADER_CG_REFINE] = "HEADER_CG_REFINE";
		stringList[HEADER_CG_MARK_LOGIN] = "HEADER_CG_MARK_LOGIN";
		stringList[HEADER_CG_MARK_CRCLIST] = "HEADER_CG_MARK_CRCLIST";
		stringList[HEADER_CG_MARK_UPLOAD] = "HEADER_CG_MARK_UPLOAD";
		stringList[HEADER_CG_HACK] = "HEADER_CG_HACK";
		stringList[HEADER_CG_CHANGE_NAME] = "HEADER_CG_CHANGE_NAME";
		stringList[HEADER_CG_LOGIN2] = "HEADER_CG_LOGIN2";
		stringList[HEADER_CG_LOGIN3] = "HEADER_CG_LOGIN3";
		stringList[HEADER_CG_GUILD_SYMBOL_UPLOAD] = "HEADER_CG_GUILD_SYMBOL_UPLOAD";
		stringList[HEADER_CG_GUILD_SYMBOL_CRC] = "HEADER_CG_GUILD_SYMBOL_CRC";
		stringList[HEADER_CG_SCRIPT_SELECT_ITEM] = "HEADER_CG_SCRIPT_SELECT_ITEM";
#ifdef _IMPROVED_PACKET_ENCRYPTION_
		stringList[HEADER_CG_KEY_AGREEMENT] = "HEADER_CG_KEY_AGREEMENT";
#endif
		stringList[HEADER_CG_TIME_SYNC] = "HEADER_CG_TIME_SYNC";
		stringList[HEADER_CG_PONG] = "HEADER_CG_PONG";
		stringList[HEADER_CG_HANDSHAKE] = "HEADER_CG_HANDSHAKE";
	}
	return stringList[header].c_str();
}

const char * GetRecvHeaderName(uint8_t header)
{
	static bool b = false;
	static std::string stringList[UCHAR_MAX+1];
	if (b==false)
	{
		for (uint32_t i = 0; i < UCHAR_MAX+1; i++)
		{
			char buf[20];
			sprintf_s(buf, "UNKNOWN[%d]", i);
			stringList[i] = buf;
		}
		stringList[HEADER_GC_CHARACTER_ADD] = "HEADER_GC_CHARACTER_ADD";
		stringList[HEADER_GC_CHARACTER_DEL] = "HEADER_GC_CHARACTER_DEL";
		stringList[HEADER_GC_CHARACTER_MOVE] = "HEADER_GC_CHARACTER_MOVE";
		stringList[HEADER_GC_CHAT] = "HEADER_GC_CHAT";
		stringList[HEADER_GC_SYNC_POSITION] = "HEADER_GC_SYNC_POSITION";
		stringList[HEADER_GC_LOGIN_SUCCESS] = "HEADER_GC_LOGIN_SUCCESS";
		stringList[HEADER_GC_LOGIN_FAILURE] = "HEADER_GC_LOGIN_FAILURE";
		stringList[HEADER_GC_PLAYER_CREATE_SUCCESS] = "HEADER_GC_PLAYER_CREATE_SUCCESS";
		stringList[HEADER_GC_PLAYER_CREATE_FAILURE] = "HEADER_GC_PLAYER_CREATE_FAILURE";
		stringList[HEADER_GC_PLAYER_DELETE_SUCCESS] = "HEADER_GC_PLAYER_DELETE_SUCCESS";
		stringList[HEADER_GC_PLAYER_DELETE_WRONG_SOCIAL_ID] = "HEADER_GC_PLAYER_DELETE_WRONG_SOCIAL_ID";

		stringList[HEADER_GC_STUN] = "HEADER_GC_STUN";
		stringList[HEADER_GC_DEAD] = "HEADER_GC_DEAD";
		stringList[HEADER_GC_MAIN_CHARACTER] = "HEADER_GC_MAIN_CHARACTER";
		stringList[HEADER_GC_PLAYER_POINTS] = "HEADER_GC_PLAYER_POINTS";
		stringList[HEADER_GC_PLAYER_POINT_CHANGE] = "HEADER_GC_PLAYER_POINT_CHANGE";
		stringList[HEADER_GC_CHANGE_SPEED] = "HEADER_GC_CHANGE_SPEED";
		stringList[HEADER_GC_CHARACTER_UPDATE] = "HEADER_GC_CHARACTER_UPDATE";
		stringList[HEADER_GC_ITEM_SET] = "HEADER_GC_ITEM_SET";
		stringList[HEADER_GC_ITEM_USE] = "HEADER_GC_ITEM_USE";
		stringList[HEADER_GC_ITEM_UPDATE] = "HEADER_GC_ITEM_UPDATE";
		stringList[HEADER_GC_ITEM_GROUND_ADD] = "HEADER_GC_ITEM_GROUND_ADD";
		stringList[HEADER_GC_ITEM_GROUND_DEL] = "HEADER_GC_ITEM_GROUND_DEL";
		stringList[HEADER_GC_QUICKSLOT_ADD] = "HEADER_GC_QUICKSLOT_ADD";
		stringList[HEADER_GC_QUICKSLOT_DEL] = "HEADER_GC_QUICKSLOT_DEL";
		stringList[HEADER_GC_QUICKSLOT_SWAP] = "HEADER_GC_QUICKSLOT_SWAP";
		stringList[HEADER_GC_ITEM_OWNERSHIP] = "HEADER_GC_ITEM_OWNERSHIP";
		stringList[HEADER_GC_WHISPER] = "HEADER_GC_WHISPER	";
		stringList[HEADER_GC_MOTION] = "HEADER_GC_MOTION";
		stringList[HEADER_GC_SHOP] = "HEADER_GC_SHOP";
		stringList[HEADER_GC_SHOP_SIGN] = "HEADER_GC_SHOP_SIGN";
		stringList[HEADER_GC_PVP] = "HEADER_GC_PVP";
		stringList[HEADER_GC_EXCHANGE] = "HEADER_GC_EXCHANGE";
		stringList[HEADER_GC_CHARACTER_POSITION] = "HEADER_GC_CHARACTER_POSITION";
		stringList[HEADER_GC_PING] = "HEADER_GC_PING";
		stringList[HEADER_GC_SCRIPT] = "HEADER_GC_SCRIPT";
		stringList[HEADER_GC_QUEST_CONFIRM] = "HEADER_GC_QUEST_CONFIRM";

		stringList[HEADER_GC_OWNERSHIP] = "HEADER_GC_OWNERSHIP";
		stringList[HEADER_GC_OWNERSHIP] = "HEADER_GC_TARGET";
		stringList[HEADER_GC_WARP] = "HEADER_GC_WARP";
		stringList[HEADER_GC_ADD_FLY_TARGETING] = "HEADER_GC_ADD_FLY_TARGETING";

		stringList[HEADER_GC_CREATE_FLY] = "HEADER_GC_CREATE_FLY";
		stringList[HEADER_GC_FLY_TARGETING] = "HEADER_GC_FLY_TARGETING";
		stringList[HEADER_GC_SKILL_LEVEL] = "HEADER_GC_SKILL_LEVEL";
		stringList[HEADER_GC_MESSENGER] = "HEADER_GC_MESSENGER";
		stringList[HEADER_GC_GUILD] = "HEADER_GC_GUILD";
		stringList[HEADER_GC_PARTY_INVITE] = "HEADER_GC_PARTY_INVITE";
		stringList[HEADER_GC_PARTY_ADD] = "HEADER_GC_PARTY_ADD";
		stringList[HEADER_GC_PARTY_UPDATE] = "HEADER_GC_PARTY_UPDATE";
		stringList[HEADER_GC_PARTY_REMOVE] = "HEADER_GC_PARTY_REMOVE";
		stringList[HEADER_GC_QUEST_INFO] = "HEADER_GC_QUEST_INFO";
		stringList[HEADER_GC_REQUEST_MAKE_GUILD] = "HEADER_GC_REQUEST_MAKE_GUILD";
		stringList[HEADER_GC_PARTY_PARAMETER] = "HEADER_GC_PARTY_PARAMETER";
		stringList[HEADER_GC_SAFEBOX_SET] = "HEADER_GC_SAFEBOX_SET";
		stringList[HEADER_GC_SAFEBOX_DEL] = "HEADER_GC_SAFEBOX_DEL";
		stringList[HEADER_GC_SAFEBOX_WRONG_PASSWORD] = "HEADER_GC_SAFEBOX_WRONG_PASSWORD";
		stringList[HEADER_GC_SAFEBOX_SIZE] = "HEADER_GC_SAFEBOX_SIZE";
		stringList[HEADER_GC_FISHING] = "HEADER_GC_FISHING";
		stringList[HEADER_GC_EMPIRE] = "HEADER_GC_EMPIRE";
		stringList[HEADER_GC_PARTY_LINK] = "HEADER_GC_PARTY_LINK";
		stringList[HEADER_GC_PARTY_UNLINK] = "HEADER_GC_PARTY_UNLINK";

		stringList[HEADER_GC_REFINE_INFORMATION] = "HEADER_GC_REFINE_INFORMATION";
		stringList[HEADER_GC_VIEW_EQUIP] = "HEADER_GC_VIEW_EQUIP";
		stringList[HEADER_GC_MARK_BLOCK] = "HEADER_GC_MARK_BLOCK";

		stringList[HEADER_GC_TIME] = "HEADER_GC_TIME";
		stringList[HEADER_GC_CHANGE_NAME] = "HEADER_GC_CHANGE_NAME";
		stringList[HEADER_GC_DUNGEON] = "HEADER_GC_DUNGEON";
		stringList[HEADER_GC_WALK_MODE] = "HEADER_GC_WALK_MODE";
		stringList[HEADER_GC_CHANGE_SKILL_GROUP] = "HEADER_GC_CHANGE_SKILL_GROUP";
		stringList[HEADER_GC_NPC_POSITION] = "HEADER_GC_NPC_POSITION";
		stringList[HEADER_GC_CHARACTER_UPDATE] = "HEADER_GC_CHARACTER_UPDATE";
		stringList[HEADER_GC_LOGIN_KEY] = "HEADER_GC_LOGIN_KEY";
		stringList[HEADER_GC_CHANNEL] = "HEADER_GC_CHANNEL";
		stringList[HEADER_GC_MALL_OPEN] = "HEADER_GC_MALL_OPEN";
		stringList[HEADER_GC_TARGET_UPDATE] = "HEADER_GC_TARGET_UPDATE";
		stringList[HEADER_GC_TARGET_DELETE] = "HEADER_GC_TARGET_DELETE";
		stringList[HEADER_GC_TARGET_CREATE] = "HEADER_GC_TARGET_CREATE";
		stringList[HEADER_GC_AFFECT_ADD] = "HEADER_GC_AFFECT_ADD";
		stringList[HEADER_GC_AFFECT_REMOVE] = "HEADER_GC_AFFECT_REMOVE";
		stringList[HEADER_GC_MALL_SET] = "HEADER_GC_MALL_SET";
		stringList[HEADER_GC_MALL_DEL] = "HEADER_GC_MALL_DEL";
		stringList[HEADER_GC_LAND_LIST] = "HEADER_GC_LAND_LIST";
		stringList[HEADER_GC_LOVER_INFO] = "HEADER_GC_LOVER_INFO";
		stringList[HEADER_GC_LOVE_POINT_UPDATE] = "HEADER_GC_LOVE_POINT_UPDATE";
		stringList[HEADER_GC_GUILD_SYMBOL_DATA] = "HEADER_GC_GUILD_SYMBOL_DATA";
		stringList[HEADER_GC_DIG_MOTION] = "HEADER_GC_DIG_MOTION";
        stringList[HEADER_GC_MAIN_CHARACTER4_BGM_VOL] = "HEADER_GC_MAIN_CHARACTER4_BGM_VOL";
		stringList[HEADER_GC_DAMAGE_INFO] = "HEADER_GC_DAMAGE_INFO";
		stringList[HEADER_GC_CHAR_ADDITIONAL_INFO] = "HEADER_GC_CHAR_ADDITIONAL_INFO";
		stringList[HEADER_GC_AUTH_SUCCESS] = "HEADER_GC_AUTH_SUCCESS";
#ifdef _IMPROVED_PACKET_ENCRYPTION_
		stringList[HEADER_GC_KEY_AGREEMENT_COMPLETED] = "HEADER_GC_KEY_AGREEMENT_COMPLETED";
		stringList[HEADER_GC_KEY_AGREEMENT] = "HEADER_GC_KEY_AGREEMENT";
#endif
		stringList[HEADER_GC_HANDSHAKE_OK] = "HEADER_GC_HANDSHAKE_OK";
		stringList[HEADER_GC_PHASE] = "HEADER_GC_PHASE";
		stringList[HEADER_GC_HANDSHAKE] = "HEADER_GC_HANDSHAKE";
	}
	return stringList[header].c_str();
}

#endif

bool CNetworkStream::Recv(int32_t size)
{
	if (!Peek(size))
		return false;

	m_recvBufOutputPos += size;
	return true;
}

bool CNetworkStream::Recv(int32_t size, char * pDestBuf)
{
	if (!Peek(size, pDestBuf)) 
		return false;
	
#ifdef _PACKETDUMP
	if (*pDestBuf != 0 )
	{
		TraceError ("RECV< %s(%d[%02X]) | %d", GetRecvHeaderName(*pDestBuf), *pDestBuf, *pDestBuf, size);
		std::string contents;
		char buf[10];
		for(int32_t i = 1; i < size; i++)
		{
			sprintf_s(buf," %02x", (uint8_t)(pDestBuf[i]));
			contents.append(buf);
		}
		TraceError(contents.c_str());
	}
		
#endif

	m_recvBufOutputPos += size;
	return true;
}

int32_t CNetworkStream::__GetSendBufferSize()
{
	return m_sendBufInputPos-m_sendBufOutputPos;
}


bool CNetworkStream::Send(int32_t size, const char * pSrcBuf)
{
	int32_t sendBufRestSize = m_sendBufSize - m_sendBufInputPos;
	if ((size + 1) > sendBufRestSize)
		return false;

	memcpy(m_sendBuf + m_sendBufInputPos, pSrcBuf, size);
	m_sendBufInputPos += size;

#ifdef _PACKETDUMP
	if (*pSrcBuf != 0 )
	{
		TraceError ("SEND> %s(%d[%02X]) | %d", GetSendHeaderName(*pSrcBuf), *pSrcBuf, *pSrcBuf, size);
		std::string contents;
		char buf[10];
		for(int32_t i = 1; i < size; i++)
		{
			sprintf_s(buf," %02x", (uint8_t)(pSrcBuf[i]));
			contents.append(buf);
		}
		TraceError(contents.c_str());

	}
#endif

	return true;
	/*
	if (size > 0)
	{
		if (IsSecurityMode())
		{
			m_sendBufInputPos += TEA_Encrypt((uint32_t *) (m_sendBuf + m_sendBufInputPos),
											 (uint32_t *) (m_sendBuf + m_sendBufInputPos),
											 (const uint32_t *) gs_szTeaKey,
											 size);
		}
		else
		{
		}
	}

	return __SendInternalBuffer();
	*/
}

bool CNetworkStream::Peek(int32_t len, void* pDestBuf)
{
	return Peek(len, static_cast<char *>(pDestBuf));
}

bool CNetworkStream::Recv(int32_t len, void* pDestBuf)
{
	return Recv(len, static_cast<char *>(pDestBuf));
}

bool CNetworkStream::SendFlush(int32_t len, const void* pSrcBuf)
{
	if (!Send(len, pSrcBuf))
		return false;

	return __SendInternalBuffer();
}

bool CNetworkStream::Send(int32_t len, const void* pSrcBuf)
{
	return Send(len, static_cast<const char *>(pSrcBuf));
}

bool CNetworkStream::IsOnline()
{
	return m_isOnline;
}

bool CNetworkStream::OnProcess()
{
	return true;
}

void CNetworkStream::OnRemoteDisconnect()
{
}

void CNetworkStream::OnDisconnect()
{
}

void CNetworkStream::OnConnectSuccess()
{
}

void CNetworkStream::OnConnectFailure()
{
	// should be overwritten
}

//void CNetworkStream::OnCheckinSuccess()
//{
//}

//void CNetworkStream::OnCheckinFailure()
//{
//}


CNetworkStream::CNetworkStream()
{
	m_sock = INVALID_SOCKET;

#ifndef _IMPROVED_PACKET_ENCRYPTION_
	m_isSecurityMode = false;
#endif
	m_isOnline = false;
	m_connectLimitTime = 0;

	m_recvTEABuf = nullptr;
	m_recvTEABufSize = 0;
	m_recvTEABufInputPos = 0;
	
	m_recvBuf = nullptr;	
	m_recvBufSize = 0;	
	m_recvBufOutputPos = 0;
	m_recvBufInputPos = 0;	

	m_sendTEABuf = nullptr;
	m_sendTEABuf = 0;
	m_sendTEABufInputPos = 0;

	m_sendBuf = nullptr;	
	m_sendBufSize = 0;	
	m_sendBufOutputPos = 0;
	m_sendBufInputPos = 0;
}

CNetworkStream::~CNetworkStream()
{
	Clear();

	if (m_sendTEABuf)
	{
		delete [] m_sendTEABuf;
		m_sendTEABuf=nullptr;
	}

	if (m_recvTEABuf)
	{
		delete [] m_recvTEABuf;
		m_recvTEABuf=nullptr;
	}

	if (m_recvBuf)
	{
		delete [] m_recvBuf;
		m_recvBuf=nullptr;
	}

	if (m_sendBuf)
	{
		delete [] m_sendBuf;
		m_sendBuf=nullptr;
	}
}

#ifdef _IMPROVED_PACKET_ENCRYPTION_
size_t CNetworkStream::Prepare(void* buffer, size_t* length)
{
	return m_cipher.Prepare(buffer, length);
}

bool CNetworkStream::Activate(size_t agreed_length, const void* buffer, size_t length)
{
	return m_cipher.Activate(true, agreed_length, buffer, length);
}

void CNetworkStream::ActivateCipher()
{
	return m_cipher.set_activated(true);
}
#endif // _IMPROVED_PACKET_ENCRYPTION_
