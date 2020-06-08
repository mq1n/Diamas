#pragma once
#include <cstdint>
#include <memory>
#include <string>
#include <mutex>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <array>
#include <list>
#include <sstream>
#include <functional>
#include "Constants.hpp"
#include "FileSystem.hpp"

namespace FileSystem
{
	struct FSRegistiredArchiveData;
	
	class FileSystemManager
	{
		using TEnumFiles = std::function<bool(const CFileName& archiveName, const FSFileInformation& pcFileInformations, void* pvUserContext)>;
		
		// Lifecycle
		public:	
			virtual ~FileSystemManager();

			FileSystemManager(const FileSystemManager&) = delete;
			FileSystemManager(FileSystemManager&&) noexcept = delete;
			FileSystemManager& operator=(const FileSystemManager&) = delete;
			FileSystemManager& operator=(FileSystemManager&&) noexcept = delete;

		// Public methods
		public:
			// Constructor
			FileSystemManager();

			// Initilization
			bool InitializeFSManager(
#ifdef ENABLE_LAYER2_FILE_ENCRYPTION
				uint8_t layer2Type
#endif
			);

			// Finalization
			bool FinalizeFSManager() const;

			// FileSystem Pack singletons
			static FileSystemManager* InstancePtr();
			static FileSystemManager& Instance();

			// Methods
			void CloseArchives();

			bool AddArchive(const CFileName& filename, const std::array <uint8_t, ARCHIVE_KEY_LENGTH>& key);
			CArchive* GetArchive(const CFileName& archivename);

			bool DoesFileExist(const CFileName& path, bool mapped_only = false) const;
			bool OpenFile(const CFileName& path, CFile& fp, bool silent_failure = false) const;

			void AddToDiskBlacklist(const std::string& extension);

			bool EnumerateFiles(const CFileName& archivename, TEnumFiles pfnEnumFiles, LPVOID pvUserContext);

#ifdef ENABLE_LAYER2_FILE_ENCRYPTION
			uint8_t GetLayer2Type() const { return m_layer2Type; };
			std::vector <uint8_t> DecryptLayer2Protection(const uint8_t* data, size_t length);
#endif

			void* GetLzoWorkMem() const;
			bool GetFileInformation(const CFileName& filename, FSFileInformation& fileInfo) const;

		private:
			mutable std::recursive_mutex m_fsMutex;

			FSFileDict m_files;
			std::vector <std::unique_ptr<CArchive> > m_archives;
			std::unordered_set <std::string> m_diskExtBlacklist;
			void* m_lzoWorkMem;
#ifdef ENABLE_LAYER2_FILE_ENCRYPTION
			uint8_t m_layer2Type;
#endif
	};
};

