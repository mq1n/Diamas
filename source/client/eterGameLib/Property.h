#pragma once

#include <string>

class CProperty
{
	public:
		CProperty(const char* c_pszFileName, uint32_t dwCRC = 0);
		~CProperty();

		void			Clear();
		bool			ReadFromMemory(const void* c_pvData, int32_t iLen, const char* c_pszFileName);

		const char* GetFileName();

		bool			GetVector(const char* c_pszKey, CTokenVector& rTokenVector);
		bool			GetString(const char* c_pszKey, const char** c_ppString);

		void			PutVector(const char* c_pszKey, const CTokenVector& c_rTokenVector);
		void			PutString(const char* c_pszKey, const char* c_pszString);

		uint32_t			GetSize();
		uint32_t			GetCRC();

	protected:
		std::string							m_stFileName;
		std::string							m_stCRC;
		const char *						mc_pFileName;
		uint32_t								m_dwCRC;

		CTokenVectorMap						m_stTokenMap;
};
