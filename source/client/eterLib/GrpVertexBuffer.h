#pragma once

#include "GrpBase.h"

class CGraphicVertexBuffer : public CGraphicBase
{
	public:
		CGraphicVertexBuffer();
		virtual ~CGraphicVertexBuffer();

		void	Destroy();
		virtual bool	Create(int32_t vtxCount, uint32_t fvf, uint32_t usage, D3DPOOL d3dPool);

		bool	CreateDeviceObjects();
		void	DestroyDeviceObjects();

		bool	Copy(int32_t bufSize, const void* srcVertices);

		bool	LockRange(uint32_t count, void** pretVertices) const;
		bool	Lock(void** pretVertices) const;
		bool	Unlock() const;

		bool	LockDynamic(void** pretVertices);
		bool	Unlock();

		void	SetStream(int32_t stride, int32_t layer=0) const;
			
		int32_t		GetVertexCount() const;
		int32_t		GetVertexStride() const;
		uint32_t	GetFlexibleVertexFormat() const;

		inline	LPDIRECT3DVERTEXBUFFER9 GetD3DVertexBuffer() const	{ return m_lpd3dVB; }
		inline	uint32_t GetBufferSize() const	{ return m_dwBufferSize; }

		bool	IsEmpty() const;

	protected:
		void	Initialize();

	protected:
		LPDIRECT3DVERTEXBUFFER9 m_lpd3dVB;

		uint32_t					m_dwBufferSize;
		uint32_t					m_dwFVF;
		uint32_t					m_dwUsage;
		D3DPOOL					m_d3dPool;
		int32_t						m_vtxCount;
		uint32_t					m_dwLockFlag;
};
