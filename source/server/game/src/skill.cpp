#include "stdafx.h"
#include "../../common/stl.h"

#include "constants.h"
#include "skill.h"
#include "char.h"

void CSkillProto::SetPointVar(const std::string& strName, double dVar)
{
	kPointPoly.SetVar(strName, dVar);
	kPointPoly2.SetVar(strName, dVar);
	kPointPoly3.SetVar(strName, dVar);
	kMasterBonusPoly.SetVar(strName, dVar);
}

void CSkillProto::SetDurationVar(const std::string& strName, double dVar)
{
	kDurationPoly.SetVar(strName, dVar);
	kDurationPoly2.SetVar(strName, dVar);
	kDurationPoly3.SetVar(strName, dVar);
}

void CSkillProto::SetSPCostVar(const std::string& strName, double dVar)
{
	kSPCostPoly.SetVar(strName, dVar);
	kGrandMasterAddSPCostPoly.SetVar(strName, dVar);
}

CSkillManager::CSkillManager()
{
}

CSkillManager::~CSkillManager()
{
	auto it = m_map_pkSkillProto.begin();
	for ( ; it != m_map_pkSkillProto.end(); ++it) {
		M2_DELETE(it->second);
	}
}

struct SPointOnType
{
	const char * c_pszName;
	int32_t		 iPointOn;
} kPointOnTypes[] = {
	{ "NONE",		POINT_NONE		},
	{ "MAX_HP",		POINT_MAX_HP		},
	{ "MAX_SP",		POINT_MAX_SP		},
	{ "HP_REGEN",	POINT_HP_REGEN		},
	{ "SP_REGEN",	POINT_SP_REGEN		},
	{ "BLOCK",		POINT_BLOCK		},
	{ "HP",		POINT_HP		},
	{ "SP",		POINT_SP		},
	{ "ATT_GRADE",	POINT_ATT_GRADE_BONUS	},
	{ "DEF_GRADE",	POINT_DEF_GRADE_BONUS	},
	{ "MAGIC_ATT_GRADE",POINT_MAGIC_ATT_GRADE_BONUS	},
	{ "MAGIC_DEF_GRADE",POINT_MAGIC_DEF_GRADE_BONUS	},
	{ "BOW_DISTANCE",	POINT_BOW_DISTANCE	},
	{ "MOV_SPEED",	POINT_MOV_SPEED		},
	{ "ATT_SPEED",	POINT_ATT_SPEED		},
	{ "POISON_PCT",	POINT_POISON_PCT	},
	{ "RESIST_RANGE",   POINT_RESIST_BOW	},
	//{ "RESIST_MELEE",	POINT_RESIST_MELEE	},
	{ "CASTING_SPEED",	POINT_CASTING_SPEED	},
	{ "REFLECT_MELEE",	POINT_REFLECT_MELEE	},
	{ "ATT_BONUS",	POINT_ATT_BONUS		},
	{ "DEF_BONUS",	POINT_DEF_BONUS		},
	{ "RESIST_NORMAL",	POINT_RESIST_NORMAL_DAMAGE },
	{ "DODGE",		POINT_DODGE		},
	{ "KILL_HP_RECOVER",POINT_KILL_HP_RECOVERY	},
	{ "KILL_SP_RECOVER",POINT_KILL_SP_RECOVER	},
	{ "HIT_HP_RECOVER",	POINT_HIT_HP_RECOVERY	},
	{ "HIT_SP_RECOVER",	POINT_HIT_SP_RECOVERY	},
	{ "CRITICAL",	POINT_CRITICAL_PCT	},
	{ "MANASHIELD",	POINT_MANASHIELD	},
	{ "SKILL_DAMAGE_BONUS", POINT_SKILL_DAMAGE_BONUS	},
	{ "NORMAL_HIT_DAMAGE_BONUS", POINT_NORMAL_HIT_DAMAGE_BONUS	},


	{ "HT", POINT_HT },
	{ "IQ", POINT_IQ },
	{ "ST", POINT_ST },
	{ "DX", POINT_DX },
	{ "STUN_PCT", POINT_STUN_PCT },
	{ "SLOW_PCT", POINT_SLOW_PCT },
	{ "PENETRATE_PCT", POINT_PENETRATE_PCT },
	{ "ATTBONUS_HUMAN", POINT_ATTBONUS_HUMAN },
	{ "STEAL_HP", POINT_STEAL_HP },
	{ "STEAL_SP", POINT_STEAL_SP },
	{ "MANA_BURN_PCT", POINT_MANA_BURN_PCT },
	{ "DAMAGE_SP_RECOVER", POINT_DAMAGE_SP_RECOVER },
	{ "RESIST_SWORD", POINT_RESIST_SWORD },
	{ "RESIST_TWOHAND", POINT_RESIST_TWOHAND },
	{ "RESIST_DAGGER", POINT_RESIST_DAGGER },
	{ "RESIST_BELL", POINT_RESIST_BELL },
	{ "RESIST_FAN", POINT_RESIST_FAN },
	{ "RESIST_BOW", POINT_RESIST_BOW },
	{ "RESIST_FIRE", POINT_RESIST_FIRE },
	{ "RESIST_ELEC", POINT_RESIST_ELEC },
	{ "RESIST_MAGIC", POINT_RESIST_MAGIC },
	{ "RESIST_WIND", POINT_RESIST_WIND },
	{ "REFLECT_CURSE", POINT_REFLECT_CURSE },
	{ "POISON_REDUCE", POINT_POISON_REDUCE },
	{ "EXP_DOUBLE_BONUS", POINT_EXP_DOUBLE_BONUS },
	{ "GOLD_DOUBLE_BONUS", POINT_GOLD_DOUBLE_BONUS },
	{ "ITEM_DROP_BONUS", POINT_ITEM_DROP_BONUS },
	{ "POTION_BONUS", POINT_POTION_BONUS },
	{ "IMMUNE_STUN", POINT_IMMUNE_STUN },
	{ "IMMUNE_SLOW", POINT_IMMUNE_SLOW },
	{ "IMMUNE_FALL", POINT_IMMUNE_FALL },
	{ "CURSE_PCT", POINT_CURSE_PCT },
	{ "STA", POINT_MAX_STAMINA },
	{ "ATTBONUS_WARRIOR", POINT_ATTBONUS_WARRIOR },
	{ "ATTBONUS_ASSASSIN", POINT_ATTBONUS_ASSASSIN },
	{ "ATTBONUS_SURA", POINT_ATTBONUS_SURA },
	{ "ATTBONUS_SHAMAN", POINT_ATTBONUS_SHAMAN },
	{ "ATTBONUS_MONSTER", POINT_ATTBONUS_MONSTER },
	{ "MAX_HP_PCT", POINT_MAX_HP_PCT },
	{ "MAX_SP_PCT", POINT_MAX_SP_PCT },
	{ "SKILL_DEFEND_BONUS", POINT_SKILL_DEFEND_BONUS },
	{ "NORMAL_HIT_DEFEND_BONUS", POINT_NORMAL_HIT_DEFEND_BONUS },
	{ "RESIST_WARRIOR", POINT_RESIST_WARRIOR },
	{ "RESIST_ASSASSIN", POINT_RESIST_ASSASSIN },
	{ "RESIST_SURA", POINT_RESIST_SURA },
	{ "RESIST_SHAMAN", POINT_RESIST_SHAMAN },
	{ "ENERGY", POINT_ENERGY },
	{ "RESIST_CRITICAL", POINT_RESIST_CRITICAL },
	{ "RESIST_PENETRATE", POINT_RESIST_PENETRATE },

#ifdef ENABLE_WOLFMAN_CHARACTER
	{ "BLEEDING_PCT",	POINT_BLEEDING_PCT	},
#endif
	{ "\n",		POINT_NONE		},
};

