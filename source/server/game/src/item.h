#ifndef __INC_METIN_II_GAME_ITEM_H__
#define __INC_METIN_II_GAME_ITEM_H__

#include "../../common/tables.h"
#include "entity.h"
#include "gposition.h"

class CItem : public CEntity
{
	protected:
		// override methods from ENTITY class
		virtual void	EncodeInsertPacket(LPENTITY entity);
		virtual void	EncodeRemovePacket(LPENTITY entity);

	public:
		CItem(uint32_t dwVnum);
		virtual ~CItem();

		int32_t			GetLevelLimit();

		bool		CheckItemUseLevel(int32_t nLevel);

		int32_t		FindApplyValue(uint8_t bApplyType);

		bool		IsStackable()		{ return (GetFlag() & ITEM_FLAG_STACKABLE)?true:false; }

		void		Initialize();
		void		Destroy();

		void		Save();

		void		SetWindow(uint8_t b)	{ m_bWindow = b; }
		uint8_t		GetWindow()		{ return m_bWindow; }

		void		SetID(uint32_t id)		{ m_dwID = id;	}
		uint32_t		GetID()			{ return m_dwID; }

		void			SetProto(const TItemTable * table);
		TItemTable const *	GetProto() const	{ return m_pProto; }

		int32_t		GetGold();
		int32_t		GetShopBuyPrice();
		const char *	GetName()  const { return m_pProto ? m_pProto->szLocaleName : nullptr; }
		const char *	GetBaseName() const		{ return m_pProto ? m_pProto->szName : nullptr; }
		uint8_t		GetSize()		{ return m_pProto ? m_pProto->bSize : 0;	}

		void		SetFlag(int32_t flag)	{ m_lFlag = flag;	}
		int32_t		GetFlag()		{ return m_lFlag;	}

		void		AddFlag(int32_t bit);
		void		RemoveFlag(int32_t bit);

		uint32_t		GetWearFlag()		{ return m_pProto ? m_pProto->dwWearFlags : 0; }
		uint32_t		GetAntiFlag()		{ return m_pProto ? m_pProto->dwAntiFlags : 0; }
		uint32_t		GetImmuneFlag()		{ return m_pProto ? m_pProto->dwImmuneFlag : 0; }

		void		SetVID(uint32_t vid)	{ m_dwVID = vid;	}
		uint32_t		GetVID()		{ return m_dwVID;	}

		bool		SetCount(uint32_t count);
		uint32_t		GetCount();

		// GetVnum과 GetOriginalVnum에 대한 comment
		// GetVnum은 Masking 된 Vnum이다. 이를 사용함으로써, 아이템의 실제 Vnum은 10이지만, Vnum이 20인 것처럼 동작할 수 있는 것이다.
		// Masking 값은 ori_to_new.txt에서 정의된 값이다.
		// GetOriginalVnum은 아이템 고유의 Vnum으로, 로그 남길 때, 클라이언트에 아이템 정보 보낼 때, 저장할 때는 이 Vnum을 사용하여야 한다.
		// 
		uint32_t		GetVnum() const		{ return m_dwMaskVnum ? m_dwMaskVnum : m_dwVnum;	}
		uint32_t		GetOriginalVnum() const		{ return m_dwVnum;	}
		uint8_t		GetType() const		{ return m_pProto ? m_pProto->bType : 0;	}
		uint8_t		GetSubType() const	{ return m_pProto ? m_pProto->bSubType : 0;	}
		uint8_t		GetLimitType(uint32_t idx) const { return m_pProto ? m_pProto->aLimits[idx].bType : 0;	}
		int32_t		GetLimitValue(uint32_t idx) const { return m_pProto ? m_pProto->aLimits[idx].lValue : 0;	}
		int32_t		GetLevel() const { return m_pProto ? (m_pProto->aLimits[0].bType == LIMIT_LEVEL ?  m_pProto->aLimits[0].lValue : 0) : 0 ; }

		// New Getters for some Code refactoring
		
