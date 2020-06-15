#ifndef __INC_METIN_II_GAME_CSkillManager_H__
#define __INC_METIN_II_GAME_CSkillManager_H__

#include "../../libthecore/include/poly.h"
#include "../../common/service.h"

enum ESkillFlags
{
	SKILL_FLAG_ATTACK			= (1 << 0),	// ���� ���
	SKILL_FLAG_USE_MELEE_DAMAGE		= (1 << 1),	// �⺻ �и� Ÿ��ġ�� b ������ ���
	SKILL_FLAG_COMPUTE_ATTGRADE		= (1 << 2),	// ���ݵ���� ����Ѵ�
	SKILL_FLAG_SELFONLY			= (1 << 3),	// �ڽſ��Ը� �� �� ����
	SKILL_FLAG_USE_MAGIC_DAMAGE		= (1 << 4),	// �⺻ ���� Ÿ��ġ�� b ������ ���
	SKILL_FLAG_USE_HP_AS_COST		= (1 << 5),	// HP�� SP��� ����
	SKILL_FLAG_COMPUTE_MAGIC_DAMAGE	= (1 << 6),
	SKILL_FLAG_SPLASH			= (1 << 7),
	SKILL_FLAG_GIVE_PENALTY		= (1 << 8),	// ������ ��õ���(3��) 2�� �������� �޴´�.
	SKILL_FLAG_USE_ARROW_DAMAGE		= (1 << 9),	// �⺻ ȭ�� Ÿ��ġ�� b ������ ���
	SKILL_FLAG_PENETRATE		= (1 << 10),	// ����
	SKILL_FLAG_IGNORE_TARGET_RATING	= (1 << 11),	// ��� ������ ����
	SKILL_FLAG_SLOW			= (1 << 12),	// ���ο� ����
	SKILL_FLAG_STUN			= (1 << 13),	// ���� ����
	SKILL_FLAG_HP_ABSORB		= (1 << 14),	// HP ���
	SKILL_FLAG_SP_ABSORB		= (1 << 15),	// SP ���
	SKILL_FLAG_FIRE_CONT		= (1 << 16),	// FIRE ���� ������
	SKILL_FLAG_REMOVE_BAD_AFFECT	= (1 << 17),	// ����ȿ�� ����
	SKILL_FLAG_REMOVE_GOOD_AFFECT	= (1 << 18),	// ����ȿ�� ����
	SKILL_FLAG_CRUSH			= (1 << 19),	// ������ ����
	SKILL_FLAG_POISON			= (1 << 20),	// �� ����
	SKILL_FLAG_TOGGLE			= (1 << 21),	// ���
	SKILL_FLAG_DISABLE_BY_POINT_UP	= (1 << 22),	// �� �ø� �� ����.
	SKILL_FLAG_CRUSH_LONG		= (1 << 23),	// ������ �ָ� ����
	SKILL_FLAG_WIND		= (1 << 24),	// �ٶ� �Ӽ� 
	SKILL_FLAG_ELEC		= (1 << 25),	// ���� �Ӽ�
	SKILL_FLAG_FIRE		= (1 << 26),	// �� �Ӽ�
#ifdef ENABLE_WOLFMAN_CHARACTER
	SKILL_FLAG_BLEEDING	= (1 << 27),
	SKILL_FLAG_PARTY	= (1 << 28),
#endif
};

enum
{
	SKILL_PENALTY_DURATION = 3,
	SKILL_TYPE_HORSE = 5,
};

enum ESkillIndexes
{
	SKILL_RESERVED = 0,

	// �����c Au��c ��e����		// - bedensel
	// A
	SKILL_SAMYEON = 1,		// 3 yonlu kesme
	SKILL_PALBANG,			// k��l��c cevirme
	// S
	SKILL_JEONGWI,			// ofke
	SKILL_GEOMKYUNG,		// hava
	SKILL_TANHWAN,			// hamle

	// �����c ��a�Ʃ� ��e����		// - zihinsel
	// A
	SKILL_GIGONGCHAM = 16,	// ruh
	SKILL_GYOKSAN,			// ?iddetli vurul
	SKILL_DAEJINGAK,		// guclu vuru?
	// S
	SKILL_CHUNKEON,			// guclu beden
	SKILL_GEOMPUNG,			// k��l��c darbesi

