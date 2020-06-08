#include "stdafx.h"
#include "GAuthServer.h"

#include <spdlog/spdlog.h>
#include <spdlog/sinks/msvc_sink.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/stdout_sinks.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/rotating_file_sink.h>

#include "../../libthecore/include/winminidump.h"

static int pid_init()
{
#if defined(_WIN32)
	return true;
#else
	FILE*	fp;
	if ((fp = fopen("pid", "w")))
	{
		fprintf(fp, "%d", getpid());
		fclose(fp);
	}
	else
	{
		printf("pid_init(): could not open file for writing. (filename: ./pid)");
		return false;
	}
	return true;
#endif
}

int main(int argc, char** argv)
{
	if (setup_minidump_generator() == false)
		return 1;

	std::vector<spdlog::sink_ptr> sinks;
		
#if defined(_WIN32)
	sinks.emplace_back(std::make_shared<spdlog::sinks::wincolor_stdout_sink_mt>());
	sinks.emplace_back(std::make_shared<spdlog::sinks::msvc_sink_mt>());
#else
	sinks.emplace_back(std::make_shared<spdlog::sinks::ansicolor_sink>(std::make_shared<spdlog::sinks::stdout_sink_mt>()));
#endif

	sinks.emplace_back(std::make_shared<spdlog::sinks::rotating_file_sink_mt>("auth", 1024 * 1024 * 4, 4));
	auto logger = std::make_shared<spdlog::logger>("auth", sinks.begin(), sinks.end());
	spdlog::register_logger(logger);
	
	net_engine::NetServiceBase _NetService;
	std::shared_ptr<GAuthServer> _GAuthServer = std::make_shared<GAuthServer>(_NetService);

	pid_init();

	try
	{
		_GAuthServer->Init(argc, argv);
		_GAuthServer->Run();
		_GAuthServer->Shutdown();
	}
	catch(std::exception& e)
	{
		sys_log(LL_ERR, e.what());
	}
	
	_GAuthServer.reset();
	spdlog::drop_all();
	
	//sys_log(LL_ERR, "ERRROR")
	//sys_log(LL_SYS, "NOERRROR")
	
	return 0;
}