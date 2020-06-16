#pragma once
#include "NetPeer.hpp"

namespace net_engine
{
	class NetClientBase : public NetPeerBase
	{
		public:
			NetClientBase(asio::io_context& service, uint32_t autoReconnectCycle = 0);
			virtual ~NetClientBase() {};

			bool Connect(const std::string& host, uint16_t port);
			bool Connect(const std::string& host, const std::string& service);

			bool Reconnect();

			void SetAutoReconnectCycle(uint32_t autoReconnectCycle);
			uint32_t GetAutoReconnectCycle() const;

		protected:
			asio::ip::tcp::resolver::iterator m_endpointStart;
			uint32_t m_autoReconnectCycle;
			asio::high_resolution_timer m_reconnectTimer;
			bool m_isReconnectOnce;

		private:
			void TryReconnect(const asio::error_code& er);
			bool Connect(asio::ip::tcp::resolver::iterator endpoint_iterator);
			static void HandleConnect(std::weak_ptr <NetClientBase> self, const asio::error_code& er, asio::ip::tcp::resolver::iterator endpoint_iterator);
	};
}