#include "StdAfx.h"
#include "resource.h"
#include "Version.h"
#include "PythonApplication.h"
#include "PythonDynamicModuleNames.h"

#include <cstdlib>
#include <fstream>
#include <xorstr.hpp>
#include <array>
#include <memory>
#include <filesystem>

#include <FileSystemIncl.hpp>
#include "../eterBase/error.h"
#include "../eterBase/lzo.h"
#include "../eterLib/Util.h"
#include "../eterWebBrowser/CWebBrowser.h"
#include "../eterSecurity/AnticheatManager.h"
#include "../eterSecurity/ProtectionMacros.h"
#include "../eterSecurity/CheatQueueManager.h"

// d3dx8.lib(cleanmesh.obj) : error LNK2019: unresolved external symbol __vsnprintf referenced in function "void __cdecl OutputError
int32_t(WINAPIV* __vsnprintf)(char*, size_t, const char*, va_list) = _vsnprintf;

extern "C"
{
	// extern int32_t _fltused;
	// volatile int32_t _AVOID_FLOATING_POINT_LIBRARY_BUG = _fltused;
	_declspec (dllexport) uint32_t NvOptimusEnablement = 0x00000001;
	__declspec (dllexport) int32_t AmdPowerXpressRequestHighPerformance = 1;
	// FILE __iob_func[3] = { *stdin,*stdout,*stderr };
};

class TimerPeriod
{
public:
	TimerPeriod()
	{
		TIMECAPS tc;
		if (timeGetDevCaps(&tc, sizeof(TIMECAPS)) != TIMERR_NOERROR)
		{
			// Unlikely to ever occur
			TraceError("Failed to get timer capabilities");
			tc.wPeriodMax = tc.wPeriodMin = 15; // default
		}

		m_period = std::min<uint32_t>(std::max<uint32_t>(tc.wPeriodMin, 5u), tc.wPeriodMax);
		timeBeginPeriod(m_period);
	}
	~TimerPeriod()
	{
		timeEndPeriod(m_period);
	}

private:
	uint32_t m_period;
};

static void GrannyError(granny_log_message_type Type, granny_log_message_origin Origin, char const * File, granny_int32x Line, char const * Error, void * UserData)
{
	//Origin==GrannyFileReadingLogMessage for granny run-time tag& revision warning (Type==GrannyWarningLogMessage)
	//Origin==GrannyControlLogMessage for miss track_group on static models as weapons warning (Type==GrannyWarningLogMessage)
	//Origin==GrannyMeshBindingLogMessage for miss bone ToSkeleton on new ymir models error (Type==GrannyErrorLogMessage)

	if (Origin == GrannyFileReadingLogMessage || Origin == GrannyControlLogMessage || Origin == GrannyMeshBindingLogMessage)
		return;

	TraceError("GRANNY: %s(%d): ERROR: %s --- [%d] %s --- [%d] %s",
		File, Line, Error, Type, GrannyGetLogMessageTypeString(Type), Origin, GrannyGetLogMessageOriginString(Origin));
}

bool PackInitialize(const std::string& folder)
{
	if (!std::filesystem::exists(folder))
	{
		TraceError("Folder: %s not found", folder.c_str());
		return false;
	}

	CTextFileLoader::SetCacheMode();
	CSoundData::SetPackMode();

	const std::string indexFile = folder + "/Index";
	const std::string devIndexFile = folder + "/DevIndex";

	CFile file;
#if defined(_DEBUG) || defined(USE_DEV_INDEX)
	if (!FileSystemManager::Instance().OpenFile(devIndexFile, file) || !file.GetSize())
#endif
	{
		if (!FileSystemManager::Instance().OpenFile(indexFile, file) || !file.GetSize())
		{
			TraceError("Failed to load %s", indexFile.c_str());
			return false;
		}
	}

	CMemoryTextFileLoader TextLoader;
	TextLoader.Bind(file.GetSize(), file.GetData());

	for (size_t i = 0; i < TextLoader.GetLineCount(); ++i)
	{
		const std::string& ArchiveName = TextLoader.GetLineString(i);
//		Tracenf("%u) %s", i, ArchiveName.c_str());

		if (ArchiveName.empty())
		{
			Tracenf("Index file line: %u is empty", i);
			continue;
		}

		const auto archiveTarget = L"data/"s + std::wstring(ArchiveName.begin(), ArchiveName.end());

		if (!std::filesystem::exists(archiveTarget))
		{
			TraceError("Archive: %ls does not exist!", archiveTarget.c_str());
			continue;
		}

		const auto& key = FileSystemManager::Instance().GetArchiveKey(ArchiveName);

		if (FileSystemManager::Instance().AddArchive(archiveTarget, key))
			Tracenf("%ls succesfully loaded!", archiveTarget.c_str());
		else
			TraceError("%ls can not loaded!", archiveTarget.c_str());
	}

	file.Close();
	return true;
}

