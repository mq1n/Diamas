#include "stdafx.h"
#include "ProtoReader.h"
#include "CsvReader.h"

bool bDumpProtoDebugFlag = false;
void writeDebug(const char* szMsg, int iInput, int type_value)
{
	char __buf[512];
	
	auto f1 = fopen("debug.log", "a+");
	if (!f1)
		return;

	if (type_value)
		snprintf(__buf, sizeof(__buf), "ERROR: (%d->%d) %s\n", type_value, iInput, szMsg);
	else
		snprintf(__buf, sizeof(__buf), "ERROR: (%d) %s\n", iInput, szMsg);
	fwrite(__buf, sizeof(char), strlen(__buf), f1);
	fclose(f1);
}

static std::string arItemType[] = {
	"ITEM_NONE",
	"ITEM_WEAPON",
	"ITEM_ARMOR",
	"ITEM_USE",
	"ITEM_AUTOUSE",
	"ITEM_MATERIAL",
	"ITEM_SPECIAL",
	"ITEM_TOOL",
	"ITEM_LOTTERY",
	"ITEM_ELK",
	"ITEM_METIN",
	"ITEM_CONTAINER",
	"ITEM_FISH",
	"ITEM_ROD",
	"ITEM_RESOURCE",
	"ITEM_CAMPFIRE",
	"ITEM_UNIQUE",
	"ITEM_SKILLBOOK",
	"ITEM_QUEST",
	"ITEM_POLYMORPH",
	"ITEM_TREASURE_BOX",
	"ITEM_TREASURE_KEY",
	"ITEM_SKILLFORGET",
	"ITEM_GIFTBOX",
	"ITEM_PICK",
	"ITEM_HAIR",
	"ITEM_TOTEM",
	"ITEM_BLEND",
	"ITEM_COSTUME",
	"ITEM_DS",
	"ITEM_SPECIAL_DS",
	"ITEM_EXTRACT",
	"ITEM_RING",
	"ITEM_BELT",
	"ITEM_PET",
	"ITEM_MEDIUM",
	"ITEM_GACHA"
};
int32_t get_Item_Type_Value(const std::string& inputString)
{
	int32_t retValue = -1;

	for (uint32_t i = 0; arItemType->size(); ++i)
	{
		std::string tempString = arItemType[i];

		if (inputString.find(tempString) != std::string::npos &&
			tempString.find(inputString) != std::string::npos)
		{
			retValue = i;
			break;
		}
	}

	assert(retValue != -1);
	return retValue;
}
std::string set_Item_Type_Value(int iInput) // enum -1=None
{
	if (iInput == (uint8_t)-1)
		return "NONE";

	std::string inputString("");
	int tmpFlag;
	for (uint32_t i = 0; i < arItemType->size(); i++)
	{
		tmpFlag = i;
		if (iInput == tmpFlag)
		{
			if (bDumpProtoDebugFlag)
				iInput = -1;
			inputString = arItemType[i];
			break;
		}
	}
	if (bDumpProtoDebugFlag && iInput != -1)
		writeDebug("Item Type not found", iInput);

	return inputString;
}

