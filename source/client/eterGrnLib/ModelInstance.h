#pragma once

//#define CACHE_DEFORMED_VERTEX
#include "../eterLib/GrpImage.h"
#include "../eterLib/GrpCollisionObject.h"

#include "Model.h"
#include "Motion.h"

class CGrannyModelInstance : public CGraphicCollisionObject
{
	public:
		enum
		{
			ANIFPS_MIN = 30,
			ANIFPS_MAX = 120
		};
	public:
		static void DestroySystem();

		static CGrannyModelInstance* New();
		static void Delete(CGrannyModelInstance* pkInst);

		static CDynamicPool<CGrannyModelInstance>		ms_kPool;

	public:
		struct FCreateDeviceObjects
		{
			void operator()(CGrannyModelInstance * pModelInstance) const { pModelInstance->CreateDeviceObjects(); }
		};

		struct FDestroyDeviceObjects
		{
			void operator()(CGrannyModelInstance * pModelInstance) const { pModelInstance->DestroyDeviceObjects(); }
		};

	public:
		CGrannyModelInstance();
		virtual ~CGrannyModelInstance();

		bool	IsEmpty();
		void	Clear();

		bool	CreateDeviceObjects();
		void	DestroyDeviceObjects();

		// Update & Render
		void	Update(uint32_t dwAniFPS);
		void	UpdateLocalTime(float fElapsedTime);
		void	UpdateTransform(D3DXMATRIX * pMatrix, float fSecondsElapsed);

		void	UpdateSkeleton(const D3DXMATRIX * c_pWorldMatrix, float fLocalTime);
		void	DeformNoSkin(const D3DXMATRIX * c_pWorldMatrix);
		void	Deform(const D3DXMATRIX * c_pWorldMatrix);

		// FIXME : ����� �ϵ������ �Ѱ�� 2���� �ؽ���� ������ �Ǿ��ִ� �����̱⿡ �̷�
		//         �Ҿ����� ��Ű���簡 ����������, �ñ����� ������ (�� �ؽ��� ����) + (ȿ���� �ؽ���)
		//         �̷����� �ڵ� ������ �̷����� ���� �ʳ� �����մϴ�. - [levites]
		// NOTE : ���ο� if���� ���� ��Ű�� ���ٴ� ������ ���ŷ����� �̷��� �Լ� �� ��ü�� �и�
		//        ��Ű�� ���� �����ս� ���� ���鿡���� �� ���� �� �����ϴ�. - [levites]
		// NOTE : �ǹ��� ������ OneTexture. ĳ���ʹ� ��쿡 ���� TwoTexture.
		void	RenderWithOneTexture();
		void	RenderWithTwoTexture();
		void	BlendRenderWithOneTexture();
		void	BlendRenderWithTwoTexture();
		void	RenderWithoutTexture();

		// Model
		CGrannyModel* GetModel();
		void	SetMaterialImagePointer(const char* c_szImageName, CGraphicImage* pImage);
		void	SetMaterialData(const char* c_szImageName, const SMaterialData& c_rkMaterialData);
		void	SetSpecularInfo(const char* c_szMtrlName, bool bEnable, float fPower);
		
		void	SetMainModelPointer(CGrannyModel* pkModel, CGraphicVertexBuffer* pkSharedDefromableVertexBuffer);
		void	SetLinkedModelPointer(CGrannyModel* pkModel, CGraphicVertexBuffer* pkSharedDefromableVertexBuffer, CGrannyModelInstance** ppkSkeletonInst);

		// Motion
		void	SetMotionPointer(const CGrannyMotion* pMotion, float blendTime=0.0f, int32_t loopCount=0, float speedRatio=1.0f);
		void	ChangeMotionPointer(const CGrannyMotion* pMotion, int32_t loopCount=0, float speedRatio=1.0f);
		void	SetMotionAtEnd();		
		bool	IsMotionPlaying();
		
		void	CopyMotion(CGrannyModelInstance * pModelInstance, bool bIsFreeSourceControl=false);

		// Time
		void	SetLocalTime(float fLocalTime);
		int32_t		ResetLocalTime();
		float	GetLocalTime();

		// WORK
		uint32_t	GetDeformableVertexCount();
		uint32_t	GetVertexCount();

		// END_OF_WORK

