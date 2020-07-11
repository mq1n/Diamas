#include "../include/NetEngine.hpp"

namespace net_engine
{
	NetServerBase::NetServerBase(asio::io_context& service, uint8_t securityLevel, const TPacketCryptKey& cryptKey) :
		m_service(service), m_acceptor(service)
	{
		static PacketManager packet_manager;
	}

	bool NetServerBase::Bind(const asio::ip::tcp::endpoint& endpoint)
	{
		m_acceptor.open(endpoint.protocol());
		m_acceptor.set_option(asio::ip::tcp::acceptor::reuse_address(true));
		m_acceptor.set_option(asio::ip::tcp::acceptor::linger(true, 10));
		m_acceptor.bind(endpoint);
		m_acceptor.listen();

		AsyncAccept();

		return true;
	}
	bool NetServerBase::Bind(uint16_t port)
	{
		return Bind(asio::ip::tcp::endpoint(asio::ip::tcp::v4(), port));
	}
	bool NetServerBase::Bind(const std::string& address, uint16_t port)
	{
		return Bind(asio::ip::tcp::endpoint(asio::ip::address::from_string(address), port));
	}

	void NetServerBase::AsyncAccept()
	{
		std::shared_ptr <NetPeerBase> peer = NewPeer();
		m_acceptor.async_accept(peer->GetSocket(), std::bind(&NetServerBase::HandleAccept, shared_from_this(), std::placeholders::_1, peer));
	}
	void NetServerBase::HandleAccept(std::weak_ptr <NetServerBase> self, const asio::error_code& er, std::shared_ptr <NetPeerBase> peer)
	{
		std::shared_ptr <NetServerBase> _this(self.lock());
		if (_this)
		{
			if (!er)
			{
				if (peer && peer.get())
				{
					peer->SetupPeer();
					// peer->OnConnect();
				}
			}
			else
			{
				NET_LOG(LL_CRI, "Fatal error: %u(%s)", er.value(), er.message().c_str());
			}

			_this->AsyncAccept();
		}
	}
}
