#pragma once

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#endif

#define _WIN32_DCOM

#include <d3d9/d3dx9.h>
#include <d3d9/DxErr.h>

#define DIRECTINPUT_VERSION 0x0800

#include <dinput.h>

#include <mmsystem.h>
#include <process.h>
#include <stdio.h>
#include <math.h>
#include <time.h>
#include <direct.h>
#include <malloc.h>
#include <algorithm>

#include "../../common/defines.h"
#include "../eterBase/StdAfx.h"
#include "../eterBase/Debug.h"
#include "../eterBase/CodePageId.h"

#ifndef VC_EXTRALEAN
#include <winsock.h>
#endif
