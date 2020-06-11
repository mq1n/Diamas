#include "StdAfx.h"
#include "../eterGameLib/GameEventManager.h"
#include "PythonDynamicModuleNames.h"

PyObject * eventMgrUpdate(PyObject * poSelf, PyObject * poArgs)
{
	float fx;
	if (!PyTuple_GetFloat(poArgs, 0, &fx))
		return Py_BuildException();
	float fy;
	if (!PyTuple_GetFloat(poArgs, 1, &fy))
		return Py_BuildException();
	float fz;
	if (!PyTuple_GetFloat(poArgs, 2, &fz))
		return Py_BuildException();

	CGameEventManager::Instance().SetCenterPosition(fx, fy, fz);
	CGameEventManager::Instance().Update();
	return Py_BuildNone();
}

void initeventmgr()
{
	static PyMethodDef s_methods[] = 
	{
		{ "Update",					eventMgrUpdate,					METH_VARARGS },
		{ nullptr,						nullptr,							0 },
	};

	PyObject* poModule = Py_InitModule(CPythonDynamicModule::Instance().GetModule(EVENT_MANAGER_MODULE).c_str(), s_methods);
}