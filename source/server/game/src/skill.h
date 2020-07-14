#pragma once
#include "../../libthecore/include/poly.h"
#include "../../../common/defines.h"
#include "../../../common/tables.h"

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

class CSkillManager : public CSingleton<CSkillManager>
{
	public:
		CSkillManager();
		virtual ~CSkillManager();

		bool Initialize(TSkillTable * pTab, int32_t iSize);
		CSkillProto * Get(uint32_t dwVnum);
		CSkillProto * Get(const char * c_pszSkillName);

	protected:
		std::map <uint32_t, CSkillProto *> m_map_pkSkillProto;
};
