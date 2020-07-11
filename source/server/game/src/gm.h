#pragma once
#include <set>
#include <vector>
#include <string>

namespace GM
{
	typedef struct
	{
		tAdminInfo Info;
	} tGM;

	extern void init(uint32_t* adwAdminConfig);
	extern void insert(const tAdminInfo & c_rInfo);
	extern void remove(const char* name);
	extern uint8_t get_level(const char * name, const char * account = nullptr, bool ignore_test_server = false);
	extern void clear();

	extern bool check_allow(uint8_t bGMLevel, uint32_t dwCheckFlag);
	extern bool check_account_allow(const std::string& stAccountName, uint32_t dwCheckFlag);

	extern std::map<std::string, tGM> g_map_GM;
	extern std::vector<std::string> get_gm_list();
}
