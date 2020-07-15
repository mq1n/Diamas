#pragma once

#if defined(_WIN32)
	#define WIN32_LEAN_AND_MEAN
	#include <windows.h>
#else
	#include <unistd.h>

	#define vsprintf_s vsprintf
	#define strncpy_s strncpy
	#define _getpid getpid
#endif

#include <iomanip>
#include <fstream>
#include <sstream>
#include <iostream>
#include <vector>
#include <map>
#include <list>
#include <set>
#include <unordered_map>
#include <unordered_set>
#include <chrono>
#include <cstdint>
#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <ctime>
#include <csignal>
#include <stack>
#include <memory>
#include <new>
#include <future>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <random>
#include <filesystem>

#include <NetEngine.hpp>
#include <amy.hpp>

#include <rapidjson/document.h>
#include <rapidjson/reader.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/error/en.h>
#include <rapidjson/filereadstream.h>
using namespace rapidjson;

#include "../../../common/common_incl.hpp"
