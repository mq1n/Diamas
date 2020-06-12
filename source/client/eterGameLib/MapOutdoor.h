#pragma once

#include "../eterLib/SkyBox.h"
#include "../eterLib/LensFlare.h"
#include "../eterLib/ScreenFilter.h"

#include "../eterTerrainLib/TerrainType.h"
#include "../eterTerrainLib/TextureSet.h"

#include "../eterTreeLib/SpeedTreeForestDirectX9.h"

#include "MapBase.h"
#include "Area.h"
#include "AreaTerrain.h"
#include "AreaLoaderThread.h"


#define LOAD_SIZE_WIDTH				1

#define AROUND_AREA_NUM				1+(LOAD_SIZE_WIDTH*2)*(LOAD_SIZE_WIDTH*2)*2
#define MAX_PREPARE_SIZE			9
#define MAX_MAPSIZE					256		// 0 ~ 255, cellsize 200 = 64km

#define TERRAINPATCH_LODMAX		3

typedef struct SOutdoorMapCoordinate
{
	int16_t m_sTerrainCoordX;		// Terrain 좌표
	int16_t m_sTerrainCoordY;
} TOutdoorMapCoordinate;

typedef std::map<const std::string, TOutdoorMapCoordinate> TOutdoorMapCoordinateMap;

class CTerrainPatchProxy;
class CTerrainQuadtreeNode;

class CMapOutdoor : public CMapBase
{
	public:
		enum
		{
			VIEW_NONE = 0,
			VIEW_PART,
			VIEW_ALL,
		};

		enum EPart
		{
			PART_TERRAIN,
			PART_OBJECT,
			PART_CLOUD,
			PART_WATER,
			PART_TREE,
			PART_SKY,
			PART_NUM,
		};

		enum ETerrainRenderSort
		{
			DISTANCE_SORT,
			TEXTURE_SORT,
		};

	public:
		CMapOutdoor();
		virtual ~CMapOutdoor();

		virtual void	OnBeginEnvironment();

	protected:
		bool			Initialize();

		virtual bool	Destroy();
		virtual void	OnSetEnvironmentDataPtr();
		virtual void	OnResetEnvironmentDataPtr();

		virtual void	OnRender();

		virtual void	OnPreAssignTerrainPtr() {};

	public:
		void			SetInverseViewAndDynamicShaodwMatrices();
		virtual bool	Load(float x, float y, float z);
		virtual float	GetHeight(float x, float y);
		virtual float	GetCacheHeight(float x, float y);

		virtual bool	Update(float fX, float fY, float fZ);
		virtual void	UpdateAroundAmbience(float fX, float fY, float fZ);

	public:
		void			Clear();

		void			SetVisiblePart(int32_t ePart, bool isVisible);
		void			SetSplatLimit(int32_t iSplatNum);
		std::vector<int32_t> &	GetRenderedSplatNum(int32_t * piPatch, int32_t * piSplat, float * pfSplatRatio);
		CArea::TCRCWithNumberVector & GetRenderedGraphicThingInstanceNum(uint32_t * pdwGraphicThingInstanceNum, uint32_t * pdwCRCNum);

		bool			LoadSetting(const char * c_szFileName);

		void			ApplyLight(uint32_t dwVersion, const D3DLIGHT9& c_rkLight);
		void			SetEnvironmentScreenFilter();
		void			SetEnvironmentSkyBox();
		void			SetEnvironmentLensFlare();

		void			CreateCharacterShadowTexture();
		void			ReleaseCharacterShadowTexture();
		void			SetShadowTextureSize(uint16_t size);

		bool			BeginRenderCharacterShadowToTexture();
		void			EndRenderCharacterShadowToTexture();
		void			RenderWater();
		void			RenderMarkedArea();
		void			RecurseRenderAttr(CTerrainQuadtreeNode *Node, bool bCullEnable=TRUE);
		void			DrawPatchAttr(int32_t patchnum);
		void			ClearGuildArea();
		void			RegisterGuildArea(int32_t isx, int32_t isy, int32_t iex, int32_t iey);

		void			VisibleMarkedArea();
		void			DisableMarkedArea();

		void			UpdateSky();
		void			RenderCollision();
		void			RenderSky();
		void			RenderCloud();
		void			RenderBeforeLensFlare();
		void			RenderAfterLensFlare();
		void			RenderScreenFiltering();
		void 			TextureChange(const char * c_szFileName);

