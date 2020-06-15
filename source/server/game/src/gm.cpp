#include "stdafx.h"
#include "constants.h"
#include "gm.h"
#include "locale_service.h"
#include "utils.h"

extern int32_t g_bIsTestServer;

namespace GM
{
	uint32_t g_adwAdminConfig[GM_MAX_NUM];
	std::map<std::string, tGM> g_map_GM;

	void init(uint32_t* adwAdminConfig)
	{
		memcpy(&g_adwAdminConfig[0], adwAdminConfig, sizeof(g_adwAdminConfig));
	}

	void insert(const tAdminInfo &rAdminInfo)
	{
		sys_log(0, "InsertGMList(account:%s, player:%s, auth:%d)",
			rAdminInfo.m_szAccount,
			rAdminInfo.m_szName,
			rAdminInfo.m_Authority);

		tGM t;

		memcpy(&t.Info, &rAdminInfo, sizeof(rAdminInfo));

		g_map_GM[rAdminInfo.m_szName] = t;
	}

	void remove(const char* szName)
	{
		g_map_GM.erase(szName);
	}

	uint8_t get_level(const char * name, const char* account, bool ignore_test_server)
	{
		if (!ignore_test_server && g_bIsTestServer) return GM_IMPLEMENTOR;

		std::map<std::string, tGM >::iterator it = g_map_GM.find(name);

		if (g_map_GM.end() == it)
			return GM_PLAYER;

		if (account)
		{
			if (strcasecmp(it->second.Info.m_szAccount, account) != 0 && strcmp(it->second.Info.m_szAccount, "[ALL]") != 0)
			{
				sys_err("GM::get_level: account compare failed [real account %s need account %s]", account, it->second.Info.m_szAccount);
				return GM_PLAYER;
			}
		}

		sys_log(0, "GM::GET_LEVEL : FIND ACCOUNT");
		return it->second.Info.m_Authority;
	}

	void clear()
	{
		g_map_GM.clear();
	}

	bool check_allow(uint8_t bGMLevel, uint32_t dwCheckFlag)
	{
		return IS_SET(g_adwAdminConfig[bGMLevel], dwCheckFlag);
	}

	bool check_account_allow(const std::string& stAccountName, uint32_t dwCheckFlag)
	{
		std::map<std::string, tGM >::iterator it = g_map_GM.begin();

		bool bHasGM = false;
		bool bCheck = false;
		while (it != g_map_GM.end() && !bCheck)
		{
			if (stAccountName.compare(it->second.Info.m_szAccount))
			{
				bHasGM = true;

				uint8_t bGMLevel = it->second.Info.m_Authority;
				bCheck = check_allow(bGMLevel, dwCheckFlag);
			}

			++it;
		}

		return !bHasGM || bCheck;
	}

	std::vector<std::string> get_gm_list()
	{
		std::vector<std::string> gm_list;

		for (auto it = g_map_GM.begin(); it != g_map_GM.end(); ++it)
			gm_list.push_back(it->second.Info.m_szName); // only push player name back

		return gm_list;
	}
}
