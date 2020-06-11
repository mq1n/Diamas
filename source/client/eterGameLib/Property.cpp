#include "StdAfx.h"
#include <cstring>

#include "PropertyManager.h"
#include "Property.h"
/*
*	CProperty 파일 포맷
*
*  0 ~ 4 bytes: fourcc
*  5 ~ 6 bytes: \r\n
*
*  그 이후의 바이트들은 텍스트 파일 로더와 같은 구조
*/
CProperty::CProperty(const char * c_pszFileName, uint32_t dwCRC) : mc_pFileName(nullptr), m_dwCRC(dwCRC)
{
	m_stFileName = c_pszFileName;
	StringPath(m_stFileName);

	mc_pFileName = strrchr(m_stFileName.c_str(), '/');

	if (!mc_pFileName)
		mc_pFileName = m_stFileName.c_str();
	else
		++mc_pFileName;
}

CProperty::~CProperty() = default;

uint32_t CProperty::GetCRC()
{
	return m_dwCRC;
}

const char * CProperty::GetFileName()
{
	return (m_stFileName.c_str());
}

bool CProperty::GetString(const char * c_pszKey, const char ** c_ppString)
{
	std::string stTempKey = c_pszKey;
	stl_lowers(stTempKey);
	auto it = m_stTokenMap.find(stTempKey);

	//	printf("GetString %s %d\n", stTempKey.c_str(), m_stTokenMap.size());

	if (m_stTokenMap.end() == it)
		return false;

	*c_ppString = it->second[0].c_str();
	return true;
}

uint32_t CProperty::GetSize()
{
	return m_stTokenMap.size();
}

bool CProperty::GetVector(const char * c_pszKey, CTokenVector & rTokenVector)
{
	std::string stTempKey = c_pszKey;
	stl_lowers(stTempKey);

	auto it = m_stTokenMap.find(stTempKey);
	if (m_stTokenMap.end() == it)
		return false;

	// NOTE : 튕김 현상 발견
	//	std::copy(rTokenVector.begin(), it->second.begin(), it->second.end());
	// NOTE : 레퍼런스에는 이런 식으로 하게끔 되어 있음
	///////////////////////////////////////////////////////////////////////////////
	//	template <class InputIterator, class OutputIterator>
	//	OutputIterator copy(InputIterator first, InputIterator last,
	//                    OutputIterator result);
	//
	//	vector<int32_t> V(5);
	//	iota(V.begin(), V.end(), 1);
	//	list<int32_t> L(V.size());
	//	copy(V.begin(), V.end(), L.begin());
	//	assert(equal(V.begin(), V.end(), L.begin()));
	///////////////////////////////////////////////////////////////////////////////
	// 헌데 그래도 튕김. - [levites]
	//	std::copy(it->second.begin(), it->second.end(), rTokenVector.begin());

	// 결국 이렇게.. - [levites]
	// 현재 사용하는 곳 : WorldEditor/Dialog/MapObjectPropertyPageBuilding.cpp
	CTokenVector & rSourceTokenVector = it->second;

	auto itor = rSourceTokenVector.begin();
	for (; itor != rSourceTokenVector.end(); ++itor)
		rTokenVector.emplace_back(*itor);

	return true;
}

void CProperty::PutString(const char * c_pszKey, const char * c_pszString)
{
	std::string stTempKey = c_pszKey;
	stl_lowers(stTempKey);

	auto itor = m_stTokenMap.find(stTempKey);
	if (itor != m_stTokenMap.end())
		m_stTokenMap.erase(itor);

	CTokenVector tokenVector;
	tokenVector.emplace_back(c_pszString);

	m_stTokenMap.emplace(stTempKey, tokenVector);
}

void CProperty::PutVector(const char * c_pszKey, const CTokenVector & c_rTokenVector)
{
	std::string stTempKey = c_pszKey;
	stl_lowers(stTempKey);

	m_stTokenMap.emplace(stTempKey, c_rTokenVector);
}

void GetTimeString(char * str, time_t ct)
{
	struct tm tm;
	tm = *localtime(&ct);

	_snprintf_s(str, 15, _TRUNCATE, "%04d%02d%02d%02d%02d%02d",
		tm.tm_year + 1900,
		tm.tm_mon + 1,
		tm.tm_mday,
		tm.tm_hour,
		tm.tm_min,
		tm.tm_sec);
}

#pragma pack(push, 1)
struct PropertyHeader
{
	uint32_t fourCc;
	char sep[2];
};
#pragma pack(pop)

bool CProperty::ReadFromMemory(const void * c_pvData, int32_t iLen, const char * c_pszFileName)
{
	PropertyHeader header;
	if (iLen < sizeof(header))
	{
		TraceError("Property file %s is too small", c_pszFileName);
		return false;
	}

	std::memcpy(&header, c_pvData, sizeof(header));

	if (header.fourCc != MAKEFOURCC('Y', 'P', 'R', 'T'))
	{
		TraceError("Property file FourCC is invalid for %s", c_pszFileName);
		return false;
	}

	if (header.sep[0] != '\r' || header.sep[1] != '\n')
	{
		TraceError("CProperty::ReadFromMemory: File format error after FourCC: %s\n", c_pszFileName);
		return false;
	}

	const char* pcData = (const char*)c_pvData + sizeof(header);
	iLen -= sizeof(header);

	CMemoryTextFileLoader textFileLoader;
	textFileLoader.Bind(iLen, pcData);

	m_stCRC = textFileLoader.GetLineString(0);
	m_dwCRC = std::stoul(m_stCRC);

	CTokenVector stTokenVector;
	for (uint32_t i = 1; i < textFileLoader.GetLineCount(); ++i)
	{
		if (!textFileLoader.SplitLine(i, &stTokenVector))
			continue;

		stl_lowers(stTokenVector[0]);
		std::string stKey = stTokenVector[0];

		stTokenVector.erase(stTokenVector.begin());
		PutVector(stKey.c_str(), stTokenVector);
	}

	//Tracef("Property: %s\n", c_pszFileName);
	return true;
}

void CProperty::Clear()
{
	m_stTokenMap.clear();
}
