#include "StdAfx.h"
#include <FileSystemIncl.hpp>
#include "../eterGameLib/RaceManager.h"
#include "../eterSecurity/PythonStackCheck.h"
#include "PythonDynamicModuleNames.h"

PyObject * packExist(PyObject * poSelf, PyObject * poArgs)
{
	bool bIsSystem = false;
	if (!PyTuple_GetBoolean(poArgs, 0, &bIsSystem) || !bIsSystem)
		return Py_BuildValue("i", 0);

	char * strFileName;
	if (!PyTuple_GetString(poArgs, 1, &strFileName))
		return Py_BuildException();

	return Py_BuildValue("i", FileSystemManager::Instance().DoesFileExist(strFileName) ? 1 : 0);
}

PyObject * packExistInPack(PyObject * poSelf, PyObject * poArgs)
{
	char * strFileName;
	if (!PyTuple_GetString(poArgs, 0, &strFileName))
		return Py_BuildException();

	return Py_BuildValue("i", FileSystemManager::Instance().DoesFileExist(strFileName, true) ? 1 : 0);
}

PyObject * packGet(PyObject * poSelf, PyObject * poArgs)
{
	char szFormat[] = { 's', '#', 0x0 };

	char* szFileName;
	if (!PyTuple_GetString(poArgs, 0, &szFileName))
		return Py_BuildException();

	CFile file;
	if (FileSystemManager::Instance().OpenFile(szFileName, file))
	{
#ifdef ENABLE_LAYER2_FILE_ENCRYPTION
		if (FileSystemManager::Instance().DoesFileExist(szFileName, true) && FileSystem::IsLayer2File(szFileName))
		{ // archived & also layer2 supported file
			auto decryptedBuffer = FileSystemManager::Instance().DecryptLayer2Protection(
				reinterpret_cast<const uint8_t*>(file.GetData()), file.GetSize()
			);
			if (decryptedBuffer.empty())
			{
#ifdef _DEBUG
				TraceError("Layer2 decryption fail! File: %s", szFileName);
#endif
				return Py_BuildException();
			}

			return Py_BuildValue("s#", decryptedBuffer.data(), decryptedBuffer.size());
		}
		else
#endif
		{
			return Py_BuildValue("s#", file.GetData(), file.GetSize());
		}
	}

	return Py_BuildNone();
}

static PyObject* packImportModule(PyObject* poSelf, PyObject* poArgs)
{
	PyObject* pyData = nullptr;

	char* pstData;
	Py_ssize_t uLength = 0;

	char* pstFileName;
	if (!PyTuple_GetString(poArgs, 0, &pstFileName))
		return Py_BuildException();

	PyCompilerFlags cf;
	cf.cf_flags = 0;
	PyEval_MergeCompilerFlags(&cf);

	CFile file;
	if (FileSystemManager::Instance().OpenFile(pstFileName, file))
	{
#ifdef ENABLE_LAYER2_FILE_ENCRYPTION
		if (FileSystemManager::Instance().DoesFileExist(pstFileName, true) && FileSystem::IsLayer2File(pstFileName))
		{ // archived & also layer2 supported file
			auto decryptedBuffer = FileSystemManager::Instance().DecryptLayer2Protection(
				reinterpret_cast<const uint8_t*>(file.GetData()), file.GetSize()
			);
			if (decryptedBuffer.size())
			{
				pyData = PyString_FromStringAndSize(reinterpret_cast<const char*>(decryptedBuffer.data()), decryptedBuffer.size());
				if (!pyData)
					return Py_BuildNone();

				pstData = PyString_AS_STRING(pyData);
				uLength = PyString_GET_SIZE(pyData);
			}
		}
#else
		pyData = PyString_FromStringAndSize(reinterpret_cast<const char*>(file.GetData()), file.GetSize());
		if (!pyData)
			return Py_BuildNone();

		pstData = PyString_AS_STRING(pyData);
		uLength = PyString_GET_SIZE(pyData);
#endif
	}

	if (!uLength)
	{
		std::string msg = "pack.importModule() filename not found: ";
		msg += pstFileName;
		PyErr_SetString(PyExc_TypeError, msg.c_str());
		goto cleanup;
	}

	if ((size_t)uLength != strlen(pstData))
	{
		PyErr_SetString(PyExc_TypeError, "pack.importModule() expected string without null bytes");
		goto cleanup;
	}

	auto result = Py_CompileStringFlags(pstData, pstFileName, Py_file_input, &cf);
cleanup:
	if (pyData)
	{
		Py_XDECREF(pyData);
	}
	return result;
}

