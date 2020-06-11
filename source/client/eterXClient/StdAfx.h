#pragma once

#if _MSC_VER >= 1400
//if don't use below, time_t is 64bit
#define _USE_32BIT_TIME_T
#endif
#define _USE_MATH_DEFINES
#include <cmath>
#include "../eterLib/StdAfx.h"
#include "../eterPythonLib/StdAfx.h"
#include "../eterGameLib/StdAfx.h"
#include "../eterScriptLib/StdAfx.h"
#include "../eterSoundLib/StdAfx.h"
#include "../eterEffectLib/StdAfx.h"
#include "../eterTerrainLib/StdAfx.h"
#include "../eterTreeLib/StdAfx.h"

#ifndef __D3DRM_H__
#define __D3DRM_H__
#endif

#include <tchar.h>
#include <strsafe.h>

#include <dshow.h>
#include <stdint.h>

#include "Locale.h"

#include "GameType.h"

enum
{
	POINT_MAX_NUM = 255,	
	CHARACTER_NAME_MAX_LEN = 24,
	PLAYER_NAME_MAX_LEN = 12,
};

void initapp();
void initime();
void initsystemSetting();
void initchr();
void initchrmgr();
void initChat();
void initTextTail();
void initime();
void initItem();
void initNonPlayer();
void initnet();
void initPlayer();
void initServerStateChecker();
void initTrade();
void initMiniMap();
void initEvent();
void initeffect();
void initsnd();
void initeventmgr();
void initBackground();
void initwndMgr();
void initshop();
void initpack();
void initskill();
void initfly();
void initquest();
void initsafebox();
void initguild();
void initMessenger();
#ifdef ENABLE_ACCE_SYSTEM
void initAcce();
#endif
