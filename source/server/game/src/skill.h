#pragma once
#include "../../libthecore/include/poly.h"
#include "../../../common/defines.h"
#include "../../../common/tables.h"

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
