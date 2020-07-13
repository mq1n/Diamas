#include "stdafx.hpp"
#include "AuthServer.hpp"
#include "AuthPeer.hpp"
#include "Packets.hpp"
using namespace net_engine;

void HandleGenericUpdateResult(asio::error_code const &ec, uint64_t affected_rows, amy::connector&connector, std::shared_ptr<GAuthPeer> peer, const std::string & what)
{
	if (affected_rows > 0 && peer)
		sys_log(LL_SYS, "Updated %s of account %s", what.c_str(), peer->GetLogin().c_str());
	return;
}

static void writePid(const char *filename) 
{
	std::ofstream out(filename, std::ofstream::out);
	if (out)
	{
#if defined(_WIN32)
		out << GetCurrentProcessId();
#else
		out << getpid();
#endif
		out.close();
	}
}



GAuthServer::GAuthServer(NetServiceBase& netService, uint8_t securityLevel, const TPacketCryptKey& cryptKey) :
	NetServerBase(netService(), securityLevel, cryptKey), m_netService(netService), m_dbManager(netService()),
	m_configXml("config.json"), m_configKey("1"), m_securityLevel(securityLevel), m_crypt_key(cryptKey)
{
}
GAuthServer::~GAuthServer()
{
}


void GAuthServer::Init(int argc, char** argv) 
{
	__parseCmdLine(argc, argv);

	if (!__parseConfig())
	{
		sys_log(LL_CRI, "Config parse fail!");
		exit(EXIT_FAILURE);
	}

	__registerHandlers();

	sys_log(LL_SYS, "Connecting to account sql %s@%s:%u", m_accountCfg.user.c_str(), m_accountCfg.address.c_str(), m_accountCfg.port);

	std::shared_ptr<GAuthServer> self(std::dynamic_pointer_cast<GAuthServer>(shared_from_this()));

	m_dbManager.async_connect(
		asio::ip::tcp::endpoint(asio::ip::address::from_string(m_accountCfg.address), m_accountCfg.port),
		amy::auth_info(m_accountCfg.user, m_accountCfg.password),
		m_accountCfg.database,
		amy::default_flags,
		std::bind(&GAuthServer::OnMySQLConnect, self, std::placeholders::_1)
	);
}



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


std::shared_ptr <GAuthPeer> GAuthServer::FindPeer(uint32_t id) const
{
	auto iter = m_peers.find(id);
	if (iter != m_peers.end())
	{
		return iter->second;
	}
	return nullptr;
}

void GAuthServer::RemovePeer(uint32_t id) 
{
	auto iter = m_peers.find(id);
	if (iter != m_peers.end()) 
	{
		m_peers.erase(iter);
	}
}

std::shared_ptr <NetPeerBase> GAuthServer::NewPeer() 
{
	auto peer = std::make_shared<GAuthPeer>(std::static_pointer_cast<GAuthServer>(shared_from_this()), m_securityLevel, m_crypt_key);
	m_peers.emplace(peer->GetId(), peer);
	return peer;
}


void GAuthServer::__parseCmdLine(int argc, char **argv) 
{
	for (int i = 1; i < argc; ++i)
	{
		if (!strncmp(argv[i], "--pid=", 6))
		{
			writePid(&argv[i][6]);
		}
		else if (!strcmp(argv[i], "-p"))
		{
			if (++i < argc)
			{
				writePid(argv[i]);
			}
			else
			{
				// printHelp();
			}
		}
		else if (!strncmp(argv[i], "--config=", 9))
		{
			m_configXml = &argv[i][9];
		}
		else if (!strcmp(argv[i], "-c"))
		{
			if (++i < argc)
			{
				m_configXml = argv[i];
			}
			else
			{
				// printHelp();
			}
		}
		else if (!strncmp(argv[i], "--config-key=", 13))
		{
			m_configKey = &argv[i][13];
		}
		else if (!strcmp(argv[i], "-k"))
		{
			if (++i < argc)
			{
				m_configKey = argv[i];
			}
			else
			{
				// printHelp();
			}
		}
		else if (!strcmp(argv[i], "--help") || !strcmp(argv[i], "-h"))
		{
			// printHelp();
		}
	}
}

