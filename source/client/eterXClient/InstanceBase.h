#pragma once
#include "../../common/defines.h"

#include "../eterGameLib/RaceData.h"
#include "../eterGameLib/ActorInstance.h"
#include "../eterLib/GrpObjectInstance.h"

#include "AffectFlagContainer.h"

class CInstanceBase
{	
	public:
		struct SCreateData
		{
			uint8_t	m_bType;
			uint32_t	m_dwStateFlags;
			uint32_t	m_dwEmpireID;
			uint32_t	m_dwGuildID;
			uint32_t	m_dwLevel;
			uint32_t	m_dwVID;
			uint32_t	m_dwRace;
			uint32_t	m_dwMovSpd;
			uint32_t	m_dwAtkSpd;
			int32_t	m_lPosX;
			int32_t	m_lPosY;
			float	m_fRot;
			uint32_t	m_dwArmor;
			uint32_t	m_dwWeapon;
			uint32_t	m_dwHair;
#ifdef ENABLE_ACCE_SYSTEM
			uint32_t	m_dwAcce;
#endif
			uint32_t	m_dwMountVnum;
			
			int16_t	m_sAlignment;
			uint8_t	m_byPKMode;
			CAffectFlagContainer	m_kAffectFlags;

			std::string m_stName;

			bool	m_isMain;
		};

	public:
		using TType = uint32_t;

		enum EDirection
		{
			DIR_NORTH,
			DIR_NORTHEAST,
			DIR_EAST,
			DIR_SOUTHEAST,
			DIR_SOUTH,
			DIR_SOUTHWEST,
			DIR_WEST,
			DIR_NORTHWEST,
			DIR_MAX_NUM
		};

		enum
		{
			AFFECT_YMIR,
			AFFECT_INVISIBILITY,
			AFFECT_SPAWN,

			AFFECT_POISON,
			AFFECT_SLOW,
			AFFECT_STUN,

			AFFECT_DUNGEON_READY,			// �������� �غ� ����
			AFFECT_SHOW_ALWAYS,				// AFFECT_DUNGEON_UNIQUE ���� ����(Ŭ���̾�Ʈ���� �ø���������)

			AFFECT_BUILDING_CONSTRUCTION_SMALL,
			AFFECT_BUILDING_CONSTRUCTION_LARGE,
			AFFECT_BUILDING_UPGRADE,

			AFFECT_MOV_SPEED_POTION,		// 11
			AFFECT_ATT_SPEED_POTION,		// 12

			AFFECT_FISH_MIND,				// 13

			AFFECT_JEONGWI,					// 14 ����ȥ
			AFFECT_GEOMGYEONG,				// 15 �˰�
			AFFECT_CHEONGEUN,				// 16 õ����
			AFFECT_GYEONGGONG,				// 17 �����
			AFFECT_EUNHYEONG,				// 18 ������
			AFFECT_GWIGEOM,					// 19 �Ͱ�
			AFFECT_GONGPO,					// 20 ����
			AFFECT_JUMAGAP,					// 21 �ָ���
			AFFECT_HOSIN,					// 22 ȣ��
			AFFECT_BOHO,					// 23 ��ȣ
			AFFECT_KWAESOK,					// 24 ���
		    AFFECT_HEUKSIN,					// 25 ��ż�ȣ
			AFFECT_MUYEONG,					// 26 ������
			AFFECT_REVIVE_INVISIBILITY,		// 27 ��Ȱ ����
			AFFECT_FIRE,					// 28 ���� ��
			AFFECT_GICHEON,					// 29 ��õ ���
			AFFECT_JEUNGRYEOK,				// 30 ���¼� 
			AFFECT_DASH,					// 31 �뽬
			AFFECT_PABEOP,					// 32 �Ĺ���
			AFFECT_FALLEN_CHEONGEUN,		// 33 �ٿ� �׷��̵� õ����
			AFFECT_POLYMORPH,				// 34 ��������
			AFFECT_WAR_FLAG1,				// 35
			AFFECT_WAR_FLAG2,				// 36
			AFFECT_WAR_FLAG3,				// 37
			AFFECT_CHINA_FIREWORK,			// 38
			AFFECT_PREMIUM_SILVER,
			AFFECT_PREMIUM_GOLD,
			AFFECT_RAMADAN_RING,			// 41 �ʽ´� ���� ���� Affect
#ifdef ENABLE_WOLFMAN_CHARACTER
			AFFECT_BLEEDING,				// 42
			AFFECT_RED_POSSESSION,			// 43
			AFFECT_BLUE_POSSESSION,			// 44
#endif

			AFFECT_NUM = 64,

			AFFECT_HWAYEOM = AFFECT_GEOMGYEONG,
		};

		enum
		{
			NEW_AFFECT_MOV_SPEED            = 200,
			NEW_AFFECT_ATT_SPEED,
			NEW_AFFECT_ATT_GRADE,
			NEW_AFFECT_INVISIBILITY,
			NEW_AFFECT_STR,
			NEW_AFFECT_DEX,                 // 205
			NEW_AFFECT_CON,
			NEW_AFFECT_INT,
			NEW_AFFECT_FISH_MIND_PILL,

			NEW_AFFECT_POISON,
			NEW_AFFECT_STUN,                // 210
			NEW_AFFECT_SLOW,
			NEW_AFFECT_DUNGEON_READY,
			NEW_AFFECT_DUNGEON_UNIQUE,

			NEW_AFFECT_BUILDING,
			NEW_AFFECT_REVIVE_INVISIBLE,    // 215
			NEW_AFFECT_FIRE,
			NEW_AFFECT_CAST_SPEED,
			NEW_AFFECT_HP_RECOVER_CONTINUE,
			NEW_AFFECT_SP_RECOVER_CONTINUE, 

			NEW_AFFECT_POLYMORPH,           // 220
			NEW_AFFECT_MOUNT,

			NEW_AFFECT_WAR_FLAG,            // 222

