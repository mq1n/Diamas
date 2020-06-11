#include "StdAfx.h"
#include <python27/frameobject.h>
#include <FileSystemIncl.hpp>
#include "PythonLauncher.h"

#ifdef _DEBUG
static PyObject* WriteToStderr(PyObject* self, PyObject* args)
{
	const char* what;
	if (!PyArg_ParseTuple(args, "s", &what))
		return nullptr;

	TraceError("WriteToStderr/ %s", what);
	Py_RETURN_NONE;
}
#endif

CPythonLauncher::CPythonLauncher() :
	m_poModule(nullptr), m_poDic(nullptr)
{
	Py_NoSiteFlag++;
	Py_OptimizeFlag++;
}

CPythonLauncher::~CPythonLauncher()
{
	Clear();
}

void CPythonLauncher::Clear()
{
	Py_Finalize();
}

bool CPythonLauncher::Create(const char* c_szProgramName)
{
	Py_SetProgramName(const_cast<char*> (c_szProgramName));
	Py_Initialize();

	m_poModule = PyImport_AddModule("__main__");
	if (!m_poModule)
		return false;

	m_poDic = PyModule_GetDict(m_poModule);

	PyObject* builtins = PyImport_ImportModule("__builtin__");
	PyModule_AddIntConstant(builtins, "TRUE", 1);
	PyModule_AddIntConstant(builtins, "FALSE", 0);
	PyDict_SetItemString(m_poDic, "__builtins__", builtins);
	Py_DECREF(builtins);

#ifdef _DEBUG
	static PyMethodDef methods[] =
	{
		{ "write", WriteToStderr, METH_VARARGS, "Write something." },
		{ nullptr, nullptr, 0, nullptr }
	};

	auto m = Py_InitModule("__log", methods);
	PySys_SetObject("stderr", m);
	PySys_SetObject("stdout", m);
#endif
	return true;
}

bool CPythonLauncher::RunFile(const std::string& filename, const std::string& modName) const
{
	CFile file;
	if (!FileSystemManager::Instance().OpenFile(filename, file))
	{
		TraceError("Get %s failed", filename.c_str());
		return false;
	}

#ifdef ENABLE_LAYER2_FILE_ENCRYPTION
	auto decryptedBuffer = FileSystemManager::Instance().DecryptLayer2Protection(
		reinterpret_cast<const uint8_t*>(file.GetData()), file.GetSize()
	);
	if (decryptedBuffer.empty())
	{
		DEBUG_LOG(LL_ERR, "PY Layer2 decryption fail!");
		return false;
	}

	std::string data(reinterpret_cast<const char*>(decryptedBuffer.data()), decryptedBuffer.size());
#else
	std::string data(reinterpret_cast<const char*>(file.GetData()), file.GetSize());
#endif

	if (data.empty())
	{
#ifdef ENABLE_LAYER2_FILE_ENCRYPTION
		decryptedBuffer.clear();
#endif
		TraceError("Data assign for %s failed", filename.c_str());
		return false;
	}
#ifdef ENABLE_LAYER2_FILE_ENCRYPTION
	decryptedBuffer.clear();
#endif

	auto code = Py_CompileString(data.c_str(), filename.c_str(), Py_file_input);
	if (!code)
	{
		data.clear();

		TraceError("Compiling %s failed", filename.c_str());
		PyErr_Print();
		return false;
	}
	data.clear();

	auto mod = PyImport_ExecCodeModule(const_cast<char*>(modName.c_str()), code);
	if (!mod)
	{
		TraceError("Module creation for %s failed", filename.c_str());
		PyErr_Print();
		return false;
	}

	return true;
}
