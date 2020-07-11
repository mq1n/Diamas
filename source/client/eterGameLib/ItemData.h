#pragma once

// NOTE : Item의 통합 관리 클래스다.
//        Icon, Model (droped on ground), Game Data

#include "../../common/item_data.h"
#include "../eterLib/GrpSubImage.h"
#include "../eterGrnLib/Thing.h"
#include "../eterXClient/locale_inc.h"

class CItemData
{
	public:
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
			APPLY_MALL_ATTBONUS,        // 64 공격력 +x%
			APPLY_MALL_DEFBONUS,        // 65 방어력 +x%
			APPLY_MALL_EXPBONUS,        // 66 경험치 +x%
			APPLY_MALL_ITEMBONUS,       // 67 아이템 드롭율 x/10배
			APPLY_MALL_GOLDBONUS,       // 68 돈 드롭율 x/10배
			APPLY_MAX_HP_PCT,           // 69 최대 생명력 +x%
			APPLY_MAX_SP_PCT,           // 70 최대 정신력 +x%
			APPLY_SKILL_DAMAGE_BONUS,   // 71 스킬 데미지 * (100+x)%
			APPLY_NORMAL_HIT_DAMAGE_BONUS,      // 72 평타 데미지 * (100+x)%
			APPLY_SKILL_DEFEND_BONUS,   // 73 스킬 데미지 방어 * (100-x)%
			APPLY_NORMAL_HIT_DEFEND_BONUS,      // 74 평타 데미지 방어 * (100-x)%
			APPLY_EXTRACT_HP_PCT,		//75
			APPLY_PC_BANG_EXP_BONUS,		//76
			APPLY_PC_BANG_DROP_BONUS,		//77
			APPLY_RESIST_WARRIOR,			//78
			APPLY_RESIST_ASSASSIN ,			//79
			APPLY_RESIST_SURA,				//80
			APPLY_RESIST_SHAMAN,			//81
			APPLY_ENERGY,					//82
			APPLY_DEF_GRADE,				// 83 방어력. DEF_GRADE_BONUS는 클라에서 두배로 보여지는 의도된 버그(...)가 있다.
			APPLY_COSTUME_ATTR_BONUS,		// 84 코스튬 아이템에 붙은 속성치 보너스
			APPLY_MAGIC_ATTBONUS_PER,		// 85 마법 공격력 +x%
			APPLY_MELEE_MAGIC_ATTBONUS_PER,			// 86 마법 + 밀리 공격력 +x%
			
			APPLY_RESIST_ICE,		// 87 냉기 저항
			APPLY_RESIST_EARTH,		// 88 대지 저항
			APPLY_RESIST_DARK,		// 89 어둠 저항

			APPLY_ANTI_CRITICAL_PCT,	//90 크리티컬 저항
			APPLY_ANTI_PENETRATE_PCT,	//91 관통타격 저항

#ifdef ENABLE_WOLFMAN_CHARACTER
			APPLY_BLEEDING_REDUCE			= 92,	//92
			APPLY_BLEEDING_PCT				= 93,	//93
			APPLY_ATT_BONUS_TO_WOLFMAN		= 94,	//94
			APPLY_RESIST_WOLFMAN			= 95,	//95
			APPLY_RESIST_CLAW				= 96,	//96
#endif

#ifdef ENABLE_ACCE_SYSTEM
			APPLY_ACCEDRAIN_RATE			= 97,	//97
#endif

#ifdef ENABLE_MAGIC_REDUCTION_SYSTEM
			APPLY_RESIST_MAGIC_REDUCTION	= 98,	//98
#endif

			MAX_APPLY_NUM					= 99,
		};

