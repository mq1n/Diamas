#pragma once
#include "../include/NetEngine.h"
#include "NetPeerManager.h"
#include "nameof.hpp"

namespace net_engine
{	
	class CNetworkServerManager : public NetServerBase
	{
		using MapPacketHandlers = std::unordered_map <
			uint8_t, /* packet_header */
			std::pair <std::size_t, /* packet_length */
					std::function <std::size_t /* sent_length */(std::shared_ptr <CNetworkPeer> /* peer */, const void* /* data */, std::size_t /* length */)>> 
		>;
	public:
		CNetworkServerManager(NetServiceBase& netService, const std::string& ip_address, uint16_t port);
		virtual ~CNetworkServerManager();	

		NetServiceBase & GetServiceInstance();

		void ServerWorker();
		bool Initialize();
		void Stop();
		void OnUpdate();
	
		virtual void Init();
		virtual void Run();
		virtual void Shutdown();
		virtual bool IsShuttingDown();
	
		virtual std::shared_ptr <CNetworkPeer> FindPeer(int32_t id) const;
		virtual void RemovePeer(int32_t id);
	
		virtual std::size_t ProcessInput(std::shared_ptr <CNetworkPeer> peer, const void* data, std::size_t maxlength);
	
		template <typename PacketT, typename HandlerT>
		inline void RegisterPacket(uint8_t type, HandlerT handler) 
		{ 
			AppendPacket(type, NAMEOF(type).data());
			m_handlers.insert(std::make_pair(type, std::make_pair(sizeof(PacketT), handler)));
		}
		inline void RemovePacket(uint8_t type)
		{
			auto iter = m_handlers.find(type);
			if (iter != m_handlers.end())
				m_handlers.erase(iter);
		}

		void BroadcastMessage(const std::string& msg);
	
	protected:
		virtual std::shared_ptr <NetPeerBase> NewPeer();
	
		virtual void SetupPacketDispatchers();
	
		std::size_t OnRecvLoginPacket(std::shared_ptr <CNetworkPeer> peer, const void * data, std::size_t maxlength);
		std::size_t OnRecvChatPacket(std::shared_ptr <CNetworkPeer> peer, const void * data, std::size_t maxlength);

	private:
		std::string m_ip_address;
		uint16_t m_port;

		std::atomic <bool> m_stopped;

		NetServiceBase&		m_netService;
		MapPacketHandlers	m_handlers;

		asio::high_resolution_timer m_updateTimer;

		std::unordered_map <int32_t, std::shared_ptr <CNetworkPeer> >	m_umPeerMap;
	};
};
