#include "StdAfx.h"
#include "PythonNonPlayer.h"
#include "InstanceBase.h"
#include "PythonCharacterManager.h"
#include "PythonDynamicModuleNames.h"
#include "../eterLib/ResourceManager.h"
#include "../eterSecurity/PythonStackCheck.h"

PyObject * nonplayerIsFileExist(PyObject * poSelf, PyObject * poArgs)
{
	char * szFileName;
	if (!PyTuple_GetString(poArgs, 0, &szFileName))
		return Py_BuildException();
	return Py_BuildValue("b", CResourceManager::Instance().IsFileExist(szFileName));
}

PyObject * nonplayerGetEventType(PyObject * poSelf, PyObject * poArgs)
{
	int32_t iVirtualNumber;
	if (!PyTuple_GetInteger(poArgs, 0, &iVirtualNumber))
		return Py_BuildException();

	uint8_t iType = CPythonNonPlayer::Instance().GetEventType(iVirtualNumber);

	return Py_BuildValue("i", iType);
}

PyObject * nonplayerGetEventTypeByVID(PyObject * poSelf, PyObject * poArgs)
{
	int32_t iVirtualID;
	if (!PyTuple_GetInteger(poArgs, 0, &iVirtualID))
		return Py_BuildException();

	uint8_t iType = CPythonNonPlayer::Instance().GetEventTypeByVID(iVirtualID);

	return Py_BuildValue("i", iType);
}

PyObject * nonplayerGetLevelByVID(PyObject * poSelf, PyObject * poArgs)
{
	CPythonStackController::Instance().CheckStackReference(CHEAT_TYPE_nonplayer_GetLevelByVID, PY_REF_FILE, PY_REF_FUNC);

	int32_t iVirtualID;
	if (!PyTuple_GetInteger(poArgs, 0, &iVirtualID))
		return Py_BuildException();

	CInstanceBase * pInstance = CPythonCharacterManager::Instance().GetInstancePtr(iVirtualID);

	if (!pInstance)
		return Py_BuildValue("i", -1);

	const CPythonNonPlayer::TMobTable * pMobTable = CPythonNonPlayer::Instance().GetTable(pInstance->GetVirtualNumber());

	if (!pMobTable)
		return Py_BuildValue("i", -1);

	float fAverageLevel = pMobTable->bLevel;//(float(pMobTable->abLevelRange[0]) + float(pMobTable->abLevelRange[1])) / 2.0f;
	fAverageLevel = floor(fAverageLevel + 0.5f);
	return Py_BuildValue("i", int32_t(fAverageLevel));
}

PyObject * nonplayerGetGradeByVID(PyObject * poSelf, PyObject * poArgs)
{
	int32_t iVirtualID;
	if (!PyTuple_GetInteger(poArgs, 0, &iVirtualID))
		return Py_BuildException();

	CInstanceBase * pInstance = CPythonCharacterManager::Instance().GetInstancePtr(iVirtualID);

	if (!pInstance)
		return Py_BuildValue("i", -1);

	const CPythonNonPlayer::TMobTable * pMobTable = CPythonNonPlayer::Instance().GetTable(pInstance->GetVirtualNumber());

	if (!pMobTable)
		return Py_BuildValue("i", -1);

	return Py_BuildValue("i", pMobTable->bRank);
}


PyObject * nonplayerGetMonsterName(PyObject * poSelf, PyObject * poArgs)
{
	CPythonStackController::Instance().CheckStackReference(CHEAT_TYPE_nonplayer_GetMonsterName, PY_REF_FILE, PY_REF_FUNC);

	int32_t iVNum;
	if (!PyTuple_GetInteger(poArgs, 0, &iVNum))
		return Py_BuildException();

	CPythonNonPlayer& rkNonPlayer=CPythonNonPlayer::Instance();
	return Py_BuildValue("s", rkNonPlayer.GetMonsterName(iVNum));
}

PyObject * nonplayerLoadNonPlayerData(PyObject * poSelf, PyObject * poArgs)
{
	char * szFileName;
	if(!PyTuple_GetString(poArgs, 0, &szFileName))
		return Py_BuildException();

	CPythonNonPlayer::Instance().LoadNonPlayerData(szFileName);
	return Py_BuildNone();
}


PyObject * nonplayerGetMobRegenCycle(PyObject * poSelf, PyObject * poArgs)
{
	int32_t race;
	if (!PyTuple_GetInteger(poArgs, 0, &race))
		return Py_BuildException();

	CPythonNonPlayer& rkNonPlayer = CPythonNonPlayer::Instance();

	return Py_BuildValue("i", rkNonPlayer.GetMobRegenCycle(race));
}

