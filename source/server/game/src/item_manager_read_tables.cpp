#include "stdafx.h"
#include "utils.h"
#include "config.h"
#include "char.h"
#include "char_manager.h"
#include "desc_client.h"
#include "db.h"
#include "log.h"
#include "skill.h"
#include "text_file_loader.h"
#include "priv_manager.h"
#include "quest_manager.h"
#include "unique_item.h"
#include "safebox.h"
#include "blend_item.h"
#include "dev_log.h"
#include "locale_service.h"
#include "item.h"
#include "item_manager.h"
#include "item_manager_private_types.h"
#include "group_text_parse_tree.h"

std::vector<CItemDropInfo> g_vec_pkCommonDropItem[MOB_RANK_MAX_NUM];

bool ITEM_MANAGER::ReadCommonDropItemFile(const char * c_pszFileName)
{
	auto fp = msl::file_ptr(c_pszFileName, "r");
	if (!fp)
	{
		sys_err("Cannot open %s", c_pszFileName);
		return false;
	}

	char buf[1024];

	int32_t lines = 0;

	while (fgets(buf, 1024, fp.get()))
	{
		++lines;

		if (!*buf || *buf == '\n')
			continue;

		TDropItem d[MOB_RANK_MAX_NUM];
		char szTemp[64];

		memset(&d, 0, sizeof(d));

		char * p = buf;
		char * p2;

		for (int32_t i = 0; i <= MOB_RANK_S_KNIGHT; ++i)
		{
			for (int32_t j = 0; j < 6; ++j)
			{
				p2 = strchr(p, '\t');

				if (!p2)
					break;

				strlcpy(szTemp, p, MIN(sizeof(szTemp), (p2 - p) + 1));
				p = p2 + 1;

				switch (j)
				{
				case 0: break;
				case 1: str_to_number(d[i].iLvStart, szTemp);	break;
				case 2: str_to_number(d[i].iLvEnd, szTemp);	break;
				case 3: d[i].fPercent = atof(szTemp);	break;
				case 4: strlcpy(d[i].szItemName, szTemp, sizeof(d[i].szItemName));	break;
				case 5: str_to_number(d[i].iCount, szTemp);	break;
				}
			}

			uint32_t dwPct = (uint32_t) (d[i].fPercent * 10000.0f);
			uint32_t dwItemVnum = 0;

			if (!ITEM_MANAGER::Instance().GetVnumByOriginalName(d[i].szItemName, dwItemVnum))
			{
				// 이름으로 못찾으면 번호로 검색
				str_to_number(dwItemVnum, d[i].szItemName);
				if (!ITEM_MANAGER::Instance().GetTable(dwItemVnum))
				{
					sys_err("No such an item (name: %s)", d[i].szItemName);
					return false;
				}
			}

			if (d[i].iLvStart == 0)
				continue;

			g_vec_pkCommonDropItem[i].push_back(CItemDropInfo(d[i].iLvStart, d[i].iLvEnd, dwPct, dwItemVnum));
		}
	}


	for (int32_t i = 0; i < MOB_RANK_MAX_NUM; ++i)
	{
		std::vector<CItemDropInfo> & v = g_vec_pkCommonDropItem[i];
		std::sort(v.begin(), v.end());

		std::vector<CItemDropInfo>::iterator it = v.begin();

		sys_log(1, "CommonItemDrop rank %d", i);

		while (it != v.end())
		{
			const CItemDropInfo & c = *(it++);
			sys_log(1, "CommonItemDrop %d %d %d %u", c.m_iLevelStart, c.m_iLevelEnd, c.m_iPercent, c.m_dwVnum);
		}
	}

	return true;
}

