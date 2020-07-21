#pragma once
#include "../../../common/singleton.h"
#include "NetBasicLog.hpp"

#include <spdlog/spdlog.h>
#include <spdlog/sinks/msvc_sink.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/stdout_sinks.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/rotating_file_sink.h>
#include <spdlog/sinks/ansicolor_sink.h>

class CAuthLogger : public CSingleton <CAuthLogger>
{
public:
	CAuthLogger(const std::string& stLoggerName, const std::string& stFileName);
	~CAuthLogger();

	void sys_log(const std::string& stFunction, int nLevel, const char* c_szFormat, ...);

private:
	mutable std::recursive_mutex		m_pkMtMutex;

	std::shared_ptr <spdlog::logger>	m_pkLoggerImpl;
	std::string							m_stLoggerName;
	std::string							m_stFileName;
};

#ifdef _WIN32
	#define auth_log(level, log, ...)		 if (CAuthLogger::InstancePtr()) { CAuthLogger::Instance().sys_log(__FUNCTION__,  level, log, __VA_ARGS__); }
#else
	#define auth_log(level, log, args...)    if (CAuthLogger::InstancePtr()) { CAuthLogger::Instance().sys_log(__FUNCTION__,  level, log, ##args); }
#endif
