#include "StdAfx.h"
#include "../eterBase/CRC32.h"
#include "../eterBase/Timer.h"
#include "../eterBase/Stl.h"
#include <FileSystemIncl.hpp>

#include "ResourceManager.h"
#include "GrpImage.h"

#include <io.h>

const int32_t c_Deleting_Wait_Time = 30000;			// 삭제 대기 시간 (30초)
const int32_t c_DeletingCountPerFrame = 30;			// 프레임당 체크 리소스 갯수
const int32_t c_Reference_Decrease_Wait_Time = 30000;	// 선로딩 리소스의 해제 대기 시간 (30초)

void CResourceManager::LoadStaticCache(const char* c_szFileName)
{
	CResource* pkRes=GetResourcePointer(c_szFileName);
	if (!pkRes)
	{
		Lognf(1, "CResourceManager::LoadStaticCache %s - FAILED", c_szFileName);
		return;
	}

	uint32_t dwCacheKey=GetCRC32(c_szFileName, strlen(c_szFileName));
	auto f = m_pCacheMap.find(dwCacheKey);
	if (m_pCacheMap.end()!=f)
		return;

	pkRes->AddReference();
	m_pCacheMap.emplace(dwCacheKey, pkRes);
}

void CResourceManager::__DestroyCacheMap()
{
	TResourcePointerMap::iterator i;
	for (i = m_pCacheMap.begin(); i != m_pCacheMap.end(); ++i)
	{
		CResource* pResource = i->second;
		pResource->Release();
	}

	m_pCacheMap.clear();
}

void CResourceManager::__DestroyDeletingResourceMap()
{
	Tracenf("CResourceManager::__DestroyDeletingResourceMap %d", m_ResourceDeletingMap.size());
	for (auto & i : m_ResourceDeletingMap)
		(i.first)->Clear();

	m_ResourceDeletingMap.clear();
}

void CResourceManager::__DestroyResourceMap()
{
	Tracenf("CResourceManager::__DestroyResourceMap %d", m_pResMap.size());

	TResourcePointerMap::iterator i;
	for (i = m_pResMap.begin(); i != m_pResMap.end(); ++i)
	{
		CResource* pResource = i->second;
		pResource->Clear();
	}

	stl_wipe_second(m_pResMap);
}

void CResourceManager::DestroyDeletingList()
{
	CResource::SetDeleteImmediately(true);

	__DestroyCacheMap();
	__DestroyDeletingResourceMap();	
}

void CResourceManager::Destroy()
{
	assert(m_ResourceDeletingMap.empty() && "CResourceManager::Destroy - YOU MUST CALL DestroyDeletingList");
	__DestroyResourceMap();
}

void CResourceManager::RegisterResourceNewFunctionPointer(const char* c_szFileExt, CResource* (*pNewFunc)(const FileSystem::CFileName&))
{
	m_newFuncs.push_back(std::make_pair(c_szFileExt, pNewFunc));
}

CResource * CResourceManager::InsertResourcePointer(uint64_t nameHash, CResource* pResource)
{
	auto itor = m_pResMap.find(nameHash);
	if (m_pResMap.end() != itor)
	{
		const auto& stRefResourceName = pResource->GetFileNameString();

		TraceError("CResource::InsertResourcePointer: %s is already registered\n", stRefResourceName.c_str());
		assert(!"CResource::InsertResourcePointer: Resource already resistered");

		delete pResource;
		return itor->second;
	}

	m_pResMap.insert(TResourcePointerMap::value_type(nameHash, pResource));
	return pResource;
}

CResource * CResourceManager::GetResourcePointer(const char* c_szFileName)
{
	if (!c_szFileName || !*c_szFileName)
	{
		TraceError("CResourceManager::GetResourcePointer: filename error!");
		return nullptr;
	}

	FileSystem::CFileName filename(c_szFileName);

	CResource * pResource = FindResourcePointer(filename.GetHash());
	if (pResource) // 이미 리소스가 있으면 리턴 한다.
		return pResource;

	CResource* (*newFunc) (const FileSystem::CFileName&) = nullptr;

	auto stFileName = filename.GetPathA();
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

	return InsertResourcePointer(filename.GetHash(), newFunc(filename));
}

CResource* CResourceManager::FindResourcePointer(uint64_t nameHash)
{
	auto itor = m_pResMap.find(nameHash);
	if (m_pResMap.end() == itor)
		return nullptr;

	return itor->second;
}

