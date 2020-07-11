#pragma once
#include "../include/NetEngine.hpp"
#include <unordered_map>
#include <variant>

namespace net_engine
{	
	using ConnectionProperty = std::variant<std::string, uint32_t, uint8_t>;

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
			virtual std::size_t OnWritePre(const void* data, std::size_t length);
			virtual void OnWritePost(bool bCompleted);
			virtual void OnError(uint32_t ulErrorType, const asio::error_code& er);

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
			void SendAsReply(std::shared_ptr<Packet> request, std::shared_ptr<Packet> reply);
#if 0
			std::size_t ProcessInput(const void* data, std::size_t maxlength);
			
			// packet crypter & sender
			virtual void SendCrypted(const SNetPacket& packet, bool flush = false);

			// packet funcs
			virtual void SendChatPacket(const std::string& data);
#endif

		private:
			std::weak_ptr <CNetworkServerManager>		m_server;
			std::map<std::string, ConnectionProperty>	m_properties;
	};
}
