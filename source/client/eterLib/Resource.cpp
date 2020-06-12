#include "StdAfx.h"
#include <FileSystemIncl.hpp>
#include "../eterBase/CRC32.h"
#include "../eterBase/Timer.h"
#include "../eterBase/Stl.h"
#include "Resource.h"
#include "ResourceManager.h"

bool CResource::ms_bDeleteImmediately = false;

CResource::CResource(const FileSystem::CFileName& filename)
	: me_state(STATE_EMPTY)
	, m_stFileName(filename)
{
	// ctor
}

CResource::~CResource()
{
}

void CResource::SetDeleteImmediately(bool isSet)
{
	ms_bDeleteImmediately = isSet;
}

void CResource::OnConstruct()
{
	Load();
}

void CResource::OnSelfDestruct()
{	
	if (ms_bDeleteImmediately)
		Clear();
	else
		CResourceManager::Instance().ReserveDeletingResource(this);
}

void CResource::Load()
{
	if (me_state != STATE_EMPTY)
		return;

	CFile file;
	if (FileSystemManager::Instance().OpenFile(m_stFileName, file))
	{
		//m_dwLoadCostMiliiSecond = ELTimer_GetMSec() - dwStart;
		//Tracef("CResource::Load %s (%d bytes) in %d ms\n", c_szFileName, file.Size(), m_dwLoadCostMiliiSecond);
		if (OnLoad(file.GetSize(), file.GetData()))
		{
			me_state = STATE_EXIST;
		}
		else
		{
			const auto& stRefResourceName = GetFileNameString();
			Tracef("CResource::Load Error %s\n", stRefResourceName.c_str());
			me_state = STATE_ERROR;
		}
	}
	else
	{
		if (OnLoad(0, nullptr))
			me_state = STATE_EXIST;
		else
		{
			const auto& stRefResourceName = GetFileNameString();
			Tracef("CResource::Load file not exist %s\n", stRefResourceName.c_str());
			me_state = STATE_ERROR;
		}
	}
}

void CResource::Reload()
{
	Clear();
	const auto& stRefResourceName = GetFileNameString();
	Tracef("CResource::Reload %s\n", stRefResourceName.c_str());

	CFile file;
	if (FileSystemManager::Instance().OpenFile(m_stFileName, file))
	{
		if (OnLoad(file.GetSize(), file.GetData()))
		{
			me_state = STATE_EXIST;
		}
		else
		{
			me_state = STATE_ERROR;
			return;
		}
	}
	else
	{
		if (OnLoad(0, nullptr))
			me_state = STATE_EXIST;
		else
		{
			me_state = STATE_ERROR;
		}
	}
}

CResource::TType CResource::StringToType(const char* c_szType)
{
	return GetCRC32(c_szType, strlen(c_szType));
}

void CResource::SetFileName(const FileSystem::CFileName& filename)
{
	// 2004. 2. 1. myevan. 쓰레드가 사용되는 상황에서 static 변수는 사용하지 않는것이 좋다.
	// 2004. 2. 1. myevan. 파일 이름 처리를 std::string 사용
	m_stFileName = filename;
}

void CResource::Clear()
{
	OnClear();
	me_state = STATE_EMPTY;
}

bool CResource::IsType(TType type)
{
	return OnIsType(type);
}

CResource::TType CResource::Type()
{
	static TType s_type = StringToType("CResource");
	return s_type;
}

bool CResource::OnIsType(TType type)
{
	if (CResource::Type() == type)
		return true;
	
	return false;
}

bool CResource::IsData() const
{
	return me_state != STATE_EMPTY;
}

bool CResource::IsEmpty() const
{
	return OnIsEmpty();
}

bool CResource::CreateDeviceObjects()
{
	return true;
}

void CResource::DestroyDeviceObjects()
{
}
