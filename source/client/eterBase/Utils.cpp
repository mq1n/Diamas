#include "StdAfx.h"

#include <stdlib.h>
#include <direct.h>
#include <io.h>
#include <assert.h>
#include <sys/stat.h>
#include "Utils.h"
#include "filedir.h"

char korean_tolower(const char c);

const char * CreateTempFileName(const char * c_pszPrefix)
{
	char szTempPath[MAX_PATH + 1];
	static char szTempName[MAX_PATH + 1];

	GetTempPath(MAX_PATH, szTempPath);

	GetTempFileName(szTempPath,									// directory for temp files 
					c_pszPrefix ? c_pszPrefix : "etb",		    // temp file name prefix 
					c_pszPrefix ? true : false,					// create unique name 
					szTempName);								// buffer for name 

	return (szTempName);
}

void GetFilePathNameExtension(const char * c_szFile, int32_t len, std::string * pstPath, std::string * pstName, std::string * pstExt)
{
	assert(pstPath != nullptr);
	assert(pstName != nullptr);
	assert(pstExt != nullptr);

	int32_t ext = len;
	int32_t pos = len;

	while (pos > 0)
	{
		--pos;
		char c = c_szFile[pos];

		if (ext == len && c == '.') 
		{
			ext = pos;			
			break;
		}
		
		if (c == '/' || c == '\\')
			break;
	}

	while (pos > 0)
	{
		--pos;
		char c = c_szFile[pos];

		if (c == '/' || c == '\\')
			break;
	}

	if (pos)
	{
		++pos;
		pstPath->append(c_szFile, pos);
	}
	
	if (ext > pos)
		pstName->append(c_szFile + pos, ext - pos);

	++ext;

	if (len > ext)
		pstExt->append(c_szFile + ext, len - ext);
}

void GetFileExtension(const char* c_szFile, int32_t len, std::string* pstExt)
{
	int32_t ext = len;
	int32_t pos = len;
	while (pos > 0)
	{
		--pos;
		char c=c_szFile[pos];
		if (ext==len && c=='.') 
		{
			ext=pos;			
			break;
		}

		if (c=='/') break;
		else if (c=='\\') break;
	}

	++ext;		
	if (len>ext)
		pstExt->append(c_szFile+ext, len-ext);
}

void GetFileNameParts(const char* c_szFile, int32_t len, char* pszPath, char* pszName, char* pszExt)
{
	assert(pszPath!=nullptr);
	assert(pszName!=nullptr);
	assert(pszExt!=nullptr);

	int32_t ext=len;
	int32_t pos=len;
	while (pos>0)
	{
		--pos;
		char c=c_szFile[pos];
		if (ext==len && c=='.') 
		{
			ext=pos;			
			break;
		}

		if (c=='/') break;
		else if (c=='\\') break;
	}

	while (pos>0)
	{
		--pos;
		char c=c_szFile[pos];

		if (c=='/') break;
		else if (c=='\\') break;
	}

	if (pos) 
	{
		++pos;
		for (int32_t i = 0; i < pos; ++i)
		{
			pszPath[i] = c_szFile[i];
		}
		pszPath[pos] = '\0';
	}

	if (ext>pos)
	{
		int32_t count = 0;
		for (int32_t i = pos; i < ext; ++i)
		{
			pszName[count++] = c_szFile[i];
		}
		pszName[count] = '\0';
	}

	++ext;		
	if (len > ext)
	{
		int32_t count = 0;
		for (int32_t i = ext; i < len; ++i)
		{
			pszExt[count++] = c_szFile[i];
		}
		pszExt[count] = '\0';
	}
}

void GetOldIndexingName(char * szName, int32_t Index)
{
	int32_t dec, sign;
	char Temp[512];
	
	strcpy(Temp, _ecvt(Index, 256, &dec, &sign));
	Temp[dec] = '\0';
	
	strcat(szName, Temp);
}

void GetIndexingName(char * szName, uint32_t Index)
{
	sprintf(szName + strlen(szName), "%u", Index);
}

