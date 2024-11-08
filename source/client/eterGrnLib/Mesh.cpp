#include "StdAfx.h"
#include "Mesh.h"
#include "Model.h"
#include "Material.h"

granny_data_type_definition GrannyPNT3322VertexType[5] =
{
	{GrannyReal32Member, GrannyVertexPositionName, 0, 3},
	{GrannyReal32Member, GrannyVertexNormalName, 0, 3},
	{GrannyReal32Member, GrannyVertexTextureCoordinatesName"0", 0, 2},
	{GrannyReal32Member, GrannyVertexTextureCoordinatesName"1", 0, 2},
	{GrannyEndMember}
};

void CGrannyMesh::LoadIndices(void * dstBaseIndices)
{
	const granny_mesh * pgrnMesh = GetGrannyMeshPointer();

	TIndex * dstIndices = ((TIndex *)dstBaseIndices) + m_idxBasePos;
	GrannyCopyMeshIndices(pgrnMesh, sizeof(TIndex), dstIndices);
}

void CGrannyMesh::LoadPNTVertices(void * dstBaseVertices)
{
	const granny_mesh * pgrnMesh = GetGrannyMeshPointer();

	if (!GrannyMeshIsRigid(pgrnMesh))
		return;

	TPNTVertex * dstVertices = ((TPNTVertex *)dstBaseVertices) + m_vtxBasePos;
	GrannyCopyMeshVertices(pgrnMesh, m_pgrnMeshType, dstVertices);
}

void CGrannyMesh::DeformPNTVertices(void * dstBaseVertices, D3DXMATRIX * boneMatrices, granny_mesh_binding* pgrnMeshBinding) const
{
	assert(dstBaseVertices != nullptr);
	assert(boneMatrices != nullptr);
	assert(m_pgrnMeshDeformer != nullptr);

	const granny_mesh * pgrnMesh = GetGrannyMeshPointer();

	TPNTVertex * srcVertices = (TPNTVertex *) GrannyGetMeshVertices(pgrnMesh);
	TPNTVertex * dstVertices = ((TPNTVertex *) dstBaseVertices) + m_vtxBasePos;
	
	int32_t vtxCount = GrannyGetMeshVertexCount(pgrnMesh);

	GrannyDeformVertices(
		m_pgrnMeshDeformer, 
		GrannyGetMeshBindingToBoneIndices(pgrnMeshBinding),
		(float *)boneMatrices,
		vtxCount,
		srcVertices,
		dstVertices);
}

bool CGrannyMesh::CanDeformPNTVertices() const
{
	return m_canDeformPNTVertex;
}

const granny_mesh * CGrannyMesh::GetGrannyMeshPointer() const
{
	return m_pgrnMesh;
}

const CGrannyMesh::TTriGroupNode * CGrannyMesh::GetTriGroupNodeList(CGrannyMaterial::EType eMtrlType) const
{
	return m_triGroupNodeLists[eMtrlType];
}

int32_t CGrannyMesh::GetVertexCount() const
{
	assert(m_pgrnMesh!=nullptr);
	return GrannyGetMeshVertexCount(m_pgrnMesh);
}

int32_t CGrannyMesh::GetVertexBasePosition() const
{
	return m_vtxBasePos;
}

int32_t CGrannyMesh::GetIndexBasePosition() const
{
	return m_idxBasePos;
}

// WORK
const granny_int32x * CGrannyMesh::GetDefaultBoneIndices() const
{
	return GrannyGetMeshBindingToBoneIndices(m_pgrnMeshBindingTemp);
}
// END_OF_WORK

bool CGrannyMesh::IsEmpty() const
{
	if (m_pgrnMesh)
		return false;

	return true;
}

bool CGrannyMesh::CreateFromGrannyMeshPointer(granny_skeleton * pgrnSkeleton, granny_mesh * pgrnMesh, int32_t vtxBasePos, int32_t idxBasePos, CGrannyMaterialPalette& rkMtrlPal)
{
	assert(IsEmpty());

	m_pgrnMesh = pgrnMesh;
	m_vtxBasePos = vtxBasePos;
	m_idxBasePos = idxBasePos;

	if (m_pgrnMesh->BoneBindingCount < 0)
		return true;

	// WORK
	m_pgrnMeshBindingTemp = GrannyNewMeshBinding(m_pgrnMesh, pgrnSkeleton, pgrnSkeleton);	
	// END_OF_WORK

	if (!GrannyMeshIsRigid(m_pgrnMesh))
	{
		m_canDeformPNTVertex = true;

		granny_data_type_definition * pgrnInputType = GrannyGetMeshVertexType(m_pgrnMesh);
		granny_data_type_definition * pgrnOutputType = m_pgrnMeshType;

		m_pgrnMeshDeformer = GrannyNewMeshDeformer(pgrnInputType, pgrnOutputType, GrannyDeformPositionNormal, GrannyAllowUncopiedTail);
		assert(m_pgrnMeshDeformer != nullptr && "Cannot create mesh deformer");
	}

	// Two Side Mesh
	if (!strncmp(m_pgrnMesh->Name, "2x", 2))
		m_isTwoSide = true;

	if (!LoadMaterials(rkMtrlPal))
		return false;

	if (!LoadTriGroupNodeList(rkMtrlPal))
		return false;

	return true;
}

