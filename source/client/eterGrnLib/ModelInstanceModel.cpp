#include "StdAfx.h"
#include "ModelInstance.h"
#include "Model.h"

void CGrannyModelInstance::Clear()
{
	m_kMtrlPal.Clear();
	
	DestroyDeviceObjects();
	// WORK
	__DestroyMeshBindingVector();
	// END_OF_WORK
	__DestroyMeshMatrices();
	__DestroyModelInstance();
	__DestroyWorldPose();

	__Initialize();
}

void CGrannyModelInstance::SetMainModelPointer(CGrannyModel* pModel, CGraphicVertexBuffer* pkSharedDeformableVertexBuffer)
{
	SetLinkedModelPointer(pModel, pkSharedDeformableVertexBuffer, nullptr);
}

void CGrannyModelInstance::SetLinkedModelPointer(CGrannyModel* pkModel, CGraphicVertexBuffer* pkSharedDeformableVertexBuffer, CGrannyModelInstance** ppkSkeletonInst)
{
	Clear();

	if (m_pModel)
		m_pModel->Release();

	m_pModel = pkModel;

	m_pModel->AddReference();
	
	if (pkSharedDeformableVertexBuffer)
		__SetSharedDeformableVertexBuffer(pkSharedDeformableVertexBuffer);
	else
		__CreateDynamicVertexBuffer();

	__CreateModelInstance();
	
	// WORK
	if (ppkSkeletonInst && *ppkSkeletonInst)
	{
		m_ppkSkeletonInst = ppkSkeletonInst;
		__CreateWorldPose(*ppkSkeletonInst);			
		__CreateMeshBindingVector(*ppkSkeletonInst);
	}
	else
	{
		__CreateWorldPose(nullptr);			
		__CreateMeshBindingVector(nullptr);
	}
	// END_OF_WORK	

	__CreateMeshMatrices();

	ResetLocalTime();
	
	m_kMtrlPal = pkModel->GetMaterialPalette();
}

// WORK
granny_world_pose* CGrannyModelInstance::__GetWorldPosePtr() const
{
	if (m_pgrnWorldPoseReal)
		return m_pgrnWorldPoseReal;
	
	if (m_ppkSkeletonInst && *m_ppkSkeletonInst)
		return (*m_ppkSkeletonInst)->m_pgrnWorldPoseReal;

	assert(m_ppkSkeletonInst!=nullptr && "__GetWorldPosePtr - NO HAVE SKELETON");		
	return nullptr;	
}

const granny_int32x* CGrannyModelInstance::__GetMeshBoneIndices(uint32_t iMeshBinding) const
{
	assert(iMeshBinding<m_vct_pgrnMeshBinding.size());
	return GrannyGetMeshBindingToBoneIndices(m_vct_pgrnMeshBinding[iMeshBinding]);
}

bool CGrannyModelInstance::__CreateMeshBindingVector(CGrannyModelInstance* pkDstModelInst)
{
	assert(m_vct_pgrnMeshBinding.empty());

	if (!m_pModel)
		return false;	
	
	granny_model* pgrnModel = m_pModel->GetGrannyModelPointer();
	if (!pgrnModel)
		return false;

	granny_skeleton* pgrnDstSkeleton = pgrnModel->Skeleton;
	if (pkDstModelInst && pkDstModelInst->m_pModel && pkDstModelInst->m_pModel->GetGrannyModelPointer())
		pgrnDstSkeleton = pkDstModelInst->m_pModel->GetGrannyModelPointer()->Skeleton;
	
	m_vct_pgrnMeshBinding.reserve(pgrnModel->MeshBindingCount);

	for (granny_int32 iMeshBinding = 0; iMeshBinding != pgrnModel->MeshBindingCount; ++iMeshBinding)
		m_vct_pgrnMeshBinding.emplace_back(
			GrannyNewMeshBinding(pgrnModel->MeshBindings[iMeshBinding].Mesh, pgrnModel->Skeleton, pgrnDstSkeleton));

	return true;
}

void CGrannyModelInstance::__DestroyMeshBindingVector()
{
	std::for_each(m_vct_pgrnMeshBinding.begin(), m_vct_pgrnMeshBinding.end(), GrannyFreeMeshBinding);
	m_vct_pgrnMeshBinding.clear();		
}

// END_OF_WORK


void CGrannyModelInstance::__CreateWorldPose(CGrannyModelInstance* pkSkeletonInst)
{
	assert(m_pgrnModelInstance != nullptr);
	assert(m_pgrnWorldPoseReal == nullptr);

	// WORK
	if (pkSkeletonInst)
		return;	
	// END_OF_WORK

	granny_skeleton * pgrnSkeleton = GrannyGetSourceSkeleton(m_pgrnModelInstance);		

	// WORK
	m_pgrnWorldPoseReal = GrannyNewWorldPose(pgrnSkeleton->BoneCount);	
	// END_OF_WORK
}

void CGrannyModelInstance::__DestroyWorldPose()
{
	if (!m_pgrnWorldPoseReal)
		return;

	GrannyFreeWorldPose(m_pgrnWorldPoseReal);
	m_pgrnWorldPoseReal = nullptr;	
}

void CGrannyModelInstance::__CreateModelInstance()
{	
	assert(m_pModel != nullptr);
	assert(m_pgrnModelInstance == nullptr);

	const granny_model * pgrnModel = m_pModel->GetGrannyModelPointer();	
	m_pgrnModelInstance = GrannyInstantiateModel(pgrnModel);
}