		bool IsSocketModifyingItem() const
		{
			return GetType() == ITEM_USE && 
				(
					GetSubType() == USE_PUT_INTO_BELT_SOCKET || 
					GetSubType() == USE_PUT_INTO_RING_SOCKET ||
					GetSubType() == USE_PUT_INTO_ACCESSORY_SOCKET ||
					GetSubType() == USE_ADD_ACCESSORY_SOCKET || 
					GetSubType() == USE_CLEAN_SOCKET
				);
		}

		// Weapon
		bool IsWeapon() const;

		bool IsSword() const { return GetType() == ITEM_WEAPON && GetSubType() == WEAPON_SWORD; }
		bool IsDagger() const { return GetType() == ITEM_WEAPON && GetSubType() == WEAPON_DAGGER; }
		bool IsBow() const { return GetType() == ITEM_WEAPON && GetSubType() == WEAPON_BOW; }
		bool IsTwoHandSword() const { return GetType() == ITEM_WEAPON && GetSubType() == WEAPON_TWO_HANDED; }
		bool IsBell() const { return GetType() == ITEM_WEAPON && GetSubType() == WEAPON_BELL; }
		bool IsFan() const { return GetType() == ITEM_WEAPON && GetSubType() == WEAPON_FAN; }
		bool IsArrow() const { return GetType() == ITEM_WEAPON && GetSubType() == WEAPON_ARROW; }
		bool IsMountSpear() const { return GetType() == ITEM_WEAPON && GetSubType() == WEAPON_MOUNT_SPEAR; }
#ifdef ENABLE_WOLFMAN_CHARACTER
		bool IsClaw() const { return GetType() == ITEM_WEAPON && GetSubType() == WEAPON_CLAW; }
#endif
		bool IsQuiver() const { return GetType() == ITEM_WEAPON && GetSubType() == WEAPON_NUM_TYPES; }

		// Armor
		bool IsArmor ()		const { return GetType() == ITEM_ARMOR; }
		bool IsArmorBody()	const { return GetType() == ITEM_ARMOR && GetSubType() == ARMOR_BODY; }
		bool IsHelmet()		const { return GetType() == ITEM_ARMOR && GetSubType() == ARMOR_HEAD; }
		bool IsShield()		const { return GetType() == ITEM_ARMOR && GetSubType() == ARMOR_SHIELD; }
		bool IsWrist()		const { return GetType() == ITEM_ARMOR && GetSubType() == ARMOR_WRIST; }
		bool IsShoe()		const { return GetType() == ITEM_ARMOR && GetSubType() == ARMOR_FOOTS; }
		bool IsNecklace()	const { return GetType() == ITEM_ARMOR && GetSubType() == ARMOR_NECK; }
		bool IsEarRing()	const { return GetType() == ITEM_ARMOR && GetSubType() == ARMOR_EAR; }

		bool IsRing()			const { return GetType() == ITEM_RING; }
		bool IsCostume()		const { return GetType() == ITEM_COSTUME;  }
//		bool IsCostumeMount()	const { return GetType() == ITEM_COSTUME && GetSubType() == COSTUME_MOUNT; }
		bool IsCostumeHair()	const { return GetType() == ITEM_COSTUME && GetSubType() == COSTUME_HAIR; }
		bool IsCostumeBody()	const { return GetType() == ITEM_COSTUME && GetSubType() == COSTUME_BODY; }
#ifdef ENABLE_ACCE_SYSTEM
		bool IsCostumeAcce()	const { return GetType() == ITEM_COSTUME && GetSubType() == COSTUME_ACCE; }
#endif
//		bool IsCostumeWeapon()	const { return GetType() == ITEM_COSTUME && GetSubType() == COSTUME_WEAPON; }
		bool IsOldHair()		const { return GetVnum() >= 74001 && GetVnum() <= 75620; }
		bool IsCostumeModifyItem() const { return GetType() == ITEM_USE && (GetSubType() == USE_CHANGE_COSTUME_ATTR || GetSubType() == USE_RESET_COSTUME_ATTR); }

