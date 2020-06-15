#pragma once

#pragma warning(disable:4786)	// character 255 넘어가는거 끄기

#include <Windows.h>
#include <vector>
#include <map>

#include "Stl.h"

class CMemoryTextFileLoader
{
	public:
		CMemoryTextFileLoader();
		virtual ~CMemoryTextFileLoader();

		void				Bind(int32_t bufSize, const void* c_pvBuf);
		uint32_t GetLineCount() const;
		bool CheckLineIndex(uint32_t dwLine) const;
		bool				SplitLine(uint32_t dwLine, CTokenVector * pstTokenVector, const char * c_szDelimeter = " \t");
		int32_t					SplitLine2(uint32_t dwLine, CTokenVector * pstTokenVector, const char * c_szDelimeter = " \t");
		bool				SplitLineByTab(uint32_t dwLine, CTokenVector* pstTokenVector);
		bool SplitLineByTabNew(uint32_t dwLine, CTokenVector * pstTokenVector);
		const std::string &	GetLineString(uint32_t dwLine);

	protected:
		std::vector<std::string> m_stLineVector;
};

class CMemoryFileLoader
{
	public:
		CMemoryFileLoader(int32_t size, const void * c_pvMemoryFile);
		virtual ~CMemoryFileLoader();

		bool Read(int32_t size, void* pvDst);

		int32_t GetPosition() const;
		int32_t GetSize() const;

	protected:
		bool IsReadableSize(int32_t size) const;
		const char * GetCurrentPositionPointer() const;

	protected:
		const char *	m_pcBase;
		int32_t				m_size;
		int32_t				m_pos;
};

//////////////////////////////////////////////////////////////////////////////////////////////////
class CDiskFileLoader
{
	public:
		CDiskFileLoader();
		virtual ~CDiskFileLoader();

		void Close();
		bool Open(const char * c_szFileName);
		bool Read(int32_t size, void * pvDst) const;

		int32_t GetSize() const;

	protected:
		void Initialize();

	protected:
		FILE *	m_fp;
		int32_t		m_size;
};

typedef std::map<std::string, std::string> TStringMap;