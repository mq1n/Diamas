#pragma once

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN		// Exclude rarely-used stuff from Windows headers
#endif
#ifndef _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#endif

#include <Windows.h>
#include <cassert>
#include <cstdio>
#include <mmsystem.h>
#include <ctime>
#include <cstdint>
#include <algorithm>
#include <string>
#include <vector>
#include <deque>
#include <list>
#include <map>
#include <fstream>
#include <fmt/format.h>

#include "../../common/common_incl.hpp"
using namespace std::string_literals;

#if _MSC_VER >= 1400
#define stricmp _stricmp
#define strnicmp _strnicmp
#define strupt _strupr
#define strcmpi _strcmpi
#define fileno _fileno
#define atoi _atoi64
#endif

#include "vk.h"
#include "Filename.h"
