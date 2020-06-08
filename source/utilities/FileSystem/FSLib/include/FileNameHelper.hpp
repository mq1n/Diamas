#pragma once
#include <string>
#include <cstdint>

namespace FileSystem
{
	class CFileName
	{
		public:
			CFileName();
			CFileName(const wchar_t* path);
			CFileName(const char* path);
			CFileName(const std::wstring& path);
			CFileName(const std::string& path);
			CFileName(const uint32_t hash);
			~CFileName() = default;

			void Set(const std::wstring& path, uint32_t length);
			void Set(const std::string& path, uint32_t length);

			explicit operator bool() { return (m_path.size() ? true : false); }
			auto& operator=(const wchar_t* path);
			auto& operator=(const char* path);
			auto& operator=(const std::wstring& path);
			auto& operator=(const std::string& path);
			auto& operator=(const uint32_t hash);

			auto GetHash() const   	{ return m_hash; }
			auto GetHashPtr() const { return reinterpret_cast<void*>(m_hash); }
			std::wstring GetPathW() const;
			std::string GetPathA() const;

		private:
			std::wstring m_path;
			uint32_t 	 m_hash;
	};
}
