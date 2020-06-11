#pragma once

#include "../eterlib/GrpVertexBuffer.h"
#include "../eterlib/GrpIndexBuffer.h"

#include "Mesh.h"

class CGrannyModel : public CReferenceObject
{
	public:
		typedef struct SMeshNode
		{
			int32_t					iMesh;
			const CGrannyMesh * pMesh;
			SMeshNode *			pNextMeshNode;
		} TMeshNode;

	public:
		CGrannyModel();
		virtual ~CGrannyModel();

		bool IsEmpty() const;
		bool CreateFromGrannyModelPointer(granny_model* pgrnModel);
		bool CreateDeviceObjects();
		void DestroyDeviceObjects();
		void Destroy();

		int32_t GetRigidVertexCount() const;
		int32_t GetDeformVertexCount() const;
		int32_t GetVertexCount() const;

		bool CanDeformPNTVertices() const;
		void DeformPNTVertices(void* dstBaseVertices, D3DXMATRIX* boneMatrices, const std::vector<granny_mesh_binding*>& c_rvct_pgrnMeshBinding) const;

		int32_t GetIdxCount();
		int32_t GetMeshCount() const;
		CGrannyMesh * GetMeshPointer(int32_t iMesh);
		granny_model * GetGrannyModelPointer();
		const CGrannyMesh* GetMeshPointer(int32_t iMesh) const;

		LPDIRECT3DVERTEXBUFFER9 GetPNTD3DVertexBuffer() const;
		LPDIRECT3DINDEXBUFFER9 GetD3DIndexBuffer() const;

		const CGrannyModel::TMeshNode*  GetMeshNodeList(CGrannyMesh::EType eMeshType, CGrannyMaterial::EType eMtrlType) const;

		bool LockVertices(void** indicies, void** vertices) const;
		void UnlockVertices() const;

		const CGrannyMaterialPalette& GetMaterialPalette() const;

	protected:
		bool LoadMeshs();		
		bool LoadPNTVertices();
		bool LoadIndices();
		void Initialize();

		BOOL CheckMeshIndex(int32_t iIndex) const;
		void AppendMeshNode(CGrannyMesh::EType eMeshType, CGrannyMaterial::EType eMtrlType, int32_t iMesh);

	protected:
		// Granny Data
		granny_model *			m_pgrnModel;

		// Static Data
		CGrannyMesh *			m_meshs;

		CGraphicVertexBuffer	m_pntVtxBuf;	// for rigid mesh
		CGraphicIndexBuffer		m_idxBuf;

		TMeshNode *				m_meshNodes;
		TMeshNode *				m_meshNodeLists[CGrannyMesh::TYPE_MAX_NUM][CGrannyMaterial::TYPE_MAX_NUM];

		int32_t						m_deformVtxCount;
		int32_t						m_rigidVtxCount;
		int32_t						m_vtxCount;
		int32_t						m_idxCount;

		int32_t						m_meshNodeSize;
		int32_t						m_meshNodeCapacity;

		bool					m_canDeformPNVertices;
		
		CGrannyMaterialPalette	m_kMtrlPal;
	private:
		bool					m_bHaveBlendThing;
	public:
		bool					HaveBlendThing() { return m_bHaveBlendThing; }
	
	//////////////////////////////////////////////////////////////////////////
	// New members to support PNT2 type models
	protected:
		bool __LoadVertices();
	protected:
		uint32_t m_dwFvF;
	// New members to support PNT2 type models
	//////////////////////////////////////////////////////////////////////////

};