			NEW_AFFECT_BLOCK_CHAT,          // 223
			NEW_AFFECT_CHINA_FIREWORK,

			NEW_AFFECT_BOW_DISTANCE,        // 225

			NEW_AFFECT_EXP_BONUS         = 500, // ������ ����
			NEW_AFFECT_ITEM_BONUS        = 501, // ������ �尩
			NEW_AFFECT_SAFEBOX           = 502, // PREMIUM_SAFEBOX,
			NEW_AFFECT_AUTOLOOT          = 503, // PREMIUM_AUTOLOOT,
			NEW_AFFECT_FISH_MIND         = 504, // PREMIUM_FISH_MIND,
			NEW_AFFECT_MARRIAGE_FAST     = 505, // ������ ���� (�ݽ�),
			NEW_AFFECT_GOLD_BONUS        = 506,

		    NEW_AFFECT_MALL              = 510, // �� ������ ����Ʈ
			NEW_AFFECT_NO_DEATH_PENALTY  = 511, // ����� ��ȣ (����ġ �г�Ƽ�� �ѹ� �����ش�)
			NEW_AFFECT_SKILL_BOOK_BONUS  = 512, // ������ ���� (å ���� ���� Ȯ���� 50% ����)
			NEW_AFFECT_SKILL_BOOK_NO_DELAY  = 513, // �־� ���� (å ���� ������ ����)

			NEW_AFFECT_EXP_BONUS_EURO_FREE = 516, // ������ ���� (���� ���� 14 ���� ���� �⺻ ȿ��)
			NEW_AFFECT_EXP_BONUS_EURO_FREE_UNDER_15 = 517,

			NEW_AFFECT_AUTO_HP_RECOVERY		= 534,		// �ڵ����� HP
			NEW_AFFECT_AUTO_SP_RECOVERY		= 535,		// �ڵ����� SP

			NEW_AFFECT_DRAGON_SOUL_QUALIFIED = 540, 
			NEW_AFFECT_DRAGON_SOUL_DECK1 = 541,
			NEW_AFFECT_DRAGON_SOUL_DECK2 = 542,

			NEW_AFFECT_RAMADAN_ABILITY = 300,
			NEW_AFFECT_RAMADAN_RING    = 301,			// �󸶴� �̺�Ʈ�� Ư�������� �ʽ´��� ���� ���� ����

			NEW_AFFECT_NOG_POCKET_ABILITY = 302,

			NEW_AFFECT_QUEST_START_IDX   = 1000,
		};

		enum EStone
		{
			STONE_SMOKE1 = 0,	// 99%
			STONE_SMOKE2 = 1,	// 85%
			STONE_SMOKE3 = 2,	// 80%
			STONE_SMOKE4 = 3,	// 60%
			STONE_SMOKE5 = 4,	// 45%
			STONE_SMOKE6 = 5,	// 40%
			STONE_SMOKE7 = 6,	// 20%
			STONE_SMOKE8 = 7,	// 10%
			STONE_SMOKE_NUM = 4
		};

		enum EBuildingAffect
		{
			BUILDING_CONSTRUCTION_SMALL = 0,
			BUILDING_CONSTRUCTION_LARGE = 1,
			BUILDING_UPGRADE = 2
		};

		enum EWeapon
		{
			WEAPON_DUALHAND,
			WEAPON_ONEHAND,
			WEAPON_TWOHAND,
			WEAPON_NUM
		};

		enum EEmpire
		{
			EMPIRE_NONE,
			EMPIRE_A,
			EMPIRE_B,
			EMPIRE_C,
			EMPIRE_NUM
		};

		enum ENameColor
		{	
			NAMECOLOR_MOB,
			NAMECOLOR_NPC,
			NAMECOLOR_PC,
			NAMECOLOR_PC_END = NAMECOLOR_PC + EMPIRE_NUM,							
			NAMECOLOR_NORMAL_MOB,
			NAMECOLOR_NORMAL_NPC,
			NAMECOLOR_NORMAL_PC,
			NAMECOLOR_NORMAL_PC_END = NAMECOLOR_NORMAL_PC + EMPIRE_NUM,
			NAMECOLOR_EMPIRE_MOB,
			NAMECOLOR_EMPIRE_NPC,
			NAMECOLOR_EMPIRE_PC,
			NAMECOLOR_EMPIRE_PC_END = NAMECOLOR_EMPIRE_PC + EMPIRE_NUM,
			NAMECOLOR_FUNC,
			NAMECOLOR_PK,
			NAMECOLOR_PVP,
			NAMECOLOR_PARTY,
			NAMECOLOR_WARP,
			NAMECOLOR_WAYPOINT,						
			NAMECOLOR_SHOP,
			NAMECOLOR_EXTRA = NAMECOLOR_FUNC + 10,
			NAMECOLOR_NUM = NAMECOLOR_EXTRA + 10,
		};
				
		enum EAlignmentType
		{
			ALIGNMENT_TYPE_WHITE,
			ALIGNMENT_TYPE_NORMAL,
			ALIGNMENT_TYPE_DARK
		};

		enum EEmoticon
		{
			EMOTICON_EXCLAMATION	= 1,
			EMOTICON_FISH			= 11,
			EMOTICON_NUM			= 128,

			TITLE_NUM				= 9,
			TITLE_NONE				= 4,
		};

		enum ERefinedEffect
		{
			EFFECT_REFINED_NONE,

			EFFECT_SWORD_REFINED7,
			EFFECT_SWORD_REFINED8,
			EFFECT_SWORD_REFINED9,

			EFFECT_BOW_REFINED7,
			EFFECT_BOW_REFINED8,
			EFFECT_BOW_REFINED9,

			EFFECT_FANBELL_REFINED7,
			EFFECT_FANBELL_REFINED8,
			EFFECT_FANBELL_REFINED9,

			EFFECT_SMALLSWORD_REFINED7,
			EFFECT_SMALLSWORD_REFINED8,
			EFFECT_SMALLSWORD_REFINED9,

