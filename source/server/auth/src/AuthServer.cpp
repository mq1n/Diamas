#include "stdafx.hpp"
#include "AuthServer.hpp"
#include "AuthPeer.hpp"
#include "AuthLogHelper.hpp"
#include "Packets.hpp"
#include <cxxopts.hpp>
using namespace net_engine;

// Cotr & dotr
GAuthServer::GAuthServer(NetServiceBase& netService, uint8_t securityLevel, const TPacketCryptKey& cryptKey) :
	NetServerBase(netService(), securityLevel, cryptKey), m_netService(netService), m_dbManager(netService()),
	m_server_key(1), m_config_file("config.json"), m_config_stage(STAGE_NULL), m_securityLevel(securityLevel), m_crypt_key(cryptKey)
{
	auth_log(LL_TRACE, "Creating server object");
	NetPacketManager::Instance().RegisterPackets(true);
}
GAuthServer::~GAuthServer()
{
	auth_log(LL_TRACE, "Destroying server object");

	if (!IsShuttingDown())
		Shutdown();
}

// Initilization funcs
bool GAuthServer::__ParseCommandLine(int argc, char** argv)
{
	cxxopts::Options options("AuthServer", "The auth core of game");

	options.add_options()
		("c,config", "Config file name", cxxopts::value<std::string>())
		("s,stage", "Config stage key", cxxopts::value<std::string>())
		("k,key", "Server key", cxxopts::value<uint8_t>())
	;

	auto result = options.parse(argc, argv);

	if (result.count("config"))
		m_config_file = result["config"].as<std::string>();

	if (result.count("stage"))
		m_config_stage = result["stage"].as<uint8_t>();

	if (result.count("key"))
		m_server_key = result["key"].as<uint8_t>();

	if (!std::filesystem::exists(m_config_file))
	{
		auth_log(LL_CRI, "Config file: %s does not exist!", m_config_file.c_str());
		return false;
	}

	return true;
}

