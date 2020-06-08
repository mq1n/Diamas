#include "../include/File.hpp"
#include "../include/LogHelper.hpp"
#include "../include/Utils.hpp"
#include "../include/Constants.hpp"
#include <xxhash.h>

namespace FileSystem
{
	CFile::CFile() :
		m_fileHandle(INVALID_HANDLE_VALUE), m_mapHandle(nullptr),
		m_mappedData(nullptr), m_mappedSize(0),
		m_rawData(nullptr), m_rawSize(0),
		m_currPos(0), m_memOwner(false),
		m_fileType(FILE_TYPE_NONE)
	{
	}
	CFile::~CFile()
	{
		Close();
	}

	bool CFile::Create(const CFileName& filename, uint8_t filemode, bool map, bool silent_failure)
	{
		std::lock_guard <std::recursive_mutex> __lock(m_fileMutex);

		Close();

		auto filename_conv = GetAbsolutePath(filename.GetPathW());

		DEBUG_LOG(LL_SYS, "File: %ls(%ls) Mode: %u", filename.GetPathW().c_str(), filename_conv.c_str(), filemode);

		uint32_t dwMode = 0, dwShareMode = FILE_SHARE_READ;
		if (filemode == FILEMODE_WRITE)
		{
			dwMode = GENERIC_READ | GENERIC_WRITE;
			dwShareMode = FILE_SHARE_READ | FILE_SHARE_WRITE;
		}
		else
		{
			dwMode = GENERIC_READ;
			dwShareMode = FILE_SHARE_READ;
		}

		m_fileHandle = CreateFileW(filename.GetPathW().c_str(), dwMode, dwShareMode, nullptr, filemode == FILEMODE_READ ? OPEN_EXISTING : OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
		if (m_fileHandle && m_fileHandle != INVALID_HANDLE_VALUE)
		{
			m_fileType = FILE_TYPE_OUTPUT;
			m_fileName = filename.GetPathW();
			m_Name = filename_conv;

			if (map)
				return Map(filename, 0, 0);
			return true;
		}
		if (!silent_failure)
		{
			FS_LOG(LL_ERR, "File: %ls can not created! Error: %u", filename.GetPathW().c_str(), GetLastError());
		}
        return false;
	}

	bool CFile::Open(const CFileName& filename, bool silent)
	{
		std::lock_guard <std::recursive_mutex> __lock(m_fileMutex);

		Close();

		DEBUG_LOG(LL_SYS, "File: %ls", filename.GetPathW().c_str());

		m_fileHandle = CreateFileW(filename.GetPathW().c_str(), GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
		if (m_fileHandle && m_fileHandle != INVALID_HANDLE_VALUE)
		{
			m_fileType = FILE_TYPE_INPUT;
			m_fileName = filename.GetPathW();
			m_Name = GetAbsolutePath(filename.GetPathW());
			return true;
		}
		if (!silent) {
			FS_LOG(LL_ERR, "File: %ls can not open! Error: %u", filename.GetPathW().c_str(), GetLastError());
		}
		return false;
	}

	void CFile::Close()
	{
		std::lock_guard <std::recursive_mutex> __lock(m_fileMutex);

//		if (m_fileName.size() > 0)
//		{
//			DEBUG_LOG(LL_SYS, "%ls", m_fileName.c_str());
//		}

		if (m_rawData)
		{
			if (m_rawData != m_mappedData && m_memOwner)
			{
				free(m_rawData);
			}
			m_rawData = nullptr;
		}


		if (m_mappedData)
		{
			UnmapViewOfFile(m_mappedData);
			m_mappedData = nullptr;
		}

		if (m_mapHandle && m_mapHandle != INVALID_HANDLE_VALUE)
		{
			CloseHandle(m_mapHandle);
			m_mapHandle = nullptr;
		}

		if (m_fileHandle && m_fileHandle != INVALID_HANDLE_VALUE)
		{
			CloseHandle(m_fileHandle);
			m_fileHandle = INVALID_HANDLE_VALUE;
		}

		m_currPos = 0;
		m_rawSize = 0;
		m_mappedSize = 0;
		m_fileName.clear();
		m_fileType = FILE_TYPE_NONE;
		m_memOwner = false;
	}

	bool CFile::Map(const CFileName& filename, uint64_t offset, uint32_t size)
	{
		std::lock_guard <std::recursive_mutex> __lock(m_fileMutex);

		Close();

		DEBUG_LOG(LL_SYS, "%ls", filename.GetPathW().c_str());

		m_fileHandle = CreateFileW(GetAbsolutePath(filename.GetPathW()).c_str(), GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
		if (!m_fileHandle || m_fileHandle == INVALID_HANDLE_VALUE)
		{
			FS_LOG(LL_ERR, "CreateFile fail! Error: %u", GetLastError());
			return false;
		}

		m_mapHandle = CreateFileMappingA(m_fileHandle, nullptr, PAGE_READONLY, 0, 0, nullptr);
		if (!m_mapHandle)
		{
			FS_LOG(LL_ERR, "CreateFileMapping fail! File: %ls Error: %u", filename.GetPathW().c_str(), GetLastError());

			CloseHandle(m_fileHandle);
			m_fileHandle = INVALID_HANDLE_VALUE;
			return false;
		}

		SYSTEM_INFO sys {};
		GetSystemInfo(&sys);

		offset -= offset % sys.dwAllocationGranularity;

		m_mappedData = static_cast<uint8_t*>(MapViewOfFile(m_mapHandle, FILE_MAP_READ, offset >> 32, offset & 0xffffffff, size));
		
		LARGE_INTEGER s;
		GetFileSizeEx(m_fileHandle, &s);
		if (size == 0)
		{
			size = static_cast<uint32_t>(s.QuadPart);
		}
		m_mappedSize = size;

		m_rawData = m_mappedData + (offset % sys.dwAllocationGranularity);
		m_rawSize = std::min<uint64_t>(size, s.QuadPart);

		DEBUG_LOG(LL_SYS, "%ls, %llu, %u/%u", filename.GetPathW().c_str(), offset, size, (uint32_t)m_rawSize);

		m_currPos = 0;
	
		if (m_rawData)
		{
			m_fileType = FILE_TYPE_MAPPED;
			m_fileName = filename.GetPathW();
			m_Name = GetAbsolutePath(filename.GetPathW());
		}

		return (m_rawData != nullptr);
	}

	bool CFile::Assign(const CFileName& filename, const void* memory, uint32_t length, bool copy)
	{
		std::lock_guard <std::recursive_mutex> __lock(m_fileMutex);

		Close();

		DEBUG_LOG(LL_SYS, "%ls, %p, %u, %s", filename.GetPathW().c_str(), memory, length, copy ? "copy" : "assign");

		if (copy)
		{
			m_rawData = static_cast<uint8_t*>(malloc(length));
			if (m_rawData)
			{
				memcpy(m_rawData, memory, length);
				m_rawSize = length;
			}
			m_memOwner = true;
		}
		else
		{
			m_rawData = static_cast<uint8_t*>(const_cast<void*>(memory));
			m_rawSize = length;
			m_memOwner = false;
		}


		if (m_rawData)
		{
			m_fileType = FILE_TYPE_MEMORY;
			m_fileName = filename.GetPathW();
			m_Name = GetAbsolutePath(filename.GetPathW());
		}

		return m_rawData;
	}


	uint32_t CFile::Read(void* buffer, uint32_t size)
	{
		std::lock_guard <std::recursive_mutex> __lock(m_fileMutex);

		DEBUG_LOG(LL_TRACE, "%ls, %p, %u", m_fileName.c_str(), buffer, size);
	
		if (!IsReadable())
			return 0;

		switch (m_fileType)
		{
			case FILE_TYPE_OUTPUT:
			case FILE_TYPE_INPUT:
			{
				DWORD cbRead = 0;
				if (!ReadFile(m_fileHandle, buffer, size, &cbRead, nullptr))
				{
					FS_LOG(LL_ERR, "ReadFile fail! Error: %u", GetLastError());
					return 0;			
				}
				return cbRead;
			} break;

			case FILE_TYPE_MAPPED:
			case FILE_TYPE_MEMORY:
			{
				auto len = std::min<uint32_t>(static_cast<uint32_t>(m_rawSize - m_currPos), size);
				memcpy(buffer, &m_rawData[m_currPos], len);
				m_currPos += len;
				return len;
			} break;
		}

		return 0;
	}

	uint32_t CFile::Write(const void* buffer, uint32_t size)
	{
		std::lock_guard <std::recursive_mutex> __lock(m_fileMutex);

		DEBUG_LOG(LL_SYS, "%ls, %p, %u", m_fileName.c_str(), buffer, size);

		if (!IsWriteable())
			return 0;

		DWORD cbWritten = 0;
		if (!WriteFile(m_fileHandle, buffer, size, &cbWritten, nullptr))
		{
			FS_LOG(LL_ERR, "WriteFile fail! Error: %u", GetLastError());
			return 0;
		}
		
		return cbWritten;
	}

	int32_t CFile::Seek(uint32_t offset, int32_t iSeekType)
	{
		std::lock_guard <std::recursive_mutex> __lock(m_fileMutex);		

		DEBUG_LOG(LL_SYS, "%ls, %u, %d", m_fileName.c_str(), offset, iSeekType);

		if (!IsValid())
			return 0;

		switch (iSeekType)
		{
			case 0: // SEEK_TYPE_BEGIN:
				if (offset > GetSize())
					offset = GetSize();

				m_currPos = offset;
				break;

			case 1: // SEEK_TYPE_CURRENT:
				m_currPos = std::min(m_currPos + offset, GetSize());
				break;

			case 2: // SEEK_TYPE_END:
				m_currPos = (GetSize() >= offset) ? GetSize() - offset : 0;
				break;
		}

		return m_currPos;
	}

	void CFile::SetPosition(int64_t offset, bool relative)
	{
		std::lock_guard <std::recursive_mutex> __lock(m_fileMutex);		

//		DEBUG_LOG(LL_TRACE, "%ls, %lld, %s", m_fileName.c_str(), offset, relative ? "relative" : "absolute");
		DEBUG_LOG(LL_SYS, "%ls, %lld, %s", m_fileName.c_str(), offset, relative ? "relative" : "absolute");

		if (!IsValid())
			return;

		switch (m_fileType)
		{
			case FILE_TYPE_OUTPUT:
			case FILE_TYPE_INPUT:
			{
				LARGE_INTEGER m;
				m.QuadPart = offset;

				if (!SetFilePointerEx(m_fileHandle, m, 0, relative ? FILE_CURRENT : FILE_BEGIN))
				{
					FS_LOG(LL_ERR, "SetFilePointerEx fail! Error: %u", GetLastError());
					return;
				}
			} break;

			case FILE_TYPE_MAPPED:
			case FILE_TYPE_MEMORY:
			{
				m_currPos = relative ? m_currPos + offset : offset;
			} break;
		}
	}


	bool CFile::IsValid() const
	{
		std::lock_guard <std::recursive_mutex> __lock(m_fileMutex);		

        auto ret = false;
		switch (m_fileType)
		{
			case FILE_TYPE_OUTPUT:
			case FILE_TYPE_INPUT:
			{
				ret = m_fileHandle && m_fileHandle != INVALID_HANDLE_VALUE;
			} break;

			case FILE_TYPE_MAPPED:
			case FILE_TYPE_MEMORY:
			{
				ret = m_rawData != nullptr;
			} break;
		}

		return ret;
	}
	bool CFile::IsReadable() const
	{
		std::lock_guard <std::recursive_mutex> __lock(m_fileMutex);

		return IsValid();
	}
	bool CFile::IsWriteable() const
	{
		std::lock_guard <std::recursive_mutex> __lock(m_fileMutex);

		return (m_fileType == FILE_TYPE_OUTPUT && IsValid());
	}


	uint8_t CFile::GetFileType() const
	{
		std::lock_guard <std::recursive_mutex> __lock(m_fileMutex);		

		return m_fileType;
	}
	
	const uint8_t* CFile::GetData() const
	{
		std::lock_guard <std::recursive_mutex> __lock(m_fileMutex);

		return reinterpret_cast<const uint8_t*>(m_rawData);
	}

	const std::wstring& CFile::GetName() const
	{
		std::lock_guard <std::recursive_mutex> __lock(m_fileMutex);

		return m_Name;
	}

	const std::wstring& CFile::GetFileName() const
	{
		std::lock_guard <std::recursive_mutex> __lock(m_fileMutex);

		return m_fileName;
	}

	uint64_t CFile::GetSize() const
	{
		std::lock_guard <std::recursive_mutex> __lock(m_fileMutex);	

		if (!IsValid())
    		return 0;

        uint64_t size = 0;
		switch (m_fileType)
		{
			case FILE_TYPE_OUTPUT:
			case FILE_TYPE_INPUT:
			{
				LARGE_INTEGER s;
				GetFileSizeEx(m_fileHandle, &s);
				size = s.QuadPart;
			} break;

			case FILE_TYPE_MAPPED:
			case FILE_TYPE_MEMORY:
			{
				size = m_rawSize;
			} break;
		}

//		return static_cast<uint32_t>(size);
		return size;
	}
	uint64_t CFile::GetPosition() const
	{
		std::lock_guard <std::recursive_mutex> __lock(m_fileMutex);

		if (!IsValid())
			return 0;

		switch (m_fileType)
		{
			case FILE_TYPE_OUTPUT:
			case FILE_TYPE_INPUT:
			{
				LARGE_INTEGER newptr{};
				LARGE_INTEGER distance{};

				SetFilePointerEx(m_fileHandle, distance, &newptr, FILE_CURRENT);
				return newptr.QuadPart;
			} break;

			case FILE_TYPE_MAPPED:
			case FILE_TYPE_MEMORY:
			{
				return m_currPos;
			} break;
		}
		return 0;
	}

	const uint8_t* CFile::GetCurrentSeekPoint() const
	{
		std::lock_guard <std::recursive_mutex> __lock(m_fileMutex);

		return reinterpret_cast<uint8_t*>((uint64_t)GetData() + m_currPos);
	}

	uint32_t CFile::GetScriptHash(const uint8_t* data, size_t size)
	{
		auto scriptHash = XXH32(reinterpret_cast<const char*>(data), size, FILE_HASH_MAGIC);
		return scriptHash;
	}
}
