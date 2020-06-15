#ifndef __INC_ETERLIB_DEBUG_H__
#define __INC_ETERLIB_DEBUG_H__

#include <Windows.h>
#include <cstdint>

extern void SetLogLevel(uint32_t uLevel);
extern void Log(uint32_t uLevel, const char* c_szMsg);
extern void Logn(uint32_t uLevel, const char* c_szMsg);
extern void Logf(uint32_t uLevel, const char* c_szFormat, ...);
extern void Lognf(uint32_t uLevel, const char* c_szFormat, ...);

extern void Trace(const char* c_szMsg);
extern void Tracen(const char* c_szMsg);
extern void Tracenf(const char* c_szFormat, ...);
extern void Tracef(const char* c_szFormat, ...);
extern void TraceError(const char* c_szFormat, ...);
extern void TraceErrorFunc(const char* func, const char* c_szFormat, ...);
extern void TraceErrorWithoutEnter(const char* c_szFormat, ...);

extern void LogBox(const char* c_szMsg, const char * c_szCaption = nullptr, HWND hWnd = nullptr);
extern void LogBoxf(const char * c_szFormat, ...);

extern void LogFile(const char* c_szMsg);
extern void LogFilef(const char * c_szMessage, ...);
extern void OpenConsoleWindow(void);
extern void CloseConsoleWindow();

extern void OpenLogFile(bool bUseLogFile = true);

extern HWND g_PopupHwnd;

#define CHECK_RETURN(flag, string)			\
	if (flag)								\
	{										\
		LogBox(string);						\
		return;								\
	}

#endif

#define FuncTrace(fmt, ...) TraceErrorFunc(__FUNCTION__, fmt, __VA_ARGS__);
