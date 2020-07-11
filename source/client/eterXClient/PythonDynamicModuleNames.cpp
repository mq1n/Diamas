#include "stdafx.h"
#include <random>
#include "PythonDynamicModuleNames.h"
#include <FileSystemIncl.hpp>
#include "../eterSecurity/PythonStackCheck.h"
#include <xorstr.hpp>
#include <nameof.hpp>

auto charset()
{
	return std::vector<char>(
		{ 
			'0','1','2','3','4',
			'5','6','7','8','9',
			'A','B','C','D','E','F',
			'G','H','I','J','K',
			'L','M','N','O','P',
			'Q','R','S','T','U',
			'V','W','X','Y','Z',
			'a','b','c','d','e','f',
			'g','h','i','j','k',
			'l','m','n','o','p',
			'q','r','s','t','u',
			'v','w','x','y','z'
		}
	);
};

CPythonDynamicModule::CPythonDynamicModule()
{
	Initialize();
}

void CPythonDynamicModule::Initialize()
{
	for (uint8_t i = 0; i < MODULE_MAX; ++i)
	{
		m_dynamicModuleNameArray[i] = GenerateRandomString();
#ifdef _DEBUG
		Tracenf("Created name: '%s' for module %d", m_dynamicModuleNameArray[i].c_str(), i);
#endif
	}
}

std::string CPythonDynamicModule::GenerateRandomString()
{
	const auto ch_set = charset();

	std::default_random_engine rng(std::random_device{}());
	std::uniform_int_distribution<> dist(0, ch_set.size() - 1);

	auto randchar = [ch_set, &dist, &rng]() {return ch_set[dist(rng)]; };

	std::uniform_int_distribution<> distLen(10, 20);

	auto length = distLen(rng);
	std::string str(length, 0);
	std::generate_n(str.begin(), length, randchar);

	return str;
}

std::string CPythonDynamicModule::GetModule(uint8_t nModuleID) const
{
#ifndef PYTHON_DYNAMIC_MODULE_NAME
	switch (nModuleID)
	{
	case NET_MODULE:
		return "net";
	case CHR_MODULE:
		return "chr";
	case CHRMGR_MODULE:
		return "chrmgr";
	case APP_MODULE:
		return "app";
	case PACK_MODULE:
		return "pack";
	case PLAYER_MODULE:
		return "player";
	case BACKGROUND_MODULE:
		return "background";
	case CHANGELOOK_MODULE:
		return "changelook";
	case CHAT_MODULE:
		return "chat";
	case COMBATZONE_MODULE:
		return "combatzone";
	case EFFECT_MODULE:
		return "effect";
	case EVENT_MANAGER_MODULE:
		return "event";
	case EXCHANGE_MODULE:
		return "exchange";
	case GAME_EVENTS_MODULE:
		return "gameevents";
	case GUILD_MODULE:
		return "guild";
	case IME_MODULE:
		return "ime";
	case ITEM_MODULE:
		return "item";
	case MESSENGER_MODULE:
		return "messenger";
	case MINIMAP_MODULE:
		return "miniMap";
	case NON_PLAYER_MODULE:
		return "nonplayer";
	case SHOP_SEARCH_MODULE:
		return "shopsearch";
	case QUEST_MODULE:
		return "quest";
	case SAFEBOX_MODULE:
		return "safebox";
	case SASH_MODULE:
		return "sash";
	case SHOP_MODULE:
		return "shop";
	case SKILL_MODULE:
		return "skill";
	case SKILL_PET_MODULE:
		return "petskill";
	case SYSTEM_MODULE:
		return "systemSetting";
	case TEXT_TAIL_MODULE:
		return "textTail";

	default:
		TraceError("Unknown module request: %u", nModuleID);
		return "";
	}
#else
	return m_dynamicModuleNameArray[nModuleID];
#endif
}