bool ITEM_MANAGER::ReadSpecialDropItemFile(const char * c_pszFileName)
{
	CTextFileLoader loader;

	if (!loader.Load(c_pszFileName))
		return false;

	std::string stName;

	for (uint32_t i = 0; i < loader.GetChildNodeCount(); ++i)
	{
		loader.SetChildNode(i);

		loader.GetCurrentNodeName(&stName);

		int32_t iVnum;

		if (!loader.GetTokenInteger("vnum", &iVnum))
		{
			sys_err("ReadSpecialDropItemFile : Syntax error %s : no vnum, node %s", c_pszFileName, stName.c_str());
			loader.SetParentNode();
			return false;
		}

		sys_log(0,"DROP_ITEM_GROUP %s %d", stName.c_str(), iVnum);

		TTokenVector * pTok;

		//
		std::string stType;
		int32_t type = CSpecialItemGroup::NORMAL;
		if (loader.GetTokenString("type", &stType))
		{
			stl_lowers(stType);
			if (stType == "pct")
			{
				type = CSpecialItemGroup::PCT;
			}
			else if (stType == "quest")
			{
				type = CSpecialItemGroup::QUEST;
				quest::CQuestManager::Instance().RegisterNPCVnum(iVnum);
			}
			else if (stType == "special")
			{
				type = CSpecialItemGroup::SPECIAL;
			}
		}

		if ("attr" == stType)
		{
			CSpecialAttrGroup * pkGroup = M2_NEW CSpecialAttrGroup(iVnum);
			for (int32_t k = 1; k < 1024; ++k)
			{
				char buf[4];
				snprintf(buf, sizeof(buf), "%d", k);

				if (loader.GetTokenVector(buf, &pTok))
				{
					uint32_t apply_type = 0;
					int32_t	apply_value = 0;
					str_to_number(apply_type, pTok->at(0).c_str());
					if (0 == apply_type)
					{
						apply_type = FN_get_apply_type(pTok->at(0).c_str());
						if (0 == apply_type)
						{
							sys_err ("Invalid APPLY_TYPE %s in Special Item Group Vnum %d", pTok->at(0).c_str(), iVnum);
							return false;
						}
					}
					str_to_number(apply_value, pTok->at(1).c_str());
					if (apply_type > MAX_APPLY_NUM)
					{
						sys_err ("Invalid APPLY_TYPE %u in Special Item Group Vnum %d", apply_type, iVnum);
						M2_DELETE(pkGroup);
						return false;
					}
					pkGroup->m_vecAttrs.push_back(CSpecialAttrGroup::CSpecialAttrInfo(apply_type, apply_value));
				}
				else
				{
					break;
				}
			}
			if (loader.GetTokenVector("effect", &pTok))
			{
				pkGroup->m_stEffectFileName = pTok->at(0);
			}
			loader.SetParentNode();
			m_map_pkSpecialAttrGroup.insert(std::make_pair(iVnum, pkGroup));
		}
		else
		{
			CSpecialItemGroup * pkGroup = M2_NEW CSpecialItemGroup(iVnum, type);
			for (int32_t k = 1; k < 1024; ++k)
			{
				char buf[4];
				snprintf(buf, sizeof(buf), "%d", k);

				if (loader.GetTokenVector(buf, &pTok))
				{
					const std::string& name = pTok->at(0);
					uint32_t dwVnum = 0;

					if (!GetVnumByOriginalName(name.c_str(), dwVnum))
					{
						if (name == "gold")
						{
							dwVnum = CSpecialItemGroup::GOLD;
						}
						if (name == "경험치" || name == "exp")
						{
							dwVnum = CSpecialItemGroup::EXP;
						}
						else if (name == "mob")
						{
							dwVnum = CSpecialItemGroup::MOB;
						}
						else if (name == "slow")
						{
							dwVnum = CSpecialItemGroup::SLOW;
						}
						else if (name == "drain_hp")
						{
							dwVnum = CSpecialItemGroup::DRAIN_HP;
						}
						else if (name == "poison")
						{
							dwVnum = CSpecialItemGroup::POISON;
						}
#ifdef ENABLE_WOLFMAN_CHARACTER
						else if (name == "bleeding")
						{
							dwVnum = CSpecialItemGroup::BLEEDING;
						}
#endif
						else if (name == "group")
						{
							dwVnum = CSpecialItemGroup::MOB_GROUP;
						}
						else
						{
							str_to_number(dwVnum, name.c_str());
							if (!ITEM_MANAGER::Instance().GetTable(dwVnum))
							{
								sys_err("ReadSpecialDropItemFile : there is no item %s : node %s", name.c_str(), stName.c_str());
								M2_DELETE(pkGroup);

								return false;
							}
						}
					}

					int32_t iCount = 0;
					str_to_number(iCount, pTok->at(1).c_str());
					int32_t iProb = 0;
					str_to_number(iProb, pTok->at(2).c_str());

					int32_t iRarePct = 0;
					if (pTok->size() > 3)
					{
						str_to_number(iRarePct, pTok->at(3).c_str());
					}

					sys_log(0,"        name %s count %d prob %d rare %d", name.c_str(), iCount, iProb, iRarePct);
					pkGroup->AddItem(dwVnum, iCount, iProb, iRarePct);

					// CHECK_UNIQUE_GROUP
					if (iVnum < 30000)
					{
						m_ItemToSpecialGroup[dwVnum] = iVnum;
					}
					// END_OF_CHECK_UNIQUE_GROUP

					continue;
				}

				break;
			}
			loader.SetParentNode();
			if (CSpecialItemGroup::QUEST == type)
			{
				m_map_pkQuestItemGroup.insert(std::make_pair(iVnum, pkGroup));
			}
			else
			{
				m_map_pkSpecialItemGroup.insert(std::make_pair(iVnum, pkGroup));
			}
		}
	}

	return true;
}


