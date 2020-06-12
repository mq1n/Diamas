#pragma once

#include "AbstractPlayer.h"
#include "Packet.h"
#include "PythonSkill.h"

class CInstanceBase;

/*
 *	메인 캐릭터 (자신이 조정하는 캐릭터) 가 가진 정보들을 관리한다.
 *
 * 2003-01-12 Levites	본래는 CPythonCharacter가 가지고 있었지만 규모가 너무 커져 버린데다
 *						위치도 애매해서 따로 분리
 * 2003-07-19 Levites	메인 캐릭터의 이동 처리 CharacterInstance에서 떼어다 붙임
 *						기존의 데이타 보존의 역할에서 완벽한 메인 플레이어 제어 클래스로
 *						탈바꿈 함.
 */

class CPythonPlayer : public CSingleton<CPythonPlayer>, public IAbstractPlayer
{
	public:
		enum
		{
			CATEGORY_NONE		= 0,
			CATEGORY_ACTIVE		= 1,
			CATEGORY_PASSIVE	= 2,
			CATEGORY_MAX_NUM	= 3,

			STATUS_INDEX_ST = 1,
			STATUS_INDEX_DX = 2,
			STATUS_INDEX_IQ = 3,
			STATUS_INDEX_HT = 4,
		};

		enum
		{
			MBT_LEFT,
			MBT_RIGHT,
			MBT_MIDDLE,
			MBT_NUM,
		};

		enum
		{
			MBF_SMART,
			MBF_MOVE,
			MBF_CAMERA,
			MBF_ATTACK,
			MBF_SKILL,
			MBF_AUTO,
		};

		enum
		{
			MBS_CLICK,
			MBS_PRESS,
		};

		enum EMode
		{
			MODE_NONE,
			MODE_CLICK_POSITION,
			MODE_CLICK_ITEM,
			MODE_CLICK_ACTOR,
			MODE_USE_SKILL,
		};

		enum EEffect
		{
			EFFECT_PICK,
			EFFECT_NUM,
		};

		enum EMetinSocketType
		{
			METIN_SOCKET_TYPE_NONE,
			METIN_SOCKET_TYPE_SILVER,
			METIN_SOCKET_TYPE_GOLD,
		};

		typedef struct SSkillInstance
		{
			uint32_t dwIndex;
			int32_t iType;
			int32_t iGrade;
			int32_t iLevel;
			float fcurEfficientPercentage;
			float fnextEfficientPercentage;
			BOOL isCoolTime;

			float fCoolTime;			// NOTE : 쿨타임 중인 스킬 슬롯을
			float fLastUsedTime;		//        퀵창에 등록할 때 사용하는 변수
			BOOL bActive;
		} TSkillInstance;

		enum EKeyBoard_UD
		{
			KEYBOARD_UD_NONE,
			KEYBOARD_UD_UP,
			KEYBOARD_UD_DOWN,
		};

		enum EKeyBoard_LR
		{
			KEYBOARD_LR_NONE,
			KEYBOARD_LR_LEFT,
			KEYBOARD_LR_RIGHT,
		};

		enum
		{
			DIR_UP,
			DIR_DOWN,
			DIR_LEFT,
			DIR_RIGHT,
		};

		typedef struct SPlayerStatus
		{		    
			TItemData			aItem[c_Inventory_Count];
			TItemData			aDSItem[c_DragonSoul_Inventory_Count];
			TQuickSlot			aQuickSlot[QUICKSLOT_MAX_NUM];
			TSkillInstance		aSkill[SKILL_MAX_NUM];
			int32_t				m_alPoint[POINT_MAX_NUM];
			int32_t				lQuickPageIndex;

			void SetPoint(uint32_t ePoint, int32_t lPoint);
			int32_t GetPoint(uint32_t ePoint);
		} TPlayerStatus;

		typedef struct SPartyMemberInfo
		{
			SPartyMemberInfo(uint32_t _dwPID, const char * c_szName) : dwVID(0), dwPID(_dwPID), strName(c_szName), byState(0), byHPPercentage(0)
			{}

			uint32_t dwVID;
			uint32_t dwPID;
			std::string strName;
			uint8_t byState;
			uint8_t byHPPercentage;
			int16_t sAffects[PARTY_AFFECT_SLOT_MAX_NUM];
		} TPartyMemberInfo;