bool CGrannyMesh::LoadTriGroupNodeList(CGrannyMaterialPalette& rkMtrlPal)
{
	assert(m_pgrnMesh != nullptr);
	assert(m_triGroupNodes == nullptr);

	int32_t mtrlCount		= m_pgrnMesh->MaterialBindingCount;
	if (mtrlCount <= 0) // õ�� ���� 2�� ũ���� �߻�
		return true;

	int32_t GroupNodeCount	= GrannyGetMeshTriangleGroupCount(m_pgrnMesh);
	if (GroupNodeCount <= 0)
		return true;

	m_triGroupNodes		= new TTriGroupNode[GroupNodeCount];

	const granny_tri_material_group * c_pgrnTriGroups = GrannyGetMeshTriangleGroups(m_pgrnMesh);

	for (int32_t g = 0; g < GroupNodeCount; ++g)
	{
		const granny_tri_material_group & c_rgrnTriGroup = c_pgrnTriGroups[g];
		TTriGroupNode * pTriGroupNode = m_triGroupNodes + g;

		pTriGroupNode->idxPos = m_idxBasePos + c_rgrnTriGroup.TriFirst * 3;
		pTriGroupNode->triCount = c_rgrnTriGroup.TriCount;
		
		int32_t iMtrl = c_rgrnTriGroup.MaterialIndex;		
		if (iMtrl < 0 || iMtrl >= mtrlCount)
		{
			pTriGroupNode->mtrlIndex=0;//m_mtrlIndexVector[iMtrl];			
		}
		else
		{	
			pTriGroupNode->mtrlIndex=m_mtrlIndexVector[iMtrl];
		}

		const CGrannyMaterial& rkMtrl=rkMtrlPal.GetMaterialRef(pTriGroupNode->mtrlIndex);
		pTriGroupNode->pNextTriGroupNode		= m_triGroupNodeLists[rkMtrl.GetType()];
		m_triGroupNodeLists[rkMtrl.GetType()]	= pTriGroupNode;

	}

	return true;
}

void CGrannyMesh::RebuildTriGroupNodeList()
{
	assert(!"CGrannyMesh::RebuildTriGroupNodeList() - Why should you rebuild it -?");
	/*
	int32_t mtrlCount = m_pgrnMesh->MaterialBindingCount;
	int32_t GroupNodeCount = GrannyGetMeshTriangleGroupCount(m_pgrnMesh);

	if (GroupNodeCount <= 0)
		return;
	
	const granny_tri_material_group * c_pgrnTriGroups = GrannyGetMeshTriangleGroups(m_pgrnMesh);
	
	for (int32_t g = 0; g < GroupNodeCount; ++g)
	{
		const granny_tri_material_group& c_rgrnTriGroup = c_pgrnTriGroups[g];
		TTriGroupNode * pTriGroupNode = m_triGroupNodes + g;

		int32_t iMtrl = c_rgrnTriGroup.MaterialIndex;
		
		if (iMtrl >= 0 && iMtrl < mtrlCount)
		{
			CGrannyMaterial & rMtrl = m_mtrls[iMtrl];

			pTriGroupNode->lpd3dTextures[0] = rMtrl.GetD3DTexture(0);
			pTriGroupNode->lpd3dTextures[1] = rMtrl.GetD3DTexture(1);
		
		}
	}
	*/
}

bool CGrannyMesh::LoadMaterials(CGrannyMaterialPalette& rkMtrlPal)
{
	assert(m_pgrnMesh != nullptr);
	
	if (m_pgrnMesh->MaterialBindingCount <= 0)
		return true;

	int32_t mtrlCount = m_pgrnMesh->MaterialBindingCount;
	bool bHaveBlendThing = false;
	
	for (int32_t m = 0; m < mtrlCount; ++m)
	{
		granny_material* pgrnMaterial = m_pgrnMesh->MaterialBindings[m].Material;
		uint32_t mtrlIndex=rkMtrlPal.RegisterMaterial(pgrnMaterial);
		m_mtrlIndexVector.push_back(mtrlIndex);	
		bHaveBlendThing |= rkMtrlPal.GetMaterialRef(mtrlIndex).GetType() == CGrannyMaterial::TYPE_BLEND_PNT;
	}
	m_bHaveBlendThing = bHaveBlendThing;

	return true;
}

bool CGrannyMesh::IsTwoSide() const
{
	return m_isTwoSide;
}

void CGrannyMesh::SetPNT2Mesh()
{
	m_pgrnMeshType = GrannyPNT3322VertexType;
}

void CGrannyMesh::Destroy()
{
	delete [] m_triGroupNodes;

	m_mtrlIndexVector.clear();

	// WORK
	if (m_pgrnMeshBindingTemp) 
		GrannyFreeMeshBinding(m_pgrnMeshBindingTemp);
	// END_OF_WORK

    if (m_pgrnMeshDeformer)
		GrannyFreeMeshDeformer(m_pgrnMeshDeformer); 	
	
	Initialize();
}

void CGrannyMesh::Initialize()
{
	for (int32_t r = 0; r < CGrannyMaterial::TYPE_MAX_NUM; ++r)
		m_triGroupNodeLists[r] = nullptr;

	m_pgrnMeshType = GrannyPNT332VertexType;
	m_pgrnMesh = nullptr;
	// WORK
	m_pgrnMeshBindingTemp = nullptr;
	// END_OF_WORK
	m_pgrnMeshDeformer = nullptr;

	m_triGroupNodes = nullptr;	
	
	m_vtxBasePos = 0;
	m_idxBasePos = 0;

	m_canDeformPNTVertex = false;
	m_isTwoSide = false;
	m_bHaveBlendThing = false;
}

CGrannyMesh::CGrannyMesh()
{
	Initialize();
}

CGrannyMesh::~CGrannyMesh()
{
	Destroy();
}