static std::map <uint32_t, std::vector <std::string>> subtypes
{
	{
		ITEM_WEAPON, {
			"WEAPON_SWORD",
			"WEAPON_DAGGER",
			"WEAPON_BOW",
			"WEAPON_TWO_HANDED",
			"WEAPON_BELL",
			"WEAPON_FAN",
			"WEAPON_ARROW",
			"WEAPON_MOUNT_SPEAR",
			"WEAPON_CLAW",
			"WEAPON_QUIVER",
			"WEAPON_BOUQUET"
		}
	},
	{
		ITEM_ARMOR, {
			"ARMOR_BODY",
			"ARMOR_HEAD",
			"ARMOR_SHIELD",
			"ARMOR_WRIST",
			"ARMOR_FOOTS",
			"ARMOR_NECK",
			"ARMOR_EAR",
			"ARMOR_NUM_TYPES"
		}
	},
	{
		ITEM_USE, {
			"USE_POTION",
			"USE_TALISMAN",
			"USE_TUNING",
			"USE_MOVE",
			"USE_TREASURE_BOX",
			"USE_MONEYBAG",
			"USE_BAIT",
			"USE_ABILITY_UP",
			"USE_AFFECT",
			"USE_CREATE_STONE",
			"USE_SPECIAL",
			"USE_POTION_NODELAY",
			"USE_CLEAR",
			"USE_INVISIBILITY",
			"USE_DETACHMENT",
			"USE_BUCKET",
			"USE_POTION_CONTINUE",
			"USE_CLEAN_SOCKET",
			"USE_CHANGE_ATTRIBUTE",
			"USE_ADD_ATTRIBUTE",
			"USE_ADD_ACCESSORY_SOCKET",
			"USE_PUT_INTO_ACCESSORY_SOCKET",
			"USE_ADD_ATTRIBUTE2",
			"USE_RECIPE",
			"USE_CHANGE_ATTRIBUTE2",
			"USE_BIND",
			"USE_UNBIND",
			"USE_TIME_CHARGE_PER",
			"USE_TIME_CHARGE_FIX",
			"USE_PUT_INTO_BELT_SOCKET",
			"USE_PUT_INTO_RING_SOCKET",
			"USE_CHANGE_COSTUME_ATTR",
			"USE_RESET_COSTUME_ATTR",
			"USE_UNK33",
			"USE_CHANGE_ATTRIBUTE_PLUS"
		}
	},
	{
		ITEM_AUTOUSE, {
			"AUTOUSE_POTION",
			"AUTOUSE_ABILITY_UP",
			"AUTOUSE_BOMB", "AUTOUSE_GOLD",
			"AUTOUSE_MONEYBAG",
			"AUTOUSE_TREASURE_BOX"
		}
	},
	{
		ITEM_MATERIAL, {
			"MATERIAL_LEATHER",
			"MATERIAL_BLOOD",
			"MATERIAL_ROOT",
			"MATERIAL_NEEDLE",
			"MATERIAL_JEWEL",
			"MATERIAL_DS_REFINE_NORMAL",
			"MATERIAL_DS_REFINE_BLESSED",
			"MATERIAL_DS_REFINE_HOLLY"
		}
	},
	{
		ITEM_SPECIAL, {
			"SPECIAL_MAP",
			"SPECIAL_KEY",
			"SPECIAL_DOC",
			"SPECIAL_SPIRIT"
		}
	},
	{
		ITEM_TOOL, {
			"TOOL_FISHING_ROD"
		}
	},
	{
		ITEM_LOTTERY, {
			"LOTTERY_TICKET",
			"LOTTERY_INSTANT"
		}
	},
	{
		ITEM_METIN, {
			"METIN_NORMAL",
			"METIN_GOLD"
		}
	},
	{
		ITEM_FISH, {
			"FISH_ALIVE",
			"FISH_DEAD"
		}
	},
	{
		ITEM_RESOURCE, {
			"RESOURCE_FISHBONE",
			"RESOURCE_WATERSTONEPIECE",
			"RESOURCE_WATERSTONE",
			"RESOURCE_BLOOD_PEARL",
			"RESOURCE_BLUE_PEARL",
			"RESOURCE_WHITE_PEARL",
			"RESOURCE_BUCKET",
			"RESOURCE_CRYSTAL",
			"RESOURCE_GEM",
			"RESOURCE_STONE",
			"RESOURCE_METIN",
			"RESOURCE_ORE"
		}
	},
	{
		ITEM_UNIQUE, {
			"UNIQUE_NONE",
			"UNIQUE_BOOK",
			"UNIQUE_SPECIAL_RIDE",
			"UNIQUE_3",
			"UNIQUE_4",
			"UNIQUE_5",
			"UNIQUE_6",
			"UNIQUE_7",
			"UNIQUE_8",
			"UNIQUE_9",
			"USE_SPECIAL"
		}
	},
	{
		ITEM_COSTUME, {
			"COSTUME_BODY",
			"COSTUME_HAIR",
			"COSTUME_MOUNT",
			"COSTUME_ACCE",
			"COSTUME_WEAPON"
		}
	},
	{
		ITEM_DS, {
			"DS_SLOT1",
			"DS_SLOT2",
			"DS_SLOT3",
			"DS_SLOT4",
			"DS_SLOT5",
			"DS_SLOT6"
		}
	},
	{
		ITEM_EXTRACT, {
			"EXTRACT_DRAGON_SOUL",
			"EXTRACT_DRAGON_HEART"
		}
	}
};
int32_t get_Item_SubType_Value(uint32_t type_value, const std::string& inputString)
{
	if (type_value < 0 || type_value >= ITEM_MAX_NUM)
	{
		sys_err("Out of range type! (type_value: %d, max valid type: %d)", type_value, ITEM_MAX_NUM);
		return -1;
	}

	// Don't process if there are no subtypes for this type 
	if (subtypes.count(type_value) == 0)
		return 0;

	std::string trimmedInput = trim(inputString);

	// Allow no subtype for ITEM_QUEST
	if (type_value == ITEM_QUEST && (trimmedInput.compare("NONE") == 0 || trimmedInput.compare("0") == 0))
		return 0;

	for (size_t i = 0, size = subtypes[type_value].size(); i < size; ++i)
	{
		//Found the subtype that's specified
		if (trimmedInput.compare(subtypes[type_value][i]) == 0)
			return i;
	}

	sys_err("Subtype %s is not valid for type %d", trimmedInput.c_str(), type_value);
	return -1;
}
std::string set_Item_SubType_Value(int type_value, int iInput) // enum -1=None
{
	if (type_value < 0 || type_value >= ITEM_MAX_NUM)
		return "0";

	if (iInput == (uint8_t)-1)
		return "0";

	if (subtypes.count(type_value) == 0)
		return "0";

	std::string inputString("");
	int tmpFlag;
	for (size_t i = 0, size = subtypes[type_value].size(); i < size; ++i)
	{
		tmpFlag = i;
		if (iInput == tmpFlag)
		{
			if (bDumpProtoDebugFlag)
				iInput = -1;
			inputString = subtypes[type_value][i];
			break;
		}
	}
	if (bDumpProtoDebugFlag && iInput != -1)
		writeDebug("Item SubType not found", iInput, type_value);
	return inputString;
}

static std::string arAntiFlag[] = {
	"ANTI_FEMALE",
	"ANTI_MALE",
	"ANTI_MUSA",
	"ANTI_ASSASSIN",
	"ANTI_SURA",
	"ANTI_MUDANG",
	"ANTI_GET",
	"ANTI_DROP",
	"ANTI_SELL",
	"ANTI_EMPIRE_A",
	"ANTI_EMPIRE_B",
	"ANTI_EMPIRE_C",
	"ANTI_SAVE",
	"ANTI_GIVE",
	"ANTI_PKDROP",
	"ANTI_STACK",
	"ANTI_MYSHOP",
	"ANTI_SAFEBOX",
	"ANTI_WOLFMAN",
	"ANTI_PET20",
	"ANTI_PET21"
};
int32_t get_Item_AntiFlag_Value(const std::string& inputString)
{
	int32_t retValue = 0;

	auto arInputString = string_split_trim(inputString, "|");
	for (size_t i = 0; i < _countof(arAntiFlag); i++)
	{
		std::string tempString = arAntiFlag[i];
		for (size_t j = 0; j < arInputString.size(); j++)
		{
			std::string tempString2 = arInputString[j];
			if (tempString2.compare(tempString) == 0)
			{
				retValue = retValue + (int32_t)pow(2, i);
			}

			if (tempString2.compare("") == 0)
				break;
		}
	}

	return retValue;
}
std::string set_Item_AntiFlag_Value(int iInput) // set
{
	if (iInput == 0)
		return "NONE";

	std::string inputString("");
	int tmpFlag;
	for (int i = 0; i < _countof(arAntiFlag); i++)
	{
		tmpFlag = static_cast<int>(pow(2.0, static_cast<double>(i)));
		if (iInput & tmpFlag)
		{
			if (bDumpProtoDebugFlag)
				iInput -= tmpFlag;
			if (!!inputString.compare(""))
				inputString += " | ";
			inputString += arAntiFlag[i];
		}
	}
	if (bDumpProtoDebugFlag && iInput)
		writeDebug("Item AntiFlag not found", iInput);
	return inputString;
}

