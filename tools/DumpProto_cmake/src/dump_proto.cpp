#include "../include/dump_proto.h"
#include "../include/OptionParser.h"

using namespace std;

static TMobTable * gs_pMobTable = nullptr;
static uint32_t gs_nMobTableSize = 0;

static SItemTable * gs_pItemTable = nullptr;
static uint32_t gs_nItemTableSize = 0;

bool BuildMobTable()
{
	//%%% <함수 설명> %%%//
	//1. 요약 : 'mob_proto.txt', 'mob_proto_test.txt', 'mob_names.txt' 파일을 읽고,
	//		gs_pMobTable 를 구성한다.
	//2. 순서
	//	1)'mob_names.txt' 파일을 읽어서 vnum:name 맵을 만든다.
	//	2)'mob_proto_test.txt' 파일을 읽어서,
	//		test_mob_table 를 만들고,
	//		vnum:TMobTable 맵을 만든다.
	//	3)'mob_proto.txt' 파일을 읽고, gs_pMobTable를 구성한다.
	//		test_mob_table에 있는 vnum은 테스트 데이터를 넣는다.
	//	4)test_mob_table 데이터중에, gs_pMobTable 에 없는 데이터를 추가한다.
	//3. 테스트
	//	1)'mob_proto.txt' 정보가 gs_pMobTable에 잘 들어갔는지. -> 완료
	//	2)'mob_names.txt' 정보가 gs_pMobTable에 잘 들어갔는지. -> 완료
	//	3)'mob_proto_test.txt' 에서 [겹치는] 정보가 gs_pMobTable 에 잘 들어갔는지. -> 완료
	//	4)'mob_proto_test.txt' 에서 [새로운] 정보가 gs_pMobTable 에 잘 들어갔는지. -> 완료
	//^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^//

	fprintf(stderr, "sizeof(TMobTable): %u\n", sizeof(TMobTable));


	//==============================================================//
	//======local별 몬스터 이름을 저장하고 있는 [맵] vnum:name======//
	//==============================================================//
	bool isNameFile = true;
	map<int,const char*> localMap;
	cCsvTable nameData;
	if(!nameData.Load("mob_names.txt",'\t'))
	{
		fprintf(stderr, "mob_names.txt Unable to read file\n");
		isNameFile = false;
	} else {
		nameData.Next();
		while(nameData.Next()) {
			localMap[atoi(nameData.AsStringByIndex(0))] = nameData.AsStringByIndex(1);
		}
	}
	//______________________________________________________________//

	//=========================================//
	//======몬스터들의 vnum을 저장할 [셋]======//
	//  *테스트용 파일을 새로 읽어올때,        //
	//  1. 기존에 있던 데이터인지 확일할때 사용//
	//=========================================//
	set<int> vnumSet;
	//_________________________________________//

	//==================================================//
	//	2)'mob_proto_test.txt' 파일을 읽어서,
	//		test_mob_table 를 만들고,
	//		vnum:TMobTable 맵을 만든다.
	//==================================================//
	map<DWORD, TMobTable *> test_map_mobTableByVnum;

	//파일 읽어오기.
	cCsvTable data;
	if(!data.Load("mob_proto.txt",'\t'))
	{
		fprintf(stderr, "mob_proto.txt Unable to read file\n");
		return false;
	}
	data.Next(); //맨 윗줄 제외 (아이템 칼럼을 설명하는 부분)



	//===== 몹 테이블 생성=====//
	if (gs_pMobTable)
	{
		delete gs_pMobTable;
		gs_pMobTable = NULL;
	}

	//새로 추가되는 갯수를 파악한다.
	int addNumber = 0;
	while(data.Next()) {
		int vnum = atoi(data.AsStringByIndex(0));
		std::map<DWORD, TMobTable *>::iterator it_map_mobTable;
		it_map_mobTable = test_map_mobTableByVnum.find(vnum);
		if(it_map_mobTable != test_map_mobTableByVnum.end()) {
			addNumber++;
		}
	}


	gs_nMobTableSize = data.m_File.GetRowCount()-1 + addNumber;

	gs_pMobTable = new TMobTable[gs_nMobTableSize];
	memset(gs_pMobTable, 0, sizeof(TMobTable) * gs_nMobTableSize);

	TMobTable * mob_table = gs_pMobTable;


	//data를 다시 첫줄로 옮긴다.(다시 읽어온다;;)
	data.Destroy();
	if(!data.Load("mob_proto.txt",'\t'))
	{
		fprintf(stderr, "mob_proto.txt Unable to read file\n");
		return false;
	}
	data.Next(); //맨 윗줄 제외 (아이템 칼럼을 설명하는 부분)

	while (data.Next())
	{
		int col = 0;
		//테스트 파일에 같은 vnum이 있는지 조사.
		std::map<DWORD, TMobTable *>::iterator it_map_mobTable;
		it_map_mobTable = test_map_mobTableByVnum.find(atoi(data.AsStringByIndex(col)));
		if(it_map_mobTable == test_map_mobTableByVnum.end()) {

			if (!Set_Proto_Mob_Table(mob_table, data, localMap))
			{
				fprintf(stderr, "Mob prototype table setting failed.\n");
			}

		} else {	//$$$$$$$$$$$$$$$$$$$$$$$ 테스트 몬스터 정보가 있다!
			TMobTable *tempTable = it_map_mobTable->second;

			mob_table->dwVnum               = tempTable->dwVnum;
			strncpy(mob_table->szName, tempTable->szName, CHARACTER_NAME_MAX_LEN);
			strncpy(mob_table->szLocaleName, tempTable->szLocaleName, CHARACTER_NAME_MAX_LEN);
			mob_table->bRank                = tempTable->bRank;
			mob_table->bType                = tempTable->bType;
			mob_table->bBattleType          = tempTable->bBattleType;
			mob_table->bLevel				= tempTable->bLevel;
			mob_table->bSize				= tempTable->bSize;
			mob_table->dwAIFlag				= tempTable->dwAIFlag;
			mob_table->dwRaceFlag				= tempTable->dwRaceFlag;
			mob_table->dwImmuneFlag				= tempTable->dwImmuneFlag;
			mob_table->bEmpire				= tempTable->bEmpire;
			strncpy(mob_table->szFolder, tempTable->szFolder, CHARACTER_NAME_MAX_LEN);
			mob_table->bOnClickType         = tempTable->bOnClickType;
			mob_table->bStr                 = tempTable->bStr;
			mob_table->bDex                 = tempTable->bDex;
			mob_table->bCon                 = tempTable->bCon;
			mob_table->bInt                 = tempTable->bInt;
			mob_table->dwDamageRange[0]     = tempTable->dwDamageRange[0];
			mob_table->dwDamageRange[1]     = tempTable->dwDamageRange[1];
			mob_table->dwMaxHP              = tempTable->dwMaxHP;
			mob_table->bRegenCycle          = tempTable->bRegenCycle;
			mob_table->bRegenPercent        = tempTable->bRegenPercent;
			mob_table->dwExp                = tempTable->dwExp;
			mob_table->wDef                 = tempTable->wDef;
			mob_table->sAttackSpeed         = tempTable->sAttackSpeed;
			mob_table->sMovingSpeed         = tempTable->sMovingSpeed;
			mob_table->bAggresiveHPPct      = tempTable->bAggresiveHPPct;
			mob_table->wAggressiveSight	= tempTable->wAggressiveSight;
			mob_table->wAttackRange		= tempTable->wAttackRange;
			mob_table->dwDropItemVnum	= tempTable->dwDropItemVnum;
			for (int i = 0; i < MOB_ENCHANTS_MAX_NUM; ++i)
				mob_table->cEnchants[i] = tempTable->cEnchants[i];
			for (int i = 0; i < MOB_RESISTS_MAX_NUM; ++i)
				mob_table->cResists[i] = tempTable->cResists[i];
			mob_table->fDamMultiply		= tempTable->fDamMultiply;
			mob_table->dwSummonVnum		= tempTable->dwSummonVnum;
			mob_table->dwDrainSP		= tempTable->dwDrainSP;
			mob_table->dwMonsterColor	= tempTable->dwMonsterColor;

		}

		fprintf(stdout, "MOB #%-5d %-16s %-16s sight: %u color %u[%d]\n", mob_table->dwVnum, mob_table->szName, mob_table->szLocaleName, mob_table->wAggressiveSight, mob_table->dwMonsterColor, 0);

		//셋에 vnum 추가
		vnumSet.insert(mob_table->dwVnum);

		++mob_table;
	}

	return true;
}




