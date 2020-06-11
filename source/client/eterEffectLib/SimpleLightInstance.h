#pragma once

#include "../eterLib/GrpScreen.h"

#include "EffectElementBaseInstance.h"
#include "SimpleLightData.h"

class CLightInstance : public CEffectElementBaseInstance
{
	public:
		friend class CLightData;

		CLightInstance();
		virtual ~CLightInstance();

	protected:
		
		void OnSetDataPointer(CEffectElementBase * pElement);

		void OnInitialize();
		void OnDestroy();

		bool OnUpdate(float fElapsedTime);
		void OnRender();
		
		uint32_t			m_LightID;
		CLightData *	m_pData;
		uint32_t			m_dwRangeIndex;

		uint32_t			m_iLoopCount;
		
	public:
		static void DestroySystem();

		static CLightInstance* New();
		static void Delete(CLightInstance* pkData);

		static CDynamicPool<CLightInstance>		ms_kPool;	
};