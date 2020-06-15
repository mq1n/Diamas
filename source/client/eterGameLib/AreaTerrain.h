#pragma once

class CMapOutdoor;

#include "../eterTerrainLib/Terrain.h"
#include "TerrainPatch.h"

class CTerrain : public CTerrainImpl, public CGraphicBase
{
	public:

		enum EBoundaryLoadPart
		{
			LOAD_INVALID,
			LOAD_NOBOUNDARY,
			LOAD_TOPLEFT,
			LOAD_TOP,
			LOAD_TOPRIGHT,
			LOAD_LEFT,
			LOAD_RIGHT,
			LOAD_BOTTOMLEFT,
			LOAD_BOTTOM,
			LOAD_BOTTOMRIGHT,
			LOAD_ALLBOUNDARY
		};

		CTerrain();
		virtual ~CTerrain();

		virtual void	Clear();
		
		void			SetMapOutDoor(CMapOutdoor * pOwnerOutdoorMap);

		//////////////////////////////////////////////////////////////////////////
		// Loading
		bool			RAW_LoadTileMap(const char * c_pszFileName, bool bBGLoading = false);
		
		bool			LoadHeightMap(const char * c_pszFileName);

		void			CalculateTerrainPatch();
		//////////////////////////////////////////////////////////////////////////

		void			CopySettingFromGlobalSetting();

		uint16_t			WE_GetHeightMapValue(int16_t sX, int16_t sY);

		bool			IsReady()						{ return m_bReady; }
		void			SetReady(bool bReady = true)	{ m_bReady = bReady; }
		
		// Height Map
		uint16_t *			GetHeightMap()			{ return m_awRawHeightMap; }
		float			GetHeight(int32_t x, int32_t y);

		// Normal Map
		bool			GetNormal(int32_t ix, int32_t iy, D3DXVECTOR3 * pv3Normal);

		// TileMap
		uint8_t *			RAW_GetTileMap()		{ return m_abyTileMap; }
		char *			GetNormalMap()			{ return m_acNormalMap; }

		// Attr
		bool			LoadAttrMap(const char * c_pszFileName);
		uint8_t *			GetAttrMap()			{ return m_abyAttrMap; }
		uint8_t 			GetAttr(uint16_t wCoordX, uint16_t wCoordY);
		bool			isAttrOn(uint16_t wCoordX, uint16_t wCoordY, uint8_t byAttrFlag);

		//////////////////////////////////////////////////////////////////////////
		// Water
		uint8_t *			GetWaterMap()	{ return m_abyWaterMap; }
		void			GetWaterHeight(uint8_t byWaterNum, int32_t * plWaterHeight);
		bool			GetWaterHeight(uint16_t wCoordX, uint16_t wCoordY, int32_t * plWaterHeight);

		// Shadow Texture
		void				LoadShadowTexture(const char * c_pszFileName);

		// Shadow Map
		bool				LoadShadowMap(const char * c_pszFileName);

		// MiniMap
		void						LoadMiniMapTexture(const char * c_pszFileName);
		inline LPDIRECT3DTEXTURE9	GetMiniMapTexture() { return m_lpMiniMapTexture; }

		// Marked Area
		BOOL						IsMarked() { return m_bMarked; }
		void						AllocateMarkedSplats(uint8_t * pbyAlphaMap);
		void						DeallocateMarkedSplats();
		TTerrainSplatPatch &		GetMarkedSplatPatch() { return m_MarkedSplatPatch; }

		// Coordinate
		void			GetCoordinate(uint16_t * usCoordX, uint16_t * usCoordY)
		{
			*usCoordX = m_wX;
			*usCoordY = m_wY;
		}

		void			SetCoordinate(uint16_t wCoordX, uint16_t wCoordY);

		std::string &	GetName() { return m_strName; }
		void			SetName(const std::string &c_strName) { m_strName = c_strName; }

		CMapOutdoor *	GetOwner() { return m_pOwnerOutdoorMap; }
		void			RAW_GenerateSplat(bool bBGLoading = false);

	protected:
		bool	Initialize();
		void	RAW_AllocateSplats(bool bBGLoading = false);
		void	RAW_DeallocateSplats(bool bBGLoading = false);
		virtual void RAW_CountTiles();

		LPDIRECT3DTEXTURE9 AddTexture32(uint8_t byImageNum, uint8_t * pbyImage, int32_t lTextureWidth, int32_t lTextureHeight);
		void PutImage32(uint8_t * pbySrc, uint8_t * pbyDst, int32_t src_pitch, int32_t dst_pitch, int32_t lTextureWidth, int32_t lTextureHeight, bool bResize = false);
		void PutImage16(uint8_t * pbySrc, uint8_t * pbyDst, int32_t src_pitch, int32_t dst_pitch, int32_t lTextureWidth, int32_t lTextureHeight, bool bResize = false);

	protected:
		void CalculateNormal(int32_t x, int32_t y);

	protected:
		std::string				m_strName;
		uint16_t					m_wX;
		uint16_t					m_wY;

	protected:
		bool					m_bReady;

		CGraphicImageInstance	m_ShadowGraphicImageInstance;

		//MiniMap
		CGraphicImageInstance	m_MiniMapGraphicImageInstance;
		LPDIRECT3DTEXTURE9		m_lpMiniMapTexture;


		// Owner COutdoorMap poineter
		CMapOutdoor *			m_pOwnerOutdoorMap;

		// Picking
		D3DXVECTOR3				m_v3Pick;

		uint32_t					m_dwNumTexturesShow;
		std::vector<uint32_t>		m_VectorNumShowTexture;

		CTerrainPatch			m_TerrainPatchList[PATCH_XCOUNT * PATCH_YCOUNT];

		BOOL					m_bMarked;
		TTerrainSplatPatch		m_MarkedSplatPatch;
		LPDIRECT3DTEXTURE9		m_lpMarkedTexture;

	public:
		CTerrainPatch *	GetTerrainPatchPtr(uint8_t byPatchNumX, uint8_t byPatchNumY);

	protected:
		void _CalculateTerrainPatch(uint8_t byPatchNumX, uint8_t byPatchNumY);

	public:
		static void DestroySystem();

		static CTerrain* New();
		static void Delete(CTerrain* pkTerrain);

		static CDynamicPool<CTerrain>		ms_kPool;
};
