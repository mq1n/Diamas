#pragma once
#include "../../common/Singleton.h"
#include "../../common/cast.h"
#include "Resource.h"
#include "FileLoaderThread.h"

#include <set>
#include <map>
#include <string>

class CResourceManager : public CSingleton <CResourceManager>
{
	using TNewFunc				= std::function<CResource*(const FileSystem::CFileName&)>;
	using NewFunc				= std::pair <std::string, TNewFunc>;
	using TResourcePointerMap	= std::unordered_map <uint64_t, CResource*>;
	using TResourceDeletingMap  = std::unordered_map<CResource*, uint64_t>;

	public:
		CResourceManager();
		virtual ~CResourceManager();

		void LoadStaticCache(const char* c_szFileName);

		void DestroyDeletingList();
		void Destroy();

		CResource* InsertResourcePointer(uint64_t nameHash, CResource* pResource);
		CResource* FindResourcePointer(uint64_t nameHash);

		template <typename T>
		T* GetResourcePointer(std::string stFileName);

		bool isResourcePointerData(uint64_t nameHash);

		void RegisterResourceNewFunctionPointer(const char* c_szFileExt, TNewFunc pResNewFunc);

		void DumpFileListToTextFile(const char* c_szFileName);
		static bool IsFileExist(const char * c_szFileName);

		void Update();
		void ReserveDeletingResource(CResource * pResource);

	protected:
		void __DestroyDeletingResourceMap();
		void __DestroyResourceMap();
		void __DestroyCacheMap();

	private:
		TResourcePointerMap		m_pCacheMap;
		TResourcePointerMap		m_pResMap;
		std::vector <NewFunc>	m_newFuncs;
		TResourceDeletingMap	m_ResourceDeletingMap;
};

template <typename T>
T* CResourceManager::GetResourcePointer(std::string stFileName)
{
	if (stFileName.empty())
	{
		TraceError("CResourceManager::GetResourcePointer: filename error!");
		return nullptr;
	}

	const auto filename = FileSystem::CFileName{stFileName};
	const auto hash = filename.GetHash();

	auto pResource = FindResourcePointer(hash);
	if (pResource)
		return msl::inherit_cast<T*>(pResource);

	TNewFunc newFunc = nullptr;

	stFileName = filename.GetPathA(); // Set lowered filename
	const auto fileExt = stFileName.rfind('.');
	if (fileExt != std::string::npos)
	{
		const auto it = std::find_if(m_newFuncs.begin(), m_newFuncs.end(), [&filename, fileExt, &stFileName] (const NewFunc& nf)
		{
			return 0 == stFileName.compare(fileExt + 1, std::string::npos, nf.first);
		});

		if (it != m_newFuncs.end())
			newFunc = it->second;
	}
	else
	{
		TraceError("ResourceManager::GetResourcePointer: BROKEN FILE NAME: %s", stFileName.c_str());
		return nullptr;
	}

	if (!newFunc)
	{
		TraceError("ResourceManager::GetResourcePointer: NOT SUPPORT FILE %s", stFileName.c_str());
		return nullptr;
	}

	pResource = InsertResourcePointer(hash, newFunc(filename));
	return msl::inherit_cast<T*>(pResource);
}
