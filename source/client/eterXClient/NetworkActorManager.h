#pragma once

#include "InstanceBase.h"

struct SNetworkActorData
{
	std::string m_stName;
	
	CAffectFlagContainer	m_kAffectFlags;

	uint8_t	m_bType;
	uint32_t	m_dwVID;
	uint32_t	m_dwStateFlags;
	uint32_t	m_dwEmpireID;
	uint32_t	m_dwRace;
	uint32_t	m_dwMovSpd;
	uint32_t	m_dwAtkSpd;
	float	m_fRot;
	int32_t	m_lCurX;
	int32_t	m_lCurY;
	int32_t	m_lSrcX;
	int32_t	m_lSrcY;
	int32_t	m_lDstX;
	int32_t	m_lDstY;
	

	uint32_t	m_dwServerSrcTime;
	uint32_t	m_dwClientSrcTime;
	uint32_t	m_dwDuration;

	uint32_t	m_dwArmor;
	uint32_t	m_dwWeapon;
	uint32_t	m_dwHair;
#ifdef ENABLE_ACCE_SYSTEM
	uint32_t	m_dwAcce;
#endif

	uint32_t	m_dwOwnerVID;

	int16_t	m_sAlignment;
	uint8_t	m_byPKMode;
	uint32_t	m_dwMountVnum;

	uint32_t	m_dwGuildID;
	uint32_t	m_dwLevel;
	
	SNetworkActorData();

	void SetDstPosition(uint32_t dwServerTime, int32_t lDstX, int32_t lDstY, uint32_t dwDuration);
	void SetPosition(int32_t lPosX, int32_t lPosY);
	void UpdatePosition();	

	// NETWORK_ACTOR_DATA_COPY
	SNetworkActorData(const SNetworkActorData& src);
	void operator=(const SNetworkActorData& src);
	void __copy__(const SNetworkActorData& src);	
	// END_OF_NETWORK_ACTOR_DATA_COPY
};

struct SNetworkMoveActorData
{
	uint32_t	m_dwVID;
	uint32_t	m_dwTime;
	int32_t	m_lPosX;
	int32_t	m_lPosY;
	float	m_fRot;
	uint32_t	m_dwFunc;
	uint32_t	m_dwArg;
	uint32_t	m_dwDuration;

	SNetworkMoveActorData()
	{
		m_dwVID=0;
		m_dwTime=0;
		m_fRot=0.0f;
		m_lPosX=0;
		m_lPosY=0;
		m_dwFunc=0;
		m_dwArg=0;
		m_dwDuration=0;
	}
};

struct SNetworkUpdateActorData
{
	uint32_t m_dwVID;
	uint32_t m_dwGuildID;
	uint32_t m_dwArmor;
	uint32_t m_dwWeapon;
	uint32_t m_dwHair;
#ifdef ENABLE_ACCE_SYSTEM
	uint32_t	m_dwAcce;
#endif
	uint32_t m_dwMovSpd;
	uint32_t m_dwAtkSpd;
	int16_t m_sAlignment;
	uint8_t m_byPKMode;
	uint32_t m_dwMountVnum;
	uint32_t m_dwStateFlags; // 본래 Create 때만 쓰이는 변수임
	CAffectFlagContainer m_kAffectFlags;

	SNetworkUpdateActorData()
	{
		m_dwGuildID=0;
		m_dwVID=0;
		m_dwArmor=0;
		m_dwWeapon=0;
		m_dwHair=0;
#ifdef ENABLE_ACCE_SYSTEM
		m_dwAcce = 0;
#endif
		m_dwMovSpd=0;
		m_dwAtkSpd=0;
		m_sAlignment=0;
		m_byPKMode=0;
		m_dwMountVnum=0;
		m_dwStateFlags=0;
		m_kAffectFlags.Clear();
	}
};

class CPythonCharacterManager;

class CNetworkActorManager : public CReferenceObject
{
	public:
		CNetworkActorManager();
		virtual ~CNetworkActorManager();

		void Destroy();

		void SetMainActorVID(uint32_t dwVID);

		void RemoveActor(uint32_t dwVID);
		void AppendActor(const SNetworkActorData& c_rkNetActorData);
		void UpdateActor(const SNetworkUpdateActorData& c_rkNetUpdateActorData);
		void MoveActor(const SNetworkMoveActorData& c_rkNetMoveActorData);

		void SyncActor(uint32_t dwVID, int32_t lPosX, int32_t lPosY);
		void SetActorOwner(uint32_t dwOwnerVID, uint32_t dwVictimVID);

		void Update();

	protected:
		void __OLD_Update();

		void __UpdateMainActor();

		bool __IsVisiblePos(int32_t lPosX, int32_t lPosY);
		bool __IsVisibleActor(const SNetworkActorData& c_rkNetActorData);
		bool __IsMainActorVID(uint32_t dwVID);

		void __RemoveAllGroundItems();
		void __RemoveAllActors();
		void __RemoveDynamicActors();
		void __RemoveCharacterManagerActor(SNetworkActorData& rkNetActorData);

		CInstanceBase* __AppendCharacterManagerActor(SNetworkActorData& rkNetActorData);
		CInstanceBase* __FindActor(SNetworkActorData& rkNetActorData);
		CInstanceBase* __FindActor(SNetworkActorData& rkNetActorData, int32_t lDstX, int32_t lDstY);

		CPythonCharacterManager& __GetCharacterManager();

	protected:
		uint32_t m_dwMainVID;

		int32_t m_lMainPosX;
		int32_t m_lMainPosY;

		std::map<uint32_t, SNetworkActorData> m_kNetActorDict;
};
