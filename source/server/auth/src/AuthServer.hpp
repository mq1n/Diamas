#pragma once
#include "AccountManager.hpp"
#include "AuthPeer.hpp"
#include "DBClient.hpp"
#include <array>

struct DBConfig
{
	std::string address;
	std::string user;
	std::string password;
	std::string database;
	uint16_t port{ 0 };
};

class GAuthServer : public net_engine::NetServerBase
{
public:
	GAuthServer(net_engine::NetServiceBase& netService, uint8_t securityLevel, const net_engine::TPacketCryptKey& cryptKey);
	virtual ~GAuthServer();
	
	auto& GetService() 			{ return m_netService; };
	auto& GetAccountManager()	{ return m_accountManager; };
	auto& GetDBClient()			{ return m_dbClient; };
	auto& GetDBManager()		{ return m_dbManager; };

	virtual void Init(int argc, char** argv);
	virtual void Run();
	virtual void Shutdown();
	virtual bool IsShuttingDown() const;
	
	virtual std::shared_ptr <GAuthPeer> FindPeer(uint32_t id) const;
	virtual void RemovePeer(uint32_t id);

	virtual void BroadcastPacket(std::shared_ptr <net_engine::Packet> packet);
	virtual void SendTo(std::shared_ptr <net_engine::Packet> packet, std::function<bool(GAuthPeer*)> filter);
	virtual void SendToPhase(std::shared_ptr <net_engine::Packet> packet, uint8_t phase);

	virtual uint8_t GetStage() const { return m_config_stage; };

protected:
	virtual std::shared_ptr <net_engine::NetPeerBase> NewPeer();
	
	virtual bool __ParseCommandLine(int argc, char** argv);
	virtual bool __ParseConfigFile();
	virtual bool __CreateDBConnection();
	
	static void OnMySQLConnect(std::weak_ptr<GAuthServer> self, const asio::error_code& e);
	static void OnLoginSQLQuery(std::weak_ptr<GAuthServer> self, const asio::error_code& e, std::shared_ptr<GAuthPeer> peer, uint32_t loginKey);
	static void OnLoginSQLResult(std::weak_ptr<GAuthServer> self, const asio::error_code& e, amy::result_set rs, std::shared_ptr<GAuthPeer> peer, uint32_t loginKey);

private:
	net_engine::NetServiceBase&									m_netService;
	GAccountManager												m_accountManager;
	std::shared_ptr <GDBClient>									m_dbClient;
	amy::connector												m_dbManager;
	std::unordered_map <uint32_t, std::shared_ptr <GAuthPeer> > m_peers;
	
	uint8_t m_server_key;
	std::string m_config_file;
	uint8_t m_config_stage;
	uint32_t m_cfgClientVersion{0};
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

	uint8_t m_securityLevel;
	net_engine::TPacketCryptKey m_crypt_key;
};
