#pragma once

#include "../eterEffectLib/StdAfx.h"
#include "../eterLib/Resource.h"
#include "../eterLib/ResourceManager.h"

enum EResourceTypes
{
	RES_TYPE_UNKNOWN
};

class CPythonResource : public CSingleton<CPythonResource>
{
	public:
		CPythonResource();
		virtual ~CPythonResource();
		
		void Destroy();

		void DumpFileList(const char * c_szFileName);

	protected:
		CResourceManager m_resManager;
};