#pragma pack(push)
#pragma pack(1)
		typedef struct SItemLimit
		{
			uint8_t        bType;
			int32_t        lValue;
		} TItemLimit;

		typedef struct SItemApply
		{
			uint8_t        bType;
			int32_t        lValue;
		} TItemApply;

		typedef struct SItemTable_r152
		{
			uint32_t       dwVnum;
			char        szName[ITEM_NAME_MAX_LEN + 1];
			char        szLocaleName[ITEM_NAME_MAX_LEN + 1];
			uint8_t        bType;
			uint8_t        bSubType;
			
			uint8_t        bWeight;
			uint8_t        bSize;
			
			uint32_t       dwAntiFlags;
			uint32_t       dwFlags;
			uint32_t       dwWearFlags;
			uint32_t       dwImmuneFlag;
						
			uint32_t       dwIBuyItemPrice;			
			uint32_t		dwISellItemPrice;
			
			TItemLimit  aLimits[ITEM_LIMIT_MAX_NUM];
			TItemApply  aApplies[ITEM_APPLY_MAX_NUM];
			int32_t        alValues[ITEM_VALUES_MAX_NUM];
			int32_t        alSockets[ITEM_SOCKET_MAX_NUM];
			uint32_t       dwRefinedVnum;
			uint16_t		wRefineSet;
			uint8_t        bAlterToMagicItemPct;
			uint8_t		bSpecular;
			uint8_t        bGainSocketPct;
		} TItemTable_r152;

		typedef struct SItemTable_r156
		{
			uint32_t       dwVnum;
			uint32_t       dwVnumRange;
			char        szName[ITEM_NAME_MAX_LEN + 1];
			char        szLocaleName[ITEM_NAME_MAX_LEN + 1];
			uint8_t        bType;
			uint8_t        bSubType;

			uint8_t        bWeight;
			uint8_t        bSize;

			uint32_t       dwAntiFlags;
			uint32_t       dwFlags;
			uint32_t       dwWearFlags;
			uint32_t       dwImmuneFlag;

			uint32_t       dwIBuyItemPrice;
			uint32_t		dwISellItemPrice;

			TItemLimit  aLimits[ITEM_LIMIT_MAX_NUM];
			TItemApply  aApplies[ITEM_APPLY_MAX_NUM];
			int32_t        alValues[ITEM_VALUES_MAX_NUM];
			int32_t        alSockets[ITEM_SOCKET_MAX_NUM];
			uint32_t       dwRefinedVnum;
			uint16_t		wRefineSet;
			uint8_t        bAlterToMagicItemPct;
			uint8_t		bSpecular;
			uint8_t        bGainSocketPct;
		} TItemTable_r156;

		typedef struct SItemTable_r158
		{
			uint32_t       dwVnum;
			uint32_t       dwVnumRange;
			char        szName[ITEM_NAME_MAX_LEN + 1];
			char        szLocaleName[ITEM_NAME_MAX_LEN + 1];
			uint8_t        bType;
			uint8_t        bSubType;

			uint8_t        bWeight;
			uint8_t        bSize;

			uint32_t       dwAntiFlags;
			uint32_t       dwFlags;
			uint32_t       dwWearFlags;
			uint32_t       dwImmuneFlag;

			uint32_t       dwIBuyItemPrice;
			uint32_t		dwISellItemPrice;

			TItemLimit  aLimits[ITEM_LIMIT_MAX_NUM];
			TItemApply  aApplies[ITEM_APPLY_MAX_NUM];
			int32_t        alValues[ITEM_VALUES_MAX_NUM];
			int32_t        alSockets[ITEM_SOCKET_MAX_NUM];
			uint32_t       dwRefinedVnum;
			uint16_t		wRefineSet;
			uint8_t        bAlterToMagicItemPct;
			uint8_t		bSpecular;
			uint8_t        bGainSocketPct;
			uint16_t		wWearableFlag;
		} TItemTable_r158;

		typedef SItemTable_r156 SItemTable, TItemTable;

#ifdef ENABLE_ACCE_SYSTEM
		struct SScaleInfo
		{
			float	fScaleX, fScaleY, fScaleZ;
			float	fPositionX, fPositionY, fPositionZ;
		};

		typedef struct SScaleTable
		{
			SScaleInfo	tInfo[10];
		} TScaleTable;
#endif

