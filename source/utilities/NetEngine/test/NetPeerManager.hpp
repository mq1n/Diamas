#pragma once
#include "../include/NetEngine.hpp"
#include <unordered_map>
#include <functional>
#include <variant>
#include <memory>

namespace net_engine
{	
	using ConnectionProperty = std::variant <std::string, uint32_t, uint8_t>;

	class CNetworkServerManager;

	class CNetworkConnectionManager : public NetPeerBase, public IUniqueID <CNetworkConnectionManager, uint32_t>
	{
		public:
			CNetworkConnectionManager(
				std::shared_ptr <CNetworkServerManager> server,
				uint8_t securityLevel, const TPacketCryptKey& cryptKey
			);
			virtual ~CNetworkConnectionManager();

			// Close peer connection
			virtual void Destroy();

			// Low level engine callbacks
			virtual void OnConnect();
			virtual void OnDisconnect(const asio::error_code& er);
			virtual void OnRead(std::shared_ptr <Packet> packet);
			virtual void OnError(uint32_t ulErrorType, const asio::error_code& er);

			void CheckDeadlineStatus();

			// Getter
			std::shared_ptr <CNetworkServerManager> GetServer() const;
			int32_t GetId() const;

			// Property
			bool HasProperty(const std::string& property) const;
			void SetProperty(const std::string& property, ConnectionProperty value);
			ConnectionProperty GetProperty(const std::string& property) const;
			template <typename T>
			T GetProperty(const std::string& property) const {
				return std::get<T>(GetProperty(property));
			}

			// IO
			void SendAsReply(std::shared_ptr <Packet> request, std::shared_ptr <Packet> reply);

		protected:
			void OnRecvKeyAgreementPacket(std::shared_ptr <Packet> packet);
			void OnRecvHandshakePacket(std::shared_ptr <Packet> packet);

		private:
			std::weak_ptr <CNetworkServerManager>		m_server;
			NetPacketDispatcher 						m_dispatcher;
			std::map <std::string, ConnectionProperty>	m_properties;
			asio::high_resolution_timer 				m_deadline_timer;
	};
}