		enum EPartyRole
		{
			PARTY_ROLE_NORMAL,
			PARTY_ROLE_LEADER,
			PARTY_ROLE_ATTACKER,
			PARTY_ROLE_TANKER,
			PARTY_ROLE_BUFFER,
			PARTY_ROLE_SKILL_MASTER,
			PARTY_ROLE_BERSERKER,
			PARTY_ROLE_DEFENDER,
			PARTY_ROLE_MAX_NUM,
		};

		enum
		{
			SKILL_NORMAL,
			SKILL_MASTER,
			SKILL_GRAND_MASTER,
			SKILL_PERFECT_MASTER,
		};

		// 자동물약 상태 관련 특화 구조체.. 이런식의 특화 처리 작업을 안 하려고 최대한 노력했지만 실패하고 결국 특화처리.
		struct SAutoPotionInfo
		{
			SAutoPotionInfo() : bActivated(false), currentAmount(0), totalAmount(0), inventorySlotIndex(0)
			{}

			bool bActivated;					// 활성화 되었는가?			
			int32_t currentAmount;					// 현재 남은 양
			int32_t totalAmount;					// 전체 양
			int32_t inventorySlotIndex;			// 사용중인 아이템의 인벤토리상 슬롯 인덱스
		};

		enum EAutoPotionType
		{
			AUTO_POTION_TYPE_HP = 0,
			AUTO_POTION_TYPE_SP = 1,
			AUTO_POTION_TYPE_NUM
		};

	public:
		CPythonPlayer(void);
		virtual ~CPythonPlayer(void);

		void	PickCloseLoot(bool bIsYangPriority=0);

		void	SetGameWindow(PyObject * ppyObject);

		void	SetObserverMode(bool isEnable);
		bool	IsObserverMode();

		//Free camera - [Think]
		void	SetFreeCameraMode(bool isEnable);
		bool	IsFreeCameraMode();
		//End free camera

		void	SetQuickCameraMode(bool isEnable);

		void	SetAttackKeyState(bool isPress);

		void	NEW_GetMainActorPosition(TPixelPosition* pkPPosActor);

		bool	RegisterEffect(uint32_t dwEID, const char* c_szEftFileName, bool isCache);

		bool	NEW_SetMouseState(int32_t eMBType, int32_t eMBState);
		bool	NEW_SetMouseFunc(int32_t eMBType, int32_t eMBFunc);
		int32_t		NEW_GetMouseFunc(int32_t eMBT);
		void	NEW_SetMouseMiddleButtonState(int32_t eMBState);

		void	NEW_SetAutoCameraRotationSpeed(float fRotSpd);
		void	NEW_ResetCameraRotation();

		void	NEW_SetSingleDirKeyState(int32_t eDirKey, bool isPress);
		void	NEW_SetSingleDIKKeyState(int32_t eDIKKey, bool isPress);
		void	NEW_SetMultiDirKeyState(bool isLeft, bool isRight, bool isUp, bool isDown);

		void	NEW_Attack();
		void	NEW_Fishing();
		bool	NEW_CancelFishing();

		bool	NEW_MoveToMouseScreenDirection();
		bool	NEW_MoveToDirection(float fDirRot, bool isBackwards);
		void	NEW_Stop();


		// Reserved
		bool	NEW_IsEmptyReservedDelayTime(float fElapsedtime);	// 네이밍 교정 논의 필요 - [levites]


		// Dungeon
		void	SetDungeonDestinationPosition(int32_t ix, int32_t iy);
		void	AlarmHaveToGo();


		CInstanceBase* NEW_FindActorPtr(uint32_t dwVID);
		CInstanceBase* NEW_GetMainActorPtr();

		// flying target set
		void	Clear();
		void	ClearSkillDict(); // 없어지거나 ClearGame 쪽으로 포함될 함수
		void	NEW_ClearSkillData(bool bAll = false);

