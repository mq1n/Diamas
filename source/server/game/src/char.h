#pragma once
#include <unordered_map>
#include "../../common/stl.h"
#include "../../common/service.h"
#include "entity.h"
#include "FSM.h"
#include "horse_rider.h"
#include "vid.h"
#include "constants.h"
#include "affect.h"
#include "affect_flag.h"
#include "cube.h"
#include "mining.h"
#include "abuse.h"
#include "sectree_manager.h"
#include "gposition.h"
#include "activity.h"

enum eMountType {MOUNT_TYPE_NONE=0, MOUNT_TYPE_NORMAL=1, MOUNT_TYPE_COMBAT=2, MOUNT_TYPE_MILITARY=3};
eMountType GetMountLevelByVnum(uint32_t dwMountVnum, bool IsNew);
uint32_t GetRandomSkillVnum(uint8_t bJob = JOB_MAX_NUM);

class CBuffOnAttributes;
class CPetSystem;
class CBattleground;

enum EInstantFlags
{
	INSTANT_FLAG_DEATH_PENALTY	= (1 << 0),
	INSTANT_FLAG_SHOP			= (1 << 1),
	INSTANT_FLAG_EXCHANGE		= (1 << 2),
	INSTANT_FLAG_STUN			= (1 << 3),
	INSTANT_FLAG_NO_REWARD		= (1 << 4),
};

enum EAiFlags
{
	AI_FLAG_NPC 			= (1 << 0),
	AI_FLAG_AGGRESSIVE		= (1 << 1),
	AI_FLAG_HELPER			= (1 << 2),
	AI_FLAG_STAYZONE		= (1 << 3),
};

extern int32_t g_nPortalLimitTime;

enum
{
	MAIN_RACE_WARRIOR_M,
	MAIN_RACE_ASSASSIN_W,
	MAIN_RACE_SURA_M,
	MAIN_RACE_SHAMAN_W,
	MAIN_RACE_WARRIOR_W,
	MAIN_RACE_ASSASSIN_M,
	MAIN_RACE_SURA_W,
	MAIN_RACE_SHAMAN_M,
#ifdef ENABLE_WOLFMAN_CHARACTER
	MAIN_RACE_WOLFMAN_M,
#endif
	MAIN_RACE_MAX_NUM,
};

enum EOther
{
	POISON_LENGTH = 30,
#ifdef ENABLE_WOLFMAN_CHARACTER
	BLEEDING_LENGTH = 30,
#endif
	STAMINA_PER_STEP = 1,
	SAFEBOX_PAGE_SIZE = 9,
	AI_CHANGE_ATTACK_POISITION_TIME_NEAR = 10000,
	AI_CHANGE_ATTACK_POISITION_TIME_FAR = 1000,
	AI_CHANGE_ATTACK_POISITION_DISTANCE = 100,
	SUMMON_MONSTER_COUNT = 3,
};

enum FlyTypes
{
	FLY_NONE,
	FLY_EXP,
	FLY_HP_MEDIUM,
	FLY_HP_BIG,
	FLY_SP_SMALL,
	FLY_SP_MEDIUM,
	FLY_SP_BIG,
	FLY_FIREWORK1,
	FLY_FIREWORK2,
	FLY_FIREWORK3,
	FLY_FIREWORK4,
	FLY_FIREWORK5,
	FLY_FIREWORK6,
	FLY_FIREWORK_CHRISTMAS,
	FLY_CHAIN_LIGHTNING,
	FLY_HP_SMALL,
	FLY_SKILL_MUYEONG,
};

enum EDamageType
{
	DAMAGE_TYPE_NONE,
	DAMAGE_TYPE_NORMAL,
	DAMAGE_TYPE_NORMAL_RANGE,
	//��ų
	DAMAGE_TYPE_MELEE,
	DAMAGE_TYPE_RANGE,
	DAMAGE_TYPE_FIRE,
	DAMAGE_TYPE_ICE,
	DAMAGE_TYPE_ELEC,
	DAMAGE_TYPE_MAGIC,
	DAMAGE_TYPE_POISON,
	DAMAGE_TYPE_SPECIAL,
#ifdef ENABLE_WOLFMAN_CHARACTER
	DAMAGE_TYPE_BLEEDING,
#endif
	DAMAGE_TYPE_MAX_NUM,
};

enum DamageFlag
{
	DAMAGE_NORMAL	= (1 << 0),
	DAMAGE_POISON	= (1 << 1),
	DAMAGE_DODGE	= (1 << 2),
	DAMAGE_BLOCK	= (1 << 3),
	DAMAGE_PENETRATE= (1 << 4),
	DAMAGE_CRITICAL = (1 << 5),
#if defined(ENABLE_WOLFMAN_CHARACTER) && !defined(USE_MOB_BLEEDING_AS_POISON)
	DAMAGE_BLEEDING	= (1 << 6),
#endif
};

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
	POINT_STAMINA,              // 9  ���׹̳�
	POINT_MAX_STAMINA,          // 10 �ִ� ���׹̳�

	POINT_GOLD,                 // 11
	POINT_ST,                   // 12 �ٷ�
	POINT_HT,                   // 13 ü��
	POINT_DX,                   // 14 ��ø��
	POINT_IQ,                   // 15 ���ŷ�
	POINT_DEF_GRADE,		// 16 ...
	POINT_ATT_SPEED,            // 17 ���ݼӵ�
	POINT_ATT_GRADE,		// 18 ���ݷ� MAX
	POINT_MOV_SPEED,            // 19 �̵��ӵ�
	POINT_CLIENT_DEF_GRADE,	// 20 �����
	POINT_CASTING_SPEED,        // 21 �ֹ��ӵ� (��ٿ�Ÿ��*100) / (100 + �̰�) = ���� ��ٿ� Ÿ��
	POINT_MAGIC_ATT_GRADE,      // 22 �������ݷ�
	POINT_MAGIC_DEF_GRADE,      // 23 ��������
	POINT_EMPIRE_POINT,         // 24 ��������
	POINT_LEVEL_STEP,           // 25 �� ���������� �ܰ�.. (1 2 3 �� �� ����, 4 �Ǹ� ���� ��)
	POINT_STAT,                 // 26 �ɷ�ġ �ø� �� �ִ� ����
	POINT_SUB_SKILL,		// 27 ���� ��ų ����Ʈ
	POINT_SKILL,		// 28 ��Ƽ�� ��ų ����Ʈ
	POINT_WEAPON_MIN,		// 29 ���� �ּ� ������
	POINT_WEAPON_MAX,		// 30 ���� �ִ� ������
	POINT_PLAYTIME,             // 31 �÷��̽ð�
	POINT_HP_REGEN,             // 32 HP ȸ����
	POINT_SP_REGEN,             // 33 SP ȸ����

	POINT_BOW_DISTANCE,         // 34 Ȱ �����Ÿ� ����ġ (meter)

	POINT_HP_RECOVERY,          // 35 ü�� ȸ�� ������
	POINT_SP_RECOVERY,          // 36 ���ŷ� ȸ�� ������

	POINT_POISON_PCT,           // 37 �� Ȯ��
	POINT_STUN_PCT,             // 38 ���� Ȯ��
	POINT_SLOW_PCT,             // 39 ���ο� Ȯ��
	POINT_CRITICAL_PCT,         // 40 ũ��Ƽ�� Ȯ��
	POINT_PENETRATE_PCT,        // 41 ����Ÿ�� Ȯ��
	POINT_CURSE_PCT,            // 42 ���� Ȯ��

	POINT_ATTBONUS_HUMAN,       // 43 �ΰ����� ����
	POINT_ATTBONUS_ANIMAL,      // 44 �������� ������ % ����
	POINT_ATTBONUS_ORC,         // 45 ���Ϳ��� ������ % ����
	POINT_ATTBONUS_MILGYO,      // 46 �б����� ������ % ����
	POINT_ATTBONUS_UNDEAD,      // 47 ��ü���� ������ % ����
	POINT_ATTBONUS_DEVIL,       // 48 ����(�Ǹ�)���� ������ % ����
	POINT_ATTBONUS_INSECT,      // 49 ������
	POINT_ATTBONUS_FIRE,        // 50 ȭ����
	POINT_ATTBONUS_ICE,         // 51 ������
	POINT_ATTBONUS_DESERT,      // 52 �縷��
	POINT_ATTBONUS_MONSTER,     // 53 ��� ���Ϳ��� ����
	POINT_ATTBONUS_WARRIOR,     // 54 ���翡�� ����
	POINT_ATTBONUS_ASSASSIN,	// 55 �ڰ����� ����
	POINT_ATTBONUS_SURA,		// 56 ���󿡰� ����
	POINT_ATTBONUS_SHAMAN,		// 57 ���翡�� ����
	POINT_ATTBONUS_TREE,     	// 58 �������� ���� 20050729.myevan UNUSED5 

	POINT_RESIST_WARRIOR,		// 59 ���翡�� ����
	POINT_RESIST_ASSASSIN,		// 60 �ڰ����� ����
	POINT_RESIST_SURA,			// 61 ���󿡰� ����
	POINT_RESIST_SHAMAN,		// 62 ���翡�� ����

	POINT_STEAL_HP,             // 63 ����� ���
	POINT_STEAL_SP,             // 64 ���ŷ� ���

	POINT_MANA_BURN_PCT,        // 65 ���� ��

	/// ���ؽ� ���ʽ� ///

	POINT_DAMAGE_SP_RECOVER,    // 66 ���ݴ��� �� ���ŷ� ȸ�� Ȯ��

	POINT_BLOCK,                // 67 ����
	POINT_DODGE,                // 68 ȸ����

	POINT_RESIST_SWORD,         // 69
	POINT_RESIST_TWOHAND,       // 70
	POINT_RESIST_DAGGER,        // 71
	POINT_RESIST_BELL,          // 72
	POINT_RESIST_FAN,           // 73
	POINT_RESIST_BOW,           // 74  ȭ��   ����   : ����� ����
	POINT_RESIST_FIRE,          // 75  ȭ��   ����   : ȭ�����ݿ� ���� ����� ����
	POINT_RESIST_ELEC,          // 76  ����   ����   : ������ݿ� ���� ����� ����
	POINT_RESIST_MAGIC,         // 77  ����   ����   : �������� ���� ����� ����
	POINT_RESIST_WIND,          // 78  �ٶ�   ����   : �ٶ����ݿ� ���� ����� ����

	POINT_REFLECT_MELEE,        // 79 ���� �ݻ�

	/// Ư�� ���ؽ� ///
	POINT_REFLECT_CURSE,		// 80 ���� �ݻ�
	POINT_POISON_REDUCE,		// 81 �������� ����

	/// �� �Ҹ�� ///
	POINT_KILL_SP_RECOVER,		// 82 �� �Ҹ�� MP ȸ��
	POINT_EXP_DOUBLE_BONUS,		// 83
	POINT_GOLD_DOUBLE_BONUS,		// 84
	POINT_ITEM_DROP_BONUS,		// 85

	/// ȸ�� ���� ///
	POINT_POTION_BONUS,			// 86
	POINT_KILL_HP_RECOVERY,		// 87

	POINT_IMMUNE_STUN,			// 88
	POINT_IMMUNE_SLOW,			// 89
	POINT_IMMUNE_FALL,			// 90
	//////////////////

	POINT_PARTY_ATTACKER_BONUS,		// 91
	POINT_PARTY_TANKER_BONUS,		// 92

	POINT_ATT_BONUS,			// 93
	POINT_DEF_BONUS,			// 94

	POINT_ATT_GRADE_BONUS,		// 95
	POINT_DEF_GRADE_BONUS,		// 96
	POINT_MAGIC_ATT_GRADE_BONUS,	// 97
	POINT_MAGIC_DEF_GRADE_BONUS,	// 98

	POINT_RESIST_NORMAL_DAMAGE,		// 99

	POINT_HIT_HP_RECOVERY,		// 100
	POINT_HIT_SP_RECOVERY, 		// 101
	POINT_MANASHIELD,			// 102 ��ż�ȣ ��ų�� ���� �������� ȿ�� ����

	POINT_PARTY_BUFFER_BONUS,		// 103
	POINT_PARTY_SKILL_MASTER_BONUS,	// 104

	POINT_HP_RECOVER_CONTINUE,		// 105
	POINT_SP_RECOVER_CONTINUE,		// 106

	POINT_STEAL_GOLD,			// 107 
	POINT_POLYMORPH,			// 108 ������ ���� ��ȣ
	POINT_MOUNT,			// 109 Ÿ���ִ� ���� ��ȣ

	POINT_PARTY_HASTE_BONUS,		// 110
	POINT_PARTY_DEFENDER_BONUS,		// 111
	POINT_STAT_RESET_COUNT,		// 112 ���� �ܾ� ����� ���� ���� ���� ����Ʈ (1�� 1����Ʈ ���°���)

	POINT_HORSE_SKILL,			// 113

	POINT_MALL_ATTBONUS,		// 114 ���ݷ� +x%
	POINT_MALL_DEFBONUS,		// 115 ���� +x%
	POINT_MALL_EXPBONUS,		// 116 ����ġ +x%
	POINT_MALL_ITEMBONUS,		// 117 ������ ����� x/10��
	POINT_MALL_GOLDBONUS,		// 118 �� ����� x/10��

	POINT_MAX_HP_PCT,			// 119 �ִ����� +x%
	POINT_MAX_SP_PCT,			// 120 �ִ����ŷ� +x%

	POINT_SKILL_DAMAGE_BONUS,		// 121 ��ų ������ *(100+x)%
	POINT_NORMAL_HIT_DAMAGE_BONUS,	// 122 ��Ÿ ������ *(100+x)%

	// DEFEND_BONUS_ATTRIBUTES
	POINT_SKILL_DEFEND_BONUS,		// 123 ��ų ��� ������
	POINT_NORMAL_HIT_DEFEND_BONUS,	// 124 ��Ÿ ��� ������
	// END_OF_DEFEND_BONUS_ATTRIBUTES

	// PC_BANG_ITEM_ADD 
	POINT_PC_BANG_EXP_BONUS,		// 125 PC�� ���� ����ġ ���ʽ�
	POINT_PC_BANG_DROP_BONUS,		// 126 PC�� ���� ��ӷ� ���ʽ�
	// END_PC_BANG_ITEM_ADD
	POINT_RAMADAN_CANDY_BONUS_EXP,			// �󸶴� ���� ����ġ ������

	POINT_ENERGY = 128,					// 128 ���

	// ��� ui ��.
	// �������� ���� �ʱ⸸, Ŭ���̾�Ʈ���� ����� �� �ð��� POINT�� �����ϱ� ������ �̷��� �Ѵ�.
	// �� �β�����
	POINT_ENERGY_END_TIME = 129,					// 129 ��� ���� �ð�

	POINT_COSTUME_ATTR_BONUS = 130,
	POINT_MAGIC_ATT_BONUS_PER = 131,
	POINT_MELEE_MAGIC_ATT_BONUS_PER = 132,

	// �߰� �Ӽ� ����
	POINT_RESIST_ICE = 133,          //   �ñ� ����   : �������ݿ� ���� ����� ����
	POINT_RESIST_EARTH = 134,        //   ���� ����   : �������ݿ� ���� ����� ����
	POINT_RESIST_DARK = 135,         //   ��� ����   : �������ݿ� ���� ����� ����

	POINT_RESIST_CRITICAL = 136,		// ũ��Ƽ�� ����	: ����� ũ��Ƽ�� Ȯ���� ����
	POINT_RESIST_PENETRATE = 137,		// ����Ÿ�� ����	: ����� ����Ÿ�� Ȯ���� ����

