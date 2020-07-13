#pragma once
#include "../include/NetEngine.hpp"
#include "nameof.hpp"
#include "ClientCipher.hpp"

namespace net_engine
{	
	class CNetworkClientManager : public NetClientBase
	{
		using THandlerFunc 		 = std::function <void(std::shared_ptr <Packet> packet)>;
		using TPacketHandler 	 = std::unordered_map <TNetOpcode, THandlerFunc>;
		using TOnPacketRegister	 = std::function <void(std::shared_ptr <PacketDefinition>)>;

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
	
		inline bool RegisterPacket(const std::string& name, TNetOpcode header, THandlerFunc handler, bool incoming, bool outgoing, TOnPacketRegister on_register)
		{ 
			std::shared_ptr <PacketDefinition> def;
			if ((def = PacketManager::Instance().RegisterPacket(name, header, incoming, outgoing)))
			{
				if (handler)
					m_handlers.emplace(header, handler);
				if (on_register)
					on_register(def);
				return true;
			}
			return false;
		}
		inline bool DeregisterPacket(TNetOpcode header, bool incoming, bool outgoing)
		{
			auto iter = m_handlers.find(header);
			if (iter != m_handlers.end())
			{
				m_handlers.erase(iter);
				return PacketManager::Instance().DeregisterPacket(header, incoming, outgoing);
			}
			return false;
		}
		
	protected:
		void OnRecvKeyAgreementCompletedPacket(std::shared_ptr <Packet> packet);
		void OnRecvPhasePacket(std::shared_ptr <Packet> packet);
		void OnRecvKeyAgreementPacket(std::shared_ptr <Packet> packet);
		void OnRecvHandshakePacket(std::shared_ptr <Packet> packet);

	private:
		NetServiceBase & m_pNetService;
		TPacketHandler m_handlers;

		struct SServerTimeSync
		{
			uint32_t m_dwChangeServerTime;
			uint32_t m_dwChangeClientTime;
		} m_kServerTimeSync;

		Cipher	m_cipher;
	};
};
