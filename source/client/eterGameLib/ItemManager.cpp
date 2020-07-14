#include "StdAfx.h"
#include <FileSystemIncl.hpp>
#include "../eterLib/ResourceManager.h"
#include "../eterBase/lzo.h"
#include "ItemManager.h"

static uint32_t s_adwItemProtoKey[4] =
{
	173217,
	72619434,
	408587239,
	27973291
};

BOOL CItemManager::SelectItemData(uint32_t dwIndex)
{
	auto f = m_ItemMap.find(dwIndex);

	if (m_ItemMap.end() == f)
	{
		int32_t n = m_vec_ItemRange.size();
		for (int32_t i = 0; i < n; i++)
		{
			CItemData * p = m_vec_ItemRange[i];
			const SItemTable * pTable = p->GetTable(); 
			if ((pTable->dwVnum < dwIndex) &&
				dwIndex < (pTable->dwVnum + pTable->dwVnumRange))
			{
				m_pSelectedItemData = p;
				return TRUE;
			}
		}
		Tracef(" CItemManager::SelectItemData - FIND ERROR [%d]\n", dwIndex);
		return FALSE;
	}

	m_pSelectedItemData = f->second;

	return TRUE;
}

CItemData * CItemManager::GetSelectedItemDataPointer()
{
	return m_pSelectedItemData;
}

BOOL CItemManager::GetItemDataPointer(uint32_t dwItemID, CItemData ** ppItemData)
{
	if (0 == dwItemID)
		return FALSE;

	auto f = m_ItemMap.find(dwItemID);

	if (m_ItemMap.end() == f)
	{
		int32_t n = m_vec_ItemRange.size();
		for (int32_t i = 0; i < n; i++)
		{
			CItemData * p = m_vec_ItemRange[i];
			const SItemTable * pTable = p->GetTable(); 
			if ((pTable->dwVnum < dwItemID) &&
				dwItemID < (pTable->dwVnum + pTable->dwVnumRange))
			{
				*ppItemData = p;
				return TRUE;
			}
		}
		Tracef(" CItemManager::GetItemDataPointer - FIND ERROR [%d]\n", dwItemID);
		return FALSE;
	}

	*ppItemData = f->second;

	return TRUE;
}

CItemData * CItemManager::MakeItemData(uint32_t dwIndex)
{
	auto f = m_ItemMap.find(dwIndex);

	if (m_ItemMap.end() == f)
	{
		CItemData * pItemData = CItemData::New();

		m_ItemMap.emplace(dwIndex, pItemData);

		return pItemData;
	}

	return f->second;
}

////////////////////////////////////////////////////////////////////////////////////////
// Load Item Table

bool CItemManager::LoadItemList(const char * c_szFileName)
{
	CFile File;
	if (!FileSystemManager::Instance().OpenFile(c_szFileName, File))
		return false;

	CMemoryTextFileLoader textFileLoader;
	textFileLoader.Bind(File.GetSize(), File.GetData());

	CTokenVector TokenVector;
    for (uint32_t i = 0; i < textFileLoader.GetLineCount(); ++i)
	{
		if (!textFileLoader.SplitLine(i, &TokenVector, "\t"))
			continue;

		if (!(TokenVector.size() == 3 || TokenVector.size() == 4))
		{
			TraceError(" CItemManager::LoadItemList(%s) - StrangeLine in %d\n", c_szFileName, i);
			continue;
		}

		const std::string & c_rstrID = TokenVector[0];
		//const std::string & c_rstrType = TokenVector[1];
		const std::string & c_rstrIcon = TokenVector[2];

		uint32_t dwItemVNum=atoi(c_rstrID.c_str());

		CItemData * pItemData = MakeItemData(dwItemVNum);

		
		if (4 == TokenVector.size())
		{
			const std::string & c_rstrModelFileName = TokenVector[3];
			pItemData->SetDefaultItemData(c_rstrIcon.c_str(), c_rstrModelFileName.c_str());
		}
		else
		{
			pItemData->SetDefaultItemData(c_rstrIcon.c_str());
		}
	}

	return true;
}

const std::string& __SnapString(const std::string& c_rstSrc, std::string& rstTemp)
{
	uint32_t uSrcLen=c_rstSrc.length();
	if (uSrcLen<2)
		return c_rstSrc;

	if (c_rstSrc[0]!='"')
		return c_rstSrc;

	uint32_t uLeftCut=1;
	
	uint32_t uRightCut=uSrcLen;
	if (c_rstSrc[uSrcLen-1]=='"')
		uRightCut=uSrcLen-1;	

	rstTemp=c_rstSrc.substr(uLeftCut, uRightCut-uLeftCut);
	return rstTemp;
}

