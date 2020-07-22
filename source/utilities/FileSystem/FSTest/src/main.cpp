#include <Windows.h>
#include <cstdlib>
#include <iostream>
#include <memory>
#include <algorithm>
#include <fstream>
#include <filesystem>

#include <xxhash.h>
#include <lz4/lz4.h>
#include <lz4/lz4hc.h>
#include <zlib/zlib.h>
#include <lzo/lzo1x.h>
#include <tbb/tbb.h>
#include <tbb/task_scheduler_init.h>

#include "../../FSLib/include/FileSystemIncl.hpp"

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

#ifdef _DEBUG
	#pragma comment( lib, "tbb_debug.lib" )
#else
	#pragma comment( lib, "tbb.lib" )
#endif

#ifdef SHOW_FILE_NAMES

void replaceAll(std::wstring& s, const std::wstring& search, const std::wstring& replace)
{
	for (size_t pos = 0; ; pos += replace.length())
	{
		pos = s.find(search, pos);
		if (pos == std::wstring::npos)
			break;

		s.erase(pos, search.length());
		s.insert(pos, replace);
	}
}
std::vector <std::wstring> DirectoryList(const std::wstring& input, const std::wstring& delim = L"\\")
{
	auto list = std::vector<std::wstring>();

	size_t start = 0;
	auto end = input.find(delim);
	while (end != std::wstring::npos)
	{
		list.emplace_back(input.substr(0, end));
		start = end + delim.length();
		end = input.find(delim, start);
	}

	return list;
}

void CheckSpecificFileIntegrity(FileSystemManager* fs, const CFileName& wstArchiveName,
	const FSFileInformation& file, const std::wstring& targetdir, const std::wstring& realdir)
{
    Log(0, "PACKED | %ls - %lu - %u -- Raw: %u Compressed: %u Crypted(Final): %u",
        file.filename, file.fileHash, file.flags, file.rawSize, file.compressedSize, file.cryptedSize);

	auto target = targetdir + L"\\" + file.filename;
	replaceAll(target, L"/", L"\\");
	auto vSubFolderList = DirectoryList(target);
	if (!vSubFolderList.empty())
	{
		for (const auto& stSubFolder : vSubFolderList)
			CreateDirectoryW(stSubFolder.c_str(), nullptr);
	}

	auto realpath = realdir + L"/" + file.filename;

	CFile fp;
	if (!fs->OpenFile(file.filename, fp, true))
    {
        Log(0, "Virtual file: %ls can NOT open!", file.filename);
        return;
    }

	auto fileSize = static_cast<uint32_t>(fp.GetSize());
	std::vector <uint8_t> vBuffer(fileSize);
	memcpy(&vBuffer[0], fp.GetData(), fileSize);

	CFile unpackedfile;
	if (!unpackedfile.Create(target, FILEMODE_WRITE, false))
	{
		Log(0, "Unpacked file: %ls can NOT created!", target.c_str());
		unpackedfile.Close();
		return;
	}

#ifdef ENABLE_LAYER2_FILE_ENCRYPTION
	if (IsLayer2File(file.filename))
	{
		auto decryptedBuffer = FileSystemManager::Instance().DecryptLayer2Protection(vBuffer.data(), fileSize);
		if (decryptedBuffer.empty())
		{
			Log(0, "Unpacked file: %ls layer2 protection can NOT decrypted!", target.c_str());
			unpackedfile.Close();
			return;			
		}

		vBuffer.resize(decryptedBuffer.size());
		memcpy(&vBuffer[0], decryptedBuffer.data(), decryptedBuffer.size());
	}
#endif

	auto writecount = unpackedfile.Write(vBuffer.data(), fileSize);
	if (writecount != fileSize)
	{
		Log(0, "Unpacked content can not writed %u/%u", writecount, fileSize);
		unpackedfile.Close();
		return;
	}
	unpackedfile.Close();
}

