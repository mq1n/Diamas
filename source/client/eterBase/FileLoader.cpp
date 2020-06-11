#include "StdAfx.h"
#include "FileLoader.h"
#include <assert.h>

CMemoryTextFileLoader::CMemoryTextFileLoader()
{
}

CMemoryTextFileLoader::~CMemoryTextFileLoader()
{
}

bool CMemoryTextFileLoader::SplitLineByTab(uint32_t dwLine, CTokenVector* pstTokenVector)
{
	pstTokenVector->reserve(10);
	pstTokenVector->clear();

	const std::string & c_rstLine = GetLineString(dwLine);
	const int32_t c_iLineLength = c_rstLine.length();

	if (0 == c_iLineLength)
		return false;

	int32_t basePos = 0;

	do
	{
		int32_t beginPos = c_rstLine.find_first_of("\t", basePos);

		pstTokenVector->push_back(c_rstLine.substr(basePos, beginPos-basePos));

		basePos = beginPos+1;
	} while (basePos < c_iLineLength && basePos > 0);

	return true;
}

int32_t CMemoryTextFileLoader::SplitLine2(uint32_t dwLine, CTokenVector* pstTokenVector, const char * c_szDelimeter)
{
	pstTokenVector->reserve(10);
	pstTokenVector->clear();

	std::string stToken;
	const std::string & c_rstLine = GetLineString(dwLine);

	uint32_t basePos = 0;

	do
	{
		int32_t beginPos = c_rstLine.find_first_not_of(c_szDelimeter, basePos);

		if (beginPos < 0)
			return -1;

		int32_t endPos;

		if (c_rstLine[beginPos] == '"')
		{
			++beginPos;
			endPos = c_rstLine.find_first_of("\"", beginPos);

			if (endPos < 0)
				return -2;

			basePos = endPos + 1;
		}
		else
		{
			endPos = c_rstLine.find_first_of(c_szDelimeter, beginPos);
			basePos = endPos;
		}

		pstTokenVector->push_back(c_rstLine.substr(beginPos, endPos - beginPos));

		// 추가 코드. 맨뒤에 탭이 있는 경우를 체크한다. - [levites]
		if (int32_t(c_rstLine.find_first_not_of(c_szDelimeter, basePos)) < 0)
			break;
	} while (basePos < c_rstLine.length());

	return 0;
}

bool CMemoryTextFileLoader::SplitLine(uint32_t dwLine, CTokenVector* pstTokenVector, const char * c_szDelimeter)
{
	pstTokenVector->reserve(10);
	pstTokenVector->clear();

	std::string stToken;
	const std::string & c_rstLine = GetLineString(dwLine);

	uint32_t basePos = 0;

	do
	{
		int32_t beginPos = c_rstLine.find_first_not_of(c_szDelimeter, basePos);
		if (beginPos < 0)
			return false;

		int32_t endPos;

		if (c_rstLine[beginPos] == '"')
		{
			++beginPos;
			endPos = c_rstLine.find_first_of("\"", beginPos);

			if (endPos < 0)
				return false;
			
			basePos = endPos + 1;
		}
		else
		{
			endPos = c_rstLine.find_first_of(c_szDelimeter, beginPos);
			basePos = endPos;
		}

		pstTokenVector->push_back(c_rstLine.substr(beginPos, endPos - beginPos));

		// 추가 코드. 맨뒤에 탭이 있는 경우를 체크한다. - [levites]
		if (int32_t(c_rstLine.find_first_not_of(c_szDelimeter, basePos)) < 0)
			break;
	} while (basePos < c_rstLine.length());

	return true;
}

uint32_t CMemoryTextFileLoader::GetLineCount()
{
	return m_stLineVector.size();
}

bool CMemoryTextFileLoader::CheckLineIndex(uint32_t dwLine)
{
	if (dwLine >= m_stLineVector.size())
		return false;

	return true;
}

const std::string & CMemoryTextFileLoader::GetLineString(uint32_t dwLine)
{
	assert(CheckLineIndex(dwLine));
	return m_stLineVector[dwLine];
}

void CMemoryTextFileLoader::Bind(int32_t bufSize, const void* c_pvBuf)
{
	m_stLineVector.reserve(128);
	m_stLineVector.clear();

	const char * c_pcBuf = (const char *)c_pvBuf;
	std::string stLine;
	int32_t pos = 0;

	while (pos < bufSize)
	{
		const char c = c_pcBuf[pos++];

		if ('\n' == c || '\r' == c)
		{
			if (pos < bufSize)
				if ('\n' == c_pcBuf[pos] || '\r' == c_pcBuf[pos])
					++pos;

			m_stLineVector.push_back(stLine);
			stLine = "";
		}
		else if (c < 0)
		{
			stLine.append(c_pcBuf + (pos-1), 2);
			++pos;
		}
		else
		{
			stLine += c;
		}
	}

	m_stLineVector.push_back(stLine);
}

//////////////////////////////////////////////////////////////////////////////////////////////////
int32_t CMemoryFileLoader::GetSize()
{
	return m_size;
}

int32_t CMemoryFileLoader::GetPosition()
{
	return m_pos;
}

bool CMemoryFileLoader::IsReadableSize(int32_t size)
{
	if (m_pos + size > m_size)
		return false;

	return true;
}

bool CMemoryFileLoader::Read(int32_t size, void* pvDst)
{
	if (!IsReadableSize(size))
		return false;

	memcpy(pvDst, GetCurrentPositionPointer(), size);
	m_pos += size;
	return true;
}

const char* CMemoryFileLoader::GetCurrentPositionPointer()
{
	assert(m_pcBase != nullptr);
	return (m_pcBase + m_pos);
}

CMemoryFileLoader::CMemoryFileLoader(int32_t size, const void* c_pvMemoryFile)
{
	assert(c_pvMemoryFile != nullptr);

	m_pos = 0;
	m_size = size;
	m_pcBase = (const char *) c_pvMemoryFile;
}

CMemoryFileLoader::~CMemoryFileLoader()
{
}

//////////////////////////////////////////////////////////////////////////////////////////////////
int32_t CDiskFileLoader::GetSize()
{
	return m_size;
}

bool CDiskFileLoader::Read(int32_t size, void* pvDst)
{
	assert(m_fp != nullptr);

	int32_t ret = fread(pvDst, size, 1, m_fp);

	if (ret <= 0)
		return false;

	return true;
}

bool CDiskFileLoader::Open(const char* c_szFileName)
{
	Close();

	if (!c_szFileName[0])
		return false;

	m_fp = fopen(c_szFileName, "rb");

	if (!m_fp)
		return false;

	fseek(m_fp, 0, SEEK_END);
	m_size = ftell(m_fp);
	fseek(m_fp, 0, SEEK_SET);
	return true;
}

void CDiskFileLoader::Close()
{
	if (m_fp)
		fclose(m_fp);

	Initialize();
}

void CDiskFileLoader::Initialize()
{
	m_fp = nullptr;
	m_size = 0;
}

CDiskFileLoader::CDiskFileLoader()
{
	Initialize();
}

CDiskFileLoader::~CDiskFileLoader()
{
	Close();
}