bool CItemManager::LoadItemDesc(const char* c_szFileName)
{
	CFile kFile;
	if (!FileSystemManager::Instance().OpenFile(c_szFileName, kFile))
	{
		Tracenf("CItemManager::LoadItemDesc(c_szFileName=%s) - Load Error", c_szFileName);
		return false;
	}

	CMemoryTextFileLoader kTextFileLoader;
	kTextFileLoader.Bind(kFile.GetSize(), kFile.GetData());

	std::string stTemp;

	CTokenVector kTokenVector;
	for (uint32_t i = 0; i < kTextFileLoader.GetLineCount(); ++i)
	{
		if (!kTextFileLoader.SplitLineByTab(i, &kTokenVector))
			continue;

		while (kTokenVector.size()<ITEMDESC_COL_NUM)
			kTokenVector.emplace_back("");

		//assert(kTokenVector.size()==ITEMDESC_COL_NUM);
		
		uint32_t dwVnum=atoi(kTokenVector[ITEMDESC_COL_VNUM].c_str());
		const std::string& c_rstDesc=kTokenVector[ITEMDESC_COL_DESC];
		const std::string& c_rstSumm=kTokenVector[ITEMDESC_COL_SUMM];
		auto f = m_ItemMap.find(dwVnum);
		if (m_ItemMap.end() == f)
			continue;

		CItemData* pkItemDataFind = f->second;

		pkItemDataFind->SetDescription(__SnapString(c_rstDesc, stTemp));
		pkItemDataFind->SetSummary(__SnapString(c_rstSumm, stTemp));
	}
	return true;
}

uint32_t GetHashCode( const char* pString )
{
	   uint32_t i,len;
	   uint32_t ch;
	   uint32_t result;
	   
	   len     = strlen( pString );
	   result = 5381;
	   for( i=0; i<len; i++ )
	   {
	   	   ch = (uint32_t)pString[i];
	   	   result = ((result<< 5) + result) + ch; // hash * 33 + ch
	   }	  

	   return result;
}

bool CItemManager::LoadItemTable(const char* c_szFileName)
{	
	CFile file;
	if (!FileSystemManager::Instance().OpenFile(c_szFileName, file))
		return false;

	uint32_t dwFourCC, dwElements, dwDataSize;
	uint32_t dwVersion=0;
	uint32_t dwStride=0;

	file.Read(&dwFourCC, sizeof(uint32_t));

	if (dwFourCC == MAKEFOURCC('M', 'I', 'P', 'X'))
	{
		file.Read(&dwVersion, sizeof(uint32_t));
		file.Read(&dwStride, sizeof(uint32_t));
	
		if (dwVersion != 1)
		{
			TraceError("CPythonItem::LoadItemTable: invalid item_proto[%s] VERSION[%d]", c_szFileName, dwVersion);
			return false;
		}

		TraceError("item_proto size: %u", dwStride);

		if (dwStride != sizeof(SItemTable))
		{
			TraceError("CPythonItem::LoadItemTable: invalid item_proto[%s] STRIDE[%d] != sizeof(SItemTable)", 
				c_szFileName, dwStride, sizeof(SItemTable));
			return false;
		}
	}
	else if (dwFourCC != MAKEFOURCC('M', 'I', 'P', 'T'))
	{
		TraceError("CPythonItem::LoadItemTable: invalid item proto type %s", c_szFileName);
		return false;
	}

	file.Read(&dwElements, sizeof(uint32_t));
	file.Read(&dwDataSize, sizeof(uint32_t));

	uint8_t * pbData = new uint8_t[dwDataSize];
	file.Read(pbData, dwDataSize);

	/////

	CLZObject zObj;

	if (!CLZO::Instance().Decompress(zObj, pbData, s_adwItemProtoKey))
	{
		delete [] pbData;
		return false;
	}

	/////

	char szName[64+1];
	std::map<uint32_t,uint32_t> itemNameMap;

	for (uint32_t i = 0; i < dwElements; ++i)
	{
		SItemTable & t = *((SItemTable *) zObj.GetBuffer() + i);
		SItemTable * table = &t;

		CItemData * pItemData;
		uint32_t dwVnum = table->dwVnum;

		auto f = m_ItemMap.find(dwVnum); 
		if (m_ItemMap.end() == f)
		{
			_snprintf_s(szName, sizeof(szName), "icon/item/%05u.tga", dwVnum);

			if (CResourceManager::Instance().IsFileExist(szName) == false)
			{
				auto itVnum = itemNameMap.find(GetHashCode(table->szName));
				
				if (itVnum != itemNameMap.end())
					_snprintf_s(szName, sizeof(szName), "icon/item/%05u.tga", itVnum->second);
				else
					_snprintf_s(szName, sizeof(szName), "icon/item/%05u.tga", dwVnum-(dwVnum % 10));

				if (CResourceManager::Instance().IsFileExist(szName) == false)
				{
#ifdef _DEBUG
					TraceError("%16s(#%-5d) cannot find icon file. setting to default.", table->szLocaleName, dwVnum);
#endif
					const uint32_t EmptyBowl = 27995;
					_snprintf_s(szName, sizeof(szName), "icon/item/%05d.tga", EmptyBowl);
				}
			}
				
			pItemData = CItemData::New();

			pItemData->SetDefaultItemData(szName);
			m_ItemMap.emplace(dwVnum, pItemData);
		}
		else
		{
			pItemData = f->second;
		}	
		if (itemNameMap.find(GetHashCode(table->szName)) == itemNameMap.end())
			itemNameMap.emplace(GetHashCode(table->szName), table->dwVnum);
		pItemData->SetItemTableData(table);
		if (0 != table->dwVnumRange)
			m_vec_ItemRange.emplace_back(pItemData);
	}

//!@#
//	SItemTable * table = (SItemTable *) zObj.GetBuffer();
//	for (uint32_t i = 0; i < dwElements; ++i, ++table)
//	{
//		CItemData * pItemData;
//		uint32_t dwVnum = table->dwVnum;
//
//		TItemMap::iterator f = m_ItemMap.find(dwVnum);
//
//		if (m_ItemMap.end() == f)
//		{
//			pItemData = CItemData::New();
//
//			pItemData->LoadItemData(_getf("d:/ymir work/item/%05d.msm", dwVnum));
//			m_ItemMap.insert(TItemMap::value_type(dwVnum, pItemData));
//		}
//		else
//		{
//			pItemData = f->second;
//		}
//		pItemData->SetItemTableData(table);
//	}

	delete [] pbData;
	return true;
}