#ifdef ENABLE_WOLFMAN_CHARACTER
	POINT_BLEEDING_REDUCE = 138,
	POINT_BLEEDING_PCT = 139,

	POINT_ATTBONUS_WOLFMAN = 140,				
	POINT_RESIST_WOLFMAN = 141,				
	POINT_RESIST_CLAW = 142,					
#endif

#ifdef ENABLE_ACCE_SYSTEM
	POINT_ACCEDRAIN_RATE = 143,
#endif
	POINT_RESIST_MAGIC_REDUCTION = 144,

	//POINT_MAX_NUM = 129	common/length.h
};

enum EPositions
{
	POS_DEAD,
	POS_SLEEPING,
	POS_RESTING,
	POS_SITTING,
	POS_FISHING,
	POS_FIGHTING,
	POS_MOUNTING,
	POS_STANDING
};

// <Factor> Dynamically evaluated CHARACTER* equivalent.
// Referring to SCharDeadEventInfo.
struct DynamicCharacterPtr {
	DynamicCharacterPtr() : is_pc(false), id(0) {}
	DynamicCharacterPtr(const DynamicCharacterPtr& o)
		: is_pc(o.is_pc), id(o.id) {}

	// Returns the LPCHARACTER found in CHARACTER_MANAGER.
	LPCHARACTER Get() const; 
	// Clears the current settings.
	void Reset() {
		is_pc = false;
		id = 0;
	}

	// Basic assignment operator.
	DynamicCharacterPtr& operator=(const DynamicCharacterPtr& rhs) {
		is_pc = rhs.is_pc;
		id = rhs.id;
		return *this;
	}
	// Supports assignment with LPCHARACTER type.
	DynamicCharacterPtr& operator=(LPCHARACTER character);
	// Supports type casting to LPCHARACTER.
	operator LPCHARACTER() const {
		return Get();
	}

	bool is_pc;
	uint32_t id;
};

/* �����ϴ� ������ */
typedef struct character_point
{
	int32_t			points[POINT_MAX_NUM];

	uint8_t			job;
	uint8_t			voice;

	uint8_t			level;
	uint32_t			exp;
	int32_t			gold;

	int32_t				hp;
	int32_t				sp;

	int32_t				iRandomHP;
	int32_t				iRandomSP;

	int32_t				stamina;

	uint8_t			skill_group;
} CHARACTER_POINT;

/* ������� �ʴ� ĳ���� ������ */
typedef struct character_point_instant
{
	int32_t			points[POINT_MAX_NUM];

	float			fRot;

	int32_t				iMaxHP;
	int32_t				iMaxSP;

	int32_t			position;

	int32_t			instant_flag;
	uint32_t			dwAIFlag;
	uint32_t			dwImmuneFlag;
	uint32_t			dwLastShoutPulse;

	uint16_t			parts[PART_MAX_NUM];

	LPITEM			pItems[INVENTORY_AND_EQUIP_SLOT_MAX];
	uint8_t			bItemGrid[INVENTORY_AND_EQUIP_SLOT_MAX];

	// ��ȥ�� �κ��丮.
	LPITEM			pDSItems[DRAGON_SOUL_INVENTORY_MAX_NUM];
	uint16_t			wDSItemGrid[DRAGON_SOUL_INVENTORY_MAX_NUM];

	// by mhh
	LPITEM			pCubeItems[CUBE_MAX_NUM];
	LPCHARACTER		pCubeNpc;
#ifdef ENABLE_ACCE_SYSTEM
	LPITEM				pAcceMaterials[ACCE_WINDOW_MAX_MATERIALS];
#endif
	LPCHARACTER			battle_victim;

	uint8_t			gm_level;

	uint8_t			bBasePart;	// ��� ��ȣ

	int32_t				iMaxStamina;

	uint8_t			bBlockMode;

	int32_t				iDragonSoulActiveDeck;
	LPENTITY		m_pDragonSoulRefineWindowOpener;
} CHARACTER_POINT_INSTANT;

#define TRIGGERPARAM		LPCHARACTER ch, LPCHARACTER causer

typedef struct trigger
{
	uint8_t	type;
	int32_t		(*func) (TRIGGERPARAM);
	int32_t	value;
} TRIGGER;

class CTrigger
{
	public:
		CTrigger() : bType(0), pFunc(nullptr)
		{
		}

		uint8_t	bType;
		int32_t	(*pFunc) (TRIGGERPARAM);
};

EVENTINFO(char_event_info)
{
	DynamicCharacterPtr ch;
};

struct TSkillUseInfo
{
	int32_t	    iHitCount;
	int32_t	    iMaxHitCount;
	int32_t	    iSplashCount;
	uint32_t   dwNextSkillUsableTime;
	int32_t	    iRange;
	bool    bUsed;
	uint32_t   dwVID;
	bool    isGrandMaster;

	std::unordered_map<VID, size_t> TargetVIDMap;

	TSkillUseInfo()
		: iHitCount(0), iMaxHitCount(0), iSplashCount(0), dwNextSkillUsableTime(0), iRange(0), bUsed(false),
		dwVID(0), isGrandMaster(false)
   	{}

	bool    HitOnce(uint32_t dwVnum = 0);

	bool    UseSkill(bool isGrandMaster, uint32_t vid, uint32_t dwCooltime, int32_t splashcount = 1, int32_t hitcount = -1, int32_t range = -1);
	uint32_t   GetMainTargetVID() const	{ return dwVID; }
	void    SetMainTargetVID(uint32_t vid) { dwVID=vid; }
	void    ResetHitCount() { if (iSplashCount) { iHitCount = iMaxHitCount; iSplashCount--; } }
};

typedef struct packet_party_update TPacketGCPartyUpdate;
class CExchange;
class CSkillProto;
class CParty;
class CDungeon;
class CWarMap;
class CAffect;
class CGuild;
class CSafebox;
class CArena;

class CShop;
typedef class CShop * LPSHOP;

class CMob;
class CMobInstance;
typedef struct SMobSkillInfo TMobSkillInfo;

//SKILL_POWER_BY_LEVEL
extern int32_t GetSkillPowerByLevelFromType(int32_t job, int32_t skillgroup, int32_t skilllevel);
//END_SKILL_POWER_BY_LEVEL

namespace marriage
{
	class WeddingMap;
}

class CHARACTER : public CEntity, public CFSM, public CHorseRider
{
	protected:
		//////////////////////////////////////////////////////////////////////////////////
		// Entity ����
		virtual void	EncodeInsertPacket(LPENTITY entity);
		virtual void	EncodeRemovePacket(LPENTITY entity);
		virtual void	PacketView(const void * data, int32_t bytes, LPENTITY except = nullptr);
		//////////////////////////////////////////////////////////////////////////////////

	public:
		LPCHARACTER			FindCharacterInView(const char * name, bool bFindPCOnly);
		void				UpdatePacket();

		//////////////////////////////////////////////////////////////////////////////////
		// FSM (Finite State Machine) ����
	protected:
		CStateTemplate<CHARACTER>	m_stateMove;
		CStateTemplate<CHARACTER>	m_stateBattle;
		CStateTemplate<CHARACTER>	m_stateIdle;

	public:
		virtual void		StateMove();
		virtual void		StateBattle();
		virtual void		StateIdle();
		virtual void		StateFlag();
		virtual void		StateFlagBase();
		void				StateHorse();

	protected:
		// STATE_IDLE_REFACTORING
		void				__StateIdle_Monster();
		void				__StateIdle_Stone();
		void				__StateIdle_NPC();
		// END_OF_STATE_IDLE_REFACTORING

	public:
		uint32_t GetAIFlag() const	{ return m_pointsInstant.dwAIFlag; }
	
		void				SetAggressive();
		bool				IsAggressive() const;

		void				SetCoward();
		bool				IsCoward() const;
		void				CowardEscape();

