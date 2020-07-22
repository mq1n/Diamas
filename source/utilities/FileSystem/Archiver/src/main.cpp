#include <Windows.h>
#include <cstdlib>
#include <iostream>
#include <memory>
#include <algorithm>
#include <fstream>
#include <filesystem>
#include <map>

#include <xxhash.h>
#include <lz4/lz4.h>
#include <lz4/lz4hc.h>
#include <zlib/zlib.h>
#include <lzo/lzo1x.h>
#include <tbb/tbb.h>
#include <tbb/task_scheduler_init.h>

#include <rapidjson/document.h>
#include <rapidjson/reader.h>
#include <rapidjson/writer.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/error/en.h>
#include <rapidjson/filereadstream.h>
using namespace rapidjson;

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

// Beautifier: https://jsonformatter.curiousconcept.com

typedef struct _PATCH_CONTEXT
{
	std::wstring from;
	std::wstring to;
} SPatchContext;

typedef struct _ARCHIVER_CONTEXT
{
	std::string					strArchiveName;
	std::array <uint8_t, 64>	arArchiveKey;
	std::wstring				stArchiveDirectory;
	std::wstring				strVisualDirectory;
	int32_t						iType;
	int32_t						iVersion;
	bool						bCythonEnabled;
	std::vector <std::wstring>	vIgnores;
	std::vector <SPatchContext>	vPatches;
} SArchiveContext;

static inline bool FindAndReplaceString(std::wstring& str, const std::wstring& from, const std::wstring& to)
{
	size_t start_pos = str.find(from);
	if (start_pos == std::string::npos)
		return false;

	str.replace(start_pos, from.length(), to);
	return true;
}

std::string LoadConfigFile(const std::string& strConfigFilename)
{
	std::string out;

	if (std::filesystem::exists(strConfigFilename) == false)
	{
		FileSystem::Log(LL_ERR, "Config file: %s NOT exists!", strConfigFilename.c_str());
		return out;
	}

	if (std::filesystem::file_size(strConfigFilename) == 0)
	{
		FileSystem::Log(LL_ERR, "Property file: %s empty!", strConfigFilename.c_str());
		return out;
	}

	std::ifstream in(strConfigFilename.c_str(), std::ios_base::binary);
	if (!in)
	{
		FileSystem::Log(LL_ERR, "Config file can NOT open!");
		return out;
	}
	in.exceptions(std::ios_base::badbit | std::ios_base::failbit | std::ios_base::eofbit);
	
	auto strcontent = std::string(std::istreambuf_iterator<char>(in), std::istreambuf_iterator<char>());
	
	in.close();

	return strcontent;
}

