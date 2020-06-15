#include "stdafx.h"
#include "PeerBase.h"
#include "../../common/service.h"

CPeerBase::CPeerBase() : m_fd(INVALID_SOCKET), m_wPort(0), m_BytesRemain(0), m_outBuffer(nullptr), m_inBuffer(nullptr)
{
}

CPeerBase::~CPeerBase()
{
	Destroy();
}

void CPeerBase::Disconnect()
{
	if (m_fd != INVALID_SOCKET)
	{
		fdwatch_del_fd(m_fdWatcher, m_fd);

		socket_close(m_fd);
		m_fd = INVALID_SOCKET;
	}
}

void CPeerBase::Destroy()
{
	Disconnect();

	if (m_outBuffer)
	{
		buffer_delete(m_outBuffer);
		m_outBuffer = nullptr;
	}

	if (m_inBuffer)
	{
		buffer_delete(m_inBuffer);
		m_inBuffer = nullptr;
	}
}

bool CPeerBase::Accept(socket_t fd_accept)
{
	struct sockaddr_in peer;

	if ((m_fd = socket_accept(fd_accept, &peer)) == INVALID_SOCKET)
	{
		Destroy();
		return false;
	}

#ifdef ENABLE_PORT_SECURITY
	// refuse if remote host != localhost (only the same machine must be able to connect in here)
	std::string targetIP = inet_ntoa(peer.sin_addr);
	if (targetIP.rfind("127.0.0.1", 0) && targetIP.rfind("192.168.", 0) && targetIP.rfind("10.", 0))
	{
		sys_err("BLOCK CONNECTION FROM %s", inet_ntoa(peer.sin_addr));
		Destroy();
		return false;
	}
#endif

	//socket_block(m_fd);
	socket_sndbuf(m_fd, 233016);
	socket_rcvbuf(m_fd, 233016);

	strlcpy(m_host, inet_ntoa(peer.sin_addr), sizeof(m_host));
	m_wPort = peer.sin_port;

	m_outBuffer = buffer_new(DEFAULT_PACKET_BUFFER_SIZE);
	m_inBuffer = buffer_new(MAX_INPUT_LEN);

	if (!m_outBuffer || !m_inBuffer)
	{
		Destroy();
		return false;
	}

	fdwatch_add_fd(m_fdWatcher, m_fd, this, FDW_READ, false);

	OnAccept();
	sys_log(0, "ACCEPT FROM %s", inet_ntoa(peer.sin_addr));
	return true;
}

bool CPeerBase::Connect(const char* host, uint16_t port)
{
	strlcpy(m_host, host, sizeof(m_host));
	m_wPort = port;

	if ((m_fd = socket_connect(host, port)) == INVALID_SOCKET)
		return false;

	m_outBuffer = buffer_new(DEFAULT_PACKET_BUFFER_SIZE);

	if (!m_outBuffer)
	{
		Destroy();
		return false;
	}

	fdwatch_add_fd(m_fdWatcher, m_fd, this, FDW_READ, false);

	OnConnect();
	return true;
}

void CPeerBase::Close()
{
	OnClose();
}

void CPeerBase::EncodeBYTE(uint8_t b)
{
	if (!m_outBuffer)
	{
		sys_err("Not ready to write");
		return;
	}

	buffer_write(m_outBuffer, &b, 1);
	fdwatch_add_fd(m_fdWatcher, m_fd, this, FDW_WRITE, true);
}

void CPeerBase::EncodeWORD(uint16_t w)
{
	if (!m_outBuffer)
	{
		sys_err("Not ready to write");
		return;
	}

	buffer_write(m_outBuffer, &w, 2);
	fdwatch_add_fd(m_fdWatcher, m_fd, this, FDW_WRITE, true);
}

void CPeerBase::EncodeDWORD(uint32_t dw)
{
	if (!m_outBuffer)
	{
		sys_err("Not ready to write");
		return;
	}

	buffer_write(m_outBuffer, &dw, 4);
	fdwatch_add_fd(m_fdWatcher, m_fd, this, FDW_WRITE, true);
}

void CPeerBase::EncodeQWORD(uint64_t dw)
{
	if (!m_outBuffer)
	{
		sys_err("Not ready to write");
		return;
	}
	buffer_write(m_outBuffer, &dw, 8);
	fdwatch_add_fd(m_fdWatcher, m_fd, this, FDW_WRITE, true);
}

void CPeerBase::Encode(const void* data, uint32_t size)
{
	if (!m_outBuffer)
	{
		sys_err("Not ready to write");
		return;
	}

	buffer_write(m_outBuffer, data, size);
	fdwatch_add_fd(m_fdWatcher, m_fd, this, FDW_WRITE, true);
}

int32_t CPeerBase::Recv()
{
	if (!m_inBuffer)
	{
		sys_err("input buffer nil");
		return -1;
	}

	buffer_adjust_size(m_inBuffer, MAX_INPUT_LEN >> 2);
	int32_t bytes_to_read = buffer_has_space(m_inBuffer);
	ssize_t bytes_read = socket_read(m_fd, (char *) buffer_write_peek(m_inBuffer), bytes_to_read);

	if (bytes_read < 0)
	{
		sys_err("socket_read failed %s", strerror(errno));
		return -1;
	}
	else if (bytes_read == 0)
		return 0;

	buffer_write_proceed(m_inBuffer, bytes_read);
	m_BytesRemain = buffer_size(m_inBuffer);
	return 1;
}

void CPeerBase::RecvEnd(int32_t proceed_bytes)
{
	buffer_read_proceed(m_inBuffer, proceed_bytes);
	m_BytesRemain = buffer_size(m_inBuffer);
}

int32_t CPeerBase::GetRecvLength()
{
	return m_BytesRemain;
}

const void * CPeerBase::GetRecvBuffer()
{
	return buffer_read_peek(m_inBuffer);
}

int32_t CPeerBase::GetSendLength()
{
	return buffer_size(m_outBuffer);
}

int32_t CPeerBase::Send()
{
	if (buffer_size(m_outBuffer) <= 0)
		return 0;

	int32_t iBufferLeft = fdwatch_get_buffer_size(m_fdWatcher, m_fd);
	int32_t iBytesToWrite = MIN(iBufferLeft, buffer_size(m_outBuffer));

	if (iBytesToWrite == 0)
		return 0;

	int32_t result = socket_write(m_fd, reinterpret_cast<const char *>(buffer_read_peek(m_outBuffer)), iBytesToWrite);

	if (result == 0)
	{
		buffer_read_proceed(m_outBuffer, iBytesToWrite);

		if (buffer_size(m_outBuffer) != 0)
			fdwatch_add_fd(m_fdWatcher, m_fd, this, FDW_WRITE, true);
	}

	return (result);
}
