#include "StdAfx.h"
#include "AnticheatManager.h"
#include "CheatQueueManager.h"
#include "NTDefinations.h"
#include <FileSystemIncl.hpp>
#include <detours.h>
#include <xorstr.hpp>
#include <Python/Python.h>

typedef PyObject*(* TPy_InitModule4)(const char *name, PyMethodDef *methods, const char *doc, PyObject *self, int apiver);
static TPy_InitModule4 OPy_InitModule4 = nullptr;

typedef struct _mod *(* TPyParser_ASTFromString)(const char *, const char *, int, PyCompilerFlags *flags,  PyArena *);
static TPyParser_ASTFromString OPyParser_ASTFromString = nullptr;

typedef struct _mod *(* TPyParser_ASTFromFile)(FILE *, const char *, int, char *, char *, PyCompilerFlags *, int *,  PyArena *);
static TPyParser_ASTFromFile OPyParser_ASTFromFile = nullptr;

static std::map <std::string, PyObject*> gs_PythonModules;
static inline bool IsKnownModule(PyObject* module)
{
	for (const auto& it : gs_PythonModules)
	{
		if (it.second == module)
			return true;
	}
	return false;
}

PyObject * Py_InitModule4Detour(const char *name, PyMethodDef *methods, const char *doc, PyObject *self, int apiver)
{
//	TraceError("[Py_InitModule4] - %s - %p - %p - %d", name, methods, self, apiver);

	auto pyObj = OPy_InitModule4(name, methods, doc, self, apiver);
	if (pyObj)
		gs_PythonModules.emplace(name, pyObj);
	return pyObj;
}
struct _mod * PyParser_ASTFromStringDetour(const char * arg1, const char * arg2, int arg3, PyCompilerFlags *flags, PyArena * arena)
{
//	TraceError("[PyParser_ASTFromString] - %s - %s - %d - %p - %p", arg1, arg2, arg3, flags, arena);
	// TODO: Analyse
	return OPyParser_ASTFromString(arg1, arg2, arg3, flags, arena);
}
struct _mod * PyParser_ASTFromFileDetour(FILE * arg1, const char * arg2, int arg3, char * arg4, char * arg5, PyCompilerFlags * arg6, int * arg7,  PyArena * arena)
{
//	TraceError("[PyParser_ASTFromFile] - %p - %s - %d - %s - %s - %p - %d - %p", arg1, arg2, arg3, arg4, arg5, arg6, arg7, arena);
	
	CAnticheatManager::Instance().ExitByAnticheat(UNALLOWED_PYTHON_API, 0, 0, true, arg2);
	return OPyParser_ASTFromFile(arg1, arg2, arg3, arg4, arg5, arg6, arg7, arena);
}

bool CAnticheatManager::InitializePythonHooks()
{
	OPy_InitModule4 = (TPy_InitModule4)DetourFunction((PBYTE)&Py_InitModule4, (PBYTE)Py_InitModule4Detour);
	if (!OPy_InitModule4)
	{
		// TraceError("Py_InitModule4 hook failed!");
		// return false;
	}
	/*
	OPyParser_ASTFromString = (TPyParser_ASTFromString)DetourFunction((PBYTE)&PyParser_ASTFromString, (PBYTE)PyParser_ASTFromStringDetour);
	if (!OPyParser_ASTFromString)
	{
		// TraceError("PyParser_ASTFromString hook failed!");
		// return false;
	}
	*/
	OPyParser_ASTFromFile = (TPyParser_ASTFromFile)DetourFunction((PBYTE)&PyParser_ASTFromFile, (PBYTE)PyParser_ASTFromFileDetour);
	if (!OPyParser_ASTFromFile)
	{
		// TraceError("PyParser_ASTFromFile hook failed!");
		// return false;
	}
	return true;
}
void CAnticheatManager::RemovePythonModuleWatcher()
{
	if (OPy_InitModule4)
	{
		if (DetourRemove((PBYTE)OPy_InitModule4, (PBYTE)Py_InitModule4Detour))
		{
			OPy_InitModule4 = nullptr;
//			Tracenf("Python module watcher removed");
		}
	}
}
void CAnticheatManager::DestroyPythonHooks()
{
	if (OPy_InitModule4)
		DetourRemove((PBYTE)OPy_InitModule4, (PBYTE)Py_InitModule4Detour);
//	if (OPyParser_ASTFromString)
//		DetourRemove((PBYTE)OPyParser_ASTFromString, (PBYTE)PyParser_ASTFromStringDetour);
	if (OPyParser_ASTFromFile)
		DetourRemove((PBYTE)OPyParser_ASTFromFile, (PBYTE)PyParser_ASTFromFileDetour);
}

// -------

inline auto PyTupleToStdVector(PyObject* pyTupleObj)
{
	auto vBuffer = std::vector <std::string>();

	if (PyTuple_Check(pyTupleObj))
	{
		for (Py_ssize_t i = 0; i < PyTuple_Size(pyTupleObj); ++i)
		{
			PyObject *value = PyTuple_GetItem(pyTupleObj, i);
			vBuffer.push_back(PyString_AsString(value));
		}
	}
	return vBuffer;
}

void CAnticheatManager::CheckPythonModules()
{
	auto modules = PyImport_GetModuleDict();

	Py_ssize_t pos = 0;
	PyObject *mod_name, *mod;
    while (PyDict_Next(modules, &pos, &mod_name, &mod))
	{
		auto stModuleName = std::string(PyString_AsString(mod_name));
//		TraceError("%s", stModuleName.c_str());

		if (PyObject_HasAttrString(mod, xorstr("__file__").crypt_get())) // file
		{
			auto pyModuleFileName  = PyObject_GetAttrString(mod, xorstr("__file__").crypt_get());
			if (!pyModuleFileName || pyModuleFileName == Py_None)
			{
				ExitByAnticheat(UNKNOWN_PYTHON_MODULE, 0, 0, true, xorstr("Unknown module detected: %s").crypt_get());
				continue;
			}

			auto stCurrModuleFile = std::string(PyString_AsString(pyModuleFileName));
			if (!FileSystemManager::Instance().DoesFileExist(stCurrModuleFile, true))
			{
				char szWarningMsg[256];
				sprintf_s(szWarningMsg, xorstr("Suspected module file detected: %s").crypt_get(), stCurrModuleFile.c_str());
				ExitByAnticheat(SUSPECTED_PYTHON_FILE, 0, 0, true, szWarningMsg);
			}
		}
		else // builtin
		{
			char szWarningMsg[256];
			sprintf_s(szWarningMsg, xorstr("Suspected module detected: %s").crypt_get(), stModuleName.c_str());

			if (!IsKnownModule(mod)) // does not created by client's python api
			{
				ExitByAnticheat(SUSPECTED_PYTHON_BUILTIN, 0, 0, true, szWarningMsg);
			}
		}
	}
}