static std::string arFlag[] = {
	"ITEM_TUNABLE",
	"ITEM_SAVE",
	"ITEM_STACKABLE",
	"COUNT_PER_1GOLD",
	"ITEM_SLOW_QUERY",
	"ITEM_UNIQUE",
	"ITEM_MAKECOUNT",
	"ITEM_IRREMOVABLE",
	"CONFIRM_WHEN_USE",
	"QUEST_USE",
	"QUEST_USE_MULTIPLE",
	"QUEST_GIVE",
	"ITEM_QUEST",
	"LOG",
	"STACKABLE",
	"SLOW_QUERY",
	"REFINEABLE",
	"IRREMOVABLE",
	"ITEM_APPLICABLE"
};
int32_t get_Item_Flag_Value(const std::string& inputString)
{
	int32_t retValue = 0;

	auto arInputString = string_split_trim(inputString, "|");
	for (size_t i = 0; i < _countof(arFlag); i++)
	{
		std::string tempString = arFlag[i];
		for (size_t j = 0; j < arInputString.size(); j++)
		{
			std::string tempString2 = arInputString[j];
			if (tempString2.compare(tempString) == 0)
			{
				retValue = retValue + (int32_t)pow(2, i);
			}

			if (tempString2.compare("") == 0)
				break;
		}
	}

	return retValue;
}
std::string set_Item_Flag_Value(int iInput) // set
{
	if (iInput == 0)
		return "NONE";

	std::string inputString("");
	int tmpFlag;
	for (int i = 0; i < _countof(arFlag); i++)
	{
		tmpFlag = static_cast<int>(pow(2.0, static_cast<double>(i)));
		if (iInput & tmpFlag)
		{
			if (bDumpProtoDebugFlag)
				iInput -= tmpFlag;
			if (!!inputString.compare(""))
				inputString += " | ";
			inputString += arFlag[i];
		}
	}
	if (bDumpProtoDebugFlag && iInput)
		writeDebug("Item Flag not found", iInput);
	return inputString;
}

static std::string arWearrFlag[] = {
	"WEAR_BODY",
	"WEAR_HEAD",
	"WEAR_FOOTS",
	"WEAR_WRIST",
	"WEAR_WEAPON",
	"WEAR_NECK",
	"WEAR_EAR",
	"WEAR_SHIELD",
	"WEAR_UNIQUE",
	"WEAR_ARROW",
	"WEAR_HAIR",
	"WEAR_ABILITY"
};
int32_t get_Item_WearFlag_Value(const std::string& inputString)
{
	int32_t retValue = 0;

	auto arInputString = string_split_trim(inputString, "|");
	for (size_t i = 0; i < _countof(arWearrFlag); i++)
	{
		std::string tempString = arWearrFlag[i];
		for (size_t j = 0; j < arInputString.size(); j++)
		{
			std::string tempString2 = arInputString.at(j);
			if (tempString2.compare(tempString) == 0)
			{
				retValue = retValue + (int32_t)pow(2, i);
			}

			if (tempString2.compare("") == 0)
				break;
		}
	}

	return retValue;
}
std::string set_Item_WearFlag_Value(int iInput) // set
{
	if (iInput == 0)
		return "NONE";

	std::string inputString("");
	int tmpFlag;
	for (int i = 0; i < _countof(arWearrFlag); i++)
	{
		tmpFlag = static_cast<int>(pow(2.0, static_cast<double>(i)));
		if (iInput & tmpFlag)
		{
			if (bDumpProtoDebugFlag)
				iInput -= tmpFlag;
			if (!!inputString.compare(""))
				inputString += " | ";
			inputString += arWearrFlag[i];
		}
	}
	if (bDumpProtoDebugFlag && iInput)
		writeDebug("Item WearFlag not found", iInput);
	return inputString;
}

static std::string arImmune[] = {
	"PARA",
	"CURSE",
	"STUN",
	"SLEEP",
	"SLOW",
	"POISON",
	"TERROR"
};
int32_t get_Item_Immune_Value(const std::string& inputString)
{
	int32_t retValue = 0;

	auto arInputString = string_split_trim(inputString, "|");
	for (size_t i = 0; i < _countof(arImmune); i++)
	{
		std::string tempString = arImmune[i];
		for (size_t j = 0; j < arInputString.size(); j++)
		{
			std::string tempString2 = arInputString.at(j);
			if (tempString2.compare(tempString) == 0)
			{
				retValue = retValue + (int32_t)pow(2, i);
			}

			if (tempString2.compare("") == 0)
				break;
		}
	}

	return retValue;
}
std::string set_Item_Immune_Value(int iInput) // set
{
	if (iInput == 0)
		return "NONE";

	std::string inputString("");
	int tmpFlag;
	for (int i = 0; i < _countof(arImmune); i++)
	{
		tmpFlag = static_cast<int>(pow(2.0, static_cast<double>(i)));
		if (iInput & tmpFlag)
		{
			if (bDumpProtoDebugFlag)
				iInput -= tmpFlag;
			if (!!inputString.compare(""))
				inputString += " | ";
			inputString += arImmune[i];
		}
	}
	if (bDumpProtoDebugFlag && iInput)
		writeDebug("Item ImmuneFlag not found", iInput);
	return inputString;
}

