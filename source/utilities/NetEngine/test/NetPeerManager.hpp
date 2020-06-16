#pragma once
#include "../include/NetEngine.hpp"
#include <unordered_map>

namespace net_engine
{	
	class CNetworkServerManager;

	class CNetworkConnectionManager : public NetPeerBase, public IUniqueID <CNetworkConnectionManager, uint32_t>
	{
		public:
			CNetworkConnectionManager(
				NetServiceBase& netService, std::shared_ptr <CNetworkServerManager> server,
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
			std::string GetData(const std::string& key) const;

			// Setter
			void SetData(const std::string& key, const std::string& value);

#if 0
			std::size_t ProcessInput(const void* data, std::size_t maxlength);
			
			// packet crypter & sender
			virtual void SendCrypted(const SNetPacket& packet, bool flush = false);

			// packet funcs
			virtual void SendChatPacket(const std::string& data);
#endif

		private:
			NetServiceBase& m_netService;

			std::weak_ptr   <CNetworkServerManager>		m_server;
			std::shared_ptr <CPacketContainer>			m_packets;

			TPacketCryptKey m_crypt_key;

			std::unordered_map <std::string, std::string> 	m_data_container;
	};
}
