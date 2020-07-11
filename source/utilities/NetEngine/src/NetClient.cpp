#include "../include/NetEngine.hpp"

namespace net_engine
{
	NetClientBase::NetClientBase(asio::io_context& service, uint8_t securityLevel, const TPacketCryptKey& cryptKey, uint32_t autoReconnectCycle) :
		NetPeerBase(service, securityLevel, cryptKey), m_autoReconnectCycle(autoReconnectCycle), m_reconnectTimer(service), m_isReconnectOnce(false)
	{
		// ctor
		static PacketManager packet_manager;
	}

	bool NetClientBase::Connect(const std::string& host, uint16_t port)
	{
		return Connect(host, std::to_string(port));
	}
	bool NetClientBase::Connect(const std::string& host, const std::string& service)
	{
		asio::ip::tcp::resolver resolver(GetSocket().get_executor());
		asio::ip::tcp::resolver::query q(host, service);
		m_endpointStart = resolver.resolve(q);
		return Connect(m_endpointStart);
	}
	bool NetClientBase::Reconnect()
	{
		if (!m_isReconnectOnce)
		{
			m_isReconnectOnce = true;

			m_reconnectTimer.expires_from_now(std::chrono::milliseconds(m_autoReconnectCycle));
			m_reconnectTimer.async_wait(
				std::bind(&NetClientBase::TryReconnect, std::static_pointer_cast<NetClientBase>(shared_from_this()), std::placeholders::_1)
			);

			return true;
		}
		return false;
	}

	void NetClientBase::SetAutoReconnectCycle(uint32_t autoReconnectCycle)
	{
		m_autoReconnectCycle = autoReconnectCycle;
	}
	uint32_t NetClientBase::GetAutoReconnectCycle() const
	{
		return m_autoReconnectCycle;
	}

	void NetClientBase::TryReconnect(const asio::error_code& er)
	{
		if (!er)
		{
			Connect(m_endpointStart);
		}
	}
	bool NetClientBase::Connect(asio::ip::tcp::resolver::iterator endpoint_iterator)
	{
		asio::async_connect(
			GetSocket(), m_endpointStart,
			std::bind(&NetClientBase::HandleConnect, std::static_pointer_cast<NetClientBase>(shared_from_this()), std::placeholders::_1, std::placeholders::_2)
		);
		return true;
	}
	void NetClientBase::HandleConnect(std::weak_ptr<NetClientBase> self, const asio::error_code& er, asio::ip::tcp::resolver::iterator endpoint_iterator)
	{
		std::shared_ptr <NetClientBase> _this(self.lock());
		if (_this)
		{
			if (!er)
			{
				_this->m_isReconnectOnce = false;
				_this->SetupPeer();
				// _this->OnConnect();
			}
			else if (endpoint_iterator != asio::ip::tcp::resolver::iterator())
			{
				// Before starting new connection close used socket in the previous connection attempt
				_this->GetSocket().close();

				_this->Connect(endpoint_iterator);
			}
			else
			{
				_this->OnError(PeerErrorConnectionFail, er);
			}

			if (_this->m_isReconnectOnce)
			{
				_this->m_isReconnectOnce = false;
				_this->Reconnect();
			}
		}
	}
}
