#pragma once

enum EApplyTypes
{
	APPLY_NONE,                 // 0
	APPLY_MAX_HP,               // 1
	APPLY_MAX_SP,               // 2
	APPLY_CON,                  // 3
	APPLY_INT,                  // 4
	APPLY_STR,                  // 5
	APPLY_DEX,                  // 6
	APPLY_ATT_SPEED,            // 7
	APPLY_MOV_SPEED,            // 8
	APPLY_CAST_SPEED,           // 9
	APPLY_HP_REGEN,             // 10
	APPLY_SP_REGEN,             // 11
	APPLY_POISON_PCT,           // 12
	APPLY_STUN_PCT,             // 13
	APPLY_SLOW_PCT,             // 14
	APPLY_CRITICAL_PCT,         // 15
	APPLY_PENETRATE_PCT,        // 16
	APPLY_ATTBONUS_HUMAN,       // 17
	APPLY_ATTBONUS_ANIMAL,      // 18
	APPLY_ATTBONUS_ORC,         // 19
	APPLY_ATTBONUS_MILGYO,      // 20
	APPLY_ATTBONUS_UNDEAD,      // 21
	APPLY_ATTBONUS_DEVIL,       // 22
	APPLY_STEAL_HP,             // 23
	APPLY_STEAL_SP,             // 24
	APPLY_MANA_BURN_PCT,        // 25
	APPLY_DAMAGE_SP_RECOVER,    // 26
	APPLY_BLOCK,                // 27
	APPLY_DODGE,                // 28
	APPLY_RESIST_SWORD,         // 29
	APPLY_RESIST_TWOHAND,       // 30
	APPLY_RESIST_DAGGER,        // 31
	APPLY_RESIST_BELL,          // 32
	APPLY_RESIST_FAN,           // 33
	APPLY_RESIST_BOW,           // 34
	APPLY_RESIST_FIRE,          // 35
	APPLY_RESIST_ELEC,          // 36
	APPLY_RESIST_MAGIC,         // 37
	APPLY_RESIST_WIND,          // 38
	APPLY_REFLECT_MELEE,        // 39
	APPLY_REFLECT_CURSE,        // 40
	APPLY_POISON_REDUCE,        // 41
	APPLY_KILL_SP_RECOVER,      // 42
	APPLY_EXP_DOUBLE_BONUS,     // 43
	APPLY_GOLD_DOUBLE_BONUS,    // 44
	APPLY_ITEM_DROP_BONUS,      // 45
	APPLY_POTION_BONUS,         // 46
	APPLY_KILL_HP_RECOVER,      // 47
	APPLY_IMMUNE_STUN,          // 48
	APPLY_IMMUNE_SLOW,          // 49
	APPLY_IMMUNE_FALL,          // 50
	APPLY_SKILL,                // 51
	APPLY_BOW_DISTANCE,         // 52
	APPLY_ATT_GRADE_BONUS,            // 53
	APPLY_DEF_GRADE_BONUS,            // 54
	APPLY_MAGIC_ATT_GRADE,      // 55
	APPLY_MAGIC_DEF_GRADE,      // 56
	APPLY_CURSE_PCT,            // 57
	APPLY_MAX_STAMINA,			// 58
	APPLY_ATT_BONUS_TO_WARRIOR, // 59
	APPLY_ATT_BONUS_TO_ASSASSIN,// 60
	APPLY_ATT_BONUS_TO_SURA,    // 61
	APPLY_ATT_BONUS_TO_SHAMAN,  // 62
	APPLY_ATT_BONUS_TO_MONSTER, // 63
	APPLY_MALL_ATTBONUS,        // 64 ���ݷ� +x%
	APPLY_MALL_DEFBONUS,        // 65 ���� +x%
	APPLY_MALL_EXPBONUS,        // 66 ����ġ +x%
	APPLY_MALL_ITEMBONUS,       // 67 ������ ����� x/10��
	APPLY_MALL_GOLDBONUS,       // 68 �� ����� x/10��
	APPLY_MAX_HP_PCT,           // 69 �ִ� ����� +x%
	APPLY_MAX_SP_PCT,           // 70 �ִ� ���ŷ� +x%
	APPLY_SKILL_DAMAGE_BONUS,   // 71 ��ų ������ * (100+x)%
	APPLY_NORMAL_HIT_DAMAGE_BONUS,      // 72 ��Ÿ ������ * (100+x)%
	APPLY_SKILL_DEFEND_BONUS,   // 73 ��ų ������ ��� * (100-x)%
	APPLY_NORMAL_HIT_DEFEND_BONUS,      // 74 ��Ÿ ������ ��� * (100-x)%
	APPLY_EXTRACT_HP_PCT,		//75
	UNUSED_APPLY_PC_BANG_EXP_BONUS,		//76
	UNUSED_APPLY_PC_BANG_DROP_BONUS,		//77
	APPLY_RESIST_WARRIOR,			//78
	APPLY_RESIST_ASSASSIN,			//79
	APPLY_RESIST_SURA,				//80
	APPLY_RESIST_SHAMAN,			//81
	APPLY_ENERGY,					//82
	APPLY_DEF_GRADE,				// 83 ����. DEF_GRADE_BONUS�� Ŭ�󿡼� �ι�� �������� �ǵ��� ����(...)�� �ִ�.
	APPLY_COSTUME_ATTR_BONUS,		// 84 �ڽ�Ƭ �����ۿ� ���� �Ӽ�ġ ���ʽ�
	APPLY_MAGIC_ATTBONUS_PER,		// 85 ���� ���ݷ� +x%
	APPLY_MELEE_MAGIC_ATTBONUS_PER,			// 86 ���� + �и� ���ݷ� +x%

	APPLY_RESIST_ICE,		// 87 �ñ� ����
	APPLY_RESIST_EARTH,		// 88 ���� ����
	APPLY_RESIST_DARK,		// 89 ��� ����

	APPLY_ANTI_CRITICAL_PCT,	//90 ũ��Ƽ�� ����
	APPLY_ANTI_PENETRATE_PCT,	//91 ����Ÿ�� ����

#ifdef ENABLE_WOLFMAN_CHARACTER
	APPLY_BLEEDING_REDUCE = 92,	//92
	APPLY_BLEEDING_PCT = 93,	//93
	APPLY_ATT_BONUS_TO_WOLFMAN = 94,	//94
	APPLY_RESIST_WOLFMAN = 95,	//95
	APPLY_RESIST_CLAW = 96,	//96
#endif

#ifdef ENABLE_ACCE_SYSTEM
	APPLY_ACCEDRAIN_RATE = 97,	//97
#endif

	APPLY_RESIST_MAGIC_REDUCTION = 98,	//98

	MAX_APPLY_NUM = 99,
};

