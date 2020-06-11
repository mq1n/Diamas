#pragma once

#include "Ref.h"
#include "Resource.h"
#include "../eterImageLib/TGAImage.h"

class CTargaResource : public CResource
{
	public:
		typedef CRef<CTargaResource> TRef;

	public:
		static TType Type();

	public:
		CTargaResource(const char * c_pszFileName);
		virtual ~CTargaResource();
		
		uint32_t *			GetMemPtr();
		void			GetRect(uint32_t & w, uint32_t & h);
		
		TGA_HEADER &	GetTgaHeader();

	protected:
		virtual bool OnLoad(int32_t iSize, const void * c_pvBuf);
		virtual void OnClear();	
		virtual bool OnIsEmpty() const;
		virtual bool OnIsType(TType type);

	protected:
		CTGAImage	image;
};