		void			SetWireframe(bool bWireFrame);
		bool			IsWireframe();

		bool			GetPickingPointWithRay(const CRay & rRay, D3DXVECTOR3 * v3IntersectPt);
		bool			GetPickingPointWithRayOnlyTerrain(const CRay & rRay, D3DXVECTOR3 * v3IntersectPt);
		bool			GetPickingPoint(D3DXVECTOR3 * v3IntersectPt);
		void			GetTerrainCount(int16_t * psTerrainCountX, int16_t * psTerrainCountY)
		{
			*psTerrainCountX = m_sTerrainCountX;
			*psTerrainCountY = m_sTerrainCountY;
		}

		bool			SetTerrainCount(int16_t sTerrainCountX, int16_t sTerrainCountY);

		// Shadow
		void			SetDrawShadow(bool bDrawShadow);
		void			SetDrawCharacterShadow(bool bDrawChrShadow);

		uint32_t			GetShadowMapColor(float fx, float fy);

	protected:
		bool			__PickTerrainHeight(float& fPos, const D3DXVECTOR3& v3Start, const D3DXVECTOR3& v3End, float fStep, float fRayRange, float fLimitRange, D3DXVECTOR3* pv3Pick);

		virtual void	__ClearGarvage();
		virtual void	__UpdateGarvage();

		virtual bool	LoadTerrain(uint16_t wTerrainCoordX, uint16_t wTerrainCoordY, uint16_t wCellCoordX, uint16_t wCellCoordY);
		virtual bool	LoadArea(uint16_t wAreaCoordX, uint16_t wAreaCoordY, uint16_t wCellCoordX, uint16_t wCellCoordY);
		virtual void	UpdateAreaList(int32_t lCenterX, int32_t lCenterY);
		bool			isTerrainLoaded(uint16_t wX, uint16_t wY);
		bool			isAreaLoaded(uint16_t wX, uint16_t wY);

		void			AssignTerrainPtr();				// 현재 좌표에서 주위(ex. 3x3)에 있는 것들의 포인터를 연결한다. (업데이트 시 불려짐)


		//////////////////////////////////////////////////////////////////////////
		// New
		//////////////////////////////////////////////////////////////////////////
		// 여러가지 맵들을 얻는다.
		void			GetHeightMap(const uint8_t & c_rucTerrainNum, uint16_t ** pwHeightMap);
		void			GetNormalMap(const uint8_t & c_rucTerrainNum, char ** pucNormalMap);

		// Water
		void			GetWaterMap(const uint8_t & c_rucTerrainNum, uint8_t ** pucWaterMap);
		void			GetWaterHeight(uint8_t byTerrainNum, uint8_t byWaterNum, int32_t * plWaterHeight);


	//////////////////////////////////////////////////////////////////////////
	// Terrain
	//////////////////////////////////////////////////////////////////////////
	protected:
		// 데이터
		CTerrain *					m_pTerrain[AROUND_AREA_NUM];	// Terrain
		CTerrainPatchProxy *		m_pTerrainPatchProxyList;			// CTerrain을 랜더링 할때 실제로 랜더링하는 폴리곤 패치들... Seamless Map 을 위해 CTerrain으로부터 독립...

		int32_t						m_lViewRadius;				// 시야 거리.. 셀단위임..
		float						m_fHeightScale;				// 높이 스케일... 1.0일때 0~655.35미터까지 표현 가능.

		int16_t						m_sTerrainCountX, m_sTerrainCountY;		// seamless map 안에 들어가는 Terrain개수

		TOutdoorMapCoordinate		m_CurCoordinate;		// 현재의 좌표

		int32_t						m_lCurCoordStartX, m_lCurCoordStartY;
		TOutdoorMapCoordinate		m_PrevCoordinate;		// 현재의 좌표
		TOutdoorMapCoordinateMap	m_EntryPointMap;

		uint16_t						m_wPatchCount;

		//////////////////////////////////////////////////////////////////////////
		// Index Buffer
#ifdef	WORLD_EDITOR
		uint16_t *						m_pwIndices;		/* temp Index buffer */

		CGraphicIndexBuffer			m_IndexBuffer;
		uint16_t						m_wNumIndices;
#else
		uint16_t *						m_pwaIndices[TERRAINPATCH_LODMAX];

