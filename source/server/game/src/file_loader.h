#pragma once

typedef std::vector< std::string > TTokenVector;

class CMemoryTextFileLoader
{
	public:
		CMemoryTextFileLoader();
		virtual ~CMemoryTextFileLoader();

		void			Bind(int32_t bufSize, const void* c_pvBuf);
		uint32_t			GetLineCount();
		bool			CheckLineIndex(uint32_t dwLine);
		bool			SplitLine(uint32_t dwLine, std::vector<std::string> * pstTokenVector, const char * c_szDelimeter = " \t");
		const std::string &	GetLineString(uint32_t dwLine);

	protected:
		std::vector<std::string> m_stLineVector;
};
