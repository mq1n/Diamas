#include "StdAfx.h"
#include <ctime>
#include <chrono>
#include <stdio.h>
#include "Debug.h"
#include "Singleton.h"
#include "Timer.h"
#include <fmt/format.h>

const uint32_t DEBUG_STRING_MAX_LEN = 1024;

static bool isLogFile = false;
HWND g_PopupHwnd = nullptr;

auto g_stSyserrFileName = ""s;
auto g_stLogFileName = ""s;

class CLogFile : public CSingleton<CLogFile>
{
public:
	CLogFile() = default;

	~CLogFile() = default;

	void Initialize()
	{
		auto st = std::time(nullptr);
		auto timestamp = static_cast<std::chrono::seconds>(st).count();
		g_stLogFileName = fmt::format("logs/sys_log_{}_{}.txt", GetCurrentProcessId(), static_cast<uint64_t>(timestamp));
	
		m_fPtr.open(g_stLogFileName, "w");
	}

	void Write(const char* c_pszMsg) const
	{
		if (!m_fPtr)
			return;

		time_t ct = time(nullptr);
		struct tm ctm = *localtime(&ct);

		fprintf(m_fPtr.get(), "%02d%02d %02d:%02d:%05u :: %s", ctm.tm_mon + 1, ctm.tm_mday, ctm.tm_hour, ctm.tm_min,
			ELTimer_GetMSec() % 60000, c_pszMsg);

		fflush(m_fPtr.get());
	}

protected:
	msl::file_ptr m_fPtr;
};

static CLogFile gs_logfile;

static uint32_t gs_uLevel=0;

void SetLogLevel(uint32_t uLevel)
{
	gs_uLevel=uLevel;
}

void Log(uint32_t uLevel, const char* c_szMsg)
{
	if (uLevel>=gs_uLevel)
		Trace(c_szMsg);
}

void Logn(uint32_t uLevel, const char* c_szMsg)
{
	if (uLevel>=gs_uLevel)
		Tracen(c_szMsg);
}

void Logf(uint32_t uLevel, const char* c_szFormat, ...)
{
	if (uLevel<gs_uLevel)
		return;
	
	char szBuf[DEBUG_STRING_MAX_LEN+1];

	va_list args;
	va_start(args, c_szFormat);
	_vsnprintf(szBuf, sizeof(szBuf), c_szFormat, args);
	va_end(args);
#ifdef _DEBUG
	OutputDebugString(szBuf);	
	fputs(szBuf, stdout);
#endif

	if (isLogFile)
		LogFile(szBuf);
}

void Lognf(uint32_t uLevel, const char* c_szFormat, ...)
{
	if (uLevel<gs_uLevel)
		return;

	va_list args;
	va_start(args, c_szFormat);

	char szBuf[DEBUG_STRING_MAX_LEN+2];
	int32_t len = _vsnprintf(szBuf, sizeof(szBuf)-1, c_szFormat, args);

	if (len > 0)
	{
		szBuf[len] = '\n';
		szBuf[len + 1] = '\0';
	}
	va_end(args);
#ifdef _DEBUG
	OutputDebugString(szBuf);	
	puts(szBuf);
#endif

	if (isLogFile)
		LogFile(szBuf);
}


void Trace(const char * c_szMsg)
{
#ifdef _DEBUG
	OutputDebugString(c_szMsg);
	printf("%s", c_szMsg);
#endif

	if (isLogFile)
		LogFile(c_szMsg);
}

void Tracen(const char* c_szMsg)
{
#ifdef _DEBUG
	char szBuf[DEBUG_STRING_MAX_LEN+1];
	_snprintf_s(szBuf, sizeof(szBuf), "%s\n", c_szMsg);
	OutputDebugString(szBuf);
	puts(c_szMsg);

	if (isLogFile)
		LogFile(szBuf);
#else
	if (isLogFile)
	{
		LogFile(c_szMsg);
		LogFile("\n");
	}
#endif
}

void Tracenf(const char* c_szFormat, ...)
{
	va_list args;
	va_start(args, c_szFormat);

	char szBuf[DEBUG_STRING_MAX_LEN+2];
	int32_t len = _vsnprintf(szBuf, sizeof(szBuf)-1, c_szFormat, args);

	if (len > 0)
	{
		szBuf[len] = '\n';
		szBuf[len + 1] = '\0';
	}
	va_end(args);
#ifdef _DEBUG
	OutputDebugString(szBuf);	
	printf("%s", szBuf);
#endif

	if (isLogFile)
		LogFile(szBuf);
}

void Tracef(const char* c_szFormat, ...)
{
	char szBuf[DEBUG_STRING_MAX_LEN+1];

	va_list args;
	va_start(args, c_szFormat);
	_vsnprintf(szBuf, sizeof(szBuf), c_szFormat, args);
	va_end(args);

#ifdef _DEBUG
	OutputDebugString(szBuf);	
	fputs(szBuf, stdout);
#endif

	if (isLogFile)
		LogFile(szBuf);
}

