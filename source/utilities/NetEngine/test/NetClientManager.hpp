#pragma once
#include "../include/NetEngine.hpp"
#include "ClientCipher.hpp"

namespace net_engine
{	
	class CNetworkClientManager : public NetClientBase
	{
	public:
		CNetworkClientManager(NetServiceBase& service, uint8_t securityLevel, const TPacketCryptKey& cryptKey);
		virtual ~CNetworkClientManager() {};

		NetServiceBase & GetServiceInstance() const;
	
		virtual void Run();
		virtual void Shutdown();
		virtual bool IsShuttingDown() const;

		virtual void OnConnect();
		virtual void OnDisconnect(const asio::error_code& e);
		virtual void OnRead(std::shared_ptr <Packet> packet);
		virtual void OnError(std::uint32_t ulErrorType, const asio::error_code & e);
		
	protected:
		void OnRecvKeyAgreementCompletedPacket(std::shared_ptr <Packet> packet);
		void OnRecvPhasePacket(std::shared_ptr <Packet> packet);
		void OnRecvKeyAgreementPacket(std::shared_ptr <Packet> packet);
		void OnRecvHandshakePacket(std::shared_ptr <Packet> packet);

	private:
		NetServiceBase & m_pNetService;
		NetPacketDispatcher m_dispatcher;

		struct SServerTimeSync
		{
			uint32_t m_dwChangeServerTime;
			uint32_t m_dwChangeClientTime;
		} m_kServerTimeSync;

		Cipher	m_cipher;
	};
};