			EFFECT_SMALLSWORD_REFINED7_LEFT,
			EFFECT_SMALLSWORD_REFINED8_LEFT,
			EFFECT_SMALLSWORD_REFINED9_LEFT,

			EFFECT_BODYARMOR_REFINED7,
			EFFECT_BODYARMOR_REFINED8,
			EFFECT_BODYARMOR_REFINED9,

			EFFECT_BODYARMOR_SPECIAL,	// ���� 4-2-1
			EFFECT_BODYARMOR_SPECIAL2,	// ���� 4-2-2

#ifdef ENABLE_LVL115_ARMOR_EFFECT
			EFFECT_BODYARMOR_SPECIAL3,	// 5-1
#endif
#ifdef ENABLE_ACCE_SYSTEM
			EFFECT_ACCE,
#endif

			EFFECT_REFINED_NUM,
		};
		
		enum DamageFlag
		{
			DAMAGE_NORMAL	= (1<<0),
			DAMAGE_POISON	= (1<<1),
			DAMAGE_DODGE	= (1<<2),
			DAMAGE_BLOCK	= (1<<3),
			DAMAGE_PENETRATE= (1<<4),
			DAMAGE_CRITICAL = (1<<5),
			// ��-_-��
		};

		enum EEffect
		{
			EFFECT_DUST,
			EFFECT_STUN,
			EFFECT_HIT,
			EFFECT_FLAME_ATTACK,
			EFFECT_FLAME_HIT,
			EFFECT_FLAME_ATTACH,
			EFFECT_ELECTRIC_ATTACK,
			EFFECT_ELECTRIC_HIT,
			EFFECT_ELECTRIC_ATTACH,
			EFFECT_SPAWN_APPEAR,
			EFFECT_SPAWN_DISAPPEAR,
			EFFECT_LEVELUP,
			EFFECT_SKILLUP,
			EFFECT_HPUP_RED,
			EFFECT_SPUP_BLUE,
			EFFECT_SPEEDUP_GREEN,
			EFFECT_DXUP_PURPLE,
			EFFECT_CRITICAL,
			EFFECT_PENETRATE,
			EFFECT_BLOCK,
			EFFECT_DODGE,
			EFFECT_FIRECRACKER,
			EFFECT_SPIN_TOP,
			EFFECT_WEAPON,
			EFFECT_WEAPON_END = EFFECT_WEAPON + WEAPON_NUM,
			EFFECT_AFFECT,
			EFFECT_AFFECT_GYEONGGONG = EFFECT_AFFECT + AFFECT_GYEONGGONG,
			EFFECT_AFFECT_KWAESOK = EFFECT_AFFECT + AFFECT_KWAESOK,
			EFFECT_AFFECT_END = EFFECT_AFFECT + AFFECT_NUM,
			EFFECT_EMOTICON,
			EFFECT_EMOTICON_END = EFFECT_EMOTICON + EMOTICON_NUM,
			EFFECT_SELECT,
			EFFECT_TARGET,
			EFFECT_EMPIRE,
			EFFECT_EMPIRE_END = EFFECT_EMPIRE + EMPIRE_NUM,
			EFFECT_HORSE_DUST,
			EFFECT_REFINED,
			EFFECT_REFINED_END = EFFECT_REFINED + EFFECT_REFINED_NUM,
			EFFECT_DAMAGE_TARGET,
			EFFECT_DAMAGE_NOT_TARGET,
			EFFECT_DAMAGE_SELFDAMAGE,
			EFFECT_DAMAGE_SELFDAMAGE2,
			EFFECT_DAMAGE_POISON,
			EFFECT_DAMAGE_MISS,
			EFFECT_DAMAGE_TARGETMISS,
			EFFECT_DAMAGE_CRITICAL,
			EFFECT_SUCCESS,
			EFFECT_FAIL,
			EFFECT_FR_SUCCESS,			
			EFFECT_LEVELUP_ON_14_FOR_GERMANY,	//������ 14�϶� ( �������� )
			EFFECT_LEVELUP_UNDER_15_FOR_GERMANY,//������ 15�϶� ( �������� )
			EFFECT_PERCENT_DAMAGE1,
			EFFECT_PERCENT_DAMAGE2,
			EFFECT_PERCENT_DAMAGE3,
			EFFECT_AUTO_HPUP,
			EFFECT_AUTO_SPUP,
			EFFECT_RAMADAN_RING_EQUIP,			// �ʽ´� ���� ���� ������ �ߵ��ϴ� ����Ʈ
			EFFECT_HALLOWEEN_CANDY_EQUIP,		// �ҷ��� ���� ���� ������ �ߵ��ϴ� ����Ʈ
			EFFECT_HAPPINESS_RING_EQUIP,				// �ູ�� ���� ���� ������ �ߵ��ϴ� ����Ʈ
			EFFECT_LOVE_PENDANT_EQUIP,				// �ູ�� ���� ���� ������ �ߵ��ϴ� ����Ʈ
			EFFECT_TEMP,
#ifdef ENABLE_ACCE_SYSTEM
			EFFECT_ACCE_SUCCEDED,
			EFFECT_ACCE_EQUIP,
#endif
			EFFECT_NUM,
		};

		enum EDuel
		{
			DUEL_NONE,
			DUEL_CANNOTATTACK,
			DUEL_START
		};

	public:
		static void DestroySystem();
		static void CreateSystem(uint32_t uCapacity);
		static bool RegisterEffect(uint32_t eEftType, const char* c_szEftAttachBone, const char* c_szEftName, bool isCache);
		static void RegisterTitleName(int32_t iIndex, const char * c_szTitleName);
		static bool RegisterNameColor(uint32_t uIndex, uint32_t r, uint32_t g, uint32_t b);
		static bool RegisterTitleColor(uint32_t uIndex, uint32_t r, uint32_t g, uint32_t b);

		static void SetDustGap(float fDustGap);
		static void SetHorseDustGap(float fDustGap);

