#include "StdAfx.h"
#include "Locale.h"
#include "PythonApplication.h"
#include "resource.h"
#include "../eterBase/CRC32.h"
#include <windowsx.h>

const char* LSS_YMIR		= "YMIR";
const char* LSS_JAPAN		= "JAPAN";
const char* LSS_ENGLISH		= "ENGLISH";
const char* LSS_HONGKONG	= "HONGKONG";
const char* LSS_TAIWAN		= "TAIWAN";
const char* LSS_NEWCIBN		= "NEWCIBN";
const char* LSS_EUROPE		= "EUROPE";
const char* LSS_GLOBAL		= "GLOBAL";

static bool IS_CHEONMA = false;

std::string __SECURITY_KEY_STRING__ = LSS_SECURITY_KEY;

char	MULTI_LOCALE_SERVICE[256]	= "YMIR";
char	MULTI_LOCALE_PATH[256]		= "locale/ymir";
char	MULTI_LOCALE_NAME[256]		= "ymir";
int		MULTI_LOCALE_CODE			= 949;
int		MULTI_LOCALE_REPORT_PORT	= 10000;

void LocaleService_LoadConfig(const char* fileName)
{
	FILE* fp = fopen(fileName, "rt");

	if (fp)
	{		
		char	line[256];			
		char	name[256];
		int		code;
		int		id;
		if (fgets(line, sizeof(line)-1, fp))
		{
			line[sizeof(line)-1] = '\0';
			sscanf(line, "%d %d %s", &id, &code, name);

			MULTI_LOCALE_REPORT_PORT		= id;
			MULTI_LOCALE_CODE				= code;
			strcpy(MULTI_LOCALE_NAME, name);
			sprintf(MULTI_LOCALE_PATH, "locale/%s", MULTI_LOCALE_NAME);
		}			
		fclose(fp);
	}
}

unsigned LocaleService_GetLastExp(int level)
{
	static const int GUILD_LEVEL_MAX = 20;

	if (LocaleService_IsCHEONMA())
	{
		static DWORD CHEONMA_GUILDEXP_LIST[GUILD_LEVEL_MAX+1] = 
		{
			0,			// 0
			15000ul,	// 1
			45000ul,	// 2
			90000ul,	// 3
			160000ul,	// 4
			235000ul,	// 5
			325000ul,	// 6
			430000ul,	// 7
			550000ul,	// 8
			685000ul,	// 9
			835000ul,	// 10
			1000000ul,	// 11
			1500000ul,	// 12
			2100000ul,	// 13
			2800000ul,	// 14
			3600000ul,	// 15
			4500000ul,	// 16
			6500000ul,	// 17
			8000000ul,	// 18
			10000000ul,	// 19			
			42000000UL	// 20
		};
		if (level < 0 && level >= GUILD_LEVEL_MAX)
			return 0;
		
		return CHEONMA_GUILDEXP_LIST[level];
	}
	
	static DWORD INTERNATIONAL_GUILDEXP_LIST[GUILD_LEVEL_MAX+1] = 
	{
		0,			// 0
		6000UL,		// 1
		18000UL,	// 2
		36000UL,	// 3
		64000UL,	// 4
		94000UL,	// 5
		130000UL,	// 6
		172000UL,	// 7
		220000UL,	// 8
		274000UL,	// 9
		334000UL,	// 10
		400000UL,	// 11
		600000UL,	// 12
		840000UL,	// 13
		1120000UL,	// 14
		1440000UL,	// 15
		1800000UL,	// 16
		2600000UL,	// 17
		3200000UL,	// 18
		4000000UL,	// 19		
		16800000UL	// 20		
	};

	if (level < 0 && level >= GUILD_LEVEL_MAX)
		return 0;
	
	return INTERNATIONAL_GUILDEXP_LIST[level];	
}