		void	Update();


		// Play Time
		uint32_t	GetPlayTime();
		void	SetPlayTime(uint32_t dwPlayTime);


		// System
		void	SetMainCharacterIndex(int32_t iIndex);

		uint32_t	GetMainCharacterIndex();
		bool	IsMainCharacterIndex(uint32_t dwIndex);
		uint32_t	GetGuildID();
		void	NotifyDeletingCharacterInstance(uint32_t dwVID);
		void	NotifyCharacterDead(uint32_t dwVID);
		void	NotifyCharacterUpdate(uint32_t dwVID);
		void	NotifyDeadMainCharacter();
		void	NotifyChangePKMode();


		// Player Status
		const char *	GetName();
		void	SetName(const char *name);
		
		void	SetRace(uint32_t dwRace);
		uint32_t	GetRace();

		void	SetWeaponPower(uint32_t dwMinPower, uint32_t dwMaxPower, uint32_t dwMinMagicPower, uint32_t dwMaxMagicPower, uint32_t dwAddPower);
		void	SetStatus(uint32_t dwType, int32_t lValue);
		int32_t		GetStatus(uint32_t dwType);


		// Item
		void	MoveItemData(TItemPos SrcCell, TItemPos DstCell);
		void	SetItemData(TItemPos Cell, const TItemData & c_rkItemInst);
		const TItemData * GetItemData(TItemPos Cell) const;
		void	SetItemCount(TItemPos Cell, uint8_t byCount);
		void	SetItemMetinSocket(TItemPos Cell, uint32_t dwMetinSocketIndex, uint32_t dwMetinNumber);
		void	SetItemAttribute(TItemPos Cell, uint32_t dwAttrIndex, uint8_t byType, int16_t sValue);
		uint32_t	GetItemIndex(TItemPos Cell);
		uint32_t	GetItemFlags(TItemPos Cell);
		uint8_t	GetItemTypeBySlot(TItemPos Cell);
		uint8_t	GetItemSubTypeBySlot(TItemPos Cell);	
				uint32_t	GetItemCount(TItemPos Cell);
		uint32_t	GetItemCountByVnum(uint32_t dwVnum);
		uint32_t	GetItemMetinSocket(TItemPos Cell, uint32_t dwMetinSocketIndex);
		void	GetItemAttribute(TItemPos Cell, uint32_t dwAttrSlotIndex, uint8_t * pbyType, int16_t * psValue);
		void	SendClickItemPacket(uint32_t dwIID);

		void	RequestAddLocalQuickSlot(uint32_t dwLocalSlotIndex, uint32_t dwWndType, uint32_t dwWndItemPos);
		void	RequestAddToEmptyLocalQuickSlot(uint32_t dwWndType, uint32_t dwWndItemPos);
		void	RequestMoveGlobalQuickSlotToLocalQuickSlot(uint32_t dwGlobalSrcSlotIndex, uint32_t dwLocalDstSlotIndex);
		void	RequestDeleteGlobalQuickSlot(uint32_t dwGlobalSlotIndex);
		void	RequestUseLocalQuickSlot(uint32_t dwLocalSlotIndex);
		uint32_t	LocalQuickSlotIndexToGlobalQuickSlotIndex(uint32_t dwLocalSlotIndex);

		void	GetGlobalQuickSlotData(uint32_t dwGlobalSlotIndex, uint32_t* pdwWndType, uint32_t* pdwWndItemPos);
		void	GetLocalQuickSlotData(uint32_t dwSlotPos, uint32_t* pdwWndType, uint32_t* pdwWndItemPos);
		void	RemoveQuickSlotByValue(int32_t iType, int32_t iPosition);

		char	IsItem(TItemPos SlotIndex);

#ifdef ENABLE_NEW_EQUIPMENT_SYSTEM
		bool    IsBeltInventorySlot(TItemPos Cell);
#endif
		bool	IsInventorySlot(TItemPos SlotIndex);
		bool	IsEquipmentSlot(TItemPos SlotIndex);

		bool	IsEquipItemInSlot(TItemPos iSlotIndex);


