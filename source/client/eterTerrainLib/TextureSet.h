#ifndef __INC_TERRAINLIB_TEXTURESET_H__
#define __INC_TERRAINLIB_TEXTURESET_H__

#include "../eterLib/GrpImageInstance.h"

typedef struct STerrainTexture
{
	STerrainTexture() :	pd3dTexture(nullptr),
		UScale(4.0f),
		VScale(4.0f),
		UOffset(0.0f),
		VOffset(0.0f),
		bSplat(true),
		Begin(0),
		End(0)
	{
	}
	
	~STerrainTexture()
	{
	}

	std::string					stFilename;
	LPDIRECT3DTEXTURE9			pd3dTexture;
	CGraphicImageInstance 		ImageInstance;
	float						UScale;
	float						VScale;
	float						UOffset;
	float						VOffset;
	bool						bSplat;
	uint16_t				Begin, End;	// 0 ~ 65535 의 16bit heightfield 높이값.
	D3DXMATRIX					m_matTransform;
} TTerrainTexture;

class CTextureSet
{
	public:
		typedef std::vector<TTerrainTexture> TTextureVector;

		CTextureSet();
		virtual ~CTextureSet();

		void			Initialize();
		void			Clear();

		void			Create();

		bool			Load(const char * c_pszFileName, float fTerrainTexCoordBase);
		bool			Save(const char * c_pszFileName);
		
		uint32_t	GetTextureCount();
		
		TTerrainTexture	& GetTexture(uint32_t ulIndex);
		bool			RemoveTexture(uint32_t ulIndex);

		bool			SetTexture(uint32_t ulIndex,
								   const char * c_szFileName,
			 					   float fuScale,
							       float fvScale,
								   float fuOffset,
								   float fvOffset,
								   bool bSplat,
								   uint16_t usBegin,
								   uint16_t usEnd,
								   float fTerrainTexCoordBase);

		void			Reload(float fTerrainTexCoordBase);
		bool			ReplaceTexture(const char * c_szFileName,
								const char * c_OldszFileName,
								float fuScale,
								float fvScale,
								float fuOffset,
								float fvOffset,
								bool bSplat,
								uint16_t usBegin,
								uint16_t usEnd,
								float fTerrainTexCoordBase);

		bool			AddTexture(const char * c_szFileName,
			 					   float fuScale,
							       float fvScale,
								   float fuOffset,
								   float fvOffset,
								   bool bSplat,
								   uint16_t usBegin,
								   uint16_t usEnd,
								   float fTerrainTexCoordBase);

		const char *	GetFileName()	{ return m_stFileName.c_str(); }

	protected:
		void			AddEmptyTexture();

	protected:
		TTextureVector			m_Textures;
		TTerrainTexture			m_ErrorTexture;
		std::string				m_stFileName;
};

#endif