#ifndef __INC_METIN_II_GAME_CSkillManager_H__
#define __INC_METIN_II_GAME_CSkillManager_H__

#include "../../libthecore/include/poly.h"
#include "../../common/service.h"

enum ESkillFlags
{
	SKILL_FLAG_ATTACK			= (1 << 0),	// 공격 기술
	SKILL_FLAG_USE_MELEE_DAMAGE		= (1 << 1),	// 기본 밀리 타격치를 b 값으로 사용
	SKILL_FLAG_COMPUTE_ATTGRADE		= (1 << 2),	// 공격등급을 계산한다
	SKILL_FLAG_SELFONLY			= (1 << 3),	// 자신에게만 쓸 수 있음
	SKILL_FLAG_USE_MAGIC_DAMAGE		= (1 << 4),	// 기본 마법 타격치를 b 값으로 사용
	SKILL_FLAG_USE_HP_AS_COST		= (1 << 5),	// HP를 SP대신 쓴다
	SKILL_FLAG_COMPUTE_MAGIC_DAMAGE	= (1 << 6),
	SKILL_FLAG_SPLASH			= (1 << 7),
	SKILL_FLAG_GIVE_PENALTY		= (1 << 8),	// 쓰고나면 잠시동안(3초) 2배 데미지를 받는다.
	SKILL_FLAG_USE_ARROW_DAMAGE		= (1 << 9),	// 기본 화살 타격치를 b 값으로 사용
	SKILL_FLAG_PENETRATE		= (1 << 10),	// 방어무시
	SKILL_FLAG_IGNORE_TARGET_RATING	= (1 << 11),	// 상대 레이팅 무시
	SKILL_FLAG_SLOW			= (1 << 12),	// 슬로우 공격
	SKILL_FLAG_STUN			= (1 << 13),	// 스턴 공격
	SKILL_FLAG_HP_ABSORB		= (1 << 14),	// HP 흡수
	SKILL_FLAG_SP_ABSORB		= (1 << 15),	// SP 흡수
	SKILL_FLAG_FIRE_CONT		= (1 << 16),	// FIRE 지속 데미지
	SKILL_FLAG_REMOVE_BAD_AFFECT	= (1 << 17),	// 나쁜효과 제거
	SKILL_FLAG_REMOVE_GOOD_AFFECT	= (1 << 18),	// 나쁜효과 제거
	SKILL_FLAG_CRUSH			= (1 << 19),	// 상대방을 날림
	SKILL_FLAG_POISON			= (1 << 20),	// 독 공격
	SKILL_FLAG_TOGGLE			= (1 << 21),	// 토글
	SKILL_FLAG_DISABLE_BY_POINT_UP	= (1 << 22),	// 찍어서 올릴 수 없다.
	SKILL_FLAG_CRUSH_LONG		= (1 << 23),	// 상대방을 멀리 날림
	SKILL_FLAG_WIND		= (1 << 24),	// 바람 속성 
	SKILL_FLAG_ELEC		= (1 << 25),	// 전기 속성
	SKILL_FLAG_FIRE		= (1 << 26),	// 불 속성
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

	// ¹≪≫c Au≫c °e¿­		// - bedensel
	// A
	SKILL_SAMYEON = 1,		// 3 yonlu kesme
	SKILL_PALBANG,			// kılıc cevirme
	// S
	SKILL_JEONGWI,			// ofke
	SKILL_GEOMKYUNG,		// hava
	SKILL_TANHWAN,			// hamle

	// ¹≪≫c ±a°ø °e¿­		// - zihinsel
	// A
	SKILL_GIGONGCHAM = 16,	// ruh
	SKILL_GYOKSAN,			// ?iddetli vurul
	SKILL_DAEJINGAK,		// guclu vuru?
	// S
	SKILL_CHUNKEON,			// guclu beden
	SKILL_GEOMPUNG,			// kılıc darbesi

	// AU°´ ¾I≫i °e¿­		// - yakın dovu?
	// A
	SKILL_AMSEOP = 31,		// suikast
	SKILL_GUNGSIN,			// hızlı saldırı
	SKILL_CHARYUN,			// bıcak cevirme
	// S
	SKILL_EUNHYUNG,			// kamuflaj 
	SKILL_SANGONG,			// zehirli bulut

	// AU°´ ±A¼o °e¿­		// - uzak dovu?
	// A
	SKILL_YEONSA = 46,		// tekrarlanan atı?
	SKILL_KWANKYEOK,		// ok ya?muru
	SKILL_HWAJO,			// ate?li ok
	// S
	SKILL_GYEONGGONG,		// hafif adım
	SKILL_GIGUNG,			// zehirli ok

	// ¼o¶o °E				// - buyulu silah
	// A
	SKILL_SWAERYUNG = 61,	// parmak darbesi
	SKILL_YONGKWON,			// ejderha donu?u
	// S
	SKILL_GWIGEOM,			// buyulu keskinlik
	SKILL_TERROR,			// deh?et
	SKILL_JUMAGAP,			// buyulu zırh
	SKILL_PABEOB,			// buyu cozme

	// ¼o¶o ¸¶¹ı			// - kara buyu
	// A
	SKILL_MARYUNG = 76,		// karanlık vuru?
	SKILL_HWAYEOMPOK,		// ate? vuru?
	SKILL_MUYEONG,			// ate? hayaleti
	// S
	SKILL_MANASHIELD,		// karanlık koruma
	SKILL_TUSOK,			// hayalet vuru?
	SKILL_MAHWAN,			// karanlık kure

