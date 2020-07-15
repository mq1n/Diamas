#include "stdafx.hpp"
#include "AuthPeer.hpp"
#include "AuthServer.hpp"
#include "AuthLogHelper.hpp"
#include "Packets.hpp"
using namespace net_engine;

void HandleGenericUpdateResult(asio::error_code const& ec, uint64_t affected_rows, amy::connector& connector, GAuthPeer* peer, const std::string& what)
{
	if (affected_rows > 0 && peer)
		auth_log(LL_SYS, "Updated %s of account %s", what.c_str(), peer->GetLogin().c_str());
	return;
}

GAuthPeer::GAuthPeer(std::shared_ptr <GAuthServer> server, uint8_t securityLevel, const TPacketCryptKey& cryptKey) :
	NetPeerBase(server->GetService()(), securityLevel, cryptKey, true, server->GetStage()), m_server(server), m_loginKey(0),
	m_deadline_timer(server->GetService()())
{
	auth_log(LL_TRACE, "Creating connection object");

	m_id = UniqueId();

	REGISTER_PACKET_HANDLER(HEADER_CG_HACK, std::bind(&GAuthPeer::OnRecvHackPacket, this, std::placeholders::_1));
	REGISTER_PACKET_HANDLER(HEADER_CG_LOGIN3, std::bind(&GAuthPeer::OnRecvLogin3Packet, this, std::placeholders::_1));
	REGISTER_PACKET_HANDLER(HEADER_CG_KEY_AGREEMENT, std::bind(&GAuthPeer::OnRecvKeyAgreementPacket, this, std::placeholders::_1));
	REGISTER_PACKET_HANDLER(HEADER_CG_HANDSHAKE, std::bind(&GAuthPeer::OnRecvHandshakePacket, this, std::placeholders::_1));

	// ---------------------
	m_deadline_timer.expires_from_now(std::chrono::seconds(30));
	m_deadline_timer.async_wait(std::bind(&GAuthPeer::CheckDeadlineStatus, this));
}
GAuthPeer::~GAuthPeer()
{
	auth_log(LL_TRACE, "Destroying connection object");
	Destroy();
}

void GAuthPeer::CheckDeadlineStatus()
{
	auto phase = GetPhase();
	switch (phase)
	{
	case PHASE_HANDSHAKE:
	case PHASE_LOGIN:
	case PHASE_AUTH:
		// TODO: drop connection
		break;

	default:
		break;
	}
}

void GAuthPeer::Destroy()
{
	auto server = GetServer();
	if (server)
		server->RemovePeer(GetId());
}

// net callbacks
void GAuthPeer::OnConnect()
{
	auth_log(LL_SYS, "Client connected %s %u", GetIP().c_str(), GetId());
	SendPhase(PHASE_AUTH); // auth phase
}
void GAuthPeer::OnDisconnect(const asio::error_code& er)
{
	auth_log(LL_SYS, "Client disconnected %s (%u) Reason: %d (%s)", GetIP().c_str(), GetId(), er.value(), er.message().c_str());

	auto server = GetServer();
	if (server)
	{
		if (!m_name.empty())
			server->GetAccountManager().DisconnectAccount(m_name);
		server->RemovePeer(GetId());
	}
}
void GAuthPeer::OnRead(std::shared_ptr <Packet> packet)
{
	const auto header = packet->GetHeader();
	auth_log(LL_SYS, "Peer: %s(%d) Packet: %u(0x%x) is ready for process!", GetIP().c_str(), GetId(), header, header);

	const auto handler = m_dispatcher.GetPacketHandler(header);
	if (!handler)
	{
		auth_log(LL_ERR, "Unknown Packet with id %d (%02x) received from PEER %d", header, header, GetId());
		Disconnect2();
		return;
	}
	handler(packet);
}
void GAuthPeer::OnError(uint32_t error_type, const asio::error_code& er)
{
	auth_log(LL_ERR, "Network error handled! ID: %u System error: %d(%s)", error_type, er.value(), er.message().c_str());
}