		static void SetEmpireNameMode(bool isEnable);
		static const D3DXCOLOR& GetIndexedNameColor(uint32_t eNameColor);

	public:
		void SetMainInstance();

		void OnSelected();
		void OnUnselected();
		void OnTargeted();
		void OnUntargeted();

	protected:
		bool __IsExistMainInstance();
		bool __IsMainInstance();
		bool __MainCanSeeHiddenThing();
		float __GetBowRange();

	protected:
		uint32_t	__AttachEffect(uint32_t eEftType);
		void	__DetachEffect(uint32_t dwEID);

	public:		
		void CreateSpecialEffect(uint32_t iEffectIndex);
		void AttachSpecialEffect(uint32_t effect);
		void RemoveAttachedSpecialEffect(uint32_t effect);

	protected:
		std::map<uint32_t, std::vector<uint32_t>> m_specialEffects;
		static std::string ms_astAffectEffectAttachBone[EFFECT_NUM];
		static uint32_t ms_adwCRCAffectEffect[EFFECT_NUM];
		static float ms_fDustGap;
		static float ms_fHorseDustGap;

	public:
		CInstanceBase();
		virtual ~CInstanceBase();

		void SetLODLimits(uint32_t index, float fLimit);

	protected:
		bool m_IsAlwaysRender;
	public:
		bool IsAlwaysRender() const;
		void SetAlwaysRender(bool val);

		bool LessRenderOrder(CInstanceBase* pkInst);

		void MountHorse(uint32_t eRace);
		void DismountHorse();		

		// ��ũ��Ʈ�� �׽�Ʈ �Լ�. ���߿� ������
		void SCRIPT_SetAffect(uint32_t eAffect, bool isVisible); 

		float CalculateDistanceSq3d(const TPixelPosition& c_rkPPosDst);

		// Instance Data
		bool IsFlyTargetObject();
		void ClearFlyTargetInstance();
		void SetFlyTargetInstance(CInstanceBase& rkInstDst);
		void AddFlyTargetInstance(CInstanceBase& rkInstDst);
		void AddFlyTargetPosition(const TPixelPosition& c_rkPPosDst);

		float GetFlyTargetDistance();

		void SetAlpha(float fAlpha);

		void DeleteBlendOut();

		void					AttachTextTail();
		void					DetachTextTail();
		void					UpdateTextTailLevel(uint32_t level);

		void					RefreshTextTail();
		void					RefreshTextTailTitle();

		bool					Create(const SCreateData& c_rkCreateData);

		bool					CreateDeviceObjects();
		void					DestroyDeviceObjects();

		void					Destroy();

		void					Update();
		bool					UpdateDeleting();

		void					Transform();
		void					Deform();
		void					Render();
		void					RenderTrace();
		void					RenderToShadowMap();
		void					RenderCollision();
		void					RegisterBoundingSphere();

		// Temporary
		void					GetBoundBox(D3DXVECTOR3 * vtMin, D3DXVECTOR3 * vtMax);

		void					SetNameString(const char* c_szName, int32_t len);
		bool					SetRace(uint32_t dwRaceIndex);
		void					SetScale(float fScale) { m_GraphicThingInstance.SetScale(fScale, fScale, fScale, true); }
		void					SetVirtualID(uint32_t wVirtualNumber);
		void					SetVirtualNumber(uint32_t dwVirtualNumber);
		void					SetInstanceType(int32_t iInstanceType);
		void					SetAlignment(int16_t sAlignment);
		void					SetPKMode(uint8_t byPKMode);
		void					SetKiller(bool bFlag);
		void					SetPartyMemberFlag(bool bFlag);
		void					SetStateFlags(uint32_t dwStateFlags);

		void					SetArmor(uint32_t dwArmor);
		void					SetShape(uint32_t eShape, float fSpecular=0.0f);
		void					SetHair(uint32_t eHair);
#ifdef ENABLE_ACCE_SYSTEM
		void					SetAcce(uint32_t dwAcce);
		void					ChangeAcce(uint32_t dwAcce);
#endif
		bool					SetWeapon(uint32_t eWeapon);
		bool					ChangeArmor(uint32_t dwArmor);
		void					ChangeWeapon(uint32_t eWeapon);
		void					ChangeHair(uint32_t eHair);
		void					ChangeGuild(uint32_t dwGuildID);
		uint32_t					GetWeaponType();

		void					SetComboType(uint32_t uComboType);
		void					SetAttackSpeed(uint32_t uAtkSpd);
		void					SetMoveSpeed(uint32_t uMovSpd);
		void					SetRotationSpeed(float fRotSpd);

		const char *			GetNameString();
#ifdef ENABLE_LEVEL_IN_TRADE
		uint32_t					GetLevel();
#endif
#ifdef ENABLE_TEXT_LEVEL_REFRESH
		void					SetLevel(uint32_t dwLevel);
#endif
		int32_t						GetInstanceType();
		uint32_t					GetPart(CRaceData::EParts part);
		uint32_t					GetShape();
		uint32_t					GetRace();
		uint32_t					GetVirtualID();
		uint32_t					GetVirtualNumber();
		uint32_t					GetEmpireID();
		uint32_t					GetGuildID();
		int32_t						GetAlignment();
		uint32_t					GetAlignmentGrade();
		int32_t						GetAlignmentType();
		uint8_t					GetPKMode();
		bool					IsKiller();
		bool					IsPartyMember();

		void					ActDualEmotion(CInstanceBase & rkDstInst, uint16_t dwMotionNumber1, uint16_t dwMotionNumber2);
		void					ActEmotion(uint32_t dwMotionNumber);
		void					LevelUp();
		void					SkillUp();
		void					Stun();
		void					Die();
		void					Hide();
		void					Show();

