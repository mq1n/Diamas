#pragma once

#include "../eterBase/FileDir.h"

class CPropertyLoader : public CDir
{
	public:
		virtual ~CPropertyLoader();

		uint32_t		RegisterFile(const char* c_szPathName, const char* c_szFileName);

		virtual bool	OnFolder(const char* c_szFilter, const char* c_szPathName, const char* c_szFileName);
		virtual bool	OnFile(const char* c_szPathName, const char* c_szFileName);
};
