#include "StdAfx.h"
#include "InsultChecker.h"

CInsultChecker& CInsultChecker::GetSingleton()
{
	static CInsultChecker s_kInsultChecker;
	return s_kInsultChecker;
}

CInsultChecker::CInsultChecker()
{
}

CInsultChecker::~CInsultChecker()
{
}

void CInsultChecker::Clear()
{
	m_kList_stInsult.clear();
}

void CInsultChecker::AppendInsult(const std::string& c_rstInsult)
{
	if (c_rstInsult.length()>0)
		m_kList_stInsult.push_back(c_rstInsult);
}

bool CInsultChecker::__GetInsultLength(const char* c_szWord, uint32_t* puInsultLen)
{
	std::list<std::string>::iterator i;
	for (i=m_kList_stInsult.begin(); i!=m_kList_stInsult.end(); ++i)
	{
		std::string& rstInsult=*i;

		int32_t ret=LocaleService_StringCompareCI(c_szWord, rstInsult.c_str(), rstInsult.length());		
		if (0==ret)
		{
			*puInsultLen=rstInsult.length();
			return true;
		}
	}

	return false;
}

bool CInsultChecker::__IsInsult(const char* c_szWord)
{
	uint32_t uInsultLen;
	return __GetInsultLength(c_szWord, &uInsultLen);
}

void CInsultChecker::FilterInsult(char* szLine, uint32_t uLineLen)
{
	const char INSULT_FILTER_CHAR = '*'; 
	for (uint32_t uPos=0; uPos<uLineLen;)
	{
		uint8_t bChr=szLine[uPos];
		uint32_t uInsultLen;
		if (__GetInsultLength(szLine+uPos, &uInsultLen))
		{
			memset(szLine+uPos, INSULT_FILTER_CHAR, uInsultLen);
			uPos += uInsultLen;
		}
		else
		{
			if ( LocaleService_IsLeadByte( bChr ) )
				uPos += 2;
			else
				uPos++;
		}
	}
}

bool CInsultChecker::IsInsultIn(const char* c_szLine, uint32_t uLineLen)
{
	for (uint32_t uPos=0; uPos<uLineLen;)
	{
		uint8_t bChr=c_szLine[uPos];
		if (bChr & 0x80)
		{
			if (__IsInsult(c_szLine+uPos))
				return true;

			uPos+=2;
		}
		else
		{
			if (__IsInsult(c_szLine+uPos))
				return true;

			uPos++;
		}
	}

	return false;
}
