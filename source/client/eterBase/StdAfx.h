#pragma once

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN		// Exclude rarely-used stuff from Windows headers
#endif
#ifndef _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#endif

#include "../eterXClient/Locale_inc.h"
#include <windows.h>
#include <assert.h>
#include <stdio.h>
#include <mmsystem.h>
#include <time.h>

#include <algorithm>
#include <string>
#include <vector>
#include <deque>
#include <list>
#include <map>
using namespace std::string_literals;

#if _MSC_VER >= 1400
#define stricmp _stricmp
#define strnicmp _strnicmp
#define strupt _strupr
#define strcmpi _strcmpi
#define fileno _fileno
//#define access _access_s
//#define _access _access_s
#define atoi _atoi64
#endif

#include "vk.h"
#include "filename.h"
#include "ServiceDefs.h"