		CGraphicIndexBuffer			m_IndexBuffer[TERRAINPATCH_LODMAX];
		uint16_t						m_wNumIndices[TERRAINPATCH_LODMAX];
#endif
		virtual void	DestroyTerrain();

		void			CreateTerrainPatchProxyList();
		void			DestroyTerrainPatchProxyList();

		void			UpdateTerrain(float fX, float fY);

		void			ConvertTerrainToTnL(int32_t lx, int32_t ly);

		void			AssignPatch(int32_t lPatchNum, int32_t lx0, int32_t ly0, int32_t lx1, int32_t ly1);

		//////////////////////////////////////////////////////////////////////////
		// Index Buffer
		void			ADDLvl1TL(uint16_t * pIndices, uint16_t & rwCount, const uint16_t & c_rwCurCount, const uint8_t & c_rucNumLineWarp);
		void			ADDLvl1T(uint16_t * pIndices, uint16_t & rwCount, const uint16_t & c_rwCurCount,  const uint8_t & c_rucNumLineWarp);
		void			ADDLvl1TR(uint16_t * pIndices, uint16_t & rwCount, const uint16_t & c_rwCurCount, const uint8_t & c_rucNumLineWarp);
		void			ADDLvl1L(uint16_t * pIndices, uint16_t & rwCount, const uint16_t & c_rwCurCount, const uint8_t & c_rucNumLineWarp);
		void			ADDLvl1R(uint16_t * pIndices, uint16_t & rwCount, const uint16_t & c_rwCurCount, const uint8_t & c_rucNumLineWarp);
		void			ADDLvl1BL(uint16_t * pIndices, uint16_t & rwCount, const uint16_t & c_rwCurCount, const uint8_t & c_rucNumLineWarp);
		void			ADDLvl1B(uint16_t * pIndices, uint16_t & rwCount, const uint16_t & c_rwCurCount, const uint8_t & c_rucNumLineWarp);
		void			ADDLvl1BR(uint16_t * pIndices, uint16_t & rwCount, const uint16_t & c_rwCurCount, const uint8_t & c_rucNumLineWarp);
		void			ADDLvl1M(uint16_t * pIndices, uint16_t & rwCount, const uint16_t & c_rwCurCount, const uint8_t & c_rucNumLineWarp);
		void			ADDLvl2TL(uint16_t * pIndices, uint16_t & rwCount, const uint16_t & c_rwCurCount, const uint8_t & c_rucNumLineWarp);
		void			ADDLvl2T(uint16_t * pIndices, uint16_t & rwCount, const uint16_t & c_rwCurCount, const uint8_t & c_rucNumLineWarp);
		void			ADDLvl2TR(uint16_t * pIndices, uint16_t & rwCount, const uint16_t & c_rwCurCount, const uint8_t & c_rucNumLineWarp);
		void			ADDLvl2L(uint16_t * pIndices, uint16_t & rwCount, const uint16_t & c_rwCurCount, const uint8_t & c_rucNumLineWarp);
		void			ADDLvl2R(uint16_t * pIndices, uint16_t & rwCount, const uint16_t & c_rwCurCount, const uint8_t & c_rucNumLineWarp);
		void			ADDLvl2BL(uint16_t * pIndices, uint16_t & rwCount, const uint16_t & c_rwCurCount, const uint8_t & c_rucNumLineWarp);
		void			ADDLvl2B(uint16_t * pIndices, uint16_t & rwCount, const uint16_t & c_rwCurCount, const uint8_t & c_rucNumLineWarp);
		void			ADDLvl2BR(uint16_t * pIndices, uint16_t & rwCount, const uint16_t & c_rwCurCount, const uint8_t & c_rucNumLineWarp);
		void			ADDLvl2M(uint16_t * pIndices, uint16_t & rwCount, const uint16_t & c_rwCurCount, const uint8_t & c_rucNumLineWarp);

	public:
		BOOL			GetTerrainPointer(uint8_t c_ucTerrainNum, CTerrain ** ppTerrain);
		float			GetTerrainHeight(float fx, float fy);
		bool			GetWaterHeight(int32_t iX, int32_t iY, int32_t * plWaterHeight);
		bool			GetNormal(int32_t ix, int32_t iy, D3DXVECTOR3 * pv3Normal);

		void			RenderTerrain();