bool ITEM_MANAGER::ConvSpecialDropItemFile()
{
	char szSpecialItemGroupFileName[256];
	snprintf(szSpecialItemGroupFileName, sizeof(szSpecialItemGroupFileName), "%s/special_item_group.txt", LocaleService_GetBasePath().c_str());

	auto fp = msl::file_ptr("special_item_group_vnum.txt", "w");
	if (!fp)
	{
		sys_err("could not open file (%s)", "special_item_group_vnum.txt");
		return false;
	}

	CTextFileLoader loader;
	if (!loader.Load(szSpecialItemGroupFileName))
		return false;

	std::string stName;

	for (uint32_t i = 0; i < loader.GetChildNodeCount(); ++i)
	{
		loader.SetChildNode(i);

		loader.GetCurrentNodeName(&stName);

		int32_t iVnum;

		if (!loader.GetTokenInteger("vnum", &iVnum))
		{
			sys_err("ConvSpecialDropItemFile : Syntax error %s : no vnum, node %s", szSpecialItemGroupFileName, stName.c_str());
			loader.SetParentNode();
			return false;
		}

		std::string str;
		int32_t type = 0;
		if (loader.GetTokenString("type", &str))
		{
			stl_lowers(str);
			if (str == "pct")
				type = 1;
			else if (str == "quest")
				type = 2;
			else if (str == "special")
				type = 3;
			else if (str == "attr")
				type = 4;
		}

		TTokenVector * pTok;

		fprintf(fp.get(), "Group	%s\n", stName.c_str());
		fprintf(fp.get(), "{\n");
		fprintf(fp.get(), "	Vnum	%i\n", iVnum);
		if (type==1)
			fprintf(fp.get(), "	Type	Pct\n");
		else if (type==2)
			fprintf(fp.get(), "	Type	Quest\n");
		else if (type==3)
			fprintf(fp.get(), "	Type	special\n");
		else if (type==4)
			fprintf(fp.get(), "	Type	ATTR\n");

		for (int32_t k = 1; k < 1024; ++k)
		{
			char buf[4];
			snprintf(buf, sizeof(buf), "%d", k);

			if (loader.GetTokenVector(buf, &pTok))
			{
				std::string name = pTok->at(0);
				uint32_t dwVnum = 0;

				if (!GetVnumByOriginalName(name.c_str(), dwVnum))
				{
					if (name == "gold" ||
						name == "exp" ||
						name == "mob" ||
						name == "slow" ||
						name == "drain_hp" ||
						name == "poison" ||
#ifdef ENABLE_WOLFMAN_CHARACTER
						name == "bleeding" ||
#endif
						name == "group")
					{
						dwVnum = 0;
					}
					else if (name == "경험치")
					{
						dwVnum = 0;
						name = "exp";
					}
					else
					{
						str_to_number(dwVnum, name.c_str());
						if (!ITEM_MANAGER::Instance().GetTable(dwVnum) && type!=4)
						{
							sys_err("ReadSpecialDropItemFile : there is no item %s : node %s", name.c_str(), stName.c_str());
							return false;
						}
					}
				}

				int32_t iCount = 0;
				str_to_number(iCount, pTok->at(1).c_str());
				int32_t iProb = 0;

				if (pTok->size() > 2)
					str_to_number(iProb, pTok->at(2).c_str());

				int32_t iRarePct = 0;
				if (pTok->size() > 3)
					str_to_number(iRarePct, pTok->at(3).c_str());

				if (type==4)
					fprintf(fp.get(), "	%d	%u	%d\n", k, dwVnum, iCount);
				else
				{
					
					if (0 == dwVnum)
						fprintf(fp.get(), "	%d	%s	%d	%d\n", k, name.c_str(), iCount, iProb);
					else
						fprintf(fp.get(), "	%d	%u	%d	%d\n", k, dwVnum, iCount, iProb);
				}

				continue;
			}

			break;
		}
		std::string effect;
		if (loader.GetTokenString("effect", &effect))
			fprintf(fp.get(), "	effect	\"%s\"\n", effect.c_str());
		fprintf(fp.get(), "}\n");
		fprintf(fp.get(), "\n");

		loader.SetParentNode();
	}

	return true;
}