		bool					CanAct();
		bool					CanMove();
		bool					CanAttack();
		bool					CanUseSkill();
		bool					CanFishing();
		bool					IsConflictAlignmentInstance(CInstanceBase& rkInstVictim);
		bool					IsAttackableInstance(CInstanceBase& rkInstVictim);
		bool					IsTargetableInstance(CInstanceBase& rkInstVictim, bool bIgnoreViewFrustrum);
		bool					IsPVPInstance(CInstanceBase& rkInstVictim);
		bool					CanChangeTarget();
		bool CanPickInstance(bool bIgnoreViewFrustrum = false);
		bool					CanViewTargetHP(CInstanceBase& rkInstVictim);


		// Movement
		BOOL					IsGoing();
		bool					NEW_Goto(const TPixelPosition& c_rkPPosDst, float fDstRot);
		void					EndGoing();

		void					SetRunMode();
		void					SetWalkMode();

		bool					IsAffect(uint32_t uAffect);
		BOOL					IsInvisibility();
		BOOL					IsParalysis();
		BOOL					IsGameMaster();
		BOOL					IsSameEmpire(CInstanceBase& rkInstDst);
		BOOL					IsBowMode();
		BOOL					IsHandMode();
		BOOL					IsFishingMode();
		BOOL					IsFishing();

		BOOL					IsWearingDress();
		BOOL					IsHoldingPickAxe();
		BOOL					IsMountingHorse();
		BOOL					IsNewMount();
		BOOL					IsForceVisible();
		BOOL					IsInSafe();
		BOOL					IsEnemy();
		BOOL					IsStone();
		BOOL					IsResource();
		BOOL					IsNPC();
		BOOL					IsPC();
		BOOL					IsPoly();
		BOOL					IsWarp();
		BOOL					IsGoto();
		BOOL					IsObject();
		BOOL					IsShop();
		BOOL					IsDoor();
		BOOL					IsBuilding();
		BOOL					IsWoodenDoor();
		BOOL					IsStoneDoor();
		BOOL					IsFlag();
		BOOL					IsGuildWall();

		BOOL					IsDead();
		BOOL					IsStun();
		BOOL					IsSleep();
		BOOL					__IsSyncing();
		BOOL					IsWaiting();
		BOOL					IsWalking();
		BOOL					IsPushing();
		BOOL					IsAttacking();
		BOOL					IsActingEmotion();
		BOOL					IsAttacked();
		BOOL					IsKnockDown();
		BOOL					IsUsingSkill();
		BOOL					IsUsingMovingSkill();
		BOOL					CanCancelSkill();
		BOOL					CanAttackHorseLevel();

		bool					NEW_CanMoveToDestPixelPosition(const TPixelPosition& c_rkPPosDst);

		void					NEW_SetAdvancingRotationFromPixelPosition(const TPixelPosition& c_rkPPosSrc, const TPixelPosition& c_rkPPosDst);
		void					NEW_SetAdvancingRotationFromDirPixelPosition(const TPixelPosition& c_rkPPosDir);
		bool					NEW_SetAdvancingRotationFromDestPixelPosition(const TPixelPosition& c_rkPPosDst);
		void					SetAdvancingRotation(float fRotation);

		void					EndWalking(float fBlendingTime=0.15f);
		void					EndWalkingWithoutBlending();

		// Battle
		void					SetEventHandler(CActorInstance::IEventHandler* pkEventHandler);

		void					PushTCPState(uint32_t dwCmdTime, const TPixelPosition& c_rkPPosDst, float fDstRot, uint32_t eFunc, uint32_t uArg);
		void					PushTCPStateExpanded(uint32_t dwCmdTime, const TPixelPosition& c_rkPPosDst, float fDstRot, uint32_t eFunc, uint32_t uArg, uint32_t uTargetVID);

		void					NEW_Stop();

		bool					NEW_UseSkill(uint32_t uSkill, uint32_t uMot, uint32_t uMotLoopCount, bool isMovingSkill);
		void					NEW_Attack();
		void					NEW_Attack(float fDirRot);
		void					NEW_AttackToDestPixelPositionDirection(const TPixelPosition& c_rkPPosDst);
		bool					NEW_AttackToDestInstanceDirection(CInstanceBase& rkInstDst, IFlyEventHandler* pkFlyHandler);
		bool					NEW_AttackToDestInstanceDirection(CInstanceBase& rkInstDst);

		bool					NEW_MoveToDestPixelPositionDirection(const TPixelPosition& c_rkPPosDst);
		void					NEW_MoveToDestInstanceDirection(CInstanceBase& rkInstDst);
		void					NEW_MoveToDirection(float fDirRot);

		float					NEW_GetDistanceFromDirPixelPosition(const TPixelPosition& c_rkPPosDir);
		float					NEW_GetDistanceFromDestPixelPosition(const TPixelPosition& c_rkPPosDst);
		float					NEW_GetDistanceFromDestInstance(CInstanceBase& rkInstDst);

		float					NEW_GetRotation();
		float					NEW_GetRotationFromDestPixelPosition(const TPixelPosition& c_rkPPosDst);
		float					NEW_GetRotationFromDirPixelPosition(const TPixelPosition& c_rkPPosDir);
		float					NEW_GetRotationFromDestInstance(CInstanceBase& rkInstDst);

		float					NEW_GetAdvancingRotationFromDirPixelPosition(const TPixelPosition& c_rkPPosDir);
		float					NEW_GetAdvancingRotationFromDestPixelPosition(const TPixelPosition& c_rkPPosDst);
		float					NEW_GetAdvancingRotationFromPixelPosition(const TPixelPosition& c_rkPPosSrc, const TPixelPosition& c_rkPPosDst);

		BOOL					NEW_IsClickableDistanceDestPixelPosition(const TPixelPosition& c_rkPPosDst);
		BOOL					NEW_IsClickableDistanceDestInstance(CInstanceBase& rkInstDst);

		bool					NEW_GetFrontInstance(CInstanceBase ** ppoutTargetInstance, float fDistance);
		void					NEW_GetRandomPositionInFanRange(CInstanceBase& rkInstTarget, TPixelPosition* pkPPosDst);
		bool					NEW_GetInstanceVectorInFanRange(float fSkillDistance, CInstanceBase& rkInstTarget, std::vector<CInstanceBase*>* pkVct_pkInst);
		bool					NEW_GetInstanceVectorInCircleRange(float fSkillDistance, std::vector<CInstanceBase*>* pkVct_pkInst);