uint32_t g_adwMobProtoKey[4] =
{
	4813894,
	18955,
	552631,
	6822045
};


void SaveMobProto()
{
	FILE * fp;
	fopen_s(&fp, "mob_proto", "wb");
	if (!fp)
	{
		printf("cannot open %s for writing\n", "mob_proto");
		return;
	}

	DWORD fourcc = MAKEFOURCC('M', 'M', 'P', 'T');
	fwrite(&fourcc, sizeof(DWORD), 1, fp);

	DWORD dwElements = gs_nMobTableSize;
	fwrite(&dwElements, sizeof(DWORD), 1, fp);

	CLZObject zObj;

	printf("sizeof(TMobTable) %d\n", sizeof(TMobTable));

	if (!CLZO::Instance().CompressEncryptedMemory(zObj, gs_pMobTable, sizeof(TMobTable) * gs_nMobTableSize, g_adwMobProtoKey))
	{
		printf("cannot compress\n");
		fclose(fp);
		return;
	}

	const CLZObject::THeader & r = zObj.GetHeader();

	printf("MobProto count %u\n%u --Compress--> %u --Encrypt--> %u, GetSize %u\n",
			gs_nMobTableSize, r.dwRealSize, r.dwCompressedSize, r.dwEncryptSize, zObj.GetSize());

	DWORD dwDataSize = zObj.GetSize();
	fwrite(&dwDataSize, sizeof(DWORD), 1, fp);
	fwrite(zObj.GetBuffer(), dwDataSize, 1, fp);

	fclose(fp);
}

