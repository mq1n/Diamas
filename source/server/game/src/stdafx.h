#pragma once
#include "../../libthecore/include/stdafx.h"
#include "../../../common/common_incl.hpp"

#include <cmath>
#include <cfloat>
#include <cctype>
#include <algorithm>
#include <list>
#include <map>
#include <set>
#include <queue>
#include <string>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <sstream>
#include <stack>
#include <functional>

#include "debug_allocator.h"
#include "typedef.h"
#include "locale.hpp"
#include "event.h"

#ifdef _WIN32
#define isdigit iswdigit
#define isspace iswspace
#define isalpha iswalpha
#endif

#define PASSES_PER_SEC(sec) ((sec) * passes_per_sec)

#ifndef M_PI
#define M_PI    3.14159265358979323846 /* pi */
#endif
#ifndef M_PI_2
#define M_PI_2  1.57079632679489661923 /* pi/2 */
#endif

#define IN
#define OUT
