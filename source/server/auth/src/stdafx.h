#pragma once

#if defined(_WIN32)
	#define WIN32_LEAN_AND_MEAN
	#include <windows.h>
#else
	#include <unistd.h>

	#define vsprintf_s vsprintf
	#define strncpy_s strncpy
	#define _getpid getpid
#endif

#include <iomanip>
#include <fstream>
#include <sstream>
#include <iostream>
#include <vector>
#include <map>
#include <list>
#include <unordered_map>
#include <set>
#include <unordered_set>
#include <chrono>
#include <cstdint>
#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <ctime>
#include <stack>
#include <memory>
#include <new>
#include <future>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <random>
#include <csignal>
#include <cstdint>

#include <NetEngine.h>

#include <amy/asio.hpp>
#include <amy/auth_info.hpp>
#include <amy/mariadb_connector.hpp>
#include <amy/placeholders.hpp>

#include <spdlog/spdlog.h>
#if defined(_WIN32)
#include <spdlog/sinks/msvc_sink.h>
#endif

#include "../../common/service.h"
#include "../../common/length.h"

#include "CompileConfig.h"

#define LL_SYS 0
#define LL_CRI 2
#define LL_ERR 1
#define LL_WARN 3
#define LL_DEV 4
#define LL_TRACE 5
inline void sys_log(int l, const char* message, ...)
{
	char s[512] = {0};
	va_list args;
	va_start(args, message);
	vsprintf_s(s, message, args);
	va_end(args);
	auto logger = spdlog::get("auth");
	if(logger)
	{
		switch(l)
		{
			case LL_SYS: logger->info(s); break;
			case LL_CRI: logger->critical(s); break;
			case LL_ERR: logger->error(s); break;
			case LL_DEV: logger->debug(s); break;
			case LL_TRACE: logger->trace(s); break;
			case LL_WARN: logger->warn(s); break;
		}
	}
}

#define GAME_LOG_FLUSH spdlog::get("game")->flush();

#define AUTH_LOG(severity, format, ...) do { \
	auto l = spdlog::get("auth"); \
	l->log(spdlog::level::severity, format, ##__VA_ARGS__); \
} while (false)

#if  defined(_DEBUG)
#define AUTH_DLOG GAME_LOG
#else
#if defined(_WIN32)
#define AUTH_DLOG  __noop
#else
#define AUTH_DLOG  ((void)0)
#endif
#endif