		const int32_t		GetViewRadius()			{ return m_lViewRadius;		}
		const float		GetHeightScale()		{ return m_fHeightScale;	}

		const TOutdoorMapCoordinate & GetCurCoordinate() { return m_CurCoordinate; }
		const TOutdoorMapCoordinate & GetPrevCoordinate() { return m_PrevCoordinate; }

	//////////////////////////////////////////////////////////////////////////
	// Area
	//////////////////////////////////////////////////////////////////////////
	protected:
		CArea *			m_pArea[AROUND_AREA_NUM];		// Data

		virtual void	DestroyArea();

		void			__UpdateArea(D3DXVECTOR3& v3Player);
#ifdef WORLD_EDITOR
		void			__NEW_WorldEditor_UpdateArea();
#endif
		void			__Game_UpdateArea(D3DXVECTOR3& v3Player);

		void			__CollectShadowReceiver(D3DXVECTOR3& v3Target, D3DXVECTOR3& v3Light);
		void			__CollectCollisionPCBlocker(D3DXVECTOR3& v3Eye, D3DXVECTOR3& v3Target, float fDistance);
		void			__CollectCollisionShadowReceiver(D3DXVECTOR3& v3Target, D3DXVECTOR3& v3Light);
		void			__UpdateAroundAreaList();
		bool			__IsInShadowReceiverList(CGraphicObjectInstance* pkObjInstTest);
		bool			__IsInPCBlockerList(CGraphicObjectInstance* pkObjInstTest);

		void			ConvertToMapCoords(float fx, float fy, int32_t *iCellX, int32_t *iCellY, uint8_t * pucSubCellX, uint8_t * pucSubCellY, uint16_t * pwTerrainNumX, uint16_t * pwTerrainNumY);

	public:
		BOOL			GetAreaPointer(const uint8_t c_ucAreaNum, CArea ** ppArea);
		void			RenderArea(bool bRenderAmbience = true);
		void			RenderBlendArea();
		void			RenderDungeon();
		void			RenderEffect();
		void			RenderPCBlocker();
		void			RenderTree();

	public:
		//////////////////////////////////////////////////////////////////////////
		// For Grass
		//////////////////////////////////////////////////////////////////////////
		float		GetHeight(float* pPos);
		bool		GetBrushColor(float fX, float fY, float* pLowColor, float* pHighColor);
		bool		isAttrOn(float fX, float fY, uint8_t byAttr);
		bool		GetAttr(float fX, float fY, uint8_t * pbyAttr);
		bool		isAttrOn(int32_t iX, int32_t iY, uint8_t byAttr);
		bool		GetAttr(int32_t iX, int32_t iY, uint8_t * pbyAttr);

		bool		GetTerrainNum(float fx, float fy, uint8_t * pbyTerrainNum);
		bool		GetTerrainNumFromCoord(uint16_t wCoordX, uint16_t wCoordY, uint8_t * pbyTerrainNum);

	protected:
		//////////////////////////////////////////////////////////////////////////
		// New
		//////////////////////////////////////////////////////////////////////////
		int32_t					m_lCenterX, m_lCenterY;		// Terrain 좌표 내의 셀 좌표...
		int32_t					m_lOldReadX, m_lOldReadY;	/* Last center */

		//////////////////////////////////////////////////////////////////////////
		// Octree
		//////////////////////////////////////////////////////////////////////////
		CTerrainQuadtreeNode * 	m_pRootNode;

		void					BuildQuadTree();
		CTerrainQuadtreeNode *	AllocQuadTreeNode(int32_t x0, int32_t y0, int32_t x1, int32_t y1);
		void					SubDivideNode(CTerrainQuadtreeNode * Node);
		void					UpdateQuadTreeHeights(CTerrainQuadtreeNode *Node);


		void					FreeQuadTree();

		struct TPatchDrawStruct
		{
			float fDistance;
			uint8_t byTerrainNum;
			int32_t lPatchNum;
			CTerrainPatchProxy * pTerrainPatchProxy;

			bool operator<( const TPatchDrawStruct & rhs) const
			{
				return fDistance < rhs.fDistance;
			}
		};

	public:
		typedef std::vector<uint8_t> TTerrainNumVector; 
		struct FSortPatchDrawStructWithTerrainNum
		{
			static TTerrainNumVector m_TerrainNumVector;
			FSortPatchDrawStructWithTerrainNum()
			{
				m_TerrainNumVector.clear();
			}

