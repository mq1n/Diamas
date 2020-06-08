#pragma once
#include <list>
#include <unordered_map>
#include <cstdint>
#include "Constants.hpp"
#include "File.hpp"

namespace FileSystem
{
	class CArchive;

	typedef std::unordered_map<
		uint32_t, /* hash */
		std::pair <CArchive*, FSFileInformation> /* archive: file_info */
	> FSFileDict;

	class CArchive
	{
	public:
		bool Create(const CFileName& filename, const std::array <uint8_t, ARCHIVE_KEY_LENGTH>& key, FSFileDict& dict);
		bool Get(const CFileName& path, const FSFileInformation& entry, CFile& fp);
		auto& GetArchiveFileStream()
		{ 
			std::lock_guard <std::recursive_mutex> __lock(m_archiveMutex);
			return m_file;
		};

	private:
		mutable std::recursive_mutex m_archiveMutex;

		CFile m_file;
		std::vector <uint8_t> m_key;
	};

	class CArchiveMaker
	{
	public:
		CArchiveMaker();
		~CArchiveMaker() = default;

		bool Create(const CFileName& filename, const std::array <uint8_t, ARCHIVE_KEY_LENGTH>& key);
		bool Add(const CFileName& archivedPath, const CFileName& diskPath, uint32_t flags);
		bool Save();

	private:
		mutable std::recursive_mutex m_archiveMutex;

		CFile m_file;
		uint8_t m_layer2Type;
		std::vector <uint8_t> m_key;
		std::vector <FSFileInformation> m_files;
	};
};
