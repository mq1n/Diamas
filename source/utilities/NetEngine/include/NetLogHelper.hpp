#pragma once
#include "NetBasicLog.hpp"

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

            static CNetLogger* InstancePtr();
            static CNetLogger& Instance();

            void Log(const std::string& stFunction, int nLevel, const char* c_szFormat, ...);

        private:
            mutable std::recursive_mutex		m_pkMtMutex;
            std::shared_ptr <spdlog::logger>	m_pkLoggerImpl;
            std::string							m_stLoggerName;
            std::string							m_stFileName;
    };
}

#ifndef NET_DEBUG_LOG
	#if defined(_DEBUG)
        #ifdef _WIN32
            #define NET_DEBUG_LOG(level, log, ...)	if (net_engine::CNetLogger::InstancePtr()) { net_engine::CNetLogger::Instance().Log(__FUNCTION__,  level, log, __VA_ARGS__); }
        #else
            #define NET_DEBUG_LOG(level, log, args...) if (net_engine::CNetLogger::InstancePtr()) { net_engine::CNetLogger::Instance().Log(__FUNCTION__,  level, log, ##args); }
        #endif
	#else
		#define NET_DEBUG_LOG(level, log, ...)
	#endif
#endif

#ifndef NET_LOG
    #ifdef _WIN32
        #define NET_LOG(level, log, ...)    if (net_engine::CNetLogger::InstancePtr()) { net_engine::CNetLogger::Instance().Log(__FUNCTION__,  level, log, __VA_ARGS__); }
    #else
        #define NET_LOG(level, log, args...)    if (net_engine::CNetLogger::InstancePtr()) { net_engine::CNetLogger::Instance().Log(__FUNCTION__,  level, log, ##args); }
    #endif
#endif
