#pragma once
#include "NetBasicLog.h"

#include <spdlog/spdlog.h>
#include <spdlog/sinks/msvc_sink.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/stdout_sinks.h>
#include <spdlog/sinks/stdout_color_sinks.h>

namespace net_engine
{
    static constexpr auto CUSTOM_LOG_FILENAME = "NetEngine.log";

    enum ENetLogLevels
    {
        LL_SYS,
        LL_ERR,
        LL_CRI,
        LL_WARN,
        LL_DEV,
        LL_TRACE
    };

    class CNetLogger
    {
        public:
            CNetLogger(const std::string& stLoggerName, const std::string& stFileName);
			~CNetLogger();

            CNetLogger* InstancePtr();
            CNetLogger& Instance();

            void Log(const std::string& stFunction, int nLevel, const char* c_szFormat, ...);

        private:
            mutable std::recursive_mutex		m_pkMtMutex;

            std::shared_ptr <spdlog::logger>	m_pkLoggerImpl;
            std::string							m_stLoggerName;
            std::string							m_stFileName;
    };

    extern CNetLogger* gs_pNetLogInstance;
}

#ifndef DEBUG_LOG
	#if defined(_DEBUG)
		#define DEBUG_LOG(level, log, ...)	if (net_engine::gs_pNetLogInstance) { net_engine::gs_pNetLogInstance->Log(__FUNCTION__,  level, log, __VA_ARGS__); }
	#else
		#define DEBUG_LOG(level, log, ...)
	#endif
#endif

#ifndef NET_LOG
    #define NET_LOG(level, log, ...)	if (net_engine::gs_pNetLogInstance) { net_engine::gs_pNetLogInstance->Log(__FUNCTION__,  level, log, __VA_ARGS__); }
#endif
