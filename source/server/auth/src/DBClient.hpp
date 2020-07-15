#pragma once

class GAuthServer;

class GDBClient : public net_engine::NetClientBase
{
public:
	GDBClient(std::shared_ptr <GAuthServer> server);
	
	void SetMainPort(uint16_t port) { m_mainPort = port; };
	uint16_t GetMainPort() const { return m_mainPort; };
		
	virtual void OnConnect();
	virtual void OnDisconnect(const asio::error_code& er);
	virtual void OnRead(std::shared_ptr <net_engine::Packet> packet);
	virtual void OnError(uint32_t ulErrorType, const asio::error_code& er);

	virtual void SendSetup(uint16_t mainPort);
	virtual void SendAuthLogin(
		uint32_t peerId, uint32_t id, const std::string& login, const std::string& socialId, const std::string& lang,
		const std::string& hwid, uint32_t loginKey, const std::vector <int32_t>& aiPremiumTimes, const uint8_t* clientKey
	);
	
	virtual void RecvAuthLogin(std::shared_ptr <net_engine::Packet> packet);
	
private:
	std::weak_ptr <GAuthServer>		m_server;
	net_engine::NetPacketDispatcher m_dispatcher;
	uint16_t						m_mainPort;
};
