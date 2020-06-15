#ifndef __Item_CSV_READER_H__
#define __Item_CSV_READER_H__

#include <iostream>
#include <map>

#include "CsvReader.h"

//csv 파일을 읽어와서 아이템 테이블에 넣어준다.
void putItemIntoTable(); //(테이블, 테스트여부)

int32_t get_Item_Type_Value(const std::string &inputString);
int32_t get_Item_SubType_Value(uint32_t type_value, const std::string &inputString);
int32_t get_Item_AntiFlag_Value(const std::string &inputString);
int32_t get_Item_Flag_Value(const std::string &inputString);
int32_t get_Item_WearFlag_Value(const std::string &inputString);
int32_t get_Item_Immune_Value(const std::string &inputString);
int32_t get_Item_LimitType_Value(const std::string &inputString);
int32_t get_Item_ApplyType_Value(const std::string &inputString);


//몬스터 프로토도 읽을 수 있다.
int32_t get_Mob_Rank_Value(const std::string &inputString);
int32_t get_Mob_Type_Value(const std::string &inputString);
int32_t get_Mob_BattleType_Value(const std::string &inputString);
int32_t get_Mob_Size_Value(const std::string &inputString);
int32_t get_Mob_AIFlag_Value(const std::string &inputString);
int32_t get_Mob_RaceFlag_Value(const std::string &inputString);
int32_t get_Mob_ImmuneFlag_Value(const std::string &inputString);

//
bool Set_Proto_Mob_Table(TMobTable *mobTable, const cCsvTable &csvTable, std::map<int32_t,const char*> &nameMap);
bool Set_Proto_Item_Table(TItemTable *itemTable, const cCsvTable &csvTable, std::map<int32_t,const char*> &nameMap);

#endif