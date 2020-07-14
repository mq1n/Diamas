#pragma once
#define _USE_32BIT_TIME_T

#include <cstdio>
#include <ctime>
#include <iostream>
#include <string>
#include <set>
#include <map>
#include <list>
#include <algorithm>

#include <../../source/common/common_incl.hpp>
#include <../../source/client/eterBase/lzo.h>
#include <../../source/client/eterBase/tea.h>
#include <../../source/server/db/src/CsvReader.h>
#include <../../source/server/db/src/ProtoReader.h>

static bool gs_bPrintDebug = false;

#define strncpy(a,b,c) strncpy_s(a,c,b,_TRUNCATE)
#define _snprintf(a,b,c,...) _snprintf_s(a,b,_TRUNCATE,c,__VA_ARGS__)

#define ENABLE_ADDONTYPE_AUTODETECT
