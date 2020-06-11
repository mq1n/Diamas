#pragma once
#include <cstdint>
#include <array>

enum EPythonModules : uint8_t
{
	NET_MODULE,
	CHR_MODULE,
	CHRMGR_MODULE,
	APP_MODULE,
	PACK_MODULE,
	PLAYER_MODULE,
	BACKGROUND_MODULE,
	CHANGELOOK_MODULE,
	CHAT_MODULE,
	COMBATZONE_MODULE,
	EFFECT_MODULE,
	EVENT_MANAGER_MODULE,
	EXCHANGE_MODULE,
	GAME_EVENTS_MODULE,
	GUILD_MODULE,
	IME_MODULE,
	ITEM_MODULE,
	MESSENGER_MODULE,
	MINIMAP_MODULE,
	NON_PLAYER_MODULE,
	SHOP_SEARCH_MODULE,
	QUEST_MODULE,
	SAFEBOX_MODULE,
	SASH_MODULE,
	SHOP_MODULE,
	SKILL_MODULE,
	SKILL_PET_MODULE,
	SYSTEM_MODULE,
	TEXT_TAIL_MODULE,
	MODULE_MAX
};

extern void initDynamicModuleMgr();

class CPythonDynamicModule : public CSingleton <CPythonDynamicModule>
{
public:
	CPythonDynamicModule();
	virtual ~CPythonDynamicModule() = default;

	void Initialize();
	std::string GetModule(uint8_t nModuleID) const;

private:
	std::string GenerateRandomString();
	std::array <std::string, MODULE_MAX> m_dynamicModuleNameArray;
};

