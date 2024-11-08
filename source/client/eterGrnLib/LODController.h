#pragma once

#pragma warning(disable:4786)

#include <deque>
#include "Thing.h"
#include "ModelInstance.h"

class CGrannyLODController : public CGraphicBase
{
	public:
		static void SetMinLODMode(bool isEnable);		

	public:
		struct FSetLocalTime
		{
			float fLocalTime;
			void operator() (CGrannyLODController * pController)
			{
				pController->SetLocalTime(fLocalTime);
			}
		};

		struct FUpdateTime
		{
			float fElapsedTime;
			
			void operator() (CGrannyLODController * pController)
			{
				if (pController->isModelInstance())
					pController->UpdateTime(fElapsedTime);
			}
		};

		struct FUpdateLODLevel
		{
			float fDistanceFromCenter;
			float fDistanceFromCamera;

			void operator() (CGrannyLODController * pController)
			{
				if (pController->isModelInstance())
					pController->UpdateLODLevel(fDistanceFromCenter, fDistanceFromCamera);
			}
		};

		struct FRenderWithOneTexture
		{
			void operator() (CGrannyLODController * pController)
			{
				if (pController->isModelInstance())
					pController->RenderWithOneTexture();
			}
		};

		struct FBlendRenderWithOneTexture
		{
			void operator() (CGrannyLODController * pController)
			{
				if (pController->isModelInstance())
					pController->BlendRenderWithOneTexture();
			}
		};

		struct FRenderWithTwoTexture
		{
			void operator() (CGrannyLODController * pController)
			{
				if (pController->isModelInstance())
					pController->RenderWithTwoTexture();
			}
		};

		struct FBlendRenderWithTwoTexture
		{
			void operator() (CGrannyLODController * pController)
			{
				if (pController->isModelInstance())
					pController->BlendRenderWithTwoTexture();
			}
		};

		struct FRenderToShadowMap
		{
			void operator() (CGrannyLODController * pController)
			{
				if (pController->isModelInstance())
					pController->RenderToShadowMap();
			}
		};

		struct FRenderShadow
		{
			void operator() (CGrannyLODController * pController)
			{
				if (pController->isModelInstance())
					pController->RenderShadow();
			}
		};
		
		struct FDeform
		{
			const D3DXMATRIX * mc_pWorldMatrix;
			
			void operator() (CGrannyLODController * pController)
			{
				if (pController->isModelInstance())
					pController->Deform(mc_pWorldMatrix);
			}
		};
		struct FDeformNoSkin
		{
			const D3DXMATRIX * mc_pWorldMatrix;
			
			void operator() (CGrannyLODController * pController)
			{
				if (pController->isModelInstance())
					pController->DeformNoSkin(mc_pWorldMatrix);
			}
		};
		struct FDeformAll
		{
			const D3DXMATRIX * mc_pWorldMatrix;
			
			void operator() (CGrannyLODController * pController)
			{
				if (pController->isModelInstance())
					pController->DeformAll(mc_pWorldMatrix);
			}
		};

		struct FCreateDeviceObjects
		{
			void operator() (CGrannyLODController * pController)
			{
				if (pController->isModelInstance())
					pController->CreateDeviceObjects();
			}
		};

		struct FDestroyDeviceObjects
		{
			void operator() (CGrannyLODController * pController)
			{
				if (pController->isModelInstance())
					pController->DestroyDeviceObjects();
			}
		};

		struct FBoundBox
		{
			D3DXVECTOR3* m_vtMin;
			D3DXVECTOR3* m_vtMax;

			FBoundBox(D3DXVECTOR3 * vtMin, D3DXVECTOR3 * vtMax)
			{
				m_vtMin = vtMin;
				m_vtMax = vtMax;
			}

			void operator() (CGrannyLODController * pController)
			{
				if (pController->isModelInstance())
					pController->GetBoundBox(m_vtMin, m_vtMax);
			}
		};

		struct FResetLocalTime
		{
			void operator() (CGrannyLODController * pController)
			{
				if (pController->isModelInstance())
					pController->ResetLocalTime();
			}
		};

		struct FReloadTexture
		{
			void operator () (CGrannyLODController * pController)
			{
				if (pController->isModelInstance())
					pController->ReloadTexture();
			}
		};

		struct FSetMotionPointer
		{
			const CGrannyMotion *	m_pMotion;
			float					m_speedRatio;
			float					m_blendTime;
			int32_t						m_loopCount;

			void operator() (CGrannyLODController * pController)
			{
				if (pController->isModelInstance())
					pController->SetMotionPointer(m_pMotion, m_blendTime, m_loopCount, m_speedRatio);
			}
		};

