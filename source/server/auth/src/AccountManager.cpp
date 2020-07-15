#include "stdafx.hpp"
#include "AccountManager.hpp"
#include "AuthLogHelper.hpp"
using namespace net_engine;

GAccountManager::GAccountManager() :
	m_mt(m_rd()), m_dist(1, std::numeric_limits<std::uint32_t>::max())
{
}
GAccountManager::~GAccountManager()
{
}

void GAccountManager::ConnectAccount(const std::string& login, uint32_t key)
{
	std::unique_lock <std::mutex> lk(m_mutex);

	auth_log(LL_SYS, "ConnectAccount %s %u", login.c_str(), key);

	m_accounts.emplace(login, key);
}
void GAccountManager::DisconnectAccount(const std::string& login)
{
	std::unique_lock<std::mutex> lk(m_mutex);

	auth_log(LL_SYS, "DisconnectAccount %s", login.c_str());

	const auto iter = m_accounts.find(login);
	if (iter != m_accounts.end())
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
	} while (m_loginKeys.find(key) != m_loginKeys.end());

	auth_log(LL_SYS, "CreateAuthKey %u", key);
	return key;
}

void GAccountManager::SetKeyMinMax(std::uint32_t kMin, std::uint32_t kMax)
{
	std::unique_lock<std::mutex> lk(m_mutex);
	
	m_dist = std::uniform_int_distribution<std::uint32_t>(kMin, kMax);
}