void GAuthPeer::SendPhase(uint8_t phaseId)
{
	SetPhaseGC(phaseId);
	auth_log(LL_SYS, "Sent phase packet to %s (%d)", GetIP().c_str(), GetId());
}
void GAuthPeer::SendLoginFailure(const std::string& status)
{
	auto packet = NetPacketManager::Instance().CreatePacket(CreateOutgoingPacketID(HEADER_GC_LOGIN_FAILURE));
	packet->SetString("szStatus", status.c_str());
	Send(packet);
}
void GAuthPeer::SendLoginSuccess(uint8_t result)
{
	auto packet = NetPacketManager::Instance().CreatePacket(CreateOutgoingPacketID(HEADER_GC_AUTH_SUCCESS));
	packet->SetField<uint32_t>("dwLoginKey", GetLoginKey());
	packet->SetField<uint8_t>("bResult", result);
	Send(packet);
}


bool GAuthPeer::HasProperty(const std::string& property) const
{
	return m_properties.find(property) != m_properties.end();
}
void GAuthPeer::SetProperty(const std::string& property, ConnectionProperty value)
{
	m_properties[property] = std::move(value);
}
GAuthPeer::ConnectionProperty GAuthPeer::GetProperty(const std::string& property) const
{
	return m_properties.at(property);
}


void GAuthPeer::SendAsReply(std::shared_ptr<Packet> request, std::shared_ptr<Packet> reply)
{
	auto id = request->GetField<uint64_t>("__REFERENCE_ID");

	reply->SetField<uint64_t>("__REFERENCE_ID", id);
	reply->SetField<uint8_t>("__REFERENCE_TYPE", 1);

	Send(reply);
}


void GAuthPeer::OnRecvKeyAgreementPacket(std::shared_ptr <Packet> packet)
{
	HandleKeyAgreementGC(packet);
}

void GAuthPeer::OnRecvHandshakePacket(std::shared_ptr <Packet> packet)
{
	HandleHandshakeGC(packet);
}

void GAuthPeer::OnRecvHackPacket(std::shared_ptr <Packet> packet)
{
	auto szBuf = packet->GetField("szBuf");
	auto szInfo = packet->GetField("szInfo");

	auth_log(LL_SYS, "Hack detected by Client %s (%d): %s - %s", GetIP().c_str(), GetId(), szBuf, szInfo);
}