		void				SetNoAttackShinsu();
		bool				IsNoAttackShinsu() const;

		void				SetNoAttackChunjo();
		bool				IsNoAttackChunjo() const;

		void				SetNoAttackJinno();
		bool				IsNoAttackJinno() const;

		void				SetAttackMob();
		bool				IsAttackMob() const;

		void				SetNoMove();

		bool				CanNPCFollowTarget(LPCHARACTER pkTarget);

		virtual void			BeginStateEmpty();
		virtual void			EndStateEmpty() {}

		void				RestartAtSamePos();
		void				RestartAtPos(int32_t lX, int32_t lY);

	protected:
		uint32_t				m_dwStateDuration;
		//////////////////////////////////////////////////////////////////////////////////

	public:
		CHARACTER();
		virtual ~CHARACTER();

		void			Create(std::string stName, uint32_t vid, bool isPC);
		void			Destroy();

		void			Disconnect(const char * c_pszReason);

	protected:
		void			Initialize();

		//////////////////////////////////////////////////////////////////////////////////
		// Basic Points
	public:
		uint32_t			GetPlayerID() const	{ return m_dwPlayerID; }

		void			SetPlayerProto(const TPlayerTable * table);
		void			CreatePlayerProto(TPlayerTable & tab);	// ���� �� ���

		void			SetProto(const CMob * c_pkMob);
		uint16_t			GetRaceNum() const;

		void			Save();		// DelayedSave
		void			SaveReal();	// ���� ����
		void			FlushDelayedSaveItem();

		const char *	GetName() const;
		std::string	GetStringName() const;
		const char* GetProtoName() const;

		const VID &		GetVID() const		{ return m_vid;		}

		void			SetName(const std::string& name) { m_stName = name; }

		void			SetRace(uint8_t race);
		bool			ChangeSex();

		uint32_t			GetAID() const;
		int32_t				GetChangeEmpireCount() const;
		void			SetChangeEmpireCount();
		int32_t				ChangeEmpire(uint8_t empire);

		uint8_t			GetJob() const;
		uint8_t			GetCharType() const;

		bool			IsPC() const		{ return GetDesc() ? true : false; }
		bool			IsNPC()	const		{ return m_bCharType != CHAR_TYPE_PC; }
		bool			IsMonster()	const	{ return m_bCharType == CHAR_TYPE_MONSTER; }
		bool			IsStone() const		{ return m_bCharType == CHAR_TYPE_STONE; }
		bool			IsDoor() const		{ return m_bCharType == CHAR_TYPE_DOOR; } 
		bool			IsBuilding() const	{ return m_bCharType == CHAR_TYPE_BUILDING;  }
		bool			IsWarp() const		{ return m_bCharType == CHAR_TYPE_WARP; }
		bool			IsGoto() const		{ return m_bCharType == CHAR_TYPE_GOTO; }
//		bool			IsPet() const		{ return m_bCharType == CHAR_TYPE_PET; }

		uint32_t			GetLastShoutPulse() const	{ return m_pointsInstant.dwLastShoutPulse; }
		void			SetLastShoutPulse(uint32_t pulse) { m_pointsInstant.dwLastShoutPulse = pulse; }
		int32_t				GetLevel() const		{ return m_points.level;	}
		void			SetLevel(uint8_t level);

		uint8_t			GetGMLevel(bool bIgnoreTestServer = false) const;
		BOOL 			IsGM() const;
		void			SetGMLevel(); 

		bool			IsGMInvisible() const { return m_bGMInvisible; }
		bool			IsGMInvisibleChanged() const { return m_bGMInvisibleChanged; }
		void			SetGMInvisible(bool bActive);
		void			ResetGMInvisibleChanged() { m_bGMInvisibleChanged = false; }

		int32_t				GetChannel() const;

		uint32_t			GetExp() const		{ return m_points.exp;	}
		void			SetExp(uint32_t exp)	{ m_points.exp = exp;	}
		uint32_t			GetNextExp() const;
		LPCHARACTER		DistributeExp();	// ���� ���� ���� ����� �����Ѵ�.
		void			DistributeHP(LPCHARACTER pkKiller);
		void			DistributeSP(LPCHARACTER pkKiller, int32_t iMethod=0);
		bool			CanFall();
#ifdef __ENABLE_KILL_EVENT_FIX__
		LPCHARACTER		GetMostAttacked();
#endif

		void			SetPosition(int32_t pos);
		bool			IsPosition(int32_t pos) const	{ return m_pointsInstant.position == pos ? true : false; }
		int32_t				GetPosition() const		{ return m_pointsInstant.position; }

		void			SetPart(uint8_t bPartPos, uint16_t wVal);
		uint16_t			GetPart(uint8_t bPartPos) const;
		uint16_t			GetOriginalPart(uint8_t bPartPos) const;

		void			SetHP(int32_t hp)		{ m_points.hp = hp; }
		int32_t				GetHP() const		{ return m_points.hp; }

		void			SetSP(int32_t sp)		{ m_points.sp = sp; }
		int32_t				GetSP() const		{ return m_points.sp; }

		void			SetStamina(int32_t stamina)	{ m_points.stamina = stamina; }
		int32_t				GetStamina() const		{ return m_points.stamina; }

		void			SetMaxHP(int32_t iVal)	{ m_pointsInstant.iMaxHP = iVal; }
		int32_t				GetMaxHP() const	{ return m_pointsInstant.iMaxHP; }

		void			SetMaxSP(int32_t iVal)	{ m_pointsInstant.iMaxSP = iVal; }
		int32_t				GetMaxSP() const	{ return m_pointsInstant.iMaxSP; }

		void			SetMaxStamina(int32_t iVal)	{ m_pointsInstant.iMaxStamina = iVal; }
		int32_t				GetMaxStamina() const	{ return m_pointsInstant.iMaxStamina; }

		void			SetRandomHP(int32_t v)	{ m_points.iRandomHP = v; }
		void			SetRandomSP(int32_t v)	{ m_points.iRandomSP = v; }

		int32_t				GetRandomHP() const	{ return m_points.iRandomHP; }
		int32_t				GetRandomSP() const	{ return m_points.iRandomSP; }

		int32_t				GetHPPct() const;

		void			SetRealPoint(uint8_t idx, int32_t val);
		int32_t				GetRealPoint(uint8_t idx) const;

		void			SetPoint(uint8_t idx, int32_t val);
		int32_t				GetPoint(uint8_t idx) const;
		int32_t				GetConvPoint(uint8_t idx) const;
		int32_t				GetLimitPoint(uint8_t idx) const;
		int32_t				GetPolymorphPoint(uint8_t idx) const;

		const TMobTable &	GetMobTable() const;
		uint8_t				GetMobRank() const;
		uint8_t				GetMobBattleType() const;
		uint8_t				GetMobSize() const;
		uint32_t				GetMobDamageMin() const;
		uint32_t				GetMobDamageMax() const;
		uint16_t				GetMobAttackRange() const;
		uint32_t				GetMobDropItemVnum() const;
		float				GetMobDamageMultiply() const;

		// NEWAI
		bool			IsBerserker() const;
		bool			IsBerserk() const;
		void			SetBerserk(bool mode);

		bool			IsStoneSkinner() const;

		bool			IsGodSpeeder() const;
		bool			IsGodSpeed() const;
		void			SetGodSpeed(bool mode);

		bool			IsDeathBlower() const;
		bool			IsDeathBlow() const;

		bool			IsReviver() const;
		bool			HasReviverInParty() const;
		bool			IsRevive() const;
		void			SetRevive(bool mode);
		// NEWAI END

		bool			IsRaceFlag(uint32_t dwBit) const;
		bool			IsSummonMonster() const;
		uint32_t			GetSummonVnum() const;

		uint32_t			GetPolymorphItemVnum() const;
		uint32_t			GetMonsterDrainSPPoint() const;

		void			MainCharacterPacket();	// ���� ����ĳ���Ͷ�� �����ش�.

		void			ComputePoints();
		void			ComputeBattlePoints();
		void			PointChange(uint8_t type, int32_t amount, bool bAmount = false, bool bBroadcast = false);
		void			PointsPacket();
		void			ApplyPoint(uint8_t bApplyType, int32_t iVal);
		void			CheckMaximumPoints();	// HP, SP ���� ���� ���� �ִ밪 ���� ������ �˻��ϰ� ���ٸ� �����.

		bool			Show(int32_t lMapIndex, int32_t x, int32_t y, int32_t z = INT_MAX, bool bShowSpawnMotion = false);

		void			Sitdown(int32_t is_ground);
		void			Standup();

		void			SetRotation(float fRot);
		void			SetRotationToXY(int32_t x, int32_t y);
		float			GetRotation() const	{ return m_pointsInstant.fRot; }

		void			MotionPacketEncode(uint8_t motion, LPCHARACTER victim, SPacketGCMotion* packet);
		void			Motion(uint8_t motion, LPCHARACTER victim = nullptr);

		void			ChatPacket(uint8_t type, const char *format, ...);
		void			MonsterChat(uint8_t bMonsterChatType);

		void			ResetPoint(int32_t iLv);

		void			SetBlockMode(uint8_t bFlag);
		void			SetBlockModeForce(uint8_t bFlag);
		bool			IsBlockMode(uint8_t bFlag) const	{ return (m_pointsInstant.bBlockMode & bFlag)?true:false; }

		bool			IsPolymorphed() const		{ return m_dwPolymorphRace>0; }
		bool			IsPolyMaintainStat() const	{ return m_bPolyMaintainStat; } // ���� ������ �����ϴ� ��������.
		void			SetPolymorph(uint32_t dwRaceNum, bool bMaintainStat = false);
		uint32_t			GetPolymorphVnum() const	{ return m_dwPolymorphRace; }
		int32_t				GetPolymorphPower() const;

		void			StartAnticheatCommunication();
	    void            StartCheckWallhack();

		// FISING	
		bool            IsNearWater() const;
		void			fishing();
		void			fishing_take();
		bool 			IsFishing() const { return m_pkFishingEvent ? true : false; }
		// END_OF_FISHING

		// MINING
		void			mining(LPCHARACTER chLoad);
		void			mining_cancel();
		void			mining_take();
		bool 			IsMining() const { return m_pkMiningEvent ? true : false; }
		// END_OF_MINING

		void			ResetPlayTime(uint32_t dwTimeRemain = 0);

		void			CreateFly(uint8_t bType, LPCHARACTER pkVictim);

		void			ResetChatCounter();
		uint8_t			IncreaseChatCounter();
		uint8_t			GetChatCounter() const;

		bool			IsExpBlocked() { return block_exp; }
		void			BlockExp() { block_exp = true; }
		void			UnblockExp() { block_exp = false; }

	protected:
		uint32_t			m_dwPolymorphRace;
		bool			m_bPolyMaintainStat;
		uint32_t			m_dwLoginPlayTime;
		uint32_t			m_dwPlayerID;
		VID				m_vid;
		std::string		m_stName;
		uint8_t			m_bCharType;

		CHARACTER_POINT		m_points;
		CHARACTER_POINT_INSTANT	m_pointsInstant;

		GPOS			m_lastMoveAblePos;
		int32_t			m_lastMoveableMapIndex;
        
		LPEVENT			m_pkAnticheatEvent;
		LPEVENT         m_pkCheckWallHackEvent;

		bool			block_exp;

		int32_t				m_iMoveCount;
		uint32_t			m_dwPlayStartTime;
		uint8_t			m_bAddChrState;
		bool			m_bSkipSave;
		uint8_t			m_bChatCounter;

		bool			m_bGMInvisible;
		bool			m_bGMInvisibleChanged;

		// End of Basic Points