bool GAuthServer::__parseConfig()
{
	/*
	using namespace boost::property_tree;
	ptree root;

	read_json("config.json", root);
	std::string currentStage = root.get<std::string>("stage", "dev");

	auto stageTree = root.get_child(currentStage);

	m_cfgClientVersion = stageTree.get<uint32_t>("version", 1);

	auto dbCache = stageTree.find("dbcache");
	if (dbCache == stageTree.not_found())
		return false;

	m_cfgDBCache = asio::ip::tcp::endpoint(asio::ip::address::from_string(dbCache->second.get<std::string>("address")), dbCache->second.get<uint16_t>("bind_port"));

	if (!stageTree.count("auth_servers"))
	{
		sys_log(LL_ERR, "cannot find auth_servers tree");
		return false;
	}

	size_t version = std::stoul(m_configKey.c_str()) - 1;
	size_t i = 0;
	uint16_t port = 0;

	auto authTree = stageTree.get_child("auth_servers");
	if (!authTree.count("ports"))
	{
		sys_log(LL_ERR, "cannot find ports in tree. (%d)", version);
		return false;
	}

	for (auto const& item : authTree.get_child("ports"))
	{
		if (version == i++)
			port = item.second.get_value<uint16_t>();
	}

	m_serverCfg.hostname = str(boost::format("AUTH_SERVER_%s") % m_configKey.c_str());
	m_serverCfg.port = port;
	m_serverCfg.bindip = "0.0.0.0";

	if (!stageTree.count("mysql"))
	{
		sys_log(LL_ERR, "cannot find mysql in tree.");
		return false;
	}

	ptree databasesTree = stageTree.get_child("mysql.databases");

	for (const auto& kv : databasesTree)
	{
		sys_log(0, "connecting to MySQL server (%s)", kv.second.get<std::string>("ident").c_str());

		// if database shouldnt be loaded in auth server just skip it
		if (kv.second.get<std::string>("ident") == "SQL_ACCOUNT")
		{
			m_accountCfg.address = kv.second.get<std::string>("address");
			m_accountCfg.user = kv.second.get<std::string>("user");
			m_accountCfg.password = kv.second.get<std::string>("password");
			m_accountCfg.database = kv.second.get<std::string>("database");
			m_accountCfg.port = 3306;
			if (kv.second.find("port") != kv.second.not_found())
				m_accountCfg.port = kv.second.get<uint16_t>("port");

			break;
		}
	}
	*/
	return true;
}

void GAuthServer::__registerHandlers()
{
//	RegisterPacket<TPacketCGLogin3>(HEADER_CG_LOGIN3, std::bind(&GAuthServer::RecvLogin, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
//	RegisterPacket<TPacketCGHack>(HEADER_CG_HACK, std::bind(&GAuthServer::RecvHack, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
}



void GAuthServer::OnMySQLConnect(std::weak_ptr<GAuthServer> self, const asio::error_code &e) 
{
	std::shared_ptr<GAuthServer> _this(self.lock());
	if (_this) 
	{
		if (!e) 
		{
			bool reconnect = true;

			if (0 != mysql_options(_this->m_dbManager.native(), MYSQL_OPT_RECONNECT, &reconnect))
				sys_log(LL_SYS, "mysql_option: %s\n", mysql_error(_this->m_dbManager.native()));

			mysql_set_character_set(_this->m_dbManager.native(), "latin1");

			sys_log(LL_SYS, "Connecting to DBCache %s:%u", _this->m_cfgDBCache.address().to_string().c_str(), _this->m_cfgDBCache.port());

			_this->m_dbClient = std::make_shared<GDBClient>(_this);
			_this->m_dbClient->SetMainPort(_this->m_serverCfg.port);
			_this->m_dbClient->Connect(_this->m_cfgDBCache.address().to_string(), _this->m_cfgDBCache.port());
			sys_log(LL_SYS, "Binding auth server on %s:%u", _this->m_serverCfg.bindip.c_str(), _this->m_serverCfg.port);
			_this->Bind(_this->m_serverCfg.bindip, _this->m_serverCfg.port);
		} 
		else 
		{
			sys_log(LL_CRI, "Cannot connect to account sql (%s)", e.message().c_str());
		}
	}
}

void GAuthServer::OnLoginSQLQuery(std::weak_ptr<GAuthServer> self, const asio::error_code &e, std::shared_ptr <GAuthPeer> peer, uint32_t loginKey) 
{
	std::shared_ptr<GAuthServer> _this(self.lock());
	if (_this) 
	{
		if (!e) 
		{
			_this->m_dbManager.async_store_result(std::bind(&GAuthServer::OnLoginSQLResult, self, std::placeholders::_1, std::placeholders::_2, peer, loginKey));

		}
		else
		{
			sys_log(LL_ERR, e.message().c_str());
			peer->LoginFailure("BESAMEKEY");
			peer->DelayedDisconnect(5000);
		}
	}
}

