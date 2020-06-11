#include "StdAfx.h"
#if defined(CHECK_LATEST_DATA_FILES)
#include "CheckLatestFiles.h"
#include "resource.h"

static struct SCHECKFILELIST
{
	LPCSTR		szFileName;
	uint32_t		dwCRC32;
	ULONGLONG	ullSize;
} s_astCRC32FileList[] = {
#include "CRC32_inc.h"
	{ nullptr,		0 }
};

static bool gs_bQuit = false;

bool CheckLatestFiles_PollEvent( void )
{
	return gs_bQuit;
}

// 데이타 파일 CRC32 값 검사하기
bool CheckFileCRC32( LPCSTR szFileName, uint32_t dwCRC32 )
{
	char szMessage[256];

	if (_access( szFileName, 4 ) != 0) {
		_snprintf(szMessage, sizeof(szMessage)/sizeof(szMessage[0])-1, (IDS_ERR_CANNOT_READ_FILE, "ERR_CANNOT_READ_FILE"), szFileName);
		ApplicationSetErrorString(szMessage);
		return false;
	}
	uint32_t dwLocalCRC32 = GetFileCRC32(szFileName);

	if (dwCRC32 != dwLocalCRC32) {
		_snprintf(szMessage, sizeof(szMessage)/sizeof(szMessage[0])-1, (IDS_ERR_NOT_LATEST_FILE, "ERR_NOT_LATEST_FILE"), szFileName);
		ApplicationSetErrorString(szMessage);
		return false;
	}

	return true;
}

uint32_t CALLBACK CheckLatestFilesEntry(void * pThis)
{
	::Sleep( 500 );
	for( int32_t i=0; s_astCRC32FileList[i].szFileName != nullptr; i++ ) {
		if (false == CheckFileCRC32(s_astCRC32FileList[i].szFileName, s_astCRC32FileList[i].dwCRC32)) {
			gs_bQuit = true;
			break;
		}
	}
	return 0;
}

bool CheckLatestFiles( void )
{
	uint32_t uThreadID = 0;

	HANDLE hThread = (HANDLE) _beginthreadex(nullptr, 0, CheckLatestFilesEntry, nullptr, 0, &uThreadID);
	::SetThreadPriority(hThread, THREAD_PRIORITY_LOWEST);
	return true;
}
#endif