		//////////////////////////////////////////////////////////////////////////////////
		// Move & Synchronize Positions
		//////////////////////////////////////////////////////////////////////////////////
	public:
		bool			IsStateMove() const			{ return IsState((CState&)m_stateMove); }
		bool			IsStateIdle() const			{ return IsState((CState&)m_stateIdle); }
		bool			IsWalking() const			{ return m_bNowWalking || GetStamina()<=0; }
		void			SetWalking(bool bWalkFlag)	{ m_bWalking=bWalkFlag; }
		void			SetNowWalking(bool bWalkFlag);	
		void			ResetWalking()			{ SetNowWalking(m_bWalking); }

		void			SetMovingWay(const TNPCMovingPosition* pWay, int32_t iMaxNum, bool bRepeat = false, bool bLocal = false);
		bool			DoMovingWay();
		bool			Goto(int32_t x, int32_t y);	// �ٷ� �̵� ��Ű�� �ʰ� ��ǥ ��ġ�� BLENDING ��Ų��.
		void			Stop();

		bool			CanMove() const;		// �̵��� �� �ִ°�?

		void			SyncPacket();
		bool			Sync(int32_t x, int32_t y);	// ���� �� �޼ҵ�� �̵� �Ѵ� (�� �� ���ǿ� ���� �̵� �Ұ��� ����)
		bool			Move(int32_t x, int32_t y);	// ������ �˻��ϰ� Sync �޼ҵ带 ���� �̵� �Ѵ�.
		void			OnMove(bool bIsAttack = false);	// �����϶� �Ҹ���. Move() �޼ҵ� �̿ܿ����� �Ҹ� �� �ִ�.
		uint32_t			GetMotionMode() const;
		float			GetMoveMotionSpeed() const;
		float			GetMoveSpeed() const;
		void			CalculateMoveDuration();
		void			SendMovePacket(uint8_t bFunc, uint8_t bArg, uint32_t x, uint32_t y, uint32_t dwDuration, uint32_t dwTime=0, float rot = -1.0f);
		uint32_t			GetCurrentMoveDuration() const	{ return m_dwMoveDuration; }
		uint32_t			GetWalkStartTime() const	{ return m_dwWalkStartTime; }
		uint32_t			GetLastMoveTime() const		{ return m_dwLastMoveTime; }
		uint32_t			GetLastAttackTime() const	{ return m_dwLastAttackTime; }

		void			SetLastAttacked(uint32_t time);	// ���������� ���ݹ��� �ð� �� ��ġ�� ������

		bool			SetSyncOwner(LPCHARACTER ch, bool bRemoveFromList = true);
		bool			IsSyncOwner(LPCHARACTER ch) const;

		bool			WarpSet(int32_t x, int32_t y, int32_t lRealMapIndex = 0);
		void			SetWarpLocation(int32_t lMapIndex, int32_t x, int32_t y);
		void			GetExitLocation(int32_t& lMapIndex, int32_t& x, int32_t& y);
		void			WarpEnd();
		const GPOS & GetWarpPosition() const { return m_posWarp; }
		bool			WarpToPID(uint32_t dwPID);

		void			SaveExitLocation();
		void			ExitToSavedLocation();

		void			StartStaminaConsume();
		void			StopStaminaConsume();
		bool			IsStaminaConsume() const;
		bool			IsStaminaHalfConsume() const;

		void			ResetStopTime();
		uint32_t			GetStopTime() const;

        const GPOS& GetLastMoveAblePosition(int32_t index = 0)
        {
            return m_lastMoveAblePos;
        }

        void SetLastMoveAblePosition(const GPOS& lastPos)
        {
            m_lastMoveAblePos = lastPos;
        }

        void SetLastMoveableMapIndex()
        {
            m_lastMoveableMapIndex = GetMapIndex();
        }

        int32_t GetLastMoveableMapIndex()
        {
            return m_lastMoveableMapIndex;
        }

	protected:
		void			ClearSync();

		float			m_fSyncTime;
		LPCHARACTER		m_pkChrSyncOwner;
		CHARACTER_LIST	m_kLst_pkChrSyncOwned;	// ���� SyncOwner�� �ڵ�

		const TNPCMovingPosition*	m_pMovingWay;
		int32_t							m_iMovingWayIndex;
		int32_t							m_iMovingWayMaxNum;
		bool						m_bMovingWayRepeat;
		int32_t						m_lMovingWayBaseX;
		int32_t						m_lMovingWayBaseY;

		GPOS	m_posDest;
		GPOS	m_posStart;
		GPOS	m_posWarp;
		int32_t			m_lWarpMapIndex;

		GPOS	m_posExit;
		int32_t			m_lExitMapIndex;

		uint32_t			m_dwMoveStartTime;
		uint32_t			m_dwMoveDuration;

		uint32_t			m_dwLastMoveTime;
		uint32_t			m_dwLastAttackTime;
		uint32_t			m_dwWalkStartTime;
		uint32_t			m_dwStopTime;

		bool			m_bWalking;
		bool			m_bNowWalking;
		bool			m_bStaminaConsume;
		// End

		// Quickslot ����
	public:
		void			SyncQuickslot(uint8_t bType, uint8_t bOldPos, uint8_t bNewPos);
		bool			GetQuickslot(uint8_t pos, TQuickSlot ** ppSlot);
		bool			SetQuickslot(uint8_t pos, TQuickSlot& rSlot);
		bool			DelQuickslot(uint8_t pos);
		bool			SwapQuickslot(uint8_t a, uint8_t b);
		void			ChainQuickslotItem(LPITEM pItem, uint8_t bType, uint8_t bOldPos);

	protected:
		TQuickSlot		m_quickslot[QUICKSLOT_MAX_NUM];

		////////////////////////////////////////////////////////////////////////////////////////
		// Affect
	public:
		void			StartAffectEvent();
		void			ClearAffect(bool bSave=false);
		void			ComputeAffect(CAffect * pkAff, bool bAdd);
		bool			AddAffect(uint32_t dwType, uint8_t bApplyOn, int32_t lApplyValue, uint32_t dwFlag, int32_t lDuration, int32_t lSPCost, bool bOverride, bool IsCube = false);
		void			RefreshAffect();
		bool			RemoveAffect(uint32_t dwType);
		bool			IsAffectFlag(uint32_t dwAff) const;

		bool			UpdateAffect();	// called from EVENT
		int32_t				ProcessAffect();

		void			LoadAffect(uint32_t dwCount, TPacketAffectElement * pElements);
		void			SaveAffect();

		// Affect loading�� ���� �����ΰ�?
		bool			IsLoadedAffect() const	{ return m_bIsLoadedAffect; }		

		bool			IsGoodAffect(uint8_t bAffectType) const;

		void			RemoveGoodAffect();
		void			RemoveBadAffect();

		CAffect *		FindAffect(uint32_t dwType, uint8_t bApply=APPLY_NONE) const;
		const std::list<CAffect *> & GetAffectContainer() const	{ return m_list_pkAffect; }
		bool			RemoveAffect(CAffect * pkAff);

	protected:
		bool			m_bIsLoadedAffect;
		TAffectFlag		m_afAffectFlag;
		std::list<CAffect *>	m_list_pkAffect;

	public:
		// PARTY_JOIN_BUG_FIX
		void			SetParty(LPPARTY pkParty);
		LPPARTY			GetParty() const	{ return m_pkParty; }

		bool			RequestToParty(LPCHARACTER leader);
		void			DenyToParty(LPCHARACTER member);
		void			AcceptToParty(LPCHARACTER member);

		/// �ڽ��� ��Ƽ�� �ٸ� character �� �ʴ��Ѵ�.
		/**
		 * @param	pchInvitee �ʴ��� ��� character. ��Ƽ�� ���� ������ �����̾�� �Ѵ�.
		 *
		 * ���� character �� ���°� ��Ƽ�� �ʴ��ϰ� �ʴ���� �� �ִ� ���°� �ƴ϶�� �ʴ��ϴ� ĳ���Ϳ��� �ش��ϴ� ä�� �޼����� �����Ѵ�.
		 */
		void			PartyInvite(LPCHARACTER pchInvitee);

		/// �ʴ��ߴ� character �� ������ ó���Ѵ�.
		/**
		 * @param	pchInvitee ��Ƽ�� ������ character. ��Ƽ�� ���������� �����̾�� �Ѵ�.
		 *
		 * pchInvitee �� ��Ƽ�� ������ �� �ִ� ��Ȳ�� �ƴ϶�� �ش��ϴ� ä�� �޼����� �����Ѵ�.
		 */
		void			PartyInviteAccept(LPCHARACTER pchInvitee);

		/// �ʴ��ߴ� character �� �ʴ� �źθ� ó���Ѵ�.
		/**
		 * @param [in]	dwPID �ʴ� �ߴ� character �� PID
		 */
		void			PartyInviteDeny(uint32_t dwPID);

		bool			BuildUpdatePartyPacket(SPacketGCPartyUpdate & out);
		int32_t				GetLeadershipSkillLevel() const;

		bool			CanSummon(int32_t iLeaderShip);

		void			SetPartyRequestEvent(LPEVENT pkEvent) { m_pkPartyRequestEvent = pkEvent; }

	protected:

		/// ��Ƽ�� �����Ѵ�.
		/**
		 * @param	pkLeader ������ ��Ƽ�� ����
		 */
		void			PartyJoin(LPCHARACTER pkLeader);

		/**
		 * ��Ƽ ������ �� �� ���� ����� �����ڵ�.
		 * Error code �� �ð��� �������ΰ��� ���� ���氡����(mutable) type �� ����(static) type ���� ������.
		 * Error code �� ���� PERR_SEPARATOR ���� ������ ���氡���� type �̰� ������ ���� type �̴�.
		 */
		enum PartyJoinErrCode {
			PERR_NONE		= 0,	///< ó������
			PERR_SERVER,			///< ���������� ��Ƽ���� ó�� �Ұ�
			PERR_DUNGEON,			///< ĳ���Ͱ� ������ ����
			PERR_OBSERVER,			///< ���������
			PERR_LVBOUNDARY,		///< ��� ĳ���Ϳ� �������̰� ��
			PERR_LOWLEVEL,			///< �����Ƽ�� �ְ������� 30���� ����
			PERR_HILEVEL,			///< �����Ƽ�� ������������ 30���� ����
			PERR_ALREADYJOIN,		///< ��Ƽ���� ��� ĳ���Ͱ� �̹� ��Ƽ��
			PERR_PARTYISFULL,		///< ��Ƽ�ο� ���� �ʰ�
			PERR_SEPARATOR,			///< Error type separator.
			PERR_DIFFEMPIRE,		///< ��� ĳ���Ϳ� �ٸ� ������
			PERR_MAX				///< Error code �ְ�ġ. �� �տ� Error code �� �߰��Ѵ�.
		};

		/// ��Ƽ �����̳� �Ἲ ������ ������ �˻��Ѵ�.
		/**
		 * @param 	pchLeader ��Ƽ�� leader �̰ų� �ʴ��� character
		 * @param	pchGuest �ʴ�޴� character
		 * @return	��� PartyJoinErrCode �� ��ȯ�� �� �ִ�.
		 */
		static PartyJoinErrCode	IsPartyJoinableCondition(const LPCHARACTER pchLeader, const LPCHARACTER pchGuest);

		/// ��Ƽ �����̳� �Ἲ ������ ������ ������ �˻��Ѵ�.
		/**
		 * @param 	pchLeader ��Ƽ�� leader �̰ų� �ʴ��� character
		 * @param	pchGuest �ʴ�޴� character
		 * @return	mutable type �� code �� ��ȯ�Ѵ�.
		 */
		static PartyJoinErrCode	IsPartyJoinableMutableCondition(const LPCHARACTER pchLeader, const LPCHARACTER pchGuest);

