#include "StdAfx.h"
#include "../eterbase/Debug.h"
#include "Thing.h"
#include "ThingInstance.h"
#include <FileSystemIncl.hpp>

namespace
{

struct my_file_info
{
	granny_uint32 Sentinel;
	granny_file_info* FileInfo;
};

// Explicitly declare them as uint32s here to avoid compiler warnings
// due to |GrannyFirstGRNStandardTag| being converted to signed int32_t.
const granny_uint32 kGrnCurrent = GrannyCurrentGRNStandardTag;
const granny_uint32 kGrnStart = GrannyFirstGRNStandardTag;

// (current - first) gives us the current "revision", which is necessary,
// since our tag needs to change when the underlying GrannyFileInfoType
// changes.
const granny_uint32 kCurrentTag = kGrnCurrent - kGrnStart + 0x70000000;

const granny_uint32 kSentinel = 0xdeadbeef;

granny_data_type_definition MyFileInfoType[] =
{
	{ GrannyUInt32Member, "Sentinel" },
	{ GrannyReferenceMember, "FileInfo", GrannyFileInfoType },
	{ GrannyEndMember }
};

my_file_info* GetMyFileInfo(granny_file* File)
{
	granny_variant Root;
	GrannyGetDataTreeFromFile(File, &Root);

	if (File->Header->TypeTag == kCurrentTag) {
		return (my_file_info*)Root.Object;
	} else {
		if (File->ConversionBuffer == nullptr) {
			// Log Warning about conversion operation
			File->ConversionBuffer =
				GrannyConvertTree(Root.Type, Root.Object, MyFileInfoType, nullptr, nullptr);
		}

		return (my_file_info*)File->ConversionBuffer;
	}
}

}

CGraphicThing::CGraphicThing(const FileSystem::CFileName& filename) : CResource(filename)
{
	Initialize();	
}

CGraphicThing::~CGraphicThing()
{
	//OnClear();
	Clear();
}

void CGraphicThing::Initialize()
{
	m_pgrnFile = nullptr;
	m_pgrnFileInfo = nullptr;
	m_pgrnAni = nullptr;

	m_models = nullptr;
	m_motions = nullptr;
}

void CGraphicThing::OnClear()
{
	delete [] m_motions;
	delete [] m_models;

	if (m_pgrnFile)
		GrannyFreeFile(m_pgrnFile);

	Initialize();
}

CGraphicThing::TType CGraphicThing::Type()
{
	static TType s_type = StringToType("CGraphicThing");
	return s_type;
}

bool CGraphicThing::OnIsEmpty() const
{
	return m_pgrnFile ? false : true;
}

bool CGraphicThing::OnIsType(TType type)
{
	if (Type() == type)
		return true;

	return CResource::OnIsType(type);
}

bool CGraphicThing::CreateDeviceObjects()
{
	if (!m_pgrnFileInfo)
		return true;
	
	for (int32_t m = 0; m < m_pgrnFileInfo->ModelCount; ++m)
	{
		CGrannyModel & rModel = m_models[m];
		rModel.CreateDeviceObjects();
	}

	return true;
}

void CGraphicThing::DestroyDeviceObjects()
{
	if (!m_pgrnFileInfo)
		return;

	for (int32_t m = 0; m < m_pgrnFileInfo->ModelCount; ++m)
	{
		CGrannyModel & rModel = m_models[m];
		rModel.DestroyDeviceObjects();
	}
}

bool CGraphicThing::CheckModelIndex(int32_t iModel) const
{
	if (!m_pgrnFileInfo)
	{
		const auto& stRefResourceName = GetFileNameString();
		Tracef("m_pgrnFileInfo == nullptr: %s\n", stRefResourceName.c_str());
		return false;
	}

	assert(m_pgrnFileInfo != nullptr);

	if (iModel < 0)
		return false;

	if (iModel >= m_pgrnFileInfo->ModelCount)
		return false;

	return true;
}

bool CGraphicThing::CheckMotionIndex(int32_t iMotion) const
{
	// Temporary
	if (!m_pgrnFileInfo)
		return false;
	// Temporary

	assert(m_pgrnFileInfo != nullptr);

	if (iMotion < 0)
		return false;
	
	if (iMotion >= m_pgrnFileInfo->AnimationCount)
		return false;

	return true;
}

CGrannyModel * CGraphicThing::GetModelPointer(int32_t iModel)
{	
	assert(CheckModelIndex(iModel));
	assert(m_models != nullptr);
	return m_models + iModel;
}

CGrannyMotion * CGraphicThing::GetMotionPointer(int32_t iMotion)
{
	assert(CheckMotionIndex(iMotion));

	if (iMotion >= m_pgrnFileInfo->AnimationCount)
		return nullptr;

	assert(m_motions != nullptr);
	return (m_motions + iMotion);
}

int32_t CGraphicThing::GetTextureCount() const
{
	if (!m_pgrnFileInfo)
		return 0;

	if (m_pgrnFileInfo->TextureCount <= 0)
		return 0;

	return (m_pgrnFileInfo->TextureCount);
}