		bool IsBelt() const { return GetType() == ITEM_BELT; }

		int32_t	GetValue(uint32_t idx) const;

		template < typename T >
		T GetValue(uint32_t idx)
		{
			assert(idx < ITEM_VALUES_MAX_NUM);

			return static_cast<T>(GetProto()->alValues[idx]);
		}

		void		SetCell(LPCHARACTER ch, uint16_t pos)	{ m_pOwner = ch, m_wCell = pos;	}
		uint16_t		GetCell()				{ return m_wCell;	}

		LPITEM		RemoveFromCharacter();
		bool		AddToCharacter(LPCHARACTER ch, const TItemPos &Cell);
		LPCHARACTER	GetOwner()		{ return m_pOwner; }

		LPITEM		RemoveFromGround();
		bool		AddToGround(int32_t lMapIndex, const GPOS & pos, bool skipOwnerCheck = false);

		int32_t			FindEquipCell(LPCHARACTER ch, int32_t bCandidateCell = -1);
		bool		IsEquipped() const		{ return m_bEquipped;	}
		bool		EquipTo(LPCHARACTER ch, uint8_t bWearCell);
		bool		IsEquipable() const;

		bool		CanUsedBy(LPCHARACTER ch);

		bool		DistanceValid(LPCHARACTER ch);

		void		UpdatePacket();
		void		UsePacketEncode(LPCHARACTER ch, LPCHARACTER victim, SPacketGCItemUse* packet);

		void		SetExchanging(bool isOn = true);
		bool		IsExchanging()		{ return m_bExchanging;	}

		bool		IsTwohanded();

		bool		IsPolymorphItem();

		void		ModifyPoints(bool bAdd);	// 아이템의 효과를 캐릭터에 부여 한다. bAdd가 false이면 제거함

		bool		CreateSocket(uint8_t bSlot, uint8_t bGold);
		const int32_t *	GetSockets()		{ return &m_alSockets[0];	}
		int32_t		GetSocket(int32_t i)	{ return m_alSockets[i];	}

		void		SetSockets(const int32_t * al);
		void		SetSocket(int32_t i, int32_t v, bool bLog = true);

		int32_t		GetSocketCount();
		bool		AddSocket();

		const TPlayerItemAttribute* GetAttributes()		{ return m_aAttr;	} 
		const TPlayerItemAttribute& GetAttribute(int32_t i)	{ return m_aAttr[i];	}

		uint8_t		GetAttributeType(int32_t i)	{ return m_aAttr[i].bType;	}
		int16_t		GetAttributeValue(int32_t i){ return m_aAttr[i].sValue;	}

		void		SetAttributes(const TPlayerItemAttribute* c_pAttribute);
		
		int32_t		FindAttribute(uint8_t bType);
		bool		RemoveAttributeAt(int32_t index);
		bool		RemoveAttributeType(uint8_t bType);

		bool		HasAttr(uint8_t bApply);
		bool		HasRareAttr(uint8_t bApply);

		void		SetDestroyEvent(LPEVENT pkEvent);
		void		StartDestroyEvent(int32_t iSec=90);

		uint32_t		GetRefinedVnum()	{ return m_pProto ? m_pProto->dwRefinedVnum : 0; }
		uint32_t		GetRefineFromVnum();
		int32_t		GetRefineLevel();

		void		SetSkipSave(bool b)	{ m_bSkipSave = b; }
		bool		GetSkipSave()		{ return m_bSkipSave; }

		bool		IsOwnership(LPCHARACTER ch);
		void		SetOwnership(LPCHARACTER ch, int32_t iSec = 10);
		void		SetOwnershipEvent(LPEVENT pkEvent);

		uint32_t	GetLastOwnerPID()	{ return m_dwLastOwnerPID; }
		void		SetLastOwnerPID(uint32_t pid) { m_dwLastOwnerPID = pid; }

		int32_t		GetAttributeSetIndex(); // 속성 붙는것을 지정한 배열의 어느 인덱스를 사용하는지 돌려준다.
		void		AlterToMagicItem();
		void		AlterToSocketItem(int32_t iSocketCount);

