#pragma once
#include "BasicLog.hpp"

#include <spdlog/spdlog.h>
#include <spdlog/sinks/msvc_sink.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/stdout_sinks.h>
#include <spdlog/sinks/stdout_color_sinks.h>

namespace FileSystem
{
    static constexpr auto CUSTOM_LOG_FILENAME = "FileSystem.log";
    static constexpr auto CUSTOM_LOG_ERROR_FILENAME = "FileSystemError.log";

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