		LPPARTY			m_pkParty;
		uint32_t			m_dwLastDeadTime;
		LPEVENT			m_pkPartyRequestEvent;

		/**
		 * ��Ƽ��û Event map.
		 * key: �ʴ���� ĳ������ PID
		 * value: event�� pointer
		 *
		 * �ʴ��� ĳ���͵鿡 ���� event map.
		 */
		typedef std::map< uint32_t, LPEVENT >	EventMap;
		EventMap		m_PartyInviteEventMap;

		// END_OF_PARTY_JOIN_BUG_FIX

		////////////////////////////////////////////////////////////////////////////////////////
		// Dungeon
	public:
		void			SetDungeon(LPDUNGEON pkDungeon);
		LPDUNGEON		GetDungeon() const	{ return m_pkDungeon; }
		LPDUNGEON		GetDungeonForce() const;
	protected:
		LPDUNGEON	m_pkDungeon;
		int32_t			m_iEventAttr;

		////////////////////////////////////////////////////////////////////////////////////////
		// Guild
	public:
		void			SetGuild(CGuild * pGuild);
		CGuild*			GetGuild() const	{ return m_pGuild; }

		void			SetWarMap(CWarMap* pWarMap);
		CWarMap*		GetWarMap() const	{ return m_pWarMap; }

	protected:
		CGuild *		m_pGuild;
		uint32_t			m_dwUnderGuildWarInfoMessageTime;
		CWarMap *		m_pWarMap;

		////////////////////////////////////////////////////////////////////////////////////////
		// Item related
	public:
		bool			CanHandleItem(bool bSkipRefineCheck = false, bool bSkipObserver = false); // ������ ���� ������ �� �� �ִ°�?

		bool			IsItemLoaded() const	{ return m_bItemLoaded; }
		void			SetItemLoaded()	{ m_bItemLoaded = true; }

		void			ClearItem();
#ifdef ENABLE_HIGHLIGHT_NEW_ITEM
		void			SetItem(const TItemPos & Cell, LPITEM item, bool bWereMine = false);
#else
		void			SetItem(const TItemPos & Cell, LPITEM item);
#endif
		LPITEM			GetItem(const TItemPos & Cell) const;
		LPITEM			GetInventoryItem(uint16_t wCell) const;
		bool			IsEmptyItemGrid(const TItemPos & Cell, uint8_t size, int32_t iExceptionCell = -1) const;
		bool			IsEmptyItemGridSpecial(const TItemPos &Cell, uint8_t bSize, int32_t iExceptionCell, std::vector<uint16_t>& vec) const;

		void			SetWear(uint16_t bCell, LPITEM item);
		LPITEM			GetWear(uint16_t bCell) const;

		// MYSHOP_PRICE_LIST
		void			UseSilkBotary(void); 		/// ��� ������ �������� ���

		/// DB ĳ�÷� ���� �޾ƿ� �������� ����Ʈ�� �������� �����ϰ� ������ ������ ����� ó���Ѵ�.
		/**
		 * @param [in] p	�������� ����Ʈ ��Ŷ
		 *
		 * ������ �� ó�� ��� ������ ������ ��� �� UseSilkBotary ���� DB ĳ�÷� �������� ����Ʈ�� ��û�ϰ�
		 * ������� ������ �� �Լ����� ���� ��ܺ����� ����� ó���Ѵ�.
		 */
		void			UseSilkBotaryReal(const TPacketMyshopPricelistHeader* p);
		// END_OF_MYSHOP_PRICE_LIST

		bool			UseItemEx(LPITEM item, const TItemPos & DestCell);
		bool			UseItem(const TItemPos & Cell, const TItemPos & DestCell = NPOS);


		bool			IsInSafezone();

		// ADD_REFINE_BUILDING
		bool			IsRefineThroughGuild() const;
		CGuild *		GetRefineGuild() const;
		int32_t				ComputeRefineFee(int32_t iCost, int32_t iMultiply = 5) const;
		void			PayRefineFee(int32_t iTotalMoney);
		void			SetRefineNPC(LPCHARACTER ch);
		// END_OF_ADD_REFINE_BUILDING

		bool			RefineItem(LPITEM pkItem, LPITEM pkTarget);
		bool			DropItem(const TItemPos &Cell,  uint8_t bCount=0);
		bool			GiveRecallItem(LPITEM item);
		void			ProcessRecallItem(LPITEM item);

		//	void			PotionPacket(int32_t iPotionType);
		void			EffectPacket(int32_t enumEffectType);
		void			SpecificEffectPacket(const char* filename);

		// ADD_MONSTER_REFINE
		bool			DoRefine(LPITEM item, bool bMoneyOnly = false);
		// END_OF_ADD_MONSTER_REFINE

		bool			DoRefineWithScroll(LPITEM item);
		bool			RefineInformation(uint16_t bCell, uint8_t bType, int32_t iAdditionalCell = -1);

		void			SetRefineMode(int32_t iAdditionalCell = -1);
		void			ClearRefineMode();

		bool			GiveItem(LPCHARACTER victim, const TItemPos &Cell);
		bool			CanReceiveItem(LPCHARACTER from, LPITEM item) const;
		void			ReceiveItem(LPCHARACTER from, LPITEM item);
		bool			GiveItemFromSpecialItemGroup(uint32_t dwGroupNum, std::vector <uint32_t> &dwItemVnums, 
							std::vector <uint32_t> &dwItemCounts, std::vector <LPITEM> &item_gets, int32_t &count);

		bool			MoveItem(const TItemPos &pos, TItemPos change_pos, uint8_t num);
		bool			PickupItem(uint32_t vid);
		bool			EquipItem(LPITEM item, int32_t iCandidateCell = -1);
		bool			UnequipItem(LPITEM item);

		// ���� item�� ������ �� �ִ� �� Ȯ���ϰ�, �Ұ��� �ϴٸ� ĳ���Ϳ��� ������ �˷��ִ� �Լ�
		bool			CanEquipNow(const LPITEM item, const TItemPos& srcCell = NPOS, const TItemPos& destCell = NPOS);

		// �������� item�� ���� �� �ִ� �� Ȯ���ϰ�, �Ұ��� �ϴٸ� ĳ���Ϳ��� ������ �˷��ִ� �Լ�
		bool			CanUnequipNow(const LPITEM item, const TItemPos& srcCell = NPOS, const TItemPos& destCell = NPOS);

		bool			SwapItem(uint16_t bCell, uint16_t bDestCell);
		int32_t				GetEmptyInventoryCount(uint8_t size) const;
		LPITEM			AutoGiveItem(uint32_t dwItemVnum, uint8_t bCount=1, int32_t iRarePct = -1, bool bMsg = true);
		void			AutoGiveItem(LPITEM item, bool longOwnerShip = false);
		int32_t				GetEmptyDragonSoulInventoryWithExceptions(LPITEM pItem, std::vector<uint16_t>& vec /*= -1*/) const;
		
		int32_t				GetEmptyInventory(uint8_t size) const;
		int32_t				GetEmptyDragonSoulInventory(LPITEM pItem) const;
		void			CopyDragonSoulItemGrid(std::vector<uint16_t>& vDragonSoulItemGrid) const;

		int32_t				CountEmptyInventory() const;

		int32_t				CountSpecifyItem(uint32_t vnum) const;
		void			RemoveSpecifyItem(uint32_t vnum, uint32_t count = 1);
		LPITEM			FindSpecifyItem(uint32_t vnum) const;
		LPITEM			FindItemByID(uint32_t id) const;

		int32_t				CountSpecifyTypeItem(uint8_t type) const;
		void			RemoveSpecifyTypeItem(uint8_t type, uint32_t count = 1);

		bool			IsEquipUniqueItem(uint32_t dwItemVnum) const;

		// CHECK_UNIQUE_GROUP
		bool			IsEquipUniqueGroup(uint32_t dwGroupVnum) const;
		// END_OF_CHECK_UNIQUE_GROUP

		void			SendEquipment(LPCHARACTER ch);
		// End of Item

	protected:

		/// �� �����ۿ� ���� ���������� �����Ѵ�.
		/**
		 * @param [in]	dwItemVnum ������ vnum
		 * @param [in]	dwItemPrice ������ ����
		 */
		void			SendMyShopPriceListCmd(uint32_t dwItemVnum, uint32_t dwItemPrice);

		bool			m_bNoOpenedShop;	///< �̹� ���� �� ���λ����� �� ���� �ִ����� ����(������ ���� ���ٸ� true)

		bool			m_bItemLoaded;
		int32_t				m_iRefineAdditionalCell;
		bool			m_bUnderRefine;
		uint32_t			m_dwRefineNPCVID;

	public:
		////////////////////////////////////////////////////////////////////////////////////////
		// Money related
		INT				GetGold() const		{ return m_points.gold;	}
		void			SetGold(INT gold)	{ m_points.gold = gold;	}
		bool			DropGold(INT gold);
		INT				GetAllowedGold() const;
		void			GiveGold(INT iAmount);	// ��Ƽ�� ������ ��Ƽ �й�, �α� ���� ó��
		// End of Money

		////////////////////////////////////////////////////////////////////////////////////////
		// Shop related
	public:
		void			SetShop(LPSHOP pkShop);
		LPSHOP			GetShop() const { return m_pkShop; };
		void			ShopPacket(uint8_t bSubHeader);

		void			SetShopOwner(LPCHARACTER ch) { m_pkChrShopOwner = ch; };
		LPCHARACTER		GetShopOwner() const { return m_pkChrShopOwner; };

		void			OpenMyShop(const char * c_pszSign, TShopItemTable * pTable, uint8_t bItemCount);
		LPSHOP			GetMyShop() const { return m_pkMyShop; };
		void			CloseMyShop();

	protected:

		LPSHOP			m_pkShop;
		LPSHOP			m_pkMyShop;
		std::string		m_stShopSign;
		LPCHARACTER		m_pkChrShopOwner;
		// End of shop

		////////////////////////////////////////////////////////////////////////////////////////
		// Exchange related
	public:
		bool			ExchangeStart(LPCHARACTER victim);
		void			SetExchange(CExchange * pkExchange);
		CExchange *		GetExchange() const	{ return m_pkExchange;	}

	protected:
		CExchange *		m_pkExchange;
		// End of Exchange

		////////////////////////////////////////////////////////////////////////////////////////
		// Battle
	public:
		struct TBattleInfo
		{
			int32_t iTotalDamage;
			int32_t iAggro;

			TBattleInfo(int32_t iTot, int32_t iAggr)
				: iTotalDamage(iTot), iAggro(iAggr)
				{}
		};
		typedef std::map<VID, TBattleInfo>	TDamageMap;

		typedef struct SAttackLog
		{
			uint32_t	dwVID;
			uint32_t	dwTime;
		} AttackLog;

		bool				Damage(LPCHARACTER pAttacker, int32_t dam, EDamageType type = DAMAGE_TYPE_NORMAL);
		bool				__Profile__Damage(LPCHARACTER pAttacker, int32_t dam, EDamageType type = DAMAGE_TYPE_NORMAL);
		void				DeathPenalty(uint8_t bExpLossPercent);
		void				ReviveInvisible(int32_t iDur);

		bool				Attack(LPCHARACTER pkVictim, uint8_t bType = 0);
		bool				IsAlive() const		{ return m_pointsInstant.position == POS_DEAD ? false : true; }
		bool				CanFight() const;

		bool				CanBeginFight() const;
		void				BeginFight(LPCHARACTER pkVictim); // pkVictimr�� �ο�� �����Ѵ�. (��������, ������ �� �ֳ� üũ�Ϸ��� CanBeginFight�� ���)

		bool				CounterAttack(LPCHARACTER pkChr); // �ݰ��ϱ� (���͸� ���)

