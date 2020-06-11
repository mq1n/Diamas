#pragma once

#include "EffectInstance.h"

class CEffectManager : public CScreen, public CSingleton<CEffectManager>
{
	public:
		enum EEffectType
		{
			EFFECT_TYPE_NONE				= 0,
			EFFECT_TYPE_PARTICLE			= 1,
			EFFECT_TYPE_ANIMATION_TEXTURE	= 2,
			EFFECT_TYPE_MESH				= 3,
			EFFECT_TYPE_SIMPLE_LIGHT		= 4,

			EFFECT_TYPE_MAX_NUM				= 4,
		};

		typedef std::map<uint32_t, CEffectData*> TEffectDataMap;
		typedef std::map<uint32_t, CEffectInstance*> TEffectInstanceMap;

	public:
		CEffectManager();
		virtual ~CEffectManager();

		void Destroy();

		void UpdateSound();
		void Update();
		void Render();

		void GetInfo(std::string* pstInfo);

		bool IsAliveEffect(uint32_t dwInstanceIndex);

		// Register
		BOOL RegisterEffect(const char * c_szFileName,bool isExistDelete=false,bool isNeedCache=false);
		BOOL RegisterEffect2(const char * c_szFileName, uint32_t* pdwRetCRC, bool isNeedCache=false);

		void DeleteAllInstances();

		// Usage
		int32_t CreateEffect(uint32_t dwID, const D3DXVECTOR3 & c_rv3Position, const D3DXVECTOR3 & c_rv3Rotation);
		int32_t CreateEffect(const char * c_szFileName, const D3DXVECTOR3 & c_rv3Position, const D3DXVECTOR3 & c_rv3Rotation);

		void CreateEffectInstance(uint32_t dwInstanceIndex, uint32_t dwID);
		BOOL SelectEffectInstance(uint32_t dwInstanceIndex);
		bool DestroyEffectInstance(uint32_t dwInstanceIndex);
		void DeactiveEffectInstance(uint32_t dwInstanceIndex);

		void SetEffectTextures(uint32_t dwID, std::vector<std::string> textures);
		void SetEffectInstancePosition(const D3DXVECTOR3 & c_rv3Position);
		void SetEffectInstanceRotation(const D3DXVECTOR3 & c_rv3Rotation);
		void SetEffectInstanceGlobalMatrix(const D3DXMATRIX & c_rmatGlobal);

		void ShowEffect();
		void HideEffect();

		// Temporary function
		uint32_t GetRandomEffect();
		int32_t GetEmptyIndex();
		bool GetEffectData(uint32_t dwID, CEffectData ** ppEffect);
		bool GetEffectData(uint32_t dwID, const CEffectData ** c_ppEffect);

		// Area에 직접 찍는 Effect용 함수... EffectInstance의 Pointer를 반환한다.
		// EffectManager 내부 EffectInstanceMap을 이용하지 않는다.
		void CreateUnsafeEffectInstance(uint32_t dwEffectDataID, CEffectInstance ** ppEffectInstance);
		bool DestroyUnsafeEffectInstance(CEffectInstance * pEffectInstance);

		int32_t GetRenderingEffectCount();

	protected:
		void __Initialize();

		void __DestroyEffectInstanceMap();
		void __DestroyEffectCacheMap();
		void __DestroyEffectDataMap();

	protected:
		bool m_isDisableSortRendering;
		TEffectDataMap					m_kEftDataMap;
		TEffectInstanceMap				m_kEftInstMap;
		TEffectInstanceMap				m_kEftCacheMap;

		CEffectInstance *				m_pSelectedEffectInstance;
};