		void					NEW_SetOwner(uint32_t dwOwnerVID);
		void					NEW_SyncPixelPosition(int32_t nPPosX, int32_t nPPosY);

		void					NEW_SetPixelPosition(const TPixelPosition& c_rkPPosDst);

		bool					NEW_IsLastPixelPosition();
		const TPixelPosition&	NEW_GetLastPixelPositionRef();


		// Battle
		BOOL					isNormalAttacking();
		BOOL					isComboAttacking();
		MOTION_KEY				GetNormalAttackIndex();
		uint32_t					GetComboIndex();
		float					GetAttackingElapsedTime();
		void					InputNormalAttack(float fAtkDirRot);
		void					InputComboAttack(float fAtkDirRot);

		void					RunNormalAttack(float fAtkDirRot);
		void					RunComboAttack(float fAtkDirRot, uint32_t wMotionIndex);

		BOOL					CheckAdvancing();


		bool					AvoidObject(const CGraphicObjectInstance& c_rkBGObj);		
		bool					IsBlockObject(const CGraphicObjectInstance& c_rkBGObj);
		void					BlockMovement();

	public:
		BOOL					CheckAttacking(CInstanceBase& rkInstVictim);
		void					ProcessHitting(uint32_t dwMotionKey, CInstanceBase * pVictimInstance);
		void					ProcessHitting(uint32_t dwMotionKey, uint8_t byEventIndex, CInstanceBase * pVictimInstance);
		void					GetBlendingPosition(TPixelPosition * pPixelPosition);
		void					SetBlendingPosition(const TPixelPosition & c_rPixelPosition);

		// Fishing
		void					StartFishing(float frot);
		void					StopFishing();
		void					ReactFishing();
		void					CatchSuccess();
		void					CatchFail();
		BOOL					GetFishingRot(int32_t * pirot);

		// Render Mode
		void					RestoreRenderMode();
		void					SetAddRenderMode();
		void					SetModulateRenderMode();
		void					SetRenderMode(int32_t iRenderMode);
		void					SetAddColor(const D3DXCOLOR & c_rColor);

		// Position
		void					SCRIPT_SetPixelPosition(float fx, float fy);
		void					NEW_GetPixelPosition(TPixelPosition * pPixelPosition);

		// Rotation
		void					NEW_LookAtFlyTarget();
		void					NEW_LookAtDestInstance(CInstanceBase& rkInstDst);
		void					NEW_LookAtDestPixelPosition(const TPixelPosition& c_rkPPosDst);

		float					GetRotation();
		float					GetAdvancingRotation();
		void					SetRotation(float fRotation);
		void					BlendRotation(float fRotation, float fBlendTime = 0.1f);

		void					SetDirection(int32_t dir);
		void					BlendDirection(int32_t dir, float blendTime);
		float					GetDegreeFromDirection(int32_t dir);

		// Motion
		//	Motion Deque
		BOOL					isLock();

		void					SetMotionMode(int32_t iMotionMode);
		int32_t						GetMotionMode(uint32_t dwMotionIndex);

		// Motion
		//	Pushing Motion
		void					ResetLocalTime();
		void					SetLoopMotion(uint16_t wMotion, float fBlendTime=0.1f, float fSpeedRatio=1.0f);
		void					PushOnceMotion(uint16_t wMotion, float fBlendTime=0.1f, float fSpeedRatio=1.0f);
		void					PushLoopMotion(uint16_t wMotion, float fBlendTime=0.1f, float fSpeedRatio=1.0f);
		void					SetEndStopMotion();

		// Intersect
		bool					IntersectDefendingSphere();
		bool					IntersectBoundingBox();

		// Part
		//void					SetParts(const uint16_t * c_pParts);
		void					Refresh(uint32_t dwMotIndex, bool isLoop);

		//void					AttachEffectByID(uint32_t dwParentPartIndex, const char * c_pszBoneName, uint32_t dwEffectID, int32_t dwLife = CActorInstance::EFFECT_LIFE_INFINITE ); // ������ ms�����Դϴ�.
		//void					AttachEffectByName(uint32_t dwParentPartIndex, const char * c_pszBoneName, const char * c_pszEffectName, int32_t dwLife = CActorInstance::EFFECT_LIFE_INFINITE ); // ������ ms�����Դϴ�.

		float					GetDistance(CInstanceBase * pkTargetInst);
		float					GetDistance(const TPixelPosition & c_rPixelPosition);

		// ETC
		CActorInstance&			GetGraphicThingInstanceRef();
		CActorInstance*			GetGraphicThingInstancePtr();		
		
		bool __Background_IsWaterPixelPosition(const TPixelPosition& c_rkPPos);
		bool __Background_GetWaterHeight(const TPixelPosition& c_rkPPos, float* pfHeight);

		/////////////////////////////////////////////////////////////
		void __ClearAffectFlagContainer();
		void __ClearAffects();
		/////////////////////////////////////////////////////////////

		void __SetAffect(uint32_t eAffect, bool isVisible);
		
		void SetAffectFlagContainer(const CAffectFlagContainer& c_rkAffectFlagContainer);

		void __SetNormalAffectFlagContainer(const CAffectFlagContainer& c_rkAffectFlagContainer);		
		void __SetStoneSmokeFlagContainer(const CAffectFlagContainer& c_rkAffectFlagContainer);

		void SetEmoticon(uint32_t eEmoticon);		
		void SetFishEmoticon();
		bool IsPossibleEmoticon();

	public:
		bool __CanSkipCollision();

	protected:
		uint32_t					__LessRenderOrder_GetLODLevel();
		void					__Initialize();
		void					__InitializeRotationSpeed();

