#include "StdAfx.h"
#include "AnticheatManager.h"
#include "CheatQueueManager.h"
#include "NTDefinations.h"
#include <detours.h>
#include <xorstr.hpp>

typedef ULONG(NTAPI* TRtlGetFullPathName_U)(PCWSTR FileName, ULONG Size, PWSTR Buffer, PWSTR* ShortName);
static TRtlGetFullPathName_U RtlGetFullPathName_U = nullptr;
static DWORD gs_dwRtlGetFullPathName_U = 0;

BYTE pHookByteBackup[5] = { 0 };

PVOID GetModuleAddressFromName(const wchar_t* c_wszName)
{
	auto pPEB = GetCurrentPEB();
	PLDR_DATA_TABLE_ENTRY_MY Current = NULL;
	PLIST_ENTRY CurrentEntry = pPEB->Ldr->InMemoryOrderModuleList.Flink;

	while (CurrentEntry != &pPEB->Ldr->InMemoryOrderModuleList && CurrentEntry != NULL)
	{
		Current = CONTAINING_RECORD(CurrentEntry, LDR_DATA_TABLE_ENTRY_MY, InMemoryOrderLinks);

		if (wcsstr(Current->FullDllName.Buffer, c_wszName))
			return Current->DllBase;

		CurrentEntry = CurrentEntry->Flink;
	}
	return nullptr;
}

ULONG NTAPI RtlGetFullPathName_UTrampoline(PCWSTR FileName, ULONG Size, PWSTR Buffer, PWSTR* ShortName)
{
	auto pModuleBase = GetModuleAddressFromName(FileName);
	if (pModuleBase)
	{
		auto wstName = std::wstring(FileName);
		auto stName = std::string(wstName.begin(), wstName.end());
		
		CCheatDetectQueueMgr::Instance().AppendDetection(DLL_INJECTION, 0, stName);
		// CAnticheatManager::Instance().ExitByAnticheat(DLL_INJECTION, 0, 0, true, stName);
	}

	return RtlGetFullPathName_U(FileName, Size, Buffer, ShortName);
}

bool CAnticheatManager::LoadDllFilter()
{
	auto hNtdll = LoadLibraryA(xorstr("ntdll").crypt_get());
	if (!hNtdll)
		return false;

	gs_dwRtlGetFullPathName_U = (DWORD)GetProcAddress(hNtdll, xorstr("RtlGetFullPathName_U").crypt_get());
	if (!gs_dwRtlGetFullPathName_U)
		return false;

	auto pFirstByte = *(BYTE*)gs_dwRtlGetFullPathName_U;
	if (pFirstByte != 0x8B) // hooked? or just mismatch?
	{
		CCheatDetectQueueMgr::Instance().AppendDetection(CORRUPTED_WINAPI, pFirstByte, "1");
		return false;
	}

	RtlGetFullPathName_U = (TRtlGetFullPathName_U)DetourFunction((PBYTE)gs_dwRtlGetFullPathName_U, (PBYTE)RtlGetFullPathName_UTrampoline);
	if (!RtlGetFullPathName_U)
	{
#ifdef _DEBUG
		TraceError("RtlGetFullPathName_U hook failed");
#endif
		return false;
	}

	RtlCopyMemory(pHookByteBackup, (LPVOID)gs_dwRtlGetFullPathName_U, sizeof(pHookByteBackup));
	return true;
}

bool CAnticheatManager::IsDllFilterHookCorrupted()
{
	if (pHookByteBackup[0] && memcmp((LPCVOID)gs_dwRtlGetFullPathName_U, pHookByteBackup, sizeof(pHookByteBackup)))
		return true;
	return false;
}