	// AU�Ƣ� ��I��i ��e����		// - yak��n dovu?
	// A
	SKILL_AMSEOP = 31,		// suikast
	SKILL_GUNGSIN,			// h��zl�� sald��r��
	SKILL_CHARYUN,			// b��cak cevirme
	// S
	SKILL_EUNHYUNG,			// kamuflaj 
	SKILL_SANGONG,			// zehirli bulut

	// AU�Ƣ� ��A��o ��e����		// - uzak dovu?
	// A
	SKILL_YEONSA = 46,		// tekrarlanan at��?
	SKILL_KWANKYEOK,		// ok ya?muru
	SKILL_HWAJO,			// ate?li ok
	// S
	SKILL_GYEONGGONG,		// hafif ad��m
	SKILL_GIGUNG,			// zehirli ok

	// ��o��o ��E				// - buyulu silah
	// A
	SKILL_SWAERYUNG = 61,	// parmak darbesi
	SKILL_YONGKWON,			// ejderha donu?u
	// S
	SKILL_GWIGEOM,			// buyulu keskinlik
	SKILL_TERROR,			// deh?et
	SKILL_JUMAGAP,			// buyulu z��rh
	SKILL_PABEOB,			// buyu cozme

	// ��o��o ���ҩ���			// - kara buyu
	// A
	SKILL_MARYUNG = 76,		// karanl��k vuru?
	SKILL_HWAYEOMPOK,		// ate? vuru?
	SKILL_MUYEONG,			// ate? hayaleti
	// S
	SKILL_MANASHIELD,		// karanl��k koruma
	SKILL_TUSOK,			// hayalet vuru?
	SKILL_MAHWAN,			// karanl��k kure

	// ���좥c ��e��A
	// A					// - ejderha gucu
	SKILL_BIPABU = 91,		// ucan t��ls��m
	SKILL_YONGBI,			// ejderha at��?��
	SKILL_PAERYONG,			// ejderha kukremesi
	// S
	//SKILL_BUDONG,			// pasif yada eski ?aman skilli
	SKILL_HOSIN,			// kutsama
	SKILL_REFLECT,			// yans��tma
	SKILL_GICHEON,			// ejderha yard��m��

	// ���좥c ��u��A
	// A					// - Iyile?tirme
	SKILL_NOEJEON = 106,	// ?im?ek atma
	SKILL_BYEURAK,			// ?im?ek Ca?��rma
	SKILL_CHAIN,			// ?im?ek pencesi
	// S
	SKILL_JEONGEOP,			// Iyile?tirme
	SKILL_KWAESOK,			// H��z
	SKILL_JEUNGRYEOK,		// Yuksek sald��r��

	// ����A�� ����A��

	SKILL_LEADERSHIP	= 121,	// Liderlik
	SKILL_COMBO			= 122,	// Kombo
	SKILL_CREATE		= 123,	// ?? - Uzman recetesi ile yukseltilen skill
	SKILL_MINING		= 124,	// Madencilik

	SKILL_LANGUAGE1		= 126,	// Shinsoo lisan 
	SKILL_LANGUAGE2		= 127,	// Chunjo Lisan
	SKILL_LANGUAGE3		= 128,	// Jinno Lisan
	SKILL_POLYMORPH		= 129,	// Donu?me

	SKILL_HORSE						= 130,	// Binicilik
	SKILL_HORSE_SUMMON				= 131,	// At ca?��rma
	SKILL_HORSE_WILDATTACK			= 137,	// At uzerinde sald��r��
	SKILL_HORSE_CHARGE				= 138,	// At yuruyu?u
	SKILL_HORSE_ESCAPE				= 139,	// Guc dalgas��
	SKILL_HORSE_WILDATTACK_RANGE	= 140,	// Ok f��rt��nas��

	SKILL_ADD_HP			= 141,	// HP uretme - Pasif skill 		/ Aktif de?il
	SKILL_RESIST_PENETRATE	= 142,	// Delici vuru? - Pasif skill 	/ Aktif de?il