bool ITEM_MANAGER::ReadEtcDropItemFile(const char * c_pszFileName)
{
	auto fp = msl::file_ptr(c_pszFileName, "r");
	if (!fp)
	{
		sys_err("Cannot open %s", c_pszFileName);
		return false;
	}

	int32_t lines = 0;

	char buf[512];
	while (fgets(buf, 512, fp.get()))
	{
		++lines;

		if (!*buf || *buf == '\n')
			continue;

		char szItemName[256];
		float fProb = 0.0f;

		strlcpy(szItemName, buf, sizeof(szItemName));
		char * cpTab = strrchr(szItemName, '\t');

		if (!cpTab)
			continue;

		*cpTab = '\0';
		fProb = atof(cpTab + 1);

		if (!*szItemName || fProb == 0.0f)
			continue;

		uint32_t dwItemVnum;

		if (!ITEM_MANAGER::Instance().GetVnumByOriginalName(szItemName, dwItemVnum))
		{
			sys_err("No such an item (name: %s)", szItemName);
			return false;
		}

		m_map_dwEtcItemDropProb[dwItemVnum] = (uint32_t) (fProb * 10000.0f);
		sys_log(0, "ETC_DROP_ITEM: %s prob %f", szItemName, fProb);
	}

	return true;
}

bool ITEM_MANAGER::ReadMonsterDropItemGroup(const char * c_pszFileName)
{
	CTextFileLoader loader;

	if (!loader.Load(c_pszFileName))
		return false;

	for (uint32_t i = 0; i < loader.GetChildNodeCount(); ++i)
	{
		std::string stName("");

		loader.GetCurrentNodeName(&stName);

		if (strncmp (stName.c_str(), "kr_", 3) == 0)
			continue;

		loader.SetChildNode(i);

		int32_t iMobVnum = 0;
		int32_t iKillDrop = 0;
		int32_t iLevelLimit = 0;

		std::string strType("");

		if (!loader.GetTokenString("type", &strType))
		{
			sys_err("ReadMonsterDropItemGroup : Syntax error %s : no type (kill|drop), node %s", c_pszFileName, stName.c_str());
			loader.SetParentNode();
			return false;
		}

		if (!loader.GetTokenInteger("mob", &iMobVnum))
		{
			sys_err("ReadMonsterDropItemGroup : Syntax error %s : no mob vnum, node %s", c_pszFileName, stName.c_str());
			loader.SetParentNode();
			return false;
		}

		if (strType == "kill")
		{
			if (!loader.GetTokenInteger("kill_drop", &iKillDrop))
			{
				sys_err("ReadMonsterDropItemGroup : Syntax error %s : no kill drop count, node %s", c_pszFileName, stName.c_str());
				loader.SetParentNode();
				return false;
			}
		}
		else
		{
			iKillDrop = 1;
		}

		if ( strType == "limit" )
		{
			if ( !loader.GetTokenInteger("level_limit", &iLevelLimit) )
			{
				sys_err("ReadmonsterDropItemGroup : Syntax error %s : no level_limit, node %s", c_pszFileName, stName.c_str());
				loader.SetParentNode();
				return false;
			}
		}
		else
		{
			iLevelLimit = 0;
		}

		sys_log(0,"MOB_ITEM_GROUP %s [%s] %d %d", stName.c_str(), strType.c_str(), iMobVnum, iKillDrop);

		if (iKillDrop == 0)
		{
			loader.SetParentNode();
			continue;
		}

		TTokenVector* pTok = nullptr;

		if (strType == "kill")
		{
			CMobItemGroup * pkGroup = M2_NEW CMobItemGroup(iMobVnum, iKillDrop, stName);

			for (int32_t k = 1; k < 256; ++k)
			{
				char buf[4];
				snprintf(buf, sizeof(buf), "%d", k);

				if (loader.GetTokenVector(buf, &pTok))
				{
					//sys_log(1, "               %s %s", pTok->at(0).c_str(), pTok->at(1).c_str());
					std::string& name = pTok->at(0);
					uint32_t dwVnum = 0;

					if (is_positive_number(name))
						str_to_number(dwVnum, name.c_str()); //Assume it's an vnum first

					if (!dwVnum)
					{
						GetVnumByOriginalName(name.c_str(), dwVnum);
						if (!ITEM_MANAGER::Instance().GetTable(dwVnum))
						{
							sys_err("ReadMonsterDropItemGroup : there is no item %s : node %s : vnum %d", name.c_str(), stName.c_str(), dwVnum);
							return false;
						}
					}

					int32_t iCount = 0;
					str_to_number(iCount, pTok->at(1).c_str());

					if (iCount<1)
					{
						sys_err("ReadMonsterDropItemGroup : there is no count for item %s : node %s : vnum %d, count %d", name.c_str(), stName.c_str(), dwVnum, iCount);
						return false;
					}

					int32_t iPartPct = 0;
					str_to_number(iPartPct, pTok->at(2).c_str());

					if (iPartPct == 0)
					{
						sys_err("ReadMonsterDropItemGroup : there is no drop percent for item %s : node %s : vnum %d, count %d, pct %d", name.c_str(), stName.c_str(), iPartPct);
						return false;
					}

					int32_t iRarePct = 0;
					str_to_number(iRarePct, pTok->at(3).c_str());
					iRarePct = MINMAX(0, iRarePct, 100);

					sys_log(0,"        %s count %d rare %d", name.c_str(), iCount, iRarePct);
					pkGroup->AddItem(dwVnum, iCount, iPartPct, iRarePct);
					continue;
				}

				break;
			}
			m_map_pkMobItemGroup.emplace(iMobVnum, pkGroup);

		}
		else if (strType == "drop")
		{
			CDropItemGroup* pkGroup;
			bool bNew = true;
			auto it = m_map_pkDropItemGroup.find (iMobVnum);
			if (it == m_map_pkDropItemGroup.end())
			{
				pkGroup = M2_NEW CDropItemGroup(0, iMobVnum, stName);
			}
			else
			{
				bNew = false;
				pkGroup = it->second;
			}

			for (int32_t k = 1; k < 256; ++k)
			{
				char buf[4];
				snprintf(buf, sizeof(buf), "%d", k);

				if (loader.GetTokenVector(buf, &pTok))
				{
					std::string& name = pTok->at(0);
					uint32_t dwVnum = 0;
					if (is_positive_number(name))
						str_to_number(dwVnum, name.c_str()); //Assume it's an vnum first

					if (!dwVnum)
					{
						GetVnumByOriginalName(name.c_str(), dwVnum);
						if (!ITEM_MANAGER::Instance().GetTable(dwVnum))
						{
							sys_err("ReadDropItemGroup : there is no item %s : node %s", name.c_str(), stName.c_str());
							M2_DELETE(pkGroup);

							return false;
						}
					}

					int32_t iCount = 0;
					str_to_number(iCount, pTok->at(1).c_str());

					if (iCount < 1)
					{
						sys_err("ReadMonsterDropItemGroup : there is no count for item %s : node %s", name.c_str(), stName.c_str());
						M2_DELETE(pkGroup);

						return false;
					}

					float fPercent = atof(pTok->at(2).c_str());

					uint32_t dwPct = (uint32_t)(10000.0f * fPercent);

					sys_log(0,"        name %s pct %d count %d", name.c_str(), dwPct, iCount);
					pkGroup->AddItem(dwVnum, dwPct, iCount);

					continue;
				}

				break;
			}
			if (bNew)
				m_map_pkDropItemGroup.emplace(iMobVnum, pkGroup);

		}
		else if ( strType == "limit" )
		{
			CLevelItemGroup* pkLevelItemGroup = M2_NEW CLevelItemGroup(iLevelLimit);

			for ( int32_t k=1; k < 256; k++ )
			{
				char buf[4];
				snprintf(buf, sizeof(buf), "%d", k);

				if ( loader.GetTokenVector(buf, &pTok) )
				{
					std::string& name = pTok->at(0);
					uint32_t dwItemVnum = 0;
					if (is_positive_number(name))
						str_to_number(dwItemVnum, name.c_str()); //Assume it's an vnum first

					if (!dwItemVnum)
					{
						GetVnumByOriginalName(name.c_str(), dwItemVnum);
						if ( !ITEM_MANAGER::Instance().GetTable(dwItemVnum) )
						{
							sys_err("ReadDropItemGroup : there is no item %s : node %s", name.c_str(), stName.c_str());
							M2_DELETE(pkLevelItemGroup);
							return false;
						}
					}

					int32_t iCount = 0;
					str_to_number(iCount, pTok->at(1).c_str());

					if (iCount < 1)
					{
						sys_err("ReadMonsterDropItemGroup : there is no count for item %s : node %s", name.c_str(), stName.c_str());
						M2_DELETE(pkLevelItemGroup);
						return false;
					}

					float fPct = atof(pTok->at(2).c_str());
					uint32_t dwPct = (uint32_t)(10000.0f * fPct);

					sys_log(0,"        name %s pct %d count %d", name.c_str(), dwPct, iCount);
					pkLevelItemGroup->AddItem(dwItemVnum, dwPct, iCount);

					continue;
				}

				break;
			}

			m_map_pkLevelItemGroup.emplace(iMobVnum, pkLevelItemGroup);
		}
		else if (strType == "thiefgloves")
		{
			CBuyerThiefGlovesItemGroup* pkGroup = M2_NEW CBuyerThiefGlovesItemGroup(0, iMobVnum, stName);

			for (int32_t k = 1; k < 256; ++k)
			{
				char buf[4];
				snprintf(buf, sizeof(buf), "%d", k);

				if (loader.GetTokenVector(buf, &pTok))
				{
					std::string& name = pTok->at(0);
					uint32_t dwVnum = 0;
					if (is_positive_number(name))
						str_to_number(dwVnum, name.c_str()); //Assume it's an vnum first

					if (!dwVnum)
					{
						GetVnumByOriginalName(name.c_str(), dwVnum);
						if (!ITEM_MANAGER::Instance().GetTable(dwVnum))
						{
							sys_err("ReadDropItemGroup : there is no item %s : node %s", name.c_str(), stName.c_str());
							M2_DELETE(pkGroup);

							return false;
						}
					}

					int32_t iCount = 0;
					str_to_number(iCount, pTok->at(1).c_str());

					if (iCount < 1)
					{
						sys_err("ReadMonsterDropItemGroup : there is no count for item %s : node %s", name.c_str(), stName.c_str());
						M2_DELETE(pkGroup);

						return false;
					}

					float fPercent = atof(pTok->at(2).c_str());

					uint32_t dwPct = (uint32_t)(10000.0f * fPercent);

					sys_log(0,"        name %s pct %d count %d", name.c_str(), dwPct, iCount);
					pkGroup->AddItem(dwVnum, dwPct, iCount);

					continue;
				}

				break;
			}

			m_map_pkGloveItemGroup.emplace(iMobVnum, pkGroup);
		}
		else
		{
			sys_err("ReadMonsterDropItemGroup : Syntax error %s : invalid type %s (kill|drop), node %s", c_pszFileName, strType.c_str(), stName.c_str());
			loader.SetParentNode();
			return false;
		}

		loader.SetParentNode();
	}

	return true;
}