// YMIR WON'T SAVE THE FOLLOWING FIELDS, SO YOU WILL GET 0 FROM THEM WHEN YOU UNPACK THE MOB_PROTO:
// gold min, gold max, polymorph item, mount capacity
// skill_*
// sp_*
// NB: damage multiply is truncated, so you will get 1 instead of 1.4 and so on
// NB2: due to a bug, all the mob_protos packed with an unfixed/default dump_proto tool will get the race flag set to 0 if it contains more than 1 value (the bug is about the splitting feature)
void LoadMobProto()
{
	FILE * fp;
	uint32_t fourcc, tableSize, dataSize;

	fopen_s(&fp, "mob_proto", "rb");
	if (fp==NULL)
	{
		printf("mob_proto not found\n");
		return;
	}

	fread(&fourcc, sizeof(DWORD), 1, fp);
	fread(&tableSize, sizeof(DWORD), 1, fp);
	fread(&dataSize, sizeof(DWORD), 1, fp);
	BYTE * data = (BYTE *) malloc(dataSize);

	printf("fourcc %u\n", fourcc);
	printf("tableSize %u\n", tableSize);
	printf("dataSize %u\n", dataSize);

	if (data)
	{
		fread(data, dataSize, 1, fp);

		CLZObject zObj;

		if (CLZO::Instance().Decompress(zObj, data, g_adwMobProtoKey))
		{
			printf("real_size %u\n", zObj.GetSize());
			DWORD structSize = zObj.GetSize() / tableSize;
			DWORD structDiff = zObj.GetSize() % tableSize;
			printf("struct_size %u\n", structSize);
			printf("struct_diff %u\n", structDiff);

			if ((zObj.GetSize() % sizeof(TMobTable)) != 0)
			{
				printf("LoadMobProto: invalid size %u check data format. structSize %u, structDiff %u\n", zObj.GetSize(), structSize, structDiff);
				return;
			}


			if (gs_bPrintDebug)
			{
				for (DWORD i = 0; i < tableSize; ++i)
				{
					TMobTable & rTable = *((TMobTable *) zObj.GetBuffer() + i);
					printf("%u %s\n", rTable.dwVnum, rTable.szLocaleName);
				}
			}
			else
			{
				FILE * mf1; fopen_s(&mf1, "mob_names.txt", "w");
				FILE * mf2; fopen_s(&mf2, "mob_proto.txt", "w");
				if (mf1==NULL)
				{
					printf("mob_names.txt not writable");
					return;
				}
				if (mf2==NULL)
				{
					printf("mob_proto.txt not writable");
					return;
				}
				fprintf(mf1, "VNUM\tLOCALE_NAME\n");
				fprintf(mf2, "VNUM\tNAME\tRANK\tTYPE\tBATTLE_TYPE\tLEVEL\tSIZE\tAI_FLAG\tMOUNT_CAPACITY\tRACE_FLAG\tIMMUNE_FLAG\tEMPIRE\tFOLDER\tON_CLICK\tST\tDX\tHT\tIQ\tDAMAGE_MIN\tDAMAGE_MAX\tMAX_HP\tREGEN_CYCLE\tREGEN_PERCENT\tGOLD_MIN\tGOLD_MAX\tEXP\tDEF\tATTACK_SPEED\tMOVE_SPEED\tAGGRESSIVE_HP_PCT\tAGGRESSIVE_SIGHT\tATTACK_RANGE\tDROP_ITEM\tRESURRECTION_VNUM\tENCHANT_CURSE\tENCHANT_SLOW\tENCHANT_POISON\tENCHANT_STUN\tENCHANT_CRITICAL\tENCHANT_PENETRATE\tRESIST_SWORD\tRESIST_TWOHAND\tRESIST_DAGGER\tRESIST_BELL\tRESIST_FAN\tRESIST_BOW\tRESIST_FIRE\tRESIST_ELECT\tRESIST_MAGIC\tRESIST_WIND\tRESIST_POISON\tDAM_MULTIPLY\tSUMMON\tDRAIN_SP\tMOB_COLOR\tPOLYMORPH_ITEM\tSKILL_LEVEL0\tSKILL_VNUM0\tSKILL_LEVEL1\tSKILL_VNUM1\tSKILL_LEVEL2\tSKILL_VNUM2\tSKILL_LEVEL3\tSKILL_VNUM3\tSKILL_LEVEL4\tSKILL_VNUM4\tSP_BERSERK\tSP_STONESKIN\tSP_GODSPEED\tSP_DEATHBLOW\tSP_REVIVE\n");
				for (DWORD i = 0; i < tableSize; ++i)
				{
					TMobTable & rTable = *((TMobTable *) zObj.GetBuffer() + i);

					fprintf(mf1, "%u	%s\n", rTable.dwVnum, rTable.szLocaleName);
					fprintf(mf2,
						"%u	%s"
						"	%s	%s	%s	%u"
						"	%s	%s	%u	%s"
						"	%s	%u	%s	%u	%u	%u	%u	%u"
						"	%u	%u	%u	%u	%u	%u	%u	%u	%u"
						"	%d	%d	%u	%u	%u	%u	%u"
						"	%d	%d	%d"
						"	%d	%d	%d"
						"	%d	%d	%d	%d"
						"	%d	%d"
						"	%d	%d"
						"	%d	%d	%d"
						"	%.1f	%u	%u	%u	%u"
						"	%u	%u	%u	%u	%u	%u"
						"	%u	%u	%u	%u"
						"	%u	%u	%u	%u	%u"
						"\n",
						rTable.dwVnum, rTable.szName,
						get_Mob_Rank_Value(rTable.bRank).c_str(), get_Mob_Type_Value(rTable.bType).c_str(), set_Mob_BattleType_Value(rTable.bBattleType).c_str(), rTable.bLevel,
						set_Mob_Size_Value(rTable.fScale).c_str(), set_Mob_AIFlag_Value(rTable.dwAIFlag).c_str(), rTable.bMountCapacity, set_Mob_RaceFlag_Value(rTable.dwRaceFlag).c_str(),
						set_Mob_ImmuneFlag_Value(rTable.dwImmuneFlag).c_str(), rTable.bEmpire, rTable.szFolder, rTable.bOnClickType, rTable.bStr, rTable.bDex, rTable.bCon, rTable.bInt,
						rTable.dwDamageRange[0], rTable.dwDamageRange[1], rTable.dwMaxHP, rTable.bRegenCycle, rTable.bRegenPercent, rTable.dwGoldMin, rTable.dwGoldMax, rTable.dwExp, rTable.wDef,
						rTable.sAttackSpeed, rTable.sMovingSpeed, rTable.bAggresiveHPPct, rTable.wAggressiveSight, rTable.wAttackRange, rTable.dwDropItemVnum, rTable.dwResurrectionVnum,
						rTable.cEnchants[MOB_ENCHANT_CURSE], rTable.cEnchants[MOB_ENCHANT_SLOW], rTable.cEnchants[MOB_ENCHANT_POISON],
						rTable.cEnchants[MOB_ENCHANT_STUN], rTable.cEnchants[MOB_ENCHANT_CRITICAL], rTable.cEnchants[MOB_ENCHANT_PENETRATE],
						rTable.cResists[MOB_RESIST_SWORD], rTable.cResists[MOB_RESIST_TWOHAND], rTable.cResists[MOB_RESIST_DAGGER], rTable.cResists[MOB_RESIST_BELL],
						rTable.cResists[MOB_RESIST_FAN], rTable.cResists[MOB_RESIST_BOW],
						rTable.cResists[MOB_RESIST_FIRE], rTable.cResists[MOB_RESIST_ELECT],
						rTable.cResists[MOB_RESIST_MAGIC], rTable.cResists[MOB_RESIST_WIND], rTable.cResists[MOB_RESIST_POISON],
						rTable.fDamMultiply, rTable.dwSummonVnum, rTable.dwDrainSP, rTable.dwMonsterColor, rTable.dwPolymorphItemVnum,
						rTable.Skills[0].bLevel, rTable.Skills[0].dwVnum, rTable.Skills[1].bLevel, rTable.Skills[1].dwVnum, rTable.Skills[2].bLevel, rTable.Skills[2].dwVnum,
						rTable.Skills[3].bLevel, rTable.Skills[3].dwVnum, rTable.Skills[4].bLevel, rTable.Skills[4].dwVnum,
						rTable.bBerserkPoint, rTable.bStoneSkinPoint, rTable.bGodSpeedPoint, rTable.bDeathBlowPoint, rTable.bRevivePoint
					);
				}
				;
			}
		}

		free(data);
	}

	fclose(fp);
}




