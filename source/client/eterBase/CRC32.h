#ifndef __INC_CRC32_H__
#define __INC_CRC32_H__

#include <windows.h>
#include <cstdint>

uint32_t GetCRC32(const char* buffer, size_t count);
uint32_t GetCaseCRC32(const char * buf, size_t len);
uint32_t GetHFILECRC32(HANDLE hFile);
uint32_t GetFileCRC32(const char* c_szFileName);
uint32_t GetFileSize(const char* c_szFileName);

#endif