		bool				IsStun() const;
		void				Stun();
		bool				IsDead() const;
		void				Dead(LPCHARACTER pkKiller = nullptr, bool bImmediateDead=false);

		void				SetNoRewardFlag() { SET_BIT(m_pointsInstant.instant_flag, INSTANT_FLAG_NO_REWARD); }
		void				RemoveNoRewardFlag() { REMOVE_BIT(m_pointsInstant.instant_flag, INSTANT_FLAG_NO_REWARD); }
		bool				HasNoRewardFlag() const { return IS_SET(m_pointsInstant.instant_flag, INSTANT_FLAG_NO_REWARD); }
		void				Reward(bool bItemDrop);
		void				RewardGold(LPCHARACTER pkAttacker);

		bool				Shoot(uint8_t bType);
		void				FlyTarget(uint32_t dwTargetVID, int32_t x, int32_t y, uint8_t bHeader);

		void				ForgetMyAttacker();
		void				AggregateMonster();
		void				AttractRanger();
		void				PullMonster();

		int32_t					GetArrowAndBow(LPITEM * ppkBow, LPITEM * ppkArrow, int32_t iArrowCount = 1);
		void				UseArrow(LPITEM pkArrow, uint32_t dwArrowCount);

		void				AttackedByPoison(LPCHARACTER pkAttacker);
		void				RemovePoison();
#ifdef ENABLE_WOLFMAN_CHARACTER
		void				AttackedByBleeding(LPCHARACTER pkAttacker);
		void				RemoveBleeding();
#endif
		void				AttackedByFire(LPCHARACTER pkAttacker, int32_t amount, int32_t count);
		void				RemoveFire();

		void				UpdateAlignment(int32_t iAmount);
		int32_t					GetAlignment() const;

		//����ġ ��� 
		int32_t					GetRealAlignment() const;
		void				ShowAlignment(bool bShow);

		void				SetKillerMode(bool bOn);
		bool				IsKillerMode() const;
		void				UpdateKillerMode();

		uint8_t				GetPKMode() const;
		void				SetPKMode(uint8_t bPKMode);

		void				ItemDropPenalty(LPCHARACTER pkKiller);

		void				UpdateAggrPoint(LPCHARACTER ch, EDamageType type, int32_t dam);

		//
		// HACK
		// 
	public:
		void SetComboSequence(uint8_t seq);
		uint8_t GetComboSequence() const;

		void SetLastComboTime(uint32_t time);
		uint32_t GetLastComboTime() const;

		int32_t GetValidComboInterval() const;
		void SetValidComboInterval(int32_t interval);

		uint8_t GetComboIndex() const;

		void IncreaseComboHackCount(int32_t k = 1);
		void ResetComboHackCount();
		void SkipComboAttackByTime(int32_t interval);
		uint32_t GetSkipComboAttackByTime() const;

	protected:
		uint8_t m_bComboSequence;
		uint32_t m_dwLastComboTime;
		int32_t m_iValidComboInterval;
		uint8_t m_bComboIndex;
		int32_t m_iComboHackCount;
		uint32_t m_dwSkipComboAttackByTime;

	protected:
		void				UpdateAggrPointEx(LPCHARACTER ch, EDamageType type, int32_t dam, TBattleInfo & info);
		void				ChangeVictimByAggro(int32_t iNewAggro, LPCHARACTER pNewVictim);

		uint32_t				m_dwFlyTargetID;
		std::vector<uint32_t>	m_vec_dwFlyTargets;
		TDamageMap			m_map_kDamage;	// � ĳ���Ͱ� ������ �󸶸�ŭ�� �������� �־��°�?
//		AttackLog			m_kAttackLog;
		uint32_t				m_dwKillerPID;

		int32_t					m_iAlignment;		// Lawful/Chaotic value -200000 ~ 200000
		int32_t					m_iRealAlignment;
		int32_t					m_iKillerModePulse;
		uint8_t				m_bPKMode;

		// Aggro
		uint32_t				m_dwLastVictimSetTime;
		int32_t					m_iMaxAggro;
		// End of Battle

		// Stone
	public:
		void				SetStone(LPCHARACTER pkChrStone);
		void				ClearStone();
		void				DetermineDropMetinStone();
		uint32_t				GetDropMetinStoneVnum() const { return m_dwDropMetinStone; }
		uint8_t				GetDropMetinStonePct() const { return m_bDropMetinStonePct; }

	protected:
		LPCHARACTER			m_pkChrStone;		// ���� ������ ��
		CHARACTER_SET		m_set_pkChrSpawnedBy;	// ���� ������ ���
		uint32_t				m_dwDropMetinStone;
		uint8_t				m_bDropMetinStonePct;
		// End of Stone

	public:
		enum
		{
			SKILL_UP_BY_POINT,
			SKILL_UP_BY_BOOK,
			SKILL_UP_BY_TRAIN,

			// ADD_GRANDMASTER_SKILL
			SKILL_UP_BY_QUEST,
			// END_OF_ADD_GRANDMASTER_SKILL
		};

		void				SkillLevelPacket();
		void				SkillLevelUp(uint32_t dwVnum, uint8_t bMethod = SKILL_UP_BY_POINT);
		bool				SkillLevelDown(uint32_t dwVnum);
		// ADD_GRANDMASTER_SKILL
		bool				UseSkill(uint32_t dwVnum, LPCHARACTER pkVictim, bool bUseGrandMaster = true);
		void				ResetSkill();
		void				SetSkillLevel(uint32_t dwVnum, uint8_t bLev);
		int32_t					GetUsedSkillMasterType(uint32_t dwVnum);

		bool				IsLearnableSkill(uint32_t dwSkillVnum) const;
		// END_OF_ADD_GRANDMASTER_SKILL

		bool				CheckSkillHitCount(const uint8_t SkillID, const VID & dwTargetVID);
		bool				CanUseSkill(uint32_t dwSkillVnum) const;
		bool				IsUsableSkillMotion(uint32_t dwMotionIndex) const;
		int32_t					GetSkillLevel(uint32_t dwVnum) const;
		int32_t					GetSkillMasterType(uint32_t dwVnum) const;
		int32_t					GetSkillPower(uint32_t dwVnum, uint8_t bLevel = 0) const;

		time_t				GetSkillNextReadTime(uint32_t dwVnum) const;
		void				SetSkillNextReadTime(uint32_t dwVnum, time_t time);
		void				SkillLearnWaitMoreTimeMessage(uint32_t dwVnum);

		void				ComputePassiveSkill(uint32_t dwVnum);
		int32_t					ComputeSkill(uint32_t dwVnum, LPCHARACTER pkVictim, uint8_t bSkillLevel = 0);
#ifdef ENABLE_WOLFMAN_CHARACTER
		int32_t					ComputeSkillParty(uint32_t dwVnum, LPCHARACTER pkVictim, uint8_t bSkillLevel = 0);
#endif
		int32_t					ComputeSkillAtPosition(uint32_t dwVnum, const GPOS& posTarget, uint8_t bSkillLevel = 0);
		void				ComputeSkillPoints();

		void				SetSkillGroup(uint8_t bSkillGroup); 
		uint8_t				GetSkillGroup() const		{ return m_points.skill_group; }

		int32_t					ComputeCooltime(int32_t time);

		void				GiveRandomSkillBook();

		void				DisableCooltime();
		bool				LearnSkillByBook(uint32_t dwSkillVnum, uint8_t bProb = 0);
		bool				LearnGrandMasterSkill(uint32_t dwSkillVnum);

	private:
		bool				m_bDisableCooltime;
		uint32_t				m_dwLastSkillTime;	///< ���������� skill �� �� �ð�(millisecond).
		// End of Skill

		// MOB_SKILL
	public:
		bool				HasMobSkill() const;
		size_t				CountMobSkill() const;
		const TMobSkillInfo * GetMobSkill(uint32_t idx) const;
		bool				CanUseMobSkill(uint32_t idx) const;
		bool				UseMobSkill(uint32_t idx);
		void				ResetMobSkillCooltime();
	protected:
		uint32_t				m_adwMobSkillCooltime[MOB_SKILL_MAX_NUM];
		// END_OF_MOB_SKILL

		// for SKILL_MUYEONG
	public:
		void				StartMuyeongEvent();
		void				StopMuyeongEvent();

	private:
		LPEVENT				m_pkMuyeongEvent;

		// for SKILL_CHAIN lighting
	public:
		int32_t					GetChainLightningIndex() const { return m_iChainLightingIndex; }
		void				IncChainLightningIndex() { ++m_iChainLightingIndex; }
		void				AddChainLightningExcept(LPCHARACTER ch) { m_setExceptChainLighting.insert(ch); }
		void				ResetChainLightningIndex() { m_iChainLightingIndex = 0; m_setExceptChainLighting.clear(); }
		int32_t					GetChainLightningMaxCount() const;
		const CHARACTER_SET& GetChainLightingExcept() const { return m_setExceptChainLighting; }

	private:
		int32_t					m_iChainLightingIndex;
		CHARACTER_SET m_setExceptChainLighting;

		// for SKILL_EUNHYUNG
	public:
		void				SetAffectedEunhyung();
		void				ClearAffectedEunhyung() { m_dwAffectedEunhyungLevel = 0; }
		bool				GetAffectedEunhyung() const { return m_dwAffectedEunhyungLevel; }

	private:
		uint32_t				m_dwAffectedEunhyungLevel;

		//
		// Skill levels
		//
	public:
		const TPlayerSkill*	GetPlayerSkills() const { return m_pSkillLevels; }

	protected:
		TPlayerSkill*					m_pSkillLevels;
		std::unordered_map<uint8_t, int32_t>		m_SkillDamageBonus;
		std::map<int32_t, TSkillUseInfo>	m_SkillUseInfo;

		////////////////////////////////////////////////////////////////////////////////////////
		// AI related
	public:
		void			AssignTriggers(const TMobTable * table);
		LPCHARACTER		GetVictim() const;	// ������ ��� ����
		void			SetVictim(LPCHARACTER pkVictim);
		LPCHARACTER		GetNearestVictim(LPCHARACTER pkChr);
		LPCHARACTER		GetProtege() const;	// ��ȣ�ؾ� �� ��� ����

		bool			Follow(LPCHARACTER pkChr, float fMinimumDistance = 150.0f);
		bool			Return();
		bool			IsGuardNPC() const;
		bool			IsChangeAttackPosition(LPCHARACTER target) const;
		void			ResetChangeAttackPositionTime() { m_dwLastChangeAttackPositionTime = get_dword_time() - AI_CHANGE_ATTACK_POISITION_TIME_NEAR;}
		void			SetChangeAttackPositionTime() { m_dwLastChangeAttackPositionTime = get_dword_time();}

		bool			OnIdle();

		void			OnAttack(LPCHARACTER pkChrAttacker);
		void			OnClick(LPCHARACTER pkChrCauser);

		VID				m_kVIDVictim;

	protected:
		uint32_t			m_dwLastChangeAttackPositionTime;
		CTrigger		m_triggerOnClick;
		// End of AI

		////////////////////////////////////////////////////////////////////////////////////////
		// Target
	protected:
		LPCHARACTER				m_pkChrTarget;		// �� Ÿ��
		CHARACTER_SET	m_set_pkChrTargetedBy;	// ���� Ÿ������ ������ �ִ� �����

	public:
		void				SetTarget(LPCHARACTER pkChrTarget);
		void				SendTargetDrop();
		void				BroadcastTargetPacket();
		void				ClearTarget();
		void				CheckTarget();
		LPCHARACTER			GetTarget() const { return m_pkChrTarget; }

		////////////////////////////////////////////////////////////////////////////////////////
		// Safebox
	public:
		int32_t					GetSafeboxSize() const;
		void				QuerySafeboxSize();
		void				SetSafeboxSize(int32_t size);