//==													==//
//==													==//
//==													==//
//===== 여기에서부터 아이템 =====//
//==													==//
//==													==//
//==													==//

string retrieveVnumRange(DWORD dwVnum, DWORD dwVnumRange)
{
	static char buf[10*2+1];
	if (dwVnumRange>0)
		snprintf(buf, sizeof(buf), "%u~%u", dwVnum, dwVnum+dwVnumRange);
	else
		snprintf(buf, sizeof(buf), "%u", dwVnum);
	return buf;
}

int retrieveAddonType(DWORD dwVnum)
{
	int addon_type = 0;
#ifdef ENABLE_ADDONTYPE_AUTODETECT
	static DWORD vnumlist[] = {180, 190, 290, 1130, 1170, 2150, 2170, 3160, 3210, 5110, 5120, 7160, 6010, 6060, 6070};
	for (DWORD i = 0; i < (sizeof(vnumlist)/sizeof(DWORD)); i++)
	{
		if ((dwVnum >= vnumlist[i]) && (dwVnum <= vnumlist[i]+9))
		{
			addon_type = -1;
		}
	}
#endif
	return addon_type;
}

bool BuildItemTable()
{
	//%%% <함수 설명> %%%//
	//1. 요약 : 'item_proto.txt', 'item_proto_test.txt', 'item_names.txt' 파일을 읽고,
	//		gs_pItemTable 를 구성한다.
	//2. 순서
	//	1)'item_names.txt' 파일을 읽어서 vnum:name 맵을 만든다.
	//	2)'item_proto_test.txt' 파일을 읽어서,
	//		test_item_table 를 만들고,
	//		vnum:TClientItemTable 맵을 만든다.
	//	3)'item_proto.txt' 파일을 읽고, gs_pItemTable를 구성한다.
	//		test_item_table에 있는 vnum은 테스트 데이터를 넣는다.
	//	4)test_item_table 데이터중에, gs_pItemTable 에 없는 데이터를 추가한다.
	//3. 테스트
	//	1)'item_proto.txt' 정보가 gs_pItemTable에 잘 들어갔는지.
	//	2)'item_names.txt' 정보가 gs_pItemTable에 잘 들어갔는지.
	//	3)'item_proto_test.txt' 에서 [겹치는] 정보가 gs_pItemTable 에 잘 들어갔는지.
	//	4)'item_proto_test.txt' 에서 [새로운] 정보가 gs_pItemTable 에 잘 들어갔는지.
	//^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^//

	fprintf(stderr, "sizeof(TClientItemTable): %u\n", sizeof(SItemTable));

	//=================================================================//
	//	1)'item_names.txt' 파일을 읽어서 vnum:name 맵을 만든다.
	//=================================================================//
	bool isNameFile = true;
	map<int,const char*> localMap;
	cCsvTable nameData;
	if(!nameData.Load("item_names.txt",'\t'))
	{
		fprintf(stderr, "item_names.txt Unable to read file\n");
		isNameFile = false;
	} else {
		nameData.Next();
		while(nameData.Next()) {
			localMap[atoi(nameData.AsStringByIndex(0))] = nameData.AsStringByIndex(1);
		}
	}
	//_________________________________________________________________//

	//===================== =======================//
	//	2)'item_proto_test.txt' 파일을 읽어서,
	//		test_item_table 를 만들고,
	//		vnum:TClientItemTable 맵을 만든다.
	//=============================================//
	map<DWORD, SItemTable *> test_map_itemTableByVnum;

	//^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^//


	//================================================================//
	//	3)'item_proto.txt' 파일을 읽고, gs_pItemTable를 구성한다.
	//		test_item_table에 있는 vnum은 테스트 데이터를 넣는다.
	//================================================================//

	//vnum들을 저장할 셋. 새로운 테스트 아이템을 판별할때 사용된다.
	set<int> vnumSet;

	//파일 읽어오기.
	cCsvTable data;
	if(!data.Load("item_proto.txt",'\t'))
	{
		fprintf(stderr, "item_proto.txt Unable to read file\n");
		return false;
	}
	data.Next(); //맨 윗줄 제외 (아이템 칼럼을 설명하는 부분)

	if (gs_pItemTable)
	{
		free(gs_pItemTable);
		gs_pItemTable = NULL;
	}

	//===== 아이템 테이블 생성 =====//
	//새로 추가되는 갯수를 파악한다.
	int addNumber = 0;
	while(data.Next()) {
		int vnum = atoi(data.AsStringByIndex(0));
		std::map<DWORD, SItemTable *>::iterator it_map_itemTable;
		it_map_itemTable = test_map_itemTableByVnum.find(vnum);
		if(it_map_itemTable != test_map_itemTableByVnum.end()) {
			addNumber++;
		}
	}
	//data를 다시 첫줄로 옮긴다.(다시 읽어온다;;)
	data.Destroy();
	if(!data.Load("item_proto.txt",'\t'))
	{
		fprintf(stderr, "item_proto.txt Unable to read file\n");
		return false;
	}
	data.Next(); //맨 윗줄 제외 (아이템 칼럼을 설명하는 부분)

	gs_nItemTableSize = data.m_File.GetRowCount()-1+addNumber;
	gs_pItemTable = new SItemTable[gs_nItemTableSize];
	memset(gs_pItemTable, 0, sizeof(SItemTable) * gs_nItemTableSize);

	SItemTable * item_table = gs_pItemTable;

	while (data.Next())
	{
		int col = 0;

		//테스트 파일에 같은 vnum이 있는지 조사.
		std::map<DWORD, SItemTable *>::iterator it_map_itemTable;
		it_map_itemTable = test_map_itemTableByVnum.find(atoi(data.AsStringByIndex(col)));
		if(it_map_itemTable == test_map_itemTableByVnum.end()) {


			if (!Set_Proto_Item_Table(item_table, data, localMap))
			{
				fprintf(stderr, "Mob prototype table setting failed.\n");
			}
		} else {	//$$$$$$$$$$$$$$$$$$$$$$$ 테스트 아이템 정보가 있다!
			SItemTable *tempTable = it_map_itemTable->second;

			item_table->dwVnum = tempTable->dwVnum;
			strncpy(item_table->szName, tempTable->szName, ITEM_NAME_MAX_LEN);
			strncpy(item_table->szLocaleName, tempTable->szLocaleName, ITEM_NAME_MAX_LEN);
			item_table->bType = tempTable->bType;
			item_table->bSubType = tempTable->bSubType;
			item_table->bSize = tempTable->bSize;
			item_table->dwAntiFlags = tempTable->dwAntiFlags;
			item_table->dwFlags = tempTable->dwFlags;
			item_table->dwWearFlags = tempTable->dwWearFlags;
			item_table->dwImmuneFlag = tempTable->dwImmuneFlag;
			item_table->dwIBuyItemPrice = tempTable->dwIBuyItemPrice;
			item_table->dwISellItemPrice = tempTable->dwISellItemPrice;
			item_table->dwRefinedVnum = tempTable->dwRefinedVnum;
			item_table->wRefineSet = tempTable->wRefineSet;
			item_table->bAlterToMagicItemPct = tempTable->bAlterToMagicItemPct;

			int i;
			for (i = 0; i < ITEM_LIMIT_MAX_NUM; ++i)
			{
				item_table->aLimits[i].bType = tempTable->aLimits[i].bType;
				item_table->aLimits[i].lValue = tempTable->aLimits[i].lValue;
			}

			for (i = 0; i < ITEM_APPLY_MAX_NUM; ++i)
			{
				item_table->aApplies[i].bType = tempTable->aApplies[i].bType;
				item_table->aApplies[i].lValue = tempTable->aApplies[i].lValue;
			}

			for (i = 0; i < ITEM_VALUES_MAX_NUM; ++i)
				item_table->alValues[i] = tempTable->alValues[i];

			item_table->bSpecular = tempTable->bSpecular;
			item_table->bGainSocketPct = tempTable->bGainSocketPct;

			item_table->bWeight = tempTable->bWeight;
		}


		fprintf(stdout, "ITEM #%-5u %-24s %-24s VAL: %ld %ld %ld %ld %ld %ld WEAR %u ANTI %u IMMUNE %u REFINE %u\n",
				item_table->dwVnum,
				item_table->szName,
				item_table->szLocaleName,
				item_table->alValues[0],
				item_table->alValues[1],
				item_table->alValues[2],
				item_table->alValues[3],
				item_table->alValues[4],
				item_table->alValues[5],
				item_table->dwWearFlags,
				item_table->dwAntiFlags,
				item_table->dwImmuneFlag,
				item_table->dwRefinedVnum);

		//vnum 저장.
		vnumSet.insert(item_table->dwVnum);
		++item_table;
	}

	return true;
}

