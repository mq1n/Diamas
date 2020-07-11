#pragma once

enum EPointTypes
{
    POINT_NONE,                 // 0
    POINT_LEVEL,                // 1
    POINT_VOICE,                // 2
    POINT_EXP,                  // 3
    POINT_NEXT_EXP,             // 4
    POINT_HP,                   // 5
    POINT_MAX_HP,               // 6
    POINT_SP,                   // 7
    POINT_MAX_SP,               // 8  
    POINT_STAMINA,              // 9  스테미너
    POINT_MAX_STAMINA,          // 10 최대 스테미너
    
    POINT_GOLD,                 // 11
    POINT_ST,                   // 12 근력
    POINT_HT,                   // 13 체력
    POINT_DX,                   // 14 민첩성
    POINT_IQ,                   // 15 정신력
    POINT_ATT_POWER,            // 16 공격력
    POINT_ATT_SPEED,            // 17 공격속도
    POINT_EVADE_RATE,           // 18 회피율
    POINT_MOV_SPEED,            // 19 이동속도
    POINT_DEF_GRADE,            // 20 방어등급
	POINT_CASTING_SPEED,        // 21 주문속도 (쿨다운타임*100) / (100 + 이값) = 최종 쿨다운 타임
	POINT_MAGIC_ATT_GRADE,      // 22 마법공격력
    POINT_MAGIC_DEF_GRADE,      // 23 마법방어력
    POINT_EMPIRE_POINT,         // 24 제국점수
    POINT_LEVEL_STEP,           // 25 한 레벨에서의 단계.. (1 2 3 될 때 보상, 4 되면 레벨 업)
    POINT_STAT,                 // 26 능력치 올릴 수 있는 개수
	POINT_SUB_SKILL,            // 27 보조 스킬 포인트
	POINT_SKILL,                // 28 액티브 스킬 포인트
//    POINT_SKILL_PASV,           // 27 패시브 기술 올릴 수 있는 개수
//    POINT_SKILL_ACTIVE,         // 28 액티브 스킬 포인트
	POINT_MIN_ATK,				// 29 최소 파괴력
	POINT_MAX_ATK,				// 30 최대 파괴력
    POINT_PLAYTIME,             // 31 플레이시간
    POINT_HP_REGEN,             // 32 HP 회복률
    POINT_SP_REGEN,             // 33 SP 회복률
    
    POINT_BOW_DISTANCE,         // 34 활 사정거리 증가치 (meter)
    
    POINT_HP_RECOVERY,          // 35 체력 회복 증가량
    POINT_SP_RECOVERY,          // 36 정신력 회복 증가량
    
    POINT_POISON_PCT,           // 37 독 확률
    POINT_STUN_PCT,             // 38 기절 확률
    POINT_SLOW_PCT,             // 39 슬로우 확률
    POINT_CRITICAL_PCT,         // 40 크리티컬 확률
    POINT_PENETRATE_PCT,        // 41 관통타격 확률
    POINT_CURSE_PCT,            // 42 저주 확률
    
    POINT_ATTBONUS_HUMAN,       // 43 인간에게 강함
    POINT_ATTBONUS_ANIMAL,      // 44 동물에게 데미지 % 증가
    POINT_ATTBONUS_ORC,         // 45 웅귀에게 데미지 % 증가
    POINT_ATTBONUS_MILGYO,      // 46 밀교에게 데미지 % 증가
    POINT_ATTBONUS_UNDEAD,      // 47 시체에게 데미지 % 증가
    POINT_ATTBONUS_DEVIL,       // 48 마귀(악마)에게 데미지 % 증가
    POINT_ATTBONUS_INSECT,      // 49 벌레족
    POINT_ATTBONUS_FIRE,        // 50 화염족
    POINT_ATTBONUS_ICE,         // 51 빙설족
    POINT_ATTBONUS_DESERT,      // 52 사막족
    POINT_ATTBONUS_UNUSED0,     // 53 UNUSED0
    POINT_ATTBONUS_UNUSED1,     // 54 UNUSED1
    POINT_ATTBONUS_UNUSED2,     // 55 UNUSED2
    POINT_ATTBONUS_UNUSED3,     // 56 UNUSED3
    POINT_ATTBONUS_UNUSED4,     // 57 UNUSED4
    POINT_ATTBONUS_UNUSED5,     // 58 UNUSED5
    POINT_ATTBONUS_UNUSED6,     // 59 UNUSED6
    POINT_ATTBONUS_UNUSED7,     // 60 UNUSED7
    POINT_ATTBONUS_UNUSED8,     // 61 UNUSED8
    POINT_ATTBONUS_UNUSED9,     // 62 UNUSED9