void GetOnlyFileName(const char * sz_Name, std::string & strFileName)
{
	strFileName = "";

	int32_t i;
	for (i=strlen(sz_Name)-1; i>=0; --i)
	{
		if ('\\' == sz_Name[i] || '/' == sz_Name[i])
		{
			++i;
			break;
		}
	}
	
	if (i == -1)
		i = 0;

	for (size_t j = i; j < strlen(sz_Name); ++j)
	{
		strFileName += sz_Name[j];
	}

	strFileName += "\0";
}

void GetExceptionPathName(const char * sz_Name, std::string & OnlyFileName)
{
	GetOnlyFileName(sz_Name, OnlyFileName);
}

void GetOnlyPathName(const char * sz_Name, std::string & OnlyPathName)
{
	int32_t i;
	for (i = strlen(sz_Name) - 1; i >= 0; --i)
	{
		if ('\\' == sz_Name[i] || '/' == sz_Name[i])
		{
			++i;
			break;
		}
	}

	if (i == -1)
		i = 0;

	OnlyPathName.reserve(strlen(sz_Name));
	OnlyPathName = "";

	for (int32_t j=0; j<i; ++j)
	{
		OnlyPathName += sz_Name[j];
	}
	OnlyPathName += "\0";
}

const char * GetOnlyPathName(const char * c_szName)
{
	static std::string strPathName;
	GetOnlyPathName(c_szName, strPathName);

	return strPathName.c_str();
}

bool GetLocalFileName(const char * c_szGlobalPath, const char * c_szFullPathFileName, std::string * pstrLocalFileName)
{
	std::string strLocalFileName;

	std::string strGlobalPath;
	std::string strFullPathFileName;
	StringPath(c_szGlobalPath, strGlobalPath);
	StringPath(c_szFullPathFileName, strFullPathFileName);

	if (strGlobalPath.length() >= strFullPathFileName.length())
		return false;

	uint32_t length = std::min(strGlobalPath.length(), strFullPathFileName.length());
	for (uint32_t dwPos = 0; dwPos < length; ++dwPos)
	{
		if (strGlobalPath[dwPos] != strFullPathFileName[dwPos])
			return false;
	}

	*pstrLocalFileName = &c_szFullPathFileName[length];

	return true;
}

void GetWorkingFolder(std::string & strFileName)
{
	char buf[128+1];
	_getcwd(buf, 128);
	strcat(buf, "/");
	strFileName = buf;
}

void StringLowers(char * String)
{
	for (uint32_t i = 0; i < strlen(String); ++i)
	{
		String[i] = korean_tolower(String[i]);
	}
}

void StringPath(std::string & rString)
{
	for (uint32_t i = 0; i < rString.length(); ++i)
	{
		if (rString[i] == '\\')
			rString[i] = '/';
		else
			rString[i] = korean_tolower(rString[i]);
	}
}

void StringPath(char * pString)
{
	for (uint32_t i = 0; i < strlen(pString); ++i)
	{
		if (pString[i] == '\\')
			pString[i] = '/';
		else
			pString[i] = korean_tolower(pString[i]);
	}
}

void StringPath(const char * c_szSrc, char * szDest)
{
	for (uint32_t i = 0; i < strlen(c_szSrc); ++i)
	{
		if (c_szSrc[i] == '\\')
			szDest[i] = '/';
		else
			szDest[i] = korean_tolower(c_szSrc[i]);
	}
}

void StringPath(const char * c_szSrc, std::string & rString)
{
	rString = "";
	rString.resize(strlen(c_szSrc));

	for (uint32_t i = 0; i < strlen(c_szSrc); ++i)
	{
		if (c_szSrc[i] == '\\')
			rString[i] = '/';
		else
			rString[i] = korean_tolower(c_szSrc[i]);
	}
}

#define ishprint(x) ((((x) & 0xE0) > 0x90) || isprint(x))