static std::string arLimitType[] = {
	"LIMIT_NONE",
	"LEVEL",
	"STR",
	"DEX",
	"INT",
	"CON",
	"PC_BANG",
	"REAL_TIME",
	"REAL_TIME_FIRST_USE",
	"TIMER_BASED_ON_WEAR"
};
int32_t get_Item_LimitType_Value(const std::string& inputString)
{
	int32_t retValue = -1;

	for (uint32_t i = 0; i < _countof(arLimitType); i++)
	{
		std::string tempString = arLimitType[i];
		std::string tempInputString = trim(inputString);

		if (tempInputString.compare(tempString) == 0)
		{
			retValue = i;
			break;
		}
	}

	assert(retValue != -1);
	return retValue;
}

std::string set_Item_LimitType_Value(int iInput) // enum -1=None
{
	if (iInput == (uint8_t)-1)
		return "NONE";

	std::string inputString("");
	int tmpFlag;
	for (int i = 0; i < _countof(arLimitType); i++)
	{
		tmpFlag = i;
		if (iInput == tmpFlag)
		{
			if (bDumpProtoDebugFlag)
				iInput = -1;
			inputString = arLimitType[i];
			break;
		}
	}
	if (bDumpProtoDebugFlag && iInput != -1)
		writeDebug("Item LimitType not found", iInput);
	return inputString;
}

static std::vector <std::string> applyTypeList = {
	"APPLY_NONE",
	"APPLY_MAX_HP",
	"APPLY_MAX_SP",
	"APPLY_CON",
	"APPLY_INT",
	"APPLY_STR",
	"APPLY_DEX",
	"APPLY_ATT_SPEED",
	"APPLY_MOV_SPEED",
	"APPLY_CAST_SPEED",
	"APPLY_HP_REGEN",
	"APPLY_SP_REGEN",
	"APPLY_POISON_PCT",
	"APPLY_STUN_PCT",
	"APPLY_SLOW_PCT",
	"APPLY_CRITICAL_PCT",
	"APPLY_PENETRATE_PCT",
	"APPLY_ATTBONUS_HUMAN",
	"APPLY_ATTBONUS_ANIMAL",
	"APPLY_ATTBONUS_ORC",
	"APPLY_ATTBONUS_MILGYO",
	"APPLY_ATTBONUS_UNDEAD",
	"APPLY_ATTBONUS_DEVIL",
	"APPLY_STEAL_HP",
	"APPLY_STEAL_SP",
	"APPLY_MANA_BURN_PCT",
	"APPLY_DAMAGE_SP_RECOVER",
	"APPLY_BLOCK",
	"APPLY_DODGE",
	"APPLY_RESIST_SWORD",
	"APPLY_RESIST_TWOHAND",
	"APPLY_RESIST_DAGGER",
	"APPLY_RESIST_BELL",
	"APPLY_RESIST_FAN",
	"APPLY_RESIST_BOW",
	"APPLY_RESIST_FIRE",
	"APPLY_RESIST_ELEC",
	"APPLY_RESIST_MAGIC",
	"APPLY_RESIST_WIND",
	"APPLY_REFLECT_MELEE",
	"APPLY_REFLECT_CURSE",
	"APPLY_POISON_REDUCE",
	"APPLY_KILL_SP_RECOVER",
	"APPLY_EXP_DOUBLE_BONUS",
	"APPLY_GOLD_DOUBLE_BONUS",
	"APPLY_ITEM_DROP_BONUS",
	"APPLY_POTION_BONUS",
	"APPLY_KILL_HP_RECOVER",
	"APPLY_IMMUNE_STUN",
	"APPLY_IMMUNE_SLOW",
	"APPLY_IMMUNE_FALL",
	"APPLY_SKILL",
	"APPLY_BOW_DISTANCE",
	"APPLY_ATT_GRADE_BONUS",
	"APPLY_DEF_GRADE_BONUS",
	"APPLY_MAGIC_ATT_GRADE",
	"APPLY_MAGIC_DEF_GRADE",
	"APPLY_CURSE_PCT",
	"APPLY_MAX_STAMINA",
	"APPLY_ATTBONUS_WARRIOR",
	"APPLY_ATTBONUS_ASSASSIN",
	"APPLY_ATTBONUS_SURA",
	"APPLY_ATTBONUS_SHAMAN",
	"APPLY_ATTBONUS_MONSTER",
	"APPLY_MALL_ATTBONUS",
	"APPLY_MALL_DEFBONUS",
	"APPLY_MALL_EXPBONUS",
	"APPLY_MALL_ITEMBONUS",
	"APPLY_MALL_GOLDBONUS",
	"APPLY_MAX_HP_PCT",
	"APPLY_MAX_SP_PCT",
	"APPLY_SKILL_DAMAGE_BONUS",
	"APPLY_NORMAL_HIT_DAMAGE_BONUS",
	"APPLY_SKILL_DEFEND_BONUS",
	"APPLY_NORMAL_HIT_DEFEND_BONUS",
	"APPLY_PC_BANG_EXP_BONUS",
	"APPLY_PC_BANG_DROP_BONUS",
	"APPLY_EXTRACT_HP_PCT",
	"APPLY_RESIST_WARRIOR",
	"APPLY_RESIST_ASSASSIN",
	"APPLY_RESIST_SURA",
	"APPLY_RESIST_SHAMAN",
	"APPLY_ENERGY",
	"APPLY_DEF_GRADE",
	"APPLY_COSTUME_ATTR_BONUS",
	"APPLY_MAGIC_ATTBONUS_PER",
	"APPLY_MELEE_MAGIC_ATTBONUS_PER",
	"APPLY_RESIST_ICE",
	"APPLY_RESIST_EARTH",
	"APPLY_RESIST_DARK",
	"APPLY_ANTI_CRITICAL_PCT",
	"APPLY_ANTI_PENETRATE_PCT",
	"APPLY_BLEEDING_REDUCE",
	"APPLY_BLEEDING_PCT",
	"APPLY_ATTBONUS_WOLFMAN",
	"APPLY_RESIST_WOLFMAN",
	"APPLY_RESIST_CLAW",
	"APPLY_ACCEDRAIN_RATE",
	"APPLY_RESIST_MAGIC_REDUCTION"
};
int32_t get_Item_ApplyType_Value(const std::string& inputString)
{
	int32_t retValue = -1;
	std::string trimmedInput = trim(inputString);

	for (size_t i = 0, until = applyTypeList.size(); i < until; ++i)
	{
		if (applyTypeList[i].compare(trimmedInput) == 0) {
			retValue = i;
			break;
		}
	}

	assert(retValue != -1);
	return retValue;
}
std::string set_Item_ApplyType_Value(int iInput) // enum -1=None
{
	if (iInput == (uint8_t)-1)
		return "NONE";

	std::string inputString("");
	int tmpFlag;
	for (size_t i = 0, until = applyTypeList.size(); i < until; ++i)
	{
		tmpFlag = i;
		if (iInput == tmpFlag)
		{
			if (bDumpProtoDebugFlag)
				iInput = -1;
			inputString = applyTypeList[i];
			break;
		}
	}
	if (bDumpProtoDebugFlag && iInput != -1)
		writeDebug("Item ApplyType not found", iInput);
	return inputString;
}