bool CreateTestArchive(FileSystemManager* fs, const std::wstring& strArchiveName, const std::array <uint8_t, ARCHIVE_KEY_LENGTH>& arArchiveKey, 
	const std::wstring& stArchiveDirectory, const std::wstring& stVisualDirectory, uint32_t uVersion, uint32_t uFlag)
{
	Log(0, "Curr archive %ls: %ls", strArchiveName.c_str(), stArchiveDirectory.c_str());

	auto archiveMaker = new CArchiveMaker();
	if (!archiveMaker)
	{
		Log(1, "FS Archive maker can NOT allocated");
		return false;
	}

//	fs->SetArchiveKey(strArchiveName, pArchiveKey);

	if (!archiveMaker->Create(strArchiveName, arArchiveKey))
	{
		Log(1, "Archive can NOT created");
		return false;
	}

	uint32_t counter = 0;

	auto workingdirectory = GetWorkingDirectory();
	for (const auto& entry : std::filesystem::recursive_directory_iterator(stArchiveDirectory))
	{
		std::wstring wstrEntryFile = entry.path().c_str();

		Log(0, "%ls", wstrEntryFile.c_str());

		if (entry.is_directory())
			continue;

		auto namewithoutpath = wstrEntryFile;
		std::size_t pos = namewithoutpath.find(stArchiveDirectory + L"\\");
		if (pos != std::wstring::npos)
		{
			namewithoutpath.erase(pos, stArchiveDirectory.size() + 1);
		}

//		Log(0, "%ls", namewithoutpath.c_str());

		auto visualdirectory = stVisualDirectory;
		if (visualdirectory.length() && visualdirectory[visualdirectory.length() - 1] != L'\\' && visualdirectory[visualdirectory.length() - 1] != L'/')
		{
			for (size_t i = 0; i < visualdirectory.length(); ++i)
			{
				if (visualdirectory[i] == L'\\')
				{
					visualdirectory[i] = L'/';
				}
			}
		}

		for (size_t i = 0; i < namewithoutpath.length(); ++i)
		{
			if (namewithoutpath[i] == L'\\')
			{
				namewithoutpath[i] = L'/';
			}
		}

//		Log(0, "'%ls'->'%ls'", visualdirectory.c_str(), namewithoutpath.c_str());

		if (visualdirectory.empty() == false)
			namewithoutpath = visualdirectory + namewithoutpath;

		if (!archiveMaker->Add(namewithoutpath, wstrEntryFile, uFlag))
		{
			Log(1, "Entry file can NOT writed");
			return false;
		}
		counter++;
	}
	
	Log(0, "Real size: %u", counter);
//	Log(0, "Archive size: %u", archive->EnumerateFiles().size());

	if (!archiveMaker->Save())
	{
		Log(1, "Archive can not save");
		return false;
	}
	return true;
}

#endif

int32_t main(int32_t argc, char* argv[])
{
#ifdef SHOW_FILE_NAMES
	auto fs = new FileSystemManager();
	if (!fs || !fs->InitializeFSManager(
#ifdef ENABLE_LAYER2_FILE_ENCRYPTION
		FILE_FLAG_XTEA
#endif
	))
	{
		printf("File system Initilization fail!\n");
		return EXIT_FAILURE;
	}

	if (argc == 2 && strstr(argv[1], "property_test"))
	{
		auto stContent = LoadPropertyFile("..\\MakePack\\root\\PropertyList.json");
		if (!stContent.empty())
		{
			TPropertyContainer vPropertyList;
			if (LoadPropertyList(stContent, vPropertyList))
				Log(0, "property load done!");
			else
				Log(1, "load list fail!");
		}
		else
		{
			Log(1, "load file fail!");
		}
		fs->FinalizeFSManager();
		std::system("PAUSE");
		return EXIT_SUCCESS;
	}

	auto targetpack = L"test.cpf";
	auto targetdir = L"test";
	auto targetunpackdir = L"test_unpacked";
	// auto key = ConvertKeyFromAscii("0000000000000000000000000000000000000000000000000000000000000001");

//	if (CreateTestArchive(fs, targetpack, DEFAULT_ARCHIVE_KEY, targetdir, L"", 1, ARCHIVE_FLAG_LZO | ARCHIVE_FLAG_RC5) == false)
	if (CreateTestArchive(fs, targetpack, DEFAULT_ARCHIVE_KEY, targetdir, L"", 1, ARCHIVE_FLAG_LZ4 | ARCHIVE_FLAG_TWOFISH) == false)
//	if (CreateTestArchive(fs, targetpack, DEFAULT_ARCHIVE_KEY, targetdir, L"", 1, ARCHIVE_FLAG_ZLIB | ARCHIVE_FLAG_AES256) == false)
//	if (CreateTestArchive(fs, targetpack, DEFAULT_ARCHIVE_KEY, targetdir, L"", 1, ARCHIVE_FLAG_AES256) == false)
//	if (CreateTestArchive(fs, targetpack, DEFAULT_ARCHIVE_KEY, targetdir, L"", 1, ARCHIVE_FLAG_ZLIB) == false)
//	if (CreateTestArchive(fs, targetpack, DEFAULT_ARCHIVE_KEY, targetdir, L"", 1, 0) == false)
	{
		Log(1, "Target archive can NOT created!");
		return EXIT_FAILURE;
	}

#if 0
	fs->RegisterArchive(targetpack);
    fs->SetArchiveKey(targetpack, DEFAULT_ARCHIVE_KEY.data());
    auto archive = fs->LoadArchive(targetpack);
    if (!archive)
    {
        Log(1, "Target archive can not load!");
		return EXIT_FAILURE;
    }
#endif

	if (!fs->AddArchive(targetpack, DEFAULT_ARCHIVE_KEY))
	{
		Log(1, "Target archive can not add!");
		return EXIT_FAILURE;
	}

	auto archive = fs->GetArchive(targetpack);
	if (!archive)
	{
		Log(1, "Target archive can not get!");
		return EXIT_FAILURE;
	}

	std::filesystem::remove_all(targetunpackdir);
	CreateDirectoryW(targetunpackdir, nullptr);

	fs->EnumerateFiles(targetpack, [&fs, &targetunpackdir, &targetdir](const CFileName& wstArchiveName, const FSFileInformation& pcFileInformations, void* pvUserContext) -> bool {
		CheckSpecificFileIntegrity(fs, wstArchiveName, pcFileInformations, targetunpackdir, targetdir);
		return true;
	}, nullptr);

	Log(0, "File system test completed!");

	fs->FinalizeFSManager();
#endif

    std::system("PAUSE");
	return EXIT_SUCCESS;
}