void GAuthPeer::OnRecvLogin3Packet(std::shared_ptr <Packet> packet)
{
	auto server = GetServer();
	if (!server)
	{
		auth_log(LL_ERR, "Server could not found by Peer: %s (%d)", GetIP().c_str(), GetId());
		return;
	}

	const auto& db_client = server->GetDBClient();
	if (!db_client || !db_client->IsConnected())
	{
		auth_log(LL_ERR, "DB is not exist nor connected!");
		SendLoginFailure("BESAMEKEY");
		DelayedDisconnect(5000);
		return;
	}

	auto login = packet->GetString("name");
	auto password = packet->GetString("pwd");
	auto adwClientKey = packet->GetField("adwClientKey");
	auto version = packet->GetField<uint32_t>("version");
	auto hwid = packet->GetString("hwid");
	auto lang = packet->GetString("lang");

	transform(login.begin(), login.end(), login.begin(), tolower);
	trim(login);

	auth_log(LL_SYS, "Client %s (%d) trying to login %s", GetIP().c_str(), GetId(), login.c_str());
	if (login.size() < 4 || !IsAlphaNumeric(login))
	{
		auth_log(LL_ERR, "Client %s (%d) sent invalid login name %s", GetIP().c_str(), GetId(), login.c_str());
		SendLoginFailure("NOID");
		return;
	}

	if (server->IsShuttingDown())
	{
		SendLoginFailure("SHUTDOWN");
		return;
	}

	// Client Version check
	if (version != CLIENT_VERSION_TIMESTAMP)
	{
		auth_log(LL_SYS, "Client %s (%d) connected with an outdated client remote(%u) != server(%u)", GetIP().c_str(), GetId(), CLIENT_VERSION_TIMESTAMP, CLIENT_VERSION_TIMESTAMP);
		SendLoginFailure("VERSION");
		return;
	}

	if (server->GetAccountManager().FindAccount(login))
	{
		auth_log(LL_SYS, "Client %s (%d) logging into an already connected account %s", GetIP().c_str(), GetId(), login.c_str());
		SendLoginFailure("ALREADY");
		return;
	}

	SetLogin(login);
	SetPassword(password);

	auth_log(LL_SYS, "Client %s (%d): Trying to login %s", GetIP().c_str(), GetId(), GetLogin().c_str());

	uint32_t loginKey = server->GetAccountManager().CreateAuthKey();
	SetLoginKey(loginKey);
	SetClientKey(reinterpret_cast<const uint8_t*>(adwClientKey));

	char szLogin[LOGIN_MAX_LEN * 2 + 1];
	mysql_real_escape_string(server->GetDBManager().native(), szLogin, login.c_str(), login.size());

	char szPasswd[PASSWD_MAX_LEN * 2 + 1];
	mysql_real_escape_string(server->GetDBManager().native(), szPasswd, GetPassword().c_str(), GetPassword().length());



	SetHwid(hwid);

	char szHWID[HWID_MAX_HASH_LEN * 2 + 1];
	mysql_real_escape_string(server->GetDBManager().native(), szHWID, GetHwid().c_str(), GetHwid().size());

	std::string statement = fmt::format("SELECT COUNT(hwid) as count FROM hwid_block WHERE hwid = '{0}' AND bannedUntil > NOW()", szHWID);
	try
	{
		server->GetDBManager().query(statement);
	}
	catch (std::exception& e)
	{
		auth_log(LL_ERR, e.what());

		SendLoginSuccess(false);
		return;
	}

	amy::result_set result_set = server->GetDBManager().store_result();
	for (auto& row : result_set)
	{
		if (row[0].as<amy::sql_tinyint_unsigned>() > 0)
		{
			auth_log(LL_SYS, "Client %s (%d) is hwid bannned", GetIP().c_str(), GetId());
			SendLoginFailure("HWIDB");
			return;
		}
	}

	amy::async_execute(server->GetDBManager(),
		fmt::format("UPDATE account SET hwid = '{0}' WHERE login = '{0}'", szHWID, szLogin),
		std::bind(HandleGenericUpdateResult, std::placeholders::_1, std::placeholders::_2, std::ref(server->GetDBManager()), this, std::string("HWID")));


	char szLang[LANG_MAX_LEN * 2 + 1];
	mysql_real_escape_string(server->GetDBManager().native(), szLang, lang.c_str(), lang.size());

	amy::async_execute(server->GetDBManager(),
		fmt::format("UPDATE account SET lang = '{0}' WHERE login = '{1}'", szLang, szLogin),
		std::bind(HandleGenericUpdateResult, std::placeholders::_1, std::placeholders::_2, std::ref(server->GetDBManager()), this, std::string("Language")));

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
		szPasswd, szLogin);

	amy::async_execute(server->GetDBManager(),
		stQuery,
		std::bind(&GAuthPeer::OnLoginSQLQuery, std::static_pointer_cast<GAuthServer>(GetServer()), std::placeholders::_1, this, loginKey)
	);
}

void GAuthPeer::OnLoginSQLQuery(std::weak_ptr <GAuthServer> self, const asio::error_code& e, GAuthPeer* peer, uint32_t loginKey)
{
	std::shared_ptr <GAuthServer> _this(self.lock());
	if (_this)
	{
		if (!e)
		{
			_this->GetDBManager().async_store_result(std::bind(&GAuthPeer::OnLoginSQLResult, self, std::placeholders::_1, std::placeholders::_2, peer, loginKey));

		}
		else
		{
			auth_log(LL_ERR, e.message().c_str());
			peer->SendLoginFailure("BESAMEKEY");
			peer->DelayedDisconnect(5000);
		}
	}
}