void PrintAsciiData(const void* void_data, int32_t bytes)
{
    int32_t                 i, j, k;
    const uint8_t* p;
	const uint8_t* data;

	data = (const uint8_t*) void_data;

    fprintf(stdout, "------------------------------------------------------------------\n");
    j = bytes;
    while (1)
    {
        k = j >= 16 ? 16 : j;

        p = data;
        for (i = 0; i < 16; ++i)
        {
            if (i >= k)
                fprintf(stdout, "   ");
            else
                fprintf(stdout, "%02x ", *p);
            p++;
        }

        fprintf(stdout, "| ");

        p = data;
        for (i = 0; i < k; ++i)
        {
            if (i >= k)
                fprintf(stdout, " ");
            else
                fprintf(stdout, "%c", ishprint(*p) ? *p : '.');
            p++;
        }

        fprintf(stdout, "\n");

        j       -= 16;
        data    += 16;

        if (j <= 0)
            break;
    }

    fprintf(stdout, "------------------------------------------------------------------\n");
}

int32_t MIN(int32_t a, int32_t b)   
{   
    return a < b ? a : b;
}   

int32_t MAX(int32_t a, int32_t b)
{       
    return a > b ? a : b;
}       

int32_t MINMAX(int32_t min, int32_t value, int32_t max)
{
	if (max < min)
		return MAX(min, value);

    register int32_t tv;
    tv = (min > value ? min : value);
    return (max < tv) ? max : tv;
}

float fMIN(float a, float b)   
{   
    return a < b ? a : b;
}   

float fMAX(float a, float b)
{       
    return a > b ? a : b;
}       

float fMINMAX(float min, float value, float max)
{               
    register float tv;
	
    tv = (min > value ? min : value);
    return (max < tv) ? max : tv;
}

bool IsFile(const char* filename)
{
	return _access(filename, 0) == 0 ? true : false;
}

bool IsGlobalFileName(const char * c_szFileName)
{
	return strchr(c_szFileName, ':') != nullptr;
}

void MyCreateDirectory(const char* path)
{
	if (!path || !*path)
		return;

	char * dir;
	const char * p;

	if (strlen(path) >= 3)
	{
		if (*(path + 1) == ':')	// C:, D: 같은 경우를 체크
			path += 3;
	}

	p = path;

	int32_t len = strlen(path) + 1;
	dir = new char[len];

	while (*p)
	{
		if (*p == '/' || *p == '\\')
		{
			memset(dir, 0, len);
			strncpy(dir, path, p - path);
			CreateDirectory(dir, nullptr);
		}

		++p;
	}

	delete [] dir;
}

class CDirRemover : public CDir
{
	public:
		CDirRemover()
		{
		}
		virtual ~CDirRemover()
		{
		}

		bool OnFolder(const char* c_szFilter, const char* c_szPathName, const char* c_szFileName)
		{
			std::string strFullPathName;
			strFullPathName = c_szPathName;
			strFullPathName += c_szFileName; 

			CDirRemover remover;
			remover.Create(c_szFilter, strFullPathName.c_str());

			std::string strWorkingFolder;
			GetWorkingFolder(strWorkingFolder);
			strWorkingFolder += strFullPathName;
			strWorkingFolder += "/";
			StringPath(strWorkingFolder);
			ms_strDirectoryDeque.push_back(strWorkingFolder);
			return true;
		}
		bool OnFile(const char* c_szPathName, const char* c_szFileName)
		{
			std::string strFullPathName;
			strFullPathName = c_szPathName;
			strFullPathName += c_szFileName; 
			_chmod(strFullPathName.c_str(), _S_IWRITE);
			DeleteFileA(strFullPathName.c_str());
			return true;
		}

		static void RemoveAllDirectory()
		{
			for (std::deque<std::string>::iterator itor = ms_strDirectoryDeque.begin(); itor != ms_strDirectoryDeque.end(); ++itor)
			{
				const std::string & c_rstrDirectory = *itor;
				RemoveDirectoryA(c_rstrDirectory.c_str());
			}

			ms_strDirectoryDeque.clear();
		}

	protected:
		static std::deque<std::string> ms_strDirectoryDeque;
};

std::deque<std::string> CDirRemover::ms_strDirectoryDeque;

