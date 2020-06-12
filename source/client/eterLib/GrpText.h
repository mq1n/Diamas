#pragma once

#include "Resource.h"
#include "Ref.h"
#include "GrpFontTexture.h"

class CGraphicText : public CResource
{
	public:
		typedef CRef<CGraphicText> TRef;
		
	public:
		static TType Type();
		
	public:
		CGraphicText(const FileSystem::CFileName& filename);
		virtual ~CGraphicText();
		
		virtual bool			CreateDeviceObjects();
		virtual void			DestroyDeviceObjects();

		CGraphicFontTexture *	GetFontTexturePointer();

	protected:
		bool		OnLoad(int32_t iSize, const void * c_pvBuf);
		void		OnClear();
		bool		OnIsEmpty() const;
		bool		OnIsType(TType type);		
		
	protected:
		CGraphicFontTexture m_fontTexture;
};
