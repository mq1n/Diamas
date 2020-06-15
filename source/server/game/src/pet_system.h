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
	virtual bool	_UpdateFollowAI();				///< ������ ����ٴϴ� AI ó��
	virtual bool	_UpdatAloneActionAI(float fMinDist, float fMaxDist);			///< ���� ��ó���� ȥ�� ��� AI ó��

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
	// ���� �ִ� �Լ��� �ŵδ� �Լ�.
	// �̰� �� �����Ѱ�, ������ ����,
	// POINT_MOV_SPEED, POINT_ATT_SPEED, POINT_CAST_SPEED�� PointChange()�� �Լ��� �Ἥ ������ ���� �ҿ��� ���°�,
	// PointChange() ���Ŀ� ��𼱰� ComputePoints()�� �ϸ� �ϴ� �ʱ�ȭ�ǰ�,
	// �� �����, ComputePoints()�� �θ��� ������ Ŭ���� POINT�� ���� ������ �ʴ´ٴ� �Ŵ�.
	// �׷��� ������ �ִ� ���� ComputePoints() ���ο��� petsystem->RefreshBuff()�� �θ����� �Ͽ���,
	// ������ ���� ���� ClearBuff()�� �θ���, ComputePoints�� �ϴ� ������ �Ѵ�.
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
	typedef	std::unordered_map<uint32_t,	CPetActor*>		TPetActorMap;		/// <VNUM, PetActor> map. (�� ĳ���Ͱ� ���� vnum�� ���� ������ ���� ���� ������..??)

public:
	CPetSystem(LPCHARACTER owner);
	virtual ~CPetSystem();

	CPetActor*	GetByVID(uint32_t vid) const;
	CPetActor*	GetByVnum(uint32_t vnum) const;

	bool		Update(uint32_t deltaTime);
	void		Destroy();

	size_t		CountSummoned() const;			///< ���� ��ȯ��(��üȭ �� ĳ���Ͱ� �ִ�) ���� ����

public:
	CPetActor*	Summon(uint32_t mobVnum, LPITEM pSummonItem, const char* petName, bool bSpawnFar, uint32_t options = CPetActor::EPetOption_Followable | CPetActor::EPetOption_Summonable);

	void		Unsummon(uint32_t mobVnum, bool bDeleteFromList = false);

	void		DeletePet(uint32_t mobVnum);
	void		DeletePet(CPetActor* petActor);
	void		RefreshBuff();

private:
	TPetActorMap	m_petActorMap;
	LPCHARACTER		m_pkOwner;					///< �� �ý����� Owner
	uint32_t			m_dwUpdatePeriod;			///< ������Ʈ �ֱ� (ms����)
	uint32_t			m_dwLastUpdateTime;
	LPEVENT			m_pkPetSystemUpdateEvent;
};




#endif	//__HEADER_PET_SYSTEM__