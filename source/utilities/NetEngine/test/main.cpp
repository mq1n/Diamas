#include "../include/NetEngine.hpp"
#include "NetServerManager.hpp"
#include "NetClientManager.hpp"
#include "config.hpp"
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

std::shared_ptr <CNetworkServerManager> netServer;
std::shared_ptr <CNetworkClientManager> netClient;

/*
		SECURITY_LEVEL_NONE = 0,  // No handshake
		SECURITY_LEVEL_BASIC = 1, // Basic handshake (No keys or Diffie-Hellman)
		SECURITY_LEVEL_XTEA = 2,  // Key/Pong keys
		SECURITY_LEVEL_KEY_AGREEMENT = 3 // Diffie-Hellman Key agreement
*/
static const auto gsc_securityLevel = ESecurityLevels::SECURITY_LEVEL_KEY_AGREEMENT;
static const auto gsc_cryptKey = DEFAULT_CRYPT_KEY;

bool InitializeServer()
{
	SetConsoleTitleA("NetEngineTest Server");

	NetServiceBase _net_service;
	netServer = std::make_shared<CNetworkServerManager>(_net_service, ip_address, port, gsc_securityLevel, gsc_cryptKey);
	if (!netServer || !netServer.get()) 
	{
		printf("netServer can NOT initialized! Last error: %u\n", GetLastError());
		abort();
	}

	return netServer->Initialize();
}
bool InitializeClient()
{
	SetConsoleTitleA("NetEngineTest Client");

	NetServiceBase _net_service;
	netClient = std::make_shared<CNetworkClientManager>(_net_service, gsc_securityLevel, gsc_cryptKey);
	if (!netClient || !netClient.get()) 
	{
		printf("netClient can NOT initialized! Last error: %u\n", GetLastError());
		abort();
	}

	try
	{
		if (!netClient->Connect(ip_address, port))
		{
			printf("netClient can NOT connected! Last error: %u\n", GetLastError());
			abort();
		}

		_net_service.Run();
	}
	catch (std::exception& e)
	{
		printf("Exception handled: %s\n", e.what());
		abort();
	}
	catch (uint32_t dwNumber)
	{
		printf("Exception handled: %p\n", (void*)dwNumber);
		abort();
	}
	catch (...)
	{
		printf( "Unhandled exception\n");
		abort();
	}

	return true;
}

int main(int argc, char* argv[])
{
	if (argc != 2)
	{
		printf("Usage: %s <type>\n", argv[0]);
		return EXIT_FAILURE;
	}
	auto type = std::string(argv[1]);

	auto logger = std::make_unique<CNetLogger>("NetEngineLogger", CUSTOM_LOG_FILENAME);
	if (!logger || !logger.get())
	{
		printf("Logger cannot initialized\n");
		return EXIT_FAILURE;		
	}

	std::unique_ptr <NetPacketManager> packet_manager(new NetPacketManager);

	if (type == "client")
	{
		if (!InitializeClient())
		{
			printf("Client cannot initialized\n");
			return EXIT_FAILURE;
		}
	}
	else if (type == "server")
	{
		if (!InitializeServer())
		{
			printf("Server cannot initialized\n");
			return EXIT_FAILURE;
		}
	}
	else
	{
		printf("Unknown type: %s\n", type.c_str());
		return EXIT_FAILURE;		
	}

	std::cin.get();
	return 0;
}
