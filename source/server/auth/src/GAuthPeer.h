#pragma once
#include "TIdManager.h"

class GAuthServer;

class GAuthPeer : public net_engine::NetPeerBase, public IUniqueID <GAuthPeer, int32_t>
{
public:
	GAuthPeer(std::shared_ptr<GAuthServer> server);
	virtual ~GAuthPeer();
	
	std::shared_ptr<GAuthServer> GetServer() const;
	int32_t GetId() const;

	void SetLogin(const std::string &name) { m_name.assign(name); };
	const std::string& GetLogin() const { return m_name; };

	void SetPassword(const std::string &val) { m_password = val; };
	const std::string& GetPassword() const { return m_password; };

	virtual void OnConnect();
	virtual void OnDisconnect();
	virtual std::size_t OnRead(const void* data, std::size_t length);
	virtual std::size_t OnWrite(const void* data, std::size_t length);
	
	virtual void Destroy();
	
	virtual void Phase(uint8_t phaseId);
	virtual void LoginFailure(const std::string& status);
	virtual void LoginSuccess(uint8_t result);
	
	virtual void SetClientKey(const uint8_t* key);
	virtual const uint8_t* GetClientKey() const;
	
	virtual void SetLoginKey(uint32_t loginKey);
	virtual uint32_t GetLoginKey() const;
	
#if defined(AUTH_USE_HWID)
	virtual void SetHwid(const std::string &hwid);
	virtual const std::string& GetHwid() const;
#endif


private:
	std::weak_ptr<GAuthServer> m_server;
	int32_t m_id;
	std::string m_name;
	std::string m_password;
	uint8_t m_clientKey[16];
	uint32_t m_loginKey;
#if defined(AUTH_USE_HWID)
	std::string m_hwid;
#endif
};