bool GAuthServer::__ParseConfigFile()
{
	std::vector <std::string> pkVecDatabases = { "SQL_ACCOUNT"};

	msl::file_ptr config_file(m_config_file);
	if (!config_file)
	{
		auth_log(LL_CRI, "Loading config file failed.");
		return false;
	}

	auto stConfigBuffer = config_file.read();
	if (stConfigBuffer.empty())
	{
		auth_log(LL_CRI, "Empty config file content.");
		return false;
	}
//	auth_log(LL_SYS, "%s", stConfigBuffer.data());

	Document document;
	document.Parse<rapidjson::kParseStopWhenDoneFlag>(stConfigBuffer.data());

	if (document.HasParseError())
	{
		auth_log(LL_CRI, "config file parse failed. Error: %s offset: %u", GetParseError_En(document.GetParseError()), document.GetErrorOffset());
		return false;
	}

	if (!document.IsObject())
	{
		auth_log(LL_CRI, "config file base is NOT object");
		return false;
	}

	std::string stStage = "";
	if (m_config_stage == STAGE_NULL)
	{
		auto& pkStage = document["stage"];
		if (!pkStage.IsString())
			return false;

		stStage = pkStage.GetString();
//		auth_log(LL_SYS, "stage: %s", stStage);

		if (stStage == "dev")
			m_config_stage = STAGE_DEV_GAME;
		else if (stStage == "beta")
			m_config_stage = STAGE_BETA_GAME;
		else if (stStage == "live")
			m_config_stage = STAGE_LIVE_GAME;
		else
			return false;
	}

	auto& pkConfigContent = document[stStage.c_str()];
	if (pkConfigContent.IsNull() || !pkConfigContent.IsObject())
	{
		auth_log(LL_ERR, "pkConfigContent context is not valid.");
		return false;
	}

	auto& pkNumVersion = pkConfigContent["version"];
	if (pkNumVersion.IsNull() || !pkNumVersion.IsUint())
	{
		auth_log(LL_ERR, "version data could not found");
		return false;
	}
	m_cfgClientVersion = pkNumVersion.GetUint();
//	auth_log(LL_SYS, "Version: %u", m_cfgClientVersion);

	auto& pkObjDbcacheTree = pkConfigContent["dbcache"];
	if (pkObjDbcacheTree.IsNull() || !pkObjDbcacheTree.IsObject())
	{
		auth_log(LL_ERR, "dbcache object could not found");
		return false;
	}
	auto& pkStrDbcacheAddress = pkObjDbcacheTree["address"];
	if (pkStrDbcacheAddress.IsNull() || !pkStrDbcacheAddress.IsString())
	{
		auth_log(LL_ERR, "Dbcache address key could not found");
		return false;
	}
	auto& pkStrDbcachePort = pkObjDbcacheTree["bind_port"];
	if (pkStrDbcachePort.IsNull() || !pkStrDbcachePort.IsUint())
	{
		auth_log(LL_ERR, "Dbcache bind_port key could not found");
		return false;
	}
//	auth_log(LL_SYS, "dbcache %s:%u", pkStrDbcacheAddress.GetString(), pkStrDbcachePort.GetUint());
	m_cfgDBCache = asio::ip::tcp::endpoint(asio::ip::address::from_string(pkStrDbcacheAddress.GetString()), pkStrDbcachePort.GetUint());

	auto& pkObjAuthServersTree = pkConfigContent["auth_servers"];
	if (pkObjAuthServersTree.IsNull() || !pkObjAuthServersTree.IsObject())
	{
		auth_log(LL_ERR, "auth_servers object could not found");
		return false;
	}

	auto& pkObjAuthPortsTree = pkObjAuthServersTree["ports"];
	if (pkObjAuthPortsTree.IsNull() || !pkObjAuthPortsTree.IsArray())
	{
		auth_log(LL_ERR, "auth_servers::ports object could not found");
		return false;
	}

	size_t version = m_server_key - 1;
	size_t i = 0;
	uint16_t port = 0;

//	auth_log(LL_SYS, "Version:%u", version);
	for (auto const& item : pkObjAuthPortsTree.GetArray())
	{
//		auth_log(LL_SYS, "Current: %u", item.GetUint());
		if (version == i++)
		{
			port = item.GetUint();
//			auth_log(LL_SYS, "Target port: %u", port);
		}
	}

	m_serverCfg.hostname = fmt::format("AUTH_SERVER_{}", m_server_key);
	m_serverCfg.port = port;
	m_serverCfg.bindip = "0.0.0.0";

	auto& pkObjDatabaseTree = pkConfigContent["databases"];
	if (pkObjDatabaseTree.IsNull() || !pkObjDatabaseTree.IsObject())
	{
		auth_log(LL_ERR, "DATABASES not configured");
		return false;
	}

	for (auto x = 0U; x < pkVecDatabases.size(); ++x)
	{
		auto stDatabase = pkVecDatabases.at(x);

		auto& pkObjDatabase = pkObjDatabaseTree[stDatabase.c_str()];
		if (pkObjDatabase.IsNull() || !pkObjDatabase.IsObject())
		{
			auth_log(LL_ERR, "Database: %s not well configured.", stDatabase.c_str());
			return false;
		}
		auth_log(LL_SYS,  "Connecting to MySQL server (%s)", stDatabase.c_str());

		auto& pkStrHost = pkObjDatabase["address"];
		auto& pkNumPort = pkObjDatabase["port"];
		auto& pkStrDatabase = pkObjDatabase["database"];
		auto& pkStrUsername = pkObjDatabase["user"];
		auto& pkStrPassword = pkObjDatabase["password"];

		if (pkStrHost.IsNull() || pkNumPort.IsNull() || pkStrDatabase.IsNull() || pkStrUsername.IsNull() || pkStrPassword.IsNull())
		{
			auth_log(LL_ERR, "Database: %s context not well configured.", stDatabase.c_str());
			return false;
		}

		m_accountCfg.address	= pkStrHost.GetString();
		m_accountCfg.port		= pkNumPort.GetUint();
		m_accountCfg.database	= pkStrDatabase.GetString();
		m_accountCfg.user		= pkStrUsername.GetString();
		m_accountCfg.password	= pkStrPassword.GetString();
	}

	return true;
}

bool GAuthServer::__CreateDBConnection()
{
	auth_log(LL_SYS, "Connecting to account sql %s@%s:%u", m_accountCfg.user.c_str(), m_accountCfg.address.c_str(), m_accountCfg.port);

	m_dbManager.async_connect(
		asio::ip::tcp::endpoint(asio::ip::address::from_string(m_accountCfg.address), m_accountCfg.port),
		amy::auth_info(m_accountCfg.user, m_accountCfg.password),
		m_accountCfg.database,
		amy::default_flags,
		std::bind(&GAuthServer::OnMySQLConnect, std::dynamic_pointer_cast<GAuthServer>(shared_from_this()), std::placeholders::_1)
	);

	return true;
}