bool InitializeConfigFile(FileSystemManager * fs, const std::string & strConfigFile, std::string stSpecificArchive, std::vector <std::shared_ptr <SArchiveContext>> & packs)
{
	FileSystem::Log(0, "Config file: %s Specific archive name: %s", strConfigFile.c_str(), stSpecificArchive.c_str());

	auto stBuffer = LoadConfigFile(strConfigFile);
	if (stBuffer.empty())
	{
		FileSystem::Log(LL_ERR, "Config file can NOT load!");
		return false;
	}

	std::transform(stSpecificArchive.begin(), stSpecificArchive.end(), stSpecificArchive.begin(), tolower);

	Document document;
	document.Parse(stBuffer.c_str());

	if (document.HasParseError())
	{
		FileSystem::Log(LL_ERR, "Config file parse failed! Error: %s offset: %u",
			GetParseError_En(document.GetParseError()), document.GetErrorOffset());

		document.Clear();
		return false;
	}

	if (!document.IsArray())
	{
		FileSystem::Log(LL_ERR, "Config file base is NOT array");
		document.Clear();
		return false;
	}

	auto specificArchiveFound = false;
	try
	{
		for (rapidjson::SizeType i = 0; i < document.Size(); ++i)
		{
			if (!document[i].IsObject())
			{
				FileSystem::Log(LL_ERR, "Config file group is NOT object");
				document.Clear();
				return false;
			}

			auto& pkDir = document[i]["dir"];
			if (pkDir.IsNull() || !pkDir.IsString())
			{
				FileSystem::Log(LL_ERR, "'dir' key not found in current node: %u", i);
				document.Clear();
				return false;
			}
			FileSystem::Log(LL_SYS, "dir: %s", pkDir.GetString());

			auto stDir = std::string(pkDir.GetString());
			auto bCythonEnabled = false;
			if (stDir == "source/root" || stDir == "root")
			{
				auto& pkCythonEnabled = document[i]["cython_enabled"];
				if (pkCythonEnabled.IsNull() || !pkCythonEnabled.IsBool())
				{
					FileSystem::Log(LL_ERR, "'cython_enabled' key not found in current node: %u", i);
					document.Clear();
					return false;
				}
				bCythonEnabled = pkCythonEnabled.GetBool();
				FileSystem::Log(LL_SYS, "Cython: %s", bCythonEnabled ? "enabled" : "disabled");
			}
			
			auto& pkVisualDir = document[i]["visualdir"];
			if (pkVisualDir.IsNull() || !pkVisualDir.IsString())
			{
				FileSystem::Log(LL_ERR, "'visualdir' key not found in current node: %u", i);
				document.Clear();
				return false;
			}
			FileSystem::Log(LL_SYS, "visualdir: %s", pkVisualDir.GetString());

			auto& pkFile = document[i]["file"];
			if (pkFile.IsNull() || !pkFile.IsString())
			{
				FileSystem::Log(LL_ERR, "'file' key not found in current node: %u", i);
				document.Clear();
				return false;
			}
			FileSystem::Log(LL_SYS, "file: %s", pkFile.GetString());

			auto& pkKey = document[i]["key"];
			if (pkKey.IsNull() || !pkKey.IsArray())
			{
				FileSystem::Log(LL_ERR, "'key' key not found in current node: %u", i);
				document.Clear();
				return false;
			}
			// FileSystem::Log(LL_SYS, "key: %p", pkKey.GetArray());

			auto& pkType = document[i]["type"];
			if (pkType.IsNull() || !pkType.IsArray())
			{
				FileSystem::Log(LL_ERR, "'type' key not found in current node: %u", i);
				document.Clear();
				return false;
			}
			// FileSystem::Log(LL_SYS, "Type: %p", pkType.GetArray());

			auto& pkVersion = document[i]["version"];
			if (pkVersion.IsNull() || !pkVersion.IsNumber())
			{
				FileSystem::Log(LL_ERR, "'version' key not found in current node: %u", i);
				document.Clear();
				return false;
			}
			FileSystem::Log(LL_SYS, "Version: %d", pkVersion.GetInt());

			// ---

			auto ctx = std::make_shared<SArchiveContext>();
			if (!ctx || !ctx.get())
			{
				FileSystem::Log(1, "Archiver container can NOT allocated");
				return false;
			}

			std::string dir = pkDir.GetString();
			ctx->stArchiveDirectory = std::wstring(dir.begin(), dir.end());
			if (std::filesystem::exists(ctx->stArchiveDirectory) == false)
			{
				FileSystem::Log(1, "Working directory: %ls is NOT exist", ctx->stArchiveDirectory.c_str());
				return false;
			}
			// FileSystem::Log(0, "Directory: %ls", ctx->stArchiveDirectory.c_str());

			ctx->bCythonEnabled = bCythonEnabled;

			auto stTargetArchiveName = GetNameFromPath(dir);
			if (!stSpecificArchive.empty() && stSpecificArchive != stTargetArchiveName)
			{
				FileSystem::Log(0, "Archive: %s passed but current is: %s", stSpecificArchive.c_str(), stTargetArchiveName.c_str());
				continue;
			}
			specificArchiveFound = true;

			std::string visdir = pkVisualDir.GetString();
			ctx->strVisualDirectory = std::wstring(visdir.begin(), visdir.end());
			// FileSystem::Log(0, "Visual dir: %ls", ctx->strVisualDirectory.c_str());

			std::string file = pkFile.GetString();
			ctx->strArchiveName = file;
			if (std::filesystem::exists(ctx->strArchiveName))
			{
				auto isdeleted = DeleteFileA(ctx->strArchiveName.c_str());
				if (!isdeleted)
				{
					FileSystem::Log(1, "Target file: %s is already exist", ctx->strArchiveName.c_str());
					return false;
				}
				else
				{
					FileSystem::Log(0, "Target file: %s is already exist, old file deleted.", ctx->strArchiveName.c_str());
				}
			}
			// FileSystem::Log(0, "Archive name: %s", ctx->strArchiveName.c_str());


			auto& key = pkKey.GetArray();
			for (size_t i = 0; i < key.Size(); ++i)
			{
  		 		ctx->arArchiveKey[i] = key[i].GetUint();
			}
			FileSystem::Log(0, "Key: %s", GetKeyAsString(ctx->arArchiveKey).c_str());


			auto& types = pkType.GetArray();
			auto typeField = 0;

			// Encryption
			for (size_t i = 0; i < types.Size(); ++i)
			{
				auto type = std::string(types[i].GetString());
				std::transform(type.begin(), type.end(), type.begin(), ::toupper);
	
				if (type == "ARCHIVE_FLAG_AES256")
					typeField |= ARCHIVE_FLAG_AES256;
				else if (type == "ARCHIVE_FLAG_TWOFISH")
					typeField |= ARCHIVE_FLAG_TWOFISH;
				else if (type == "ARCHIVE_FLAG_RC5")
					typeField |= ARCHIVE_FLAG_RC5;
			}
			if (!typeField)
			{
				FileSystem::Log(1, "Cryption type is not defined!!!");
			}

			// Compression
			for (size_t i = 0; i < types.Size(); ++i)
			{
				auto type = std::string(types[i].GetString());			
				std::transform(type.begin(), type.end(), type.begin(), ::toupper);

				if (type == "ARCHIVE_FLAG_LZ4")
					typeField |= ARCHIVE_FLAG_LZ4;
				else if (type == "ARCHIVE_FLAG_LZO")
					typeField |= ARCHIVE_FLAG_LZO;
				else if (type == "ARCHIVE_FLAG_ZLIB")
					typeField |= ARCHIVE_FLAG_ZLIB;														
			}
			if (!typeField)
			{
				FileSystem::Log(2, "Compression type is not defined!!! Settings saved as raw archive!");
				typeField |= ARCHIVE_FLAG_RAW;
			}
			ctx->iType = typeField;
			FileSystem::Log(0, "Type: %d", ctx->iType);

			ctx->iVersion = pkVersion.GetInt();
			if (ctx->iVersion <= 0)
			{
				FileSystem::Log(1, "Version is null");
				return false;
			}
			// FileSystem::Log(0, "Version: %d", ctx->iVersion);

			if (document[i].HasMember("ignores"))
			{
				auto& pkIgnores = document[i]["ignores"];			
				if (!pkIgnores.IsArray())
				{
					FileSystem::Log(LL_ERR, "'ignores' key not found in current node: %u", i);
					document.Clear();
					return false;
				}

				auto ignores = std::vector <std::wstring>();

				for (const auto& ignore : pkIgnores.GetArray())
				{
					std::string curr = ignore.GetString();
					auto wcurr = std::wstring(curr.begin(), curr.end());
					ignores.emplace_back(wcurr.c_str());
				}

				ctx->vIgnores = ignores;
			}
			if (document[i].HasMember("patches"))
			{
				auto& pkPatches = document[i]["patches"];
				if (!pkPatches.IsObject())
				{
					FileSystem::Log(LL_ERR, "'patches' key not found in current node: %u", i);
					document.Clear();
					return false;
				}

				auto patches = std::vector <SPatchContext>();

				for (auto childNode = pkPatches.MemberBegin(); childNode != pkPatches.MemberEnd(); ++childNode)
				{
					SPatchContext patch{};

					std::string stkey = childNode->name.GetString();
					patch.from = std::wstring(stkey.begin(), stkey.end());

					std::string stval = childNode->value.GetString();
					patch.to = std::wstring(stval.begin(), stval.end());

					patches.emplace_back(patch);
				}

				ctx->vPatches = patches;
			}

			FileSystem::Log(0, "%s: %ls(%ls)", ctx->strArchiveName.c_str(), ctx->stArchiveDirectory.c_str(), ctx->strVisualDirectory.c_str());
			packs.emplace_back(ctx);
		}
	}
	catch (std::exception & e)
	{
		FileSystem::Log(1, "Exception: handled: %s", e.what());
		return false;
	}
	catch (...)
	{
		FileSystem::Log(1, "Unhandled Exception!");
		return false;
	}

	if (packs.empty())
	{
		FileSystem::Log(1, "Pack work list is empty!");
		return false;
	}

	if (!stSpecificArchive.empty() && !specificArchiveFound)
	{
		FileSystem::Log(1, "Target archive: %s not found in config!", stSpecificArchive.c_str());
		return false;		
	}

	FileSystem::Log(0, "Pack work list: %u", packs.size());

	document.Clear();
	return true;
}


