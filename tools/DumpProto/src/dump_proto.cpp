#include "../include/dump_proto.h"
#include "../../../source/server/libthecore/include/main.h"
#include "../../../source/server/libthecore/include/log.h"
#include <cxxopts.hpp>
#include <filesystem>

#ifdef _DEBUG
	#pragma comment( lib, "lzo2_debug.lib" )
#else
	#pragma comment( lib, "lzo2.lib" )
#endif

static bool gs_bPrintLoadedThings = false;

static std::vector <TMobTable> gs_vecMobTable;
static std::vector <SItemTable> gs_vecItemTable;

static uint32_t g_adwMobProtoKey[4] =
{
	4813894,
	18955,
	552631,
	6822045
};
static uint32_t g_adwItemProtoKey[4] =
{
	173217,
	72619434,
	408587239,
	27973291
};


struct FCompareVnumItem
{
	bool operator () (const SItemTable& a, const SItemTable& b) const
	{
		return (a.dwVnum < b.dwVnum);
	}
};
struct FCompareVnumMob
{
	bool operator () (const TMobTable& a, const TMobTable& b) const
	{
		return (a.dwVnum < b.dwVnum);
	}
};


static std::string __RetrieveVnumRange(uint32_t dwVnum, uint32_t dwVnumRange)
{
	char buf[21];

	if (dwVnumRange > 0)
		snprintf(buf, sizeof(buf), "%u~%u", dwVnum, dwVnum + dwVnumRange);
	else
		snprintf(buf, sizeof(buf), "%u", dwVnum);

	return buf;
}
static int __RetrieveAddonType(uint32_t dwVnum)
{
	int addon_type = 0;
#ifdef ENABLE_ADDONTYPE_AUTODETECT
	uint32_t vnumlist[] = { 180, 190, 290, 1130, 1170, 2150, 2170, 3160, 3210, 5110, 5120, 7160, 6010, 6060, 6070 };
	for (uint32_t i = 0; i < _countof(vnumlist); i++)
	{
		if ((dwVnum >= vnumlist[i]) && (dwVnum <= vnumlist[i] + 9))
		{
			addon_type = -1;
		}
	}
#endif
	return addon_type;
}