void GAuthServer::OnLoginSQLResult(std::weak_ptr<GAuthServer> self, const asio::error_code &e, amy::result_set rs, std::shared_ptr<GAuthPeer> peer, uint32_t loginKey)
{
	std::shared_ptr <GAuthServer> _this(self.lock());
	if (_this)
	{
		if (!e) 
		{
			if (rs.empty()) 
			{
				sys_log(LL_ERR, "Client %s (%d): Account not found %s", peer->GetIP().c_str(), peer->GetId(), peer->GetLogin().c_str());
				peer->LoginFailure("NOID");
				peer->DelayedDisconnect(5000);
				return;
			}

			uint32_t col = 0;
			for (const auto &result : rs)
			{
				// id
				// pw1
				// pw2
				// social_id
				// status
				// lang
				// availDt - NOW() > 0
				// UNIX_TIMESTAMP(silver_expire)
				// UNIX_TIMESTAMP(gold_expire)
				// UNIX_TIMESTAMP(safebox_expire)
				// UNIX_TIMESTAMP(autoloot_expire)
				// UNIX_TIMESTAMP(fish_mind_expire)
				// UNIX_TIMESTAMP(marriage_fast_expire)
				// UNIX_TIMESTAMP(money_drop_rate_expire)
				// UNIX_TIMESTAMP(shop_double_up_expire)
				// UNIX_TIMESTAMP(create_time)
				const amy::field &fieldID = result[col++];
				const amy::field &fieldPasswd1 = result[col++];
				const amy::field &fieldPasswd2 = result[col++];
				const amy::field &fieldSocialID = result[col++];
				const amy::field &fieldStatus = result[col++];
				const amy::field &fieldLang = result[col++];
				const amy::field &fieldAvail = result[col++];
				const amy::field &fieldSilverExpire = result[col++];
				const amy::field &fieldGoldExpire = result[col++];
				const amy::field &fieldSafeboxExpire = result[col++];
				const amy::field &fieldAutoLootExpire = result[col++];
				const amy::field &fieldFishMindExpire = result[col++];
				const amy::field &fieldMarriageFastExpire = result[col++];
				const amy::field &fieldMoneyDropRateExpire = result[col++];
				const amy::field &fieldShopDoubleUpExpire = result[col++];

				if (fieldID.is_null() || fieldPasswd1.is_null() || fieldPasswd2.is_null() || fieldStatus.is_null()) 
				{
					sys_log(LL_ERR, "Invalid MySQL result set for client %s (%d)", peer->GetIP().c_str(), peer->GetId());
					peer->LoginFailure("BESAMEKEY");
					peer->DelayedDisconnect(5000);
					break;
				}

				if (fieldPasswd1.as<amy::sql_text>() != fieldPasswd2.as<amy::sql_text>()) 
				{
					sys_log(LL_ERR, "Client %s (%d) sent the wrong password for %s", peer->GetIP().c_str(), peer->GetId(), peer->GetLogin().c_str());
					//peer->LoginFailure("WRONGPWD");

					//using NOID here because otherwise people could bruteforce accounts with the authserver.
					peer->LoginFailure("NOID");
					peer->DelayedDisconnect(5000);
					break;
				}

				if (_this->m_accountManager.FindAccount(peer->GetLogin())) 
				{
					sys_log(LL_SYS, "Client %s (%d) logging into an already connected account %s", peer->GetIP().c_str(), peer->GetId(), peer->GetLogin().c_str());
					peer->LoginFailure("ALREADY");
					peer->DelayedDisconnect(5000);
					break;
				}

				//if users status isn't OK or if the user has a time ban.
				if (fieldStatus.as<amy::sql_text>() != "OK") 
				{
					sys_log(LL_SYS, "Client %s (%d) logging into an unavailable account %s (status=%s)", peer->GetIP().c_str(), peer->GetId(), peer->GetLogin().c_str(), fieldStatus.as<amy::sql_text>().c_str());
					peer->LoginFailure(fieldStatus.as<amy::sql_text>());
					peer->DelayedDisconnect(5000);
					break;
				}

				if (!fieldAvail.is_null() && fieldAvail.as<amy::sql_int>() > 0)
				{
					sys_log(LL_SYS, "Client %s (%d) logging into an unavailable account %s (status=%s)", peer->GetIP().c_str(), peer->GetId(), peer->GetLogin().c_str(), "AVAILDT");
					peer->LoginFailure("BLOCK");
					peer->DelayedDisconnect(5000);
					break;
				}

				std::vector<int32_t> aiPremiumTimes(9);

				aiPremiumTimes[0] = fieldSilverExpire.is_null() ? 0 : fieldSilverExpire.as<amy::sql_int>();
				aiPremiumTimes[1] = fieldGoldExpire.is_null() ? 0 : fieldGoldExpire.as<amy::sql_int>();
				aiPremiumTimes[2] = fieldSafeboxExpire.is_null() ? 0 : fieldSafeboxExpire.as<amy::sql_int>();
				aiPremiumTimes[3] = fieldAutoLootExpire.is_null() ? 0 : fieldAutoLootExpire.as<amy::sql_int>();
				aiPremiumTimes[4] = fieldFishMindExpire.is_null() ? 0 : fieldFishMindExpire.as<amy::sql_int>();
				aiPremiumTimes[5] = fieldMarriageFastExpire.is_null() ? 0 : fieldMarriageFastExpire.as<amy::sql_int>();
				aiPremiumTimes[6] = fieldMoneyDropRateExpire.is_null() ? 0 : fieldMoneyDropRateExpire.as<amy::sql_int>();
				aiPremiumTimes[7] = fieldShopDoubleUpExpire.is_null() ? 0 : fieldShopDoubleUpExpire.as<amy::sql_int>();

				auto accId = fieldID.as<amy::sql_bigint>();
				auto socialId = fieldSocialID.as<amy::sql_text>();
				auto lang = fieldLang.as<amy::sql_text>();

				amy::async_execute(_this->m_dbManager,
						fmt::format("UPDATE account SET last_play = NOW() WHERE login = '{0}'", peer->GetLogin()),
						std::bind(HandleGenericUpdateResult, std::placeholders::_1, std::placeholders::_2, std::ref(_this->m_dbManager), peer, std::string("Last Play")
				));

				_this->m_accountManager.ConnectAccount(peer->GetLogin(), loginKey);

				_this->m_dbClient->SendAuthLogin(peer->GetId(), accId, peer->GetLogin(), socialId, lang,
						peer->GetHwid(),
						loginKey, aiPremiumTimes, peer->GetClientKey()
				);

				break;
			}
		} 
		else 
		{
			sys_log(LL_ERR, e.message().c_str());
			peer->LoginFailure("BESAMEKEY");
			peer->DelayedDisconnect(5000);
		}
	}
}



