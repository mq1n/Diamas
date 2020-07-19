#pragma once
#include <python27/Python.h>

/* rootlib module */
/* rootlib.isExist */
/* rootlib.moduleImport */
/* this lib includes modules under this lines.
	GuildBuildingList
	Prototype
	colorInfo
	consoleModule
	constInfo
	debugInfo
	dragon_soul_refine_settings
	emotion
	exception
	game
	interfaceModule
	introCreate
	introEmpire
	introLoading
	introLogin
	introLogo
	introSelect
	localeInfo
	mouseModule
	musicInfo
	networkModule
	npcList
	playerSettingModule
	serverInfo
	stringCommander
	system
	ui
	uiAcce
	uiAffectShower
	uiAttachMetin
	uiCharacter
	uiChat
	uiChestDrop
	uiCommon
	uiCube
	uiDragonSoul
	uiEquipmentDialog
	uiExchange
	uiGameButton
	uiGameOption
	uiGuild
	uiInventory
	uiMapNameShower
	uiMessenger
	uiMiniMap
	uiParty
	uiPhaseCurtain
	uiPickMoney
	uiPlayerGauge
	uiPrivateShopBuilder
	uiQuest
	uiRefine
	uiRestart
	uiSafebox
	uiScriptLocale
	uiSelectItem
	uiSelectMusic
	uiShop
	uiSystem
	uiSystemOption
	uiTarget
	uiTaskBar
	uiTip
	uiToolTip
	uiUploadMark
	uiWeb
	uiWhisper
*/
#ifdef __USE_CYTHON__
	PyMODINIT_FUNC initsystem();
#endif

void initrootlibManager();