		uint16_t		GetRefineSet()		{ return m_pProto ? m_pProto->wRefineSet : 0;	}

		void		StartUniqueExpireEvent();
		void		SetUniqueExpireEvent(LPEVENT pkEvent);

		void		StartTimerBasedOnWearExpireEvent();
		void		SetTimerBasedOnWearExpireEvent(LPEVENT pkEvent);

		void		StartRealTimeExpireEvent();
		bool		IsRealTimeItem();
		bool		IsUsedTimeItem();

		void		StopUniqueExpireEvent();
		void		StopTimerBasedOnWearExpireEvent();
		void		StopAccessorySocketExpireEvent();

		//			일단 REAL_TIME과 TIMER_BASED_ON_WEAR 아이템에 대해서만 제대로 동작함.
		int32_t			GetDuration();

		int32_t		GetAttributeCount();
		void		ClearAttribute();
		void		ChangeAttribute(const int32_t* aiChangeProb=nullptr);
		void		AddAttribute();
		void		AddAttribute(uint8_t bType, int16_t sValue);

		void		ApplyAddon(int32_t iAddonType);

		int32_t		GetSpecialGroup() const;
		bool	IsSameSpecialGroup(const LPITEM item) const;

		// ACCESSORY_REFINE
		// 액세서리에 광산을 통해 소켓을 추가
		bool		IsAccessoryForSocket();

		int32_t		GetAccessorySocketGrade();
		int32_t		GetAccessorySocketMaxGrade();
		int32_t		GetAccessorySocketDownGradeTime();

		void		SetAccessorySocketGrade(int32_t iGrade);
		void		SetAccessorySocketMaxGrade(int32_t iMaxGrade);
		void		SetAccessorySocketDownGradeTime(uint32_t time);

		void		AccessorySocketDegrade();

		// 악세사리 를 아이템에 밖았을때 타이머 돌아가는것( 구리, 등 )
		void		StartAccessorySocketExpireEvent();
		void		SetAccessorySocketExpireEvent(LPEVENT pkEvent);

		bool		CanPutInto(LPITEM item);
		// END_OF_ACCESSORY_REFINE

		void		CopyAttributeTo(LPITEM pItem);
		void		CopySocketTo(LPITEM pItem);

		int32_t			GetRareAttrCount();
		bool		AddRareAttribute();
		bool		ChangeRareAttribute();

		void		AttrLog();

		void		Lock(bool f) { m_isLocked = f; }
		bool		isLocked() const { return m_isLocked; }

	private :
		void		SetAttribute(int32_t i, uint8_t bType, int16_t sValue);
	public:
		void		SetForceAttribute(int32_t i, uint8_t bType, int16_t sValue);

	protected:
		bool		EquipEx(bool is_equip);
		bool		Unequip();

		void		AddAttr(uint8_t bApply, uint8_t bLevel);
		void		PutAttribute(const int32_t * aiAttrPercentTable);
		void		PutAttributeWithLevel(uint8_t bLevel);

	public:
		void		AddRareAttribute2(const int32_t * aiAttrPercentTable = nullptr);
	protected:
		void		AddRareAttr(uint8_t bApply, uint8_t bLevel);
		void		PutRareAttribute(const int32_t * aiAttrPercentTable);
		void		PutRareAttributeWithLevel(uint8_t bLevel);

	protected:
		friend class CInputDB;
		bool		OnAfterCreatedItem();			// 서버상에 아이템이 모든 정보와 함께 완전히 생성(로드)된 후 불리우는 함수.

	public:
		bool		IsRideItem();
		bool		IsRamadanRing();

		void		ClearMountAttributeAndAffect();
		bool		IsNewMountItem();