void TraceErrorFunc(const char* func, const char* c_szFormat, ...)
{
	char szBuf[DEBUG_STRING_MAX_LEN+2];

	strncpy_s(szBuf, "SYSERR: ", DEBUG_STRING_MAX_LEN);
	int32_t len = strlen(szBuf);

	va_list args;
	va_start(args, c_szFormat);
	len = _vsnprintf(szBuf + len, sizeof(szBuf) - (len + 1), c_szFormat, args) + len;
	va_end(args);

	szBuf[len] = '\n';
	szBuf[len + 1] = '\0';

	time_t ct = time(nullptr);
	struct tm ctm = *localtime(&ct);

	fprintf(stderr, "%02d%02d %02d:%02d:%05u (%s) :: %s",
		ctm.tm_mon + 1,
		ctm.tm_mday,
		ctm.tm_hour,
		ctm.tm_min,
		ELTimer_GetMSec() % 60000,
		func,
					szBuf + 8);
	fflush(stderr);

#ifdef _DEBUG
	OutputDebugString(szBuf);
	fputs(szBuf, stdout);
#endif

	if (isLogFile)
		LogFile(szBuf);
}

void TraceError(const char* c_szFormat, ...)
{
	char szBuf[DEBUG_STRING_MAX_LEN + 2];

	strncpy(szBuf, "SYSERR: ", DEBUG_STRING_MAX_LEN);
	int32_t len = strlen(szBuf);

	va_list args;
	va_start(args, c_szFormat);
	len = _vsnprintf(szBuf + len, sizeof(szBuf) - (len + 1), c_szFormat, args) + len;
	va_end(args);

	szBuf[len] = '\n';
	szBuf[len + 1] = '\0';

	time_t ct = time(nullptr);
	struct tm ctm = *localtime(&ct);

	fprintf(stderr, "%02d%02d %02d:%02d:%05u :: %s", ctm.tm_mon + 1, ctm.tm_mday, ctm.tm_hour, ctm.tm_min, ELTimer_GetMSec() % 60000,
		szBuf + 8);
	fflush(stderr);

#ifdef _DEBUG
	OutputDebugString(szBuf);
	fputs(szBuf, stdout);
#endif

	if (isLogFile)
		LogFile(szBuf);
}

void TraceErrorWithoutEnter(const char* c_szFormat, ...)
{
	char szBuf[DEBUG_STRING_MAX_LEN + 1];

	strncpy(szBuf, "SYSERR: ", DEBUG_STRING_MAX_LEN);
	int32_t len = strlen(szBuf);

	va_list args;
	va_start(args, c_szFormat);
	len = _vsnprintf(szBuf + len, sizeof(szBuf) - (len + 1), c_szFormat, args) + len;
	va_end(args);

	szBuf[len] = '\0';

	time_t ct = time(nullptr);
	struct tm ctm = *localtime(&ct);

	fprintf(stderr, "%02d%02d %02d:%02d:%05u :: %s", ctm.tm_mon + 1, ctm.tm_mday, ctm.tm_hour, ctm.tm_min, ELTimer_GetMSec() % 60000,
		szBuf + 8);
	fflush(stderr);

#ifdef _DEBUG
	OutputDebugString(szBuf);
	fputs(szBuf, stdout);
#endif

	if (isLogFile)
		LogFile(szBuf);
}

void LogBoxf(const char* c_szFormat, ...)
{
	va_list args;
	va_start(args, c_szFormat);

	char szBuf[2048];
	_vsnprintf(szBuf, sizeof(szBuf), c_szFormat, args);

	LogBox(szBuf);
}

void LogBox(const char* c_szMsg, const char * c_szCaption, HWND hWnd)
{
	if (!hWnd)
		hWnd = g_PopupHwnd;

	MessageBox(hWnd, c_szMsg, c_szCaption ? c_szCaption : "LOG", MB_OK);
	Tracen(c_szMsg);
}

void LogFile(const char * c_szMsg)
{
	CLogFile::Instance().Write(c_szMsg);
}

void LogFilef(const char * c_szMessage, ...)
{
	va_list args;
	va_start(args, c_szMessage);
	char szBuf[DEBUG_STRING_MAX_LEN+1];
	_vsnprintf(szBuf, sizeof(szBuf), c_szMessage, args);

	CLogFile::Instance().Write(szBuf);
}

void OpenLogFile(bool bUseLogFIle)
{
    auto st = std::time(nullptr);
    auto timestamp = static_cast<std::chrono::seconds>(st).count();
	g_stSyserrFileName = fmt::format("logs/sys_err_{}_{}.txt", GetCurrentProcessId(), static_cast<uint64_t>(timestamp));

    freopen(g_stSyserrFileName.c_str(), "w", stderr);

	if (bUseLogFIle)
	{
		isLogFile = true;
		CLogFile::Instance().Initialize();
	}
}

void OpenConsoleWindow()
{
	AllocConsole();

	freopen("CONOUT$", "a", stdout);
	freopen("CONIN$", "r", stdin);
}

void CloseConsoleWindow()
{
	FreeConsole();
}