		// Bone & Attaching
		const float *	GetBoneMatrixPointer(int32_t iBone) const;
		const float *	GetCompositeBoneMatrixPointer(int32_t iBone) const;
		bool			GetMeshMatrixPointer(int32_t iMesh, const D3DXMATRIX ** c_ppMatrix) const;
		bool			GetBoneIndexByName(const char * c_szBoneName, int32_t * pBoneIndex) const;
		void			SetParentModelInstance(const CGrannyModelInstance* c_pParentModelInstance, const char * c_szBoneName);
		void			SetParentModelInstance(const CGrannyModelInstance* c_pParentModelInstance, int32_t iBone);

		// Collision Detection
		bool	Intersect(const D3DXMATRIX * c_pMatrix, float * pu, float * pv, float * pt);
		void	MakeBoundBox(TBoundBox* pBoundBox, const float* mat, const float* OBBMin, const float* OBBMax, D3DXVECTOR3* vtMin, D3DXVECTOR3* vtMax);
		void	GetBoundBox(D3DXVECTOR3 * vtMin, D3DXVECTOR3* vtMax);

		// Reload Texture
		void	ReloadTexture();


	protected:
		void	__Initialize();
				
		void	__DestroyModelInstance();
		void	__DestroyMeshMatrices();
		void	__DestroyDynamicVertexBuffer();

		
		void	__CreateModelInstance();
		void	__CreateMeshMatrices();
		void	__CreateDynamicVertexBuffer();

		// WORK		
		void	__DestroyWorldPose();
		void	__CreateWorldPose(CGrannyModelInstance* pkSrcModelInst);

		bool	__CreateMeshBindingVector(CGrannyModelInstance* pkDstModelInst);
		void	__DestroyMeshBindingVector();
		
		const granny_int32x*	__GetMeshBoneIndices(uint32_t iMeshBinding) const;

		bool	__IsDeformableVertexBuffer();
		void	__SetSharedDeformableVertexBuffer(CGraphicVertexBuffer* pkSharedDeformableVertexBuffer);
		
		IDirect3DVertexBuffer9* __GetDeformableD3DVertexBufferPtr();
		CGraphicVertexBuffer&	__GetDeformableVertexBufferRef();
		
		granny_world_pose* __GetWorldPosePtr() const;
		// END_OF_WORK


		// Update & Render
		void	UpdateWorldPose();
		void	UpdateWorldMatrices(const D3DXMATRIX * c_pWorldMatrix);
		void	DeformPNTVertices(void * pvDest);

		void	RenderMeshNodeListWithOneTexture(CGrannyMesh::EType eMeshType, CGrannyMaterial::EType eMtrlType);
		void	RenderMeshNodeListWithTwoTexture(CGrannyMesh::EType eMeshType, CGrannyMaterial::EType eMtrlType);
		void	RenderMeshNodeListWithoutTexture(CGrannyMesh::EType eMeshType, CGrannyMaterial::EType eMtrlType);

	protected:
		// Static Data
		CGrannyModel *					m_pModel;

		// Granny Data
		granny_model_instance *			m_pgrnModelInstance;

		//granny_world_pose *				m_pgrnWorldPose;		// ���� ���� ���� ������

		granny_control *				m_pgrnCtrl;
		granny_animation *				m_pgrnAni;

		// Meshes' Transform Data
		D3DXMATRIX *					m_meshMatrices;

		
		// Attaching Data
		const CGrannyModelInstance *	mc_pParentInstance;
		int32_t								m_iParentBoneIndex;

		// Game Data
		float							m_fLocalTime;
		float							m_fSecondsElapsed;	

		uint32_t							m_dwOldUpdateFrame;

		CGrannyMaterialPalette			m_kMtrlPal;

		// WORK
		granny_world_pose*					m_pgrnWorldPoseReal;	// ���� �޸𸮴� ���⿡ �Ҵ�
		std::vector<granny_mesh_binding*>	m_vct_pgrnMeshBinding;		

		// Dynamic Vertex Buffer
		CGraphicVertexBuffer*			m_pkSharedDeformableVertexBuffer;
		CGraphicVertexBuffer			m_kLocalDeformableVertexBuffer;
		bool							m_isDeformableVertexBuffer;		
		// END_OF_WORK

		// TEST
		CGrannyModelInstance** m_ppkSkeletonInst;
		// END_OF_TEST

	public:
		bool							HaveBlendThing() { return m_pModel->HaveBlendThing(); }
};