		void					__Create_SetName(const SCreateData& c_rkCreateData);
		void					__Create_SetWarpName(const SCreateData& c_rkCreateData);

		CInstanceBase*			__GetMainInstancePtr();
		CInstanceBase*			__FindInstancePtr(uint32_t dwVID);

		bool  __FindRaceType(uint32_t dwRace, uint8_t* pbType);
		uint32_t __GetRaceType();

		bool __IsShapeAnimalWear();
		BOOL __IsChangableWeapon(int32_t iWeaponID);

		void __EnableSkipCollision();
		void __DisableSkipCollision();

		void __ClearMainInstance();

		void __Shaman_SetParalysis(bool isParalysis);
		void __Warrior_SetGeomgyeongAffect(bool isVisible);
		void __Assassin_SetEunhyeongAffect(bool isVisible);
		void __SetReviveInvisibilityAffect(bool isVisible);

		BOOL __CanProcessNetworkStatePacket();
		
		bool __IsInDustRange();

		// Emotion
		void __ProcessFunctionEmotion(uint32_t dwMotionNumber, uint32_t dwTargetVID, const TPixelPosition & c_rkPosDst);
		void __EnableChangingTCPState();
		void __DisableChangingTCPState();
		BOOL __IsEnableTCPProcess(uint32_t eCurFunc);

		// 2004.07.17.levites.isShow�� ViewFrustumCheck�� ����
		bool __CanRender();
		bool __IsInViewFrustum();

		// HORSE
		void __AttachHorseSaddle();
		void __DetachHorseSaddle();
		
		struct SHORSE
		{
			bool m_isMounting;
			CActorInstance* m_pkActor;
			
			SHORSE();			
			~SHORSE();
			
			void Destroy();
			void Create(const TPixelPosition& c_rkPPos, uint32_t eRace, uint32_t eHitEffect);
			
			void SetAttackSpeed(uint32_t uAtkSpd);
			void SetMoveSpeed(uint32_t uMovSpd);
			void Deform();
			void Render();
			CActorInstance& GetActorRef();
			CActorInstance* GetActorPtr();

			bool IsMounting();
			bool CanAttack();
			bool CanUseSkill();

			uint32_t GetLevel();
			bool IsNewMount();

			void __Initialize();
		} m_kHorse;


	protected:
		// Blend Mode
		void					__SetBlendRenderingMode();
		void					__SetAlphaValue(float fAlpha);
		float					__GetAlphaValue();

		void					__ComboProcess();
		void					MovementProcess();
		void					StateProcess();
		void					AttackProcess();

		void					StartWalking();
		float					GetLocalTime();

		void					RefreshState(uint32_t dwMotIndex, bool isLoop);
		void					RefreshActorInstance();

	protected:
		void					OnSyncing();
		void					OnWaiting();
		void					OnMoving();

		void					NEW_SetSrcPixelPosition(const TPixelPosition& c_rkPPosDst);
		void					NEW_SetDstPixelPosition(const TPixelPosition& c_rkPPosDst);
		void					NEW_SetDstPixelPositionZ(float z);

		const TPixelPosition&	NEW_GetCurPixelPositionRef();
		const TPixelPosition&	NEW_GetSrcPixelPositionRef();

	public:
		const TPixelPosition&	NEW_GetDstPixelPositionRef();
		
	protected:
		BOOL m_isTextTail;		

		// Instance Data
		std::string				m_stName;

		uint32_t					m_awPart[CRaceData::PART_MAX_NUM];

		uint32_t					m_dwLevel;
		uint32_t					m_dwEmpireID;
		uint32_t					m_dwGuildID;
		bool					m_bIsHidden;
	public:
		bool					IsHidden();

	protected:		
		CAffectFlagContainer	m_kAffectFlagContainer;
		uint32_t					m_adwCRCAffectEffect[AFFECT_NUM];
		
		uint32_t	__GetRefinedEffect(CItemData* pItem);		
		void	__ClearWeaponRefineEffect();
		void	__ClearArmorRefineEffect();

#ifdef ENABLE_ACCE_SYSTEM
		void	ClearAcceEffect();
#endif

	protected:
		void __AttachSelectEffect();
		void __DetachSelectEffect();

		void __AttachTargetEffect();
		void __DetachTargetEffect();

		void __AttachEmpireEffect(uint32_t eEmpire);

	protected:
		struct SEffectContainer
		{
			typedef std::map<uint32_t, uint32_t> Dict;
			Dict m_kDct_dwEftID;
		} m_kEffectContainer;

		void __EffectContainer_Initialize();
		void __EffectContainer_Destroy();
		void __EffectContainer_Suspend();
		void __EffectContainer_Continue();

		uint32_t __EffectContainer_AttachEffect(uint32_t eEffect);
		void __EffectContainer_DetachEffect(uint32_t eEffect);

		SEffectContainer::Dict& __EffectContainer_GetDict();

	protected:
		struct SStoneSmoke 
		{
			uint32_t m_dwEftID;
		} m_kStoneSmoke;

		void __StoneSmoke_Inialize();
		void __StoneSmoke_Destroy();
		void __StoneSmoke_Create(uint32_t eSmoke);


	protected:
		// Emoticon
		//uint32_t					m_adwCRCEmoticonEffect[EMOTICON_NUM];

		uint8_t					m_eType;
		uint8_t					m_eRaceType;
		uint32_t					m_eShape;
		uint32_t					m_dwRace;
		uint32_t					m_dwVirtualNumber;
		int16_t					m_sAlignment;
		uint8_t					m_byPKMode;
		bool					m_isKiller;
		bool					m_isPartyMember;

		// Movement
		int32_t						m_iRotatingDirection;

		uint32_t					m_dwAdvActorVID;
		uint32_t					m_dwLastDmgActorVID;

		int32_t					m_nAverageNetworkGap;
		uint32_t					m_dwNextUpdateHeightTime;

		bool					m_isGoing;

		TPixelPosition			m_kPPosDust;

		uint32_t					m_dwLastComboIndex;

