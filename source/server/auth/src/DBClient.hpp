#pragma once

class GAuthServer;

class GDBClient : public net_engine::NetClientBase
{
private:
	typedef std::unordered_map<uint8_t, std::pair<std::size_t, std::function<std::size_t(const void*, std::size_t)> > > MapPacketHandlers;
	
public:
	GDBClient(std::shared_ptr <GAuthServer> server);
	
	template<typename PacketT, typename HandlerT> inline void RegisterPacket(uint8_t type, HandlerT handler) { m_handlers.insert(std::make_pair(type, std::make_pair(PacketT::size(), handler))); }
	inline void RemovePacket(uint8_t type)
	{
		auto iter = m_handlers.find(type);
		if(iter != m_handlers.end())
		{
			m_handlers.erase(iter);
		}
	}
	
	void SetMainPort(uint16_t port) { m_mainPort = port; };
	uint16_t GetMainPort() const { return m_mainPort; };
		
	virtual void OnConnect();
	virtual void OnDisconnect(const asio::error_code& er);
	virtual void OnRead(std::shared_ptr <net_engine::Packet> packet);
	virtual void OnError(uint32_t ulErrorType, const asio::error_code& er);

	virtual void SendSetup(uint16_t mainPort);
	virtual void SendAuthLogin(
		uint32_t peerId, int64_t id, const std::string& login, const std::string& socialId, const std::string& lang,
		const std::string& hwid, uint32_t loginKey, const std::vector <int32_t>& aiPremiumTimes, const uint8_t* clientKey
	);
	
	virtual std::size_t RecvAuthLogin(const void* data, std::size_t maxlength);
	
private:
	std::weak_ptr <GAuthServer> m_server;
	MapPacketHandlers m_handlers;
	uint16_t m_mainPort;
};