		// 독일에서 기존 캐시 아이템과 같지만, 교환 가능한 캐시 아이템을 만든다고 하여,
		// 오리지널 아이템에, 교환 금지 플래그만 삭제한 새로운 아이템들을 새로운 아이템 대역에 할당하였다.
		// 문제는 새로운 아이템도 오리지널 아이템과 같은 효과를 내야하는데,
		// 서버건, 클라건, vnum 기반으로 되어있어
		// 새로운 vnum을 죄다 서버에 새로 다 박아야하는 안타까운 상황에 맞닿았다.
		// 그래서 새 vnum의 아이템이면, 서버에서 돌아갈 때는 오리지널 아이템 vnum으로 바꿔서 돌고 하고,
		// 저장할 때에 본래 vnum으로 바꿔주도록 한다.

		// Mask vnum은 어떤 이유(ex. 위의 상황)로 인해 vnum이 바뀌어 돌아가는 아이템을 위해 있다.
		void		SetMaskVnum(uint32_t vnum)	{	m_dwMaskVnum = vnum; }
		uint32_t		GetMaskVnum()			{	return m_dwMaskVnum; }
		bool		IsMaskedItem()	{	return m_dwMaskVnum != 0;	}

		// 용혼석
		bool		IsDragonSoul();
		int32_t		GiveMoreTime_Per(float fPercent);
		int32_t		GiveMoreTime_Fix(uint32_t dwTime);

	private:
		TItemTable const * m_pProto;		// 프로토 타잎

		uint32_t		m_dwVnum;
		LPCHARACTER	m_pOwner;

		uint8_t		m_bWindow;		// 현재 아이템이 위치한 윈도우 
		uint32_t		m_dwID;			// 고유번호
		bool		m_bEquipped;	// 장착 되었는가?
		uint32_t		m_dwVID;		// VID
		uint16_t		m_wCell;		// 위치
		uint32_t		m_dwCount;		// 개수
		int32_t		m_lFlag;		// 추가 flag
		uint32_t		m_dwLastOwnerPID;	// 마지막 가지고 있었던 사람의 PID

		bool		m_bExchanging;	///< 현재 교환중 상태 

		int32_t		m_alSockets[ITEM_SOCKET_MAX_NUM];	// 아이템 소캣
		TPlayerItemAttribute	m_aAttr[ITEM_ATTRIBUTE_MAX_NUM];

		LPEVENT		m_pkDestroyEvent;
		LPEVENT		m_pkExpireEvent;
		LPEVENT		m_pkUniqueExpireEvent;
		LPEVENT		m_pkTimerBasedOnWearExpireEvent;
		LPEVENT		m_pkRealTimeExpireEvent;
		LPEVENT		m_pkAccessorySocketExpireEvent;
		LPEVENT		m_pkOwnershipEvent;

		uint32_t		m_dwOwnershipPID;

		bool		m_bSkipSave;

		bool		m_isLocked;
		
		uint32_t		m_dwMaskVnum;
		uint32_t		m_dwSIGVnum;
	public:
		void SetSIGVnum(uint32_t dwSIG)
		{
			m_dwSIGVnum = dwSIG;
		}
		uint32_t	GetSIGVnum() const
		{
			return m_dwSIGVnum;
		}

	private:
		enum EGMOwnerTypes : uint8_t
		{
			GM_OWNER_UNSET,
			GM_OWNER_PLAYER,
			GM_OWNER_GM,
		};
		uint8_t		m_bIsGMOwner;
	public:
		bool		IsGMOwner() const { return m_bIsGMOwner == GM_OWNER_GM; }
		void		SetGMOwner(bool bGMOwner) { m_bIsGMOwner = bGMOwner ? GM_OWNER_GM : GM_OWNER_PLAYER; }
};

EVENTINFO(item_event_info)
{
	LPITEM item;
	char szOwnerName[CHARACTER_NAME_MAX_LEN];

	item_event_info() 
	: item( 0 )
	{
		::memset( szOwnerName, 0, CHARACTER_NAME_MAX_LEN );
	}
};

EVENTINFO(item_vid_event_info)
{
	uint32_t item_vid;

	item_vid_event_info() 
	: item_vid( 0 )
	{
	}
};

#endif
