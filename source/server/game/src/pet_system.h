#ifndef	__HEADER_PET_SYSTEM__
#define	__HEADER_PET_SYSTEM__

class CHARACTER;

/**
*/
class CPetActor //: public CHARACTER
{
public:
	enum EPetOptions
	{
		EPetOption_Followable		= 1 << 0,
		EPetOption_Mountable		= 1 << 1,
		EPetOption_Summonable		= 1 << 2,
		EPetOption_Combatable		= 1 << 3,
	};


protected:
	friend class CPetSystem;

	CPetActor(LPCHARACTER owner, uint32_t vnum, uint32_t options = EPetOption_Followable | EPetOption_Summonable);

	virtual ~CPetActor();

	virtual bool	Update(uint32_t deltaTime);

protected:
	virtual bool	_UpdateFollowAI();				///< 주인을 따라다니는 AI 처리
	virtual bool	_UpdatAloneActionAI(float fMinDist, float fMaxDist);			///< 주인 근처에서 혼자 노는 AI 처리

private:
	bool Follow(float fMinDistance = 50.f);

public:
	LPCHARACTER		GetCharacter()	const					{ return m_pkChar; }
	LPCHARACTER		GetOwner()	const						{ return m_pkOwner; }
	uint32_t			GetVID() const							{ return m_dwVID; }
	uint32_t			GetVnum() const							{ return m_dwVnum; }

	bool			HasOption(EPetOptions option) const		{ return m_dwOptions & option; }

	void			SetName(const char* petName);

	uint32_t			Summon(const char* petName, LPITEM pSummonItem, bool bSpawnFar = false);
	void			Unsummon();

	bool			IsSummoned() const			{ return 0 != m_pkChar; }
	void			SetSummonItem (LPITEM pItem);
	uint32_t			GetSummonItemVID () { return m_dwSummonItemVID; }
	// 버프 주는 함수와 거두는 함수.
	// 이게 좀 괴랄한게, 서버가 ㅄ라서,
	// POINT_MOV_SPEED, POINT_ATT_SPEED, POINT_CAST_SPEED는 PointChange()란 함수만 써서 변경해 봐야 소용이 없는게,
	// PointChange() 이후에 어디선가 ComputePoints()를 하면 싹다 초기화되고,
	// 더 웃긴건, ComputePoints()를 부르지 않으면 클라의 POINT는 전혀 변하지 않는다는 거다.
	// 그래서 버프를 주는 것은 ComputePoints() 내부에서 petsystem->RefreshBuff()를 부르도록 하였고,
	// 버프를 빼는 것은 ClearBuff()를 부르고, ComputePoints를 하는 것으로 한다.
	void			GiveBuff();
	void			ClearBuff();

private:
	uint32_t			m_dwVnum;
	uint32_t			m_dwVID;
	uint32_t			m_dwOptions;
	uint32_t			m_dwLastActionTime;
	uint32_t			m_dwSummonItemVID;
	uint32_t			m_dwSummonItemVnum;

	int16_t			m_originalMoveSpeed;

	std::string		m_name;

	LPCHARACTER		m_pkChar;					// Instance of pet(CHARACTER)
	LPCHARACTER		m_pkOwner;
};

/**
*/
class CPetSystem
{
public:
	typedef	std::unordered_map<uint32_t,	CPetActor*>		TPetActorMap;		/// <VNUM, PetActor> map. (한 캐릭터가 같은 vnum의 펫을 여러개 가질 일이 있을까..??)

public:
	CPetSystem(LPCHARACTER owner);
	virtual ~CPetSystem();

	CPetActor*	GetByVID(uint32_t vid) const;
	CPetActor*	GetByVnum(uint32_t vnum) const;

	bool		Update(uint32_t deltaTime);
	void		Destroy();

	size_t		CountSummoned() const;			///< 현재 소환된(실체화 된 캐릭터가 있는) 펫의 개수

public:
	CPetActor*	Summon(uint32_t mobVnum, LPITEM pSummonItem, const char* petName, bool bSpawnFar, uint32_t options = CPetActor::EPetOption_Followable | CPetActor::EPetOption_Summonable);

	void		Unsummon(uint32_t mobVnum, bool bDeleteFromList = false);

	void		DeletePet(uint32_t mobVnum);
	void		DeletePet(CPetActor* petActor);
	void		RefreshBuff();

private:
	TPetActorMap	m_petActorMap;
	LPCHARACTER		m_pkOwner;					///< 펫 시스템의 Owner
	uint32_t			m_dwUpdatePeriod;			///< 업데이트 주기 (ms단위)
	uint32_t			m_dwLastUpdateTime;
	LPEVENT			m_pkPetSystemUpdateEvent;
};




#endif	//__HEADER_PET_SYSTEM__