static std::string arRank[] = {
	"PAWN",
	"S_PAWN",
	"KNIGHT",
	"S_KNIGHT",
	"BOSS",
	"KING"
};
int32_t get_Mob_Rank_Value(const std::string& inputString)
{
	int32_t retValue = -1;

	for (uint32_t i = 0; i < _countof(arRank); i++)
	{
		std::string tempString = arRank[i];
		std::string tempInputString = trim(inputString);

		if (tempInputString.compare(tempString) == 0)
		{
			retValue = i;
			break;
		}
	}

	assert(retValue != -1);
	return retValue;
}
std::string set_Mob_Rank_Value(int iInput) // enum -1=None
{
	if (iInput == (uint8_t)-1)
		return "";

	std::string inputString("");
	int tmpFlag;
	for (int i = 0; i < _countof(arRank); i++)
	{
		tmpFlag = i;
		if (iInput == tmpFlag)
		{
			if (bDumpProtoDebugFlag)
				iInput = -1;
			inputString = arRank[i];
			break;
		}
	}
	if (bDumpProtoDebugFlag && iInput != -1)
		writeDebug("Mob Rank not found", iInput);
	return inputString;
}

static std::string arMobType[] = {
	"MONSTER",
	"NPC",
	"STONE",
	"WARP",
	"DOOR",
	"BUILDING",
	"PC",
	"POLYMORPH_PC",
	"HORSE",
	"GOTO"
};
int32_t get_Mob_Type_Value(const std::string& inputString)
{
	int32_t retValue = -1;

	for (uint32_t i = 0; i < _countof(arMobType); i++)
	{
		std::string tempString = arMobType[i];
		std::string tempInputString = trim(inputString);

		if (tempInputString.compare(tempString) == 0)
		{
			retValue = i;
			break;
		}
	}

	assert(retValue != -1);
	return retValue;
}
std::string set_Mob_Type_Value(int iInput) // enum -1=None
{
	if (iInput == (uint8_t)-1)
		return "";

	std::string inputString("");
	int tmpFlag;
	for (int i = 0; i < _countof(arMobType); i++)
	{
		tmpFlag = i;
		if (iInput == tmpFlag)
		{
			if (bDumpProtoDebugFlag)
				iInput = -1;
			inputString = arMobType[i];
			break;
		}
	}
	if (bDumpProtoDebugFlag && iInput != -1)
		writeDebug("Mob Type not found", iInput);
	return inputString;
}

static std::string arBattleType[] = {
	"MELEE",
	"RANGE",
	"MAGIC",
	"SPECIAL",
	"POWER",
	"TANKER",
	"SUPER_POWER",
	"SUPER_TANKER"
};
int32_t get_Mob_BattleType_Value(const std::string& inputString)
{
	int32_t retValue = -1;

	for (uint32_t i = 0; i < _countof(arBattleType); i++)
	{
		std::string tempString = arBattleType[i];
		std::string tempInputString = trim(inputString);

		if (tempInputString.compare(tempString) == 0)
		{
			retValue = i;
			break;
		}
	}

	assert(retValue != -1);
	return retValue;
}
std::string set_Mob_BattleType_Value(int iInput) // enum -1=None
{
	if (iInput == (uint8_t)-1)
		return "";

	std::string inputString("");
	int tmpFlag;
	for (int i = 0; i < _countof(arBattleType); i++)
	{
		tmpFlag = i;
		if (iInput == tmpFlag)
		{
			if (bDumpProtoDebugFlag)
				iInput = -1;
			inputString = arBattleType[i];
			break;
		}
	}
	if (bDumpProtoDebugFlag && iInput != -1)
		writeDebug("Mob BattleType not found", iInput);
	return inputString;
}

static std::string arSize[] = {
	"SMALL",
	"MEDIUM",
	"BIG"
};
int32_t get_Mob_Size_Value(const std::string& inputString)
{
	int32_t retValue = 0;

	for (uint32_t i = 0; i < _countof(arSize); i++)
	{
		std::string tempString = arSize[i];

		std::string tempInputString = trim(inputString);
		if (tempInputString.compare(tempString) == 0)
		{
			retValue = i + 1;
			break;
		}
	}

	return retValue;
}
std::string set_Mob_Size_Value(int iInput) // enum 0=None
{
	if (iInput == 0)
		return "";

	std::string inputString("");
	int tmpFlag;
	for (int i = 0; i < _countof(arSize); i++)
	{
		tmpFlag = i + 1;
		if (iInput == tmpFlag)
		{
			if (bDumpProtoDebugFlag)
				iInput = 0;
			inputString = arSize[i];
			break;
		}
	}
	if (bDumpProtoDebugFlag && iInput)
		writeDebug("Mob Size not found", iInput);
	return inputString;
}

