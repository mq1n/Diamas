#include "stdafx.h"
#include "locale_service.h"
#include "constants.h"
#include "banword.h"
#include "utils.h"
#include "mob_manager.h"
#include "config.h"
#include "skill_power.h"

extern std::string				g_stQuestDir;
extern std::set <std::string> 	g_setQuestObjectDir;

std::string g_stServiceBasePath = ".";
std::string g_stServiceMapPath = "data/map";

std::string g_stLocale = "latin5";
std::string g_stLocaleBasePath = "locale/turkey";

int32_t(*check_name) (const char* str) = nullptr;

int32_t check_name_independent(const char * str)
{
	if (CBanwordManager::Instance().CheckString(str, strlen(str)))
		return 0;

	std::string stTemp(str);
	std::transform(stTemp.begin(), stTemp.end(), stTemp.begin(), ::tolower);

	if (CMobManager::Instance().Get(stTemp.c_str(), false))
		return 0;

	return 1;
}

int32_t check_name_alphabet(const char* str)
{
	const char* tmp;

	if (!str || !*str)
		return 0;

	if (strlen(str) < 2)
		return 0;

	for (tmp = str; *tmp; ++tmp)
	{
		// ¾ËÆÄºª°ú ¼öÀÚ¸¸ Çã¿ë
		if (isdigit(*tmp) || isalpha(*tmp) || *tmp == '_' || *tmp == 'ä' || *tmp == 'ü' || *tmp == 'ö' || *tmp == 'ß')
			continue;

		return 0;
	}

	return check_name_independent(str);
}

bool LocaleService_Init(const std::string& c_rstServiceName)
{
	g_stServiceBasePath = "locale/" + c_rstServiceName;
	g_stServiceMapPath = g_stServiceBasePath + "/map";
	g_stQuestDir = g_stServiceBasePath + "/quest";

	g_setQuestObjectDir.clear();
	g_setQuestObjectDir.insert(g_stQuestDir + "/object");

	fprintf(stdout, "Setting Locale \"%s\" (Path: %s)\n", c_rstServiceName.c_str(), g_stServiceBasePath.c_str());

	return true;
}

void LocaleService_TransferDefaultSetting()
{
	if (!check_name)
		check_name = check_name_alphabet;

	if (!exp_table)
		exp_table = exp_table_common;

	if (!CTableBySkill::Instance().Check())
		exit(1);

	if (!aiPercentByDeltaLevForBoss)
		aiPercentByDeltaLevForBoss = aiPercentByDeltaLevForBoss_euckr;

	if (!aiPercentByDeltaLev)
		aiPercentByDeltaLev = aiPercentByDeltaLev_euckr;

	if (!aiChainLightningCountBySkillLevel)
		aiChainLightningCountBySkillLevel = aiChainLightningCountBySkillLevel_euckr;
}

void Locale_LoadLocaleStringFile()
{
	if (g_bAuthServer)
		return;

	std::string stLocaleStringFile = g_stLocaleBasePath + "/locale_string.txt";

	fprintf(stderr, "Load LocaleString %s\n", stLocaleStringFile.c_str());

	locale_init(stLocaleStringFile.c_str());
}

void Locale_Init()
{
	if (!CTableBySkill::Instance().Check())
		exit(1);

	Locale_LoadLocaleStringFile();
}

const std::string& LocaleService_GetBasePath()
{
	return g_stServiceBasePath;
}

const std::string& LocaleService_GetMapPath()
{
	return g_stServiceMapPath;
}

const std::string& LocaleService_GetQuestPath()
{
	return g_stQuestDir;
}