		// Quickslot
		int32_t		GetQuickPage();
		void	SetQuickPage(int32_t nPageIndex);
		void	AddQuickSlot(int32_t QuickslotIndex, char IconType, char IconPosition);
		void	DeleteQuickSlot(int32_t QuickslotIndex);
		void	MoveQuickSlot(int32_t Source, int32_t Target);


		// Skill
		void	SetSkill(uint32_t dwSlotIndex, uint32_t dwSkillIndex);
		bool	GetSkillSlotIndex(uint32_t dwSkillIndex, uint32_t* pdwSlotIndex);
		int32_t		GetSkillIndex(uint32_t dwSlotIndex);
		int32_t		GetSkillGrade(uint32_t dwSlotIndex);
		int32_t		GetSkillLevel(uint32_t dwSlotIndex);
		float	GetSkillCurrentEfficientPercentage(uint32_t dwSlotIndex);
		float	GetSkillNextEfficientPercentage(uint32_t dwSlotIndex);
		void	SetSkillLevel(uint32_t dwSlotIndex, uint32_t dwSkillLevel);
		void	SetSkillLevel_(uint32_t dwSkillIndex, uint32_t dwSkillGrade, uint32_t dwSkillLevel);
		BOOL	IsToggleSkill(uint32_t dwSlotIndex);
		void	ClickSkillSlot(uint32_t dwSlotIndex);
		void	ChangeCurrentSkillNumberOnly(uint32_t dwSlotIndex);
		bool	FindSkillSlotIndexBySkillIndex(uint32_t dwSkillIndex, uint32_t * pdwSkillSlotIndex);

		void	SetSkillCoolTime(uint32_t dwSkillIndex);
		void	EndSkillCoolTime(uint32_t dwSkillIndex);

		float	GetSkillCoolTime(uint32_t dwSlotIndex);
		float	GetSkillElapsedCoolTime(uint32_t dwSlotIndex);
		BOOL	IsSkillActive(uint32_t dwSlotIndex);
		BOOL	IsSkillCoolTime(uint32_t dwSlotIndex);
		void	UseGuildSkill(uint32_t dwSkillSlotIndex);
		bool	AffectIndexToSkillSlotIndex(uint32_t uAffect, uint32_t* pdwSkillSlotIndex);
		bool	AffectIndexToSkillIndex(uint32_t dwAffectIndex, uint32_t * pdwSkillIndex);

		void	SetAffect(uint32_t uAffect);
		void	ResetAffect(uint32_t uAffect);
		void	ClearAffects();


		// Target
		void	SetTarget(uint32_t dwVID, BOOL bForceChange = TRUE, bool bIgnoreViewFrustrum = false);
		void	OpenCharacterMenu(uint32_t dwVictimActorID);
		uint32_t	GetTargetVID();


		// Party
		void	ExitParty();
		void	AppendPartyMember(uint32_t dwPID, const char * c_szName);
		void	LinkPartyMember(uint32_t dwPID, uint32_t dwVID);
		void	UnlinkPartyMember(uint32_t dwPID);
		void	UpdatePartyMemberInfo(uint32_t dwPID, uint8_t byState, uint8_t byHPPercentage);
		void	UpdatePartyMemberAffect(uint32_t dwPID, uint8_t byAffectSlotIndex, int16_t sAffectNumber);
		void	RemovePartyMember(uint32_t dwPID);
		bool	IsPartyMemberByVID(uint32_t dwVID);
		bool	IsPartyMemberByName(const char * c_szName);
		bool	GetPartyMemberPtr(uint32_t dwPID, TPartyMemberInfo ** ppPartyMemberInfo);
		bool	PartyMemberPIDToVID(uint32_t dwPID, uint32_t * pdwVID);
		bool	PartyMemberVIDToPID(uint32_t dwVID, uint32_t * pdwPID);
		bool	IsSamePartyMember(uint32_t dwVID1, uint32_t dwVID2);


