#include <Windows.h>
#include <sstream>
#include "../include/NetLogHelper.h"

namespace net_engine
{
    CNetLogger* gs_pNetLogInstance = nullptr;

    static void LogErrorHandler(const std::string& stMessage)
    {
        Logf(CUSTOM_LOG_FILENAME, "Log error handled: %s\n", stMessage.c_str());
        MessageBoxA(0, stMessage.c_str(), "Logger initilization fail!", 0);
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
            sinks.push_back(std::make_shared<spdlog::sinks::msvc_sink_mt>());
            sinks.push_back(std::make_shared<spdlog::sinks::basic_file_sink_mt>(m_stFileName.c_str()));

            m_pkLoggerImpl = std::make_shared<spdlog::logger>(m_stLoggerName.c_str(), sinks.begin(), sinks.end());
            m_pkLoggerImpl->set_error_handler(LogErrorHandler);

            spdlog::register_logger(m_pkLoggerImpl);
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
	    auto pkLoggerInterface = spdlog::get("NetEngineLogger");
        if (!pkLoggerInterface)
        {
            Logf(CUSTOM_LOG_FILENAME, "Logger interface not found");
            return;
        }

        char szTmpBuffer[8192] = { 0 };
        va_list vaArgList;
        va_start(vaArgList, c_szFormat);
        vsprintf_s(szTmpBuffer, c_szFormat, vaArgList);
        va_end(vaArgList);

        std::stringstream ss;
        ss << stFunction << " | " << szTmpBuffer;

        auto stBuffer = ss.str();

        try
        {
            switch (nLevel)
            {
                case LL_SYS:
                    pkLoggerInterface->info(stBuffer.c_str());
                    break;
                case LL_CRI:
                    pkLoggerInterface->critical(stBuffer.c_str());
                    break;
                case LL_ERR:
                    pkLoggerInterface->error(stBuffer.c_str());
                    break;
                case LL_DEV:
                    pkLoggerInterface->debug(stBuffer.c_str());
                    break;
                case LL_TRACE:
                    pkLoggerInterface->trace(stBuffer.c_str());
                    break;
                case LL_WARN:
                    pkLoggerInterface->warn(stBuffer.c_str());
                    break;
            }
            pkLoggerInterface->flush();
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
