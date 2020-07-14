#pragma once
#include "../../common/defines.h"
#include "../../common/item_data.h"
#include "../../common/tables.h"
#include "../eterGameLib/ItemData.h"

struct SAffects
{
	enum
	{
		AFFECT_MAX_NUM = 32,
	};

	SAffects() : dwAffects(0) {}
	SAffects(const uint32_t & c_rAffects)
	{
		__SetAffects(c_rAffects);
	}
	int32_t operator = (const uint32_t & c_rAffects)
	{
		__SetAffects(c_rAffects);
	}

	BOOL IsAffect(uint8_t byIndex)
	{
		return dwAffects & (1 << byIndex);
	}

	void __SetAffects(const uint32_t & c_rAffects)
	{
		dwAffects = c_rAffects;
	}

	uint32_t dwAffects;
};

extern std::string g_strGuildSymbolPathName;

const uint32_t c_Name_Max_Length = 64;
const uint32_t c_FileName_Max_Length = 128;
const uint32_t c_Short_Name_Max_Length = 32;

const uint32_t c_QuickBar_Line_Count = 3;
const uint32_t c_QuickBar_Slot_Count = 12;

const float c_Idle_WaitTime = 5.0f;

const int32_t c_Monster_Race_Start_Number = 6;
const int32_t c_Monster_Model_Start_Number = 20001;

const float c_fAttack_Delay_Time = 0.2f;
const float c_fHit_Delay_Time = 0.1f;
const float c_fCrash_Wave_Time = 0.2f;
const float c_fCrash_Wave_Distance = 3.0f;

const float c_fHeight_Step_Distance = 50.0f;

enum
{
	DISTANCE_TYPE_FOUR_WAY,
	DISTANCE_TYPE_EIGHT_WAY,
	DISTANCE_TYPE_ONE_WAY,
	DISTANCE_TYPE_MAX_NUM,
};

const float c_fMagic_Script_Version = 1.0f;
const float c_fSkill_Script_Version = 1.0f;
const float c_fMagicSoundInformation_Version = 1.0f;
const float c_fBattleCommand_Script_Version = 1.0f;
const float c_fEmotionCommand_Script_Version = 1.0f;
const float c_fActive_Script_Version = 1.0f;
const float c_fPassive_Script_Version = 1.0f;

// Used by PushMove
const float c_fWalkDistance = 175.0f;
const float c_fRunDistance = 310.0f;

#define FILE_MAX_LEN 128


inline float GetSqrtDistance(int32_t ix1, int32_t iy1, int32_t ix2, int32_t iy2) // By sqrt
{
	float dx, dy;

	dx = float(ix1 - ix2);
	dy = float(iy1 - iy2);

	return sqrtf(dx*dx + dy*dy);
}

// DEFAULT_FONT
void DefaultFont_Startup();
void DefaultFont_Cleanup();
void DefaultFont_SetName(const char * c_szFontName);
CResource* DefaultFont_GetResource();
CResource* DefaultItalicFont_GetResource();
// END_OF_DEFAULT_FONT

void SetGuildSymbolPath(const char * c_szPathName);
const char * GetGuildSymbolFileName(uint32_t dwGuildID);
uint8_t SlotTypeToInvenType(uint8_t bSlotType);