void RemoveAllDirectory(const char * c_szDirectoryName)
{
	{
		CDirRemover remover;
		remover.Create("*.*", c_szDirectoryName);
		CDirRemover::RemoveAllDirectory();
	}
	RemoveDirectory(c_szDirectoryName);
}

void StringExceptCharacter(std::string * pstrString, const char * c_szCharacter)
{
	int32_t icurPos = 0;
	int32_t iNextPos = 0;

	while((iNextPos = pstrString->find_first_of(c_szCharacter, icurPos)) >= 0)
	{
		std::string strFront = pstrString->substr(icurPos, iNextPos - icurPos);
		std::string strBack = pstrString->substr(iNextPos+1, pstrString->length() - iNextPos - 1);
		*pstrString = strFront + strBack;
	}
}

bool SplitLine(const char * c_szLine, const char * c_szDelimeter, std::vector<std::string> * pkVec_strToken)
{
	pkVec_strToken->reserve(10);
	pkVec_strToken->clear();

	std::string strLine = c_szLine;

	uint32_t basePos = 0;

	do
	{
		int32_t beginPos = strLine.find_first_not_of(c_szDelimeter, basePos);
		if (beginPos < 0)
			return false;

		int32_t endPos;

		if (strLine[beginPos] == '"')
		{
			++beginPos;
			endPos = strLine.find_first_of("\"", beginPos);

			if (endPos < 0)
				return false;
			
			basePos = endPos + 1;
		}
		else
		{
			endPos = strLine.find_first_of(c_szDelimeter, beginPos);
			basePos = endPos;
		}

		pkVec_strToken->push_back(strLine.substr(beginPos, endPos - beginPos));
	} while (basePos < strLine.length());

	return true;
}

void GetExcutedFileName(std::string & r_str)
{
	char szPath[MAX_PATH+1];

	GetModuleFileName(nullptr, szPath, MAX_PATH);
	szPath[MAX_PATH] = '\0';

	r_str = szPath;
}

const char * _getf(const char* c_szFormat, ...)
{
	static char szBuf[256];

	va_list args;
	va_start(args, c_szFormat);
	_vsnprintf(szBuf, sizeof(szBuf), c_szFormat, args);
	va_end(args);

	return szBuf;
}

PCHAR* CommandLineToArgv( PCHAR CmdLine, int32_t* _argc )
{
	PCHAR* argv;
	PCHAR  _argv;
	ULONG   len;
	ULONG   argc;
	CHAR   a;
	ULONG   i, j;

	BOOLEAN  in_QM;
	BOOLEAN  in_TEXT;
	BOOLEAN  in_SPACE;

	len = strlen(CmdLine);
	i = ((len+2)/2)*sizeof(PVOID) + sizeof(PVOID);

	argv = (PCHAR*)GlobalAlloc(GMEM_FIXED,
		i + (len+2)*sizeof(CHAR));

	_argv = (PCHAR)(((PUCHAR)argv)+i);

	argc = 0;
	argv[argc] = _argv;
	in_QM = FALSE;
	in_TEXT = FALSE;
	in_SPACE = TRUE;
	i = 0;
	j = 0;

	while( (a = CmdLine[i]) != 0 ) {
		if(in_QM) {
			if(a == '\"') {
				in_QM = FALSE;
			} else {
				_argv[j] = a;
				j++;
			}
		} else {
			switch(a) {
				case '\"':
					in_QM = TRUE;
					in_TEXT = TRUE;
					if(in_SPACE) {
						argv[argc] = _argv+j;
						argc++;
					}
					in_SPACE = FALSE;
					break;
				case ' ':
				case '\t':
				case '\n':
				case '\r':
					if(in_TEXT) {
						_argv[j] = '\0';
						j++;
					}
					in_TEXT = FALSE;
					in_SPACE = TRUE;
					break;
				default:
					in_TEXT = TRUE;
					if(in_SPACE) {
						argv[argc] = _argv+j;
						argc++;
					}
					_argv[j] = a;
					j++;
					in_SPACE = FALSE;
					break;
			}
		}
		i++;
	}
	_argv[j] = '\0';
	argv[argc] = nullptr;

	(*_argc) = argc;
	return argv;
}