PyObject * nonplayerGetMobRegenPercent(PyObject * poSelf, PyObject * poArgs)
{
	int32_t race;
	if (!PyTuple_GetInteger(poArgs, 0, &race))
		return Py_BuildException();

	CPythonNonPlayer& rkNonPlayer = CPythonNonPlayer::Instance();

	return Py_BuildValue("i", rkNonPlayer.GetMobRegenPercent(race));
}

PyObject * nonplayerGetMobGoldMin(PyObject * poSelf, PyObject * poArgs)
{
	int32_t race;
	if (!PyTuple_GetInteger(poArgs, 0, &race))
		return Py_BuildException();

	CPythonNonPlayer& rkNonPlayer = CPythonNonPlayer::Instance();

	return Py_BuildValue("i", rkNonPlayer.GetMobGoldMin(race));
}

PyObject * nonplayerGetMobGoldMax(PyObject * poSelf, PyObject * poArgs)
{
	int32_t race;
	if (!PyTuple_GetInteger(poArgs, 0, &race))
		return Py_BuildException();

	CPythonNonPlayer& rkNonPlayer = CPythonNonPlayer::Instance();

	return Py_BuildValue("i", rkNonPlayer.GetMobGoldMax(race));
}

PyObject * nonplayerGetMonsterMaxHP(PyObject * poSelf, PyObject * poArgs)
{
	int32_t race;
	if (!PyTuple_GetInteger(poArgs, 0, &race))
		return Py_BuildException();

	CPythonNonPlayer& rkNonPlayer = CPythonNonPlayer::Instance();

	return Py_BuildValue("i", rkNonPlayer.GetMonsterMaxHP(race));
}

PyObject * nonplayerGetRaceNumByVID(PyObject * poSelf, PyObject * poArgs)
{
	int32_t iVirtualID;
	if (!PyTuple_GetInteger(poArgs, 0, &iVirtualID))
		return Py_BuildException();

	CInstanceBase * pInstance = CPythonCharacterManager::Instance().GetInstancePtr(iVirtualID);

	if (!pInstance)
		return Py_BuildValue("i", -1);

	const CPythonNonPlayer::TMobTable * pMobTable = CPythonNonPlayer::Instance().GetTable(pInstance->GetVirtualNumber());

	if (!pMobTable)
		return Py_BuildValue("i", -1);

	return Py_BuildValue("i", pMobTable->dwVnum);
}

PyObject * nonplayerGetMonsterRaceFlag(PyObject * poSelf, PyObject * poArgs)
{
	int32_t race;
	if (!PyTuple_GetInteger(poArgs, 0, &race))
		return Py_BuildException();

	CPythonNonPlayer& rkNonPlayer = CPythonNonPlayer::Instance();

	return Py_BuildValue("i", rkNonPlayer.GetMonsterRaceFlag(race));
}

PyObject * nonplayerGetMonsterLevel(PyObject * poSelf, PyObject * poArgs)
{
	int32_t race;
	if (!PyTuple_GetInteger(poArgs, 0, &race))
		return Py_BuildException();

	CPythonNonPlayer& rkNonPlayer = CPythonNonPlayer::Instance();

	return Py_BuildValue("i", rkNonPlayer.GetMonsterLevel(race));
}

PyObject * nonplayerGetMonsterDamage(PyObject * poSelf, PyObject * poArgs)
{
	int32_t race;
	if (!PyTuple_GetInteger(poArgs, 0, &race))
		return Py_BuildException();

	CPythonNonPlayer& rkNonPlayer = CPythonNonPlayer::Instance();

	uint32_t dmg1 = rkNonPlayer.GetMonsterDamage1(race);
	uint32_t dmg2 = rkNonPlayer.GetMonsterDamage2(race);

	return Py_BuildValue("ii", dmg1, dmg2);
}

PyObject * nonplayerGetMonsterExp(PyObject * poSelf, PyObject * poArgs)
{
	int32_t race;
	if (!PyTuple_GetInteger(poArgs, 0, &race))
		return Py_BuildException();

	CPythonNonPlayer& rkNonPlayer = CPythonNonPlayer::Instance();

	return Py_BuildValue("i", rkNonPlayer.GetMonsterExp(race));
}

PyObject * nonplayerGetMonsterDamageMultiply(PyObject * poSelf, PyObject * poArgs)
{
	int32_t race;
	if (!PyTuple_GetInteger(poArgs, 0, &race))
		return Py_BuildException();

	CPythonNonPlayer& rkNonPlayer = CPythonNonPlayer::Instance();

	return Py_BuildValue("f", rkNonPlayer.GetMonsterDamageMultiply(race));
}