const char * CGraphicThing::GetTexturePath(int32_t iTexture)
{
	if(iTexture >= GetTextureCount())
		return "";

	return m_pgrnFileInfo->Textures[iTexture]->FromFileName;
}

int32_t CGraphicThing::GetModelCount() const
{
	if (!m_pgrnFileInfo)
		return 0;

	return (m_pgrnFileInfo->ModelCount);
}

int32_t CGraphicThing::GetMotionCount() const
{
	if (!m_pgrnFileInfo)
		return 0;

	return (m_pgrnFileInfo->AnimationCount);
}

bool CGraphicThing::OnLoad(int32_t iSize, const void * c_pvBuf)
{
	if (!c_pvBuf)
		return false;

#ifdef ENABLE_LAYER2_FILE_ENCRYPTION
	auto decryptedBufer = FileSystemManager::Instance().DecryptLayer2Protection(reinterpret_cast<const uint8_t*>(c_pvBuf), iSize);
	if (decryptedBufer.empty())
	{
		DEBUG_LOG(LL_ERR, "GR2 Layer2 decryption fail!");
		return false;
	}

	m_pgrnFile = GrannyReadEntireFileFromMemory(decryptedBufer.size(), reinterpret_cast<void *>(decryptedBufer.data()));
#else
	m_pgrnFile = GrannyReadEntireFileFromMemory(iSize, const_cast<void *>(c_pvBuf));
#endif

	if (!m_pgrnFile)
	{
		DEBUG_LOG(LL_ERR, "Granny read mem fail!");

#ifdef ENABLE_LAYER2_FILE_ENCRYPTION
		decryptedBufer.clear();
#endif
		return false;
	}

#ifdef ENABLE_LAYER2_FILE_ENCRYPTION
	decryptedBufer.clear();
#endif

	auto info = GetMyFileInfo(m_pgrnFile);
	if (!info)
		return false;

	// Converting a standard .gr2 file to our wrapper type should
	// result in a zero sentinel, which is invalid.
	if (!info->FileInfo || info->Sentinel != kSentinel) {
#if !defined(_DEBUG) && !defined(WORLD_EDITOR)
		// We do not support standard gr2 files in client release builds
		return false;
#else
		// If the gr2 file's format is old, we need to convert again. Since
		// we can hold only one conversion buffer
		// (and GrannyGetFileInfo just gives us the already existing buffer back if one already exists),
		// we need to free our current buffer here.
		if (m_pgrnFile->ConversionBuffer) {
			GrannyFreeBuilderResult(m_pgrnFile->ConversionBuffer);
			m_pgrnFile->ConversionBuffer = nullptr;
		}

		m_pgrnFileInfo = GrannyGetFileInfo(m_pgrnFile);
		if (!m_pgrnFileInfo)
			return false;
#endif
	}
	else {
		m_pgrnFileInfo = info->FileInfo;
	}

	LoadModels();
	LoadMotions();
	return true;
}

// SUPPORT_LOCAL_TEXTURE
static std::string gs_modelLocalPath;

const std::string& GetModelLocalPath()
{
	return gs_modelLocalPath;
}
// END_OF_SUPPORT_LOCAL_TEXTURE

bool CGraphicThing::LoadModels()
{
	assert(m_pgrnFile != nullptr);
	assert(m_models == nullptr);
	
	if (m_pgrnFileInfo->ModelCount <= 0)
		return false;	

	// SUPPORT_LOCAL_TEXTURE
	const std::string& fileName = GetFileNameString();

	//char localPath[256] = "";
	if (fileName.length() > 2 && fileName[1] != ':')
	{				
		int32_t sepPos = fileName.rfind('\\');
		gs_modelLocalPath.assign(fileName, 0, sepPos+1);
	}
	// END_OF_SUPPORT_LOCAL_TEXTURE

	int32_t modelCount = m_pgrnFileInfo->ModelCount;

	m_models = new CGrannyModel[modelCount];

	for (int32_t m = 0; m < modelCount; ++m)
	{
		CGrannyModel & rModel = m_models[m];
		granny_model * pgrnModel = m_pgrnFileInfo->Models[m];

		if (!rModel.CreateFromGrannyModelPointer(pgrnModel))
			return false;
	}

	GrannyFreeFileSection(m_pgrnFile, GrannyStandardRigidVertexSection);
	GrannyFreeFileSection(m_pgrnFile, GrannyStandardRigidIndexSection);
	GrannyFreeFileSection(m_pgrnFile, GrannyStandardDeformableIndexSection);
	GrannyFreeFileSection(m_pgrnFile, GrannyStandardTextureSection);
	return true;
}

bool CGraphicThing::LoadMotions()
{
	assert(m_pgrnFile != nullptr);
	assert(m_motions == nullptr);

	if (m_pgrnFileInfo->AnimationCount <= 0)
		return false;
	
	int32_t motionCount = m_pgrnFileInfo->AnimationCount;

	m_motions = new CGrannyMotion[motionCount];
	
	for (int32_t m = 0; m < motionCount; ++m)
		if (!m_motions[m].BindGrannyAnimation(m_pgrnFileInfo->Animations[m]))
			return false;

	return true;
}