static std::string arAIFlag[] = {
	"AGGR",
	"NOMOVE",
	"COWARD",
	"NOATTSHINSU",
	"NOATTCHUNJO",
	"NOATTJINNO",
	"ATTMOB",
	"BERSERK",
	"STONESKIN",
	"GODSPEED",
	"DEATHBLOW",
	"REVIVE"
};
int32_t get_Mob_AIFlag_Value(const std::string& inputString)
{
	int32_t retValue = 0;

	auto arInputString = string_split_trim(inputString, ",");
	for (size_t i = 0; i < _countof(arAIFlag); i++)
	{
		std::string tempString = arAIFlag[i];
		for (size_t j = 0; j < arInputString.size(); j++)
		{
			std::string tempString2 = arInputString.at(j);
			if (tempString2.compare(tempString) == 0)
			{
				retValue = retValue + (int32_t)pow(2, i);
			}

			if (tempString2.compare("") == 0)
				break;
		}
	}

	return retValue;
}
std::string set_Mob_AIFlag_Value(int iInput) // set
{
	if (iInput == 0)
		return "";

	std::string inputString("");
	int tmpFlag;
	for (int i = 0; i < _countof(arAIFlag); i++)
	{
		tmpFlag = static_cast<int>(pow(2.0, static_cast<double>(i)));
		if (iInput & tmpFlag)
		{
			if (bDumpProtoDebugFlag)
				iInput -= tmpFlag;
			if (!!inputString.compare(""))
				inputString += ",";
			inputString += arAIFlag[i];
		}
	}
	if (bDumpProtoDebugFlag && iInput)
		writeDebug("Mob AIFlag not found", iInput);
	return inputString;
}

static std::string arRaceFlag[] = {
	"ANIMAL",
	"UNDEAD",
	"DEVIL",
	"HUMAN",
	"ORC",
	"MILGYO",
	"INSECT",
	"FIRE",
	"ICE",
	"DESERT",
	"TREE",
	"ATT_ELEC",
	"ATT_FIRE",
	"ATT_ICE",
	"ATT_WIND",
	"ATT_EARTH",
	"ATT_DARK"
};
int32_t get_Mob_RaceFlag_Value(const std::string& inputString)
{
	int32_t retValue = 0;

	auto arInputString = string_split_trim(inputString, ",");
	for (size_t i = 0; i < _countof(arRaceFlag); i++)
	{
		std::string tempString = arRaceFlag[i];
		for (size_t j = 0; j < arInputString.size(); j++)
		{
			std::string tempString2 = arInputString.at(j);
			if (tempString2.compare(tempString) == 0)
			{
				retValue = retValue + (int32_t)pow(2, i);
			}

			if (tempString2.compare("") == 0)
				break;
		}
	}

	return retValue;
}
std::string set_Mob_RaceFlag_Value(int iInput) // set
{
	if (iInput == 0)
		return "";

	std::string inputString("");
	int tmpFlag;
	for (int i = 0; i < _countof(arRaceFlag); i++)
	{
		tmpFlag = static_cast<int>(pow(2.0, static_cast<double>(i)));
		if (iInput & tmpFlag)
		{
			if (bDumpProtoDebugFlag)
				iInput -= tmpFlag;
			if (!!inputString.compare(""))
				inputString += ",";
			inputString += arRaceFlag[i];
		}
	}
	if (bDumpProtoDebugFlag && iInput)
		writeDebug("Mob RaceFlag not found", iInput);
	return inputString;
}

static std::string arImmuneFlag[] = {
	"STUN",
	"SLOW",
	"FALL",
	"CURSE",
	"POISON",
	"TERROR",
	"REFLECT"
};
int32_t get_Mob_ImmuneFlag_Value(const std::string& inputString)
{
	int32_t retValue = 0;

	auto arInputString = string_split_trim(inputString, ",");
	for (size_t i = 0; i < _countof(arImmuneFlag); i++)
	{
		std::string tempString = arImmuneFlag[i];
		for (size_t j = 0; j < arInputString.size(); j++)
		{
			std::string tempString2 = arInputString.at(j);
			if (tempString2.compare(tempString) == 0)
			{
				retValue = retValue + (int32_t)pow(2, i);
			}

			if (tempString2.compare("") == 0)
				break;
		}
	}

	return retValue;
}
std::string set_Mob_ImmuneFlag_Value(int iInput) // set
{
	if (iInput == 0)
		return "";

	std::string inputString("");
	int tmpFlag;
	for (int i = 0; i < _countof(arImmuneFlag); i++)
	{
		tmpFlag = static_cast<int>(pow(2.0, static_cast<double>(i)));
		if (iInput & tmpFlag)
		{
			if (bDumpProtoDebugFlag)
				iInput -= tmpFlag;
			if (!!inputString.compare(""))
				inputString += ",";
			inputString += arImmuneFlag[i];
		}
	}
	if (bDumpProtoDebugFlag && iInput)
		writeDebug("Mob ImmuneFlag not found", iInput);
	return inputString;
}


