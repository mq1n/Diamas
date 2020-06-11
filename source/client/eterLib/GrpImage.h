#ifndef __INC_GRPIMAGE_H__
#define __INC_GRPIMAGE_H__

#include "Ref.h"
#include "Resource.h"
#include "GrpImageTexture.h"

class CGraphicImage : public CResource
{
	public:
		typedef CRef<CGraphicImage> TRef;

	public:
		static TType Type();

	public:
		CGraphicImage(const char* c_szFileName, uint32_t dwFilter = D3DX_FILTER_LINEAR);
		virtual ~CGraphicImage();

		virtual bool CreateDeviceObjects();
		virtual void DestroyDeviceObjects();

		int32_t GetWidth() const;
		int32_t GetHeight() const;

		const RECT & GetRectReference() const;

		const CGraphicTexture & GetTextureReference() const;
		CGraphicTexture * GetTexturePointer();

	protected:
		bool OnLoad(int32_t iSize, const void * c_pvBuf);
		
		void OnClear();	
		bool OnIsEmpty() const;
		bool OnIsType(TType type);

	protected:
		CGraphicImageTexture	m_imageTexture;
		RECT					m_rect;
		uint32_t					m_dwFilter;
};

#endif