static PyObject* packGetNpcList(PyObject* poSelf, PyObject* poArgs)
{
	char* filename;
	if (!PyTuple_GetString(poArgs, 0, &filename))
		return Py_BuildException();

	CFile kFile;
	if (!FileSystemManager::Instance().OpenFile(filename, kFile))
	{
		TraceError("packGetNpcList(c_szFileName=%s) - Load Error", filename);
		return Py_BuildException();
	}

	std::string fileData((const char*)kFile.GetData(), kFile.GetSize());
	const auto& lines = string_split_any(fileData, "\r\n");
	for (const auto& line : lines)
	{
		auto& kTokenVector = string_split(line, "\t");
		for (auto& token : kTokenVector)
			trim_in_place(token);

		if (kTokenVector.size() < 2)
			continue;

		uint32_t vnum = atoi(kTokenVector[0].c_str());
		if (vnum)
			CRaceManager::Instance().RegisterRaceName(vnum, kTokenVector[1].c_str());
		else if (kTokenVector.size() >= 3)
			CRaceManager::Instance().RegisterRaceSrcName(kTokenVector[1].c_str(), kTokenVector[2].c_str());
		else
			TraceError("packLoadNpcList(c_szFileName=%s) - Line Error %s %s", filename, kTokenVector[0].c_str(), kTokenVector[1].c_str());
	}
	return Py_BuildNone();
}

static PyObject* packGetLocaleInterface(PyObject* poSelf, PyObject* poArgs)
{
	char* filename;
	if (!PyTuple_GetString(poArgs, 0, &filename))
		return Py_BuildException();

	PyObject* localeDict;
	if (!PyTuple_GetObject(poArgs, 1, &localeDict))
		return Py_BuildException();

	CFile kFile;
	if (!FileSystemManager::Instance().OpenFile(filename, kFile))
	{
		TraceError("packGetLocaleInterface(c_szFileName=%s) - Load Error", filename);
		return Py_BuildException();
	}

	std::string fileData((const char*)kFile.GetData(), kFile.GetSize());
	const auto& lines = string_split_any(fileData, "\r\n");
	for (const auto& line : lines)
	{
		auto& kTokenVector = string_split(line, "\t");
		for (auto& token : kTokenVector)
			trim_in_place(token);

		if (kTokenVector.size() < 2)
			continue;

		PyDict_SetItemString(localeDict, kTokenVector[0].c_str(), PyString_FromString(kTokenVector[1].c_str()));
	}
	return Py_BuildNone();
}

static PyObject* packGetLocaleGame(PyObject* poSelf, PyObject* poArgs)
{
	char* filename;
	if (!PyTuple_GetString(poArgs, 0, &filename))
		return Py_BuildException();

	PyObject* localeDict;
	if (!PyTuple_GetObject(poArgs, 1, &localeDict))
		return Py_BuildException();

	PyObject* funcDict;
	if (!PyTuple_GetObject(poArgs, 2, &funcDict))
		return Py_BuildException();

	CFile kFile;
	if (!FileSystemManager::Instance().OpenFile(filename, kFile))
	{
		TraceError("packGetLocaleGame(c_szFileName=%s) - Load Error", filename);
		return Py_BuildException();
	}

	std::string fileData((const char*)kFile.GetData(), kFile.GetSize());
	const auto& lines = string_split_any(fileData, "\r\n");
	for (const auto& line : lines)
	{
		auto& kTokenVector = string_split(line, "\t");
		for (auto& token : kTokenVector)
			trim_in_place(token);

		if (kTokenVector.size() < 2) // skip empty or incomplete
			continue;

		PyObject* itemValue = nullptr;

		// if 3 tabs, key0=func[type2](value1)
		if (kTokenVector.size() > 2 && !kTokenVector[2].empty())
		{
			PyObject* funcType = PyDict_GetItemString(funcDict, kTokenVector[2].c_str());
			if (funcType)
				itemValue = PyObject_CallFunction(funcType, "s#", kTokenVector[1].c_str(), kTokenVector[1].size());
			else
			{
				TraceError("packLoadLocaleGame(c_szFileName=%s, funcDict=%s) - Tag Error %d", filename, kTokenVector[2].c_str(),
					kTokenVector.size());
				return Py_BuildException();
			}
		}
		else
			itemValue = PyString_FromString(kTokenVector[1].c_str());

		// if 2 tabs, key0=value1
		PyDict_SetItemString(localeDict, kTokenVector[0].c_str(), itemValue);
	}
	return Py_BuildNone();
}

void initpack()
{
	static PyMethodDef s_methods[] =
	{
		{ "Exist",					packExist,				METH_VARARGS },
		{ "ExistInPack",			packExistInPack,		METH_VARARGS },

		{ "Get",					packGet,				METH_VARARGS }, // DELETEME
		{ "ImportModule",			packImportModule,		METH_VARARGS },

		{ "GetLocaleInterface",		packGetLocaleInterface,	METH_VARARGS },
		{ "GetLocaleGame",			packGetLocaleGame,		METH_VARARGS },
		{ "GetNpcList",				packGetNpcList,			METH_VARARGS },

		{ nullptr, nullptr },
	};
	
	Py_InitModule(CPythonDynamicModule::Instance().GetModule(PACK_MODULE).c_str(), s_methods);
}
