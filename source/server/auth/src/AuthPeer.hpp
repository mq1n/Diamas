#pragma once
#include <NetPeerIdManager.hpp>

class GAuthServer;

class GAuthPeer : public net_engine::NetPeerBase, public net_engine::IUniqueID <GAuthPeer, uint32_t>
{
public:
	GAuthPeer(std::shared_ptr <GAuthServer> server, uint8_t securityLevel, const net_engine::TPacketCryptKey& cryptKey);
	virtual ~GAuthPeer();
	
	std::shared_ptr <GAuthServer> GetServer() const { return m_server.lock(); }
	uint32_t GetId() const { return m_id; }

	void SetLogin(const std::string &name) { m_name.assign(name); };
	const std::string& GetLogin() const { return m_name; };

	void SetPassword(const std::string &val) { m_password = val; };
	const std::string& GetPassword() const { return m_password; };

	virtual void OnConnect();
	virtual void OnDisconnect(const asio::error_code& er);
	virtual void OnRead(std::shared_ptr <net_engine::Packet> packet);
	virtual void OnError(uint32_t ulErrorType, const asio::error_code& er);

	virtual void Destroy();
	
	virtual void Phase(uint8_t phaseId);
	virtual void LoginFailure(const std::string& status);
	virtual void LoginSuccess(uint8_t result);
	
	virtual void SetClientKey(const uint8_t* key);
	virtual const uint8_t* GetClientKey() const;
	
	virtual void SetLoginKey(uint32_t loginKey);
	virtual uint32_t GetLoginKey() const;
	
	virtual void SetHwid(const std::string &hwid);
	virtual const std::string& GetHwid() const;


private:
	std::weak_ptr<GAuthServer> m_server;
	uint32_t m_id;
	std::string m_name;
	std::string m_password;
	uint8_t m_clientKey[16];
	uint32_t m_loginKey;
	std::string m_hwid;
};
