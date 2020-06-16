#pragma once
#include "../../libthecore/include/stdafx.h"

#ifndef _WIN32
#include <semaphore.h>
#else
#define isdigit iswdigit
#define isspace iswspace
#define isalpha iswalpha
#endif

#include "../../common/service.h"
#include "../../common/length.h"
#include "../../common/tables.h"
#include "../../common/singleton.h"
#include "../../common/utils.h"
#include "../../common/stl.h"