PyObject * nonplayerGetMonsterST(PyObject * poSelf, PyObject * poArgs)
{
	int32_t race;
	if (!PyTuple_GetInteger(poArgs, 0, &race))
		return Py_BuildException();

	CPythonNonPlayer& rkNonPlayer = CPythonNonPlayer::Instance();

	return Py_BuildValue("i", rkNonPlayer.GetMonsterST(race));
}

PyObject * nonplayerGetMonsterDX(PyObject * poSelf, PyObject * poArgs)
{
	int32_t race;
	if (!PyTuple_GetInteger(poArgs, 0, &race))
		return Py_BuildException();

	CPythonNonPlayer& rkNonPlayer = CPythonNonPlayer::Instance();

	return Py_BuildValue("i", rkNonPlayer.GetMonsterDX(race));
}

PyObject * nonplayerIsMonsterStone(PyObject * poSelf, PyObject * poArgs)
{
	int32_t race;
	if (!PyTuple_GetInteger(poArgs, 0, &race))
		return Py_BuildException();

	CPythonNonPlayer& rkNonPlayer = CPythonNonPlayer::Instance();

	return Py_BuildValue("i", rkNonPlayer.IsMonsterStone(race) ? 1 : 0);
}

void initNonPlayer()
{
	static PyMethodDef s_methods[] =
	{
		{ "GetEventType",				nonplayerGetEventType,				METH_VARARGS },
		{ "GetEventTypeByVID",			nonplayerGetEventTypeByVID,			METH_VARARGS },
		{ "GetLevelByVID",				nonplayerGetLevelByVID,				METH_VARARGS },
		{ "GetGradeByVID",				nonplayerGetGradeByVID,				METH_VARARGS },
		{ "GetMonsterName",				nonplayerGetMonsterName,			METH_VARARGS },

		{ "LoadNonPlayerData",			nonplayerLoadNonPlayerData,			METH_VARARGS },

		{ "GetRaceNumByVID", nonplayerGetRaceNumByVID, METH_VARARGS },
		{ "GetMonsterMaxHP", nonplayerGetMonsterMaxHP, METH_VARARGS },
		{ "GetMonsterRaceFlag", nonplayerGetMonsterRaceFlag, METH_VARARGS },
		{ "GetMonsterLevel", nonplayerGetMonsterLevel, METH_VARARGS },
		{ "GetMonsterDamage", nonplayerGetMonsterDamage, METH_VARARGS },
		{ "GetMonsterExp", nonplayerGetMonsterExp, METH_VARARGS },
		{ "GetMonsterDamageMultiply", nonplayerGetMonsterDamageMultiply, METH_VARARGS },
		{ "GetMonsterST", nonplayerGetMonsterST, METH_VARARGS },
		{ "GetMonsterDX", nonplayerGetMonsterDX, METH_VARARGS },
		{ "IsMonsterStone", nonplayerIsMonsterStone, METH_VARARGS },

		{ "GetMobRegenCycle", nonplayerGetMobRegenCycle, METH_VARARGS },
		{ "GetMobRegenPercent", nonplayerGetMobRegenPercent, METH_VARARGS },
		{ "GetMobGoldMin", nonplayerGetMobGoldMin, METH_VARARGS },
		{ "GetMobGoldMax", nonplayerGetMobGoldMax, METH_VARARGS },

		{ "IsFileExist",				nonplayerIsFileExist,				METH_VARARGS },

		{ nullptr,							nullptr,								0		 },
	};

	PyObject* poModule = Py_InitModule(CPythonDynamicModule::Instance().GetModule(NON_PLAYER_MODULE).c_str(), s_methods);

	PyModule_AddIntConstant(poModule, "ON_CLICK_EVENT_NONE",		CPythonNonPlayer::ON_CLICK_EVENT_NONE);
	PyModule_AddIntConstant(poModule, "ON_CLICK_EVENT_BATTLE",		CPythonNonPlayer::ON_CLICK_EVENT_BATTLE);
	PyModule_AddIntConstant(poModule, "ON_CLICK_EVENT_SHOP",		CPythonNonPlayer::ON_CLICK_EVENT_SHOP);
	PyModule_AddIntConstant(poModule, "ON_CLICK_EVENT_TALK",		CPythonNonPlayer::ON_CLICK_EVENT_TALK);
	PyModule_AddIntConstant(poModule, "ON_CLICK_EVENT_VEHICLE",		CPythonNonPlayer::ON_CLICK_EVENT_VEHICLE);

	PyModule_AddIntConstant(poModule, "PAWN", 0);
	PyModule_AddIntConstant(poModule, "S_PAWN", 1);
	PyModule_AddIntConstant(poModule, "KNIGHT", 2);
	PyModule_AddIntConstant(poModule, "S_KNIGHT", 3);
	PyModule_AddIntConstant(poModule, "BOSS", 4);
	PyModule_AddIntConstant(poModule, "KING", 5);
}