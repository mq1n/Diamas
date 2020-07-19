#pragma once
#ifdef __USE_CYTHON__
#include "PythonrootlibManager.h"

struct SMethodDef
{
	char* func_name;
	void (*func)();
};

PyMODINIT_FUNC initGuildBuildingList();
PyMODINIT_FUNC initPrototype();
PyMODINIT_FUNC initcolorInfo();
PyMODINIT_FUNC initconsoleModule();
PyMODINIT_FUNC initconstInfo();
PyMODINIT_FUNC initdebugInfo();
PyMODINIT_FUNC initdragon_soul_refine_settings();
PyMODINIT_FUNC initemotion();
PyMODINIT_FUNC initexception();
PyMODINIT_FUNC initgame();
PyMODINIT_FUNC initinterfaceModule();
PyMODINIT_FUNC initintroCreate();
PyMODINIT_FUNC initintroEmpire();
PyMODINIT_FUNC initintroLoading();
PyMODINIT_FUNC initintroLogin();
PyMODINIT_FUNC initintroLogo();
PyMODINIT_FUNC initintroSelect();
PyMODINIT_FUNC initlocaleInfo();
PyMODINIT_FUNC initmouseModule();
PyMODINIT_FUNC initmusicInfo();
PyMODINIT_FUNC initnetworkModule();
PyMODINIT_FUNC initnpcList();
PyMODINIT_FUNC initplayerSettingModule();
PyMODINIT_FUNC initserverInfo();
PyMODINIT_FUNC initstringCommander();
PyMODINIT_FUNC initui();
PyMODINIT_FUNC inituiAcce();
PyMODINIT_FUNC inituiAffectShower();
PyMODINIT_FUNC inituiAttachMetin();
PyMODINIT_FUNC inituiCharacter();
PyMODINIT_FUNC inituiChat();
PyMODINIT_FUNC inituiChestDrop();
PyMODINIT_FUNC inituiCommon();
PyMODINIT_FUNC inituiCube();
PyMODINIT_FUNC inituiDragonSoul();
PyMODINIT_FUNC inituiEquipmentDialog();
PyMODINIT_FUNC inituiExchange();
PyMODINIT_FUNC inituiGameButton();
PyMODINIT_FUNC inituiGameOption();
PyMODINIT_FUNC inituiGuild();
PyMODINIT_FUNC inituiInventory();
PyMODINIT_FUNC inituiMapNameShower();
PyMODINIT_FUNC inituiMessenger();
PyMODINIT_FUNC inituiMiniMap();
PyMODINIT_FUNC inituiParty();
PyMODINIT_FUNC inituiPhaseCurtain();
PyMODINIT_FUNC inituiPickMoney();
PyMODINIT_FUNC inituiPlayerGauge();
PyMODINIT_FUNC inituiPrivateShopBuilder();
PyMODINIT_FUNC inituiQuest();
PyMODINIT_FUNC inituiRefine();
PyMODINIT_FUNC inituiRestart();
PyMODINIT_FUNC inituiSafebox();
PyMODINIT_FUNC inituiScriptLocale();
PyMODINIT_FUNC inituiSelectItem();
PyMODINIT_FUNC inituiSelectMusic();
PyMODINIT_FUNC inituiShop();
PyMODINIT_FUNC inituiSystem();
PyMODINIT_FUNC inituiSystemOption();
PyMODINIT_FUNC inituiTarget();
PyMODINIT_FUNC inituiTaskBar();
PyMODINIT_FUNC inituiTip();
PyMODINIT_FUNC inituiToolTip();
PyMODINIT_FUNC inituiUploadMark();
PyMODINIT_FUNC inituiWeb();
PyMODINIT_FUNC inituiWhisper();

