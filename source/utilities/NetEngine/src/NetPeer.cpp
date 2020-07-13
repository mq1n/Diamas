#include "../include/NetEngine.hpp"
#include <random.hpp>
#include <functional>
#include "../../../common/constants.h"

namespace net_engine
{
	using Random = effolkronium::random_static;

	NetPeerBase::NetPeerBase(asio::io_context& service, uint8_t securityLevel, const TPacketCryptKey& cryptKey, bool is_server) :
		m_strand(asio::make_strand(service.get_executor())),
		m_socket(m_strand),
		m_service(service), 
		m_isShutingDown(false),
		m_buffer(),
		m_writeBuffer(),
		m_port(0), disconnect_timer(service),
		m_logFlag(0),
		m_crypt_key(cryptKey), m_securityLevel(securityLevel), m_phase(0), m_core_time(0),
		m_handshaking(false),
		m_is_server(is_server)
	{
		m_handshake = Random::get<uint32_t>(
			std::numeric_limits<uint32_t>::min(),
			std::numeric_limits<uint32_t>::max()
		);

		if (securityLevel == SECURITY_LEVEL_KEY_AGREEMENT)
		{
			m_cryptation = std::make_unique<KeyAgreementCryptation>();
		}
		else if (securityLevel == SECURITY_LEVEL_XTEA)
		{
			m_cryptation = std::make_unique<XTEACryptation>();
			m_cryptation->AddData(XTEA_CRYPTATION_START_KEY, m_crypt_key.data(), PACKET_CRYPT_KEY_LENGTH);
		}
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

	void NetPeerBase::SetupPeer()
	{
		NET_LOG(LL_SYS, "New connection: %s handled!", m_socket.remote_endpoint().address().to_string().c_str());

		BeginRead();

		if (m_securityLevel > SECURITY_LEVEL_NONE && m_is_server)
		{
			StartHandshakeGC();
		}
		else
		{
			OnConnect();
		}
	}

	void NetPeerBase::Disconnect(const asio::error_code& er)
	{
		try
		{
			NET_LOG(LL_SYS, "Shuting down connection: %s", m_socket.remote_endpoint().address().to_string().c_str());

			m_isShutingDown = true;

			disconnect_timer.cancel();

			if (m_socket.is_open())
			{
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

				OnDisconnect(er);
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
	void NetPeerBase::Disconnect2()
	{
		asio::error_code er;
		Disconnect(er);
	}

	// this supposed to be called outside of the network thread
	void NetPeerBase::PostShutdown()
	{
		if (m_isShutingDown.exchange(true))
			return;

		asio::post(m_strand, std::bind(&NetPeerBase::Disconnect2, shared_from_this()));
	}

	void NetPeerBase::Send(std::shared_ptr <Packet> packet)
	{
		if (m_isShutingDown)
			return;

		m_send_queue.Enqueue(packet);

		asio::post(m_strand, std::bind(&NetPeerBase::HandleSend, shared_from_this()));
	}

	void NetPeerBase::HandleSend(std::weak_ptr<NetPeerBase> self)
	{
		std::shared_ptr <NetPeerBase> _this(self.lock());
		if (_this)
		{
			if (_this->m_isShutingDown)
				return;

			if (!_this->m_socket.is_open())
				return;
			
			std::shared_ptr <Packet> packet;
			if (_this->m_send_queue.Dequeue(packet))
    		{
				auto header = packet->GetHeader();
				auto data 	= packet->GetData();
				auto buffer = _this->m_writeBuffer.prepare(1 + data.size());

				NET_LOG(LL_SYS, "Sending packet: %s %u(0x%x) Size: %u",
					packet->GetName().c_str(), header, header, data.size());


				// debug
				/*
				std::stringstream ss;

				ss << "Data: \n";
				for (size_t i = 0; i < data.size(); ++i)
					ss << "0x" << std::hex << std::to_string(data.at(i)) << ", ";

				auto str = ss.str();
				std::cout << str.substr(0, str.size() - 2) << std::endl;
				*/

				if (_this->m_cryptation && _this->m_cryptation->IsReady())
				{
					_this->m_cryptation->Encrypt(static_cast<uint8_t*>(buffer.data()), &header, 1);
					_this->m_cryptation->Encrypt(static_cast<uint8_t*>(buffer.data()) + 1, data.data(), data.size());
				}
				else
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
	}

	void NetPeerBase::HandleWrite(std::weak_ptr<NetPeerBase> self, const asio::error_code& er, std::size_t succesed_size)
	{
		std::shared_ptr <NetPeerBase> _this(self.lock());
		if (_this)
		{
			_this->m_writeBuffer.consume(succesed_size);

			if (!er)
			{
				NET_LOG(LL_SYS, "Written: %u bytes!", succesed_size);
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

	void NetPeerBase::BeginRead()
	{
		NET_LOG(LL_SYS, "Reading next packet");

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
			if (_this->m_isShutingDown)
				return;

			if (!_this->m_socket.is_open())
				return;
				
			_this->m_buffer.commit(succesed_size);

			if (!er)
			{
				NET_LOG(LL_SYS, "(HEADER) Received %u bytes", succesed_size);

				uint8_t header;
				asio::buffer_copy(asio::buffer(&header, sizeof(header)), _this->m_buffer.data());

				if (_this->m_cryptation)
				{
					if (_this->m_phase > PHASE_HANDSHAKE && !_this->m_cryptation->IsReady())
					{
						// workaround for broken client implementation
						_this->m_cryptation->Activate();
					}

					if (_this->m_cryptation->IsReady())
					{
						_this->m_cryptation->Decrypt(&header, reinterpret_cast<const uint8_t*>(asio::buffer(_this->m_buffer.data(), sizeof(header)).data()), 1);
					}
				}

				_this->m_buffer.consume(1);

				NET_LOG(LL_SYS, "Received header: %d(0x%02x)", header, header);

				auto packet = PacketManager::Instance().CreatePacket(header, EPacketDirection::Incoming);
				if (!packet)
				{
					NET_LOG(LL_SYS, "Received unknown header: %d(0x%02x)", header, header);
					_this->OnError(PeerErrorUnknownHeader, er);
					_this->Disconnect(er);
					return;
				}

				// Check if packet is dynamic size
				if (packet->IsDynamicSized())
				{
					NET_LOG(LL_SYS, "Reading dynamic packet size");

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

				NET_LOG(LL_SYS, "Reading the next %u bytes", sizeToRead);

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
			NET_LOG(LL_SYS, "(SIZE) Received %u bytes", succesed_size);

			_this->m_buffer.commit(succesed_size);

			if (!er)
			{
				uint32_t size = 0;

				if (_this->m_cryptation && _this->m_cryptation->IsReady())
				{
					std::vector<uint8_t> buf(succesed_size);

					_this->m_cryptation->Decrypt(buf.data(), reinterpret_cast<const uint8_t*>(_this->m_buffer.data().data()), succesed_size);
					_this->m_buffer.consume(succesed_size);

					size = (buf[1] << 8) | buf[0];
				}
				else
				{
					asio::buffer_copy(asio::buffer(static_cast<void*>(&size), 2), _this->m_buffer.data());  // todo: clean solution
				}

				NET_LOG(LL_SYS, "Size of packet is %u", size);

				auto sizeToRead = size - 1 - 2;  // already read header (1byte) and size (2bytes)
				if (packet->HasSequence())
				{
					sizeToRead += 1;
				}

				NET_LOG(LL_SYS, "Reading the next %u bytes", sizeToRead);
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
			NET_LOG(LL_SYS, "(DATA) Received %u bytes", succesed_size);
			_this->m_buffer.commit(succesed_size);

			if (!er)
			{
				if (_this->m_cryptation && _this->m_cryptation->IsReady())
				{
					std::vector<uint8_t> buf(succesed_size);

					_this->m_cryptation->Decrypt(buf.data(), reinterpret_cast<const uint8_t*>(_this->m_buffer.data().data()), succesed_size);
					_this->m_buffer.consume(succesed_size);

					packet->CopyData(buf, packet->IsDynamicSized() ? 2 : 0);
				}
				else
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
		std::shared_ptr <NetPeerBase> _this(self.lock());
		if (_this)
		{
			if (!er)
			{
				NET_LOG(LL_SYS, "Delayed disconnect timeout! Connection dropped");
				_this->Disconnect(er);
			}
		}
	}


	// TODO: Call & set right time
	uint32_t NetPeerBase::GetCoreTime() const
	{
		return m_core_time;
	}

	void NetPeerBase::SetCoreTime(uint32_t time)
	{
		m_core_time = time;
	}



	void NetPeerBase::SetPhaseGC(uint8_t phase)
	{
		NET_LOG(LL_SYS, "Set phase to 0X%X", phase);

		auto packet = PacketManager::Instance().CreatePacket(ReservedGCHeaders::HEADER_GC_PHASE, EPacketDirection::Outgoing);
		if (!packet)
		{
			NET_LOG(LL_CRI, "Phase packet could not created!");
			abort();
			return;
		}

		packet->SetField<uint8_t>("phase", phase);

		m_phase = phase;

		Send(packet);
	}


	void NetPeerBase::StartHandshakeGC()
	{
		m_handshaking = true;

		SetPhaseGC(PHASE_HANDSHAKE);
		SendHandshakeGC(m_core_time, 0);
	}

	void NetPeerBase::SendHandshakeGC(uint32_t time, uint32_t delta)
	{
		auto packet = PacketManager::Instance().CreatePacket(ReservedGCHeaders::HEADER_GC_HANDSHAKE, EPacketDirection::Outgoing);
		if (!packet)
		{
			NET_LOG(LL_CRI, "Handshake packet could not created!");
			abort();
			return;
		}
		packet->SetField<uint32_t>("handshake", m_handshake);
		packet->SetField<uint32_t>("time", time);
		packet->SetField<uint32_t>("delta", 0);

		Send(packet);
	}

	void NetPeerBase::HandleHandshakeGC(std::shared_ptr <Packet> packet)
	{
		auto handshake = packet->GetField<uint32_t>("handshake");
		auto time = packet->GetField<uint32_t>("time");
		auto delta = packet->GetField<uint32_t>("delta");

		asio::error_code er;

		if (handshake != m_handshake)
		{
			NET_LOG(LL_ERR, "Closing connection because of handshake mismatch");
			Disconnect(er);
			return;
		}

		auto currentTime = GetCoreTime();

		auto diff = currentTime - (time + delta);
		if (diff >= 0 && diff <= 50)
		{
			NET_LOG(LL_SYS, "Time sync done, handshake done");

			if (m_securityLevel >= SECURITY_LEVEL_KEY_AGREEMENT)
			{
				StartKeyAgreementGC();
			}
			else
			{
				m_handshaking = false;
				OnConnect();

				if (m_securityLevel == SECURITY_LEVEL_XTEA)
				{
					NET_LOG(LL_SYS, "(XTEA) Cryptation enabled");

					if (!m_cryptation->Finalize())
					{
						Disconnect(er);
					}

					m_cryptation->Activate();
				}
			}
			return;
		}

		auto newDelta = (currentTime - time) / 2;
		if (newDelta < 0)
		{
			NET_LOG(LL_ERR, "Failed to sync time");
			Disconnect(er);
			return;
		}

		// TODO: max retries?
		SendHandshakeGC(currentTime, newDelta);
	}

	void NetPeerBase::StartKeyAgreementGC()
	{
		NET_DEBUG_LOG(LL_SYS, "(KEYS) Agreement start!");

		asio::error_code er;

		const CryptoPP::SecByteBlock* staticKey;
		const CryptoPP::SecByteBlock* ephemeralKey;
		const uint32_t* agreedValue;

		if (!m_cryptation->Initialize())
		{
			NET_LOG(LL_ERR, "(KEYS) Cannot initialize!");
			Disconnect(er);
			return;
		}

		staticKey = (const CryptoPP::SecByteBlock*)(m_cryptation->GetData(KEY_AGREEMENT_DATA_PUBLIC_STATIC_SERVER_KEY));
		ephemeralKey = (const CryptoPP::SecByteBlock*)(m_cryptation->GetData(KEY_AGREEMENT_DATA_PUBLIC_EPHEMERAL_SERVER_KEY));

		agreedValue = (const uint32_t*)m_cryptation->GetData(KEY_AGREEMENT_DATA_AGREED_VALUE);
		if (*agreedValue < 1 || staticKey->size() < 1 || ephemeralKey->size() < 1 || (ephemeralKey->size() + staticKey->size() > 256))
		{
			NET_LOG(LL_ERR, "(KEYS) Invalid values!");
			Disconnect(er);
			return;
		}

		char keys[256];
		memset(keys, 0, sizeof(keys));
		memcpy(keys, staticKey->data(), staticKey->size());
		memcpy(keys + staticKey->size(), ephemeralKey->data(), ephemeralKey->size());

		auto packet = PacketManager::Instance().CreatePacket(ReservedGCHeaders::HEADER_GC_KEY_AGREEMENT, EPacketDirection::Outgoing);
		if (!packet)
		{
			NET_LOG(LL_CRI, "Key agreement packet could not created!");
			abort();
			return;
		}
		packet->SetField<uint16_t>("valuelen", static_cast<uint16_t>(*agreedValue));
		packet->SetField<uint16_t>("datalen", static_cast<uint16_t>(ephemeralKey->size() + staticKey->size()));
		packet->SetField("data", (uint8_t*)keys, sizeof(keys));

		Send(packet);
	}

	void NetPeerBase::HandleKeyAgreementGC(std::shared_ptr<Packet> packet)
	{
		asio::error_code er;

		if (!m_cryptation->IsInitialized())
		{
			NET_LOG(LL_ERR, "(KEYS) Connection sent handle keys without letting the server initialize them");
			Disconnect(er);
			return;
		}

		uint8_t* data = (uint8_t*)packet->GetField("data");

		{
			const CryptoPP::SecByteBlock* staticKey = (const CryptoPP::SecByteBlock*)m_cryptation->GetData(KEY_AGREEMENT_DATA_PUBLIC_STATIC_SERVER_KEY);
			const CryptoPP::SecByteBlock* ephemeralKey = (const CryptoPP::SecByteBlock*)m_cryptation->GetData(KEY_AGREEMENT_DATA_PUBLIC_EPHEMERAL_SERVER_KEY);

			uint16_t agreedValue = packet->GetField<uint16_t>("valuelen");
			m_cryptation->AddData(KEY_AGREEMENT_DATA_AGREED_VALUE, &agreedValue, sizeof(agreedValue));

			m_cryptation->AddData(KEY_AGREEMENT_DATA_PUBLIC_STATIC_CLIENT_KEY, data, staticKey->size());
			m_cryptation->AddData(KEY_AGREEMENT_DATA_PUBLIC_EPHEMERAL_CLIENT_KEY, data + staticKey->size(), ephemeralKey->size());
		}

		if (!m_cryptation->Finalize())
		{
			NET_LOG(LL_ERR, "(KEYS) Cannot agree");
			Disconnect(er);
			return;
		}

		auto packet2 = PacketManager::Instance().CreatePacket(ReservedGCHeaders::HEADER_GC_KEY_AGREEMENT_COMPLETED, EPacketDirection::Outgoing);
		if (!packet2)
		{
			NET_LOG(LL_CRI, "Key agreement completed packet could not created!");
			abort();
			return;
		}
		Send(packet2);

		NET_LOG(LL_SYS, "(KEYS) Agreement completed!");
		m_handshaking = false;
		
		OnConnect();
	}

	void NetPeerBase::ChangeXTEAKeyGC(uint32_t* key)
	{
		m_cryptation->AddData(XTEA_CRYPTATION_LOGIN_DECRYPT_KEY, key, 16);
	}
}
