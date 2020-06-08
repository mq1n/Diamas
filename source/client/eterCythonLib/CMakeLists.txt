cmake_minimum_required(VERSION 3.2)
project(eterCythonLib)

set(ETERCYTHONLIB_SOURCES
	${PROJECT_SOURCE_DIR}/../../../../M2020_Client/CRYPTION/source/cyTemp/animation.cpp
	${PROJECT_SOURCE_DIR}/../../../../M2020_Client/CRYPTION/source/cyTemp/camModule.cpp
	${PROJECT_SOURCE_DIR}/../../../../M2020_Client/CRYPTION/source/cyTemp/colorInfo.cpp
	${PROJECT_SOURCE_DIR}/../../../../M2020_Client/CRYPTION/source/cyTemp/consoleModule.cpp
	${PROJECT_SOURCE_DIR}/../../../../M2020_Client/CRYPTION/source/cyTemp/constInfo.cpp
	${PROJECT_SOURCE_DIR}/../../../../M2020_Client/CRYPTION/source/cyTemp/debugInfo.cpp
	${PROJECT_SOURCE_DIR}/../../../../M2020_Client/CRYPTION/source/cyTemp/dragon_soul_refine_settings.cpp
	${PROJECT_SOURCE_DIR}/../../../../M2020_Client/CRYPTION/source/cyTemp/emotion.cpp
	${PROJECT_SOURCE_DIR}/../../../../M2020_Client/CRYPTION/source/cyTemp/exception.cpp
	${PROJECT_SOURCE_DIR}/../../../../M2020_Client/CRYPTION/source/cyTemp/game.cpp
	${PROJECT_SOURCE_DIR}/../../../../M2020_Client/CRYPTION/source/cyTemp/GuildBuildingList.cpp
	${PROJECT_SOURCE_DIR}/../../../../M2020_Client/CRYPTION/source/cyTemp/interfaceModule.cpp
	${PROJECT_SOURCE_DIR}/../../../../M2020_Client/CRYPTION/source/cyTemp/introCreate.cpp
	${PROJECT_SOURCE_DIR}/../../../../M2020_Client/CRYPTION/source/cyTemp/introEmpire.cpp
	${PROJECT_SOURCE_DIR}/../../../../M2020_Client/CRYPTION/source/cyTemp/introLoading.cpp
	${PROJECT_SOURCE_DIR}/../../../../M2020_Client/CRYPTION/source/cyTemp/introLogin.cpp
	${PROJECT_SOURCE_DIR}/../../../../M2020_Client/CRYPTION/source/cyTemp/introLogo.cpp
	${PROJECT_SOURCE_DIR}/../../../../M2020_Client/CRYPTION/source/cyTemp/introSelect.cpp
	${PROJECT_SOURCE_DIR}/../../../../M2020_Client/CRYPTION/source/cyTemp/localeInfo.cpp
	${PROJECT_SOURCE_DIR}/../../../../M2020_Client/CRYPTION/source/cyTemp/looptimer.cpp
	${PROJECT_SOURCE_DIR}/../../../../M2020_Client/CRYPTION/source/cyTemp/mouseModule.cpp
	${PROJECT_SOURCE_DIR}/../../../../M2020_Client/CRYPTION/source/cyTemp/musicInfo.cpp
	${PROJECT_SOURCE_DIR}/../../../../M2020_Client/CRYPTION/source/cyTemp/networkModule.cpp
	${PROJECT_SOURCE_DIR}/../../../../M2020_Client/CRYPTION/source/cyTemp/New_introSelect.cpp
	${PROJECT_SOURCE_DIR}/../../../../M2020_Client/CRYPTION/source/cyTemp/NpcList.cpp
	${PROJECT_SOURCE_DIR}/../../../../M2020_Client/CRYPTION/source/cyTemp/playerSettingModule.cpp
	${PROJECT_SOURCE_DIR}/../../../../M2020_Client/CRYPTION/source/cyTemp/Prototype.cpp
	${PROJECT_SOURCE_DIR}/../../../../M2020_Client/CRYPTION/source/cyTemp/rootlibcythonizer.cpp
	${PROJECT_SOURCE_DIR}/../../../../M2020_Client/CRYPTION/source/cyTemp/serverCommandParser.cpp
	${PROJECT_SOURCE_DIR}/../../../../M2020_Client/CRYPTION/source/cyTemp/serverLoginData.cpp
	${PROJECT_SOURCE_DIR}/../../../../M2020_Client/CRYPTION/source/cyTemp/stringCommander.cpp
	${PROJECT_SOURCE_DIR}/../../../../M2020_Client/CRYPTION/source/cyTemp/switchbot.cpp
	${PROJECT_SOURCE_DIR}/../../../../M2020_Client/CRYPTION/source/cyTemp/system.cpp
	${PROJECT_SOURCE_DIR}/../../../../M2020_Client/CRYPTION/source/cyTemp/ui.cpp
	${PROJECT_SOURCE_DIR}/../../../../M2020_Client/CRYPTION/source/cyTemp/uiAcce.cpp
	${PROJECT_SOURCE_DIR}/../../../../M2020_Client/CRYPTION/source/cyTemp/uiAffectShower.cpp
	${PROJECT_SOURCE_DIR}/../../../../M2020_Client/CRYPTION/source/cyTemp/uiAttachMetin.cpp
	${PROJECT_SOURCE_DIR}/../../../../M2020_Client/CRYPTION/source/cyTemp/uiCards.cpp
	${PROJECT_SOURCE_DIR}/../../../../M2020_Client/CRYPTION/source/cyTemp/uiCharacter.cpp
	${PROJECT_SOURCE_DIR}/../../../../M2020_Client/CRYPTION/source/cyTemp/uiChat.cpp
	${PROJECT_SOURCE_DIR}/../../../../M2020_Client/CRYPTION/source/cyTemp/uiCommon.cpp
	${PROJECT_SOURCE_DIR}/../../../../M2020_Client/CRYPTION/source/cyTemp/uiCube.cpp
	${PROJECT_SOURCE_DIR}/../../../../M2020_Client/CRYPTION/source/cyTemp/uiDragonSoul.cpp
	${PROJECT_SOURCE_DIR}/../../../../M2020_Client/CRYPTION/source/cyTemp/uiEquipmentDialog.cpp
	${PROJECT_SOURCE_DIR}/../../../../M2020_Client/CRYPTION/source/cyTemp/uiEventCalendar.cpp
	${PROJECT_SOURCE_DIR}/../../../../M2020_Client/CRYPTION/source/cyTemp/uiExchange.cpp
	${PROJECT_SOURCE_DIR}/../../../../M2020_Client/CRYPTION/source/cyTemp/uiGameButton.cpp
	${PROJECT_SOURCE_DIR}/../../../../M2020_Client/CRYPTION/source/cyTemp/uiGameOption.cpp
	${PROJECT_SOURCE_DIR}/../../../../M2020_Client/CRYPTION/source/cyTemp/uiGuild.cpp
	${PROJECT_SOURCE_DIR}/../../../../M2020_Client/CRYPTION/source/cyTemp/uiHelp.cpp
	${PROJECT_SOURCE_DIR}/../../../../M2020_Client/CRYPTION/source/cyTemp/uiInventory.cpp
	${PROJECT_SOURCE_DIR}/../../../../M2020_Client/CRYPTION/source/cyTemp/uiMapNameShower.cpp
	${PROJECT_SOURCE_DIR}/../../../../M2020_Client/CRYPTION/source/cyTemp/uiMessenger.cpp
	${PROJECT_SOURCE_DIR}/../../../../M2020_Client/CRYPTION/source/cyTemp/uiMiniMap.cpp
	${PROJECT_SOURCE_DIR}/../../../../M2020_Client/CRYPTION/source/cyTemp/uiNation.cpp
	${PROJECT_SOURCE_DIR}/../../../../M2020_Client/CRYPTION/source/cyTemp/uiOption.cpp
	${PROJECT_SOURCE_DIR}/../../../../M2020_Client/CRYPTION/source/cyTemp/uiParty.cpp
	${PROJECT_SOURCE_DIR}/../../../../M2020_Client/CRYPTION/source/cyTemp/uiPetInfo.cpp
	${PROJECT_SOURCE_DIR}/../../../../M2020_Client/CRYPTION/source/cyTemp/uiPhaseCurtain.cpp
	${PROJECT_SOURCE_DIR}/../../../../M2020_Client/CRYPTION/source/cyTemp/uiPickMoney.cpp
	${PROJECT_SOURCE_DIR}/../../../../M2020_Client/CRYPTION/source/cyTemp/uiPlayerGauge.cpp
	${PROJECT_SOURCE_DIR}/../../../../M2020_Client/CRYPTION/source/cyTemp/uiPointReset.cpp
	${PROJECT_SOURCE_DIR}/../../../../M2020_Client/CRYPTION/source/cyTemp/uiPrivateShopBuilder.cpp
	${PROJECT_SOURCE_DIR}/../../../../M2020_Client/CRYPTION/source/cyTemp/uiPrivateShopEditor.cpp
	${PROJECT_SOURCE_DIR}/../../../../M2020_Client/CRYPTION/source/cyTemp/uiPrivateShopSearch.cpp
	${PROJECT_SOURCE_DIR}/../../../../M2020_Client/CRYPTION/source/cyTemp/uiQuest.cpp
	${PROJECT_SOURCE_DIR}/../../../../M2020_Client/CRYPTION/source/cyTemp/uiRefine.cpp
	${PROJECT_SOURCE_DIR}/../../../../M2020_Client/CRYPTION/source/cyTemp/uiRestart.cpp
	${PROJECT_SOURCE_DIR}/../../../../M2020_Client/CRYPTION/source/cyTemp/uiSafebox.cpp
	${PROJECT_SOURCE_DIR}/../../../../M2020_Client/CRYPTION/source/cyTemp/uiScriptLocale.cpp
	${PROJECT_SOURCE_DIR}/../../../../M2020_Client/CRYPTION/source/cyTemp/uiSelectItem.cpp
	${PROJECT_SOURCE_DIR}/../../../../M2020_Client/CRYPTION/source/cyTemp/uiSelectLanguage.cpp
	${PROJECT_SOURCE_DIR}/../../../../M2020_Client/CRYPTION/source/cyTemp/uiSelectMusic.cpp
	${PROJECT_SOURCE_DIR}/../../../../M2020_Client/CRYPTION/source/cyTemp/uiShop.cpp
	${PROJECT_SOURCE_DIR}/../../../../M2020_Client/CRYPTION/source/cyTemp/uiSwitchChannel.cpp
	${PROJECT_SOURCE_DIR}/../../../../M2020_Client/CRYPTION/source/cyTemp/uiSystem.cpp
	${PROJECT_SOURCE_DIR}/../../../../M2020_Client/CRYPTION/source/cyTemp/uiSystemOption.cpp
	${PROJECT_SOURCE_DIR}/../../../../M2020_Client/CRYPTION/source/cyTemp/uiTarget.cpp
	${PROJECT_SOURCE_DIR}/../../../../M2020_Client/CRYPTION/source/cyTemp/uiTaskBar.cpp
	${PROJECT_SOURCE_DIR}/../../../../M2020_Client/CRYPTION/source/cyTemp/uiTip.cpp
	${PROJECT_SOURCE_DIR}/../../../../M2020_Client/CRYPTION/source/cyTemp/uiToolTip.cpp
	${PROJECT_SOURCE_DIR}/../../../../M2020_Client/CRYPTION/source/cyTemp/uiUploadMark.cpp
	${PROJECT_SOURCE_DIR}/../../../../M2020_Client/CRYPTION/source/cyTemp/uiVote.cpp
	${PROJECT_SOURCE_DIR}/../../../../M2020_Client/CRYPTION/source/cyTemp/uiWeb.cpp
	${PROJECT_SOURCE_DIR}/../../../../M2020_Client/CRYPTION/source/cyTemp/uiWhisper.cpp
	${PROJECT_SOURCE_DIR}/../../../../M2020_Client/CRYPTION/source/cyTemp/utils.cpp
)

if(WIN32)
	set(PYTHON_CMD C:/Python27/python)
else()
	set(PYTHON_CMD python27)
endif()

add_custom_target(special_process COMMAND ${PYTHON_CMD} special_process.py WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}/../../../../M2020_Client/CRYPTION)
add_custom_target(root_process COMMAND ${PYTHON_CMD} conv/root_process.py cython WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}/../../../../M2020_Client/CRYPTION DEPENDS special_process)

add_library(${PROJECT_NAME} STATIC ${ETERCYTHONLIB_SOURCES})
add_dependencies(${PROJECT_NAME} root_process)
