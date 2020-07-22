#include "stdafx.hpp"
#include "AuthServer.hpp"
#include "AuthLogHelper.hpp"
#include "../../libthecore/include/winminidump.h"
using namespace net_engine;

#ifdef _WIN32
	#ifdef _DEBUG
		#ifdef CI_BUILD
			#pragma comment( lib, "cryptopp_debug.lib" )
		#else
			#pragma comment( lib, "cryptopp-static_debug.lib" )
		#endif
	#else
		#ifdef CI_BUILD
			#pragma comment( lib, "cryptopp_release.lib" )
		#else
			#pragma comment( lib, "cryptopp-static_release.lib" )
		#endif
	#endif
#endif

#ifdef _IMPROVED_PACKET_ENCRYPTION_
static const auto gsc_securityLevel = ESecurityLevels::SECURITY_LEVEL_KEY_AGREEMENT;
#else
static const auto gsc_securityLevel = ESecurityLevels::SECURITY_LEVEL_XTEA;
#endif
static const auto gsc_cryptKey = DEFAULT_CRYPT_KEY;

static bool save_process_id()
{
	std::ofstream out("pid", std::ofstream::out);
	if (out)
	{
#if defined(_WIN32)
		out << GetCurrentProcessId();
#else
		out << getpid();
#endif
		out.close();
	}
	else
	{
		printf("save_process_id(): could not open file for writing. (filename: ./pid)");
		return false;
	}
	return true;
}

int main(int argc, char** argv)
{
	if (!setup_minidump_generator())
		return EXIT_FAILURE;


	std::unique_ptr <CNetLogger> engine_logger(new CNetLogger("net_engine", CUSTOM_LOG_FILENAME));
	std::unique_ptr <CAuthLogger> log_manager(new CAuthLogger("auth_logger", "auth.log"));
	std::unique_ptr <NetPacketManager> packet_manager(new NetPacketManager);


	NetServiceBase _net_service;
	auto auth_server = std::make_shared<GAuthServer>(_net_service, gsc_securityLevel, gsc_cryptKey);
	if (!auth_server || !auth_server.get())
	{
		auth_log(LL_CRI, "auth_server could NOT allocated!");
		abort();
	}

	if (!save_process_id())
	{
		auth_log(LL_CRI, "process id could NOT saved!");
		abort();
	}

	try
	{
		auth_server->Init(argc, argv);
	}
	catch (std::exception& e)
	{
		auth_log(LL_ERR, e.what());
	}
	
	auth_server.reset();
	spdlog::drop_all();
		
	return EXIT_SUCCESS;
}
