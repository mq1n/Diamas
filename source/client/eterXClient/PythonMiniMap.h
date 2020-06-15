#pragma once

#include "PythonBackground.h"

class CPythonMiniMap : public CScreen, public CSingleton<CPythonMiniMap>
{
	public:
		enum
		{
			EMPIRE_NUM = 4,

			MINI_WAYPOINT_IMAGE_COUNT = 12,
			WAYPOINT_IMAGE_COUNT = 15,
			TARGET_MARK_IMAGE_COUNT = 2,
			STORM_CIRCLE_IMAGE_COUNT = 1,
			SAFE_ZONE_IMAGE_COUNT = 1
		};

		enum
		{
			TYPE_OPC,
			TYPE_OPCPVP,
			TYPE_OPCPVPSELF,
			TYPE_NPC,
			TYPE_MONSTER,
			TYPE_WARP,
			TYPE_WAYPOINT,
			TYPE_PARTY,
			TYPE_EMPIRE,
			TYPE_EMPIRE_END = TYPE_EMPIRE + EMPIRE_NUM,
			TYPE_TARGET,
			TYPE_SHOP,
			TYPE_COUNT
		};

	public:
		CPythonMiniMap();
		virtual ~CPythonMiniMap();

		void Destroy();
		bool Create();

		bool IsAtlas();
		bool CanShow();
		bool CanShowAtlas();

		void SetMiniMapSize(float fWidth, float fHeight);
		void SetScale(float fScale);
		void ScaleUp();
		void ScaleDown();

		void SetCenterPosition(float fCenterX, float fCenterY);

		void Update(float fCenterX, float fCenterY);
		void Render(float fScreenX, float fScreenY);

		void Show();
		void Hide();

		bool GetPickedInstanceInfo(float fScreenX, float fScreenY, std::string & rReturnName, float * pReturnPosX, float * pReturnPosY, uint32_t * pdwTextColor);

		// Atlas
		bool LoadAtlas();
		void UpdateAtlas();
		void RenderAtlas(float fScreenX, float fScreenY);
		void ShowAtlas();
		void HideAtlas();
		bool ToggleAtlasMarker(int32_t type);

		bool GetAtlasInfo(float fScreenX, float fScreenY, std::string & rReturnString, float * pReturnPosX, float * pReturnPosY, uint32_t * pdwTextColor, uint32_t * pdwGuildID);
		bool GetAtlasSize(float * pfSizeX, float * pfSizeY);

		void AddObserver(uint32_t dwVID, float fSrcX, float fSrcY);
		void MoveObserver(uint32_t dwVID, float fDstX, float fDstY);
		void RemoveObserver(uint32_t dwVID);

		// WayPoint
		void AddWayPoint(uint8_t byType, uint32_t dwID, float fX, float fY, const std::string &strText, uint32_t dwChrVID=0);
		void RemoveWayPoint(uint32_t dwID);

		// SignalPoint
		void AddSignalPoint(float fX, float fY);
		void ClearAllSignalPoint();

		void RegisterAtlasWindow(PyObject* poHandler);
		void UnregisterAtlasWindow();
		void OpenAtlasWindow();
		void SetAtlasCenterPosition(int32_t x, int32_t y);

		// NPC List
		void ClearAtlasMarkInfo();
		void ClearAtlasShopInfo();
		void ClearAtlasPartyInfo();
		void RegisterAtlasMark(uint8_t byType, const char * c_szName, int32_t lx, int32_t ly);

		// Guild
		void ClearGuildArea();
		void RegisterGuildArea(uint32_t dwID, uint32_t dwGuildID, int32_t x, int32_t y, int32_t width, int32_t height);
		void UpdateGuildArea(uint32_t updateID, uint32_t updatedGuild);
		uint32_t GetGuildAreaID(uint32_t x, uint32_t y);

		// Target
		void CreateTarget(int32_t iID, const char * c_szName);
		void CreateTarget(int32_t iID, const char * c_szName, uint32_t dwVID);
		void UpdateTarget(int32_t iID, uint32_t ix, uint32_t iy);
		void DeleteTarget(int32_t iID);

		// Primal Law
		void SetStormCircle(int32_t ix, int32_t iy, int32_t radius);
		void SetNextSafeZoneCircle(int32_t ix, int32_t iy, int32_t radius);

	protected:
		void __Initialize();
		void __SetPosition();
		void __LoadAtlasMarkInfo();

		void __RenderWayPointMark(int32_t ixCenter, int32_t iyCenter);
		void __RenderMiniWayPointMark(int32_t ixCenter, int32_t iyCenter);
		void __RenderTargetMark(int32_t ixCenter, int32_t iyCenter);
		void __RenderStormCircle();
		void __RenderNextSafeZoneCircle();

		void __GlobalPositionToAtlasPosition(int32_t lx, int32_t ly, float * pfx, float * pfy);

	protected:
		// Atlas
		typedef struct 
		{
			uint8_t m_byType;
			uint32_t m_dwID; // For WayPoint
			float m_fX;
			float m_fY;
			float m_fScreenX;
			float m_fScreenY;
			float m_fMiniMapX;
			float m_fMiniMapY;
			uint32_t m_dwChrVID;
			std::string m_strText;
		} TAtlasMarkInfo;