		struct FChangeMotionPointer
		{
			const CGrannyMotion *	m_pMotion;
			float					m_speedRatio;
			int32_t						m_loopCount;

			void operator() (CGrannyLODController * pController)
			{
				if (pController->isModelInstance())
					pController->ChangeMotionPointer(m_pMotion, m_loopCount, m_speedRatio);
			}
		};

		struct FEndStopMotionPointer
		{
			const CGrannyMotion *	m_pMotion;

			void operator () (CGrannyLODController * pController)
			{
				if (pController->isModelInstance())
					pController->SetMotionAtEnd();
			}
		};

		CGrannyLODController();
		virtual ~CGrannyLODController();

		void	Clear();

		void	CreateDeviceObjects();
		void	DestroyDeviceObjects();

		void	AddModel(CGraphicThing * pThing, int32_t iSrcModel, CGrannyLODController * pSkelLODController=nullptr);		
		void	AttachModelInstance(CGrannyLODController * pSrcLODController, const char * c_szBoneName);
		void	DetachModelInstance(CGrannyLODController * pSrcLODController);
		void	SetLODLimits(float fNearLOD, float fFarLOD);
		void	SetLODLevel(uint8_t bLODLevel);
		uint8_t	GetLODLevel() { return m_bLODLevel; }
		void	SetMaterialImagePointer(const char* c_szImageName, CGraphicImage* pImage);
		void	SetMaterialData(const char* c_szImageName, const SMaterialData& c_rkMaterialData);
		void	SetSpecularInfo(const char* c_szMtrlName, bool bEnable, float fPower);

		void	RenderWithOneTexture();
		void	RenderWithTwoTexture();
		void	BlendRenderWithOneTexture();
		void	BlendRenderWithTwoTexture();

		void	Update(float fElapsedTime, float fDistanceFromCenter, float fDistanceFromCamera);
		void	UpdateLODLevel(float fDistanceFromCenter, float fDistanceFromCamera);
		void	UpdateTime(float fElapsedTime);
		
		void	UpdateSkeleton(const D3DXMATRIX * c_pWorldMatrix, float fElapsedTime);
		void	Deform(const D3DXMATRIX * c_pWorldMatrix);
		void	DeformNoSkin(const D3DXMATRIX * c_pWorldMatrix);
		void	DeformAll(const D3DXMATRIX * c_pWorldMatrix);
		
		void	RenderToShadowMap();
		void	RenderShadow();
		void	ReloadTexture();

		void	GetBoundBox(D3DXVECTOR3 * vtMin, D3DXVECTOR3 * vtMax);
		bool	Intersect(const D3DXMATRIX * c_pMatrix, float * u, float * v, float * t);

		void	SetLocalTime(float fLocalTime);
		void	ResetLocalTime();

		void	SetMotionPointer(const CGrannyMotion * c_pMotion, float fBlendTime, int32_t iLoopCount, float speedRatio);
		void	ChangeMotionPointer(const CGrannyMotion * c_pMotion, int32_t iLoopCount, float speedRatio);
		void	SetMotionAtEnd();

		BOOL	isModelInstance();
		CGrannyModelInstance*	GetModelInstance();
		bool	HaveBlendThing() { return 0 != GetModelInstance() ? GetModelInstance()->HaveBlendThing() : false; }		// NOTE: GetModelInstance() == 0일 때 클라 크래쉬나는 문제 수정(2012. 05. 07)

	protected:
		void	SetCurrentModelInstance(CGrannyModelInstance * pgrnModelInstance);
		void	RefreshAttachedModelInstance();

		void	__ReserveSharedDeformableVertexBuffer(uint32_t deformableVertexCount);

	protected:
		float								m_fLODDistance;
		uint32_t								m_dwLODAniFPS;

		//// Attaching Link Data
		// Data of Parent Side
		typedef struct SAttachingModelData
		{
			CGrannyLODController *	pkLODController;
			std::string				strBoneName;
		} TAttachingModelData;
		
		std::vector<TAttachingModelData>	m_AttachedModelDataVector;
		// Data of Child Side
		CGrannyLODController *				m_pAttachedParentModel;

		uint8_t								m_bLODLevel;
		CGrannyModelInstance *				m_pCurrentModelInstance;		

		// WORK
		std::deque<CGrannyModelInstance *>	m_que_pkModelInst;

		CGraphicVertexBuffer*	m_pkSharedDeformableVertexBuffer;
		// END_OF_WORK
};
