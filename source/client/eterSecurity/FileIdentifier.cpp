#include "StdAfx.h"
#include "AnticheatManager.h"
#include "CheatQueueManager.h"
#include "dirent_win.h"
#include <xorstr.hpp>

inline static int32_t getdir(const std::string & dir, std::vector <std::string> & files)
{
	DIR * dp;
	if ((dp = opendir(dir.c_str())) == nullptr)
	{
		return errno;
	}

	struct dirent *dirp;
	while ((dirp = readdir(dp)) != nullptr)
	{
		auto file = std::string(dirp->d_name);
		if (file != "." && file != "..")
			files.push_back(std::string(dirp->d_name));
	}

	closedir(dp);
	return 0;
}


void CAnticheatManager::CheckMainFolderFiles()
{
	auto dir = std::string(".");
	auto files = std::vector<std::string>();

	getdir(dir, files);

	for (size_t i = 0; i < files.size(); i++) 
	{
		std::string file = files[i];
		std::transform(file.begin(), file.end(), file.begin(), tolower);

		if (file.substr(file.find_last_of(".") + 1) == xorstr("mix").crypt_get() ||
			file.substr(file.find_last_of(".") + 1) == xorstr("flt").crypt_get() ||
			file.substr(file.find_last_of(".") + 1) == xorstr("asi").crypt_get() ||
			file.substr(file.find_last_of(".") + 1) == xorstr("m3d").crypt_get() ||
			file.substr(file.find_last_of(".") + 1) == xorstr("def").crypt_get() ||
			file.substr(file.find_last_of(".") + 1) == xorstr("py").crypt_get())
		{
			TraceError(xorstr("Unallowed file found on main folder! File: %s").crypt_get(), file.c_str());
			abort();
		}

		if (file == xorstr("mss32.dll").crypt_get() && GetFileMd5(file) != "6400e224b8b44ece59a992e6d8233719")
		{
			TraceError(xorstr("mss32.dll file is corrupted! Please delete it and restart game").crypt_get());
			abort();
		}

		if (file == xorstr("devil.dll").crypt_get() && GetFileMd5(file) != "26eec5cc3d26cb38c93de01a3eb84cff")
		{
			TraceError(xorstr("devil.dll file is corrupted! Please delete it and restart game").crypt_get());
			abort();
		}
	}
}

void CAnticheatManager::CheckLibFolderForPythonLibs()
{
	if (IsDirExist(xorstr("stdlib").crypt_get()))
	{
		TraceError(xorstr("Please delete stdlib folder and restart game.").crypt_get());
		abort();
	}
}

void CAnticheatManager::CheckMilesFolderForMilesPlugins()
{
	auto dir = std::string(xorstr("miles").crypt_get());
	auto files = std::vector<std::string>();

	getdir(dir, files);

	static std::map <std::string /* strFileName */, std::string /* strMd5 */> mapKnownFiles =
	{
		{ xorstr("mssa3d.m3d").crypt_get(),	xorstr("e089ce52b0617a6530069f22e0bdba2a").crypt_get() },
		{ xorstr("mssds3d.m3d").crypt_get(),	xorstr("85267776d45dbf5475c7d9882f08117c").crypt_get() },
		{ xorstr("mssdsp.flt").crypt_get(),	xorstr("cb71b1791009eca618e9b1ad4baa4fa9").crypt_get() },
		{ xorstr("mssdx7.m3d").crypt_get(),	xorstr("2727e2671482a55b2f1f16aa88d2780f").crypt_get() },
		{ xorstr("msseax.m3d").crypt_get(),	xorstr("788bd950efe89fa5166292bd6729fa62").crypt_get() },
		{ xorstr("mssmp3.asi").crypt_get(),	xorstr("189576dfe55af3b70db7e3e2312cd0fd").crypt_get() },
		{ xorstr("mssrsx.m3d").crypt_get(),	xorstr("7fae15b559eb91f491a5f75cfa103cd4").crypt_get() },
		{ xorstr("msssoft.m3d").crypt_get(),	xorstr("bdc9ad58ade17dbd939522eee447416f").crypt_get() },
		{ xorstr("mssvoice.asi").crypt_get(),	xorstr("3d5342edebe722748ace78c930f4d8a5").crypt_get() },
		{ xorstr("mss32.dll").crypt_get(),		xorstr("6400e224b8b44ece59a992e6d8233719").crypt_get() }
	};

	if (files.size() > mapKnownFiles.size())
	{
		TraceError(xorstr("Unknown file detected on miles folder! Please delete miles folder and restart game.").crypt_get());
		abort();
	}

	for (const auto & strCurrFolderFile : files)
	{
		auto strCurrFileLower = strCurrFolderFile;
		std::transform(strCurrFileLower.begin(), strCurrFileLower.end(), strCurrFileLower.begin(), tolower);

		auto it = mapKnownFiles.find(strCurrFileLower);
		if (it == mapKnownFiles.end())
		{
			TraceError(xorstr("Unknown file detected on miles folder! File: %s").crypt_get(), strCurrFolderFile.c_str());
			abort();
		}

		std::string szPath = xorstr("miles/").crypt_get();
		auto strCurrentMd5 = GetFileMd5(szPath + strCurrFileLower);
		auto strCorrectMd5 = it->second;
		if (strCurrentMd5 != strCorrectMd5)
		{
			TraceError(xorstr("Corrupted file detected on miles folder! File: %s").crypt_get(), strCurrFolderFile.c_str());
			abort();
		}
	}
}

void CAnticheatManager::CheckYmirFolder()
{
	if (IsDirExist(xorstr("d:/ymir work").crypt_get()) || IsDirExist(xorstr("d:\\ymir work").crypt_get()))
	{
		TraceError(xorstr("Unallowed folder: 'd:/ymir work' detected! Please delete it and restart game").crypt_get());
		abort();
	}
}

