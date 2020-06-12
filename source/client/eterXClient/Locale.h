#pragma once

#include "Locale_inc.h"

const char*	LocaleService_GetLocaleName();
const char*	LocaleService_GetLocalePath();
const char*	LocaleService_GetSecurityKey();

BOOL		LocaleService_IsLeadByte( const char chByte );
int32_t			LocaleService_StringCompareCI( LPCSTR szStringLeft, LPCSTR szStringRight, size_t sizeLength );

void		LocaleService_ForceSetLocale(const char* name, const char* localePath);
void		LocaleService_LoadConfig(const char* fileName);
uint32_t	LocaleService_GetLastExp(int32_t level);
int32_t			LocaleService_GetSkillPower(uint32_t level);

