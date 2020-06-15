#pragma once
#include <d3dx9.h>

#include "../eterLib/GrpScreen.h"
#include "../eterLib/Resource.h"
#include "../eterLib/GrpImageInstance.h"
#include "../eterLib/TextFileLoader.h"

#include "Type.h"
#include "EffectElementBase.h"

class CEffectMesh : public CResource
{
	public:
		typedef struct SEffectFrameData
		{
			uint8_t byChangedFrame;
			float fVisibility;
			uint32_t dwVertexCount;
			uint32_t dwTextureVertexCount;
			uint32_t dwIndexCount;
			std::vector<TPTVertex> PDTVertexVector;
		} TEffectFrameData;

		typedef struct SEffectMeshData
		{
			char szObjectName[32];
			char szDiffuseMapFileName[128];

			std::vector<TEffectFrameData> EffectFrameDataVector;
			std::vector<CGraphicImage*> pImageVector;

			static SEffectMeshData* New();
			static void Delete(SEffectMeshData* pkData);
			
			static void DestroySystem();

			static CDynamicPool<SEffectMeshData> ms_kPool;
		} TEffectMeshData;

	// About Resource Code
	public:
		using TRef = CRef<CEffectMesh>;

	public:
		static TType Type();

	public:
		CEffectMesh(const FileSystem::CFileName& filename);
		virtual ~CEffectMesh();

		uint32_t GetFrameCount();
		uint32_t GetMeshCount();
		TEffectMeshData * GetMeshDataPointer(uint32_t dwMeshIndex);
		
		std::vector<CGraphicImage*>* GetTextureVectorPointer(uint32_t dwMeshIndex);
		std::vector<CGraphicImage*>& GetTextureVectorReference(uint32_t dwMeshIndex);

		// Exceptional function for tool
		BOOL GetMeshElementPointer(uint32_t dwMeshIndex, TEffectMeshData ** ppMeshData);

	protected:
		bool OnLoad(int32_t iSize, const void * c_pvBuf);

		void OnClear();	
		bool OnIsEmpty() const;
		bool OnIsType(TType type);		

		BOOL __LoadData_Ver001(int32_t iSize, const uint8_t * c_pbBuf);
		BOOL __LoadData_Ver002(int32_t iSize, const uint8_t * c_pbBuf);

	protected:
		int32_t								m_iGeomCount;
		int32_t								m_iFrameCount;
		std::vector<TEffectMeshData *>	m_pEffectMeshDataVector;

		bool							m_isData;		
};

class CEffectMeshScript : public CEffectElementBase
{
	public:
		typedef struct SMeshData
		{
			uint8_t byBillboardType;

			BOOL bBlendingEnable;
			uint8_t byBlendingSrcType;
			uint8_t byBlendingDestType;
			BOOL bTextureAlphaEnable;

			uint8_t byColorOperationType;
			D3DXCOLOR ColorFactor;

			BOOL bTextureAnimationLoopEnable;
			float fTextureAnimationFrameDelay;

			uint32_t dwTextureAnimationStartFrame;
			
			TTimeEventTableFloat TimeEventAlpha;
			
			SMeshData()
			{
				TimeEventAlpha.clear();
			}
		} TMeshData;
		using TMeshDataVector = std::vector<TMeshData>;

	public:
		CEffectMeshScript();
		virtual ~CEffectMeshScript();

		const char * GetMeshFileName();

		void ReserveMeshData(uint32_t dwMeshCount);
		bool CheckMeshIndex(uint32_t dwMeshIndex);
		bool GetMeshDataPointer(uint32_t dwMeshIndex, TMeshData ** ppMeshData);
		int32_t GetMeshDataCount();

		int32_t GetBillboardType(uint32_t dwMeshIndex);
		BOOL isBlendingEnable(uint32_t dwMeshIndex);
		uint8_t GetBlendingSrcType(uint32_t dwMeshIndex);
		uint8_t GetBlendingDestType(uint32_t dwMeshIndex);
		BOOL isTextureAlphaEnable(uint32_t dwMeshIndex);
		BOOL GetColorOperationType(uint32_t dwMeshIndex, uint8_t * pbyType);
		BOOL GetColorFactor(uint32_t dwMeshIndex, D3DXCOLOR * pColor);
		BOOL GetTimeTableAlphaPointer(uint32_t dwMeshIndex, TTimeEventTableFloat ** pTimeEventAlpha);

		BOOL isMeshAnimationLoop();
		BOOL GetMeshAnimationLoopCount();
		float GetMeshAnimationFrameDelay();
		BOOL isTextureAnimationLoop(uint32_t dwMeshIndex);
		float GetTextureAnimationFrameDelay(uint32_t dwMeshIndex);
		uint32_t GetTextureAnimationStartFrame(uint32_t dwMeshIndex);

	protected:
		void OnClear();
		bool OnIsData();
		BOOL OnLoadScript(CTextFileLoader & rTextFileLoader);

	protected:
		BOOL m_isMeshAnimationLoop;
		int32_t m_iMeshAnimationLoopCount;
		float m_fMeshAnimationFrameDelay;
		TMeshDataVector m_MeshDataVector;

		std::string m_strMeshFileName;

	public:
		static void DestroySystem();

		static CEffectMeshScript* New();
		static void Delete(CEffectMeshScript* pkData);

		static CDynamicPool<CEffectMeshScript> ms_kPool;	
};