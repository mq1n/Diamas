#pragma once

#include "Type.h"

class CEmitterProperty
{
	friend class CParticleSystemData;
	friend class CParticleSystemInstance;
	public:
		enum
		{
			EMITTER_SHAPE_POINT,
			EMITTER_SHAPE_ELLIPSE,
			EMITTER_SHAPE_SQUARE,
			EMITTER_SHAPE_SPHERE,
		};

		enum
		{
			EMITTER_ADVANCED_TYPE_FREE,
			EMITTER_ADVANCED_TYPE_OUTER,
			EMITTER_ADVANCED_TYPE_INNER,
		};

	public:
		CEmitterProperty();
		virtual ~CEmitterProperty();

		void Clear();

		uint32_t GetMaxEmissionCount()
		{
			return m_dwMaxEmissionCount;
		}
		
		float GetCycleLength()
		{
			return m_fCycleLength;
		}

		BOOL isCycleLoop()
		{
			return m_bCycleLoopFlag;
		}
			
		int32_t	GetLoopCount()
		{
			return m_iLoopCount;
		}
			

		uint8_t GetEmitterShape();
		uint8_t GetEmitterAdvancedType();
		BOOL isEmitFromEdge();

		void GetEmittingSize(float fTime, float * pfValue);
		void GetEmittingAngularVelocity(float fTime, float * pfValue);

		void GetEmittingDirectionX(float fTime, float * pfValue);
		void GetEmittingDirectionY(float fTime, float * pfValue);
		void GetEmittingDirectionZ(float fTime, float * pfValue);
		void GetEmittingVelocity(float fTime, float * pfValue);
		void GetEmissionCountPerSecond(float fTime, float * pfValue);
		void GetParticleLifeTime(float fTime, float * pfValue);
		void GetParticleSizeX(float fTime, float * pfValue);
		void GetParticleSizeY(float fTime, float * pfValue);

	/////

		uint32_t m_dwMaxEmissionCount;

		float m_fCycleLength;
		BOOL m_bCycleLoopFlag;
		int32_t	m_iLoopCount;

		uint8_t m_byEmitterShape;
		uint8_t m_byEmitterAdvancedType;
		BOOL m_bEmitFromEdgeFlag;
		D3DXVECTOR3 m_v3EmittingSize;
		float m_fEmittingRadius;

		D3DXVECTOR3 m_v3EmittingDirection;

		//TTimeEventTableFloat m_TimeEventEmittingRadius;
		TTimeEventTableFloat m_TimeEventEmittingSize;
		TTimeEventTableFloat m_TimeEventEmittingAngularVelocity;
		TTimeEventTableFloat m_TimeEventEmittingDirectionX;
		TTimeEventTableFloat m_TimeEventEmittingDirectionY;
		TTimeEventTableFloat m_TimeEventEmittingDirectionZ;
		TTimeEventTableFloat m_TimeEventEmittingVelocity;
		TTimeEventTableFloat m_TimeEventEmissionCountPerSecond;
		TTimeEventTableFloat m_TimeEventLifeTime;
		TTimeEventTableFloat m_TimeEventSizeX;
		TTimeEventTableFloat m_TimeEventSizeY;
};