bool BuildMobTable()
{
	// Load names
	std::map <int32_t, const char*> localMap;

	cCsvTable nameData;
	if(!nameData.Load("mob_names.txt",'\t'))
	{
		sys_err("mob_names.txt doesn't exist/incorrect format. Using mob_proto names!");
		return false;
	}
	nameData.Next();

	while (nameData.Next())
	{
		localMap[atoi(nameData.AsStringByIndex(0))] = nameData.AsStringByIndex(1);
	}

	// Load data
	cCsvTable data;
	if (!data.Load("mob_proto.csv", '\t'))
	{
		sys_err("mob_proto.csv doesn't exist or is incorrectly formatted");
		return false;
	}
	data.Next();

	// Clear old data (if exist)
	if (!gs_vecMobTable.empty())
	{
		sys_log(0, "RELOAD: mob_proto");
		gs_vecMobTable.clear();
	}

	// Move data to struct
	gs_vecMobTable.resize(data.m_File.GetRowCount() - 1);
	memset(&gs_vecMobTable[0], 0, sizeof(TMobTable) * gs_vecMobTable.size());
	TMobTable* mob_table = &gs_vecMobTable[0];

	for (; data.Next(); ++mob_table)
	{
		if (!Set_Proto_Mob_Table(mob_table, data, localMap))
			sys_err("Unable to set current proto table entry into mob_table VNUM: %d.", mob_table->dwVnum);

		printf("MOB #%-5d %-24s level: %-3u rank: %u empire: %d", mob_table->dwVnum, mob_table->szLocaleName, mob_table->bLevel, mob_table->bRank, mob_table->bEmpire);
	}

	sort(gs_vecMobTable.begin(), gs_vecMobTable.end(), FCompareVnumMob());

	data.Destroy();
	nameData.Destroy();

	return true;
}
void SaveMobProto()
{
	msl::file_ptr file("mob_proto", "wb");
	if (!file)
	{
		sys_err("mob_proto could not open!");
		return;
	}

	auto fourcc = MAKEFOURCC('M', 'M', 'P', 'T');
	file.write(&fourcc, sizeof(fourcc));

	auto dwElements = gs_vecMobTable.size();
	file.write(&dwElements, sizeof(dwElements));

	CLZObject zObj;
	if (!CLZO::Instance().CompressEncryptedMemory(zObj, gs_vecMobTable.data(), sizeof(TMobTable) * gs_vecMobTable.size(), g_adwMobProtoKey))
	{
		sys_err("mob_proto could not compress!");
		return;
	}

	const auto& r = zObj.GetHeader();

	sys_log(0, "MobProto element count %u table size: %u", gs_vecMobTable.size(), sizeof(TMobTable) * gs_vecMobTable.size());
	sys_log(0, "MobProto real_size: %u compressed_size: %u encrypted_size: %u output_size: %u", r.dwRealSize, r.dwCompressedSize, r.dwEncryptSize, zObj.GetSize());

	auto dwDataSize = zObj.GetSize();
	file.write(&dwDataSize, sizeof(dwDataSize));

	file.write(zObj.GetBuffer(), dwDataSize);
}
bool LoadMobProto(bool validate)
{
	msl::file_ptr file("mob_proto", "rb");
	if (!file)
	{
		sys_err("mob_proto could not open!");
		return false;
	}

	uint32_t fourcc = 0;
	file.read(&fourcc, sizeof(fourcc));
	sys_log(0, "mob_proto fourcc %u", fourcc);

	if (fourcc != MAKEFOURCC('M', 'M', 'P', 'T'))
	{
		sys_err("mob_proto magic is not valid: %p", fourcc);
		return false;
	}

	uint32_t tableSize = 0;
	file.read(&tableSize, sizeof(tableSize));
	sys_log(0, "mob_proto tableSize %u", tableSize);

	uint32_t dataSize = 0;
	file.read(&dataSize, sizeof(dataSize));
	sys_log(0, "mob_proto dataSize %u", dataSize);

	std::unique_ptr <uint8_t[]> buf(new uint8_t[dataSize]);

	if (buf)
	{
		file.read(buf.get(), dataSize);

		CLZObject zObj;
		if (!CLZO::Instance().Decompress(zObj, buf.get(), g_adwMobProtoKey))
		{
			sys_err("mob_proto could not decompress!");
			return false;
		}

		auto realSize = zObj.GetSize();
		sys_log(0, "mob_proto decompressed object real_size %u", realSize);

		auto structSize = realSize / tableSize;
		sys_log(0, "mob_proto decompressed object structSize %u", structSize);
	
		auto structDiff = realSize % tableSize;
		sys_log(0, "mob_proto decompressed object structDiff %u", structDiff);

		if ((realSize % sizeof(TMobTable)) != 0)
		{
			sys_err("LoadMobProto: invalid size %u check data format", realSize);
			return false;
		}

		if (gs_bPrintLoadedThings)
		{
			for (uint32_t i = 0; i < tableSize; ++i)
			{
				const auto& rTable = *((TMobTable*)zObj.GetBuffer() + i);
				sys_log(0, "%u %s", rTable.dwVnum, rTable.szLocaleName);
			}
		}
		else if (!validate)
		{
			msl::file_ptr namef("mob_names.txt", "w");
			if (!file)
			{
				sys_err("mob_names.txt could not open!");
				return false;
			}
			msl::file_ptr tablef("mob_proto.csv", "w");
			if (!file)
			{
				sys_err("mob_proto.csv could not open!");
				return false;
			}

			// Top of the file, Commented info line
			namef.string_write("VNUM\tLOCALE_NAME\n");
			tablef.string_write("VNUM\tNAME\tRANK\tTYPE\tBATTLE_TYPE\tLEVEL\tSIZE\tAI_FLAG\tMOUNT_CAPACITY\tRACE_FLAG\tIMMUNE_FLAG\tEMPIRE\tFOLDER\tON_CLICK\tST\tDX\tHT\tIQ\tDAMAGE_MIN\tDAMAGE_MAX\tMAX_HP\tREGEN_CYCLE\tREGEN_PERCENT\tGOLD_MIN\tGOLD_MAX\tEXP\tDEF\tATTACK_SPEED\tMOVE_SPEED\tAGGRESSIVE_HP_PCT\tAGGRESSIVE_SIGHT\tATTACK_RANGE\tDROP_ITEM\tRESURRECTION_VNUM\tENCHANT_CURSE\tENCHANT_SLOW\tENCHANT_POISON\tENCHANT_STUN\tENCHANT_CRITICAL\tENCHANT_PENETRATE\tRESIST_SWORD\tRESIST_TWOHAND\tRESIST_DAGGER\tRESIST_BELL\tRESIST_FAN\tRESIST_BOW\tRESIST_FIRE\tRESIST_ELECT\tRESIST_MAGIC\tRESIST_WIND\tRESIST_POISON\tDAM_MULTIPLY\tSUMMON\tDRAIN_SP\tMOB_COLOR\tPOLYMORPH_ITEM\tSKILL_LEVEL0\tSKILL_VNUM0\tSKILL_LEVEL1\tSKILL_VNUM1\tSKILL_LEVEL2\tSKILL_VNUM2\tSKILL_LEVEL3\tSKILL_VNUM3\tSKILL_LEVEL4\tSKILL_VNUM4\tSP_BERSERK\tSP_STONESKIN\tSP_GODSPEED\tSP_DEATHBLOW\tSP_REVIVE\n");

			// Data
			for (uint32_t i = 0; i < tableSize; ++i)
			{
				const auto& rTable = *((TMobTable*)zObj.GetBuffer() + i);
				namef.write("%u\t%s\n", rTable.dwVnum, rTable.szLocaleName);
				tablef.write(
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
					set_Mob_Rank_Value(rTable.bRank).c_str(), set_Mob_Type_Value(rTable.bType).c_str(), set_Mob_BattleType_Value(rTable.bBattleType).c_str(), rTable.bLevel,
					set_Mob_Size_Value(rTable.bSize).c_str(), set_Mob_AIFlag_Value(rTable.dwAIFlag).c_str(), rTable.bMountCapacity, set_Mob_RaceFlag_Value(rTable.dwRaceFlag).c_str(),
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
		}
	}
	return true;
}


bool BuildItemTable()
{
	// Load names
	std::map <int32_t, const char*> localMap;

	cCsvTable nameData;
	if (!nameData.Load("item_names.txt", '\t'))
	{
		sys_err("item_names.txt couldn't be loaded or its format is incorrect.");
		return false; // There's no reason to continue without names for us (i dont like korean)
	}

	nameData.Next(); // skip the description

	while (nameData.Next())
	{
		localMap[atoi(nameData.AsStringByIndex(0))] = nameData.AsStringByIndex(1);
	}

	// Load data
	cCsvTable data;
	if (!data.Load("item_proto.csv", '\t'))
	{
		sys_err("item_proto.csv couldn't be loaded or the format is incorrect.");
		return false;
	}
	data.Next();

	// Clear old data (if exist)
	if (!gs_vecItemTable.empty())
	{
		sys_log(0, "RELOAD: item_proto");
		gs_vecItemTable.clear();
	}

	// Move data to struct
	gs_vecItemTable.resize(data.m_File.GetRowCount() - 1);
	memset(&gs_vecItemTable[0], 0, sizeof(SItemTable) * gs_vecItemTable.size());
	SItemTable* item_table = &gs_vecItemTable[0];

	for (; data.Next(); ++item_table)
	{
		if (!Set_Proto_Item_Table(reinterpret_cast<SItemTable_Server*>(item_table), data, localMap))
			sys_err("Invalid item table. VNUM: %d", item_table->dwVnum);

		printf("ITEM #%-5d %-24s\n", item_table->dwVnum, item_table->szLocaleName);
	}

	sort(gs_vecItemTable.begin(), gs_vecItemTable.end(), FCompareVnumItem());

	data.Destroy();
	nameData.Destroy();

	return true;
}
void SaveItemProto()
{
	msl::file_ptr file("item_proto", "wb");
	if (!file)
	{
		sys_err("item_proto could not open!");
		return;
	}

	auto fourcc = MAKEFOURCC('M', 'I', 'P', 'X');
	file.write(&fourcc, sizeof(fourcc));

	auto dwVersion = 1U;
	file.write(&dwVersion, sizeof(dwVersion));

	auto dwStride = sizeof(SItemTable);
	file.write(&dwStride, sizeof(dwStride));

	auto dwElements = static_cast<uint32_t>(gs_vecItemTable.size());
	file.write(&dwElements, sizeof(dwElements));

	CLZObject zObj;
	if (!CLZO::Instance().CompressEncryptedMemory(zObj, gs_vecItemTable.data(), sizeof(SItemTable) * gs_vecItemTable.size(), g_adwItemProtoKey))
	{
		sys_err("item_proto could not compress!");
		return;
	}

	const auto& r = zObj.GetHeader();

	sys_log(0, "ItemProto element count %u table size: %u", gs_vecItemTable.size(), sizeof(SItemTable) * gs_vecItemTable.size());
	sys_log(0, "ItemProto real_size: %u compressed_size: %u encrypted_size: %u output_size: %u", r.dwRealSize, r.dwCompressedSize, r.dwEncryptSize, zObj.GetSize());

	auto dwDataSize = zObj.GetSize();
	file.write(&dwDataSize, sizeof(dwDataSize));

	file.write(zObj.GetBuffer(), dwDataSize);
}
bool LoadItemProto(bool validate)
{
	msl::file_ptr file("item_proto", "rb");
	if (!file)
	{
		sys_err("item_proto could not open!");
		return false;
	}

	uint32_t fourcc = 0;
	file.read(&fourcc, sizeof(fourcc));
	sys_log(0, "item_proto fourcc %u", fourcc);

	if (fourcc != MAKEFOURCC('M', 'I', 'P', 'X'))
	{
		sys_err("item_proto magic is not valid: %p", fourcc);
		return false;
	}

	uint32_t protoVersion = 0;
	file.read(&protoVersion, sizeof(protoVersion));
	sys_log(0, "item_proto protoVersion %u", protoVersion);

	uint32_t structSize = 0;
	file.read(&structSize, sizeof(structSize));
	sys_log(0, "item_proto structSize %u", structSize);

	uint32_t tableSize = 0;
	file.read(&tableSize, sizeof(tableSize));
	sys_log(0, "item_proto tableSize %u", tableSize);

	uint32_t dataSize = 0;
	file.read(&dataSize, sizeof(dataSize));
	sys_log(0, "item_proto dataSize %u", dataSize);

	if (structSize != sizeof(SItemTable))
	{
		sys_err("LoadItemProto: invalid item_proto structSize[%d] != sizeof(SItemTable)[%d]\n", structSize, sizeof(SItemTable));
		return false;
	}

	std::unique_ptr <uint8_t[]> buf(new uint8_t[dataSize]);

	if (buf)
	{
		file.read(buf.get(), dataSize);

		CLZObject zObj;
		if (!CLZO::Instance().Decompress(zObj, buf.get(), g_adwItemProtoKey))
		{
			sys_err("item_proto could not decompress!");
			return false;
		}

		sys_log(0, "item_proto real_size %u", zObj.GetSize());

		if (gs_bPrintLoadedThings)
		{
			for (uint32_t i = 0; i < tableSize; ++i)
			{
				const auto& rTable = *((SItemTable*)zObj.GetBuffer() + i);
				sys_log(0, "%u %s", rTable.dwVnum, rTable.szLocaleName);
			}
		}
		else if (!validate)
		{
			msl::file_ptr namef("item_names.txt", "w");
			if (!file)
			{
				sys_err("item_names.txt could not open!");
				return false;
			}
			msl::file_ptr tablef("item_proto.csv", "w");
			if (!file)
			{
				sys_err("item_proto.csv could not open!");
				return false;
			}

			// Top of the file, Commented info line
			namef.string_write("VNUM\tLOCALE_NAME\n");
			tablef.string_write("ITEM_VNUM~RANGE\tITEM_NAME(K)\tITEM_TYPE\tSUB_TYPE\tSIZE\tANTI_FLAG\tFLAG\tITEM_WEAR\tIMMUNE\tGOLD\tSHOP_BUY_PRICE\tREFINE\tREFINESET\tMAGIC_PCT\tLIMIT_TYPE0\tLIMIT_VALUE0\tLIMIT_TYPE1\tLIMIT_VALUE1\tADDON_TYPE0\tADDON_VALUE0\tADDON_TYPE1\tADDON_VALUE1\tADDON_TYPE2\tADDON_VALUE2\tVALUE0\tVALUE1\tVALUE2\tVALUE3\tVALUE4\tVALUE5\tSpecular\tSOCKET\tATTU_ADDON\n");
			
			// Data
			for (uint32_t i = 0; i < tableSize; ++i)
			{
				const auto& rTable = *((SItemTable*)zObj.GetBuffer() + i);
				namef.write("%u\t%s\n", rTable.dwVnum, rTable.szLocaleName);
				tablef.write(
					"%s	%s" // 2
					"	%s	%s	%u	%s" // 6
					"	%s	%s	%s" // 9
					"	%u	%u	%u	%u	%u" // 14
					"	%s	%d	%s	%d" // 18
					"	%s	%d	%s	%d	%s	%d" // 24
					"	%d	%d	%d	%d	%d	%d" // 30
					"	%u	%u	%d" // 33
					"\n",
					__RetrieveVnumRange(rTable.dwVnum, rTable.dwVnumRange).c_str(), rTable.szName, // 2
					set_Item_Type_Value(rTable.bType).c_str(), set_Item_SubType_Value(rTable.bType, rTable.bSubType).c_str(), rTable.bSize, set_Item_AntiFlag_Value(rTable.dwAntiFlags).c_str(), // 6
					set_Item_Flag_Value(rTable.dwFlags).c_str(), set_Item_WearFlag_Value(rTable.dwWearFlags).c_str(), set_Item_Immune_Value(rTable.dwImmuneFlag).c_str(), // 9
					rTable.dwIBuyItemPrice, rTable.dwISellItemPrice, rTable.dwRefinedVnum, rTable.wRefineSet, rTable.bAlterToMagicItemPct, // 14
					set_Item_LimitType_Value(rTable.aLimits[0].bType).c_str(), rTable.aLimits[0].lValue, set_Item_LimitType_Value(rTable.aLimits[1].bType).c_str(), rTable.aLimits[1].lValue, // 18
					set_Item_ApplyType_Value(rTable.aApplies[0].bType).c_str(), rTable.aApplies[0].lValue, set_Item_ApplyType_Value(rTable.aApplies[1].bType).c_str(), rTable.aApplies[1].lValue,
					set_Item_ApplyType_Value(rTable.aApplies[2].bType).c_str(), rTable.aApplies[2].lValue, // 24
					rTable.alValues[0], rTable.alValues[1], rTable.alValues[2], rTable.alValues[3], rTable.alValues[4], rTable.alValues[5], // 30
					rTable.bSpecular, rTable.bGainSocketPct, __RetrieveAddonType(rTable.dwVnum) // 33
				);
			}
		}
	}
	return true;
}


int main(int argc, char ** argv)
{
	// The core
	thecore_init();
	bool is_thecore_initialized = thecore_set(25,[](LPHEART heart, int32_t pulse) -> void {});
	if (!is_thecore_initialized)
	{
		fprintf(stderr, "Could not initialize thecore\n");
		return EXIT_FAILURE;
	}
#ifdef _DEBUG
	sys_log(0, "TheCore initilization completed");
#endif

	// Initialize singletons
	static CLZO _lzo;

	// Struct sizes
#ifdef _DEBUG
	sys_log(0, "sizeof(TMobTable): %u", sizeof(TMobTable));
	sys_log(0, "sizeof(SItemTable): %u", sizeof(SItemTable));
#endif

	// Parse command line
	cxxopts::Options options("DumpProto", "The dump proto tool");

	options.add_options()
		("h,help",	"Print usage")
		("d,debug",	"Enable debug mode")
		("i,iproto","Process item proto")
		("m,mproto","Process mob proto")
		("p,pack",	"Pack")
		("u,unpack","Unpack")
	;
	if (argc < 2)
	{
		std::cerr << std::endl << "Examples:" << std::endl <<
			"\tGet help about usage of the program:" << std::endl <<
			"\t\t" << argv[0] << " --help" << std::endl <<
			"\tDebug | Pack | Item | Mob:" << std::endl <<
			"\t\t" << argv[0] << " -dpim" << std::endl <<
			"\tDebug | Unpack | Item | Mob:" << std::endl <<
			"\t\t" << argv[0] << " -duim" << std::endl <<
			"\tDebug | Pack | Mob:" << std::endl <<
			"\t\t" << argv[0] << " -dpm" << std::endl <<
			"\tDebug | Unpack | Mob:" << std::endl <<
			"\t\t" << argv[0] << " -dum" << std::endl <<
			"\tDebug | Pack | Item:" << std::endl <<
			"\t\t" << argv[0] << " -dpi" << std::endl <<
			"\tDebug | Unpack | Item:" << std::endl <<
			"\t\t" << argv[0] << " -dui";
		return EXIT_FAILURE;
	}

	auto bProcessItemProto = false;
	auto bProcessMobProto = false;
	auto bPack = std::optional<bool>();
	try
	{
		auto result = options.parse(argc, argv);
		if (result.count("help"))
		{
			std::cerr << options.help() << std::endl;
			return EXIT_FAILURE;
		}

		if (result.count("debug")) {
			bDumpProtoDebugFlag = true;
			sys_log(0, "Debugging enabled");
		}
		if (result.count("iproto")) {
			bProcessItemProto = true;
			sys_log(0, "item_proto processing enabled");
		}
		if (result.count("mproto")) {
			bProcessMobProto = true;
			sys_log(0, "mob_proto processing enabled");
		}
		if (result.count("pack")) {
			bPack = true;
			sys_log(0, "Work type: packing");
		}
		if (result.count("unpack")) {
			bPack = false;
			sys_log(0, "Work type: unpacking");
		}
	}
	catch (const cxxopts::OptionException& ex)
	{
		sys_err("cxxopts exception: %s", ex.what());
		return EXIT_FAILURE;
	}

	if (!bPack.has_value())
	{
		sys_err("Undefined work type! Please define work type (pack/unpack)");
		return EXIT_FAILURE;
	}
	if (!bProcessItemProto && !bProcessMobProto)
	{
		sys_err("Undefined process type! Please define process type (item/mob)");
		return EXIT_FAILURE;
	}

	if (bProcessItemProto)
	{
		sys_log(0, "item_proto processing started.");

		if (bPack.value() == true) // Pack processing
		{
			// Step 1: Create item table in memory from resources(.txt/.csv)
			if (!BuildItemTable())
			{
				sys_err("Item table could not build");
				return EXIT_FAILURE;
			}

			// Step 2: Save created item table to item_proto file
			SaveItemProto();

			// Step 3: Validation saved item_proto
			if (!LoadItemProto(true))
			{
				sys_err("Item table could validate");
				std::filesystem::remove("item_proto");
				return EXIT_FAILURE;
			}
		}
		else // Unpack processing
		{
			// Step 1: Load & save from item_proto to .txt/.csv resources
			if (!LoadItemProto(false))
			{
				sys_err("Item table could validate");
				std::filesystem::remove("item_proto");
				return EXIT_FAILURE;
			}
		}
		sys_log(0, "item_proto processing completed.");
	}
	if (bProcessMobProto)
	{
		sys_log(0, "mob_proto processing started.");

		if (bPack.value() == true) // Pack processing
		{
			// Step 1: Create mob table in memory from resources(.txt/.csv)
			if (!BuildMobTable())
			{
				sys_err("Mob table could not build");
				return EXIT_FAILURE;
			}

			// Step 2: Save created mob table to mob_proto file
			SaveMobProto();

			// Step 3: Validation saved mob_proto
			if (!LoadMobProto(true))
			{
				sys_err("Mob table could validate");
				std::filesystem::remove("mob_proto");
				return EXIT_FAILURE;
			}
		}
		else // Unpack processing
		{
			// Step 1: Load & save from mob_proto to .txt/.csv resources
			if (!LoadMobProto(false))
			{
				sys_err("Mob table could validate");
				std::filesystem::remove("mob_proto");
				return EXIT_FAILURE;
			}
		}
		sys_log(0, "mob_proto processing completed.");
	}

	thecore_destroy();
	return EXIT_SUCCESS;
}
