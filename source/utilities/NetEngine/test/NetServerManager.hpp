#pragma once
#include "../include/NetEngine.hpp"
#include "NetPeerManager.hpp"
#include "nameof.hpp"

namespace net_engine
{	
	class CNetworkServerManager : public NetServerBase
	{
//		using THandlerFunc = std::function <std::size_t /* sent_length */(std::shared_ptr <CNetworkConnectionManager> /* peer */, const void* /* data */, std::size_t /* length */)>;
//		using TPacketHandler = std::unordered_map <TNetOpcode, THandlerFunc>;

	public:
		CNetworkServerManager(NetServiceBase& netService, const std::string& ip_address, uint16_t port);
		virtual ~CNetworkServerManager();	

		NetServiceBase & GetServiceInstance();

		void ServerWorker();
		bool Initialize();
		void Stop();
		void OnUpdate();
	
		virtual void Run();
		virtual void Shutdown();
		virtual bool IsShuttingDown();
	
		virtual std::shared_ptr <CNetworkConnectionManager> FindPeer(int32_t id) const;
		virtual void RemovePeer(int32_t id);
		
		inline void RegisterPacket(TNetOpcode header, uint8_t type, bool is_dynamic, THandlerFunc handler)
		{ 
			CPacketContainer::Instance().AppendPacket(header, NAMEOF(header).data(), type, handler, is_dynamic);
//			m_handlers.emplace(header, handler);
		}
		inline void DeregisterPacket(TNetOpcode header)
		{
			CPacketContainer::Instance().RemovePacket(header, NAMEOF(header).data());
//			auto iter = m_handlers.find(header);
//			if (iter != m_handlers.end())
//				m_handlers.erase(iter);
		}

		void BroadcastMessage(const std::string& msg);
	
	protected:
		virtual std::shared_ptr <NetPeerBase> NewPeer();
		
		std::size_t OnRecvLoginPacket(const void * data, std::size_t maxlength);
		std::size_t OnRecvChatPacket(const void * data, std::size_t maxlength);

	private:
		std::shared_ptr <CNetworkConnectionManager> m_net_manager;

		std::string m_ip_address;
		uint16_t m_port;

		std::atomic <bool> m_stopped;

		NetServiceBase&		m_netService;
//		TPacketHandler		m_handlers;

		asio::high_resolution_timer m_updateTimer;

		std::unordered_map <int32_t, std::shared_ptr <CNetworkConnectionManager> >	m_peers;
	};
};
