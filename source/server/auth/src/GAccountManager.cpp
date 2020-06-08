#include "stdafx.h"
#include "GAccountManager.h"

GAccountManager::GAccountManager() : m_mt(m_rd()), m_dist(1, std::numeric_limits<std::uint32_t>::max()) {}

void GAccountManager::ConnectAccount(const std::string& login, uint32_t key)
{
	std::unique_lock<std::mutex> lk(m_mutex);
	AUTH_LOG(info, "ConnectAccount {0} {1}", login.c_str(), key);
	m_accounts.insert(std::unordered_map<std::string, uint32_t>::value_type(login, key));
}
void GAccountManager::DisconnectAccount(const std::string& login)
{
	std::unique_lock<std::mutex> lk(m_mutex);
	AUTH_LOG(info, "DisconnectAccount {0}", login);
	auto iter = m_accounts.find(login);
	if(iter != m_accounts.end())
	{
		auto it = m_loginKeys.find(iter->second);
		if(it != m_loginKeys.end())
		{
			m_loginKeys.erase(it);
		}
		m_accounts.erase(iter);
	}
}
bool GAccountManager::FindAccount(const std::string& login) const
{
	std::unique_lock<std::mutex> lk(m_mutex);
	return m_accounts.find(login) != m_accounts.end();
}
uint32_t GAccountManager::CreateAuthKey()
{
	std::unique_lock<std::mutex> lk(m_mutex);
	uint32_t key = 0;
	do
	{
		key = m_dist(m_mt);
	} while(m_loginKeys.find(key) != m_loginKeys.end());
	AUTH_LOG(info, "CreateAuthKey {0}", key);
	return key;
}
