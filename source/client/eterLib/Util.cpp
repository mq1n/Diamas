#include "StdAfx.h"
#include <FileSystemIncl.hpp>

#include "TextFileLoader.h"

void PrintfTabs(FILE * File, int32_t iTabCount, const char * c_szString, ...)
{
	va_list args;
	va_start(args, c_szString);

	static char szBuf[1024];
	_vsnprintf(szBuf, sizeof(szBuf), c_szString, args);
	va_end(args);

	for (int32_t i = 0; i < iTabCount; ++i)
		fprintf(File, "    ");

	fprintf(File, "%s", szBuf);
}

bool LoadTextData(const char * c_szFileName, CTokenMap & rstTokenMap)
{
	CFile File;
	if (!FileSystemManager::Instance().OpenFile(c_szFileName, File))
		return false;

	CMemoryTextFileLoader textFileLoader;
	CTokenVector stTokenVector;

	textFileLoader.Bind(File.GetSize(), File.GetData());

	for (uint32_t i = 0; i < textFileLoader.GetLineCount(); ++i)
	{
		if (!textFileLoader.SplitLine(i, &stTokenVector))
			continue;

		if (2 != stTokenVector.size())
			return false;

		stl_lowers(stTokenVector[0]);
		stl_lowers(stTokenVector[1]);

		rstTokenMap[stTokenVector[0]] = stTokenVector[1];
	}

	return true;
}

bool LoadMultipleTextData(const char * c_szFileName, CTokenVectorMap & rstTokenVectorMap)
{
	CFile File;
	if (!FileSystemManager::Instance().OpenFile(c_szFileName, File))
		return false;

	uint32_t i;

	CMemoryTextFileLoader textFileLoader;
	CTokenVector stTokenVector;

	textFileLoader.Bind(File.GetSize(), File.GetData());

	for (i = 0; i < textFileLoader.GetLineCount(); ++i)
	{
		if (!textFileLoader.SplitLine(i, &stTokenVector))
			continue;

		stl_lowers(stTokenVector[0]);

		// Start or End
		if ("start" == stTokenVector[0])
		{
			CTokenVector stSubTokenVector;

			stl_lowers(stTokenVector[1]);
			std::string key = stTokenVector[1];
			stTokenVector.clear();

			for (i=i+1; i < textFileLoader.GetLineCount(); ++i)
			{
				if (!textFileLoader.SplitLine(i, &stSubTokenVector))
					continue;

				stl_lowers(stSubTokenVector[0]);

				if ("end" == stSubTokenVector[0])
					break;

				for (auto & j : stSubTokenVector)
					stTokenVector.emplace_back(j);
			}

			rstTokenVectorMap.emplace(key, stTokenVector);
		}
		else
		{
			std::string key = stTokenVector[0];
			stTokenVector.erase(stTokenVector.begin());
			rstTokenVectorMap.emplace(key, stTokenVector);
		}
	}

	return true;
}

D3DXVECTOR3 TokenToVector(CTokenVector & rVector)
{
	if (3 != rVector.size())
	{
		assert(!"Size of token vector which will be converted to vector is not 3");
		return D3DXVECTOR3(0.0f, 0.0f, 0.0f);
	}

	return D3DXVECTOR3(atof(rVector[0].c_str()),
						atof(rVector[1].c_str()),
						atof(rVector[2].c_str()));
}

D3DXCOLOR TokenToColor(CTokenVector & rVector)
{
	if (4 != rVector.size())
	{
		assert(!"Size of token vector which will be converted to color is not 4");
		return D3DXCOLOR(1.0f, 1.0f, 1.0f, 1.0f);
	}

	return D3DXCOLOR(atof(rVector[0].c_str()),
						atof(rVector[1].c_str()),
						atof(rVector[2].c_str()),
						atof(rVector[3].c_str()));
}

///////////////////////////////////////////////////////////////////////////////////////////////////

int32_t __base64_get( const int32_t c )
{
	if( 'A' <= c && c <= 'Z' )
		return c-'A';
	if( 'a' <= c && c <= 'z' )
		return c - 'a' + 26;
	if( '0' <= c && c <= '9' )
		return c - '0' + 52;
	if( c == '+' )
		return 62;
	if( c == '/' )
		return 63;
	if( c == '=' )	// end of line
		return -1;
	return -2;	// non value;
}

void __strcat1(char * str,int32_t i)
{
	char result[2];
	result[0] = static_cast<char>(i);
	result[1] = '\0';
	strcat(str,result);
}

void base64_decode(const char * str,char * resultStr)
{
	int32_t nCount=0, i=0, r, result;
	int32_t length = strlen(str);
	char szDest[5]="";

	strcpy(resultStr,"");
	while(nCount < length)
	{
		i=0;
		strcpy(szDest, "");
		while(nCount<length && i<4)	// 4개의 바이트를 얻는다.
		{			
			r = str[nCount++];
			result = __base64_get(r);
			if(result!=-2)
			{
				if(result!=-1)
					szDest[i++] = static_cast<char>(result);
				else
					szDest[i++] = '@';	// It's end  (64번은 디코딩시 사용되지 않기 때문)
			}
		}

		if(i==4)	// 4개의 소스를 모두 얻어냈다. 디코드 시작
		{	
			if( nCount+3 >= length )	// 데이터의 끝에 도달했다.
			{
				if( szDest[1] == '@' )
				{
					__strcat1(resultStr,(szDest[0]<<2));		
					break;
				}// exit while loop
				else 
					__strcat1(resultStr,(szDest[0]<<2 | szDest[1]>>4));	// 1 Byte
				if( szDest[2] == '@' )
				{
					__strcat1(resultStr,(szDest[1]<<4));
					break;
				}
				else 
					__strcat1(resultStr,(szDest[1]<<4 | szDest[2]>>2));	// 2 Byte				
				if( szDest[3] == '@' )
				{
					__strcat1(resultStr,(szDest[2]<<6));
					break;
				}
				else 
					__strcat1(resultStr,(szDest[2]<<6 | szDest[3]));	// 3 Byte				
			}
			else
			{
				__strcat1(resultStr,(szDest[0]<<2 | szDest[1]>>4));	// 1 Byte
				__strcat1(resultStr,(szDest[1]<<4 | szDest[2]>>2));	// 2 Byte				
				__strcat1(resultStr,(szDest[2]<<6 | szDest[3]));	// 3 Byte
			}
		}
		
	}// end of while

	for (uint32_t x = 0; x < strlen(resultStr); x++)
	{
		char c = resultStr[x];
		int32_t xor = x + 5;
		resultStr[x] = char(c ^ xor);
	}	
	// E
}
