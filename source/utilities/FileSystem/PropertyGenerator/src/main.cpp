#include <windows.h>
#include <iostream>
#include <vector>
#include "../../FSLib/include/FileSystemIncl.hpp"

#ifdef _DEBUG
	#pragma comment( lib, "cryptopp-static_debug.lib" )
#else
	#pragma comment( lib, "cryptopp-static.lib" )
#endif

int32_t main(int32_t argc, char* argv[])
{
	if (argc != 3)
	{
		printf("Usage: %s <property_folder> <list_output_file>\n", argv[0]);
		return EXIT_FAILURE;
	}
	auto szPropertyFolder = argv[1];
	auto szListOutputFile = argv[2];

	auto fs = new FileSystemManager();
	if (!fs || !fs->InitializeFSManager(
#ifdef ENABLE_LAYER2_FILE_ENCRYPTION
		FILE_FLAG_XTEA
#endif
	))
	{
		printf("File system initilization fail!\n");
		return EXIT_FAILURE;
	}
	Log(0, "Diamas_Client Property generator started!");
	Sleep(2000);

	DeleteFileA(szListOutputFile);

	try
	{
		if (GeneratePropertyList(szPropertyFolder, szListOutputFile) == false)
		{
			printf("Property list can NOT created!\n");
			return EXIT_FAILURE;
		}
	}
	catch (std::exception & e)
	{
		printf("Exception handled: %s\n", e.what());
		return EXIT_FAILURE;
	}
	catch (...)
	{
		printf("Unhandled exception\n");
		return EXIT_FAILURE;
	}

	printf("Property list succesfully created!\n");

	fs->FinalizeFSManager();
	std::system("PAUSE");
	return EXIT_SUCCESS;
};
