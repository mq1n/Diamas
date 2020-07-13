#pragma once
#include <vector>
#include <map>
#include <fstream>

#include <rapidjson/document.h>
#include <rapidjson/reader.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/error/en.h>
#include <rapidjson/filereadstream.h>
using namespace rapidjson;

class CConfigManager : public CSingleton <CConfigManager>
{
	public:
		CConfigManager() : m_bHasParsed(false)
		{
		}
		~CConfigManager() = default;

		bool CConfigManager::ParseFile(const std::string& stConfigFileName)
		{
			auto pFile = fopen(stConfigFileName.c_str(), "rb");
			if (!pFile)
			{
				sys_err("Config file: %s can NOT read", stConfigFileName.c_str());
				return false;		
			}

			fseek(pFile, 0, SEEK_END); // seek to end of file
			auto dwSize = ftell(pFile); // get current file pointer
			fseek(pFile, 0, SEEK_SET); // seek back to beginning of file

			auto pBuffer = (char*)malloc(dwSize);
			FileReadStream is(pFile, pBuffer, dwSize);

			m_bHasParsed = true;
			m_stConfigContent = pBuffer;

			fclose(pFile);
			free(pBuffer);
			return true;
		}

		bool CConfigManager::HasParsed() const
		{
			return m_bHasParsed;
		}
				
		auto GetConfigFileContent() const
		{
			return m_stConfigContent;
		}

	private:
		std::string m_stConfigContent;
		bool 		m_bHasParsed;
};
