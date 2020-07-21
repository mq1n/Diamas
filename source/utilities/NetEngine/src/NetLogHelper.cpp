#ifdef _WIN32
#include <Windows.h>
#endif
#include <sstream>
#include "../include/NetLogHelper.hpp"

namespace net_engine
{
    static CNetLogger* gs_pNetLogInstance = nullptr;

    static void LogErrorHandler(const std::string& stMessage)
    {
        Logf(CUSTOM_LOG_FILENAME, "Log error handled: %s\n", stMessage.c_str());
#ifdef _WIN32
        MessageBoxA(0, stMessage.c_str(), "Logger initilization fail!", 0);
#endif
        abort();
    }

    CNetLogger* CNetLogger::InstancePtr()
	{
		return gs_pNetLogInstance;
	}
	CNetLogger& CNetLogger::Instance()
	{
		assert(gs_pNetLogInstance);
		return *gs_pNetLogInstance;
	}

    CNetLogger::CNetLogger(const std::string& stLoggerName, const std::string& stFileName) :
        m_stLoggerName(stLoggerName), m_stFileName(stFileName)
    {
 		assert(!gs_pNetLogInstance);
		gs_pNetLogInstance = this;
        
       try
        {
            auto sinks = std::vector<spdlog::sink_ptr>();

            sinks.push_back(std::make_shared<spdlog::sinks::stdout_color_sink_mt>());
#ifdef _WIN32
            sinks.push_back(std::make_shared<spdlog::sinks::msvc_sink_mt>());
#endif
            sinks.push_back(std::make_shared<spdlog::sinks::basic_file_sink_mt>(m_stFileName.c_str()));

            m_pkLoggerImpl = std::make_shared<spdlog::logger>(m_stLoggerName.c_str(), sinks.begin(), sinks.end());
            m_pkLoggerImpl->set_error_handler(LogErrorHandler);
        }
        catch (const spdlog::spdlog_ex & ex)
        {
            Logf(CUSTOM_LOG_FILENAME, "Exception throw on InitLogger (spdlog::spdlog_ex): %s\n", ex.what());
            abort();
        }
        catch (uint32_t dwNumber)
        {
            Logf(CUSTOM_LOG_FILENAME, "Exception throw on InitLogger (w/ number): %p\n", dwNumber);
            abort();
        }
        catch (...)
        {
            Logf(CUSTOM_LOG_FILENAME, "Exception throw on InitLogger (w/o information!)\n");
            abort();
        }
    }
    CNetLogger::~CNetLogger()
    {
    	assert(gs_pNetLogInstance == this);
		gs_pNetLogInstance = nullptr;
    }

    void CNetLogger::Log(const std::string& stFunction, int nLevel, const char* c_szFormat, ...)
    {
 		std::lock_guard <std::recursive_mutex> __lock(m_pkMtMutex);

        if (!m_pkLoggerImpl.get())
        {
            Logf(CUSTOM_LOG_FILENAME, "Logger interface not found");
            return;
        }

        char szTmpBuffer[8192] = { 0 };
        va_list vaArgList;
        va_start(vaArgList, c_szFormat);
        vsprintf(szTmpBuffer, c_szFormat, vaArgList);
        va_end(vaArgList);

        std::stringstream ss;
        ss << stFunction << " | " << szTmpBuffer;

        auto stBuffer = ss.str();

        try
        {
            switch (nLevel)
            {
                case LL_SYS:
                    m_pkLoggerImpl->info(stBuffer.c_str());
                    break;
                case LL_CRI:
                    m_pkLoggerImpl->critical(stBuffer.c_str());
                    break;
                case LL_ERR:
                    m_pkLoggerImpl->error(stBuffer.c_str());
                    break;
                case LL_DEV:
                    m_pkLoggerImpl->debug(stBuffer.c_str());
                    break;
                case LL_TRACE:
                    m_pkLoggerImpl->trace(stBuffer.c_str());
                    break;
                case LL_WARN:
                    m_pkLoggerImpl->warn(stBuffer.c_str());
                    break;
            }

            m_pkLoggerImpl->flush();

            if (nLevel == LL_CRI)
            {
                abort();
            }
        }
        catch (const spdlog::spdlog_ex& ex)
        {
            Logf(CUSTOM_LOG_FILENAME, "Exception throw on sys_log (spdlog::spdlog_ex %s\n", ex.what());
            abort();
        }
        catch (uint32_t dwNumber)
        {
            Logf(CUSTOM_LOG_FILENAME, "Exception throw on sys_log (w/ number): %p\n", dwNumber);
            abort();
        }
        catch (...)
        {
            Logf(CUSTOM_LOG_FILENAME, "Exception throw on sys_log (w/o information!");
            abort();
        }
    }
}
