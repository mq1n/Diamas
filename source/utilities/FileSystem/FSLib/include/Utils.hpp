#pragma once
#include <Windows.h>
#include <string>
#include <cstdint>
#include <vector>
#include <sstream>
#include <xxhash.h>

#include "Constants.hpp"
#include "LogHelper.hpp"
#include "DataBuffer.hpp"
#include "FileNameHelper.hpp"

namespace FileSystem
{	
	struct StringHashGenerator
	{
		size_t GetHash(const std::string& str) const
		{
			auto s = reinterpret_cast<const uint8_t*>(str.c_str());
			auto end = s + str.size();
			size_t h = 0;

			while (s < end)
			{
				h *= 16777619;
				h ^= (uint8_t) * (uint8_t*)(s++);
			}

			return h;
		}

		size_t operator () (const std::string& str) const
		{
			return GetHash(str);
		}
	};

	static bool IsLayer2File(const CFileName& file)
	{
		auto fileName = file.GetPathW();
		auto extension = fileName.substr(fileName.find_last_of(L".") + 1);

		if (
			fileName == L"system_python" || extension == L"py" ||
			std::find(ENCRYPTED_RESOURCES_TYPES.begin(), ENCRYPTED_RESOURCES_TYPES.end(), extension) != ENCRYPTED_RESOURCES_TYPES.end())
		{
			return true;
		}
		return false;
	}

	static uint32_t GetFileHash(const uint8_t* data, size_t length)
	{
		auto hash = XXH32(reinterpret_cast<const char*>(data), length, FILE_HASH_MAGIC);
		return hash;
	}

	static std::string ConvertToLower(const std::string& in)
	{
		std::string out = in;
		std::transform(out.begin(), out.end(), out.begin(), ::tolower);
		return out;
	}

	static std::string GetNameFromPath(std::string stFileName)
	{
		auto slashPos = stFileName.find_last_of("\\/");
		stFileName = stFileName.substr(slashPos + 1, stFileName.length() - slashPos);
		return stFileName;
	}
	static std::wstring GetNameFromPath(std::wstring wstFileName)
	{
		auto slashPos = wstFileName.find_last_of(L"\\/");
		wstFileName = wstFileName.substr(slashPos + 1, wstFileName.length() - slashPos);
		return wstFileName;
	}

    static std::string GetCurrentPath()
    {
        char szBuffer[MAX_PATH] { 0 };
        GetModuleFileNameA(nullptr, szBuffer, MAX_PATH);

        auto stBuffer = std::string(szBuffer);
        auto pos = stBuffer.find_last_of("\\/");
        return stBuffer.substr(0, pos);
    }

	static std::wstring GetAbsolutePath(const std::wstring& path)
	{
		wchar_t wszBuffer[MAX_PATH] = { '\0' };
		_wfullpath(wszBuffer, path.c_str(), MAX_PATH);

		return wszBuffer;
	}

	static std::wstring GetWorkingDirectory()
	{
		wchar_t wszBuffer[MAX_PATH] = { '\0' };
		GetCurrentDirectoryW(MAX_PATH, wszBuffer);

		return wszBuffer;
	}
	
	static std::string GetKeyAsString(const uint8_t* key)
	{
		std::stringstream ss;

		std::vector <uint8_t> buffer(ARCHIVE_KEY_LENGTH);
		memcpy(&buffer[0], key, ARCHIVE_KEY_LENGTH);

		for (size_t i = 0; i < ARCHIVE_KEY_LENGTH; ++i)
			ss << "0x" << std::hex << std::to_string(buffer.at(i)) << ", ";

		auto str = ss.str();
		return str.substr(0, str.size() - 2);
	}
	static std::string GetKeyAsString(const std::vector <uint8_t>& key)
	{
		return GetKeyAsString(key.data());
	}
	static std::string GetKeyAsString(const std::array <uint8_t, ARCHIVE_KEY_LENGTH>& key)
	{
		return GetKeyAsString(key.data());
	}
	static std::string GetKeyAsString(const std::array <uint8_t, LAYER2_KEY_LENGTH>& key)
	{
		return GetKeyAsString(key.data());
	}

	static std::wstring GetExecutableDirectory()
	{
		wchar_t wszBuffer[MAX_PATH] = { '\0' };
		GetModuleFileNameW(GetModuleHandleA(nullptr), wszBuffer, MAX_PATH);

		for (size_t i = wcslen(wszBuffer); i > 0; --i)
		{
			if (wszBuffer[i] == L'\\' || wszBuffer[i] == L'/')
			{
				wszBuffer[i] = 0;
				break;
			}
		}
		for (size_t i = 0; i < MAX_PATH && wszBuffer[i] != 0; ++i)
		{
			if (wszBuffer[i] == L'\\')
			{
				wszBuffer[i] = L'/';
			}
		}
		return wszBuffer;
	}
	
	static bool WildcardMatch(const std::wstring& str, const std::wstring& match)
	{
//		DEBUG_LOG(LL_SYS, "%ls, %ls", str.c_str(), match.c_str());

		const wchar_t* pMatch = match.c_str(), * pString = str.c_str();
		while (*pMatch)
		{
			if (*pMatch == '?')
			{
				if (!*pString)
				{
					return false;
				}
				++pString;
				++pMatch;
			}
			else if (*pMatch == '*')
			{
				if (WildcardMatch(pString, pMatch + 1) || (*pString && WildcardMatch(pString + 1, pMatch)))
				{
					return true;
				}
				return false;
			}
			else
			{
				if (*pString++ != *pMatch++)
				{
					return false;
				}
			}
		}
		return !*pString && !*pMatch;
	}

	static void Log(int32_t level, const char* c_szFormat, ...)
	{
		if (gs_pFSLogInstance)
		{
			char szBuffer[8192] = { 0 };

			va_list vaArgList;
			va_start(vaArgList, c_szFormat);
			vsprintf_s(szBuffer, c_szFormat, vaArgList);
			va_end(vaArgList);

			gs_pFSLogInstance->Log(__FUNCTION__, level, szBuffer);
		}
	}
}