uint32_t g_adwItemProtoKey[4] =
{
	173217,
	72619434,
	408587239,
	27973291
};

// YMIR WON'T SAVE THE FOLLOWING FIELDS, SO YOU WILL GET 0 FROM THEM WHEN YOU UNPACK THE ITEM_PROTO:
// addon_type
void LoadItemProto()
{
	FILE * fp;
	DWORD fourcc, tableSize, dataSize, protoVersion, structSize;

	fopen_s(&fp, "item_proto", "rb");
	if (fp==NULL)
	{
		printf("item_proto not found\n");
		return;
	}


	fread(&fourcc, sizeof(DWORD), 1, fp);
	fread(&protoVersion, sizeof(DWORD), 1, fp);
	fread(&structSize, sizeof(DWORD), 1, fp);
	fread(&tableSize, sizeof(DWORD), 1, fp);
	fread(&dataSize, sizeof(DWORD), 1, fp);
	BYTE * data = (BYTE *) malloc(dataSize);

	printf("fourcc %u\n", fourcc);
	printf("protoVersion %u\n", protoVersion);
	printf("struct_size %u\n", structSize);
	printf("tableSize %u\n", tableSize);
	printf("dataSize %u\n", dataSize);

	if (structSize != sizeof(SItemTable))
	{
		printf("LoadItemProto: invalid item_proto structSize[%d] != sizeof(SItemTable)[%d]\n", structSize, sizeof(SItemTable));
		return;
	}

	if (data)
	{
		fread(data, dataSize, 1, fp);

		CLZObject zObj;

		if (CLZO::Instance().Decompress(zObj, data, g_adwItemProtoKey))
		{
			printf("real_size %u\n", zObj.GetSize());

			if (gs_bPrintDebug)
			{
				for (DWORD i = 0; i < tableSize; ++i)
				{
					SItemTable & rTable = *((SItemTable *) zObj.GetBuffer() + i);

					printf("%u %s\n", rTable.dwVnum, rTable.szLocaleName);
				}
			}
			else
			{
				FILE * mf1; fopen_s(&mf1, "item_names.txt", "w");
				FILE * mf2; fopen_s(&mf2, "item_proto.txt", "w");
				if (mf1==NULL)
				{
					printf("item_names.txt not writable");
					return;
				}
				if (mf2==NULL)
				{
					printf("item_proto.txt not writable");
					return;
				}
				fprintf(mf1, "VNUM\tLOCALE_NAME\n");
				fprintf(mf2, "ITEM_VNUM~RANGE\tITEM_NAME(K)\tITEM_TYPE\tSUB_TYPE\tSIZE\tANTI_FLAG\tFLAG\tITEM_WEAR\tIMMUNE\tGOLD\tSHOP_BUY_PRICE\tREFINE\tREFINESET\tMAGIC_PCT\tLIMIT_TYPE0\tLIMIT_VALUE0\tLIMIT_TYPE1\tLIMIT_VALUE1\tADDON_TYPE0\tADDON_VALUE0\tADDON_TYPE1\tADDON_VALUE1\tADDON_TYPE2\tADDON_VALUE2\tVALUE0\tVALUE1\tVALUE2\tVALUE3\tVALUE4\tVALUE5\tSpecular\tSOCKET\tATTU_ADDON\n");
				for (DWORD i = 0; i < tableSize; ++i)
				{
					SItemTable & rTable = *((SItemTable *) zObj.GetBuffer() + i);

					fprintf(mf1, "%u	%s\n", rTable.dwVnum, rTable.szLocaleName);
					fprintf(mf2,
						"%s	%s" // 2
						"	%s	%s	%u	%s" // 6
						"	%s	%s	%s" // 9
						"	%u	%u	%u	%u	%u" // 14
						"	%s	%d	%s	%d" // 18
						"	%s	%d	%s	%d	%s	%d" // 24
						"	%d	%d	%d	%d	%d	%d" // 30
						"	%u	%u	%d" // 33
						"\n",
						retrieveVnumRange(rTable.dwVnum, rTable.dwVnumRange).c_str(), rTable.szName, // 2
						set_Item_Type_Value(rTable.bType).c_str(), set_Item_SubType_Value(rTable.bType, rTable.bSubType).c_str(), rTable.bSize, set_Item_AntiFlag_Value(rTable.dwAntiFlags).c_str(), // 6
						set_Item_Flag_Value(rTable.dwFlags).c_str(), set_Item_WearFlag_Value(rTable.dwWearFlags).c_str(), set_Item_Immune_Value(rTable.dwImmuneFlag).c_str(), // 9
						rTable.dwIBuyItemPrice, rTable.dwISellItemPrice, rTable.dwRefinedVnum, rTable.wRefineSet, rTable.bAlterToMagicItemPct, // 14
						set_Item_LimitType_Value(rTable.aLimits[0].bType).c_str(), rTable.aLimits[0].lValue, set_Item_LimitType_Value(rTable.aLimits[1].bType).c_str(), rTable.aLimits[1].lValue, // 18
						set_Item_ApplyType_Value(rTable.aApplies[0].bType).c_str(), rTable.aApplies[0].lValue, set_Item_ApplyType_Value(rTable.aApplies[1].bType).c_str(), rTable.aApplies[1].lValue,
						set_Item_ApplyType_Value(rTable.aApplies[2].bType).c_str(), rTable.aApplies[2].lValue, // 24
						rTable.alValues[0], rTable.alValues[1], rTable.alValues[2], rTable.alValues[3], rTable.alValues[4], rTable.alValues[5], // 30
						rTable.bSpecular, rTable.bGainSocketPct, retrieveAddonType(rTable.dwVnum) // 33
					);
				}
				;
			}
		}

		free(data);
	}

	fclose(fp);
}