int LocaleService_GetSkillPower(unsigned level)
{
	static const unsigned SKILL_POWER_NUM = 50;

	if (level >= SKILL_POWER_NUM)
		return 0;

	if (LocaleService_IsCHEONMA())
	{
		static unsigned CHEONMA_SKILL_POWERS[SKILL_POWER_NUM]=
		{
			0,  
				5,  7,  9, 11, 13, 
				15, 17, 19, 20, 22, 
				24, 26, 28, 30, 32, 
				34, 36, 38, 40, 50, // master
				52, 55, 58, 61, 63,
				66, 69, 72, 75, 80, // grand_master
				82, 84, 87, 90, 95,
				100,110,120,130,150,// perfect_master
				150,
		};
		return CHEONMA_SKILL_POWERS[level];		
	}
	
	// 0 5 6 8 10 12 14 16 18 20 22 24 26 28 30 32 34 36 38 40 50 52 54 56 58 60 63 66 69 72 82 85 88 91 94 98 102 106 110 115 125 125 125 125 125
	static unsigned INTERNATIONAL_SKILL_POWERS[SKILL_POWER_NUM]=
	{
		0, 
			5,  6,  8, 10, 12, 
			14, 16, 18, 20, 22, 
			24, 26, 28, 30, 32, 
			34, 36, 38, 40, 50, // master
			52, 54, 56, 58, 60, 
			63, 66, 69, 72, 82, // grand_master
			85, 88, 91, 94, 98, 
			102,106,110,115,125,// perfect_master
			125,	
	};
	return INTERNATIONAL_SKILL_POWERS[level];
}

const char*	LocaleService_GetSecurityKey()
{
	return __SECURITY_KEY_STRING__.c_str();
}

// CHEONMA
void LocaleService_SetCHEONMA(bool isEnable)
{
	IS_CHEONMA = isEnable;
}

bool LocaleService_IsCHEONMA()
{
	return LocaleService_IsYMIR();
}

// END_OF_CHEONMA

#define _LSS_USE_LOCALE_CFG			1
#define _LSS_SERVICE_NAME			LSS_EUROPE

#if defined(_LSS_USE_LOCALE_CFG)
#if defined(_LSS_SERVICE_NAME)
const char* LocaleService_GetName()				{ return _LSS_SERVICE_NAME;}
#else
const char* LocaleService_GetName()				{ return MULTI_LOCALE_SERVICE; }
#endif
unsigned int LocaleService_GetCodePage()		{ return MULTI_LOCALE_CODE; }
const char*	LocaleService_GetLocaleName()		{ return MULTI_LOCALE_NAME; }
const char*	LocaleService_GetLocalePath()		{ return MULTI_LOCALE_PATH; }
#elif defined(_LSS_SERVICE_NAME)
const char* LocaleService_GetName()				{ return _LSS_SERVICE_NAME;}
unsigned int LocaleService_GetCodePage()		{ return _LSS_SERVICE_CODEPAGE; }
const char*	LocaleService_GetLocaleName()		{ return _LSS_SERVICE_LOCALE_NAME; }
const char*	LocaleService_GetLocalePath()		{ return _LSS_SERVICE_LOCALE_PATH; }
#endif

void LocaleService_ForceSetLocale(const char* name, const char* localePath)
{
	strcpy(MULTI_LOCALE_NAME, name);
	strcpy(MULTI_LOCALE_PATH, localePath);
}

bool LocaleService_LoadGlobal(HINSTANCE hInstance)
{
	return false;
}

bool LocaleService_IsYMIR()		{ return (stricmp( LocaleService_GetName(), LSS_YMIR ) == 0) || (stricmp( LocaleService_GetLocaleName(), "ymir" ) == 0);	}
bool LocaleService_IsJAPAN()	{ return (stricmp( LocaleService_GetName(), LSS_JAPAN ) == 0) || (stricmp( LocaleService_GetLocaleName(), "japan" ) == 0);	}
bool LocaleService_IsENGLISH()	{ return (stricmp( LocaleService_GetName(), LSS_ENGLISH ) == 0);	}
bool LocaleService_IsEUROPE()	{ return (stricmp( LocaleService_GetName(), LSS_EUROPE ) == 0);		}
bool LocaleService_IsHONGKONG()	{ return (stricmp( LocaleService_GetName(), LSS_HONGKONG ) == 0);	}
bool LocaleService_IsTAIWAN()	{ return (stricmp( LocaleService_GetName(), LSS_TAIWAN ) == 0);		}
bool LocaleService_IsNEWCIBN()	{ return (stricmp( LocaleService_GetName(), LSS_NEWCIBN ) == 0);	}


BOOL LocaleService_IsLeadByte( const char chByte )
{
	return (((unsigned char) chByte) & 0x80) != 0;
}

int LocaleService_StringCompareCI( LPCSTR szStringLeft, LPCSTR szStringRight, size_t sizeLength )
{
	return strnicmp( szStringLeft, szStringRight, sizeLength );
}
