#pragma once
#include "../include/NetEngine.h"
#include <unordered_map>

namespace net_engine
{	
	enum EPeerPhaseList
	{
		PHASE_NULL,
		PHASE_CONN_REQUEST,
		PHASE_ENCRYPTED_COMM_READY,
		PHASE_CHECK_COMPLETED,
		PHASE_MAX
	};

	class CNetworkServerManager;

	class CNetworkPeer : public NetPeerBase, public IUniqueID <CNetworkPeer, int32_t>
	{
		public:
			CNetworkPeer(std::shared_ptr <CNetworkServerManager> server);
			virtual ~CNetworkPeer() {};

			// setter getter
			std::shared_ptr <CNetworkServerManager> GetServer() const;
			int32_t GetId() const;

			void SetData(const std::string& key, const std::string& value);
			std::string GetData(const std::string& key) const;

			// net callbacks
			virtual void OnConnect();
			virtual void OnDisconnect(const asio::error_code& er);
			virtual std::size_t OnRead(const void* data, std::size_t length);
			virtual std::size_t OnWritePre(const void* data, std::size_t length);
			virtual void OnWritePost(bool bCompleted);
			virtual void OnError(uint32_t ulErrorType, const asio::error_code& er);
			
			// packet crypter & sender
			virtual void SendCrypted(const SNetPacket& packet, bool flush = false);

			// finalization routine for a peer
			virtual void Destroy();
			
			// packet funcs
			virtual void SendChatPacket(const std::string& data);
			
		private:
			std::weak_ptr <CNetworkServerManager>					m_pServer;
			std::unordered_map <std::string, std::string> 	m_pkMapDataContainer;
	};
}
