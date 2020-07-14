#pragma once

#include <string>

#include "../../common/defines.h"
#include "../eterLib/TextFileLoader.h"
#include "../eterLib/CollisionData.h"

extern float g_fGameFPS;

extern std::string g_strResourcePath;
extern std::string g_strImagePath;

// Motion Key
#define MOTION_KEY									uint32_t
#define MAKE_MOTION_KEY(mode, index)				(((uint32_t)(((uint8_t)(mode)) << 24) | ((uint32_t)((uint16_t)(index))) << 8) | ((uint32_t)((uint8_t)(0))))
#define MAKE_RANDOM_MOTION_KEY(mode, index, type)	(((uint32_t)(((uint8_t)(mode)) << 24) | ((uint32_t)((uint16_t)(index))) << 8) | ((uint32_t)((uint8_t)(type))))
#define GET_MOTION_MODE(key)						((uint8_t)(((uint32_t)(key) >> 24) & 0xFF))
#define GET_MOTION_INDEX(key)						((uint16_t)(((uint32_t)(key) >> 8) & 0xFFFF))
#define GET_MOTION_SUB_INDEX(key)					((uint8_t)(((uint32_t)(key)) & 0xFF))
/*
#define MAKE_MOTION_KEY(mode, index)		MAKELONG(index, mode)
#define GET_MOTION_MODE(key)				HIWORD(key)
#define GET_MOTION_INDEX(key)				LOWORD(key)
*/

// Race Data
namespace NRaceData
{
	enum EJobs
	{
		JOB_WARRIOR,
		JOB_ASSASSIN,
		JOB_SURA,
		JOB_SHAMAN,
#ifdef ENABLE_WOLFMAN_CHARACTER
		JOB_WOLFMAN,
#endif
		JOB_MAX_NUM
	};

	// Character Attacking Data
	enum EAttackType
	{
		ATTACK_TYPE_SPLASH,
		ATTACK_TYPE_SNIPE
	};
	enum EHitType
	{
		HIT_TYPE_NONE,
		HIT_TYPE_GREAT,
		HIT_TYPE_GOOD
	};
	enum EMotionType
	{
		MOTION_TYPE_NONE,
		MOTION_TYPE_NORMAL,
		MOTION_TYPE_COMBO,
		MOTION_TYPE_SKILL
	};

	typedef CDynamicSphereInstance THitTimePosition;
	typedef std::map<float, THitTimePosition> THitTimePositionMap;

	typedef struct SAttackData
	{
		int32_t iAttackType;
		int32_t iHittingType;
		float fInvisibleTime;
		float fExternalForce;
		float fStiffenTime;
		int32_t iHitLimitCount;
	} TAttackData;

	typedef struct SHitData
	{
		float fAttackStartTime;
		float fAttackEndTime;

		float fWeaponLength;
		std::string strBoneName;

		THitTimePositionMap mapHitPosition;

		bool Load(CTextFileLoader & rTextFileLoader);
	} THitData;

	typedef std::vector<THitData> THitDataContainer;

	typedef struct SMotionAttackData : public SAttackData
	{
		int32_t iMotionType;

		THitDataContainer HitDataContainer;
	} TMotionAttackData;

	/////////////////////////////////////////////////////////////////////////////////
	// Character Attaching Collision Data
	enum ECollisionType
	{
		COLLISION_TYPE_NONE,

		COLLISION_TYPE_BODY,
		COLLISION_TYPE_ATTACKING,
		COLLISION_TYPE_DEFENDING,
		COLLISION_TYPE_SPLASH
	};
	enum ECollisionShape
	{
		COLLISION_SHAPE_SPHERE,
		COLLISION_SHAPE_CYLINDER
	};
	
	typedef struct SCollisionData
	{
		int32_t					iCollisionType;
		CSphereCollisionInstanceVector	SphereDataVector;

		SCollisionData(): iCollisionType(0)
		{}
		virtual ~SCollisionData() {}
	} TCollisionData;

	typedef struct SAttachingEffectData
	{
		std::string			strFileName;
		D3DXVECTOR3			v3Position;
		D3DXVECTOR3			v3Rotation;

		SAttachingEffectData() {}
		virtual ~SAttachingEffectData() {}
	} TAttachingEffectData;

	typedef struct SAttachingObjectData
	{
		std::string			strFileName;

		SAttachingObjectData() {}
		virtual ~SAttachingObjectData() {}
	} TAttachingObjectData;

	typedef std::vector<TCollisionData> TCollisionDataVector;

	void DestroySystem();

	void SaveAttackData(FILE * File, int32_t iTabCount, const TAttackData & c_rData);
	void SaveMotionAttackData(FILE * File, int32_t iTabCount, const TMotionAttackData & c_rData);
	void SaveCollisionData(FILE * File, int32_t iTabCount, const TCollisionData & c_rCollisionData);
	void SaveEffectData(FILE * File, int32_t iTabCount, const TAttachingEffectData & c_rEffectData);
	void SaveObjectData(FILE * File, int32_t iTabCount, const TAttachingObjectData & c_rObjectData);

	BOOL LoadAttackData(CTextFileLoader & rTextFileLoader, TAttackData * pData);
	BOOL LoadMotionAttackData(CTextFileLoader & rTextFileLoader, TMotionAttackData * pData);
	BOOL LoadCollisionData(CTextFileLoader & rTextFileLoader, TCollisionData * pCollisionData);
	BOOL LoadEffectData(CTextFileLoader & rTextFileLoader, TAttachingEffectData * pEffectData);
	BOOL LoadObjectData(CTextFileLoader & rTextFileLoader, TAttachingObjectData * pObjectData);

	/////////////////////////////////////////////////////////////////////////////////
	// Attaching Data
	enum EAttachingDataType
	{
		ATTACHING_DATA_TYPE_NONE,

		ATTACHING_DATA_TYPE_COLLISION_DATA,
		ATTACHING_DATA_TYPE_EFFECT,
		ATTACHING_DATA_TYPE_OBJECT,

		ATTACHING_DATA_TYPE_MAX_NUM
	};

	typedef struct SAttachingData
	{
		SAttachingData() 
			: dwType(0)
			, isAttaching(false)
			, dwAttachingModelIndex(0)
			, pCollisionData(nullptr)
			, pEffectData(nullptr)
			, pObjectData(nullptr)
		{}
		uint32_t dwType;

		BOOL isAttaching;
		uint32_t dwAttachingModelIndex;
		std::string strAttachingBoneName;

		TCollisionData * pCollisionData;
		TAttachingEffectData * pEffectData;
		TAttachingObjectData * pObjectData;
	} TAttachingData;

	typedef std::vector<TAttachingData> TAttachingDataVector;

	void SaveAttachingData(FILE * File, int32_t iTabCount, const TAttachingDataVector & c_rAttachingDataVector);
	BOOL LoadAttachingData(CTextFileLoader & rTextFileLoader, TAttachingDataVector * pAttachingDataVector);

	extern CDynamicPool<TCollisionData>			g_CollisionDataPool;
	extern CDynamicPool<TAttachingEffectData>	g_EffectDataPool;
	extern CDynamicPool<TAttachingObjectData>	g_ObjectDataPool;
};
