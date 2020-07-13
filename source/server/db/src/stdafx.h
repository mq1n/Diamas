#pragma once
#include "../../libthecore/include/stdafx.h"
#include "../../../common/common_incl.hpp"

#include <cmath>
#include <sstream>
#include <array>

#ifndef _WIN32
#include <semaphore.h>
#else
#define isdigit iswdigit
#define isspace iswspace
#define isalpha iswalpha
#endif

#include "../../common/tables.h"