void GAuthPeer::OnLoginSQLResult(std::weak_ptr <GAuthServer> self, const asio::error_code& e, amy::result_set rs, GAuthPeer* peer, uint32_t loginKey)
{
	std::shared_ptr <GAuthServer> _this(self.lock());
	if (_this)
	{
		if (!e)
		{
			if (rs.empty())
			{
				auth_log(LL_ERR, "Client %s (%d): Account not found %s", peer->GetIP().c_str(), peer->GetId(), peer->GetLogin().c_str());
				peer->SendLoginFailure("NOID");
				peer->DelayedDisconnect(5000);
				return;
			}

			uint32_t col = 0;
			for (const auto& result : rs)
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
				const amy::field& fieldID = result[col++];
				const amy::field& fieldPasswd1 = result[col++];
				const amy::field& fieldPasswd2 = result[col++];
				const amy::field& fieldSocialID = result[col++];
				const amy::field& fieldStatus = result[col++];
				const amy::field& fieldLang = result[col++];
				const amy::field& fieldAvail = result[col++];
				const amy::field& fieldSilverExpire = result[col++];
				const amy::field& fieldGoldExpire = result[col++];
				const amy::field& fieldSafeboxExpire = result[col++];
				const amy::field& fieldAutoLootExpire = result[col++];
				const amy::field& fieldFishMindExpire = result[col++];
				const amy::field& fieldMarriageFastExpire = result[col++];
				const amy::field& fieldMoneyDropRateExpire = result[col++];
				const amy::field& fieldShopDoubleUpExpire = result[col++];

				if (fieldID.is_null() || fieldPasswd1.is_null() || fieldPasswd2.is_null() || fieldStatus.is_null())
				{
					auth_log(LL_ERR, "Invalid MySQL result set for client %s (%d)", peer->GetIP().c_str(), peer->GetId());
					peer->SendLoginFailure("BESAMEKEY");
					peer->DelayedDisconnect(5000);
					break;
				}

				if (fieldPasswd1.as<amy::sql_text>() != fieldPasswd2.as<amy::sql_text>())
				{
					auth_log(LL_ERR, "Client %s (%d) sent the wrong password for %s", peer->GetIP().c_str(), peer->GetId(), peer->GetLogin().c_str());
					//peer->SendLoginFailure("WRONGPWD");

					//using NOID here because otherwise people could bruteforce accounts with the authserver.
					peer->SendLoginFailure("NOID");
					peer->DelayedDisconnect(5000);
					break;
				}

				if (_this->GetAccountManager().FindAccount(peer->GetLogin()))
				{
					auth_log(LL_SYS, "Client %s (%d) logging into an already connected account %s", peer->GetIP().c_str(), peer->GetId(), peer->GetLogin().c_str());
					peer->SendLoginFailure("ALREADY");
					peer->DelayedDisconnect(5000);
					break;
				}

				//if users status isn't OK or if the user has a time ban.
				if (fieldStatus.as<amy::sql_text>() != "OK")
				{
					auth_log(LL_SYS, "Client %s (%d) logging into an unavailable account %s (status=%s)", peer->GetIP().c_str(), peer->GetId(), peer->GetLogin().c_str(), fieldStatus.as<amy::sql_text>().c_str());
					peer->SendLoginFailure(fieldStatus.as<amy::sql_text>());
					peer->DelayedDisconnect(5000);
					break;
				}

				if (!fieldAvail.is_null() && fieldAvail.as<amy::sql_int>() > 0)
				{
					auth_log(LL_SYS, "Client %s (%d) logging into an unavailable account %s (status=%s)", peer->GetIP().c_str(), peer->GetId(), peer->GetLogin().c_str(), "AVAILDT");
					peer->SendLoginFailure("BLOCK");
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

				amy::async_execute(_this->GetDBManager(),
					fmt::format("UPDATE account SET last_play = NOW() WHERE login = '{0}'", peer->GetLogin()),
					std::bind(HandleGenericUpdateResult, std::placeholders::_1, std::placeholders::_2, std::ref(_this->GetDBManager()), peer, std::string("Last Play")
					));

				_this->GetAccountManager().ConnectAccount(peer->GetLogin(), loginKey);

				_this->GetDBClient()->SendAuthLogin(peer->GetId(), accId, peer->GetLogin(), socialId, lang,
					peer->GetHwid(),
					loginKey, aiPremiumTimes, peer->GetClientKey()
				);

				break;
			}
		}
		else
		{
			auth_log(LL_ERR, e.message().c_str());
			peer->SendLoginFailure("BESAMEKEY");
			peer->DelayedDisconnect(5000);
		}
	}
}
