#pragma once
#include <NetPeerIdManager.hpp>
#include <unordered_map>
#include <functional>
#include <variant>
#include <memory>

class GAuthServer;

class GAuthPeer : public net_engine::NetPeerBase, public net_engine::IUniqueID <GAuthPeer, uint32_t>
{
	using ConnectionProperty = std::variant <std::string, uint32_t, uint8_t>;

public:
	GAuthPeer(
		std::shared_ptr <GAuthServer> server,
		uint8_t securityLevel, const net_engine::TPacketCryptKey& cryptKey
	);
	virtual ~GAuthPeer();
	
	// Close peer connection
	virtual void Destroy();

	// IDLE deadline
	virtual void CheckDeadlineStatus();

	// Low level engine callbacks
	virtual void OnConnect();
	virtual void OnDisconnect(const asio::error_code& er);
	virtual void OnRead(std::shared_ptr <net_engine::Packet> packet);
	virtual void OnError(uint32_t ulErrorType, const asio::error_code& er);

	// Packet funcs
	virtual void SendPhase(uint8_t phaseId);
	virtual void SendLoginFailure(const std::string& status);
	virtual void SendLoginSuccess(uint8_t result);

	// Property
	bool HasProperty(const std::string& property) const;
	void SetProperty(const std::string& property, ConnectionProperty value);
	ConnectionProperty GetProperty(const std::string& property) const;
	template <typename T>
	T GetProperty(const std::string& property) const {
		return std::get<T>(GetProperty(property));
	}

	// Setter & Getter
	std::shared_ptr <GAuthServer> GetServer() const { return m_server.lock(); }
	uint32_t GetId() const { return m_id; }

	void SetLogin(const std::string &name) { m_name.assign(name); };
	const std::string& GetLogin() const { return m_name; };

	void SetPassword(const std::string &val) { m_password = val; };
	const std::string& GetPassword() const { return m_password; };

	void SetClientKey(const uint8_t* key) { memcpy(m_clientKey, key, sizeof(m_clientKey)); };
	const uint8_t* GetClientKey() const { return m_clientKey; };
	
	void SetLoginKey(uint32_t loginKey) { m_loginKey = loginKey; };
	uint32_t GetLoginKey() const { return m_loginKey; };
	
	void SetHwid(const std::string& hwid) { m_hwid = hwid; };
	const std::string& GetHwid() const { return m_hwid; };

	// IO
	virtual void SendAsReply(std::shared_ptr <net_engine::Packet> request, std::shared_ptr <net_engine::Packet> reply);

protected:
	void OnRecvHackPacket(std::shared_ptr <net_engine::Packet> packet);
	void OnRecvLogin3Packet(std::shared_ptr <net_engine::Packet> packet);
	void OnRecvKeyAgreementPacket(std::shared_ptr <net_engine::Packet> packet);
	void OnRecvHandshakePacket(std::shared_ptr <net_engine::Packet> packet);

	static void OnLoginSQLQuery(std::weak_ptr<GAuthServer> self, const asio::error_code& e, GAuthPeer* peer, uint32_t loginKey);
	static void OnLoginSQLResult(std::weak_ptr<GAuthServer> self, const asio::error_code& e, amy::result_set rs, GAuthPeer* peer, uint32_t loginKey);

private:
	std::weak_ptr <GAuthServer> m_server;
	net_engine::NetPacketDispatcher m_dispatcher;

	// todo: move to properties
	uint32_t m_id;
	std::string m_name;
	std::string m_password;
	uint8_t m_clientKey[16];
	uint32_t m_loginKey;
	std::string m_hwid;
	// end

	std::map <std::string, ConnectionProperty>	m_properties;
	asio::high_resolution_timer 				m_deadline_timer;
};
