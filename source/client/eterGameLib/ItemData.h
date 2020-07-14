#pragma once
#include "../../common/defines.h"
#include "../../common/item_data.h"
#include "../eterLib/GrpSubImage.h"
#include "../eterGrnLib/Thing.h"

class CItemData
{
	public:
		CItemData();
		virtual ~CItemData();

		void Clear();
		void SetSummary(const std::string& c_rstSumm);
		void SetDescription(const std::string& c_rstDesc);

		CGraphicThing * GetModelThing();
		CGraphicThing * GetSubModelThing();
		CGraphicThing * GetDropModelThing();
		CGraphicSubImage * GetIconImage(const std::string& stIconFileName = "");

		uint32_t GetLODModelThingCount();
		BOOL GetLODModelThingPointer(uint32_t dwIndex, CGraphicThing ** ppModelThing);

		uint32_t GetAttachingDataCount();
		BOOL GetCollisionDataPointer(uint32_t dwIndex, const NRaceData::TAttachingData ** c_ppAttachingData);
		BOOL GetAttachingDataPointer(uint32_t dwIndex, const NRaceData::TAttachingData ** c_ppAttachingData);

		/////
		const SItemTable* GetTable() const;
		uint32_t GetIndex() const;
		const char * GetName() const;
		const char * GetDescription() const;
		const char * GetSummary() const;
		uint8_t GetType() const;
		uint8_t GetSubType() const;
		uint32_t GetRefine() const;
		const char* GetUseTypeString() const;
		uint32_t GetWeaponType() const;
		uint8_t GetSize() const;
		BOOL IsAntiFlag(uint32_t dwFlag) const;
		BOOL IsFlag(uint32_t dwFlag) const;
		BOOL IsWearableFlag(uint32_t dwFlag) const;
		BOOL HasNextGrade() const;

		uint32_t GetFlags() const;
		uint32_t GetAntiFlags() const;
		uint32_t GetWearFlags() const;
		uint32_t GetIBuyItemPrice() const;
		uint32_t GetISellItemPrice() const;
		int32_t GetLevelLimit() const;
		BOOL GetLimit(uint8_t byIndex, TItemLimit* pItemLimit) const;
		BOOL GetApply(uint8_t byIndex, TItemApply* pItemApply) const;
		int32_t GetValue(uint8_t byIndex) const;
		int32_t GetSocket(uint8_t byIndex) const;
		int32_t SetSocket(uint8_t byIndex,uint32_t value);
		int32_t GetSocketCount() const;
		uint32_t GetIconNumber() const;

		uint32_t	GetSpecularPoweru() const;
		float	GetSpecularPowerf() const;
	
		/////

		BOOL IsEquipment() const;

		/////

		//BOOL LoadItemData(const char * c_szFileName);
		void SetDefaultItemData(const char * c_szIconFileName, const char * c_szModelFileName  = nullptr);
		void SetItemTableData(SItemTable* pItemTable);

#ifdef ENABLE_ACCE_SYSTEM
		void SetItemScale(const std::string strJob, const std::string strSex, const std::string strScaleX, const std::string strScaleY, const std::string strScaleZ, const std::string strPositionX, const std::string strPositionY, const std::string strPositionZ);
		bool GetItemScale(uint32_t dwPos, float & fScaleX, float & fScaleY, float & fScaleZ, float & fPositionX, float & fPositionY, float & fPositionZ);
#endif

		bool IsStackable() const { return IsFlag(ITEM_FLAG_STACKABLE) && !IsAntiFlag(ITEM_ANTIFLAG_STACK); }
	protected:
		void __LoadFiles();
		void __SetIconImage(const char * c_szFileName, bool renew = false);

	protected:
		std::string m_strModelFileName;
		std::string m_strSubModelFileName;
		std::string m_strDropModelFileName;
		std::string m_strIconFileName;
		std::string m_strDescription;
		std::string m_strSummary;
		std::vector<std::string> m_strLODModelFileNameVector;

		CGraphicThing * m_pModelThing;
		CGraphicThing * m_pSubModelThing;
		CGraphicThing * m_pDropModelThing;
		CGraphicSubImage * m_pIconImage;
		std::vector<CGraphicThing *> m_pLODModelThingVector;

		NRaceData::TAttachingDataVector m_AttachingDataVector;
		uint32_t		m_dwVnum;
		SItemTable m_ItemTable;
#ifdef ENABLE_ACCE_SYSTEM
		TScaleTable	m_ScaleTable;
#endif
		
	public:
		static void DestroySystem();

		static CItemData* New();
		static void Delete(CItemData* pkItemData);

		static CDynamicPool <CItemData>		ms_kPool;
};
