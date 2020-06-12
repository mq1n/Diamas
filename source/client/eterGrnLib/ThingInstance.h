#pragma once

#include "../eterbase/Stl.h"
#include "../eterlib/GrpObjectInstance.h"

#include "LODController.h"
		
const int32_t DONTUSEVALUE = -1;
class CGraphicThingInstance : public CGraphicObjectInstance
{
	public:		
		typedef struct SModelThingSet
		{
			void Clear()
			{
				stl_wipe(m_pLODThingRefVector);
			}

			std::vector<CGraphicThing::TRef *>	m_pLODThingRefVector;
		} TModelThingSet;
		
	public:
		enum
		{
			ID = THING_OBJECT
		};
		int32_t GetType() const { return ID; }

		CGraphicThingInstance();
		virtual ~CGraphicThingInstance();

		void		DeformNoSkin();

		void		UpdateLODLevel();
		void		UpdateTime();
		void		DeformAll(); // ¸ðµç LOD µðÆû
		
		bool		LessRenderOrder(CGraphicThingInstance* pkThingInst);

		bool		Picking(const D3DXVECTOR3 & v, const D3DXVECTOR3 & dir, float & out_x, float & out_y);

		void		OnInitialize();

		bool		CreateDeviceObjects();
		void		DestroyDeviceObjects();

		void		ReserveModelInstance(int32_t iCount);
		void		ReserveModelThing(int32_t iCount);

		bool		CheckModelInstanceIndex(int32_t iModelInstance);
		bool		CheckModelThingIndex(int32_t iModelThing);
		bool		CheckMotionThingIndex(uint32_t dwMotionKey);
		bool		GetMotionThingPointer(uint32_t dwKey, CGraphicThing ** ppMotion);
		bool		IsMotionThing();

		void		RegisterModelThing(int32_t iModelThing, CGraphicThing * pModelThing);
		void		RegisterLODThing(int32_t iModelThing, CGraphicThing * pModelThing);
		void		RegisterMotionThing(uint32_t dwMotionKey, CGraphicThing * pMotionThing);

		bool		SetModelInstance(int32_t iDstModelInstance, int32_t iSrcModelThing, int32_t iSrcModel,int32_t iSkelInstance = DONTUSEVALUE);
		void		SetEndStopMotion();
		void		SetMotionAtEnd();

		void		AttachModelInstance(int32_t iDstModelInstance, const char * c_szBoneName, int32_t iSrcModelInstance);
		void		AttachModelInstance(int32_t iDstModelInstance, const char * c_szBoneName, CGraphicThingInstance & rsrcInstance, int32_t iSrcModelInstance);
		void		DetachModelInstance(int32_t iDstModelInstance, CGraphicThingInstance & rSrcInstance, int32_t SrcModelInstance);
		bool		FindBoneIndex(int32_t iModelInstance, const char* c_szBoneName, int32_t * iRetBone);
		bool		GetBonePosition(int32_t iModelIndex, int32_t iBoneIndex, float * pfx, float * pfy, float * pfz);

		void		ResetLocalTime();
		void		InsertDelay(float fDelay);

		void		SetMaterialImagePointer(uint32_t ePart, const char* c_szImageName, CGraphicImage* pImage);
		void		SetMaterialData(uint32_t ePart, const char* c_szImageName, const SMaterialData &kMaterialData);
		void		SetSpecularInfo(uint32_t ePart, const char* c_szMtrlName, bool bEnable, float fPower);

		void		__SetLocalTime(float fLocalTime); // Only Used by Tools
		float		GetLastLocalTime();
		float		GetLocalTime();
		float		GetSecondElapsed();
		float		GetAverageSecondElapsed();

		uint8_t		GetLODLevel(uint32_t dwModelInstance);
		float		GetHeight();

		void		RenderWithOneTexture();
		void		RenderWithTwoTexture();
		void		BlendRenderWithOneTexture();
		void		BlendRenderWithTwoTexture();

		uint32_t		GetLODControllerCount() const;
		CGrannyLODController * GetLODControllerPointer(uint32_t dwModelIndex) const;
		CGrannyLODController * GetLODControllerPointer(uint32_t dwModelIndex);

		void		ReloadTexture();

	public:
		CGraphicThing* GetBaseThingPtr();

		bool		SetMotion(uint32_t dwMotionKey, float blendTime = 0.0f, int32_t loopCount = 0, float speedRatio=1.0f);
		bool		ChangeMotion(uint32_t dwMotionKey, int32_t loopCount = 0, float speedRatio=1.0f);
		bool		Intersect(float * pu, float * pv, float * pt);
		void		GetBoundBox(D3DXVECTOR3 * vtMin, D3DXVECTOR3 * vtMax);
		BOOL		GetBoundBox(uint32_t dwModelInstanceIndex, D3DXVECTOR3 * vtMin, D3DXVECTOR3 * vtMax);
		BOOL		GetBoneMatrix(uint32_t dwModelInstanceIndex, uint32_t dwBoneIndex, D3DXMATRIX ** ppMatrix);
		BOOL		GetCompositeBoneMatrix(uint32_t dwModelInstanceIndex, uint32_t dwBoneIndex, D3DXMATRIX ** ppMatrix);
		void		UpdateTransform(D3DXMATRIX * pMatrix, float fSecondsElapsed = 0.0f, int32_t iModelInstanceIndex = 0);

	public:
		void			BuildBoundingSphere();
		void			BuildBoundingAABB();
		virtual void	CalculateBBox();
		virtual bool	GetBoundingSphere(D3DXVECTOR3 & v3Center, float & fRadius);
		virtual bool	GetBoundingAABB(D3DXVECTOR3 & v3Min, D3DXVECTOR3 & v3Max);

		void			ClearPart(uint8_t part_index);

	protected:
		void		OnClear();
		void		OnDeform();
		void		OnUpdate();
		void		OnRender();
		void		OnBlendRender();
		void		OnRenderToShadowMap();
		void		OnRenderShadow();
		void		OnRenderPCBlocker();

	protected:
		bool									m_bUpdated;
		float									m_fLastLocalTime;
		float									m_fLocalTime;
		float									m_fDelay;
		float									m_fSecondElapsed;
		float									m_fAverageSecondElapsed;
		float									m_fRadius;
		D3DXVECTOR3								m_v3Center;
		D3DXVECTOR3								m_v3Min, m_v3Max;

		std::vector<CGrannyLODController *>		m_LODControllerVector;
		std::vector<TModelThingSet>				m_modelThingSetVector;
		std::map<uint32_t, CGraphicThing::TRef *>	m_roMotionThingMap;

	protected:
		virtual void		OnUpdateCollisionData(const CStaticCollisionDataVector * pscdVector);
		virtual void		OnUpdateHeighInstance(CAttributeInstance * pAttributeInstance);
		virtual bool		OnGetObjectHeight(float fX, float fY, float * pfHeight);

	public:
		static void CreateSystem(uint32_t uCapacity);
		static void DestroySystem();

		static CGraphicThingInstance* New();
		static void Delete(CGraphicThingInstance* pkInst);

		static CDynamicPool<CGraphicThingInstance>		ms_kPool;

		bool	HaveBlendThing();

	public:
		float		m_nextAllowedMovement;
};
