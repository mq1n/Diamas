#pragma once

#include "GrpImage.h"
#include "Pool.h"

class CGraphicMarkInstance
{
	public:
		static uint32_t Type();
		BOOL IsType(uint32_t dwType);

		void SetImageFileName(const char* c_szFileName);
		const std::string& GetImageFileName();

	public:
		CGraphicMarkInstance();
		virtual ~CGraphicMarkInstance();

		void Destroy();

		void Render();

		void SetDiffuseColor(float fr, float fg, float fb, float fa);
		void SetPosition(float fx, float fy, float fz = 0.0f);
		void SetIndex(uint32_t uIndex);
		void SetScale(float fScale);

		void Load();
		bool IsEmpty() const;

		int32_t GetWidth();
		int32_t GetHeight();

		CGraphicTexture * GetTexturePointer();
		const CGraphicTexture &	GetTextureReference() const;
		CGraphicImage * GetGraphicImagePointer();

		bool operator == (const CGraphicMarkInstance & rhs) const;

	protected:
		enum
		{
			MARK_WIDTH = 16,
			MARK_HEIGHT = 12,
		};

		void Initialize();

		virtual void OnRender();
		virtual void OnSetImagePointer();

		virtual BOOL OnIsType(uint32_t dwType);

		void SetImagePointer(CGraphicImage * pImage);

	protected:
		D3DXCOLOR m_DiffuseColor;
		D3DXVECTOR3 m_v3Position;

		uint32_t m_uIndex;
	
		float m_fScale;
		float m_fDepth;

		CGraphicImage::TRef m_roImage;
		std::string m_stImageFileName;

	public:
		static void CreateSystem(uint32_t uCapacity);
		static void DestroySystem();

		static CGraphicMarkInstance* New();
		static void Delete(CGraphicMarkInstance* pkImgInst);

		static CDynamicPool<CGraphicMarkInstance> ms_kPool;
};