		CSafebox *			GetSafebox() const;
		void				LoadSafebox(int32_t iSize, uint32_t dwGold, int32_t iItemCount, TPlayerItem * pItems);
		void				ChangeSafeboxSize(uint8_t bSize);
		void				CloseSafebox();

		/// â�� ���� ��û
		/**
		 * @param [in]	pszPassword 1�� �̻� 6�� ������ â�� ��й�ȣ
		 *
		 * DB �� â���⸦ ��û�Ѵ�.
		 * â��� �ߺ����� ���� ���ϸ�, �ֱ� â�� ���� �ð����� ���� 10�� �̳����� �� �� ���Ѵ�.
		 */
		void				ReqSafeboxLoad(const char* pszPassword);

		/// â�� ���� ��û�� ���
		/**
		 * ReqSafeboxLoad �� ȣ���ϰ� CloseSafebox ���� �ʾ��� �� �� �Լ��� ȣ���ϸ� â�� �� �� �ִ�.
		 * â������ ��û�� DB �������� ���������� �޾��� ��� �� �Լ��� ����ؼ� ��û�� �� �� �ְ� ���ش�.
		 */
		void				CancelSafeboxLoad( void ) { m_bOpeningSafebox = false; }

		void				SetMallLoadTime(int32_t t) { m_iMallLoadTime = t; }
		int32_t					GetMallLoadTime() const { return m_iMallLoadTime; }

		CSafebox *			GetMall() const;
		void				LoadMall(int32_t iItemCount, TPlayerItem * pItems);
		void				CloseMall();

		void				SetSafeboxOpenPosition();
		float				GetDistanceFromSafeboxOpen() const;

	protected:
		CSafebox *			m_pkSafebox;
		int32_t					m_iSafeboxSize;
		int32_t					m_iSafeboxLoadTime;
		bool				m_bOpeningSafebox;	///< â�� ���� ��û ���̰ų� �����ִ°� ����, true �� ��� �����û�̰ų� ��������.

		CSafebox *			m_pkMall;
		int32_t					m_iMallLoadTime;

		GPOS		m_posSafeboxOpen;

		////////////////////////////////////////////////////////////////////////////////////////

	public:
		void SetBattleground(CBattleground* pkBattleground, uint8_t nBGTeamID)
		{
			m_pkBattleground = pkBattleground;
			m_nBGTeamID = nBGTeamID;
			m_isBattlegroundEntity = true;
		};
		CBattleground*		GetBattleground() { return m_pkBattleground; };
		uint8_t				GetBattlegroundTeamID() { return m_nBGTeamID; };
		
		void				RegisterBattlegroundMovingWay(uint8_t step, int32_t x, int32_t y);
		void				MoveBattlegroundMinnion();
		bool				DoMoveBattlegroundMinnion();

		bool				HasBattlegroundMoveTarget() { return m_bBGHasMoveTarget && !m_bBGMoveCompleted; };
		LPCHARACTER			GetEnemyNexus() { return m_pkCharBattlegroundEnemyNexus; };

		void				SetBattlegroundProtege(LPCHARACTER pkChar) { m_pkCharBattlegroundProtege = pkChar; };
		void				SetEnemyNexus(LPCHARACTER pkChar) { m_pkCharBattlegroundEnemyNexus = pkChar; };
		
	private:
		CBattleground*		m_pkBattleground;
		uint8_t				m_nBGTeamID;

		GPOS				m_pkBattlegroundMobFirstMovePos;
		GPOS				m_pkBattlegroundMobSecondMovePos;
		GPOS				m_pkBattlegroundMobLastMovePos;

		GPOS				m_pkBattlegroundCurrentMovePos;
		uint8_t				m_nBGCurrentMovingWayIndex;

		int32_t				m_nBGMovingWayBaseXpos; 
		int32_t				m_nBGMovingWayBaseYpos; 
		bool				m_bBGMoveCompleted;
		bool				m_bBGHasMoveTarget;
		bool				m_bBGHasMovedFirstTime;

		LPCHARACTER			m_pkCharBattlegroundProtege;
		LPCHARACTER			m_pkCharBattlegroundEnemyNexus;

		////////////////////////////////////////////////////////////////////////////////////////
		// Mounting
	public:
		void				MountVnum(uint32_t vnum);
		uint32_t				GetMountVnum() const { return m_dwMountVnum; }
		uint32_t				GetLastMountTime() const { return m_dwMountTime; }

		bool				CanUseHorseSkill();

		// Horse
		virtual	void		SetHorseLevel(int32_t iLevel);

		virtual	bool		StartRiding();
		virtual	bool		StopRiding();

		virtual	uint32_t		GetMyHorseVnum() const;

		virtual	void		HorseDie();
		virtual bool		ReviveHorse();

		virtual void		SendHorseInfo();
		virtual	void		ClearHorseInfo();

		void				HorseSummon(bool bSummon, bool bFromFar = false, uint32_t dwVnum = 0);

		LPCHARACTER			GetHorse() const			{ return m_chHorse; }	 // ���� ��ȯ���� ��
		LPCHARACTER			GetRider() const; // rider on horse
		void				SetRider(LPCHARACTER ch);

		bool				IsRiding() const;

	public:
		CPetSystem*			GetPetSystem()				{ return m_petSystem; }

	protected:
		CPetSystem*			m_petSystem;

	public:

	protected:
		LPCHARACTER			m_chHorse;
		LPCHARACTER			m_chRider;

		uint32_t				m_dwMountVnum;
		uint32_t				m_dwMountTime;

		uint8_t				m_bSendHorseLevel;
		uint8_t				m_bSendHorseHealthGrade;
		uint8_t				m_bSendHorseStaminaGrade;


		////////////////////////////////////////////////////////////////////////////////////////
		// Empire

	public:
		void 				SetEmpire(uint8_t bEmpire);
		uint8_t				GetEmpire() const { return m_bEmpire; }

	protected:
		uint8_t				m_bEmpire;

		////////////////////////////////////////////////////////////////////////////////////////
		// Regen
	public:
		void				SetRegen(LPREGEN pkRegen);

	protected:
		GPOS			m_posRegen;
		float				m_fRegenAngle;
		LPREGEN				m_pkRegen;
		size_t				regen_id_; // to help dungeon regen identification
		// End of Regen

		////////////////////////////////////////////////////////////////////////////////////////
		// Resists & Proofs
	public:
		bool				CannotMoveByAffect() const;	// Ư�� ȿ���� ���� ������ �� ���� �����ΰ�?
		bool				IsImmune(uint32_t dwImmuneFlag);
		void				SetImmuneFlag(uint32_t dw) { m_pointsInstant.dwImmuneFlag = dw; }

	protected:
		void				ApplyMobAttribute(const TMobTable* table);
		// End of Resists & Proofs

		////////////////////////////////////////////////////////////////////////////////////////
		// QUEST
		// 
	public:
		void				SetQuestNPCID(uint32_t vid);
		uint32_t				GetQuestNPCID() const { return m_dwQuestNPCVID; }
		LPCHARACTER			GetQuestNPC() const;

		void				SetQuestItemPtr(LPITEM item);
		void				ClearQuestItemPtr();
		LPITEM				GetQuestItemPtr() const;

		void				SetQuestBy(uint32_t dwQuestVnum)	{ m_dwQuestByVnum = dwQuestVnum; }
		uint32_t				GetQuestBy() const			{ return m_dwQuestByVnum; }

		int32_t					GetQuestFlag(const std::string& flag) const;
		void				SetQuestFlag(const std::string& flag, int32_t value);

		void				ConfirmWithMsg(const char* szMsg, int32_t iTimeout, uint32_t dwRequestPID);

	private:
		uint32_t				m_dwQuestNPCVID;
		uint32_t				m_dwQuestByVnum;
		LPITEM				m_pQuestItem;

		// Events
	public:
		bool				StartStateMachine(int32_t iPulse = 1);
		void				StopStateMachine();
		void				UpdateStateMachine(uint32_t dwPulse);
		void				SetNextStatePulse(int32_t iPulseNext);

		// ĳ���� �ν��Ͻ� ������Ʈ �Լ�. ������ �̻��� ��ӱ����� CFSM::Update �Լ��� ȣ���ϰų� UpdateStateMachine �Լ��� ����ߴµ�, ������ ������Ʈ �Լ� �߰���.
		void				UpdateCharacter(uint32_t dwPulse);

	protected:
		uint32_t				m_dwNextStatePulse;

		// Marriage
	public:
		LPCHARACTER			GetMarryPartner() const;
		void				SetMarryPartner(LPCHARACTER ch);
		int32_t					GetMarriageBonus(uint32_t dwItemVnum, bool bSum = true);

		void				SetWeddingMap(marriage::WeddingMap* pMap);
		marriage::WeddingMap* GetWeddingMap() const { return m_pWeddingMap; }

	private:
		marriage::WeddingMap* m_pWeddingMap;
		LPCHARACTER			m_pkChrMarried;

		// Warp Character
	public:
		void				StartWarpNPCEvent();

	public:
		void				StartSaveEvent();
		void				StartRecoveryEvent();
		void				StartCheckSpeedHackEvent();
		void				StartDestroyWhenIdleEvent();

		LPEVENT				m_pkDeadEvent;
		LPEVENT				m_pkStunEvent;
		LPEVENT				m_pkSaveEvent;
		LPEVENT				m_pkRecoveryEvent;
		LPEVENT				m_pkTimedEvent;
		LPEVENT				m_pkFishingEvent;
		LPEVENT				m_pkAffectEvent;
		LPEVENT				m_pkPoisonEvent;
#ifdef ENABLE_WOLFMAN_CHARACTER
		LPEVENT				m_pkBleedingEvent;
#endif
		LPEVENT				m_pkFireEvent;
		LPEVENT				m_pkWarpNPCEvent;
		//DELAYED_WARP
		//END_DELAYED_WARP

		// MINING
		LPEVENT				m_pkMiningEvent;
		// END_OF_MINING
		LPEVENT				m_pkWarpEvent;
		LPEVENT				m_pkCheckSpeedHackEvent;
		LPEVENT				m_pkDestroyWhenIdleEvent;
		LPEVENT				m_pkPetSystemUpdateEvent;

		bool IsWarping() const { return m_pkWarpEvent ? true : false; }

		bool				m_bHasPoisoned;
#ifdef ENABLE_WOLFMAN_CHARACTER
		bool				m_bHasBled;
#endif

		const CMob *		m_pkMobData;
		CMobInstance *		m_pkMobInst;

		std::map<int32_t, LPEVENT> m_mapMobSkillEvent;

		friend struct FuncSplashDamage;
		friend struct FuncSplashAffect;
		friend class CFuncShoot;

	public:
		int32_t				GetPremiumRemainSeconds(uint8_t bType) const;

	private:
		int32_t				m_aiPremiumTimes[PREMIUM_MAX_NUM];

		// CHANGE_ITEM_ATTRIBUTES
		// static const uint32_t		msc_dwDefaultChangeItemAttrCycle;	///< ����Ʈ ������ �Ӽ����� ���� �ֱ�
		static const char		msc_szLastChangeItemAttrFlag[];		///< �ֱ� ������ �Ӽ��� ������ �ð��� Quest Flag �̸�
		// static const char		msc_szChangeItemAttrCycleFlag[];		///< ������ �Ӽ����� ���� �ֱ��� Quest Flag �̸�
		// END_OF_CHANGE_ITEM_ATTRIBUTES

		// NEW_HAIR_STYLE_ADD
	public :
		bool ItemProcess_Hair(LPITEM item, int32_t iDestCell);
		// END_NEW_HAIR_STYLE_ADD

	public :
		void ClearSkill();
		void ClearSubSkill();