std::size_t GAuthServer::RecvLogin(std::shared_ptr<GAuthPeer> peer, const void* data, std::size_t maxlength)
{
#if 0
	if (maxlength < TPacketCGLogin3::size())
		return 0;

	if (!m_dbClient || !m_dbClient->IsConnected()) 
	{
		peer->LoginFailure("BESAMEKEY");
		peer->DelayedDisconnect(5000);
		return TPacketCGLogin3::size();
	}

	auto packet = reinterpret_cast<const SPacketCGLogin3 *>(data);

	std::string login(packet->login, std::min<std::size_t>(strlen(packet->login), sizeof(packet->login)));
	std::string password(packet->password, std::min<std::size_t>(strlen(packet->password), sizeof(packet->password)));

	transform(login.begin(), login.end(), login.begin(), tolower);
	trim(login);

	sys_log(LL_SYS, "Client %s (%d) trying to login %s", peer->GetIP().c_str(), peer->GetId(), login.c_str());
	if (!IsAlphaNumeric(login))
	{
		sys_log(LL_ERR, "Client %s (%d) sent invalid login name %s", peer->GetIP().c_str(), peer->GetId(), login.c_str());
		peer->LoginFailure("NOID");
		return packet->size();
	}

	if (IsShuttingDown())
	{
		peer->LoginFailure("SHUTDOWN");
		return packet->size();
	}

	// Client Version check
	auto date = std::stoul(packet->timestamp);
	if (date == m_cfgClientVersion)
	{
		sys_log(LL_SYS, "Client %s (%d) connected with an outdated client remote(%u) != server(%u)", peer->GetIP().c_str(), peer->GetId(), date, m_cfgClientVersion);
		peer->LoginFailure("VERSION");
		return packet->size();
	}

	if (m_accountManager.FindAccount(login))
	{
		sys_log(LL_SYS, "Client %s (%d) logging into an already connected account %s", peer->GetIP().c_str(), peer->GetId(), login.c_str());
		peer->LoginFailure("ALREADY");
		return packet->size();
	}

	peer->SetLogin(login);
	peer->SetPassword(packet->password);

	sys_log(LL_SYS, "Client %s (%d): Trying to login %s", peer->GetIP().c_str(), peer->GetId(), peer->GetLogin().c_str());

	uint32_t loginKey = m_accountManager.CreateAuthKey();
	peer->SetLoginKey(loginKey);
	peer->SetClientKey(packet->clientKey);

	char szLogin[sizeof(packet->login) * 2 + 1];
	mysql_real_escape_string(m_dbManager.native(), szLogin, login.c_str(), login.size());

	char szPasswd[sizeof(packet->password) * 2 + 1];
	mysql_real_escape_string(m_dbManager.native(), szPasswd, peer->GetPassword().c_str(), peer->GetPassword().length());



	peer->SetHwid(packet->hwid);

	char szHWID[sizeof(packet->hwid) * 2 + 1];
	mysql_real_escape_string(m_dbManager.native(), szHWID, peer->GetHwid().c_str(), peer->GetHwid().size());

	std::string statement = fmt::format("SELECT COUNT(hwid) as count FROM hwid_block WHERE hwid = '{0}' AND bannedUntil > NOW()", szHWID);
	try 
	{
		m_dbManager.query(statement);
	}
	catch (std::exception &e) 
	{
		sys_log(LL_ERR, e.what());

		peer->LoginSuccess(false);
		return packet->size();
	}

	amy::result_set result_set = m_dbManager.store_result();
	for (auto &row  : result_set)
	{
		if (row[0].as<amy::sql_tinyint_unsigned>() > 0) 
		{
			sys_log(LL_SYS, "Client %s (%d) is hwid bannned", peer->GetIP().c_str(), peer->GetId());
			peer->LoginFailure("HWIDB");
			return packet->size();
		}
	}

	amy::async_execute(m_dbManager,
			fmt::format("UPDATE account SET hwid = '{0}' WHERE login = '{0}'", szHWID, szLogin),
			std::bind(HandleGenericUpdateResult, std::placeholders::_1, std::placeholders::_2, std::ref(m_dbManager), peer, std::string("HWID")));



	std::string lang = std::string(packet->lang, std::min<std::size_t>(strlen(packet->lang), sizeof(packet->lang)));

	char szLang[sizeof(packet->lang) * 2 + 1];
	mysql_real_escape_string(m_dbManager.native(), szLang, lang.c_str(), lang.size());

	amy::async_execute(m_dbManager,
			fmt::format("UPDATE account SET lang = '{0}' WHERE login = '{1}'", szLang, szLogin),
			std::bind(HandleGenericUpdateResult, std::placeholders::_1, std::placeholders::_2, std::ref(m_dbManager), peer, std::string("Language")));

	std::string stQuery = fmt::format(
			"SELECT id, PASSWORD('{0}'), password, social_id,"
			" status,"
			" lang,"
			" availDt - NOW() > 0,"
			" UNIX_TIMESTAMP(silver_expire),"
			" UNIX_TIMESTAMP(gold_expire),"
			" UNIX_TIMESTAMP(safebox_expire),"
			" UNIX_TIMESTAMP(autoloot_expire),"
			" UNIX_TIMESTAMP(fish_mind_expire),"
			" UNIX_TIMESTAMP(marriage_fast_expire),"
			" UNIX_TIMESTAMP(money_drop_rate_expire),"
			" UNIX_TIMESTAMP(shop_double_up_expire),"
			" UNIX_TIMESTAMP(create_time)"
			" FROM account WHERE login = '{1}'",
			szPasswd , szLogin);

	amy::async_execute(m_dbManager, stQuery, std::bind(&GAuthServer::OnLoginSQLQuery, std::static_pointer_cast<GAuthServer>(shared_from_this()), std::placeholders::_1, peer, loginKey));

//	return TPacketCGLogin3::size();
#endif
	return 1;
}

std::size_t GAuthServer::RecvHack(std::shared_ptr<GAuthPeer> peer, const void *data, std::size_t maxlength)
{
	/*
	if (maxlength < TPacketCGHack::size())
		return 0;

	auto packet = reinterpret_cast<const TPacketCGHack *>(data);
	sys_log(LL_SYS, "Hack detected by Client %s (%d): %s", peer->GetIP().c_str(), peer->GetId(), packet->message);

	return TPacketCGHack::size();
	*/
	return 1;
}
