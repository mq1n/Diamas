#pragma once
#include "BasicLog.hpp"

#include <spdlog/spdlog.h>
#include <spdlog/sinks/msvc_sink.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/stdout_sinks.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <fmt/format.h>

namespace FileSystem
{
 	static const auto st = std::time(nullptr);
	static const auto timestamp = static_cast<std::chrono::seconds>(st).count();
    
    static const auto CUSTOM_LOG_FILENAME = fmt::format("logs/fs_log_{}_{}.txt", GetCurrentProcessId(), static_cast<uint64_t>(timestamp));
    static const auto CUSTOM_LOG_ERROR_FILENAME = fmt::format("logs/fs_err_{}_{}.txt", GetCurrentProcessId(), static_cast<uint64_t>(timestamp));

    enum ELogLevels
    {
        LL_SYS,
        LL_ERR,
        LL_CRI,
        LL_WARN,
        LL_DEV,
        LL_TRACE
    };

    class CLog
    {
        public:
            CLog() = default;
            CLog(const std::string& stLoggerName, const std::string& stFileName);
			~CLog() = default;

            void Log(const std::string& stFunction, int32_t nLevel, const char* c_szFormat, ...);

        private:
            mutable std::recursive_mutex		m_pkMtMutex;

            std::shared_ptr <spdlog::logger>	m_pkLoggerImpl;
            std::string							m_stLoggerName;
            std::string							m_stFileName;
    };

    extern CLog* gs_pFSLogInstance;
}

#ifndef DEBUG_LOG
	#if defined(_DEBUG)
		#define DEBUG_LOG(level, log, ...)	if (FileSystem::gs_pFSLogInstance) { FileSystem::gs_pFSLogInstance->Log(__FUNCTION__,  level, log, __VA_ARGS__); }
	#else
		#define DEBUG_LOG(level, log, ...)
	#endif
#endif

#ifndef FS_LOG
    #define FS_LOG(level, log, ...)	if (FileSystem::gs_pFSLogInstance) { FileSystem::gs_pFSLogInstance->Log(__FUNCTION__,  level, log, __VA_ARGS__); }
#endif