		// Fight
		void	RememberChallengeInstance(uint32_t dwVID);
		void	RememberRevengeInstance(uint32_t dwVID);
		void	RememberCantFightInstance(uint32_t dwVID);
		void	ForgetInstance(uint32_t dwVID);
		bool	IsChallengeInstance(uint32_t dwVID);
		bool	IsRevengeInstance(uint32_t dwVID);
		bool	IsCantFightInstance(uint32_t dwVID);


		// Private Shop
		void	OpenPrivateShop();
		void	ClosePrivateShop();
		bool	IsOpenPrivateShop();



		// Stamina
		void	StartStaminaConsume(uint32_t dwConsumePerSec, uint32_t dwCurrentStamina);
		void	StopStaminaConsume(uint32_t dwCurrentStamina);


		// PK Mode
		uint32_t	GetPKMode();


		// Combo
		void	SetComboSkillFlag(BOOL bFlag);


		// System
		void	SetMovableGroundDistance(float fDistance);


		// Emotion
		void	ActEmotion(uint32_t dwEmotionID);
		void	StartEmotionProcess();
		void	EndEmotionProcess();


		// Function Only For Console System
		BOOL	__ToggleCoolTime();
		BOOL	__ToggleLevelLimit();

		__inline const	SAutoPotionInfo& GetAutoPotionInfo(int32_t type) const	{ return m_kAutoPotionInfo[type]; }
		__inline		SAutoPotionInfo& GetAutoPotionInfo(int32_t type)		{ return m_kAutoPotionInfo[type]; }
		__inline void					 SetAutoPotionInfo(int32_t type, const SAutoPotionInfo& info)	{ m_kAutoPotionInfo[type] = info; }		

	protected:
		TQuickSlot &	__RefLocalQuickSlot(int32_t SlotIndex);
		TQuickSlot &	__RefGlobalQuickSlot(int32_t SlotIndex);


		uint32_t	__GetLevelAtk();
		uint32_t	__GetStatAtk();
		uint32_t	__GetWeaponAtk(uint32_t dwWeaponPower);		
		uint32_t	__GetTotalAtk(uint32_t dwWeaponPower, uint32_t dwRefineBonus);
		uint32_t	__GetRaceStat();		
		uint32_t	__GetHitRate();
		uint32_t	__GetEvadeRate();

		void	__UpdateBattleStatus();

		void	__DeactivateSkillSlot(uint32_t dwSlotIndex);
		void	__ActivateSkillSlot(uint32_t dwSlotIndex);

		void	__OnPressSmart(CInstanceBase& rkInstMain, bool isAuto);
		void	__OnClickSmart(CInstanceBase& rkInstMain, bool isAuto);

		void	__OnPressItem(CInstanceBase& rkInstMain, uint32_t dwPickedItemID);
		void	__OnPressActor(CInstanceBase& rkInstMain, uint32_t dwPickedActorID, bool isAuto);
		void	__OnPressGround(CInstanceBase& rkInstMain, const TPixelPosition& c_rkPPosPickedGround);
		void	__OnPressScreen(CInstanceBase& rkInstMain);

		void	__OnClickActor(CInstanceBase& rkInstMain, uint32_t dwPickedActorID, bool isAuto);
		void	__OnClickItem(CInstanceBase& rkInstMain, uint32_t dwPickedItemID);
		void	__OnClickGround(CInstanceBase& rkInstMain, const TPixelPosition& c_rkPPosPickedGround);

		bool	__IsMovableGroundDistance(CInstanceBase& rkInstMain, const TPixelPosition& c_rkPPosPickedGround);

		bool	__GetPickedActorPtr(CInstanceBase** pkInstPicked);

		bool	__GetPickedActorID(uint32_t* pdwActorID);
		bool	__GetPickedItemID(uint32_t* pdwItemID);
		bool	__GetPickedGroundPos(TPixelPosition* pkPPosPicked);

		void	__ClearReservedAction();
		void	__ReserveClickItem(uint32_t dwItemID);
		void	__ReserveClickActor(uint32_t dwActorID);
		void	__ReserveClickGround(const TPixelPosition& c_rkPPosPickedGround);
		void	__ReserveUseSkill(uint32_t dwActorID, uint32_t dwSkillSlotIndex, uint32_t dwRange);

