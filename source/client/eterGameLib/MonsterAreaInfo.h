// MonsterAreaInfo.h: interface for the CMonsterAreaInfo class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_MONSTERAREAINFO_H__B177E257_2EB2_4CE5_89D3_D06B5618BE1B__INCLUDED_)
#define AFX_MONSTERAREAINFO_H__B177E257_2EB2_4CE5_89D3_D06B5618BE1B__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CMonsterAreaInfo  
{
public:
	enum EMonsterAreaInfoType
	{
		MONSTERAREAINFOTYPE_INVALID,
		MONSTERAREAINFOTYPE_MONSTER,
		MONSTERAREAINFOTYPE_GROUP,
	};
	enum EMonsterDir
	{
		DIR_RANDOM = 0,
		DIR_NORTH,
		DIR_NORTHEAST,
		DIR_EAST,
		DIR_SOUTHEAST,
		DIR_SOUTH,
		DIR_SOUTHWEST,
		DIR_WEST,
		DIR_NORTHWEST
	};

	CMonsterAreaInfo();
	virtual ~CMonsterAreaInfo();

	void			SetID(uint32_t dwID)									{ m_dwID = dwID; }
	uint32_t			GetID()												{ return m_dwID; }

	void			Clear();

	//
	void			SetOrigin(int32_t lOriginX, int32_t lOriginY);
	void			GetOrigin(int32_t * plOriginX, int32_t * plOriginY);
	
	void			SetSize(int32_t lSizeX, int32_t lSizeY);
	void			GetSize(int32_t * plSizeX, int32_t * plSizeY);
	
	int32_t			GetLeft()											{ return m_lLeft; }
	int32_t			GetTop()											{ return m_lTop; }
	int32_t			GetRight()											{ return m_lRight; }
	int32_t			GetBottom()											{ return m_lBottom; }

public:
	void			SetMonsterAreaInfoType(EMonsterAreaInfoType eMonsterAreaInfoType) { m_eMonsterAreaInfoType = eMonsterAreaInfoType; }
	EMonsterAreaInfoType GetMonsterAreaInfoType() { return m_eMonsterAreaInfoType; }

public:
	void			SetMonsterGroupID(uint32_t dwGroupID) { m_dwGroupID = dwGroupID;}
	void			SetMonsterGroupName(std::string strGroupName) { m_strGroupName = strGroupName; }
	void			SetMonsterGroupLeaderName(std::string strGroupLeaderName) { m_strLeaderName = strGroupLeaderName; }
	void			SetMonsterGroupFollowerCount(uint32_t dwFollowerCount) { m_dwFollowerCount = dwFollowerCount; }

	uint32_t			GetMonsterGroupID() { return m_dwGroupID; }
	std::string		GetMonsterGroupName() { return m_strGroupName; }
	std::string		GetMonsterGroupLeaderName() { return m_strLeaderName; }
	uint32_t			GetMonsterGroupFollowerCount() { return m_dwFollowerCount; }

public:
	void			SetMonsterName(std::string strMonsterName) { m_strMonsterName = strMonsterName; }
	void			SetMonsterVID(uint32_t dwVID) { m_dwVID = dwVID; }

	std::string		GetMonsterName() { return m_strMonsterName;	}
	uint32_t			GetMonsterVID() { return m_dwVID; }

public:
	void			SetMonsterCount(uint32_t dwCount);
 	void			SetMonsterDirection(EMonsterDir eMonsterDir);
	void			RemoveAllMonsters();

	uint32_t			GetMonsterCount() { return m_dwMonsterCount; }
	EMonsterDir		GetMonsterDir() { return m_eMonsterDir; }
	D3DXVECTOR2		GetMonsterDirVector() { return m_v2Monsterdirection; }
	D3DXVECTOR2		GetTempMonsterPos(uint32_t dwIndex);

protected:
	void			SetLRTB();

protected:
	EMonsterAreaInfoType	m_eMonsterAreaInfoType;

	// Group Type 정보
	uint32_t			m_dwGroupID;
	std::string		m_strGroupName;
	std::string		m_strLeaderName;
	uint32_t			m_dwFollowerCount;

	// Monster Type 정보
	uint32_t			m_dwVID;
	std::string		m_strMonsterName;

	// 공통 정보
	uint32_t			m_dwMonsterCount;
	EMonsterDir		m_eMonsterDir;
	D3DXVECTOR2		m_v2Monsterdirection;

	uint32_t			m_dwID;

	int32_t			m_lOriginX;
	int32_t			m_lOriginY;
	int32_t			m_lSizeX;
	int32_t			m_lSizeY;

	int32_t			m_lLeft;			
	int32_t			m_lRight;
	int32_t			m_lTop;
	int32_t			m_lBottom;

	std::vector<D3DXVECTOR2> m_TempMonsterPosVector;
};

typedef std::vector<CMonsterAreaInfo *>					TMonsterAreaInfoPtrVector;
typedef TMonsterAreaInfoPtrVector::iterator				TMonsterAreaInfoPtrVectorIterator;

#endif // !defined(AFX_MONSTERAREAINFO_H__B177E257_2EB2_4CE5_89D3_D06B5618BE1B__INCLUDED_)