void CItemManager::Destroy()
{
	for (auto & i : m_ItemMap)
		CItemData::Delete(i.second);

	m_ItemMap.clear();
}

#ifdef ENABLE_ACCE_SYSTEM
bool CItemManager::LoadItemScale(const char * c_szFileName)
{
	CFile kFile;
	if (!FileSystemManager::Instance().OpenFile(c_szFileName, kFile))
		return false;

	CMemoryTextFileLoader kTextFileLoader;
	kTextFileLoader.Bind(kFile.GetSize(), kFile.GetData());

	CTokenVector kTokenVector;
	for (uint32_t i = 0; i < kTextFileLoader.GetLineCount(); ++i)
	{
		if (!kTextFileLoader.SplitLineByTab(i, &kTokenVector))
			continue;

		if (kTokenVector.size() < ITEMSCALE_NUM)
		{
			TraceError("LoadItemScale: invalid line %d (%s).", i, c_szFileName);
			continue;
		}

		const std::string & strJob = kTokenVector[ITEMSCALE_JOB];
		const std::string & strSex = kTokenVector[ITEMSCALE_SEX];
		const std::string & strScaleX = kTokenVector[ITEMSCALE_SCALE_X];
		const std::string & strScaleY = kTokenVector[ITEMSCALE_SCALE_Y];
		const std::string & strScaleZ = kTokenVector[ITEMSCALE_SCALE_Z];
		const std::string & strPositionX = kTokenVector[ITEMSCALE_POSITION_X];
		const std::string & strPositionY = kTokenVector[ITEMSCALE_POSITION_Y];
		const std::string & strPositionZ = kTokenVector[ITEMSCALE_POSITION_Z];

		for (int32_t j = 0; j < 5; ++j)
		{
			CItemData * pItemData = MakeItemData(atoi(kTokenVector[ITEMSCALE_VNUM].c_str()) + j);
			pItemData->SetItemScale(strJob, strSex, strScaleX, strScaleY, strScaleZ, strPositionX, strPositionY, strPositionZ);
		}
	}

	return true;
}
#endif

CItemManager::CItemManager() : m_pSelectedItemData(nullptr)
{
}
CItemManager::~CItemManager()
{
	Destroy();
}