		uint32_t					m_swordRefineEffectRight;
		uint32_t					m_swordRefineEffectLeft;
		uint32_t					m_armorRefineEffect;

#ifdef ENABLE_ACCE_SYSTEM
		uint32_t					m_dwAcceEffect;
#endif

		struct SMoveAfterFunc
		{
			uint32_t eFunc;
			uint32_t uArg;

			// For Emotion Function
			uint32_t uArgExpanded;
			TPixelPosition kPosDst;
		};

		SMoveAfterFunc m_kMovAfterFunc;

		float m_fDstRot;
		float m_fAtkPosTime;
		float m_fRotSpd;
		float m_fMaxRotSpd;

		BOOL m_bEnableTCPState;

		// Graphic Instance
		CActorInstance m_GraphicThingInstance;


	protected:
		struct SCommand
		{
			uint32_t	m_dwChkTime;
			uint32_t	m_dwCmdTime;
			float	m_fDstRot;
			uint32_t 	m_eFunc;
			uint32_t 	m_uArg;
			uint32_t	m_uTargetVID;
			TPixelPosition m_kPPosDst;
		};

		using CommandQueue = std::list<SCommand>;

		uint32_t		m_dwBaseChkTime;
		uint32_t		m_dwBaseCmdTime;

		uint32_t		m_dwSkipTime;

		CommandQueue m_kQue_kCmdNew;

		BOOL		m_bDamageEffectType;

		struct SEffectDamage
		{
			uint32_t damage;
			uint8_t flag;
			BOOL bSelf;
			BOOL bTarget;
		};

		using CommandDamageQueue = std::list<SEffectDamage>;
		CommandDamageQueue m_DamageQueue;

		void ProcessDamage();

	public:
		void AddDamageEffect(uint32_t damage,uint8_t flag,BOOL bSelf,BOOL bTarget);

	protected:
		struct SWarrior
		{
			uint32_t m_dwGeomgyeongEffect;
		};

		SWarrior m_kWarrior;

		void __Warrior_Initialize();

	public:
		static void ClearPVPKeySystem();

		static void InsertPVPKey(uint32_t dwSrcVID, uint32_t dwDstVID);
		static void InsertPVPReadyKey(uint32_t dwSrcVID, uint32_t dwDstVID);
		static void RemovePVPKey(uint32_t dwSrcVID, uint32_t dwDstVID);

		static void InsertGVGKey(uint32_t dwSrcGuildVID, uint32_t dwDstGuildVID);
		static void RemoveGVGKey(uint32_t dwSrcGuildVID, uint32_t dwDstGuildVID);

		static void InsertDUELKey(uint32_t dwSrcVID, uint32_t dwDstVID);

		uint32_t GetNameColorIndex();

		const D3DXCOLOR& GetNameColor();
		const D3DXCOLOR& GetTitleColor();

	protected:
		static uint32_t __GetPVPKey(uint32_t dwSrcVID, uint32_t dwDstVID);
		static bool __FindPVPKey(uint32_t dwSrcVID, uint32_t dwDstVID);
		static bool __FindPVPReadyKey(uint32_t dwSrcVID, uint32_t dwDstVID);
		static bool __FindGVGKey(uint32_t dwSrcGuildID, uint32_t dwDstGuildID);
		static bool __FindDUELKey(uint32_t dwSrcGuildID, uint32_t dwDstGuildID);

	protected:
		CActorInstance::IEventHandler* GetEventHandlerPtr();
		CActorInstance::IEventHandler& GetEventHandlerRef();

	protected:
		static float __GetBackgroundHeight(float x, float y);
		static uint32_t __GetShadowMapColor(float x, float y);

	public:
		static void ResetPerformanceCounter();
		static void GetInfo(std::string* pstInfo);

	public:
		static CInstanceBase* New();
		static void Delete(CInstanceBase* pkInst);

		static CDynamicPool<CInstanceBase>	ms_kPool;

	protected:
		static uint32_t ms_dwUpdateCounter;
		static uint32_t ms_dwRenderCounter;
		static uint32_t ms_dwDeformCounter;

	public:		
		uint32_t					GetDuelMode();
		void					SetDuelMode(uint32_t type);
	protected:
		uint32_t					m_dwDuelMode;
		uint32_t					m_dwEmoticonTime;

#if defined(WJ_SHOW_MOB_INFO) && defined(ENABLE_SHOW_MOBAIFLAG)
	public:
		void					MobInfoAiFlagRefresh();
#endif
#if defined(WJ_SHOW_MOB_INFO) && defined(ENABLE_SHOW_MOBLEVEL)
	public:
		void					MobInfoLevelRefresh();
#endif

	protected:
		CInstanceBase *mp_flyTargetInstance;
	public:
		CInstanceBase* GetFlyTargetInstance() { return mp_flyTargetInstance; }

	protected:
		uint32_t m_dwMiningVID;
		
	public:
		bool IsMiningVID(uint32_t vid);
		void StartMining(uint32_t vid);
		void CancelMining();

	public:
		void SetGuildData(const std::string& stName, uint32_t dwMemberCount, uint32_t dwMemberCapacity);
		std::string GetGuildName() const;
		uint32_t GetGuildMemberCount() const;
		uint32_t GetGuildMemberCapacity() const;

	private:
		std::string m_stGuildName;
		uint32_t m_dwMemberCount;
		uint32_t m_dwMemberCapacity;
};

inline int32_t RaceToJob(int32_t race)
{
#ifdef ENABLE_WOLFMAN_CHARACTER
	if (race==8)
		return 4;
#endif
	const int32_t JOB_NUM = 4;
	return race % JOB_NUM;
}

inline int32_t RaceToSex(int32_t race)
{
	switch (race)
	{
		case 0:
		case 2:
		case 5:
		case 7:
#ifdef ENABLE_WOLFMAN_CHARACTER
		case 8:
#endif
			return 1;
		case 1:
		case 3:
		case 4:
		case 6:
			return 0;

	}
	return 0;
}