		// GuildArea
		typedef struct
		{
			uint32_t dwID;
			uint32_t dwGuildID;
			int32_t lx, ly;
			int32_t lwidth, lheight;

			float fsxRender, fsyRender;
			float fexRender, feyRender;
		} TGuildAreaInfo;

		struct SObserver
		{
			float fCurX;
			float fCurY;
			float fSrcX;
			float fSrcY;
			float fDstX;
			float fDstY;

			uint32_t dwSrcTime;
			uint32_t dwDstTime;
		};

		// 캐릭터 리스트
		typedef struct 
		{
			uint32_t	m_bType;
			float	m_fX;
			float	m_fY;
			uint32_t	m_eNameColor;
		} TMarkPosition;

		typedef std::vector<TMarkPosition>				TInstanceMarkPositionVector;
		typedef TInstanceMarkPositionVector::iterator	TInstancePositionVectorIterator;

	protected:
		bool __GetWayPoint(uint32_t dwID, TAtlasMarkInfo ** ppkInfo);
		void __UpdateWayPoint(TAtlasMarkInfo * pkInfo, int32_t ix, int32_t iy);

	protected:
		float							m_fWidth;
		float							m_fHeight;

		float							m_fScale;

		float							m_fCenterX;
		float							m_fCenterY;

		float							m_fCenterCellX;
		float							m_fCenterCellY;

		float							m_fScreenX;
		float							m_fScreenY;

		float							m_fMiniMapRadius;

		// 맵 그림...
		LPDIRECT3DTEXTURE9				m_lpMiniMapTexture[AROUND_AREA_NUM];

		// 미니맵 커버
		CGraphicImageInstance			m_MiniMapFilterGraphicImageInstance;
		CGraphicExpandedImageInstance	m_MiniMapCameraraphicImageInstance;

		// 캐릭터 마크
		CGraphicExpandedImageInstance	m_PlayerMark;
		CGraphicImageInstance			m_WhiteMark;

		std::vector<TMarkPosition>		m_MinimapPosVector;
		std::map<uint32_t, SObserver>		m_kMap_dwVID_kObserver;

		bool							m_bAtlas;
		bool							m_bShow;

		CGraphicVertexBuffer			m_VertexBuffer;
		CGraphicIndexBuffer				m_IndexBuffer;

		D3DXMATRIX						m_matIdentity;
		D3DXMATRIX						m_matWorld;
		D3DXMATRIX						m_matMiniMapCover;

		bool							m_bShowAtlas;
		bool							m_bAtlasRenderShops;
		bool							m_bAtlasRenderNpc;
		bool							m_bAtlasRenderWarp;
		bool							m_bAtlasRenderWaypoint;
		bool							m_bAtlasRenderParty;

		CGraphicImageInstance			m_AtlasImageInstance;
		D3DXMATRIX						m_matWorldAtlas;
		CGraphicExpandedImageInstance	m_AtlasPlayerMark;

		float							m_fAtlasScreenX;
		float							m_fAtlasScreenY;

		uint32_t							m_dwAtlasBaseX;
		uint32_t							m_dwAtlasBaseY;

		float							m_fAtlasMaxX;
		float							m_fAtlasMaxY;

		float							m_fAtlasImageSizeX;
		float							m_fAtlasImageSizeY;

		typedef std::vector<TAtlasMarkInfo>		TAtlasMarkInfoVector;
		typedef TAtlasMarkInfoVector::iterator	TAtlasMarkInfoVectorIterator;
		typedef std::vector<TGuildAreaInfo>		TGuildAreaInfoVector;
		typedef TGuildAreaInfoVector::iterator	TGuildAreaInfoVectorIterator;
		TAtlasMarkInfoVectorIterator			m_AtlasMarkInfoListIterator;
		TAtlasMarkInfoVector					m_AtlasNPCInfoVector;
		TAtlasMarkInfoVector					m_AtlasShopInfoVector;
		TAtlasMarkInfoVector					m_AtlasWarpInfoVector;
		TAtlasMarkInfoVector					m_AtlasPartyInfoVector;

		// WayPoint
		CGraphicExpandedImageInstance			m_MiniWayPointGraphicImageInstances[MINI_WAYPOINT_IMAGE_COUNT];
		CGraphicExpandedImageInstance			m_WayPointGraphicImageInstances[WAYPOINT_IMAGE_COUNT];
		CGraphicExpandedImageInstance			m_TargetMarkGraphicImageInstances[TARGET_MARK_IMAGE_COUNT];
		CGraphicImageInstance					m_GuildAreaFlagImageInstance;
		TAtlasMarkInfoVector					m_AtlasWayPointInfoVector;
		TGuildAreaInfoVector					m_GuildAreaInfoVector;

		// Primal Law
		CGraphicExpandedImageInstance			m_StormIndicatorInstances[STORM_CIRCLE_IMAGE_COUNT];
		CGraphicExpandedImageInstance			m_NextSafezoneIndicatorInstances[SAFE_ZONE_IMAGE_COUNT];
		bool									m_bShowStormCircle;
		bool									m_bShowSafezoneCircle;
		float									m_fStormCircleAlpha;
		float									m_fSafezoneCircleAlpha;

		// SignalPoint
		struct TSignalPoint
		{
			D3DXVECTOR2 v2Pos;
			uint32_t id;
		};
		std::vector<TSignalPoint>				m_SignalPointVector;

		PyObject*							m_poHandler;
};