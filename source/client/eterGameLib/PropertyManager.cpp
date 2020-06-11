#include "StdAfx.h"
#include <FileSystemIncl.hpp>
#include "../eterBase/file_ptr.h"
#include "PropertyManager.h"
#include "Property.h"

CPropertyManager::CPropertyManager()
{
	m_bInitialized = false;
}

CPropertyManager::~CPropertyManager()
{
	Clear();
}

bool CPropertyManager::Initialize(const char * c_pszPackFileName)
{
	if (m_bInitialized)
		return true;

	CFile kPropertyFile;
	if (!FileSystemManager::Instance().OpenFile(c_pszPackFileName, kPropertyFile))
	{
		TraceError("Property list: %s can NOT load", c_pszPackFileName);
		return false;
	}

	auto stContent = std::string(reinterpret_cast<const char*>(kPropertyFile.GetData()), kPropertyFile.GetSize());
	if (stContent.empty())
	{
		TraceError("Property list: %s is null", c_pszPackFileName);
		return false;
	}

	TPropertyContainer vPropertyList;
	if (LoadPropertyList(stContent, vPropertyList) == false)
	{
		TraceError("Property list: %s can NOT parsed", c_pszPackFileName);
		return false;
	}
	if (vPropertyList.empty())
	{
		TraceError("Property list: %s is empty");
		return false;
	}
	// TraceError("Property container size: %u", vPropertyList.size());

	for (auto& element : vPropertyList)
	{
		CProperty* pProperty = new CProperty(element["filename"].c_str(), std::stoul(element["crc"]));

		uint32_t dwCRC = pProperty->GetCRC();
		auto stCRC = std::to_string(dwCRC);

		if (stCRC != element["crc"])
		{
			TraceError("CPropertyManager::Initialize: Cannot register '%s' Hash: %s/%s",
				element["filename"].c_str(), stCRC.c_str(), element["crc"].c_str());
			delete pProperty;
			continue;
		}

		auto itor = m_PropertyByCRCMap.find(dwCRC);
		if (itor != m_PropertyByCRCMap.end())
		{
			Tracef("CPropertyManager::Initialize: Property already registered, replace %s to %s\n",
				itor->second->GetFileName(), element["filename"].c_str());

			delete itor->second;
			itor->second = pProperty;
		}
		else
		{
			m_PropertyByCRCMap.emplace(dwCRC, pProperty);
			// TraceError("Property registired! File: %s CRC: %u", element["filename"].c_str(), element["crc"].c_str());
		}

		for (auto& elementdata : element)
		{
			CTokenVector kVec;
			kVec.push_back(elementdata.second.c_str());
			pProperty->PutVector(elementdata.first.c_str(), kVec);

			// TraceError("PutVector: '%s'-'%s'", elementdata.first.c_str(), elementdata.second.c_str());
		}
	}

	m_bInitialized = true;
	return true;
}

void CPropertyManager::ReserveCRC(uint32_t dwCRC)
{
	m_ReservedCRCSet.emplace(dwCRC);
}

uint32_t CPropertyManager::GetUniqueCRC(const char * c_szSeed)
{
	std::string stTmp = c_szSeed;

	while (true)
	{
		uint32_t dwCRC = GetCRC32(stTmp.c_str(), stTmp.length());

		if (m_ReservedCRCSet.find(dwCRC) == m_ReservedCRCSet.end() &&
			m_PropertyByCRCMap.find(dwCRC) == m_PropertyByCRCMap.end())
			return dwCRC;

		char szAdd[2];
		_snprintf_s(szAdd, sizeof(szAdd), "%u", random() % 10);
		stTmp += szAdd;
	}
}

bool CPropertyManager::Register(const char * c_pszFileName, CProperty ** ppProperty)
{
	CFile file;
	if (!FileSystemManager::Instance().OpenFile(c_pszFileName, file))
	{
		TraceError("Property: %s can NOT open!", c_pszFileName);
		return false;
	}

	auto * pProperty = new CProperty(c_pszFileName);
	if (!pProperty->ReadFromMemory(file.GetData(), file.GetSize(), c_pszFileName))
	{
		delete pProperty;
		TraceError("Property: %s can NOT read!", c_pszFileName);
		return false;
	}

	uint32_t dwCRC = pProperty->GetCRC();

	auto itor = m_PropertyByCRCMap.find(dwCRC);

	if (m_PropertyByCRCMap.end() != itor)
	{
		Tracef("Property(%u) already registered, replace %s to %s\n", dwCRC, itor->second->GetFileName(), c_pszFileName);

		delete itor->second;
		itor->second = pProperty;
	}
	else
		m_PropertyByCRCMap.emplace(dwCRC, pProperty);

	if (ppProperty)
		*ppProperty = pProperty;

	return true;
}

bool CPropertyManager::Get(const char * c_pszFileName, CProperty ** ppProperty)
{
	return Register(c_pszFileName, ppProperty);
}

bool CPropertyManager::Get(uint32_t dwCRC, CProperty ** ppProperty)
{
	auto itor = m_PropertyByCRCMap.find(dwCRC);
	if (m_PropertyByCRCMap.end() == itor)
		return false;

	*ppProperty = itor->second;
	return true;
}

bool CPropertyManager::Erase(uint32_t dwCRC)
{
	auto itor = m_PropertyByCRCMap.find(dwCRC);
	if (m_PropertyByCRCMap.end() == itor)
		return false;

	CProperty * pProperty = itor->second;
	m_PropertyByCRCMap.erase(itor);

	DeleteFileA(pProperty->GetFileName());
	ReserveCRC(pProperty->GetCRC());

	msl::file_ptr fPtr("property/reserve", "a+");
	if (!fPtr)
		LogBox("Cannot open the CRC file 'property/reserve'.");
	else
	{
		char szCRC[64 + 1];
		_snprintf_s(szCRC, sizeof(szCRC), "%u\r\n", pProperty->GetCRC());

		fputs(szCRC, fPtr.get());
	}

	delete pProperty;
	return true;
}

bool CPropertyManager::Erase(const char * c_pszFileName)
{
	CProperty * pProperty = nullptr;

	if (Get(c_pszFileName, &pProperty))
		return Erase(pProperty->GetCRC());
	
	return false;
}

void CPropertyManager::Clear()
{
	stl_wipe_second(m_PropertyByCRCMap);
}