// Initilization manager
void GAuthServer::Init(int argc, char** argv) 
{
	auth_log(LL_SYS, "Auth server initializing...");

	if (!__ParseCommandLine(argc, argv))
	{
		auth_log(LL_ERR, "ParseCommandLine has been failed!");
		abort();
	}

	if (!__ParseConfigFile())
	{
		auth_log(LL_ERR, "ParseConfigFile has been failed!");
		abort();
	}

	if (!__CreateDBConnection())
	{
		auth_log(LL_ERR, "CreateDBConnection has been failed!");
		abort();
	}
	auth_log(LL_SYS, "Auth server initialized");

	auth_log(LL_SYS, "Network engine starting...");
	Run();

	auth_log(LL_SYS, "Network engine shutting down...");
	Shutdown();
}

// I/O Service wrappers
void GAuthServer::Run()
{
	m_netService.Run();
}
void GAuthServer::Shutdown()
{
	m_netService.Stop();
}
bool GAuthServer::IsShuttingDown() const
{
	return m_netService.HasStopped();
}

// Peer stuffs
std::shared_ptr <GAuthPeer> GAuthServer::FindPeer(uint32_t id) const
{
	auto iter = m_peers.find(id);
	if (iter != m_peers.end())
		return iter->second;

	return nullptr;
}
std::shared_ptr <NetPeerBase> GAuthServer::NewPeer()
{
	auto peer = std::make_shared<GAuthPeer>(std::static_pointer_cast<GAuthServer>(shared_from_this()), m_securityLevel, m_crypt_key);
	m_peers.emplace(peer->GetId(), peer);

	return peer;
}
void GAuthServer::RemovePeer(uint32_t id) 
{
	auto iter = m_peers.find(id);
	if (iter != m_peers.end()) 
		m_peers.erase(iter);
}

// Packet wrappers
void GAuthServer::BroadcastPacket(std::shared_ptr <Packet> packet)
{
	for (const auto& [id, peer] : m_peers)
	{
		if (peer && peer.get())
		{
			peer->Send(packet);
		}
	}
}
void GAuthServer::SendTo(std::shared_ptr <Packet> packet, std::function<bool(GAuthPeer*)> filter)
{
	for (const auto& [id, peer] : m_peers)
	{
		if (peer && peer.get())
		{
			if (filter && filter(peer.get()))
			{
				peer->Send(packet);
			}
		}
	}
}
void GAuthServer::SendToPhase(std::shared_ptr <Packet> packet, uint8_t phase)
{
	SendTo(std::move(packet), [phase](GAuthPeer* connection) {
		return connection->GetPhase() == phase;
	});
}

void GAuthServer::OnMySQLConnect(std::weak_ptr<GAuthServer> self, const asio::error_code &e) 
{
	std::shared_ptr <GAuthServer> _this(self.lock());
	if (_this) 
	{
		if (!e) 
		{
			bool reconnect = true;

			if (0 != mysql_options(_this->m_dbManager.native(), MYSQL_OPT_RECONNECT, &reconnect))
				auth_log(LL_SYS, "mysql_option: %s\n", mysql_error(_this->m_dbManager.native()));

			mysql_set_character_set(_this->m_dbManager.native(), "latin1");

			auth_log(LL_SYS, "Connecting to DBCache %s:%u", _this->m_cfgDBCache.address().to_string().c_str(), _this->m_cfgDBCache.port());

			_this->m_dbClient = std::make_shared<GDBClient>(_this);
			_this->m_dbClient->SetMainPort(_this->m_serverCfg.port);
			_this->m_dbClient->Connect(_this->m_cfgDBCache.address().to_string(), _this->m_cfgDBCache.port());

			auth_log(LL_SYS, "Binding auth server on %s:%u", _this->m_serverCfg.bindip.c_str(), _this->m_serverCfg.port);
			_this->Bind(_this->m_serverCfg.bindip, _this->m_serverCfg.port);
		} 
		else 
		{
			auth_log(LL_CRI, "Cannot connect to account sql (%s)", e.message().c_str());
		}
	}
}
