#pragma once

class GAccountManager
{
public:
	GAccountManager();
	~GAccountManager();

	void ConnectAccount(const std::string& login, uint32_t key);
	void DisconnectAccount(const std::string& login);
	bool FindAccount(const std::string& login) const;
	uint32_t CreateAuthKey();
	
private:
	mutable std::mutex m_mutex;

	std::random_device m_rd;
	std::mt19937 m_mt;
	std::uniform_int_distribution <uint32_t> m_dist;
	std::unordered_map <std::string, uint32_t> m_accounts;
	std::unordered_set <uint32_t> m_loginKeys;
};
