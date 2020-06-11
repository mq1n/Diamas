#pragma once

#include "GrpVertexBuffer.h"

class CDynamicVertexBuffer : public CGraphicVertexBuffer
{
	public:
		CDynamicVertexBuffer();
		virtual ~CDynamicVertexBuffer();

		bool Create(int32_t vtxCount, int32_t fvf);



	protected:
		int32_t m_vtxCount;
		int32_t m_fvf;
};