bool Set_Proto_Mob_Table(TMobTable* mobTable, const cCsvTable& csvTable, std::map<int32_t, const char*>& nameMap)
{
	int32_t col = 0;

	str_to_number(mobTable->dwVnum, csvTable.AsStringByIndex(col++));
	strlcpy(mobTable->szName, csvTable.AsStringByIndex(col++), sizeof(mobTable->szName));

	// Set locale names
	const auto it = nameMap.find(mobTable->dwVnum);
	if (it != nameMap.end())
	{
		const char* localeName = it->second;
		strlcpy(mobTable->szLocaleName, localeName, sizeof(mobTable->szLocaleName));
	}
	else
	{
		strlcpy(mobTable->szLocaleName, mobTable->szName, sizeof(mobTable->szLocaleName));
	}

	// Rank
	int32_t rankValue = get_Mob_Rank_Value(csvTable.AsStringByIndex(col++));
	mobTable->bRank = rankValue;

	// Type
	int32_t typeValue = get_Mob_Type_Value(csvTable.AsStringByIndex(col++));
	mobTable->bType = typeValue;

	// Battle Type
	int32_t battleTypeValue = get_Mob_BattleType_Value(csvTable.AsStringByIndex(col++));
	mobTable->bBattleType = battleTypeValue;

	// Level
	str_to_number(mobTable->bLevel, csvTable.AsStringByIndex(col++));

	// Scale
	str_to_number(mobTable->bSize, csvTable.AsStringByIndex(col++));

	// AI Flag
	int32_t aiFlagValue = get_Mob_AIFlag_Value(csvTable.AsStringByIndex(col++));
	mobTable->dwAIFlag = aiFlagValue;

	// Mount Capacity;
	col++;

	// Race Flag
	int32_t raceFlagValue = get_Mob_RaceFlag_Value(csvTable.AsStringByIndex(col++));
	mobTable->dwRaceFlag = raceFlagValue;

	// Immune Flag
	int32_t immuneFlagValue = get_Mob_ImmuneFlag_Value(csvTable.AsStringByIndex(col++));
	mobTable->dwImmuneFlag = immuneFlagValue;

	str_to_number(mobTable->bEmpire, csvTable.AsStringByIndex(col++));  //col = 11

	strlcpy(mobTable->szFolder, csvTable.AsStringByIndex(col++), sizeof(mobTable->szFolder));

	str_to_number(mobTable->bOnClickType, csvTable.AsStringByIndex(col++));

	str_to_number(mobTable->bStr, csvTable.AsStringByIndex(col++));
	str_to_number(mobTable->bDex, csvTable.AsStringByIndex(col++));
	str_to_number(mobTable->bCon, csvTable.AsStringByIndex(col++));
	str_to_number(mobTable->bInt, csvTable.AsStringByIndex(col++));
	str_to_number(mobTable->dwDamageRange[0], csvTable.AsStringByIndex(col++));
	str_to_number(mobTable->dwDamageRange[1], csvTable.AsStringByIndex(col++));
	str_to_number(mobTable->dwMaxHP, csvTable.AsStringByIndex(col++));
	str_to_number(mobTable->bRegenCycle, csvTable.AsStringByIndex(col++));
	str_to_number(mobTable->bRegenPercent, csvTable.AsStringByIndex(col++));
	str_to_number(mobTable->dwGoldMin, csvTable.AsStringByIndex(col++));
	str_to_number(mobTable->dwGoldMax, csvTable.AsStringByIndex(col++));
	str_to_number(mobTable->dwExp, csvTable.AsStringByIndex(col++));
	str_to_number(mobTable->wDef, csvTable.AsStringByIndex(col++));
	str_to_number(mobTable->sAttackSpeed, csvTable.AsStringByIndex(col++));
	str_to_number(mobTable->sMovingSpeed, csvTable.AsStringByIndex(col++));
	col++; // AggressiveHpPct [no use] - TODO: Remove column
	str_to_number(mobTable->wAggressiveSight, csvTable.AsStringByIndex(col++));
	str_to_number(mobTable->wAttackRange, csvTable.AsStringByIndex(col++));

	str_to_number(mobTable->dwDropItemVnum, csvTable.AsStringByIndex(col++));	//32
	str_to_number(mobTable->dwResurrectionVnum, csvTable.AsStringByIndex(col++));
	for (int32_t i = 0; i < MOB_ENCHANTS_MAX_NUM; ++i)
		str_to_number(mobTable->cEnchants[i], csvTable.AsStringByIndex(col++));

	for (int32_t i = 0; i < MOB_RESISTS_MAX_NUM; ++i)
		str_to_number(mobTable->cResists[i], csvTable.AsStringByIndex(col++));

	str_to_float(mobTable->fDamMultiply, csvTable.AsStringByIndex(col++));
	str_to_number(mobTable->dwSummonVnum, csvTable.AsStringByIndex(col++));
	str_to_number(mobTable->dwDrainSP, csvTable.AsStringByIndex(col++));

	//Mob_Color
	++col;

	str_to_number(mobTable->dwPolymorphItemVnum, csvTable.AsStringByIndex(col++));

	str_to_number(mobTable->Skills[0].bLevel, csvTable.AsStringByIndex(col++));
	str_to_number(mobTable->Skills[0].dwVnum, csvTable.AsStringByIndex(col++));
	str_to_number(mobTable->Skills[1].bLevel, csvTable.AsStringByIndex(col++));
	str_to_number(mobTable->Skills[1].dwVnum, csvTable.AsStringByIndex(col++));
	str_to_number(mobTable->Skills[2].bLevel, csvTable.AsStringByIndex(col++));
	str_to_number(mobTable->Skills[2].dwVnum, csvTable.AsStringByIndex(col++));
	str_to_number(mobTable->Skills[3].bLevel, csvTable.AsStringByIndex(col++));
	str_to_number(mobTable->Skills[3].dwVnum, csvTable.AsStringByIndex(col++));
	str_to_number(mobTable->Skills[4].bLevel, csvTable.AsStringByIndex(col++));
	str_to_number(mobTable->Skills[4].dwVnum, csvTable.AsStringByIndex(col++));

	str_to_number(mobTable->bBerserkPoint, csvTable.AsStringByIndex(col++));
	str_to_number(mobTable->bStoneSkinPoint, csvTable.AsStringByIndex(col++));
	str_to_number(mobTable->bGodSpeedPoint, csvTable.AsStringByIndex(col++));
	str_to_number(mobTable->bDeathBlowPoint, csvTable.AsStringByIndex(col++));
	str_to_number(mobTable->bRevivePoint, csvTable.AsStringByIndex(col++));

	sys_log(0, "MOB #%-5u %-24s level: %-3u rank: %u empire: %d", mobTable->dwVnum, mobTable->szLocaleName, mobTable->bLevel, mobTable->bRank, mobTable->bEmpire);

	return true;
}