	// ¹≪´c ¿e½A
	// A					// - ejderha gucu
	SKILL_BIPABU = 91,		// ucan tılsım
	SKILL_YONGBI,			// ejderha atı?ı
	SKILL_PAERYONG,			// ejderha kukremesi
	// S
	//SKILL_BUDONG,			// pasif yada eski ?aman skilli
	SKILL_HOSIN,			// kutsama
	SKILL_REFLECT,			// yansıtma
	SKILL_GICHEON,			// ejderha yardımı

	// ¹≪´c ³u½A
	// A					// - Iyile?tirme
	SKILL_NOEJEON = 106,	// ?im?ek atma
	SKILL_BYEURAK,			// ?im?ek Ca?ırma
	SKILL_CHAIN,			// ?im?ek pencesi
	// S
	SKILL_JEONGEOP,			// Iyile?tirme
	SKILL_KWAESOK,			// Hız
	SKILL_JEUNGRYEOK,		// Yuksek saldırı

	// º¸A¶ ½ºA³

	SKILL_LEADERSHIP	= 121,	// Liderlik
	SKILL_COMBO			= 122,	// Kombo
	SKILL_CREATE		= 123,	// ?? - Uzman recetesi ile yukseltilen skill
	SKILL_MINING		= 124,	// Madencilik

	SKILL_LANGUAGE1		= 126,	// Shinsoo lisan 
	SKILL_LANGUAGE2		= 127,	// Chunjo Lisan
	SKILL_LANGUAGE3		= 128,	// Jinno Lisan
	SKILL_POLYMORPH		= 129,	// Donu?me

	SKILL_HORSE						= 130,	// Binicilik
	SKILL_HORSE_SUMMON				= 131,	// At ca?ırma
	SKILL_HORSE_WILDATTACK			= 137,	// At uzerinde saldırı
	SKILL_HORSE_CHARGE				= 138,	// At yuruyu?u
	SKILL_HORSE_ESCAPE				= 139,	// Guc dalgası
	SKILL_HORSE_WILDATTACK_RANGE	= 140,	// Ok fırtınası

	SKILL_ADD_HP			= 141,	// HP uretme - Pasif skill 		/ Aktif de?il
	SKILL_RESIST_PENETRATE	= 142,	// Delici vuru? - Pasif skill 	/ Aktif de?il

	GUILD_SKILL_START		= 151,			// [LONCA] Ba?langıc
	GUILD_SKILL_EYE			= 151,			// Ejderha Gozleri 				/ Ejderha Hayaletleri
	GUILD_SKILL_BLOOD		= 152,			// Ejderha Tanrısı Kanı 		/ Max HP
	GUILD_SKILL_BLESS		= 153,			// Ejderha Tanrısı Kutsaması 	/ Max SP
	GUILD_SKILL_SEONGHWI	= 154,			// Kutsal Zırh 					/ Savunma
	GUILD_SKILL_ACCEL		= 155,			// Hızlandırma 					/ Hareket Hızı
	GUILD_SKILL_BUNNO		= 156,			// Ejderha Tanrısı ofkesi 		/ Kritik
	GUILD_SKILL_JUMUN		= 157,			// Buyu hızı deste?i 			/ Buyu hızı
	GUILD_SKILL_END			= 162,			// [LONCA] Son

	GUILD_SKILL_COUNT		= GUILD_SKILL_END - GUILD_SKILL_START + 1,

#ifdef ENABLE_WOLFMAN_CHARACTER
	SKILL_CHAYEOL			= 170,			// Yırtma	Parcalama
	SKILL_SALPOONG			= 171,			// Kurt Nefesi
	SKILL_GONGDAB			= 172,			// Kurt Atlayı?ı
	SKILL_PASWAE			= 173,			// Kurt Pencesi
	SKILL_JEOKRANG			= 174,			// Kırmızı Kurt Ruhu
	SKILL_CHEONGRANG		= 175,			// Civit Kurt Ruhu
#endif
};

class CSkillProto
{
	public:
		char	szName[64];
		uint32_t	dwVnum;			// 번호

		uint32_t	dwType;			// 0: 전직업, 1: 무사, 2: 자객, 3: 수라, 4: 무당
		uint8_t	bMaxLevel;		// 최대 수련도
		uint8_t	bLevelLimit;		// 레벨제한
		int32_t	iSplashRange;		// 스플래쉬 거리 제한

		uint8_t	bPointOn;		// 어디에 결과값을 적용 시키는가? (타격치, MAX HP, HP REGEN 등등등)
		CPoly	kPointPoly;		// 결과값 만드는 공식

		CPoly	kSPCostPoly;		// 사용 SP 공식
		CPoly	kDurationPoly;		// 지속 시간 공식
		CPoly	kDurationSPCostPoly;	// 지속 SP 공식
		CPoly	kCooldownPoly;		// 쿨다운 시간 공식
		CPoly	kMasterBonusPoly;	// 마스터일 때 보너스 공식
		CPoly	kSplashAroundDamageAdjustPoly;	// 스플래쉬 공격일 경우 주위 적에게 입히는 데미지 감소 비율

		uint32_t	dwFlag;			// 스킬옵션
		uint32_t	dwAffectFlag;		// 스킬에 맞은 경우 적용되는 Affect

		uint8_t	bLevelStep;		// 한번에 올리는데 필요한 스킬 포인트 수
		uint32_t	preSkillVnum;		// 배우는데 필요한 이전에 배워야할 스킬
		uint8_t	preSkillLevel;		// 이전에 배워야할 스킬의 레벨

		int32_t	lMaxHit;

		uint8_t	bSkillAttrType;

		// 2차 적용
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

		// 3차 적용
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