PyObject* pythonImportManager(PyObject* poSelf, PyObject* poArgs)
{
	CHAR __net[] = { 'n', 'e', 't', 0x0 }; // net
	CHAR __chr[] = { 'c', 'h', 'r', 0x0 }; // chr
	CHAR __chrmgr[] = { 'c', 'h', 'r', 'm', 'g', 'r', 0x0 }; // chrmgr
	CHAR __app[] = { 'a', 'p', 'p', 0x0 }; // app
	CHAR __pack[] = { 'p', 'a', 'c', 'k', 0x0 }; // pack
	CHAR __player[] = { 'p', 'l', 'a', 'y', 'e', 'r', 0x0 }; // player
	CHAR __background[] = { 'b', 'a', 'c', 'k', 'g', 'r', 'o', 'u', 'n', 'd', 0x0 }; // background
	CHAR __changelook[] = { 'c', 'h', 'a', 'n', 'g', 'e', 'l', 'o', 'o', 'k', 0x0 }; // changelook
	CHAR __chat[] = { 'c', 'h', 'a', 't', 0x0 }; // chat
	CHAR __combatzone[] = { 'c', 'o', 'm', 'b', 'a', 't', 'z', 'o', 'n', 'e', 0x0 }; // combatzone
	CHAR __effect[] = { 'e', 'f', 'f', 'e', 'c', 't', 0x0 }; // effect
	CHAR __eventmanager[] = { 'e', 'v', 'e', 'n', 't', 0x0 }; // event
	CHAR __exchange[] = { 'e', 'x', 'c', 'h', 'a', 'n', 'g', 'e', 0x0 }; // exchange
	CHAR __gamevents[] = { 'g', 'a', 'm', 'e', 'E', 'v', 'e', 'n', 't', 's', 0x0 }; // gameevents
	CHAR __guild[] = { 'g', 'u', 'i', 'l', 'd', 0x0 }; // guild
	CHAR __ime[] = { 'i', 'm', 'e', 0x0 }; // ime
	CHAR __item[] = { 'i', 't', 'e', 'm', 0x0 }; // item
	CHAR __messenger[] = { 'm', 'e', 's', 's', 'e', 'n', 'g', 'e', 'r', 0x0 }; // messenger
	CHAR __minimap[] = { 'm', 'i', 'n', 'i', 'M', 'a', 'p', 0x0 }; // minimap
	CHAR __nonplayer[] = { 'n', 'o', 'n', 'p', 'l', 'a', 'y', 'e', 'r', 0x0 };
	CHAR __shopsearch[] = { 'p', 'r', 'i', 'v', 'a', 't', 'e', 'S', 'h', 'o', 'p', 'S', 'e', 'a', 'r', 'c', 'h', 0x0 };
	CHAR __profiler[] = { 'p', 'r', 'o', 'f', 'i', 'l', 'e', 'r', 0x0 };
	CHAR __quest[] = { 'q', 'u', 'e', 's', 't', 0x0 };
	CHAR __safebox[] = { 's', 'a', 'f', 'e', 'b', 'o', 'x', 0x0 };
	CHAR __sash[] = { 's', 'a', 's', 'h', 0x0 };
	CHAR __shop[] = { 's', 'h', 'o', 'p', 0x0 };
	CHAR __skill[] = { 's', 'k', 'i', 'l', 'l', 0x0 };
	CHAR __skillpet[] = { 's', 'k', 'i', 'l', 'l', 'p', 'e', 't', 0x0 };
	CHAR __systemModule[] = { 's', 'y', 's', 't', 'e', 'm', 'S', 'e', 't', 't', 'i', 'n', 'g', 0x0 };
	CHAR __textTail[] = { 't', 'e', 'x', 't', 'T', 'a', 'i', 'l', 0x0 };
	CHAR __s[] = { 's', 0x0 }; // s

	std::string stReferFile = PY_REF_FILE;
	std::string stReferFunc = PY_REF_FUNC;

	char* szOldName;
	if (!PyTuple_GetString(poArgs, 0, &szOldName))
		return Py_BuildException();

#ifndef PYTHON_DYNAMIC_MODULE_NAME
	return Py_BuildValue(__s, szOldName);
#endif

	if (!stReferFile.empty() && !FileSystemManager::Instance().DoesFileExist(stReferFile, true))
	{
		TraceError(xorstr("Unknown module import reference from '%s':'%s'").crypt_get(), stReferFile.c_str(), stReferFunc.c_str());
		return Py_BuildNone();
	}

	if (!strcmp(szOldName, __net))
		return Py_BuildValue(__s, CPythonDynamicModule::Instance().GetModule(NET_MODULE).c_str());
	else if (!strcmp(szOldName, __chr))
		return Py_BuildValue(__s, CPythonDynamicModule::Instance().GetModule(CHR_MODULE).c_str());
	else if (!strcmp(szOldName, __chrmgr))
		return Py_BuildValue(__s, CPythonDynamicModule::Instance().GetModule(CHRMGR_MODULE).c_str());
	else if (!strcmp(szOldName, __app))
		return Py_BuildValue(__s, CPythonDynamicModule::Instance().GetModule(APP_MODULE).c_str());
	else if (!strcmp(szOldName, __pack))
		return Py_BuildValue(__s, CPythonDynamicModule::Instance().GetModule(PACK_MODULE).c_str());
	else if (!strcmp(szOldName, __player))
		return Py_BuildValue(__s, CPythonDynamicModule::Instance().GetModule(PLAYER_MODULE).c_str());
	else if (!strcmp(szOldName, __background))
		return Py_BuildValue(__s, CPythonDynamicModule::Instance().GetModule(BACKGROUND_MODULE).c_str());
	else if (!strcmp(szOldName, __changelook))
		return Py_BuildValue(__s, CPythonDynamicModule::Instance().GetModule(CHANGELOOK_MODULE).c_str());
	else if (!strcmp(szOldName, __chat))
		return Py_BuildValue(__s, CPythonDynamicModule::Instance().GetModule(CHAT_MODULE).c_str());
	else if (!strcmp(szOldName, __combatzone))
		return Py_BuildValue(__s, CPythonDynamicModule::Instance().GetModule(COMBATZONE_MODULE).c_str());
	else if (!strcmp(szOldName, __effect))
		return Py_BuildValue(__s, CPythonDynamicModule::Instance().GetModule(EFFECT_MODULE).c_str());
	else if (!strcmp(szOldName, __eventmanager))
		return Py_BuildValue(__s, CPythonDynamicModule::Instance().GetModule(EVENT_MANAGER_MODULE).c_str());
	else if (!strcmp(szOldName, __exchange))
		return Py_BuildValue(__s, CPythonDynamicModule::Instance().GetModule(EXCHANGE_MODULE).c_str());
	else if (!strcmp(szOldName, __gamevents))
		return Py_BuildValue(__s, CPythonDynamicModule::Instance().GetModule(GAME_EVENTS_MODULE).c_str());
	else if (!strcmp(szOldName, __guild))
		return Py_BuildValue(__s, CPythonDynamicModule::Instance().GetModule(GUILD_MODULE).c_str());
	else if (!strcmp(szOldName, __ime))
		return Py_BuildValue(__s, CPythonDynamicModule::Instance().GetModule(IME_MODULE).c_str());
	else if (!strcmp(szOldName, __item))
		return Py_BuildValue(__s, CPythonDynamicModule::Instance().GetModule(ITEM_MODULE).c_str());
	else if (!strcmp(szOldName, __messenger))
		return Py_BuildValue(__s, CPythonDynamicModule::Instance().GetModule(MESSENGER_MODULE).c_str());
	else if (!strcmp(szOldName, __minimap))
		return Py_BuildValue(__s, CPythonDynamicModule::Instance().GetModule(MINIMAP_MODULE).c_str());
	else if (!strcmp(szOldName, __nonplayer))
		return Py_BuildValue(__s, CPythonDynamicModule::Instance().GetModule(NON_PLAYER_MODULE).c_str());
	else if (!strcmp(szOldName, __shopsearch))
		return Py_BuildValue(__s, CPythonDynamicModule::Instance().GetModule(SHOP_SEARCH_MODULE).c_str());
	else if (!strcmp(szOldName, __quest))
		return Py_BuildValue(__s, CPythonDynamicModule::Instance().GetModule(QUEST_MODULE).c_str());
	else if (!strcmp(szOldName, __safebox))
		return Py_BuildValue(__s, CPythonDynamicModule::Instance().GetModule(SAFEBOX_MODULE).c_str());
	else if (!strcmp(szOldName, __sash))
		return Py_BuildValue(__s, CPythonDynamicModule::Instance().GetModule(SASH_MODULE).c_str());
	else if (!strcmp(szOldName, __shop))
		return Py_BuildValue(__s, CPythonDynamicModule::Instance().GetModule(SHOP_MODULE).c_str());
	else if (!strcmp(szOldName, __skill))
		return Py_BuildValue(__s, CPythonDynamicModule::Instance().GetModule(SKILL_MODULE).c_str());
	else if (!strcmp(szOldName, __skillpet))
		return Py_BuildValue(__s, CPythonDynamicModule::Instance().GetModule(SKILL_PET_MODULE).c_str());
	else if (!strcmp(szOldName, __systemModule))
		return Py_BuildValue(__s, CPythonDynamicModule::Instance().GetModule(SYSTEM_MODULE).c_str());
	else if (!strcmp(szOldName, __textTail))
		return Py_BuildValue(__s, CPythonDynamicModule::Instance().GetModule(TEXT_TAIL_MODULE).c_str());
	else
		TraceError(xorstr("Error! Unknown module name('%s') request from '%s':'%s'").crypt_get(), szOldName, stReferFile.c_str(), stReferFunc.c_str());

	return Py_BuildNone();
}

void initDynamicModuleMgr()
{
	char szModuleName[] = { 'i', 'm', 'p', 'M', 'g', 'r', 0x0 };
	char szGetMethod[] = { 'g', 'e', 't', 'M', 'o', 'd', 'u', 'l', 'e', 0x0 };

	static PyMethodDef s_methods[] =
	{
		{ szGetMethod,					pythonImportManager,				METH_VARARGS},
		{ NULL,							NULL,								NULL },
	};
	Py_InitModule(szModuleName, s_methods);
}