			bool operator () (const TPatchDrawStruct & lhs, const TPatchDrawStruct & rhs)
			{
				uint32_t lhsTerrainNumOrder = 0, rhsTerrainNumOrder = 0;
				bool blhsOrderFound = false;
				bool brhsOrderFound = false;

				TTerrainNumVector::iterator lhsIterator = std::find(m_TerrainNumVector.begin(), m_TerrainNumVector.end(), lhs.byTerrainNum);
				TTerrainNumVector::iterator rhsIterator = std::find(m_TerrainNumVector.begin(), m_TerrainNumVector.end(), rhs.byTerrainNum);

				if (lhsIterator != m_TerrainNumVector.end())
				{
					blhsOrderFound = true;
					lhsTerrainNumOrder = lhsIterator - m_TerrainNumVector.begin();
				}
				if (rhsIterator != m_TerrainNumVector.end())
				{
					brhsOrderFound = true;
					rhsTerrainNumOrder = rhsIterator - m_TerrainNumVector.begin();
				}
				if (!brhsOrderFound)
				{
					m_TerrainNumVector.push_back(rhs.byTerrainNum);
					rhsTerrainNumOrder = m_TerrainNumVector.size() -1;
				}
				if (!blhsOrderFound)
				{
					lhsIterator = std::find(m_TerrainNumVector.begin(), m_TerrainNumVector.end(), lhs.byTerrainNum);
					if (lhsIterator != m_TerrainNumVector.end())
					{
						blhsOrderFound = true;
						lhsTerrainNumOrder = lhsIterator - m_TerrainNumVector.begin();
					}
					if (!blhsOrderFound)
					{
						m_TerrainNumVector.push_back(lhs.byTerrainNum);
						lhsTerrainNumOrder = m_TerrainNumVector.size() -1;
					}
				}

				return lhsTerrainNumOrder < rhsTerrainNumOrder;
			}
		};

	protected:

		std::vector<std::pair<float, int32_t> > m_PatchVector;
		std::vector<TPatchDrawStruct> m_PatchDrawStructVector;

		void					SetPatchDrawVector();

		void					NEW_DrawWireFrame(CTerrainPatchProxy * pTerrainPatchProxy, uint16_t wPrimitiveCount, D3DPRIMITIVETYPE ePrimitiveType);

		void					DrawWireFrame(int32_t patchnum, uint16_t wPrimitiveCount, D3DPRIMITIVETYPE ePrimitiveType);
		void					DrawWater(int32_t patchnum);

		bool					m_bDrawWireFrame;
		bool					m_bDrawShadow;
		bool					m_bDrawChrShadow;

		//////////////////////////////////////////////////////////////////////////
		// Water
		D3DXMATRIX				m_matBump;
		void					LoadWaterTexture();
		void					UnloadWaterTexture();
		//Water
		//////////////////////////////////////////////////////////////////////////

		//////////////////////////////////////////////////////////////////////////
		// Character Shadow
		LPDIRECT3DTEXTURE9		m_lpCharacterShadowMapTexture;
		LPDIRECT3DSURFACE9		m_lpCharacterShadowMapRenderTargetSurface;
		LPDIRECT3DSURFACE9		m_lpCharacterShadowMapDepthSurface;
		D3DVIEWPORT9			m_ShadowMapViewport;
		uint16_t					m_wShadowMapSize;

		// Backup Device Context
		LPDIRECT3DSURFACE9		m_lpBackupRenderTargetSurface;
		LPDIRECT3DSURFACE9		m_lpBackupDepthSurface;
		D3DVIEWPORT9			m_BackupViewport;

		// Character Shadow
		//////////////////////////////////////////////////////////////////////////

		// View Frustum Culling
		D3DXPLANE					m_plane[6];

		void BuildViewFrustum(D3DXMATRIX & mat);

		CTextureSet					m_TextureSet;

	protected:
		CSkyBox						m_SkyBox;
		CLensFlare					m_LensFlare;
		CScreenFilter				m_ScreenFilter;

	protected:
		void SetIndexBuffer();
		void SelectIndexBuffer(uint8_t byLODLevel, uint16_t * pwPrimitiveCount, D3DPRIMITIVETYPE * pePrimitiveType);