		void	__ReserveProcess_ClickActor();

		void	__ShowPickedEffect(const TPixelPosition& c_rkPPosPickedGround);

		void	__ClearAutoAttackTargetActorID();
		void	__SetAutoAttackTargetActorID(uint32_t dwActorID);

		void	NEW_ShowEffect(int32_t dwEID, TPixelPosition kPPosDst);

		void	NEW_SetMouseSmartState(int32_t eMBS, bool isAuto);
		void	NEW_SetMouseMoveState(int32_t eMBS);
		void	NEW_SetMouseCameraState(int32_t eMBS);
		void	NEW_GetMouseDirRotation(float fScrX, float fScrY, float* pfDirRot);
		void	NEW_GetMultiKeyDirRotation(bool isLeft, bool isRight, bool isUp, bool isDown, float* pfDirRot);

		float	GetDegreeFromDirection(int32_t iUD, int32_t iLR);
		float	GetDegreeFromPosition(int32_t ix, int32_t iy, int32_t iHalfWidth, int32_t iHalfHeight);

		void	NEW_RefreshMouseWalkingDirection();


		// Instances
		bool	__CanShot(CInstanceBase& rkInstMain, CInstanceBase& rkInstTarget);
		bool	__CanUseSkill();

		bool	__CanMove();
		
		bool	__CanAttack();
		bool	__CanChangeTarget();

		bool	__CheckSkillUsable(uint32_t dwSlotIndex);
		void	__UseCurrentSkill();
		bool	__UseSkill(uint32_t dwSlotIndex);
		bool	__CheckSpecialSkill(uint32_t dwSkillIndex);

		bool	__CheckRestSkillCoolTime(uint32_t dwSkillSlotIndex);
		bool	__CheckShortLife(TSkillInstance & rkSkillInst, CPythonSkill::TSkillData& rkSkillData);
		bool	__CheckShortMana(TSkillInstance & rkSkillInst, CPythonSkill::TSkillData& rkSkillData);
		bool	__CheckShortArrow(TSkillInstance & rkSkillInst, CPythonSkill::TSkillData& rkSkillData);
		bool	__CheckDashAffect(CInstanceBase& rkInstMain);

		void	__SendUseSkill(uint32_t dwSkillSlotIndex, uint32_t dwTargetVID);
		void	__RunCoolTime(uint32_t dwSkillSlotIndex);
		void	SendClickActorPacket(CInstanceBase& rkInstVictim);

		uint8_t	__GetSkillType(uint32_t dwSkillSlotIndex);

		bool	__IsReservedUseSkill(uint32_t dwSkillSlotIndex);
		bool	__IsMeleeSkill(CPythonSkill::TSkillData& rkSkillData);
		bool	__IsChargeSkill(CPythonSkill::TSkillData& rkSkillData);
		uint32_t	__GetSkillTargetRange(CPythonSkill::TSkillData& rkSkillData);
		bool	__SearchNearTarget();
		bool	__IsUsingChargeSkill();

		bool	__ProcessEnemySkillTargetRange(CInstanceBase& rkInstMain, CInstanceBase& rkInstTarget, CPythonSkill::TSkillData& rkSkillData, uint32_t dwSkillSlotIndex);


		// Item
		bool	__HasEnoughArrow();
		bool	__HasItem(uint32_t dwItemID);
		uint32_t	__GetPickableDistance();


		// Target
		CInstanceBase*		__GetTargetActorPtr();
		void				__ClearTarget();
		uint32_t				__GetTargetVID();
		void				__SetTargetVID(uint32_t dwVID);
		bool				__IsSameTargetVID(uint32_t dwVID);
		bool				__IsTarget();
		bool				__ChangeTargetToPickedInstance();

		CInstanceBase *		__GetSkillTargetInstancePtr(CPythonSkill::TSkillData& rkSkillData);
		CInstanceBase *		__GetAliveTargetInstancePtr();
		CInstanceBase *		__GetDeadTargetInstancePtr();

		BOOL				__IsRightButtonSkillMode();