SMethodDef rootlib_init_methods[] =
{
	{ "GuildBuildingList", initGuildBuildingList },
	{ "Prototype", initPrototype },
	{ "colorInfo", initcolorInfo },
	{ "consoleModule", initconsoleModule },
	{ "constInfo", initconstInfo },
	{ "debugInfo", initdebugInfo },
	{ "dragon_soul_refine_settings", initdragon_soul_refine_settings },
	{ "emotion", initemotion },
	{ "exception", initexception },
	{ "game", initgame },
	{ "interfaceModule", initinterfaceModule },
	{ "introCreate", initintroCreate },
	{ "introEmpire", initintroEmpire },
	{ "introLoading", initintroLoading },
	{ "introLogin", initintroLogin },
	{ "introLogo", initintroLogo },
	{ "introSelect", initintroSelect },
	{ "localeInfo", initlocaleInfo },
	{ "mouseModule", initmouseModule },
	{ "musicInfo", initmusicInfo },
	{ "networkModule", initnetworkModule },
	{ "npcList", initnpcList },
	{ "playerSettingModule", initplayerSettingModule },
	{ "serverInfo", initserverInfo },
	{ "stringCommander", initstringCommander },
	{ "system", initsystem },
	{ "ui", initui },
	{ "uiAcce", inituiAcce },
	{ "uiAffectShower", inituiAffectShower },
	{ "uiAttachMetin", inituiAttachMetin },
	{ "uiCharacter", inituiCharacter },
	{ "uiChat", inituiChat },
	{ "uiChestDrop", inituiChestDrop },
	{ "uiCommon", inituiCommon },
	{ "uiCube", inituiCube },
	{ "uiDragonSoul", inituiDragonSoul },
	{ "uiEquipmentDialog", inituiEquipmentDialog },
	{ "uiExchange", inituiExchange },
	{ "uiGameButton", inituiGameButton },
	{ "uiGameOption", inituiGameOption },
	{ "uiGuild", inituiGuild },
	{ "uiInventory", inituiInventory },
	{ "uiMapNameShower", inituiMapNameShower },
	{ "uiMessenger", inituiMessenger },
	{ "uiMiniMap", inituiMiniMap },
	{ "uiParty", inituiParty },
	{ "uiPhaseCurtain", inituiPhaseCurtain },
	{ "uiPickMoney", inituiPickMoney },
	{ "uiPlayerGauge", inituiPlayerGauge },
	{ "uiPrivateShopBuilder", inituiPrivateShopBuilder },
	{ "uiQuest", inituiQuest },
	{ "uiRefine", inituiRefine },
	{ "uiRestart", inituiRestart },
	{ "uiSafebox", inituiSafebox },
	{ "uiScriptLocale", inituiScriptLocale },
	{ "uiSelectItem", inituiSelectItem },
	{ "uiSelectMusic", inituiSelectMusic },
	{ "uiShop", inituiShop },
	{ "uiSystem", inituiSystem },
	{ "uiSystemOption", inituiSystemOption },
	{ "uiTarget", inituiTarget },
	{ "uiTaskBar", inituiTaskBar },
	{ "uiTip", inituiTip },
	{ "uiToolTip", inituiToolTip },
	{ "uiUploadMark", inituiUploadMark },
	{ "uiWeb", inituiWeb },
	{ "uiWhisper", inituiWhisper },
	{ NULL, NULL },
};

static PyObject* isExist(PyObject *self, PyObject *args)
{
	char* func_name;

	if(!PyArg_ParseTuple(args, "s", &func_name))
		return NULL;

	for (int i = 0; NULL != rootlib_init_methods[i].func_name;i++)
	{
		if (0 == strcmp(rootlib_init_methods[i].func_name, func_name))
		{
			return Py_BuildValue("i", 1);
		}
	}
	return Py_BuildValue("i", 0);
}

static PyObject* import(PyObject *self, PyObject *args)
{
	char* func_name;

	if(!PyArg_ParseTuple(args, "s", &func_name))
		return NULL;

	for (int i = 0; NULL != rootlib_init_methods[i].func_name;i++)
	{
		if (0 == strcmp(rootlib_init_methods[i].func_name, func_name))
		{
			rootlib_init_methods[i].func();
			if (PyErr_Occurred())
				return NULL;
			PyObject* m = PyDict_GetItemString(PyImport_GetModuleDict(), func_name);
			if (m == NULL) {
				PyErr_SetString(PyExc_SystemError,
					"dynamic module not initialized properly");
				return NULL;
			}
			Py_INCREF(m);
			return Py_BuildValue("S", m);
		}
	}
	return NULL;
}


void initrootlibManager()
{
	static struct PyMethodDef methods[] =
	{
		{"isExist", isExist, METH_VARARGS},
		{"moduleImport", import, METH_VARARGS},
		{nullptr, nullptr, 0},
	};

	Py_InitModule("rootlib", methods);
}
#endif
