#pragma once
#include "../../libthecore/include/stdafx.h"
#include "../../../common/common_incl.hpp"

#include <cmath>
#include <csignal>
#include <sstream>
#include <array>

#include <rapidjson/document.h>
#include <rapidjson/reader.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/error/en.h>
#include <rapidjson/filereadstream.h>
using namespace rapidjson;

#ifndef _WIN32
#include <semaphore.h>
#else
#define isdigit iswdigit
#define isspace iswspace
#define isalpha iswalpha
#endif
