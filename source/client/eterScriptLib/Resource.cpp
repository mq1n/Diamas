#include "StdAfx.h"
#include "../eterLib/GrpExpandedImageInstance.h"
#include "../eterLib/GrpTextInstance.h"
#include "../eterLib/GrpMarkInstance.h"
#include "../eterLib/GrpSubImage.h"
#include "../eterLib/GrpText.h"
#include "../eterLib/AttributeData.h"
#include "../eterGrnLib/Thing.h"
#include "../eterGrnLib/ThingInstance.h"
#include "../eterEffectLib/EffectMesh.h"
#include "../eterEffectLib/EffectInstance.h"
#include "../eterGameLib/WeaponTrace.h"
#include "../eterGameLib/MapType.h"
#include "../eterGameLib/GameType.h"
#include "../eterGameLib/RaceData.h"
#include "../eterGameLib/RaceMotionData.h"
#include "../eterGameLib/ActorInstance.h"
#include "../eterGameLib/Area.h"
#include "../eterGameLib/ItemData.h"
#include "../eterGameLib/FlyingData.h"
#include "../eterGameLib/FlyTrace.h"
#include "../eterGameLib/FlyingInstance.h"
#include "../eterGameLib/FlyingData.h"

#include "Resource.h"

CResource * NewImage(const FileSystem::CFileName& c_szFileName)
{
	return new CGraphicImage(c_szFileName);
}

CResource * NewSubImage(const FileSystem::CFileName& c_szFileName)
{
	return new CGraphicSubImage(c_szFileName);
}

CResource * NewText(const FileSystem::CFileName& c_szFileName)
{
	return new CGraphicText(c_szFileName);
}

CResource * NewThing(const FileSystem::CFileName& c_szFileName)
{
	return new CGraphicThing(c_szFileName);
}

CResource * NewEffectMesh(const FileSystem::CFileName& c_szFileName)
{
	return new CEffectMesh(c_szFileName);
}

CResource * NewAttributeData(const FileSystem::CFileName& c_szFileName)
{
	return new CAttributeData(c_szFileName);
}

void CPythonResource::DumpFileList(const char * c_szFileName)
{
	m_resManager.DumpFileListToTextFile(c_szFileName);
}

void CPythonResource::Destroy()
{		
	CFlyingInstance::DestroySystem();
	CActorInstance::DestroySystem();
	CArea::DestroySystem();
	CGraphicExpandedImageInstance::DestroySystem();
	CGraphicImageInstance::DestroySystem();	
	CGraphicMarkInstance::DestroySystem();
	CGraphicThingInstance::DestroySystem();
	CGrannyModelInstance::DestroySystem();
	CGraphicTextInstance::DestroySystem();
	CEffectInstance::DestroySystem();
	CWeaponTrace::DestroySystem();	
	CFlyTrace::DestroySystem();
	
	m_resManager.DestroyDeletingList();

	CFlyingData::DestroySystem();
	CItemData::DestroySystem();
	CEffectData::DestroySystem();
	CEffectMesh::SEffectMeshData::DestroySystem();
	CRaceData::DestroySystem();
	NRaceData::DestroySystem();
	CRaceMotionData::DestroySystem();	

	m_resManager.Destroy();	
}

CPythonResource::CPythonResource()
{
	m_resManager.RegisterResourceNewFunctionPointer("sub", NewSubImage);
	m_resManager.RegisterResourceNewFunctionPointer("dds", NewImage);
	m_resManager.RegisterResourceNewFunctionPointer("jpg", NewImage);
	m_resManager.RegisterResourceNewFunctionPointer("tga", NewImage);
	m_resManager.RegisterResourceNewFunctionPointer("png", NewImage);
	m_resManager.RegisterResourceNewFunctionPointer("bmp", NewImage);
	m_resManager.RegisterResourceNewFunctionPointer("fnt", NewText);
	m_resManager.RegisterResourceNewFunctionPointer("gr2", NewThing);
	m_resManager.RegisterResourceNewFunctionPointer("mde", NewEffectMesh);
	m_resManager.RegisterResourceNewFunctionPointer("mdatr", NewAttributeData);
}

CPythonResource::~CPythonResource() = default;