		D3DXMATRIX m_matWorldForCommonUse;
		D3DXMATRIX m_matViewInverse;

		D3DXMATRIX m_matSplatAlpha;
		D3DXMATRIX m_matStaticShadow;
		D3DXMATRIX m_matDynamicShadow;
		D3DXMATRIX m_matDynamicShadowScale;
		D3DXMATRIX m_matLightView;

		float m_fTerrainTexCoordBase;
		float m_fWaterTexCoordBase;

		float m_fXforDistanceCaculation, m_fYforDistanceCaculation;

	protected:
		typedef std::vector<CTerrain *>		TTerrainPtrVector;
		typedef TTerrainPtrVector::iterator TTerrainPtrVectorIterator;
		typedef std::vector<CArea *>		TAreaPtrVector;
		typedef TAreaPtrVector::iterator	TAreaPtrVectorIterator;

		TTerrainPtrVector			m_TerrainVector;
		TTerrainPtrVector			m_TerrainDeleteVector;
		TTerrainPtrVector			m_TerrainLoadRequestVector;
		TTerrainPtrVector			m_TerrainLoadWaitVector;
		TTerrainPtrVectorIterator	m_TerrainPtrVectorIterator;

		TAreaPtrVector				m_AreaVector;
		TAreaPtrVector				m_AreaDeleteVector;
		TAreaPtrVector				m_AreaLoadRequestVector;
		TAreaPtrVector				m_AreaLoadWaitVector;
		TAreaPtrVectorIterator		m_AreaPtrVectorIterator;

		struct FPushToDeleteVector
		{
			enum EDeleteDir
			{
				DELETE_LEFT,
				DELETE_RIGHT,
				DELETE_TOP,
				DELETE_BOTTOM,
			};

			EDeleteDir m_eLRDeleteDir;
			EDeleteDir m_eTBDeleteDir;
			TOutdoorMapCoordinate m_CurCoordinate;

			FPushToDeleteVector(EDeleteDir eLRDeleteDir, EDeleteDir eTBDeleteDir, TOutdoorMapCoordinate CurCoord)
			{
				m_eLRDeleteDir = eLRDeleteDir;
				m_eTBDeleteDir = eTBDeleteDir;
				m_CurCoordinate = CurCoord;
			}
		};

		struct FPushTerrainToDeleteVector : public FPushToDeleteVector
		{
			TTerrainPtrVector	m_ReturnTerrainVector;

			FPushTerrainToDeleteVector(EDeleteDir eLRDeleteDir, EDeleteDir eTBDeleteDir, TOutdoorMapCoordinate CurCoord)
				: FPushToDeleteVector(eLRDeleteDir, eTBDeleteDir, CurCoord)
			{
				m_ReturnTerrainVector.clear();
			}

			void operator() (CTerrain * pTerrain);
		};

		struct FPushAreaToDeleteVector : public FPushToDeleteVector
		{
			TAreaPtrVector		m_ReturnAreaVector;

			FPushAreaToDeleteVector(EDeleteDir eLRDeleteDir, EDeleteDir eTBDeleteDir, TOutdoorMapCoordinate CurCoord)
				: FPushToDeleteVector(eLRDeleteDir, eTBDeleteDir, CurCoord)
			{
				m_ReturnAreaVector.clear();
			}

			void operator() (CArea * pArea);
		};

	protected:
		void InitializeVisibleParts();
		bool IsVisiblePart(int32_t ePart);

		float __GetNoFogDistance();
		float __GetFogDistance();


	protected:
		uint32_t m_dwVisiblePartFlags;

		int32_t m_iRenderedSplatNumSqSum;
		int32_t m_iRenderedSplatNum;
		int32_t m_iRenderedPatchNum;
		std::vector<int32_t> m_RenderedTextureNumVector;
		int32_t m_iSplatLimit;

	protected:
		int32_t m_iPatchTerrainVertexCount;
		int32_t m_iPatchWaterVertexCount;

		int32_t m_iPatchTerrainVertexSize;
		int32_t m_iPatchWaterVertexSize;

		uint32_t m_dwRenderedCRCNum;
		uint32_t m_dwRenderedGraphicThingInstanceNum;

		std::list<RECT> m_rkList_kGuildArea;