bool CResourceManager::isResourcePointerData(uint64_t nameHash)
{
	auto itor = m_pResMap.find(nameHash);

	if (m_pResMap.end() == itor)
		return false;

	return (itor->second)->IsData();
}

typedef struct SDumpData
{
	const char *	filename;
	float			KB;
	uint32_t			cost;
} TDumpData;

bool DumpKBCompare(const TDumpData& lhs, const TDumpData& rhs)
{
	return (lhs.KB > rhs.KB) ? true : false;
}

bool DumpCostCompare(const TDumpData& lhs, const TDumpData& rhs)
{
	return (lhs.cost > rhs.cost) ? true : false;
}

struct FDumpPrint
{
	FILE * m_fp;
	static float m_totalKB;

	void operator () (TDumpData & data)
	{
		m_totalKB += data.KB;
		fprintf(m_fp, "%6.1f %s\n", data.KB, data.filename);
	}
};

float FDumpPrint::m_totalKB;

struct FDumpCostPrint
{
	FILE * m_fp;

	void operator() (TDumpData & data)
	{
		fprintf(m_fp, "%-4u %s\n", data.cost, data.filename);
	}
};

void CResourceManager::DumpFileListToTextFile(const char* c_szFileName)
{
	std::vector<TDumpData> dumpVector;

	for (auto & i : m_pResMap)
	{
		CResource * pResource = i.second;
		TDumpData data;

		if (pResource->IsEmpty())
			continue;
		
		auto stResourceName = pResource->GetFileNameString();
		data.filename = stResourceName.c_str();

		int32_t filesize;

		const char * ext = strrchr(data.filename, '.');

		if (pResource->IsType(CGraphicImage::Type()) && strnicmp(ext, ".sub", 4))
			filesize = ((CGraphicImage*) pResource)->GetWidth() * ((CGraphicImage*) pResource)->GetHeight() * 4;
		else
		{
			msl::file_ptr fPtr2(data.filename, "rb");

			if (fPtr2)
				filesize = fPtr2.size();
			else
				filesize = 0;
		}

		data.KB = (float) filesize / (float) 1024;
//		data.cost = pResource->GetLoadCostMilliSecond();

		dumpVector.emplace_back(data);
	}

	msl::file_ptr fPtr(c_szFileName, "w");

	if (fPtr)
	{
		std::sort(dumpVector.begin(), dumpVector.end(), DumpKBCompare);

		FDumpPrint DumpPrint;
		DumpPrint.m_fp = fPtr.get();
		DumpPrint.m_totalKB = 0;

		std::for_each(dumpVector.begin(), dumpVector.end(), DumpPrint);
		fprintf(fPtr.get(), "total: %.2fmb", DumpPrint.m_totalKB / 1024.0f);

		FDumpCostPrint DumpCostPrint;
		DumpCostPrint.m_fp = fPtr.get();

		std::sort(dumpVector.begin(), dumpVector.end(), DumpCostCompare);
		std::for_each(dumpVector.begin(), dumpVector.end(), DumpCostPrint);
		fprintf(fPtr.get(), "total: %.2fmb", DumpPrint.m_totalKB / 1024.0f);
	}
}

bool CResourceManager::IsFileExist(const char * c_szFileName)
{
	return FileSystemManager::Instance().DoesFileExist(c_szFileName);
}

void CResourceManager::Update()
{
	uint32_t CurrentTime = ELTimer_GetMSec();
	CResource * pResource;
	int32_t Count = 0;

	auto itor = m_ResourceDeletingMap.begin();

	while (itor != m_ResourceDeletingMap.end())
	{
		pResource = itor->first;

		if (CurrentTime >= itor->second)
		{
			if (pResource->canDestroy())
			{
				//Tracef("Resource Clear %s\n", pResource->GetFileName());
				pResource->Clear();
			}

			itor = m_ResourceDeletingMap.erase(itor);

			if (++Count >= c_DeletingCountPerFrame)
				break;
		}
		else
			++itor;
	}
}

void CResourceManager::ReserveDeletingResource(CResource * pResource)
{
	uint32_t dwCurrentTime = ELTimer_GetMSec();
	m_ResourceDeletingMap.emplace(pResource, dwCurrentTime + c_Deleting_Wait_Time);
}

CResourceManager::CResourceManager() = default;

CResourceManager::~CResourceManager()
{
	Destroy();
}
