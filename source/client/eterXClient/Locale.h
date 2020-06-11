#pragma once

#include "Locale_inc.h"

bool		LocaleService_IsYMIR();
bool		LocaleService_IsJAPAN();
bool		LocaleService_IsENGLISH();
bool		LocaleService_IsHONGKONG();
bool		LocaleService_IsTAIWAN();
bool		LocaleService_IsNEWCIBN();
bool		LocaleService_IsEUROPE();
bool		LocaleService_IsWorldEdition();

uint32_t	LocaleService_GetCodePage();
const char* LocaleService_GetName();
const char*	LocaleService_GetLocaleName();
const char*	LocaleService_GetLocalePath();
const char*	LocaleService_GetSecurityKey();
BOOL		LocaleService_IsLeadByte( const char chByte );
int32_t			LocaleService_StringCompareCI( LPCSTR szStringLeft, LPCSTR szStringRight, size_t sizeLength );

void		LocaleService_ForceSetLocale(const char* name, const char* localePath);
void		LocaleService_LoadConfig(const char* fileName);
bool		LocaleService_LoadGlobal(HINSTANCE hInstance);
uint32_t	LocaleService_GetLastExp(int32_t level);
int32_t			LocaleService_GetSkillPower(uint32_t level);

// CHEONMA
void		LocaleService_SetCHEONMA(bool isEnable);
bool		LocaleService_IsCHEONMA();
// END_OF_CHEONMA