	protected:
		void __RenderTerrain_RecurseRenderQuadTree(CTerrainQuadtreeNode *Node, bool bCullCheckNeed = true);
		int32_t	 __RenderTerrain_RecurseRenderQuadTree_CheckBoundingCircle(const D3DXVECTOR3 & c_v3Center, const float & c_fRadius);

		void __RenderTerrain_AppendPatch(const D3DXVECTOR3& c_rv3Center, float fDistance, int32_t lPatchNum);

		void __RenderTerrain_RenderHardwareTransformPatch();

	protected:
		void __HardwareTransformPatch_RenderPatchSplat(int32_t patchnum, uint16_t wPrimitiveCount, D3DPRIMITIVETYPE ePrimitiveType);
		void __HardwareTransformPatch_RenderPatchNone(int32_t patchnum, uint16_t wPrimitiveCount, D3DPRIMITIVETYPE ePrimitiveType);

	protected:
		std::vector<CGraphicObjectInstance *> m_ShadowReceiverVector;
		std::vector<CGraphicObjectInstance *> m_PCBlockerVector;

	protected:
		float	m_fOpaqueWaterDepth;
		CGraphicImageInstance m_WaterInstances[30];

	public:
		float	GetOpaqueWaterDepth() { return m_fOpaqueWaterDepth;	}
		void	SetOpaqueWaterDepth(float fOpaqueWaterDepth) { m_fOpaqueWaterDepth = fOpaqueWaterDepth; }
		void	SetTerrainRenderSort(ETerrainRenderSort eTerrainRenderSort) { m_eTerrainRenderSort = eTerrainRenderSort;}
		ETerrainRenderSort	GetTerrainRenderSort() { return m_eTerrainRenderSort; }

	protected:
		ETerrainRenderSort m_eTerrainRenderSort;

	protected:
		CGraphicImageInstance	m_attrImageInstance;
		CGraphicImageInstance	m_BuildingTransparentImageInstance;
		D3DXMATRIX				m_matBuildingTransparent;

	public:
		void GetBaseXY(uint32_t * pdwBaseX, uint32_t * pdwBaseY);
		void SetBaseXY(uint32_t dwBaseX, uint32_t dwBaseY);

		void SetTransparentTree(bool bTransparentTree) { m_bTransparentTree = bTransparentTree;}
		void EnableTerrainOnlyForHeight(bool bFlag) { m_bEnableTerrainOnlyForHeight = bFlag; }
		void EnablePortal(bool bFlag);
		bool IsEnablePortal() { return m_bEnablePortal; }

	protected:
		uint32_t			m_dwBaseX;
		uint32_t			m_dwBaseY;

		D3DXVECTOR3		m_v3Player;
		
		bool			m_bShowEntirePatchTextureCount;
		bool			m_bTransparentTree;
		bool			m_bEnableTerrainOnlyForHeight;
		bool			m_bEnablePortal;

	// XMas
	private:
		struct SXMasTree
		{
			CSpeedTreeWrapper* m_pkTree;
			int32_t m_iEffectID;
		} m_kXMas;

		void __XMasTree_Initialize();
		void __XMasTree_Create(float x, float y, float z, const char* c_szTreeName, const char* c_szEffName);

	public:
		void XMasTree_Destroy();
		void XMasTree_Set(float x, float y, float z, const char* c_szTreeName, const char* c_szEffName);

	// Special Effect
	private:
		typedef std::map<uint32_t, int32_t> TSpecialEffectMap;
		TSpecialEffectMap m_kMap_dwID_iEffectID;

	public:
		void SpecialEffect_Create(uint32_t dwID, float x, float y, float z, const char* c_szEffName);
		void SpecialEffect_Delete(uint32_t dwID);
		void SpecialEffect_Destroy();

	private:
		struct SHeightCache 
		{
			struct SItem
			{
				uint32_t	m_dwKey;
				float	m_fHeight;
			};

			enum
			{
				HASH_SIZE = 100,
			};

			std::vector<SItem> m_akVct_kItem[HASH_SIZE];

			bool m_isUpdated;
		} m_kHeightCache;
		
		void __HeightCache_Init();
		void __HeightCache_Update();

	public:
		void SetEnvironmentDataName(const std::string& strEnvironmentDataName);
		std::string& GetEnvironmentDataName();

	protected:
		std::string		m_settings_envDataName;
		std::string		m_envDataName;

	private:
		bool m_bSettingTerrainVisible;
		bool m_bSettingIsLavaMap;
};