bool Set_Proto_Item_Table(SItemTable_Server* itemTable, const cCsvTable& csvTable, std::map<int32_t, const char*>& nameMap)
{
	int32_t col = 0;

	int32_t dataArray[33];
	for (size_t i = 0; i < _countof(dataArray); i++)
	{
		int32_t validCheck = 0;
		if (i == 2) {
			dataArray[i] = get_Item_Type_Value(csvTable.AsStringByIndex(col));
			validCheck = dataArray[i];
		}
		else if (i == 3) {
			dataArray[i] = get_Item_SubType_Value(dataArray[i - 1], csvTable.AsStringByIndex(col));
			validCheck = dataArray[i];
		}
		else if (i == 5) {
			dataArray[i] = get_Item_AntiFlag_Value(csvTable.AsStringByIndex(col));
			validCheck = dataArray[i];
		}
		else if (i == 6) {
			dataArray[i] = get_Item_Flag_Value(csvTable.AsStringByIndex(col));
			validCheck = dataArray[i];
		}
		else if (i == 7) {
			dataArray[i] = get_Item_WearFlag_Value(csvTable.AsStringByIndex(col));
			validCheck = dataArray[i];
		}
		else if (i == 8) {
			dataArray[i] = get_Item_Immune_Value(csvTable.AsStringByIndex(col));
			validCheck = dataArray[i];
		}
		else if (i == 14) {
			dataArray[i] = get_Item_LimitType_Value(csvTable.AsStringByIndex(col));
			validCheck = dataArray[i];
		}
		else if (i == 16) {
			dataArray[i] = get_Item_LimitType_Value(csvTable.AsStringByIndex(col));
			validCheck = dataArray[i];
		}
		else if (i == 18) {
			dataArray[i] = get_Item_ApplyType_Value(csvTable.AsStringByIndex(col));
			validCheck = dataArray[i];
		}
		else if (i == 20) {
			dataArray[i] = get_Item_ApplyType_Value(csvTable.AsStringByIndex(col));
			validCheck = dataArray[i];
		}
		else if (i == 22) {
			dataArray[i] = get_Item_ApplyType_Value(csvTable.AsStringByIndex(col));
			validCheck = dataArray[i];
		}
		else {
			str_to_number(dataArray[i], csvTable.AsStringByIndex(col));
		}

		if (validCheck == -1)
		{
			std::ostringstream dataStream;

			for (uint32_t j = 0; j < i; ++j)
				dataStream << dataArray[j] << ",";

			//fprintf(stderr, "ItemProto Reading Failed : Invalid value.\n");
			sys_err("ItemProto Reading Failed : Invalid value. (index: %d, col: %d, value: %s)", i, col, csvTable.AsStringByIndex(col));
			sys_err("\t%d ~ %d Values: %s", 0, i, dataStream.str().c_str());

			abort();
		}

		col = col + 1;
	}

	// vnum 및 vnum range �?기.
	{
		std::string s(csvTable.AsStringByIndex(0));
		uint32_t pos = s.find("~");
		// vnum 필드에 '~'가 없다면 �?스
		if (std::string::npos == pos)
		{
			itemTable->dwVnum = dataArray[0];
			itemTable->dwVnumRange = 0;
		}
		else
		{
			std::string s_start_vnum(s.substr(0, pos));
			std::string s_end_vnum(s.substr(pos + 1));

			int32_t start_vnum = atoi(s_start_vnum.c_str());
			int32_t end_vnum = atoi(s_end_vnum.c_str());
			if (0 == start_vnum || (0 != end_vnum && end_vnum < start_vnum))
			{
				sys_err("INVALID VNUM %s", s.c_str());
				return false;
			}
			itemTable->dwVnum = start_vnum;
			itemTable->dwVnumRange = end_vnum - start_vnum;
		}
	}

	strlcpy(itemTable->szName, csvTable.AsStringByIndex(1), ITEM_NAME_MAX_LEN + 1);
	//지역별 이름 넣어주기.
	std::map<int32_t, const char*>::iterator it = nameMap.find(itemTable->dwVnum);
	strlcpy(itemTable->szLocaleName, (it != nameMap.end()) ? it->second : itemTable->szName, ITEM_NAME_MAX_LEN + 1);

	itemTable->bType = dataArray[2];
	itemTable->bSubType = dataArray[3];
	itemTable->bSize = dataArray[4];
	itemTable->dwAntiFlags = dataArray[5];
	itemTable->dwFlags = dataArray[6];
	itemTable->dwWearFlags = dataArray[7];
	itemTable->dwImmuneFlag = dataArray[8];
	itemTable->dwISellItemPrice = dataArray[9];
	itemTable->dwIBuyItemPrice = dataArray[10];
	itemTable->dwRefinedVnum = dataArray[11];
	itemTable->wRefineSet = dataArray[12];
	itemTable->bAlterToMagicItemPct = dataArray[13];
	itemTable->cLimitRealTimeFirstUseIndex = -1;
	itemTable->cLimitTimerBasedOnWearIndex = -1;

	int32_t i;

	for (i = 0; i < ITEM_LIMIT_MAX_NUM; ++i)
	{
		itemTable->aLimits[i].bType = dataArray[14 + i * 2];
		itemTable->aLimits[i].lValue = dataArray[15 + i * 2];

		if (LIMIT_REAL_TIME_START_FIRST_USE == itemTable->aLimits[i].bType)
			itemTable->cLimitRealTimeFirstUseIndex = (char)i;

		if (LIMIT_TIMER_BASED_ON_WEAR == itemTable->aLimits[i].bType)
			itemTable->cLimitTimerBasedOnWearIndex = (char)i;

	}

	for (i = 0; i < ITEM_APPLY_MAX_NUM; ++i)
	{
		itemTable->aApplies[i].bType = dataArray[18 + i * 2];
		itemTable->aApplies[i].lValue = dataArray[19 + i * 2];
	}

	for (i = 0; i < ITEM_VALUES_MAX_NUM; ++i)
		itemTable->alValues[i] = dataArray[24 + i];

	//column for 'Specular'
	itemTable->bGainSocketPct = dataArray[31];
	itemTable->sAddonType = dataArray[32];

	//test
	str_to_number(itemTable->bWeight, "0");

	return true;
}
