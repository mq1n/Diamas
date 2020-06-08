#pragma once
#include <Windows.h>
#include <memory>
#include <algorithm>
#include <string>
#include <mutex>
#include <vector>
#include "FileNameHelper.hpp"

namespace FileSystem
{	
	class CFile
	{
		public:	
			CFile();
			~CFile();

			bool Create(const CFileName& filename, uint8_t filemode, bool map, bool silent_failure = false);
			bool Open(const CFileName& filename, bool silent = false);
			bool Map(const CFileName& filename, uint64_t offset = 0, uint32_t size = 0);
			bool Assign(const CFileName& filename, const void* memory, uint32_t length, bool copy = true);

			void Close();

			uint32_t Read(void* buffer, uint32_t size);
			uint32_t Write(const void* buffer, uint32_t size);

			int32_t CFile::Seek(uint32_t offset, int32_t iSeekType);			
			void SetPosition(int64_t offset, bool relative = false);

			bool IsValid() const;
			bool IsReadable() const;
			bool IsWriteable() const;

			uint8_t GetFileType() const;
			const uint8_t* GetData() const;
			const std::wstring& GetFileName() const;
			const std::wstring& GetName() const;
			uint64_t GetSize() const;
			uint64_t GetPosition() const;
			const uint8_t* GetCurrentSeekPoint() const;

			uint32_t GetScriptHash(const uint8_t* data, size_t size);

		private:
			mutable std::recursive_mutex m_fileMutex;
			
			std::wstring m_Name;
			std::wstring m_fileName;

			HANDLE m_fileHandle;
			HANDLE m_mapHandle;

			uint8_t* m_mappedData;
			uint64_t m_mappedSize;

			uint8_t* m_rawData;
			uint64_t m_rawSize;

			uint64_t m_currPos;
			bool m_memOwner;
			int32_t m_fileType;
	};
}