#ifdef ENABLE_PROTOSTRUCT_AUTODETECT
		typedef struct SItemTableAll
		{
			static bool IsValidStruct(uint32_t structSize)
			{
				switch (structSize)
				{
					case sizeof(TItemTable_r152):
					case sizeof(TItemTable_r156):
					case sizeof(TItemTable_r158):
						return true;
						break;
				}
				return false;
			}

			static void Process(void* obj, uint32_t structSize, uint32_t i, CItemData::TItemTable& t)
			{
				#define ITABLE_COPY_STR(x) strncpy_s(t.##x##, sizeof(t.##x##), r.##x##, _TRUNCATE)
				#define ITABLE_COPY_INT(x) t.##x## = r.##x
				#define ITABLE_ZERO_STR(x) memset(t.##x##, 0, sizeof(t.##x##));
				#define ITABLE_ZERO_INT(x) t.##x## = 0
				#define ITABLE_COUNT(x) _countof(t.##x##)
				#define ITABLE_PROCESS(len)\
					CItemData::TItemTable_r##len## & r = *((CItemData::TItemTable_r##len## *) obj + i);\
					ITABLE_COPY_INT(dwVnum);\
					ITABLE_COPY_STR(szName);\
					ITABLE_COPY_STR(szLocaleName);\
					ITABLE_COPY_INT(bType);\
					ITABLE_COPY_INT(bSubType);\
					ITABLE_COPY_INT(bWeight);\
					ITABLE_COPY_INT(bSize);\
					ITABLE_COPY_INT(dwAntiFlags);\
					ITABLE_COPY_INT(dwFlags);\
					ITABLE_COPY_INT(dwWearFlags);\
					ITABLE_COPY_INT(dwImmuneFlag);\
					ITABLE_COPY_INT(dwIBuyItemPrice);\
					ITABLE_COPY_INT(dwISellItemPrice);\
					for (size_t i=0; i<ITABLE_COUNT(aLimits); ++i)\
					{\
						ITABLE_COPY_INT(aLimits[i].bType);\
						ITABLE_COPY_INT(aLimits[i].lValue);\
					}\
					for (size_t i=0; i<EItemMisc::ITEM_APPLY_MAX_NUM; ++i)\
					{\
						ITABLE_COPY_INT(aApplies[i].bType);\
						ITABLE_COPY_INT(aApplies[i].lValue);\
					}\
					for (size_t i=0; i<EItemMisc::ITEM_VALUES_MAX_NUM; ++i)\
					{\
						ITABLE_COPY_INT(alValues[i]);\
					}\
					for (size_t i=0; i<EItemMisc::ITEM_SOCKET_MAX_NUM; ++i)\
					{\
						ITABLE_COPY_INT(alSockets[i]);\
					}\
					ITABLE_COPY_INT(dwRefinedVnum);\
					ITABLE_COPY_INT(wRefineSet);\
					ITABLE_COPY_INT(bAlterToMagicItemPct);\
					ITABLE_COPY_INT(bSpecular);\
					ITABLE_COPY_INT(bGainSocketPct);

				switch (structSize)
				{
					case sizeof(TItemTable_r152):
						{
							ITABLE_PROCESS(152);
							ITABLE_ZERO_INT(dwVnumRange);
						}
						break;
					case sizeof(TItemTable_r156):
						{
							ITABLE_PROCESS(156);
							ITABLE_COPY_INT(dwVnumRange);
						}
						break;
					case sizeof(TItemTable_r158):
						{
							ITABLE_PROCESS(158);
							ITABLE_COPY_INT(dwVnumRange);
						}
						break;
				}
			}
		} TItemTableAll;
#endif //ENABLE_PROTOSTRUCT_AUTODETECT

