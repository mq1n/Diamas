#pragma once

#include "../eterGrnLib/Thing.h"

class CRaceMotionData;
class CAttributeData;

#define COMBO_KEY									uint32_t
#define MAKE_COMBO_KEY(motion_mode, combo_type)		(	(uint32_t(motion_mode) << 16) | (uint32_t(combo_type))	)
#define COMBO_KEY_GET_MOTION_MODE(key)				(	uint16_t(uint32_t(key) >> 16 & 0xFFFF)						)
#define COMBO_KEY_GET_COMBO_TYPE(key)				(	uint16_t(uint32_t(key) & 0xFFFF)							)

class CRaceData
{
	public:
		enum EParts
		{
			// Share index with server 
			// ECharacterEquipmentPart도 수정해주세요.
			//패킷 크기가 변합니다 서버와 상의후 추가해주세요.
			PART_MAIN,
			PART_WEAPON,
			PART_HEAD,
			PART_WEAPON_LEFT,
			PART_HAIR,

#ifdef ENABLE_ACCE_SYSTEM
			PART_ACCE,
#endif

			PART_MAX_NUM,
		};
		
		enum
		{
			SMOKE_NUM = 4, 
		};

		/////////////////////////////////////////////////////////////////////////////////
		// Graphic Resource

		// Model
		typedef std::map<uint16_t, CGraphicThing*> TGraphicThingMap;
		typedef std::map<uint32_t, std::string> TAttachingBoneNameMap;

		// Motion
		typedef struct SMotion
		{
			uint8_t byPercentage;
			CGraphicThing * pMotion;
			CRaceMotionData * pMotionData;
		} TMotion;
		typedef std::vector<TMotion> TMotionVector;
		typedef std::map<uint16_t, TMotionVector> TMotionVectorMap;

		typedef struct SMotionModeData
		{
			uint16_t wMotionModeIndex;

			TMotionVectorMap MotionVectorMap;
			
			SMotionModeData() {}
			virtual ~SMotionModeData() {}
		} TMotionModeData;
		typedef std::map<uint16_t, TMotionModeData*> TMotionModeDataMap;
		typedef TMotionModeDataMap::iterator TMotionModeDataIterator; 

		/////////////////////////////////////////////////////////////////////////////////
		// Model Data
		typedef struct SModelData
		{
			NRaceData::TAttachingDataVector AttachingDataVector;
		} TModelData;
		typedef std::map<uint32_t, TModelData> TModelDataMap;
		typedef TModelDataMap::iterator TModelDataMapIterator;

		/////////////////////////////////////////////////////////////////////////////////
		// Motion Data
		typedef std::map<uint32_t, CRaceMotionData*> TMotionDataMap;

		/////////////////////////////////////////////////////////////////////////////////
		// Combo Data
		typedef std::vector<uint32_t> TComboIndexVector;
		typedef struct SComboAttackData
		{
			TComboIndexVector ComboIndexVector;
		} TComboData;
		typedef std::map<uint32_t, uint32_t> TNormalAttackIndexMap;
		typedef std::map<COMBO_KEY, TComboData> TComboAttackDataMap;
		typedef TComboAttackDataMap::iterator TComboAttackDataIterator;

		struct SSkin
		{
			int32_t m_ePart;

			std::string m_stSrcFileName;
			std::string m_stDstFileName;

			SSkin()
			{
				m_ePart=0;
			}
			SSkin(const SSkin& c_rkSkin)
			{
				Copy(c_rkSkin);
			}
			void operator=(const SSkin& c_rkSkin)
			{
				Copy(c_rkSkin);
			}
			void Copy(const SSkin& c_rkSkin)
			{
				m_ePart=c_rkSkin.m_ePart;
				m_stSrcFileName=c_rkSkin.m_stSrcFileName;
				m_stDstFileName=c_rkSkin.m_stDstFileName;
			}
		};

		struct SHair
		{
			std::string m_stModelFileName;
			std::vector<SSkin> m_kVct_kSkin;
		};

		struct SShape
		{
			std::string m_stModelFileName;
			std::vector<SSkin> m_kVct_kSkin;
		};

	public:
		static CRaceData* New();
		static void Delete(CRaceData* pkRaceData);
		static void CreateSystem(uint32_t uCapacity, uint32_t uMotModeCapacity);
		static void DestroySystem();

	public:
		CRaceData();
		virtual ~CRaceData();

		void Destroy();

		// Codes For Client
		const char* GetBaseModelFileName() const;
		const char* GetAttributeFileName() const;
		const char* GetMotionListFileName() const;
		CGraphicThing * GetBaseModelThing();
		CGraphicThing * GetLODModelThing();
		CAttributeData * GetAttributeDataPtr();
		BOOL GetAttachingBoneName(uint32_t dwPartIndex, const char ** c_pszBoneName);
		BOOL CreateMotionModeIterator(TMotionModeDataIterator & itor);
		BOOL NextMotionModeIterator(TMotionModeDataIterator & itor);

		BOOL GetMotionKey(uint16_t wMotionModeIndex, uint16_t wMotionIndex, MOTION_KEY * pMotionKey);

