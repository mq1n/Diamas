#include "../include/NetEngine.hpp"
#include "NetServerManager.hpp"
#include "NetClientManager.hpp"
#include "config.hpp"
using namespace net_engine;

std::shared_ptr <CNetworkServerManager> netServer;
std::shared_ptr <CNetworkClientManager> netClient;

static const auto gsc_securityLevel = ESecurityLevels::SECURITY_LEVEL_NONE;
static const auto gsc_cryptKey = DEFAULT_CRYPT_KEY;

bool InitializeServer()
{
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
	catch (DWORD dwNumber)
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

		printf("Enter message: ");

		std::string msg;
		while (true)
		{
			std::getline(std::cin, msg);
			if (msg.empty() || msg == "x")
				break;
			netServer->BroadcastMessage(msg);
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
