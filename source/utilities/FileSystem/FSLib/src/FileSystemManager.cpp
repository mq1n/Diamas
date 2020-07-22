#include "../include/FileSystemManager.hpp"
#include "../include/LogHelper.hpp"
#include "../include/Constants.hpp"
#include "../include/Keys.hpp"
#include "../include/FileSystem.hpp"
#include "../include/File.hpp"
#include "../include/Utils.hpp"

#include <ppl.h>
#include <lzo/lzo2a.h>
#include <filesystem>
#include <utility>

#include <cryptopp/modes.h>
#include <cryptopp/tea.h>
#include <cryptopp/blowfish.h>

#ifdef _DEBUG
	#pragma comment( lib, "cryptopp-static_debug.lib" )
	#pragma comment( lib, "xxhash_debug.lib" )
	#pragma comment( lib, "lzo2_debug.lib" )
	#pragma comment( lib, "lz4_debug.lib" )
	#pragma comment( lib, "zlib_debug.lib" )
#else
	#pragma comment( lib, "cryptopp-static_release.lib" )
	#pragma comment( lib, "xxhash.lib" )
	#pragma comment( lib, "lzo2.lib" )
	#pragma comment( lib, "lz4.lib" )
	#pragma comment( lib, "zlib.lib" )
#endif

namespace FileSystem
{
	static FileSystemManager* gs_pFSInstance = nullptr;

	FileSystemManager* FileSystemManager::InstancePtr()
	{
		return gs_pFSInstance;
	}
	FileSystemManager& FileSystemManager::Instance()
	{
		assert(gs_pFSInstance);
		return *gs_pFSInstance;
	}

	FileSystemManager::FileSystemManager()
	{
		assert(!gs_pFSInstance);

		gs_pFSInstance = this;
		m_lzoWorkMem = nullptr;
	}
	FileSystemManager::~FileSystemManager()
	{
		assert(gs_pFSInstance == this);

		gs_pFSInstance = nullptr;
		if (m_lzoWorkMem)
		{
			free(m_lzoWorkMem);
			m_lzoWorkMem = nullptr;
		}
	}

	bool FileSystemManager::InitializeFSManager(
#ifdef ENABLE_LAYER2_FILE_ENCRYPTION
		uint8_t layer2Type
#endif
	)
	{
		std::lock_guard <std::recursive_mutex> __lock(m_fsMutex);

		assert(!gs_pFSLogInstance);

		CreateDirectoryA("logs", nullptr);
		gs_pFSLogInstance = new CLog("FSLogger", CUSTOM_LOG_FILENAME);

		if (!gs_pFSLogInstance)
		{
			Logf(CUSTOM_LOG_ERROR_FILENAME, "FileSystemManager::InitializeFSManager: File system log manager inilization fail!");
			return false;
		}

		auto lzoret = lzo_init();
		if (lzoret != LZO_E_OK)
		{
			FS_LOG(LL_ERR, "LZO can not initialized. Error: %d", lzoret);
			return false;
		}

		m_lzoWorkMem = malloc(LZO2A_999_MEM_COMPRESS);
		if (!m_lzoWorkMem)
		{
			FS_LOG(LL_ERR, "LZO work memory can not allocated");
			return false;		
		}

#ifdef ENABLE_LAYER2_FILE_ENCRYPTION
		m_layer2Type = layer2Type;
#endif

		DEBUG_LOG(LL_SYS, "Build: %s", __TIMESTAMP__);
		return true;
	}
	bool FileSystemManager::FinalizeFSManager() const
	{
		std::lock_guard <std::recursive_mutex> __lock(m_fsMutex);

		if (gs_pFSLogInstance)
		{
			delete gs_pFSLogInstance;
			gs_pFSLogInstance = nullptr;
		}
		return true;
	}

	void FileSystemManager::CloseArchives()
	{
		std::lock_guard <std::recursive_mutex> __lock(m_fsMutex);

		for (const auto& archive : m_archives)
		{
			archive->GetArchiveFileStream().Close();
		}
	}
	bool FileSystemManager::AddArchive(const CFileName& filename, const std::array <uint8_t, ARCHIVE_KEY_LENGTH>& key)
	{
		std::lock_guard <std::recursive_mutex> __lock(m_fsMutex);

		std::unique_ptr <CArchive> pack(new CArchive());

		if (!pack->Create(filename, key, m_files))
		{
			FS_LOG(LL_ERR, "Failed to load Archive %ls", filename.GetPathW().c_str());
			return false;
		}

		m_archives.emplace_back(std::move(pack));
		return true;
	}

	CArchive* FileSystemManager::GetArchive(const CFileName& archivename)
	{
		std::lock_guard <std::recursive_mutex> __lock(m_fsMutex);

		for (const auto& archive : m_archives)
		{
			if (archive && archive->GetArchiveFileStream().GetFileName() == archivename.GetPathW())
			{
				return archive.get();
			}
		}
		return nullptr;
	}

