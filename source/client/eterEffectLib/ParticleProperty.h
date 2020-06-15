#pragma once

#include <vector>
#include "../eterLib/GrpImageInstance.h"

#include "Type.h"

class CParticleProperty
{
	friend class CParticleSystemData;
	friend class CParticleSystemInstance;
	public:
		enum
		{
			ROTATION_TYPE_NONE,
			ROTATION_TYPE_TIME_EVENT,
			ROTATION_TYPE_CW,
			ROTATION_TYPE_CCW,
			ROTATION_TYPE_RANDOM_DIRECTION
		};

		enum
		{
			TEXTURE_ANIMATION_TYPE_NONE,
			TEXTURE_ANIMATION_TYPE_CW,
			TEXTURE_ANIMATION_TYPE_CCW,
			TEXTURE_ANIMATION_TYPE_RANDOM_FRAME,
			TEXTURE_ANIMATION_TYPE_RANDOM_DIRECTION
		};

	public:
		CParticleProperty();
		virtual ~CParticleProperty();

		void Clear();

		void InsertTexture(const char * c_szFileName);
		bool SetTexture(const char * c_szFileName);

		__forceinline uint8_t GetTextureAnimationType() const { return m_byTexAniType; }

		__forceinline uint32_t GetTextureAnimationFrameCount() const { return m_ImageVector.size(); }

		__forceinline float GetTextureAnimationFrameDelay() const { return m_fTexAniDelay; }


		uint8_t m_byTexAniType;
		float m_fTexAniDelay;
		BOOL m_bTexAniRandomStartFrameFlag;

		uint8_t m_bySrcBlendType;
		uint8_t m_byDestBlendType;
		uint8_t m_byColorOperationType;
		uint8_t m_byBillboardType;

		uint8_t m_byRotationType;
		float m_fRotationSpeed;
		uint16_t m_wRotationRandomStartingBegin;
		uint16_t m_wRotationRandomStartingEnd;

		BOOL m_bAttachFlag;
		BOOL m_bStretchFlag;

		TTimeEventTableFloat m_TimeEventGravity;
		TTimeEventTableFloat m_TimeEventAirResistance;

		TTimeEventTableFloat m_TimeEventScaleX;
		TTimeEventTableFloat m_TimeEventScaleY;
#ifdef WORLD_EDITOR
		TTimeEventTableFloat m_TimeEventColorRed;
		TTimeEventTableFloat m_TimeEventColorGreen;
		TTimeEventTableFloat m_TimeEventColorBlue;
		TTimeEventTableFloat m_TimeEventAlpha;
		
		std::vector<std::string> m_TextureNameVector;
#else
		TTimeEventTableColor m_TimeEventColor;
#endif
		TTimeEventTableFloat m_TimeEventRotation;

		std::vector<CGraphicImage*> m_ImageVector;
		
		CParticleProperty & operator = ( const CParticleProperty& c_ParticleProperty );
		
		// pre-transformed variables
		D3DXVECTOR3 m_v3ZAxis;
};