void SaveItemProto()
{
	FILE * fp;
	fopen_s(&fp, "item_proto", "wb");
	if (!fp)
	{
		printf("cannot open %s for writing\n", "item_proto");
		return;
	}

	DWORD fourcc = MAKEFOURCC('M', 'I', 'P', 'X');
	fwrite(&fourcc, sizeof(DWORD), 1, fp);

	DWORD dwVersion = 0x00000001;
	fwrite(&dwVersion, sizeof(DWORD), 1, fp);

	DWORD dwStride = sizeof(SItemTable);
	fwrite(&dwStride, sizeof(DWORD), 1, fp);

	DWORD dwElements = gs_nItemTableSize;
	fwrite(&dwElements, sizeof(DWORD), 1, fp);

	CLZObject zObj;
	std::vector <SItemTable> vec_item_table (&gs_pItemTable[0], &gs_pItemTable[gs_nItemTableSize - 1]);
	sort (&gs_pItemTable[0], &gs_pItemTable[0] + gs_nItemTableSize);
	if (!CLZO::Instance().CompressEncryptedMemory(zObj, gs_pItemTable, sizeof(SItemTable) * gs_nItemTableSize, g_adwItemProtoKey))
	{
		printf("cannot compress\n");
		fclose(fp);
		return;
	}

	const CLZObject::THeader & r = zObj.GetHeader();

	printf("Elements %d\n%u --Compress--> %u --Encrypt--> %u, GetSize %u\n",
			gs_nItemTableSize,
			r.dwRealSize,
			r.dwCompressedSize,
			r.dwEncryptSize,
			zObj.GetSize());

	DWORD dwDataSize = zObj.GetSize();
	fwrite(&dwDataSize, sizeof(DWORD), 1, fp);
	fwrite(zObj.GetBuffer(), dwDataSize, 1, fp);
	fclose(fp);

	fopen_s(&fp, "item_proto", "rb");
	if (!fp)
	{
		printf("Error!!\n");
		return;
	}

	fread(&fourcc, sizeof(DWORD), 1, fp);
	fread(&dwElements, sizeof(DWORD), 1, fp);

	printf("Elements Check %u fourcc match %d\n", dwElements, fourcc == MAKEFOURCC('M', 'I', 'P', 'T'));
	fclose(fp);
}