bool RunMainScript(CPythonLauncher& pyLauncher, const char* lpCmdLine)
{
	initDynamicModuleMgr();
#ifndef __USE_CYTHON__
	initpack();
#endif
	initdbg();
	initime();
	initgrp();
	initgrpImage();
	initgrpText();
	initwndMgr();
	/////////////////////////////////////////////
	initapp();
	initsystemSetting();
	initchr();
	initchrmgr();
	initPlayer();
	initItem();
	initNonPlayer();
	initTrade();
	initChat();
	initTextTail();
	initnet();
	initMiniMap();
	initEvent();
	initeffect();
	initfly();
	initsnd();
	initeventmgr();
	initshop();
	initskill();
	initquest();
	initBackground();
	initMessenger();
	initsafebox();
	initguild();
	initServerStateChecker();
#ifdef ENABLE_ACCE_SYSTEM
	initAcce();
#endif

	PyObject* builtins = PyImport_ImportModule("__builtin__");
#ifdef _NDEBUG
	PyModule_AddIntConstant(builtins, "__DEBUG__", 0);
#else
	PyModule_AddIntConstant(builtins, "__DEBUG__", 1);
#endif

#ifdef __USE_CYTHON__
	PyModule_AddIntConstant(builtins, "__USE_CYTHON__", 1);
#else
	PyModule_AddIntConstant(builtins, "__USE_CYTHON__", 0);
#endif

	PyModule_AddStringConstant(builtins, "__COMMAND_LINE__", "");

	char szSystemMod[] = { 's', 'y', 's', 't', 'e', 'm', '_', 'p', 'y', 't', 'h', 'o', 'n', 0x0 }; // system_python
	char szSystemFile[] = { 's', 'y', 's', 't', 'e', 'm', '_', 'p', 'y', 't', 'h', 'o', 'n', '.', 'p', 'y', 0x0 }; // system_python.py
	char szSystemPyFailMsg[] = { 'M', 'a', 'i', 'n', ' ', 's', 'c', 'r', 'i', 'p', 't', ' ', 'i', 'n', 'i', 't', 'i', 'l', 'i', 'z', 'a', 't', 'i', 'o', 'n', ' ', 'f', 'a', 'i', 'l', 'e', 'd', '!', 0x0 }; // Main script initilization failed!

	__PROTECTOR_START__("RunFile")

#ifdef __USE_CYTHON__
	initrootlibManager();
	initsystem();
	PyErr_Print();
#else
	if (!pyLauncher.RunFile(szSystemFile, szSystemMod))
	{
		TraceError(szSystemPyFailMsg);
		return false;
	}
#endif
	__PROTECTOR_END__("RunFile")

	return true;
}

bool Main(HINSTANCE hInstance, LPSTR lpCmdLine)
{
	bool ret = false;

	uint32_t dwRandSeed=time(nullptr)+uint32_t(GetCurrentProcess());
	srandom(dwRandSeed);
	srand(random());

	ilInit();

	GrannyFilterMessage(GrannyFileReadingLogMessage, false);

	granny_log_callback Callback;
	Callback.Function = GrannyError;
	Callback.UserData = nullptr;
	GrannySetLogCallback(&Callback);

	static CLZO	lzo;
	static FileSystemManager fileSystem;

	if (!fileSystem.InitializeFSManager(
#ifdef ENABLE_LAYER2_FILE_ENCRYPTION
		FILE_FLAG_XTEA
#endif
	))
	{
		LogBox("File system Initialization failed. Check FileSystem.log file..");
		return ret;
	}

	fileSystem.AddToDiskBlacklist("py");
	fileSystem.AddToDiskBlacklist("pyc");

	if (!PackInitialize("data"))
	{
		LogBox("Data Initialization failed. Check log.txt file..");
		return ret;
	}

	CPythonApplication * app = new CPythonApplication;

	app->Initialize(hInstance);

#ifdef ENABLE_ANTICHEAT
	CCheatDetectQueueMgr::Instance().Initialize();
#endif

	auto period = std::make_unique<TimerPeriod>();

	CPythonLauncher pyLauncher;

	char __pyApi[] = { 'p', 'y', 'A', 'p', 'i', 0x0 }; // pyApi

	__PROTECTOR_START__("RunMainScript")
	if (pyLauncher.Create(__pyApi))
		ret=RunMainScript(pyLauncher, lpCmdLine);
	__PROTECTOR_END__("RunMainScript")

	app->Clear();

	pyLauncher.Clear();
		
#ifdef ENABLE_ANTICHEAT
	CAnticheatManager::Instance().FinalizeAnticheatRoutines();
#endif

	app->Destroy();

	fileSystem.FinalizeFSManager();

	delete app;
	return ret;
}

int32_t APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int32_t nCmdShow)
{
	SetEterExceptionHandler();

	CreateDirectoryA("logs", nullptr);
	CreateDirectoryA("config", nullptr);

#ifdef _DEBUG
	_CrtSetDbgFlag( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_CHECK_CRT_DF | _CRTDBG_LEAK_CHECK_DF );

	OpenConsoleWindow();
	OpenLogFile(true); // true == uses syserr.txt and log.txt
#else
	OpenLogFile(true); // true == uses syserr.txt and log.txt
#endif

#ifndef _DEBUG
	SetLogLevel(2);
#else
	SetLogLevel(1);
#endif

#ifdef LEAK_DETECT
	SymInitialize(GetCurrentProcess(), 0, true);
	SymSetOptions(SYMOPT_LOAD_LINES);
#endif

#ifdef ENABLE_ANTICHEAT
	static CAnticheatManager kAnticheatManager;
	kAnticheatManager.InitializePythonHooks();
#endif

	LocaleService_LoadConfig("config/locale.cfg");

	WebBrowser_Startup(hInstance);

	// Main routine
	Main(hInstance, lpCmdLine);

	// Cleanup Web Browser
	WebBrowser_Cleanup();

	// Cleanup COM
	::CoUninitialize();

#ifndef _DEBUG
	RemoveEterException();
#endif

	return EXIT_SUCCESS;
}