	TArchiveKey FileSystemManager::GetArchiveKey(const CFileName& archivename)
	{
		std::lock_guard <std::recursive_mutex> __lock(m_fsMutex);

		for (const auto& key : gs_vecFileAndKeys)
		{
			if (std::get<KEY_CONTAINER_FILENAME>(key).GetHash() == archivename.GetHash())
			{
				return std::get<KEY_CONTAINER_KEY>(key);
			}
		}

		DEBUG_LOG(LL_ERR, "Archive: %s key does not exist!", archivename.GetPathA().c_str());
		return DEFAULT_ARCHIVE_KEY;
	}

	bool FileSystemManager::EnumerateFiles(const CFileName& archivename, TEnumFiles pfnEnumFiles, LPVOID pvUserContext)
	{
		std::lock_guard <std::recursive_mutex> __lock(m_fsMutex);

		if (!pfnEnumFiles)
			return false;

		for (const auto& [hash, archivectx] : m_files)
		{
			if (archivectx.first && archivectx.first->GetArchiveFileStream().GetFileName() == archivename.GetPathW())
			{
				if (pfnEnumFiles(archivectx.first->GetArchiveFileStream().GetFileName(), archivectx.second, pvUserContext) == false)
					return false;
			}
		}

		return true;
	}

	bool FileSystemManager::DoesFileExist(const CFileName& path, bool mapped_only) const
	{
		std::lock_guard <std::recursive_mutex> __lock(m_fsMutex);

		if (m_files.find(path.GetHash()) != m_files.end())
			return true;

		if (!mapped_only)
			return 0 == _waccess(path.GetPathW().c_str(), 0);

		return false;
	}

	bool FileSystemManager::OpenFile(const CFileName& path, CFile& fp, bool silent_failure) const
	{
		std::lock_guard <std::recursive_mutex> __lock(m_fsMutex);

		const auto it = m_files.find(path.GetHash());
		if (it != m_files.end()) {
			const auto& entry = it->second;
			return entry.first->Get(path, entry.second, fp);
		}

		const auto fileName = path.GetPathA();
		const auto extPos = fileName.find_last_of('.');
		if (extPos != std::string::npos) 
		{
			const auto ext = fileName.substr(extPos + 1);
			if (m_diskExtBlacklist.find(ext) != m_diskExtBlacklist.end())
			{
				DEBUG_LOG(LL_ERR, "Loading %ls from disk is forbidden", path.GetPathW().c_str());
				return false;
			}
		}

		if (fp.Create(path.GetPathW(), FILEMODE_READ, true, silent_failure))
			return true;

		if (!silent_failure)
		{
			DEBUG_LOG(LL_ERR, "Failed to load %ls %u", path.GetPathW().c_str(), path.GetHash());
		}
		return false;
	}

	void FileSystemManager::AddToDiskBlacklist(const std::string& extension)
	{
		std::lock_guard <std::recursive_mutex> __lock(m_fsMutex);

		DEBUG_LOG(LL_SYS, "Blocked extension: %s", extension.c_str());
		m_diskExtBlacklist.emplace(extension);
	}

#ifdef ENABLE_LAYER2_FILE_ENCRYPTION
	std::vector <uint8_t> FileSystemManager::DecryptLayer2Protection(const uint8_t* data, size_t length)
	{
		std::lock_guard <std::recursive_mutex> __lock(m_fsMutex);

		std::vector <uint8_t> outBuffer(length);

		if (m_layer2Type == FILE_FLAG_RAW)
		{
			memcpy(&outBuffer[0], data, length);
		}
		else if (m_layer2Type == FILE_FLAG_BLOWFISH)
		{
			try
			{
				CryptoPP::CTR_Mode<CryptoPP::Blowfish>::Decryption dec(&DEFAULT_LAYER2_KEY[0], 16, &DEFAULT_LAYER2_KEY[16]);
				dec.ProcessData(&outBuffer[0], data, length);
			}
			catch (const CryptoPP::Exception& exception)
			{
				FS_LOG(LL_CRI, "Caught exception on layer2 type 1 decryption: %s", exception.what());
				abort();
			}
		}
		else if (m_layer2Type == FILE_FLAG_XTEA)
		{
			try
			{
				CryptoPP::CTR_Mode<CryptoPP::XTEA>::Decryption dec(&DEFAULT_LAYER2_KEY[0], 16, &DEFAULT_LAYER2_KEY[16]);
				dec.ProcessData(&outBuffer[0], data, length);
			}
			catch (const CryptoPP::Exception& exception)
			{
				FS_LOG(LL_CRI, "Caught exception on layer2 type 2 decryption: %s", exception.what());
				abort();
			}
		}
		else
		{
			DEBUG_LOG(LL_CRI, "Unknown layer2 encryption type: %u", m_layer2Type);
			abort();
		}
		return outBuffer;
	}
#endif

	void* FileSystemManager::GetLzoWorkMem() const
	{
		std::lock_guard <std::recursive_mutex> __lock(m_fsMutex);

		return m_lzoWorkMem;
	}

	bool FileSystemManager::GetFileInformation(const CFileName& filename, FSFileInformation& fileInfo) const
	{
		std::lock_guard <std::recursive_mutex> __lock(m_fsMutex);

		for (const auto& it : m_files)
		{
			if (it.second.first && it.second.second.filenameHash == filename.GetHash())
			{
				fileInfo = it.second.second;
				return true;
			}
		}

		FS_LOG(LL_ERR, "File: %ls not found!", filename.GetPathW().c_str());
		return false;
	}
};