    POINT_STEAL_HP,             // 63 생명력 흡수
    POINT_STEAL_SP,             // 64 정신력 흡수

    POINT_MANA_BURN_PCT,        // 65 마나 번

    /// 피해시 보너스 ///

    POINT_DAMAGE_SP_RECOVER,    // 66 공격당할 시 정신력 회복 확률

    POINT_BLOCK,                // 67 블럭율
    POINT_DODGE,                // 68 회피율

    POINT_RESIST_SWORD,         // 69
    POINT_RESIST_TWOHAND,       // 70
    POINT_RESIST_DAGGER,        // 71
    POINT_RESIST_BELL,          // 72
    POINT_RESIST_FAN,           // 73
    POINT_RESIST_BOW,           // 74  화살   저항   : 대미지 감소
    POINT_RESIST_FIRE,          // 75  화염   저항   : 화염공격에 대한 대미지 감소
    POINT_RESIST_ELEC,          // 76  전기   저항   : 전기공격에 대한 대미지 감소
    POINT_RESIST_MAGIC,         // 77  술법   저항   : 모든술법에 대한 대미지 감소
    POINT_RESIST_WIND,          // 78  바람   저항   : 바람공격에 대한 대미지 감소

    POINT_REFLECT_MELEE,        // 79 공격 반사

    /// 특수 피해시 ///
    POINT_REFLECT_CURSE,        // 80 저주 반사
    POINT_POISON_REDUCE,        // 81 독데미지 감소

    /// 적 소멸시 ///
    POINT_KILL_SP_RECOVER,      // 82 적 소멸시 MP 회복
    POINT_EXP_DOUBLE_BONUS,     // 83
    POINT_GOLD_DOUBLE_BONUS,    // 84
    POINT_ITEM_DROP_BONUS,      // 85

    /// 회복 관련 ///
    POINT_POTION_BONUS,         // 86
    POINT_KILL_HP_RECOVER,      // 87

    POINT_IMMUNE_STUN,          // 88
    POINT_IMMUNE_SLOW,          // 89
    POINT_IMMUNE_FALL,          // 90
    //////////////////

    POINT_PARTY_ATT_GRADE,      // 91
    POINT_PARTY_DEF_GRADE,      // 92

    POINT_ATT_BONUS,            // 93
    POINT_DEF_BONUS,            // 94

    POINT_ATT_GRADE_BONUS,			// 95
    POINT_DEF_GRADE_BONUS,			// 96
    POINT_MAGIC_ATT_GRADE_BONUS,	// 97
    POINT_MAGIC_DEF_GRADE_BONUS,	// 98

    POINT_RESIST_NORMAL_DAMAGE,		// 99

	POINT_STAT_RESET_COUNT = 112,
    POINT_HORSE_SKILL = 113,

	POINT_MALL_ATTBONUS,		// 114 공격력 +x%
	POINT_MALL_DEFBONUS,		// 115 방어력 +x%
	POINT_MALL_EXPBONUS,		// 116 경험치 +x%
	POINT_MALL_ITEMBONUS,		// 117 아이템 드롭율 x/10배
	POINT_MALL_GOLDBONUS,		// 118 돈 드롭율 x/10배
    POINT_MAX_HP_PCT,			// 119 최대생명력 +x%
    POINT_MAX_SP_PCT,			// 120 최대정신력 +x%

	POINT_SKILL_DAMAGE_BONUS,       // 121 스킬 데미지 *(100+x)%
	POINT_NORMAL_HIT_DAMAGE_BONUS,  // 122 평타 데미지 *(100+x)%
   
    POINT_SKILL_DEFEND_BONUS,       // 123 스킬 방어 데미지
    POINT_NORMAL_HIT_DEFEND_BONUS,  // 124 평타 방어 데미지
    POINT_PC_BANG_EXP_BONUS,        // 125
	POINT_PC_BANG_DROP_BONUS,       // 126 PC방 전용 드롭률 보너스

	POINT_ENERGY = 128,				// 128 기력

	// 기력 ui 용.
	// 이렇게 하고 싶지 않았지만, 
	// uiTaskBar에서는 affect에 접근할 수 없고,
	// 더구나 클라리언트에서는 blend_affect는 관리하지 않아,
	// 임시로 이렇게 둔다.
	POINT_ENERGY_END_TIME = 129,	// 129 기력 종료 시간

	// 클라이언트 포인트
	POINT_MIN_WEP = 200,
	POINT_MAX_WEP,
	POINT_MIN_MAGIC_WEP,
	POINT_MAX_MAGIC_WEP,
	POINT_HIT_RATE,


    //POINT_MAX_NUM = 255,=>stdafx.h 로/
};