int32_t FindPointType(const char * c_sz)
{
	for (int32_t i = 0; *kPointOnTypes[i].c_pszName != '\n'; ++i)
	{
		if (!strcasecmp(c_sz, kPointOnTypes[i].c_pszName))
			return kPointOnTypes[i].iPointOn;
	}
	return -1;
}

bool CSkillManager::Initialize(TSkillTable * pTab, int32_t iSize)
{
	char buf[1024];
	std::map<uint32_t, CSkillProto *> map_pkSkillProto;

	TSkillTable * t = pTab;
	bool bError = false;

	for (int32_t i = 0; i < iSize; ++i, ++t)
	{
		CSkillProto * pkProto = M2_NEW CSkillProto;

		pkProto->dwVnum = t->dwVnum;
		strlcpy(pkProto->szName, t->szName, sizeof(pkProto->szName));
		pkProto->dwType = t->bType;
		pkProto->bMaxLevel = t->bMaxLevel;
		pkProto->dwFlag = t->dwFlag;
		pkProto->dwAffectFlag = t->dwAffectFlag;
		pkProto->dwAffectFlag2 = t->dwAffectFlag2;

		pkProto->bLevelStep = t->bLevelStep;
		pkProto->bLevelLimit = t->bLevelLimit;
		pkProto->iSplashRange = t->dwSplashRange;
		pkProto->dwTargetRange = t->dwTargetRange;
		pkProto->preSkillVnum = t->preSkillVnum;
		pkProto->preSkillLevel = t->preSkillLevel;

		pkProto->lMaxHit = t->lMaxHit;

		pkProto->bSkillAttrType = t->bSkillAttrType;

		int32_t iIdx = FindPointType(t->szPointOn);

		if (iIdx < 0)
		{
			snprintf(buf, sizeof(buf), "SkillManager::Initialize : cannot find point type on skill: %s szPointOn: %s", t->szName, t->szPointOn);
			sys_err("%s", buf);
			SendLog(buf);
			bError = true;
			M2_DELETE(pkProto);
			continue;
		}

		pkProto->bPointOn = iIdx;

		int32_t iIdx2 = FindPointType(t->szPointOn2);

		if (iIdx2 < 0)
		{
			snprintf(buf, sizeof(buf), "SkillManager::Initialize : cannot find point type on skill: %s szPointOn2: %s", t->szName, t->szPointOn2);
			sys_err("%s", buf);
			SendLog(buf);
			bError = true;
			M2_DELETE(pkProto);
			continue;
		}

		pkProto->bPointOn2 = iIdx2;

		int32_t iIdx3 = FindPointType(t->szPointOn3);

		if (iIdx3 < 0)
		{
			if (t->szPointOn3[0] == 0)
			{
				iIdx3 = POINT_NONE;
			}
			else
			{
				snprintf(buf, sizeof(buf), "SkillManager::Initialize : cannot find point type on skill: %s szPointOn3: %s", t->szName, t->szPointOn3);
				sys_err("%s", buf);
				SendLog(buf);
				bError = true;
				M2_DELETE(pkProto);
				continue;
			}
		}

		pkProto->bPointOn3 = iIdx3;

		if (!pkProto->kSplashAroundDamageAdjustPoly.Analyze(t->szSplashAroundDamageAdjustPoly))
		{
			snprintf(buf, sizeof(buf), "SkillManager::Initialize : syntax error skill: %s szSplashAroundDamageAdjustPoly: %s", t->szName, t->szSplashAroundDamageAdjustPoly);
			sys_err("%s", buf);
			SendLog(buf);
			bError = true;
			M2_DELETE(pkProto);
			continue;
		}

		if (!pkProto->kPointPoly.Analyze(t->szPointPoly))
		{
			snprintf(buf, sizeof(buf), "SkillManager::Initialize : syntax error skill: %s szPointPoly: %s", t->szName, t->szPointPoly);
			sys_err("%s", buf);
			SendLog(buf);
			bError = true;
			M2_DELETE(pkProto);
			continue;
		}

		if (!pkProto->kPointPoly2.Analyze(t->szPointPoly2))
		{
			snprintf(buf, sizeof(buf), "SkillManager::Initialize : syntax error skill: %s szPointPoly2: %s", t->szName, t->szPointPoly2);
			sys_err("%s", buf);
			SendLog(buf);
			bError = true;
			M2_DELETE(pkProto);
			continue;
		}

		if (!pkProto->kPointPoly3.Analyze(t->szPointPoly3))
		{
			snprintf(buf, sizeof(buf), "SkillManager::Initialize : syntax error skill: %s szPointPoly3: %s", t->szName, t->szPointPoly3);
			sys_err("%s", buf);
			SendLog(buf);
			bError = true;
			M2_DELETE(pkProto);
			continue;
		}

		if (!pkProto->kSPCostPoly.Analyze(t->szSPCostPoly))
		{
			snprintf(buf, sizeof(buf), "SkillManager::Initialize : syntax error skill: %s szSPCostPoly: %s", t->szName, t->szSPCostPoly);
			sys_err("%s", buf);
			SendLog(buf);
			bError = true;
			M2_DELETE(pkProto);
			continue;
		}

		if (!pkProto->kGrandMasterAddSPCostPoly.Analyze(t->szGrandMasterAddSPCostPoly))
		{
			snprintf(buf, sizeof(buf), "SkillManager::Initialize : syntax error skill: %s szGrandMasterAddSPCostPoly: %s", t->szName, t->szGrandMasterAddSPCostPoly);
			sys_err("%s", buf);
			SendLog(buf);
			bError = true;
			M2_DELETE(pkProto);
			continue;
		}

		if (!pkProto->kDurationPoly.Analyze(t->szDurationPoly))
		{
			snprintf(buf, sizeof(buf), "SkillManager::Initialize : syntax error skill: %s szDurationPoly: %s", t->szName, t->szDurationPoly);
			sys_err("%s", buf);
			SendLog(buf);
			bError = true;
			M2_DELETE(pkProto);
			continue;
		}

		if (!pkProto->kDurationPoly2.Analyze(t->szDurationPoly2))
		{
			snprintf(buf, sizeof(buf), "SkillManager::Initialize : syntax error skill: %s szDurationPoly2: %s", t->szName, t->szDurationPoly2);
			sys_err("%s", buf);
			SendLog(buf);
			bError = true;
			M2_DELETE(pkProto);
			continue;
		}

		if (!pkProto->kDurationPoly3.Analyze(t->szDurationPoly3))
		{
			snprintf(buf, sizeof(buf), "SkillManager::Initialize : syntax error skill: %s szDurationPoly3: %s", t->szName, t->szDurationPoly3);
			sys_err("%s", buf);
			SendLog(buf);
			bError = true;
			M2_DELETE(pkProto);
			continue;
		}

		if (!pkProto->kDurationSPCostPoly.Analyze(t->szDurationSPCostPoly))
		{
			snprintf(buf, sizeof(buf), "SkillManager::Initialize : syntax error skill: %s szDurationSPCostPoly: %s", t->szName, t->szDurationSPCostPoly);
			sys_err("%s", buf);
			SendLog(buf);
			bError = true;
			M2_DELETE(pkProto);
			continue;
		}

		if (!pkProto->kCooldownPoly.Analyze(t->szCooldownPoly))
		{
			snprintf(buf, sizeof(buf), "SkillManager::Initialize : syntax error skill: %s szCooldownPoly: %s", t->szName, t->szCooldownPoly);
			sys_err("%s", buf);
			SendLog(buf);
			bError = true;
			M2_DELETE(pkProto);
			continue;
		}

		sys_log(0, "Master %s", t->szMasterBonusPoly);
		if (!pkProto->kMasterBonusPoly.Analyze(t->szMasterBonusPoly))
		{
			snprintf(buf, sizeof(buf), "SkillManager::Initialize : syntax error skill: %s szMasterBonusPoly: %s", t->szName, t->szMasterBonusPoly);
			sys_err("%s", buf);
			SendLog(buf);
			bError = true;
			M2_DELETE(pkProto);
			continue;
		}

		sys_log(0, "#%-3d %-24s type %u flag %u affect %u point_poly: %s", 
				pkProto->dwVnum, pkProto->szName, pkProto->dwType, pkProto->dwFlag, pkProto->dwAffectFlag, t->szPointPoly);

		map_pkSkillProto.insert(std::map<uint32_t, CSkillProto *>::value_type(pkProto->dwVnum, pkProto));
	}

	if (!bError)
	{
		// 기존 테이블의 내용을 지운다.
		auto it = m_map_pkSkillProto.begin();

		while (it != m_map_pkSkillProto.end()) {
			M2_DELETE(it->second);
			++it;
		}

		m_map_pkSkillProto.clear();

		// 새로운 내용을 삽입
		it = map_pkSkillProto.begin();

		while (it != map_pkSkillProto.end())
		{
			m_map_pkSkillProto.insert(std::map<uint32_t, CSkillProto *>::value_type(it->first, it->second));
			++it;
		}

		SendLog("Skill Prototype reloaded!");
	}
	else
		SendLog("There were erros when loading skill table");

	return !bError;
}

CSkillProto * CSkillManager::Get(uint32_t dwVnum)
{
	std::map<uint32_t, CSkillProto *>::iterator it = m_map_pkSkillProto.find(dwVnum);

	if (it == m_map_pkSkillProto.end())
		return nullptr;

	return it->second;
}

CSkillProto * CSkillManager::Get(const char * c_pszSkillName)
{
	std::map<uint32_t, CSkillProto *>::iterator it = m_map_pkSkillProto.begin();

	while (it != m_map_pkSkillProto.end())
	{
		if (!strcasecmp(it->second->szName, c_pszSkillName))
			return it->second;

		it++;
	}

	return nullptr;
}