		BOOL GetMotionModeDataPointer(uint16_t wMotionMode, TMotionModeData ** ppMotionModeData);
		BOOL GetModelDataPointer(uint32_t dwModelIndex, const TModelData ** c_ppModelData);
		BOOL GetMotionVectorPointer(uint16_t wMotionMode, uint16_t wMotionIndex, const TMotionVector ** c_ppMotionVector);
		BOOL GetMotionDataPointer(uint16_t wMotionMode, uint16_t wMotionIndex, uint16_t wMotionSubIndex, CRaceMotionData** ppMotionData);
		BOOL GetMotionDataPointer(uint32_t dwMotionKey, CRaceMotionData ** ppMotionData);

		uint32_t GetAttachingDataCount();
		BOOL GetAttachingDataPointer(uint32_t dwIndex, const NRaceData::TAttachingData ** c_ppAttachingData);
		BOOL GetCollisionDataPointer(uint32_t dwIndex, const NRaceData::TAttachingData ** c_ppAttachingData);
		BOOL GetBodyCollisionDataPointer(const NRaceData::TAttachingData ** c_ppAttachingData);

		BOOL IsTree();
		const char * GetTreeFileName();

		///////////////////////////////////////////////////////////////////
		// Setup by Script
		BOOL LoadRaceData(const char * c_szFileName);

		CGraphicThing* RegisterMotionData(uint16_t wMotionMode, uint16_t wMotionIndex, const char * c_szFileName, uint8_t byPercentage = 100);

		///////////////////////////////////////////////////////////////////
		// Setup by Python
		void SetRace(uint32_t dwRaceIndex);
		void RegisterAttachingBoneName(uint32_t dwPartIndex, const char * c_szBoneName);

		void RegisterMotionMode(uint16_t wMotionModeIndex);
		void SetMotionModeParent(uint16_t wParentMotionModeIndex, uint16_t wMotionModeIndex);
		void OLD_RegisterMotion(uint16_t wMotionModeIndex, uint16_t wMotionIndex, const char * c_szFileName, uint8_t byPercentage = 100);
		CGraphicThing* NEW_RegisterMotion(CRaceMotionData* pkMotionData, uint16_t wMotionModeIndex, uint16_t wMotionIndex, const char * c_szFileName, uint8_t byPercentage = 100);
		bool SetMotionRandomWeight(uint16_t wMotionModeIndex, uint16_t wMotionIndex, uint16_t wMotionSubIndex, uint8_t byPercentage);

		void RegisterNormalAttack(uint16_t wMotionModeIndex, uint16_t wMotionIndex);
		BOOL GetNormalAttackIndex(uint16_t wMotionModeIndex, uint16_t * pwMotionIndex);

		void ReserveComboAttack(uint16_t wMotionModeIndex, uint16_t wComboType, uint32_t dwComboCount);
		void RegisterComboAttack(uint16_t wMotionModeIndex, uint16_t wComboType, uint32_t dwComboIndex, uint16_t wMotionIndex);
		BOOL GetComboDataPointer(uint16_t wMotionModeIndex, uint16_t wComboType, TComboData ** ppComboData);
		
		void SetShapeModel(uint32_t eShape, const char* c_szModelFileName);
		void AppendShapeSkin(uint32_t eShape, uint32_t ePart, const char* c_szSrcFileName, const char* c_szDstFileName);

		void SetHairSkin(uint32_t eHair, uint32_t ePart, const char* c_szModelFileName, const char* c_szSrcFileName, const char* c_szDstFileName);

		/////

		uint32_t GetSmokeEffectID(uint32_t eSmoke);

		const std::string& GetSmokeBone();

		SHair* FindHair(uint32_t eHair);
		SShape* FindShape(uint32_t eShape);

	protected:
		void __Initialize();

		void __OLD_RegisterMotion(uint16_t wMotionMode, uint16_t wMotionIndex, const TMotion & rMotion);

		BOOL GetMotionVectorPointer(uint16_t wMotionMode, uint16_t wMotionIndex, TMotionVector ** ppMotionVector);

	protected:
		uint32_t m_dwRaceIndex;
		uint32_t m_adwSmokeEffectID[SMOKE_NUM];
		
		CGraphicThing * m_pBaseModelThing;
		CGraphicThing * m_pLODModelThing;

		std::string m_strBaseModelFileName;
		std::string m_strTreeFileName;
		std::string m_strAttributeFileName;
		std::string m_strMotionListFileName;
		std::string m_strSmokeBoneName;

		TModelDataMap m_ModelDataMap;
		TMotionModeDataMap m_pMotionModeDataMap;
		TAttachingBoneNameMap m_AttachingBoneNameMap;		
		TComboAttackDataMap m_ComboAttackDataMap;
		TNormalAttackIndexMap m_NormalAttackIndexMap;
	
		std::map<uint32_t, SHair> m_kMap_dwHairKey_kHair;
		std::map<uint32_t, SShape> m_kMap_dwShapeKey_kShape;

		NRaceData::TAttachingDataVector m_AttachingDataVector;		

	protected:	
		static CDynamicPool<TMotionModeData>	ms_MotionModeDataPool;
		static CDynamicPool<CRaceData>			ms_kPool;
};
