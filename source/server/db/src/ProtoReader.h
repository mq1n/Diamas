#pragma once
#include <iostream>
#include <map>
#include <string>

#include "CsvReader.h"

// ## DumpProto DEBUG
extern bool bDumpProtoDebugFlag;
void writeDebug(const char* szMsg, int iInput, int type_value = 0);

// ## COMMON
int32_t get_Item_Type_Value(const std::string &inputString);
int32_t get_Item_SubType_Value(uint32_t type_value, const std::string &inputString);
int32_t get_Item_AntiFlag_Value(const std::string &inputString);
int32_t get_Item_Flag_Value(const std::string &inputString);
int32_t get_Item_WearFlag_Value(const std::string &inputString);
int32_t get_Item_Immune_Value(const std::string &inputString);
int32_t get_Item_LimitType_Value(const std::string &inputString);
int32_t get_Item_ApplyType_Value(const std::string &inputString);

int32_t get_Mob_Rank_Value(const std::string &inputString);
int32_t get_Mob_Type_Value(const std::string &inputString);
int32_t get_Mob_BattleType_Value(const std::string &inputString);
int32_t get_Mob_Size_Value(const std::string &inputString);
int32_t get_Mob_AIFlag_Value(const std::string &inputString);
int32_t get_Mob_RaceFlag_Value(const std::string &inputString);
int32_t get_Mob_ImmuneFlag_Value(const std::string &inputString);

// ## DUMP
std::string set_Item_Type_Value(int iInput);
std::string set_Item_SubType_Value(int type_value, int iInput);
std::string set_Item_AntiFlag_Value(int iInput);
std::string set_Item_Flag_Value(int iInput);
std::string set_Item_WearFlag_Value(int iInput);
std::string set_Item_Immune_Value(int iInput);
std::string set_Item_LimitType_Value(int iInput);
std::string set_Item_ApplyType_Value(int iInput);

std::string set_Mob_Rank_Value(int iInput);
std::string set_Mob_Type_Value(int iInput);
std::string set_Mob_BattleType_Value(int iInput);
std::string set_Mob_Size_Value(int iInput);
std::string set_Mob_AIFlag_Value(int iInput);
std::string set_Mob_RaceFlag_Value(int iInput);
std::string set_Mob_ImmuneFlag_Value(int iInput);

//
bool Set_Proto_Mob_Table(TMobTable *mobTable, const cCsvTable &csvTable, std::map<int32_t,const char*> &nameMap);
bool Set_Proto_Item_Table(SItemTable_Server *itemTable, const cCsvTable &csvTable, std::map<int32_t,const char*> &nameMap);