#define NL "\n"
enum  optionIndex { UNKNOWN, HELP, DEBUG_MODE, ITEM_PROTO, MOB_PROTO, PACK, UNPACK };

const option::Descriptor usage[] = // don't use TAB in here!
{
	{UNKNOWN,		0,	"",		"",				option::Arg::None,		"USAGE: example [options]"},
	{HELP,			0,	"h",	"help",			option::Arg::None,		"    --help, -h             Print usage and exit." },
	{DEBUG_MODE,	0,	"d",	"debug",		option::Arg::None,		"    --debug, -d            Enable debug mode" },
	{ITEM_PROTO,	0,	"i",	"iproto",		option::Arg::None,		"    --iproto, -i           Process item proto" },
	{MOB_PROTO,		0,	"m",	"mproto",		option::Arg::None,		"    --mproto, -m           Process mob proto" },
	{PACK,			0,	"p",	"pack",			option::Arg::None,		"    --pack, -p             Pack" },
	{UNPACK,		0,	"u",	"unpack",		option::Arg::None,		"    --unpack, -u           Unpack" },
	{UNKNOWN,		0,	"",		"",				option::Arg::None,		NL "Examples:"
																		NL "    dump_proto.exe --help"
																		NL "    dump_proto.exe -dpim"
																		NL "    dump_proto.exe -duim"
																		NL "    dump_proto.exe -dpm"
																		NL "    dump_proto.exe -dum"
																		NL "    dump_proto.exe -dpi"
																		NL "    dump_proto.exe -dui"
	},
	{0,0,0,0,0,0}
};

