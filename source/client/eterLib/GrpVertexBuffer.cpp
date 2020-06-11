#include "StdAfx.h"
#include "../eterBase/Stl.h"
#include "GrpVertexBuffer.h"
#include "StateManager.h"

int32_t	CGraphicVertexBuffer::GetVertexStride() const
{
	int32_t retSize = D3DXGetFVFVertexSize(m_dwFVF);
	return retSize;
}

uint32_t CGraphicVertexBuffer::GetFlexibleVertexFormat() const
{
	return m_dwFVF;
}

int32_t CGraphicVertexBuffer::GetVertexCount() const
{
	return m_vtxCount;
}

void CGraphicVertexBuffer::SetStream(int32_t stride, int32_t layer) const
{
	assert(ms_lpd3dDevice != nullptr);
	STATEMANAGER.SetStreamSource(layer, m_lpd3dVB, stride);	
}

bool CGraphicVertexBuffer::LockRange(uint32_t count, void** pretVertices) const
{
	if (!m_lpd3dVB)
		return false;

	uint32_t dwLockSize=GetVertexStride() * count;
	if (FAILED(m_lpd3dVB->Lock(0, dwLockSize, (void **)pretVertices, m_dwLockFlag)))
		return false;

	return true;
}

bool CGraphicVertexBuffer::Lock(void ** pretVertices) const
{
	if (!m_lpd3dVB)
		return false;

	uint32_t dwLockSize=GetVertexStride()*GetVertexCount();
	if (FAILED(m_lpd3dVB->Lock(0, dwLockSize, (void **)pretVertices, m_dwLockFlag)))
		return false;

	return true;
}

bool CGraphicVertexBuffer::Unlock() const
{
	if (!m_lpd3dVB)
		return false;

	if ( FAILED(m_lpd3dVB->Unlock()) )
		return false;
	return true;
}

bool CGraphicVertexBuffer::IsEmpty() const
{
	if (m_lpd3dVB)
		return true;
	else
		return false;
}

bool CGraphicVertexBuffer::LockDynamic(void** pretVertices)
{
	if (!m_lpd3dVB)
		return false;

	if (FAILED(m_lpd3dVB->Lock(0, 0, (void **)pretVertices, 0)))
		return false;

	return true;
}

bool CGraphicVertexBuffer::Lock(void ** pretVertices)
{
	if (!m_lpd3dVB)
		return false;

	if (FAILED(m_lpd3dVB->Lock(0, 0, (void **)pretVertices, m_dwLockFlag)))
		return false;

	return true;
}

bool CGraphicVertexBuffer::Unlock()
{
	if (!m_lpd3dVB)
		return false;

	if ( FAILED(m_lpd3dVB->Unlock()) )
		return false;
	return true;
}

bool CGraphicVertexBuffer::Copy(int32_t bufSize, const void* srcVertices)
{
	void * dstVertices;

	if (!Lock(&dstVertices))
		return false;

	memcpy(dstVertices, srcVertices, bufSize);
	
	Unlock();
	return true;
}

bool CGraphicVertexBuffer::CreateDeviceObjects()
{
	assert(ms_lpd3dDevice != nullptr);
	assert(m_lpd3dVB == nullptr);

	if (FAILED(
		ms_lpd3dDevice->CreateVertexBuffer(
		m_dwBufferSize, 
		m_dwUsage, 
		m_dwFVF, 
		m_d3dPool, 
		&m_lpd3dVB,
		nullptr)
		))
		return false;

	return true;
}

void CGraphicVertexBuffer::DestroyDeviceObjects()
{
	safe_release(m_lpd3dVB);
}

bool CGraphicVertexBuffer::Create(int32_t vtxCount, uint32_t fvf, uint32_t usage, D3DPOOL d3dPool)
{
	assert(ms_lpd3dDevice != nullptr);
	assert(vtxCount > 0);

	Destroy();

	m_vtxCount = vtxCount;
	m_dwBufferSize = D3DXGetFVFVertexSize(fvf) * m_vtxCount;
	m_d3dPool = d3dPool;
	m_dwUsage = usage;
	m_dwFVF = fvf;

	if (usage == D3DUSAGE_WRITEONLY || usage == D3DUSAGE_DYNAMIC)
		m_dwLockFlag = 0;
	else
		m_dwLockFlag = D3DLOCK_READONLY;

	return CreateDeviceObjects();
}

void CGraphicVertexBuffer::Destroy()
{
	DestroyDeviceObjects();
}

void CGraphicVertexBuffer::Initialize()
{
	m_lpd3dVB = nullptr;
	m_vtxCount = 0;
	m_dwBufferSize = 0;
}

CGraphicVertexBuffer::CGraphicVertexBuffer()
{
	Initialize();
}

CGraphicVertexBuffer::~CGraphicVertexBuffer()
{
	Destroy();
}