void CGrannyModelInstance::__DestroyModelInstance()
{
	if (!m_pgrnModelInstance) 
		return;

	GrannyFreeModelInstance(m_pgrnModelInstance);
	m_pgrnModelInstance = nullptr;
}

void CGrannyModelInstance::__CreateMeshMatrices()
{
	assert(m_pModel != nullptr);
	
	if (m_pModel->GetMeshCount() <= 0) // �޽��� ���� (ī�޶� ����) �𵨵� ��Ȥ �ִ�..
		return;
	
	int32_t meshCount = m_pModel->GetMeshCount();	
	m_meshMatrices = new D3DXMATRIX[meshCount];
}

void CGrannyModelInstance::__DestroyMeshMatrices()
{
	if (!m_meshMatrices)
		return;

	delete [] m_meshMatrices;
	m_meshMatrices = nullptr;
}

uint32_t CGrannyModelInstance::GetDeformableVertexCount()
{
	if (!m_pModel)
		return 0;

	return m_pModel->GetDeformVertexCount();
}

uint32_t CGrannyModelInstance::GetVertexCount()
{
	if (!m_pModel)
		return 0;

	return m_pModel->GetVertexCount();
}

// WORK

void CGrannyModelInstance::__SetSharedDeformableVertexBuffer(CGraphicVertexBuffer* pkSharedDeformableVertexBuffer)
{
	m_pkSharedDeformableVertexBuffer = pkSharedDeformableVertexBuffer;
}

bool CGrannyModelInstance::__IsDeformableVertexBuffer()
{
	if (m_pkSharedDeformableVertexBuffer)
		return true;

	return m_kLocalDeformableVertexBuffer.IsEmpty();
}

IDirect3DVertexBuffer9* CGrannyModelInstance::__GetDeformableD3DVertexBufferPtr()
{
	return __GetDeformableVertexBufferRef().GetD3DVertexBuffer();
}

CGraphicVertexBuffer& CGrannyModelInstance::__GetDeformableVertexBufferRef()
{
	if (m_pkSharedDeformableVertexBuffer)
		return *m_pkSharedDeformableVertexBuffer;

	return m_kLocalDeformableVertexBuffer;
}

void CGrannyModelInstance::__CreateDynamicVertexBuffer()
{
	assert(m_pModel != nullptr);
	assert(m_kLocalDeformableVertexBuffer.IsEmpty());

	int32_t vtxCount = m_pModel->GetDeformVertexCount();

	if (0 != vtxCount)
	{
		if (!m_kLocalDeformableVertexBuffer.Create(vtxCount,
									   D3DFVF_XYZ|D3DFVF_NORMAL|D3DFVF_TEX1,
									   //D3DUSAGE_DYNAMIC, D3DPOOL_SYSTEMMEM
									   
									   D3DUSAGE_WRITEONLY, D3DPOOL_MANAGED 
		))
			return;
	}	
}

void CGrannyModelInstance::__DestroyDynamicVertexBuffer()
{
	m_kLocalDeformableVertexBuffer.Destroy();
	m_pkSharedDeformableVertexBuffer = nullptr;
}

// END_OF_WORK

bool CGrannyModelInstance::GetBoneIndexByName(const char * c_szBoneName, int32_t * pBoneIndex) const
{
	assert(m_pgrnModelInstance != nullptr);

	granny_skeleton * pgrnSkeleton = GrannyGetSourceSkeleton(m_pgrnModelInstance);

	if (!GrannyFindBoneByName(pgrnSkeleton, c_szBoneName, pBoneIndex))
		return false;

	return true;
}

const float * CGrannyModelInstance::GetBoneMatrixPointer(int32_t iBone) const
{
	const float* bones = GrannyGetWorldPose4x4(__GetWorldPosePtr(), iBone);
	if (!bones)
	{
		//granny_model* pModel = m_pModel->GetGrannyModelPointer();		
		//TraceError("GrannyModelInstance(%s).GetBoneMatrixPointer(boneIndex(%d)).NOT_FOUND_BONE", pModel->Name, iBone);
		return nullptr;
	}
	return bones;
}

const float * CGrannyModelInstance::GetCompositeBoneMatrixPointer(int32_t iBone) const
{
	// NOTE : GrannyGetWorldPose4x4�� ������ ������ �߸����� �� ����.. �׷��ϰ� �ӵ��� ����
	//        GrannyGetWorldPose4x4�� ��� matrix ���Ҹ� �� ������ ���� ����
	return GrannyGetWorldPoseComposite4x4(__GetWorldPosePtr(), iBone);
}

void CGrannyModelInstance::ReloadTexture()
{
	assert("Not currently used - CGrannyModelInstance::ReloadTexture()");
/*
	assert(m_pModel != nullptr);
	const CGrannyMaterialPalette & c_rGrannyMaterialPalette = m_pModel->GetMaterialPalette();
	uint32_t dwMaterialCount = c_rGrannyMaterialPalette.GetMaterialCount();
	for (uint32_t dwMtrIndex = 0; dwMtrIndex < dwMaterialCount; ++dwMtrIndex)
	{
		const CGrannyMaterial & c_rGrannyMaterial = c_rGrannyMaterialPalette.GetMaterialRef(dwMtrIndex);
		CGraphicImage * pImageStage0 = c_rGrannyMaterial.GetImagePointer(0);
		if (pImageStage0)
			pImageStage0->Reload();
		CGraphicImage * pImageStage1 = c_rGrannyMaterial.GetImagePointer(1);
		if (pImageStage1)
			pImageStage1->Reload();
	}
*/
}