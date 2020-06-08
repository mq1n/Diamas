#pragma once
#include "../include/NetEngine.h"
#include "nameof.hpp"

namespace net_engine
{	
	class CNetworkClientManager : public NetClientBase
	{
		using MapPacketHandlers = std::unordered_map <
			uint8_t, /* packet_header */
			std::pair <std::size_t, /* packet_length */
					   std::function <std::size_t /* sent_length */(const void* /* data */, std::size_t /* length */)>> 
		>;
	public:
		CNetworkClientManager(NetServiceBase& service);
		virtual ~CNetworkClientManager() {};

		NetServiceBase & GetServiceInstance() const;
	
//		virtual void Init();
		virtual void Run();
		virtual void Shutdown();
		virtual bool IsShuttingDown() const;

		virtual void			OnConnect();
		virtual void			OnDisconnect(const asio::error_code& e);
		virtual std::size_t		OnRead(const void* data, std::size_t length);
		virtual std::size_t		OnWritePre(const void* data, std::size_t length);
		virtual void			OnWritePost(bool bCompleted);
		virtual void			OnError(std::uint32_t ulErrorType, const asio::error_code & e);

		void SendCrypted(const SNetPacket& packet, bool flush = false);

		std::size_t ProcessInput(const void* data, std::size_t maxlength);
	

		std::size_t OnRecvChatPacket(const void* data, std::size_t maxlength);


		template <typename PacketT, typename HandlerT>
		inline void RegisterPacket(uint8_t type, HandlerT handler)
		{
			AppendPacket(type, NAMEOF(type).data());
			m_handlers.insert(std::make_pair(type, std::make_pair(sizeof(PacketT), handler)));
		}

		inline void RemovePacket(uint8_t type)
		{
			auto it = m_handlers.find(type);
			if (it != m_handlers.end())
				m_handlers.erase(it);
		}

	private:
		NetServiceBase & m_pNetService;
		MapPacketHandlers m_handlers;
	};
};
