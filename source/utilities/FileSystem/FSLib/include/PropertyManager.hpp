#pragma once
#include <string>
#include <vector>
#include <map>

namespace FileSystem
{
	using TPropertyNode = std::map <std::string /* key */, std::string /* value */>;
	using TPropertyContainer = std::vector <TPropertyNode>;

	bool GetPropertyObject(const std::string& strPropertyFileContent, uint32_t dwCRC, TPropertyNode& content);
	std::string LoadPropertyFile(const std::string& strPropertyListFile);
	bool LoadPropertyList(const std::string& strPropertyFileContent, TPropertyContainer& vPropertyItems);
	bool GeneratePropertyList(const std::string& strPropertyDir, const std::string& stOutputFile);
};
