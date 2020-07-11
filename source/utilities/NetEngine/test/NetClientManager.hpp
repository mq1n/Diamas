#pragma once
#include "../include/NetEngine.hpp"
#include "nameof.hpp"

namespace net_engine
{	
	class CNetworkClientManager : public NetClientBase
	{
//		using TPacketHandler = std::unordered_map <TNetOpcode, THandlerFunc>;
		
	public:
		CNetworkClientManager(NetServiceBase& service, uint8_t securityLevel, const TPacketCryptKey& cryptKey);
		virtual ~CNetworkClientManager() {};

		NetServiceBase & GetServiceInstance() const;
	
//		virtual void Init();
		virtual void Run();
		virtual void Shutdown();
		virtual bool IsShuttingDown() const;

		virtual void		OnConnect();
		virtual void		OnDisconnect(const asio::error_code& e);
		virtual void		OnRead(std::shared_ptr <Packet> packet);
		virtual std::size_t	OnWritePre(const void* data, std::size_t length);
		virtual void		OnWritePost(bool bCompleted);
		virtual void		OnError(std::uint32_t ulErrorType, const asio::error_code & e);

//		void SendCrypted(const SNetPacket& packet, bool flush = false);

		std::size_t ProcessInput(const void* data, std::size_t maxlength);
	

		std::size_t OnRecvChatPacket(const void* data, std::size_t maxlength);

		/*
		inline void RegisterPacket(TNetOpcode header, uint8_t type, bool is_dynamic, THandlerFunc handler)
		{
			CPacketContainer::Instance().AppendPacket(header, NAMEOF(header).data(), type, handler, is_dynamic);
//			m_handlers.emplace(header, handler);
		}

		inline void DeregisterPacket(uint8_t header)
		{
			CPacketContainer::Instance().RemovePacket(header, NAMEOF(header).data());
//			auto iter = m_handlers.find(header);
//			if (iter != m_handlers.end())
//				m_handlers.erase(iter);
		}
		*/

	private:
		NetServiceBase & m_pNetService;
//		TPacketHandler m_handlers;
	};
};