		// Update
		void				__Update_AutoAttack();
		void				__Update_NotifyGuildAreaEvent();

		

		// Emotion
		BOOL				__IsProcessingEmotion();


	protected:
		PyObject *				m_ppyGameWindow;

		// Client Player Data
		std::map<uint32_t, uint32_t>	m_skillSlotDict;
		std::string				m_stName;
		uint32_t					m_dwMainCharacterIndex;		
		uint32_t					m_dwRace;
		uint32_t					m_dwWeaponMinPower;
		uint32_t					m_dwWeaponMaxPower;
		uint32_t					m_dwWeaponMinMagicPower;
		uint32_t					m_dwWeaponMaxMagicPower;
		uint32_t					m_dwWeaponAddPower;

		// Todo
		uint32_t					m_dwSendingTargetVID;
		float					m_fTargetUpdateTime;

		// Attack
		uint32_t					m_dwAutoAttackTargetVID;

		// NEW_Move
		EMode					m_eReservedMode;
		float					m_fReservedDelayTime;

		float					m_fMovDirRot;

		float					m_fLastClientFocusTime;

		bool					m_isUp;
		bool					m_isDown;
		bool					m_isLeft;
		bool					m_isRight;
		bool					m_isAtkKey;
		bool					m_isDirKey;
		bool					m_isCmrRot;
		bool					m_isSmtMov;
		bool					m_isDirMov;

		float					m_fCmrRotSpd;

		TPlayerStatus			m_playerStatus;

		uint32_t					m_iComboOld;
		uint32_t					m_dwVIDReserved;
		uint32_t					m_dwIIDReserved;

		uint32_t					m_dwcurSkillSlotIndex;
		uint32_t					m_dwSkillSlotIndexReserved;
		uint32_t					m_dwSkillRangeReserved;

		TPixelPosition			m_kPPosInstPrev;
		TPixelPosition			m_kPPosReserved;

		// Emotion
		BOOL					m_bisProcessingEmotion;

		// Dungeon
		BOOL					m_isDestPosition;
		int32_t						m_ixDestPos;
		int32_t						m_iyDestPos;
		int32_t						m_iLastAlarmTime;

		// Party
		std::map<uint32_t, TPartyMemberInfo>	m_PartyMemberMap;

		// PVP
		std::set<uint32_t>			m_ChallengeInstanceSet;
		std::set<uint32_t>			m_RevengeInstanceSet;
		std::set<uint32_t>			m_CantFightInstanceSet;

		// Private Shop
		bool					m_isOpenPrivateShop;
		bool					m_isObserverMode;

		// Free camera mode - [Think]
		bool					m_isFreeCameraMode;

		// Stamina
		BOOL					m_isConsumingStamina;
		float					m_fCurrentStamina;
		float					m_fConsumeStaminaPerSec;

		// Guild
		uint32_t					m_inGuildAreaID;

		// System
		BOOL					m_sysIsCoolTime;
		BOOL					m_sysIsLevelLimit;

	protected:
		// Game Cursor Data
		TPixelPosition			m_MovingCursorPosition;
		float					m_fMovingCursorSettingTime;
		uint32_t					m_adwEffect[EFFECT_NUM];

		uint32_t					m_dwVIDPicked;
		uint32_t					m_dwIIDPicked;
		int32_t						m_aeMBFButton[MBT_NUM];

		uint32_t					m_dwTargetVID;
		uint32_t					m_dwTargetEndTime;
		uint32_t					m_dwPlayTime;

		SAutoPotionInfo			m_kAutoPotionInfo[AUTO_POTION_TYPE_NUM];

	protected:
		float					MOVABLE_GROUND_DISTANCE;

	private:
		std::map<uint32_t, uint32_t> m_kMap_dwAffectIndexToSkillIndex;
};

extern const int32_t c_iFastestSendingCount;
extern const int32_t c_iSlowestSendingCount;
extern const float c_fFastestSendingDelay;
extern const float c_fSlowestSendingDelay;
extern const float c_fRotatingStepTime;

extern const float c_fComboDistance;
extern const float c_fPickupDistance;
extern const float c_fClickDistance;