		// RESET_ONE_SKILL
		bool ResetOneSkill(uint32_t dwVnum);
		// END_RESET_ONE_SKILL

	private :
		void SendDamagePacket(LPCHARACTER pAttacker, int32_t Damage, uint8_t DamageFlag);

	// ARENA
	private :
		CArena *m_pArena;
		bool m_ArenaObserver;
		int32_t m_nPotionLimit;

	public :
		void 	SetArena(CArena* pArena) { m_pArena = pArena; }
		void	SetArenaObserverMode(bool flag) { m_ArenaObserver = flag; }

		CArena* GetArena() const { return m_pArena; }
		bool	GetArenaObserverMode() const { return m_ArenaObserver; }

		void	SetPotionLimit(int32_t count) { m_nPotionLimit = count; }
		int32_t		GetPotionLimit() const { return m_nPotionLimit; }
	// END_ARENA

		//PREVENT_TRADE_WINDOW
	public:
		bool	IsOpenSafebox() const { return m_isOpenSafebox ? true : false; }
		void 	SetOpenSafebox(bool b) { m_isOpenSafebox = b; }

		int32_t		GetSafeboxLoadTime() const { return m_iSafeboxLoadTime; }
		void	SetSafeboxLoadTime() { m_iSafeboxLoadTime = thecore_pulse(); }
		//END_PREVENT_TRADE_WINDOW
	private:
		bool	m_isOpenSafebox;

	public:
		int32_t		GetSkillPowerByLevel(int32_t level, bool bMob = false) const;
		
		int32_t	GetRefineTime() const { return m_iRefineTime; }
		void	SetRefineTime() { m_iRefineTime = thecore_pulse(); }

		int32_t GetUseSeedOrMoonBottleTime() const { return m_iSeedTime; }
		void  	SetUseSeedOrMoonBottleTime() { m_iSeedTime = thecore_pulse(); }

		int32_t	GetExchangeTime() const { return m_iExchangeTime; }
		void	SetExchangeTime() { m_iExchangeTime = thecore_pulse(); }

		int32_t	GetMyShopTime() const	{ return m_iMyShopTime; }
		void	SetMyShopTime() { m_iMyShopTime = thecore_pulse(); }

		uint32_t GetLastPrivateShopOpenTime() const { return m_dwLastShopOpenTime; }
		void	 SetLastPrivateShopOpenTime() { m_dwLastShopOpenTime = get_dword_time(); }

		// Hack ������ ���� üũ.
		bool	IsHack(bool bSendMsg = true, bool bCheckShopOwner = true, int32_t limittime = g_nPortalLimitTime);

		void	Say(const std::string & s);

	private:
		int32_t m_iSeedTime;
		int32_t	m_iExchangeTime;
		int32_t	m_iRefineTime;
		int32_t m_iMyShopTime;
		uint32_t m_dwLastShopOpenTime;

	public:
		bool ItemProcess_Polymorph(LPITEM item);

		// by mhh
		LPITEM*	GetCubeItem() { return m_pointsInstant.pCubeItems; }
		bool IsCubeOpen () const	{ return (m_pointsInstant.pCubeNpc?true:false); }
		void SetCubeNpc(LPCHARACTER npc)	{ m_pointsInstant.pCubeNpc = npc; }
		bool CanDoCube() const;

	private:
		int32_t		m_deposit_pulse;

	public:
		void	UpdateDepositPulse();
		bool	CanDeposit() const;

	private:
		void	__OpenPrivateShop();

	public:
		struct AttackedLog
		{
			uint32_t 	dwPID;
			uint32_t	dwAttackedTime;
			
			AttackedLog() : dwPID(0), dwAttackedTime(0)
			{
			}
		};

		AttackLog	m_kAttackLog;
		AttackedLog m_AttackedLog;
		int32_t			m_speed_hack_count;

	private :
		std::string m_strNewName;

	public :
		const std::string GetNewName() const { return this->m_strNewName; }
		void SetNewName(const std::string & name) { this->m_strNewName = name; }

	public :
		bool IsInHome();
		void GoHome();

	private :
		std::set<uint32_t>	m_known_guild;

	public :
		void SendGuildName(CGuild* pGuild);
		void SendGuildName(uint32_t dwGuildID);

	private :
		uint32_t m_dwLogOffInterval;

	public :
		uint32_t GetLogOffInterval() const { return m_dwLogOffInterval; }

	public:
		bool UnEquipSpecialRideUniqueItem ();

		bool CanWarp () const;

	private:
		uint32_t m_dwLastGoldDropTime;
		uint32_t m_dwLastItemDropTime;
		uint32_t m_dwLastBoxUseTime;
		uint32_t m_dwLastBuySellTime;
	public:
		uint32_t GetLastBuySellTime() const { return m_dwLastBuySellTime; }
		void SetLastBuySellTime(uint32_t dwLastBuySellTime) { m_dwLastBuySellTime = dwLastBuySellTime; }

	public:
		void AutoRecoveryItemProcess (const EAffectTypes);

	public:
		void BuffOnAttr_AddBuffsFromItem(LPITEM pItem);
		void BuffOnAttr_RemoveBuffsFromItem(LPITEM pItem);

	private:
		void BuffOnAttr_ValueChange(uint8_t bType, uint8_t bOldValue, uint8_t bNewValue);
		void BuffOnAttr_ClearAll();

		typedef std::map <uint8_t, CBuffOnAttributes*> TMapBuffOnAttrs;
		TMapBuffOnAttrs m_map_buff_on_attrs;
		// ���� : ��Ȱ�� �׽�Ʈ�� ���Ͽ�.
	public:
		void SetArmada() { cannot_dead = true; }
		void ResetArmada() { cannot_dead = false; }
	private:
		bool cannot_dead;

	private:
		bool m_bIsPet;
	public:
		void SetPet() { m_bIsPet = true; }
		bool IsPet() { return m_bIsPet; }

	private:
		uint32_t m_dwNDRFlag;
		std::set<uint32_t> m_setNDAFlag;
	public:
		uint32_t GetNoDamageRaceFlag();
		void SetNoDamageRaceFlag(uint32_t dwRaceFlag);
		void UnsetNoDamageRaceFlag(uint32_t dwRaceFlag);
		void ResetNoDamageRaceFlag();
		const std::set<uint32_t> & GetNoDamageAffectFlag();
		void SetNoDamageAffectFlag(uint32_t dwAffectFlag);
		void UnsetNoDamageAffectFlag(uint32_t dwAffectFlag);
		void ResetNoDamageAffectFlag();


	
	private:
		float m_fAttMul;
		float m_fDamMul;
	public:
		float GetAttMul() { return this->m_fAttMul; }
		void SetAttMul(float newAttMul) {this->m_fAttMul = newAttMul; }
		float GetDamMul() { return this->m_fDamMul; }
		void SetDamMul(float newDamMul) {this->m_fDamMul = newDamMul; }

	private:
		bool IsValidItemPosition(const TItemPos & Pos) const;

	public:
		

		
		
		void	DragonSoul_Initialize();

		bool	DragonSoul_IsQualified() const;
		void	DragonSoul_GiveQualification();

		int32_t		DragonSoul_GetActiveDeck() const;
		bool	DragonSoul_IsDeckActivated() const;
		bool	DragonSoul_ActivateDeck(int32_t deck_idx);

		void	DragonSoul_DeactivateAll();
		
		
		
		
		//
		
		
		
		
		void	DragonSoul_CleanUp();
		
	public:
		bool		DragonSoul_RefineWindow_Open(LPENTITY pEntity);
		bool		DragonSoul_RefineWindow_Close();
		LPENTITY	DragonSoul_RefineWindow_GetOpener() { return  m_pointsInstant.m_pDragonSoulRefineWindowOpener; }
		bool		DragonSoul_RefineWindow_CanRefine();

	private:
		uint32_t itemAward_vnum;
		char		 itemAward_cmd[20];
		//bool		 itemAward_flag;
	public:
		uint32_t GetItemAward_vnum() { return itemAward_vnum; }
		char*		 GetItemAward_cmd() { return itemAward_cmd;	  }
		//bool		 GetItemAward_flag() { return itemAward_flag; }
		void		 SetItemAward_vnum(uint32_t vnum) { itemAward_vnum = vnum; }
		void		 SetItemAward_cmd(char* cmd) { strcpy(itemAward_cmd,cmd); }
		//void		 SetItemAward_flag(bool flag) { itemAward_flag = flag; }
	private:
		int32_t m_dwCmdAntiFloodPulse;
		uint32_t m_dwCmdAntiFloodCount;
	public:
		int32_t GetCmdAntiFloodPulse(){return m_dwCmdAntiFloodPulse;}
		uint32_t GetCmdAntiFloodCount(){return m_dwCmdAntiFloodCount;}
		uint32_t IncreaseCmdAntiFloodCount(){return ++m_dwCmdAntiFloodCount;}
		void SetCmdAntiFloodPulse(int32_t dwPulse){m_dwCmdAntiFloodPulse=dwPulse;}
		void SetCmdAntiFloodCount(uint32_t dwCount){m_dwCmdAntiFloodCount=dwCount;}
	private:
		
		
		timeval		m_tvLastSyncTime;
		int32_t			m_iSyncPlayerHackCount;
		int32_t			m_iSyncMonsterHackCount;
		int32_t			m_iSyncNoHackCount;

	public:
		void			SetLastSyncTime(const timeval& tv) { memcpy(&m_tvLastSyncTime, &tv, sizeof(timeval)); }
		const timeval& GetLastSyncTime() { return m_tvLastSyncTime; }
		void			UpdateSyncHackCount(const std::string& who, bool increase);
		int32_t				GetSyncHackCount(const std::string& who) const;

		//Overload
		int32_t				GetSyncHackCount() { return m_iSyncMonsterHackCount + m_iSyncPlayerHackCount; }

		//No hack counters
		int32_t				GetNoHackCount() { return m_iSyncNoHackCount; }
		void			SetNoHackCount(int32_t iCount) { m_iSyncNoHackCount = iCount; }

	//Abuse
	private:
		spAbuseController	m_abuse;

	public:
		spAbuseController	GetAbuseController() const { return m_abuse; }

	//Activity
	public:
		spActivityHandler GetActivityHandler() { return m_activityHandler; }

		int32_t GetActivity() const;
		void LoadActivity(TActivityTable* data) const;
	private:
		spActivityHandler m_activityHandler;

		void	SetCoins(int32_t val);
		void	UpdateCoins(uint32_t dwAID, int32_t val);
		int32_t	GetCoins();

	public:
		bool		IsPrivateMap(int32_t lMapIndex = 0) const;


#ifdef ENABLE_ACCE_SYSTEM
	protected:
		bool	m_bAcceCombination, m_bAcceAbsorption;

	public:
		bool	isAcceOpened(bool bCombination) {return bCombination ? m_bAcceCombination : m_bAcceAbsorption;}
		void	OpenAcce(bool bCombination);
		void	CloseAcce();
		void	ClearAcceMaterials();
		bool	CleanAcceAttr(LPITEM pkItem, LPITEM pkTarget);
		LPITEM*	GetAcceMaterials() {return m_pointsInstant.pAcceMaterials;}
		bool	AcceIsSameGrade(int32_t lGrade);
		uint32_t	GetAcceCombinePrice(int32_t lGrade);
		void	GetAcceCombineResult(uint32_t & dwItemVnum, uint32_t & dwMinAbs, uint32_t & dwMaxAbs);
		uint8_t	CheckEmptyMaterialSlot();
		void	AddAcceMaterial(TItemPos tPos, uint8_t bPos);
		void	RemoveAcceMaterial(uint8_t bPos);
		uint8_t	CanRefineAcceMaterials();
		void	RefineAcceMaterials();
#endif

};

ESex GET_SEX(LPCHARACTER ch);
