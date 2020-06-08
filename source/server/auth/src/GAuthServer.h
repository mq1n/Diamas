#pragma once
#include "GAccountManager.h"
#include "GDBClient.h"

class GAuthPeer;

class GAuthServer : public net_engine::NetServerBase
{
private:
	typedef std::unordered_map<uint8_t, std::pair<std::size_t, std::function<std::size_t(std::shared_ptr<GAuthPeer>, const void*, std::size_t)> > > MapPacketHandlers;
	struct DBConfig
	{
		std::string address;
		std::string user;
		std::string password;
		std::string database;
		uint16_t port = {0};
	};
	
public:
	GAuthServer(net_engine::NetServiceBase& netService);
	virtual ~GAuthServer();
	
	net_engine::NetServiceBase& GetService() const;
	GAccountManager& GetAccountManager();
	
	virtual void Init(int argc, char** argv);
	virtual void Run();
	virtual void Shutdown();
	virtual bool IsShuttingDown() const;
	
	virtual std::shared_ptr<GAuthPeer> FindPeer(int32_t id) const;
	virtual void RemovePeer(int32_t id);
	
	virtual std::size_t ProcessInput(std::shared_ptr<GAuthPeer> peer, const void* data, std::size_t maxlength);
	
	template<typename PacketT, typename HandlerT> inline void RegisterPacket(uint8_t type, HandlerT handler) { m_handlers.insert(std::make_pair(type, std::make_pair(PacketT::size(), handler))); }
	inline void RemovePacket(uint8_t type)
	{
		auto iter = m_handlers.find(type);
		if(iter != m_handlers.end())
		{
			m_handlers.erase(iter);
		}
	}
	
protected:
	virtual std::shared_ptr<net_engine::NetPeerBase> NewPeer();
	
	virtual void __parseCmdLine(int argc, char** argv);
	virtual bool __parseConfig();
	virtual void __registerHandlers();
	
	static void OnMySQLConnect(std::weak_ptr<GAuthServer> self, const asio::error_code& e);
	static void OnLoginSQLQuery(std::weak_ptr<GAuthServer> self, const asio::error_code& e, std::shared_ptr<GAuthPeer> peer, uint32_t loginKey);
	static void OnLoginSQLResult(std::weak_ptr<GAuthServer> self, const asio::error_code& e, amy::result_set rs, std::shared_ptr<GAuthPeer> peer, uint32_t loginKey);

	std::size_t RecvLogin(std::shared_ptr<GAuthPeer> peer, const void * data, std::size_t maxlength);
	std::size_t RecvHack(std::shared_ptr<GAuthPeer> peer, const void * data, std::size_t maxlength);
private:
	net_engine::NetServiceBase& m_netService;
	GAccountManager m_accountManager;
	std::shared_ptr<GDBClient> m_dbClient;
	amy::mariadb_connector m_dbManager;
	std::unordered_map<int32_t, std::shared_ptr<GAuthPeer> > m_peers;
	MapPacketHandlers m_handlers;
	
	// config
	std::string m_configXml;
	std::string m_configKey;
	uint32_t m_cfgClientVersion = {0};
	asio::ip::tcp::endpoint m_cfgDBCache;
	struct
	{
		std::string hostname;
		uint16_t port = {0};
		std::string bindip;
	} m_serverCfg;
	DBConfig m_accountCfg;
	DBConfig m_playerCfg;
	DBConfig m_commonCfg;
	DBConfig m_logCfg;
};