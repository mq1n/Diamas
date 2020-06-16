#include "../include/NetEngine.hpp"

namespace net_engine
{
	NetPeerBase::NetPeerBase(asio::io_context& service) :
		m_strand(asio::make_strand(service.get_executor())),
		m_service(service), 
		m_socket(asio::make_strand(service.get_executor())), // , m_socket(service),
		m_port(0), disconnect_timer(service)
	{
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
			NET_LOG(LL_TRACE, "Shuting down connection: %s", m_socket.remote_endpoint().address().to_string().c_str());

			m_isShutingDown = true;

			disconnect_timer.cancel();

			if (m_socket.is_open())
			{
				OnDisconnect(er);

				asio::error_code ec;

				m_socket.shutdown(asio::socket_base::shutdown_both, ec);
				if (ec)
				{
					NET_LOG(LL_ERR, "Failed to shutdown connection. Error: %d(%s)", ec.value(), ec.message().c_str());
				}

				m_socket.close(ec);
				if (ec)
				{
					NET_LOG(LL_ERR, "Failed to close connection. Error: %d(%s)", ec.value(), ec.message().c_str());
				}
			}
		}
		catch (const asio::system_error& e)
		{
			NET_LOG(LL_ERR, "Exception occured: %s", e.what());
		}
		catch (...)
		{
			NET_LOG(LL_ERR, "Unhandled exception occured");
		}
	}

	// this supposed to be called outside of the network thread
	void NetPeerBase::PostShutdown()
	{
		if (m_isShutingDown.exchange(true))
			return;

//		asio::post(m_strand, std::bind(&NetPeerBase::Disconnect, shared_from_this(), std::placeholders::_1));
	}

	void NetPeerBase::Send(std::shared_ptr <Packet> packet)
	{
		std::lock_guard<std::mutex> guard(m_sendMutex);
		m_sendQueue.push(packet);

		// if the send queue has data before push it means that the HandleSend
		// is already posted or its running from the HandleWrite
		if (m_sendQueue.size() == 1)
		{
			asio::post(m_strand, std::bind(&NetPeerBase::HandleSend, shared_from_this()));
		}
	}

	void NetPeerBase::HandleWrite(std::weak_ptr<NetPeerBase> self, const asio::error_code& er, std::size_t succesed_size)
	{
		std::shared_ptr <NetPeerBase> _this(self.lock());
		if (_this)
		{
			_this->m_writeBuffer.consume(succesed_size);

			if (!er)
			{
				if (_this->m_sendQueue.empty())
					return;

				_this->m_sendQueue.pop();

				if (_this->m_sendQueue.empty())
					return;
				
				HandleSend(self);
			}
			else
			{
				if (er != asio::error::operation_aborted)
				{
					NET_LOG(LL_CRI, "Write operation fail! Error: %u(%s)", er.value(), er.message().c_str());
					_this->OnError(PeerErrorWriteFail, er);
					_this->Disconnect(er);
				}
			}
		}
	}

	void NetPeerBase::HandleSend(std::weak_ptr<NetPeerBase> self)
	{
		std::shared_ptr <NetPeerBase> _this(self.lock());
		if (_this)
		{
			if (!_this->m_socket.is_open())
				return;
			
			if (_this->m_sendQueue.empty())
				return;

			auto packet = _this->m_sendQueue.front();

			NET_LOG(LL_TRACE, "Sending packet: %s(%u)", packet->GetName().c_str(), packet->GetHeader());

			uint8_t header = packet->GetHeader();
			auto data = packet->GetData();
			auto buffer = _this->m_writeBuffer.prepare(1 + data.size());

#if ENABLE_CRYPT
			if (_cryptation && _cryptation->IsReady())
			{
				_cryptation->Encrypt(static_cast<uint8_t*>(buffer.data()), &header, 1);
				_cryptation->Encrypt(static_cast<uint8_t*>(buffer.data()) + 1, data.data(), data.size());
			}
			else
#endif
			{
				asio::buffer_copy(asio::buffer(buffer, 1), asio::buffer(&header, 1));
				asio::buffer_copy(asio::buffer(buffer + 1, data.size()), asio::buffer(data.data(), data.size()));
			}

			_this->m_writeBuffer.commit(buffer.size());

			_this->m_socket.async_write_some(buffer,
				asio::bind_executor(_this->m_strand,
					std::bind(&NetPeerBase::HandleWrite, self, std::placeholders::_1, std::placeholders::_2)
				)
			);
		}
	}

	void NetPeerBase::BeginRead()
	{
		NET_LOG(LL_TRACE, "Reading next packet");

		asio::async_read(
			m_socket,
			m_buffer.prepare(1), asio::transfer_exactly(1),
			asio::bind_executor(m_strand,
				std::bind(&NetPeerBase::HandleReadHeader, shared_from_this(), std::placeholders::_1, std::placeholders::_2)
			)
		);
	}

	void NetPeerBase::HandleReadHeader(std::weak_ptr <NetPeerBase> self, const asio::error_code& er, std::size_t succesed_size)
	{
		std::shared_ptr <NetPeerBase> _this(self.lock());
		if (_this)
		{
			_this->m_buffer.commit(succesed_size);

			if (!er)
			{
				NET_LOG(LL_TRACE, "(HEADER) Received %u bytes", succesed_size);

				uint8_t header;
				asio::buffer_copy(asio::buffer(&header, sizeof(header)), _this->m_buffer.data());

#if ENABLE_CRYPT
				if (_cryptation)
				{
					if (_phase > PHASE_HANDSHAKE && !_cryptation->IsReady()) {
						// workaround for broken client implementation
						_cryptation->Activate();
					}

					if (_cryptation->IsReady()) {
						_cryptation->Decrypt(
							&header,
							reinterpret_cast<const uint8_t*>(asio::buffer(_buffer.data(), sizeof(header)).data()),
							1);
					}
				}
#endif
				_this->m_buffer.consume(1);

				NET_LOG(LL_TRACE, "Received header: 0x%02x", header);

				auto packet = PacketManager::Instance().CreatePacket(header, Incoming);
				if (!packet)
				{
					NET_LOG(LL_TRACE, "Received unknown header: 0x%02x", header);
					_this->OnError(PeerErrorUnknownHeader, er);
					_this->Disconnect(er);
					return;
				}

				// Check if packet is dynamic size
				if (packet->IsDynamicSized())
				{
					NET_LOG(LL_TRACE, "Reading dynamic packet size");

					// We have to read our packet size first
					asio::async_read(
						_this->m_socket,
						_this->m_buffer.prepare(2), asio::transfer_exactly(2),
						asio::bind_executor(_this->m_strand,
							std::bind(&NetPeerBase::HandleReadSize, self, packet, std::placeholders::_1, std::placeholders::_2)
						)
					);
					return;
				}

				auto sizeToRead = packet->GetSize();
				if (packet->HasSequence())
					sizeToRead += 1;

				NET_LOG(LL_TRACE, "Reading the next %u bytes", sizeToRead);

				asio::async_read(
					_this->m_socket,
					_this->m_buffer.prepare(sizeToRead), asio::transfer_exactly(sizeToRead),
					asio::bind_executor(_this->m_strand,
						std::bind(&NetPeerBase::HandleReadData, self, packet, std::placeholders::_1, std::placeholders::_2)
					)
				);
			}
			else
			{
				if (er == asio::error::eof || er == asio::error::connection_reset)
				{
					NET_LOG(LL_SYS, "Connection closed by remote: %s", _this->m_socket.remote_endpoint().address().to_string().c_str());
				}
				else if (er != asio::error::operation_aborted)
				{
					NET_LOG(LL_CRI, "Read operation fail! Error: %u(%s)", er.value(), er.message().c_str());
					_this->OnError(PeerErrorReadFail, er);
				}
				_this->Disconnect(er);
			}
		}
	}
	void NetPeerBase::HandleReadSize(std::weak_ptr <NetPeerBase> self, std::shared_ptr<Packet> packet, const asio::error_code& er, std::size_t succesed_size)
	{
        assert(succesed_size == 2);

		std::shared_ptr <NetPeerBase> _this(self.lock());
		if (_this)
		{
			NET_LOG(LL_TRACE, "(SIZE) Received %u bytes", succesed_size);

			_this->m_buffer.commit(succesed_size);

			if (!er)
			{
				uint32_t size = 0;

#if ENABLE_CRYPT
				if (_cryptation && _cryptation->IsReady())
				{
					std::vector<uint8_t> buf(transferred);

					_cryptation->Decrypt(buf.data(), reinterpret_cast<const uint8_t*>(_buffer.data().data()), transferred);
					_buffer.consume(transferred);

					size = (buf[1] << 8) | buf[0];
				}
				else
#endif
				{
					asio::buffer_copy(asio::buffer(static_cast<void*>(&size), 2), _this->m_buffer.data());  // todo: clean solution
				}

				NET_LOG(LL_TRACE, "Size of packet is %u", size);

				auto sizeToRead = size - 1 - 2;  // already read header (1byte) and size (2bytes)
				if (packet->HasSequence())
				{
					sizeToRead += 1;
				}

				NET_LOG(LL_TRACE, "Reading the next %u bytes", sizeToRead);
				asio::async_read(
					_this->m_socket,
					_this->m_buffer.prepare(sizeToRead), asio::transfer_exactly(sizeToRead),
					asio::bind_executor(_this->m_strand,
						std::bind(&NetPeerBase::HandleReadData, self, packet, std::placeholders::_1, std::placeholders::_2)
					)
				);
			}
			else
			{
				if (er == asio::error::eof || er == asio::error::connection_reset)
				{
					NET_LOG(LL_SYS, "Connection closed by remote: %s", _this->m_socket.remote_endpoint().address().to_string().c_str());
				}
				else if (er != asio::error::operation_aborted)
				{
					NET_LOG(LL_CRI, "Read operation fail! Error: %u(%s)", er.value(), er.message().c_str());
					_this->OnError(PeerErrorReadFail, er);
				}
				_this->Disconnect(er);
			}
		}
    }

	void NetPeerBase::HandleReadData(std::weak_ptr <NetPeerBase> self, std::shared_ptr<Packet> packet, const asio::error_code& er, std::size_t succesed_size)
	{
		std::shared_ptr <NetPeerBase> _this(self.lock());
		if (_this)
		{
			NET_LOG(LL_TRACE, "(DATA) Received %u bytes", succesed_size);
			_this->m_buffer.commit(succesed_size);

			if (!er)
			{
#if ENABLE_CRYPT
				if (_cryptation && _cryptation->IsReady())
				{
					std::vector<uint8_t> buf(transferred);

					_cryptation->Decrypt(buf.data(), reinterpret_cast<const uint8_t*>(_buffer.data().data()), transferred);
					_buffer.consume(transferred);

					packet->CopyData(buf, packet->IsDynamicSized() ? 2 : 0);
				}
				else
#endif
				{
					packet->CopyData(_this->m_buffer);
				}

				_this->OnRead(packet);

				_this->BeginRead();
			}
			else
			{
				if (er == asio::error::eof || er == asio::error::connection_reset)
				{
					NET_LOG(LL_SYS, "Connection closed by remote: %s", _this->m_socket.remote_endpoint().address().to_string().c_str());
				}
				else if (er != asio::error::operation_aborted)
				{
					NET_LOG(LL_CRI, "Read operation fail! Error: %u(%s)", er.value(), er.message().c_str());
					_this->OnError(PeerErrorReadFail, er);
				}
				_this->Disconnect(er);
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
