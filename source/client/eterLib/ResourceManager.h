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

		void LoadStaticCache(const char* c_szFileName);

		void DestroyDeletingList();
		void Destroy();

		CResource* InsertResourcePointer(uint64_t nameHash, CResource* pResource);
		CResource* FindResourcePointer(uint64_t nameHash);
		CResource* GetResourcePointer(const char* c_szFileName);

		// Ãß°¡
		bool isResourcePointerData(uint64_t nameHash);

		void RegisterResourceNewFunctionPointer(const char* c_szFileExt,
		                                        CResource* (*pResNewFunc)(const FileSystem::CFileName&));

		void DumpFileListToTextFile(const char* c_szFileName);
		bool IsFileExist(const char * c_szFileName);

		void Update();
		void ReserveDeletingResource(CResource * pResource);

	protected:
		typedef std::pair<std::string, CResource* (*)(const FileSystem::CFileName&)> NewFunc;
		typedef std::unordered_map<uint64_t, CResource*> TResourcePointerMap;
		typedef std::unordered_map<CResource*, uint64_t> TResourceDeletingMap;

		void __DestroyDeletingResourceMap();
		void __DestroyResourceMap();
		void __DestroyCacheMap();

		TResourcePointerMap m_pCacheMap;
		TResourcePointerMap m_pResMap;
		std::vector<NewFunc> m_newFuncs;
		TResourceDeletingMap m_ResourceDeletingMap;
};

