#include <Windows.h>
#include <sstream>
#include "../include/LogHelper.hpp"

namespace FileSystem
{
    CLog* gs_pFSLogInstance = nullptr;

    static void LogErrorHandler(const std::string& stMessage)
    {
 //       Logf(CUSTOM_LOG_FILENAME, "Log error handled: %s\n", stMessage.c_str());
 //       MessageBoxA(0, stMessage.c_str(), "File system log manager initilization fail!", 0);
 //       abort();
    }

    CLog::CLog(const std::string& stLoggerName, const std::string& stFileName) :
        m_stLoggerName(stLoggerName), m_stFileName(stFileName)
    {
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
            Logf(CUSTOM_LOG_ERROR_FILENAME, "Exception throw on InitLogger (spdlog::spdlog_ex): %s\n", ex.what());
            abort();
        }
        catch (uint32_t dwNumber)
        {
            Logf(CUSTOM_LOG_ERROR_FILENAME, "Exception throw on InitLogger (w/ number): %p\n", dwNumber);
            abort();
        }
        catch (...)
        {
            Logf(CUSTOM_LOG_ERROR_FILENAME, "Exception throw on InitLogger (w/o information!)\n");
            abort();
        }
    }

    void CLog::Log(const std::string& stFunction, int32_t nLevel, const char* c_szFormat, ...)
    {
	    auto pkLoggerInterface = spdlog::get("FSLogger");
        if (!pkLoggerInterface)
        {
            Logf(CUSTOM_LOG_ERROR_FILENAME, "Logger interface not found");
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
            Logf(CUSTOM_LOG_ERROR_FILENAME, "Exception throw on sys_log (spdlog::spdlog_ex %s\n", ex.what());
            abort();
        }
        catch (uint32_t dwNumber)
        {
            Logf(CUSTOM_LOG_ERROR_FILENAME, "Exception throw on sys_log (w/ number): %p\n", dwNumber);
            abort();
        }
        catch (...)
        {
            Logf(CUSTOM_LOG_ERROR_FILENAME, "Exception throw on sys_log (w/o information!");
            abort();
        }
    }
}
