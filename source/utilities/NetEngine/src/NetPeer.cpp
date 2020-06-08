#include "../include/NetEngine.h"

#ifndef NETWORK_MAX_ALLOWED_SIZE
	#define NETWORK_MAX_ALLOWED_SIZE	asio::detail::default_max_transfer_size_t::default_max_transfer_size
#endif

namespace net_engine
{
	NetPeerBase::NetPeerBase(asio::io_context& service) :
		m_service(service), m_socket(service),
		m_inputOffset(0), m_outputOffset(0),
		m_port(0), disconnect_timer(service)
	{
		m_input.resize(NETWORK_MAX_ALLOWED_SIZE / 2); // 32767
		m_output.resize(NETWORK_MAX_ALLOWED_SIZE); // 65536
	}

	const std::string& NetPeerBase::GetIP() const
	{
		try
		{
			std::const_pointer_cast<NetPeerBase>(shared_from_this())->m_ip = m_socket.remote_endpoint().address().to_string();
		}
		catch (...) {}
		return m_ip;
	}
	uint16_t NetPeerBase::GetPort() const
	{
		try
		{
			std::const_pointer_cast<NetPeerBase>(shared_from_this())->m_port = m_socket.remote_endpoint().port();
		}
		catch (...) {}
		return m_port;
	}
	bool NetPeerBase::IsConnected() const
	{
		return m_socket.is_open();
	}
	asio::ip::tcp::socket& NetPeerBase::GetSocket()
	{
		return m_socket;
	}
	const asio::ip::tcp::socket& NetPeerBase::GetSocket() const
	{
		return m_socket;
	}

	void NetPeerBase::Disconnect(const asio::error_code& er)
	{
		try
		{
			disconnect_timer.cancel();

			if (m_socket.is_open())
			{
				OnDisconnect(er);

				m_socket.shutdown(asio::socket_base::shutdown_both);
				m_socket.close();

				m_inputOffset = 0;
				m_outputOffset = 0;
			}
		}
		catch (const asio::system_error& e)
		{
			NET_LOG(LL_ERR, "Exception occured: %s", e.what());
			m_inputOffset = 0;
			m_outputOffset = 0;
		}
		catch (...)
		{
			NET_LOG(LL_ERR, "Unhandled exception occured");
			m_inputOffset = 0;
			m_outputOffset = 0;
		}
	}

	std::size_t NetPeerBase::Send(const void* data, std::size_t length, bool flush)
	{
		if (m_outputOffset + length > m_output.capacity())
			length = m_output.size() - m_outputOffset;
		memcpy(&m_output[m_outputOffset], data, length);
		m_outputOffset += length;

		if (flush)
			Flush();
		return length;
	}
	void NetPeerBase::Flush()
	{
		if (m_outputOffset > 0)
		{
			std::size_t processedSize = OnWritePre(&m_output[0], m_outputOffset);
			asio::async_write(m_socket, asio::buffer(&m_output[0], processedSize), std::bind(&NetPeerBase::HandleWrite, shared_from_this(), std::placeholders::_1, std::placeholders::_2, processedSize));
		}
	}
	// todo async

	void NetPeerBase::HandleWrite(std::weak_ptr<NetPeerBase> self, const asio::error_code& er, std::size_t succesed_size, std::size_t length)
	{
		std::shared_ptr<NetPeerBase> _this(self.lock());
		if (_this)
		{
			if (!er)
			{
				// todo async
				if (length < _this->m_outputOffset)
				{
					memcpy(&_this->m_output[0], &_this->m_output[length], _this->m_outputOffset - length);
					_this->m_outputOffset = _this->m_outputOffset - length;
				}
				else
				{
					_this->m_outputOffset = 0;
				}
			}
			else
			{
				if (er != asio::error::operation_aborted)
				{
					NET_LOG(LL_CRI, "Write operation fail! Error: %u(%s)", er.value(), er.message().c_str());
					_this->OnError(NET_PEER_ERR_WRITE_ERROR, er);
					_this->Disconnect(er);
				}
			}
		}
	}

	void NetPeerBase::BeginRead() // todo async
	{
		m_socket.async_read_some(
			asio::buffer(&m_input[m_inputOffset], m_input.size() - m_inputOffset),
			std::bind(&NetPeerBase::HandleRead, shared_from_this(), std::placeholders::_1, std::placeholders::_2)
		);
	}

	void NetPeerBase::HandleRead(std::weak_ptr<NetPeerBase> self, const asio::error_code& er, std::size_t succesed_size)
	{
		std::shared_ptr<NetPeerBase> _this(self.lock());
		if (_this)
		{
			if (!er)
			{
				// todo async

				std::size_t l = _this->OnRead(&_this->m_input[0], succesed_size + _this->m_inputOffset);
				if (l < succesed_size + _this->m_inputOffset)
				{
					memcpy(&_this->m_input[0], &_this->m_input[l], succesed_size + _this->m_inputOffset - l);
					_this->m_inputOffset = succesed_size + _this->m_inputOffset - l;
				}
				else
				{
					_this->m_inputOffset = 0;
				}
			}
			else
			{
				if (er == asio::error::eof || er == asio::error::connection_reset)
				{
					NET_LOG(LL_SYS, "Disconnected by user!");
					_this->Disconnect(er);					
				}
				else if (er != asio::error::operation_aborted)
				{
					NET_LOG(LL_CRI, "Read operation fail! Error: %u(%s)", er.value(), er.message().c_str());
					_this->OnError(NET_PEER_ERR_READ_ERROR, er);
					_this->Disconnect(er);
				}
			}
		}
	}

	void NetPeerBase::DelayedDisconnect(int timeOut)
	{
		disconnect_timer.expires_from_now(std::chrono::milliseconds(timeOut));
		disconnect_timer.async_wait(std::bind(&NetPeerBase::HandleDelayedDisconnect, shared_from_this(), std::placeholders::_1));
	}

	void NetPeerBase::HandleDelayedDisconnect(std::weak_ptr<NetPeerBase> self, const asio::error_code& er)
	{
		std::shared_ptr<NetPeerBase> _this(self.lock());
		if (_this)
		{
			if (!er)
			{
				NET_LOG(LL_SYS, "Delayed disconnect timeout! Connection dropped");
				_this->Disconnect(er);
			}
		}
	}
}