int main(int argc, char ** argv)
{
	argc-=(argc>0); argv+=(argc>0); // skip program name argv[0] if present
	option::Stats  stats(usage, argc, argv);
	std::vector<option::Option> options(stats.options_max);
	std::vector<option::Option> buffer(stats.buffer_max);
	option::Parser parse(usage, argc, argv, &options[0], &buffer[0]);

	if (parse.error())
		return 1;

	if (argc == 0)
	{
		if (BuildMobTable())
		{
			SaveMobProto();
			cout << "BuildMobTable working normal" << endl;
		}

		if (BuildItemTable())
		{
			SaveItemProto();
			cout << "BuildItemTable working normal" << endl;
		}
		return 0;
	}

	bool bAItemProto = false;
	bool bAMobProto = false;
	bool bXPhase = false;
	for (int idx1 = 0; idx1 < parse.optionsCount(); ++idx1)
	{
		option::Option& opt = buffer[idx1];
		switch (opt.index())
		{
			case DEBUG_MODE:
				gs_bPrintDebug = true;
				break;
			case ITEM_PROTO:
				bAItemProto = true;
				break;
			case MOB_PROTO:
				bAMobProto = true;
				break;
			case PACK:
				bXPhase = false;
				break;
			case UNPACK:
				bXPhase = true;
				break;
			case HELP:
			default:
				option::printUsage(std::cout, usage);
				exit(0);
				break;
		}
	}

#ifdef _DEBUG
	printf("sizeof(SItemTable) %d\n", sizeof(SItemTable));
	printf("sizeof(TMobTable) %d\n", sizeof(TMobTable));
#endif

	if (bAItemProto)
	{
		if (bXPhase)
			LoadItemProto();
		else if (BuildItemTable())
			SaveItemProto();
	}
	if (bAMobProto)
	{
		if (bXPhase)
			LoadMobProto();
		else if (BuildMobTable())
			SaveMobProto();
	}

	return 0;
}