bool ITEM_MANAGER::ReadDropItemGroup(const char * c_pszFileName)
{
	CTextFileLoader loader;

	if (!loader.Load(c_pszFileName))
		return false;

	std::string stName;

	for (uint32_t i = 0; i < loader.GetChildNodeCount(); ++i)
	{
		loader.SetChildNode(i);

		loader.GetCurrentNodeName(&stName);

		int32_t iVnum;
		int32_t iMobVnum;

		if (!loader.GetTokenInteger("vnum", &iVnum))
		{
			sys_err("ReadDropItemGroup : Syntax error %s : no vnum, node %s", c_pszFileName, stName.c_str());
			loader.SetParentNode();
			return false;
		}

		if (!loader.GetTokenInteger("mob", &iMobVnum))
		{
			sys_err("ReadDropItemGroup : Syntax error %s : no mob vnum, node %s", c_pszFileName, stName.c_str());
			loader.SetParentNode();
			return false;
		}

		sys_log(0,"DROP_ITEM_GROUP %s %d", stName.c_str(), iMobVnum);

		TTokenVector * pTok;

		auto it = m_map_pkDropItemGroup.find(iMobVnum);

		CDropItemGroup* pkGroup;

		if (it == m_map_pkDropItemGroup.end())
			pkGroup = M2_NEW CDropItemGroup(iVnum, iMobVnum, stName);
		else
			pkGroup = it->second;

		for (int32_t k = 1; k < 256; ++k)
		{
			char buf[4];
			snprintf(buf, sizeof(buf), "%d", k);

			if (loader.GetTokenVector(buf, &pTok))
			{
				std::string& name = pTok->at(0);
				uint32_t dwVnum = 0;

				if (!dwVnum)
				{
					GetVnumByOriginalName(name.c_str(), dwVnum);
					if (!ITEM_MANAGER::Instance().GetTable(dwVnum))
					{
						sys_err("ReadDropItemGroup : there is no item %s : node %s", name.c_str(), stName.c_str());

						if (it == m_map_pkDropItemGroup.end())
							M2_DELETE(pkGroup);

						return false;
					}
				}

				float fPercent = atof(pTok->at(1).c_str());

				uint32_t dwPct = (uint32_t)(10000.0f * fPercent);

				int32_t iCount = 1;
				if (pTok->size() > 2)
					str_to_number(iCount, pTok->at(2).c_str());

				if (iCount < 1)
				{
					sys_err("ReadDropItemGroup : there is no count for item %s : node %s", name.c_str(), stName.c_str());

					if (it == m_map_pkDropItemGroup.end())
						M2_DELETE(pkGroup);

					return false;
				}

				sys_log(0,"        %s %d %d", name.c_str(), dwPct, iCount);
				pkGroup->AddItem(dwVnum, dwPct, iCount);
				continue;
			}

			break;
		}

		if (it == m_map_pkDropItemGroup.end())
			m_map_pkDropItemGroup.emplace(iMobVnum, pkGroup);

		loader.SetParentNode();
	}

	return true;
}

bool ITEM_MANAGER::ReadItemVnumMaskTable(const char * c_pszFileName)
{
	auto fp = msl::file_ptr(c_pszFileName, "r");
	if (!fp)
		return false;

	int32_t ori_vnum, new_vnum;
	while (fscanf(fp.get(), "%u %u", &ori_vnum, &new_vnum) != EOF)
	{
		m_map_new_to_ori.emplace(new_vnum, ori_vnum);
	}
	return true;
}
