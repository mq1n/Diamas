#pragma once
#include "NetPeer.h"

namespace net_engine
{
	class NetServerBase : asio::noncopyable, public std::enable_shared_from_this <NetServerBase>
	{
		public:
			NetServerBase(asio::io_context& service);
			virtual ~NetServerBase() {};

			bool Bind(uint16_t port);
			bool Bind(const std::string& address, uint16_t port);
			bool Bind(const asio::ip::tcp::endpoint& endpoint);

		protected:
			asio::io_context& m_service;
			asio::ip::tcp::acceptor m_acceptor;

		private:
			void AsyncAccept();
			static void HandleAccept(std::weak_ptr <NetServerBase> self, const asio::error_code& er, std::shared_ptr <NetPeerBase> peer);

			virtual std::shared_ptr <NetPeerBase> NewPeer() = 0;
	};
}
