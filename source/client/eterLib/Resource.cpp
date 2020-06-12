#include "StdAfx.h"
#include <FileSystemIncl.hpp>
#include "../eterBase/CRC32.h"
#include "../eterBase/Timer.h"
#include "../eterBase/Stl.h"
#include "Resource.h"
#include "ResourceManager.h"

bool CResource::ms_bDeleteImmediately = false;

CResource::CResource(const char* c_szFileName) : me_state(STATE_EMPTY)
{
	SetFileName(c_szFileName);
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

	const char * c_szFileName = GetFileName();
	
	if (c_szFileName && c_szFileName[0] == '\0') {
		me_state = STATE_ERROR;
		return;
	}

	uint32_t		dwStart = ELTimer_GetMSec();
	CFile	file;

	//Tracenf("Load %s", c_szFileName);

	if (FileSystemManager::Instance().OpenFile(c_szFileName, file))
	{
		m_dwLoadCostMiliiSecond = ELTimer_GetMSec() - dwStart;
		//Tracef("CResource::Load %s (%d bytes) in %d ms\n", c_szFileName, file.GetSize(), m_dwLoadCostMiliiSecond);

		if (OnLoad(file.GetSize(), file.GetData()))
		{
			me_state = STATE_EXIST;
		}
		else
		{
			Tracef("CResource::Load Error %s\n", c_szFileName);
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
			Tracef("CResource::Load file not exist %s\n", c_szFileName);
			me_state = STATE_ERROR;
		}
	}
}

void CResource::Reload()
{
	Clear();
	Tracef("CResource::Reload %s\n", GetFileName());

	CFile	file;
	if (FileSystemManager::Instance().OpenFile(GetFileName(), file))
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

int32_t CResource::ConvertPathName(const char * c_szPathName, char * pszRetPathName, int32_t retLen)
{
	const char * pc;
	int32_t len = 0;

	for (pc = c_szPathName; *pc && len < retLen; ++pc, ++len)
	{
		if (*pc == '/')
			*(pszRetPathName++) = '\\';
		else
			*(pszRetPathName++) = (char) ascii_tolower(*pc);
	}

	*pszRetPathName = '\0';
	return len;
}

void CResource::SetFileName(const char* c_szFileName)
{
	// 2004. 2. 1. myevan. �����尡 ���Ǵ� ��Ȳ���� static ������ ������� �ʴ°��� ����.
	// 2004. 2. 1. myevan. ���� �̸� ó���� std::string ���
	m_stFileName=c_szFileName;
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