bool ProcessArchiveFile(FileSystemManager * fs, const std::shared_ptr <SArchiveContext> & pack)
{
	FileSystem::Log(0, "Curr archive %s: %ls", pack->strArchiveName.c_str(), pack->stArchiveDirectory.c_str());

	CFile file;
	if (!file.Create(pack->strArchiveName, FILEMODE_WRITE, false))
	{
		FileSystem::Log(1, "File can NOT created");
		return false;
	}
	file.Close();

	auto archiveMaker = std::make_unique<CArchiveMaker>();
	if (!archiveMaker || !archiveMaker.get())
	{
		FileSystem::Log(1, "Archive can NOT allocated");
		return false;
	}

	if (!archiveMaker->Create(pack->strArchiveName, pack->arArchiveKey))
	{
		FileSystem::Log(1, "Archive can NOT created");
		return false;
	}

	auto workingdirectory = GetWorkingDirectory();
	for (const auto& entry : std::filesystem::recursive_directory_iterator(pack->stArchiveDirectory))
	{
		if (entry.is_directory())
			continue;

		auto target_file = entry.path();
//		FileSystem::Log(0, "%ls", target_file.c_str());

		/*
		while (std::filesystem::is_symlink(target_file))
		{
			FileSystem::Log(0, "Symlink detected: %ls", target_file.c_str());

			target_file = std::filesystem::read_symlink(target_file);

			FileSystem::Log(0, "Symlink redirected to: %ls", target_file.c_str());
		}
		*/

		auto extension = target_file.extension().wstring();
		auto namewithoutpath = target_file.wstring();
		auto wstrdirectory = std::wstring(pack->stArchiveDirectory.begin(), pack->stArchiveDirectory.end());
		std::size_t pos = namewithoutpath.find(wstrdirectory + L"\\");
		if (pos != std::string::npos)
		{
			namewithoutpath.erase(pos, pack->stArchiveDirectory.size() + 1);
		}
		for (const auto& patch : pack->vPatches)
		{
			if (FindAndReplaceString(namewithoutpath, patch.from, patch.to))
            {
                FileSystem::Log(0, "Patch: %ls->%ls applied to: %ls", patch.from.c_str(), patch.to.c_str(), namewithoutpath.c_str());
            }
		}

		FileSystem::Log(0, "ProcessArchiveFile-AppendFile: %ls", namewithoutpath.c_str());

		if (pack->strVisualDirectory.length() && pack->strVisualDirectory[pack->strVisualDirectory.length() - 1] != L'\\' && pack->strVisualDirectory[pack->strVisualDirectory.length() - 1] != L'/')
		{
			for (size_t i = 0; i < pack->strVisualDirectory.length(); ++i)
			{
				if (pack->strVisualDirectory[i] == L'\\')
				{
					pack->strVisualDirectory[i] = L'/';
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

		auto skipfile = false;

		if (pack->bCythonEnabled &&
			namewithoutpath.find(L'/') == std::wstring::npos &&
			extension == L".py")
		{
			skipfile = true;
		}

		for (const auto & ignore : pack->vIgnores)
		{
			if (WildcardMatch(namewithoutpath.c_str(), ignore.c_str()))
			{
				skipfile = true;
				break;
			}
		}
		if (skipfile)
		{
			FileSystem::Log(0, "Content skipped: %ls", target_file.c_str());
			continue;
		}

		FileSystem::Log(0, "'%ls'->'%ls'", pack->strVisualDirectory.c_str(), namewithoutpath.c_str());

		if (entry.file_size() == 0)
		{
			FileSystem::Log(1, "Entry file is null");
			continue;
		}

		if (!pack->strVisualDirectory.empty())
			namewithoutpath = pack->strVisualDirectory + namewithoutpath;

		if (!archiveMaker->Add(namewithoutpath, target_file.wstring(), pack->iType))
		{
			FileSystem::Log(1, "Entry file can NOT writed");
			return false;
		}
	}

	if (!archiveMaker->Save())
	{
		FileSystem::Log(1, "Archive can not save");
		return false;
	}
	return true;
}

struct SPackProcessor
{
	std::vector <std::shared_ptr <SArchiveContext>>& _archives;
	FileSystemManager* _fs;
	SPackProcessor(FileSystemManager *& fs, std::vector <std::shared_ptr <SArchiveContext>>& archives) :
		_fs(fs), _archives(archives)
	{
	}

	void operator()(const tbb::blocked_range <uint32_t>& range) const
	{
		for (size_t i = range.begin(); i != range.end(); ++i)
		{
			if (ProcessArchiveFile(_fs, _archives.at(i)) == false)
			{
				FileSystem::Log(1, "ProcessArchiveFile fail! Index: %u", i);
				abort();
			}
		}
	}
};

int32_t main(int32_t argc, char* argv[])
{
	auto packs = std::vector<std::shared_ptr<SArchiveContext>>();

	auto fs = new FileSystemManager();
	if (!fs || !fs->InitializeFSManager(
#ifdef ENABLE_LAYER2_FILE_ENCRYPTION
		FILE_FLAG_XTEA
#endif
	))
	{
		printf("File System initilization fail!\n");
		return EXIT_FAILURE;
	}
	FileSystem::Log(0, "Diamas_Client archiver started!");
	Sleep(1000);

	const auto configFile = std::string("config.json");
	auto secondArg = std::string(configFile);
	auto thirdArg = std::string("");
	if (argc >= 2)
		secondArg = argv[1];
	if (argc >= 3)
		thirdArg = argv[2];
	
	FileSystem::Log(0, "Arg count: %d Data: '%s'-'%s'-'%s'", argc, argv[0], secondArg.c_str(), thirdArg.c_str());
	if (!std::count(secondArg.begin(), secondArg.end(), '.') && thirdArg.empty()) // if second arg has not contain dot(which is mean it's seems not like a file) swap args
	{
		std::swap(secondArg, thirdArg); // second arg now empty, third arg swapped specific archive name
		secondArg = configFile; // second arg reverted to default config name
	}

checkIO:
	// also check console input for confirm specific target
	if (thirdArg.empty())
	{
		printf("Target archive: ");
		std::getline(std::cin, thirdArg);

		if (thirdArg == "break" || thirdArg.empty())
			goto doExit;
	}
	if (thirdArg == "all")
		thirdArg.clear();

	if (!InitializeConfigFile(fs, secondArg, thirdArg, packs))
	{
		FileSystem::Log(LL_ERR, "InitializeConfigFile failed");
		return EXIT_FAILURE;
	}

	// ------------------------------------------------------

	try
	{
		tbb::task_scheduler_init init(tbb::task_scheduler_init::default_num_threads());

		tbb::parallel_for(tbb::blocked_range<uint32_t>(0, packs.size()), SPackProcessor(fs, packs));
	}
	catch (std::exception & e)
	{
		FileSystem::Log(1, "Exception: handled: %s", e.what());
		return false;
	}
	catch (...)
	{
		FileSystem::Log(1, "Unhandled Exception!");
		return false;
	}

	// ------------------------------------------------------

	for (const auto& pack : packs)
	{
		auto archive = fs->AddArchive(pack->strArchiveName, pack->arArchiveKey);
		FileSystem::Log(0, "archive: %s %d", pack->strArchiveName.c_str(), archive);

		if (archive)
		{
			const std::string& log_filename = pack->strArchiveName + ".log";
			if (std::filesystem::exists(log_filename))
				std::filesystem::remove(log_filename);

			std::ofstream log_file(log_filename, std::ofstream::out | std::ofstream::app);
			if (log_file.is_open())
			{
				fs->EnumerateFiles(pack->strArchiveName, [&fs, &log_file](const CFileName& wstArchiveName, const FSFileInformation& pcFileInformations, void* pvUserContext) -> bool {
					char fileinfo[512];
#ifdef SHOW_FILE_NAMES
					sprintf_s(fileinfo, "%lu(%ls): %lu", pcFileInformations.filenameHash, pcFileInformations.filename, pcFileInformations.fileHash);
#else
					sprintf_s(fileinfo, "%lu: %lu", pcFileInformations.filenameHash, pcFileInformations.fileHash);
#endif
					log_file << fileinfo << std::endl;
					// FileSystem::Log(0, "File: %s", fileinfo);

					return true;
				}, nullptr);

				log_file.close();
			}
		}
	}

	FileSystem::Log(0, "File system archive processing completed!");

	thirdArg.clear();
	fs->CloseArchives();
	packs.clear();
	goto checkIO;

doExit:
	fs->FinalizeFSManager();
	return EXIT_SUCCESS;
}