#pragma pack(pop)

	public:
		CItemData();
		virtual ~CItemData();

		void Clear();
		void SetSummary(const std::string& c_rstSumm);
		void SetDescription(const std::string& c_rstDesc);

		CGraphicThing * GetModelThing();
		CGraphicThing * GetSubModelThing();
		CGraphicThing * GetDropModelThing();
		CGraphicSubImage * GetIconImage(const std::string& stIconFileName = "");

		uint32_t GetLODModelThingCount();
		BOOL GetLODModelThingPointer(uint32_t dwIndex, CGraphicThing ** ppModelThing);

		uint32_t GetAttachingDataCount();
		BOOL GetCollisionDataPointer(uint32_t dwIndex, const NRaceData::TAttachingData ** c_ppAttachingData);
		BOOL GetAttachingDataPointer(uint32_t dwIndex, const NRaceData::TAttachingData ** c_ppAttachingData);

		/////
		const TItemTable*	GetTable() const;
		uint32_t GetIndex() const;
		const char * GetName() const;
		const char * GetDescription() const;
		const char * GetSummary() const;
		uint8_t GetType() const;
		uint8_t GetSubType() const;
		uint32_t GetRefine() const;
		const char* GetUseTypeString() const;
		uint32_t GetWeaponType() const;
		uint8_t GetSize() const;
		BOOL IsAntiFlag(uint32_t dwFlag) const;
		BOOL IsFlag(uint32_t dwFlag) const;
		BOOL IsWearableFlag(uint32_t dwFlag) const;
		BOOL HasNextGrade() const;

		uint32_t GetFlags() const;
		uint32_t GetAntiFlags() const;
		uint32_t GetWearFlags() const;
		uint32_t GetIBuyItemPrice() const;
		uint32_t GetISellItemPrice() const;
		int32_t GetLevelLimit() const;
		BOOL GetLimit(uint8_t byIndex, TItemLimit * pItemLimit) const;
		BOOL GetApply(uint8_t byIndex, TItemApply * pItemApply) const;
		int32_t GetValue(uint8_t byIndex) const;
		int32_t GetSocket(uint8_t byIndex) const;
		int32_t SetSocket(uint8_t byIndex,uint32_t value);
		int32_t GetSocketCount() const;
		uint32_t GetIconNumber() const;

		uint32_t	GetSpecularPoweru() const;
		float	GetSpecularPowerf() const;
	
		/////

		BOOL IsEquipment() const;

		/////

		//BOOL LoadItemData(const char * c_szFileName);
		void SetDefaultItemData(const char * c_szIconFileName, const char * c_szModelFileName  = nullptr);
		void SetItemTableData(TItemTable * pItemTable);

#ifdef ENABLE_ACCE_SYSTEM
		void SetItemScale(const std::string strJob, const std::string strSex, const std::string strScaleX, const std::string strScaleY, const std::string strScaleZ, const std::string strPositionX, const std::string strPositionY, const std::string strPositionZ);
		bool GetItemScale(uint32_t dwPos, float & fScaleX, float & fScaleY, float & fScaleZ, float & fPositionX, float & fPositionY, float & fPositionZ);
#endif

		bool IsStackable() const { return IsFlag(ITEM_FLAG_STACKABLE) && !IsAntiFlag(ITEM_ANTIFLAG_STACK); }
	protected:
		void __LoadFiles();
		void __SetIconImage(const char * c_szFileName, bool renew = false);

	protected:
		std::string m_strModelFileName;
		std::string m_strSubModelFileName;
		std::string m_strDropModelFileName;
		std::string m_strIconFileName;
		std::string m_strDescription;
		std::string m_strSummary;
		std::vector<std::string> m_strLODModelFileNameVector;

		CGraphicThing * m_pModelThing;
		CGraphicThing * m_pSubModelThing;
		CGraphicThing * m_pDropModelThing;
		CGraphicSubImage * m_pIconImage;
		std::vector<CGraphicThing *> m_pLODModelThingVector;

		NRaceData::TAttachingDataVector m_AttachingDataVector;
		uint32_t		m_dwVnum;
		TItemTable m_ItemTable;
#ifdef ENABLE_ACCE_SYSTEM
		TScaleTable	m_ScaleTable;
#endif
		
	public:
		static void DestroySystem();

		static CItemData* New();
		static void Delete(CItemData* pkItemData);

		static CDynamicPool<CItemData>		ms_kPool;
};

inline bool operator<(const CItemData::TItemTable & lhs, const CItemData::TItemTable & rhs)
{
	return lhs.dwVnum < rhs.dwVnum;
}