	GUILD_SKILL_START		= 151,			// [LONCA] Ba?lang��c
	GUILD_SKILL_EYE			= 151,			// Ejderha Gozleri 				/ Ejderha Hayaletleri
	GUILD_SKILL_BLOOD		= 152,			// Ejderha Tanr��s�� Kan�� 		/ Max HP
	GUILD_SKILL_BLESS		= 153,			// Ejderha Tanr��s�� Kutsamas�� 	/ Max SP
	GUILD_SKILL_SEONGHWI	= 154,			// Kutsal Z��rh 					/ Savunma
	GUILD_SKILL_ACCEL		= 155,			// H��zland��rma 					/ Hareket H��z��
	GUILD_SKILL_BUNNO		= 156,			// Ejderha Tanr��s�� ofkesi 		/ Kritik
	GUILD_SKILL_JUMUN		= 157,			// Buyu h��z�� deste?i 			/ Buyu h��z��
	GUILD_SKILL_END			= 162,			// [LONCA] Son

	GUILD_SKILL_COUNT		= GUILD_SKILL_END - GUILD_SKILL_START + 1,

#ifdef ENABLE_WOLFMAN_CHARACTER
	SKILL_CHAYEOL			= 170,			// Y��rtma	Parcalama
	SKILL_SALPOONG			= 171,			// Kurt Nefesi
	SKILL_GONGDAB			= 172,			// Kurt Atlay��?��
	SKILL_PASWAE			= 173,			// Kurt Pencesi
	SKILL_JEOKRANG			= 174,			// K��rm��z�� Kurt Ruhu
	SKILL_CHEONGRANG		= 175,			// Civit Kurt Ruhu
#endif
};

class CSkillProto
{
	public:
		char	szName[64];
		uint32_t	dwVnum;			// ��ȣ

		uint32_t	dwType;			// 0: ������, 1: ����, 2: �ڰ�, 3: ����, 4: ����
		uint8_t	bMaxLevel;		// �ִ� ���õ�
		uint8_t	bLevelLimit;		// ��������
		int32_t	iSplashRange;		// ���÷��� �Ÿ� ����

		uint8_t	bPointOn;		// ��� ������� ���� ��Ű�°�? (Ÿ��ġ, MAX HP, HP REGEN ����)
		CPoly	kPointPoly;		// ����� ����� ����

		CPoly	kSPCostPoly;		// ��� SP ����
		CPoly	kDurationPoly;		// ���� �ð� ����
		CPoly	kDurationSPCostPoly;	// ���� SP ����
		CPoly	kCooldownPoly;		// ��ٿ� �ð� ����
		CPoly	kMasterBonusPoly;	// �������� �� ���ʽ� ����
		CPoly	kSplashAroundDamageAdjustPoly;	// ���÷��� ������ ��� ���� ������ ������ ������ ���� ����

		uint32_t	dwFlag;			// ��ų�ɼ�
		uint32_t	dwAffectFlag;		// ��ų�� ���� ��� ����Ǵ� Affect

		uint8_t	bLevelStep;		// �ѹ��� �ø��µ� �ʿ��� ��ų ����Ʈ ��
		uint32_t	preSkillVnum;		// ���µ� �ʿ��� ������ ������� ��ų
		uint8_t	preSkillLevel;		// ������ ������� ��ų�� ����

		int32_t	lMaxHit;

		uint8_t	bSkillAttrType;

		// 2�� ����
		uint8_t	bPointOn2;		
		CPoly	kPointPoly2;		
		CPoly	kDurationPoly2;		
		uint32_t	dwFlag2;			
		uint32_t	dwAffectFlag2;		

		uint32_t   dwTargetRange;

		bool	IsChargeSkill()
		{
			return dwVnum == SKILL_TANHWAN || dwVnum == SKILL_HORSE_CHARGE; 
		}

		// 3�� ����
		uint8_t bPointOn3;
		CPoly kPointPoly3;
		CPoly kDurationPoly3;

		CPoly kGrandMasterAddSPCostPoly;

		void SetPointVar(const std::string& strName, double dVar);
		void SetDurationVar(const std::string& strName, double dVar);
		void SetSPCostVar(const std::string& strName, double dVar);
};

class CSkillManager : public singleton<CSkillManager>
{
	public:
		CSkillManager();
		virtual ~CSkillManager();

		bool Initialize(TSkillTable * pTab, int32_t iSize);
		CSkillProto * Get(uint32_t dwVnum);
		CSkillProto * Get(const char * c_pszSkillName);

	protected:
		std::map<uint32_t, CSkillProto *> m_map_pkSkillProto;
};

#endif
