#pragma once
#include "../eterBase/Singleton.h"
#include "Resource.h"
#include "FileLoaderThread.h"

#include <set>
#include <map>
#include <string>

class CResourceManager : public CSingleton<CResourceManager>
{
	public:
		CResourceManager();
		virtual ~CResourceManager();
		
		void		LoadStaticCache(const char* c_szFileName);

		void		DestroyDeletingList();
		void		Destroy();
		
		void		BeginThreadLoading();
		void		EndThreadLoading();

		CResource *	InsertResourcePointer(uint32_t dwFileCRC, CResource* pResource);
		CResource *	FindResourcePointer(uint32_t dwFileCRC);
		CResource *	GetResourcePointer(const char * c_szFileName);
		CResource *	GetTypeResourcePointer(const char * c_szFileName, int32_t iType=-1);

		// 추가
		bool		isResourcePointerData(uint32_t dwFileCRC);

		void		RegisterResourceNewFunctionPointer(const char* c_szFileExt, CResource* (*pResNewFunc)(const char* c_szFileName));
		void		RegisterResourceNewFunctionByTypePointer(int32_t iType, CResource* (*pNewFunc) (const char* c_szFileName));
		
		void		DumpFileListToTextFile(const char* c_szFileName);
		bool		IsFileExist(const char * c_szFileName);

		void		Update();
		void		ReserveDeletingResource(CResource * pResource);

	public:
		void		ProcessBackgroundLoading();
		void		PushBackgroundLoadingSet(std::set<std::string> & LoadingSet);

	protected:
		void		__DestroyDeletingResourceMap();
		void		__DestroyResourceMap();
		void		__DestroyCacheMap();

		uint32_t		__GetFileCRC(const char * c_szFileName, const char ** c_pszLowerFile = nullptr);
	
	protected:
		typedef std::map<uint32_t,	CResource *>									TResourcePointerMap;
		typedef std::map<std::string, CResource* (*)(const char*)>				TResourceNewFunctionPointerMap;
		typedef std::map<int32_t, CResource* (*)(const char*)>						TResourceNewFunctionByTypePointerMap;
		typedef std::map<CResource *, uint32_t>									TResourceDeletingMap;
		typedef std::map<uint32_t, std::string>									TResourceRequestMap;
		typedef std::map<int32_t, CResource*>										TResourceRefDecreaseWaitingMap;

	protected:
		TResourcePointerMap						m_pCacheMap;
		TResourcePointerMap						m_pResMap;
		TResourceNewFunctionPointerMap			m_pResNewFuncMap;
		TResourceNewFunctionByTypePointerMap	m_pResNewFuncByTypeMap;
		TResourceDeletingMap					m_ResourceDeletingMap;
		TResourceRequestMap						m_RequestMap;	// 쓰레드로 로딩 요청한 리스트
		TResourceRequestMap						m_WaitingMap;
		TResourceRefDecreaseWaitingMap			m_pResRefDecreaseWaitingMap;

		static CFileLoaderThread				ms_loadingThread;
};

extern int32_t g_iLoadingDelayTime;
