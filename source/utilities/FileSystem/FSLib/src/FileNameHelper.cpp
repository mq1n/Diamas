#include "../include/FileNameHelper.hpp"
#include "../include/Constants.hpp"
#include "xxhash.h"
#include <cstring>
#include <algorithm>

namespace FileSystem
{
	CFileName::CFileName() :
		m_hash(0)
	{
	}
	CFileName::CFileName(const std::wstring& path)
	{
		Set(path.data(), path.length());
	}
	CFileName::CFileName(const std::string& path)
	{
		Set(path.data(), path.length());
	}
	CFileName::CFileName(const wchar_t* path)
	{
		Set(path, std::wcslen(path));
	}
	CFileName::CFileName(const char* path)
	{
		Set(path, std::strlen(path));
	}
	CFileName::CFileName(const uint32_t hash)
	{
		m_hash = hash;
		m_path.clear();
	}

	auto& CFileName::operator=(const std::wstring& path)
	{
		Set(path.data(), path.length());
		return *this;
	}
	auto& CFileName::operator=(const std::string& path)
	{
		Set(path.data(), path.length());
		return *this;
	}
	auto& CFileName::operator=(const wchar_t* path)
	{
		Set(path, std::wcslen(path));
		return *this;
	}
	auto& CFileName::operator=(const char* path)
	{
		Set(path, std::strlen(path));
		return *this;
	}
	auto& CFileName::operator=(const uint32_t hash)
	{
		m_hash = hash;
		m_path.clear();
		return *this;
	}

	void CFileName::Set(const std::wstring& path, uint32_t length)
	{
		m_path.resize(length);

		m_path = path;
		for (size_t i = 0; i < m_path.size(); ++i)
		{
			if (m_path[i] == L'\\')
				m_path[i] = L'/';
		}
		std::transform(m_path.begin(), m_path.end(), m_path.begin(), ::tolower);

		m_hash = XXH32(m_path.data(), length * sizeof(wchar_t), FILE_NAME_MAGIC);
	}
	void CFileName::Set(const std::string& path, uint32_t length)
	{
		m_path.resize(length);

		m_path = std::wstring(path.begin(), path.end());
		for (size_t i = 0; i < m_path.size(); ++i)
		{
			if (m_path[i] == L'\\')
				m_path[i] = L'/';
		}
		std::transform(m_path.begin(), m_path.end(), m_path.begin(), ::tolower);

		m_hash = XXH32(m_path.data(), length * sizeof(wchar_t), FILE_NAME_MAGIC);
	}

	std::wstring CFileName::GetPathW() const
	{
		return m_path;
	}
	std::string CFileName::GetPathA() const
	{
		return std::string(m_path.begin(), m_path.end());
	}
}
