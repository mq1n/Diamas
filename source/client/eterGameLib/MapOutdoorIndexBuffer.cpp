#include "StdAfx.h"
#include "MapOutdoor.h"

void CMapOutdoor::SetIndexBuffer()
{
	void * pIndices;
	int32_t x, y;
	
	uint32_t dwIndexNum = TERRAIN_PATCHSIZE * TERRAIN_PATCHSIZE * 4;

#ifdef WORLD_EDITOR
	m_pwIndices = new uint16_t[dwIndexNum];
	if (!m_pwIndices)
		TraceError("CMapOutdoor::SetIndexBuffer() IndexBuffer is nullptr");

	memset(m_pwIndices, 0, sizeof(uint16_t) * dwIndexNum);
	if (!m_IndexBuffer.Create(dwIndexNum, D3DFMT_INDEX16))
		TraceError("CMapOutdoor::SetIndexBuffer() IndexBuffer Create Error");
	
	uint16_t count = 0;
	uint16_t count2 = 0;
	int32_t ry = 0;
	
	uint8_t ucNumLineWarp = TERRAIN_PATCHSIZE + 1;
	
	for (y = 0; y < TERRAIN_PATCHSIZE; y++)
	{
		if (ry % 2 == 0)
		{
			m_pwIndices[count++] = count2;
			m_pwIndices[count++] = count2+ucNumLineWarp;
		}
		else
		{
			m_pwIndices[count++] = count2+ucNumLineWarp;
			m_pwIndices[count++] = count2;
		}
		
		for (x = 0; x < TERRAIN_PATCHSIZE; x++)
		{
			if (ry % 2 == 1)
			{
				m_pwIndices[count++] = (uint16_t) (count2+ucNumLineWarp-1);
				m_pwIndices[count++] = (uint16_t) (count2-1);
				count2 -= (int16_t) 1;
			}
			else
			{
				m_pwIndices[count++] = (uint16_t) (count2+1);
				m_pwIndices[count++] = (uint16_t) (count2+ucNumLineWarp+1);
				count2 += (int16_t) 1;
			}
		}
		
		if (y < TERRAIN_PATCHSIZE-1)
		{
			m_pwIndices[count++] = (uint16_t) (count2+ucNumLineWarp);
			m_pwIndices[count++] = (uint16_t) (count2+ucNumLineWarp);
			count2 += ucNumLineWarp;
		}
		ry++;
	}
	
	m_wNumIndices = count;
	if (!m_IndexBuffer.Lock((void **) &pIndices))
		TraceError("CMapOutdoor::SetIndexBuffer() IndexBuffer Unlock Error");
	memcpy(pIndices, m_pwIndices, count * sizeof(uint16_t));
	m_IndexBuffer.Unlock();
	
	delete [] m_pwIndices;
	m_pwIndices = nullptr;
#else
	uint16_t	count[TERRAINPATCH_LODMAX], count2[TERRAINPATCH_LODMAX];
	uint8_t uci;
	for (uci = 0; uci < TERRAINPATCH_LODMAX; ++uci)
	{		
		m_pwaIndices[uci] = new uint16_t[dwIndexNum];
		memset(m_pwaIndices[uci], 0, sizeof(uint16_t) * dwIndexNum);
		count[uci] = 0;
		count2[uci] = 0;
		if ( !m_IndexBuffer[uci].Create(dwIndexNum, D3DFMT_INDEX16) )
			TraceError("CMapOutdoor::SetIndexBuffer() IndexBuffer Create Error");
	}

	uint8_t ucNumLineWarp = TERRAIN_PATCHSIZE + 1;
	
	for (y = 0; y < TERRAIN_PATCHSIZE; y++)
	{
		if (y%2 == 0)
		{
			m_pwaIndices[0][(count[0])++] = count2[0];
			m_pwaIndices[0][(count[0])++] = count2[0]+ucNumLineWarp;
		}
		else
		{
			m_pwaIndices[0][(count[0])++] = count2[0]+ucNumLineWarp;
			m_pwaIndices[0][(count[0])++] = count2[0];
		}
		
		for (x = 0; x < TERRAIN_PATCHSIZE; x++)
		{
			if (y%2 == 0)
			{
				m_pwaIndices[0][(count[0])++] = (uint16_t) (count2[0]+1);
				m_pwaIndices[0][(count[0])++] = (uint16_t) (count2[0]+ucNumLineWarp+1);
				count2[0] += (int16_t) 1;
			}
			else
			{
				m_pwaIndices[0][(count[0])++] = (uint16_t) (count2[0]+ucNumLineWarp-1);
				m_pwaIndices[0][(count[0])++] = (uint16_t) (count2[0]-1);
				count2[0] -= (int16_t) 1;
			}
			
			if (0 == x%2)
			{
				if (0 == y)
				{
					if (0 == x)
						ADDLvl1TL(m_pwaIndices[1], count[1], count2[1], ucNumLineWarp);
					else if ((TERRAIN_PATCHSIZE - 2) == x)
						ADDLvl1TR(m_pwaIndices[1], count[1], count2[1], ucNumLineWarp);
					else
						ADDLvl1T(m_pwaIndices[1], count[1], count2[1], ucNumLineWarp);
				}
				else if ((TERRAIN_PATCHSIZE - 2) == y)
				{
					if (0 == x)
						ADDLvl1BL(m_pwaIndices[1], count[1], count2[1], ucNumLineWarp);
					else if ((TERRAIN_PATCHSIZE - 2) == x)
						ADDLvl1BR(m_pwaIndices[1], count[1], count2[1], ucNumLineWarp);
					else
						ADDLvl1B(m_pwaIndices[1], count[1], count2[1], ucNumLineWarp);
				}
				else if (0 == y%2)
				{
					if (0 == x)
						ADDLvl1L(m_pwaIndices[1], count[1], count2[1], ucNumLineWarp);
					else if ((TERRAIN_PATCHSIZE - 2) == x)
						ADDLvl1R(m_pwaIndices[1], count[1], count2[1], ucNumLineWarp);
					else
						ADDLvl1M(m_pwaIndices[1], count[1], count2[1], ucNumLineWarp);
				}
				count2[1] += 2;
			}
			
			if (0 == x%4)
			{
				if (0 == y)
				{
					if (0 == x)
						ADDLvl2TL(m_pwaIndices[2], count[2], count2[2], ucNumLineWarp);
					else if ((TERRAIN_PATCHSIZE - 4) == x)
						ADDLvl2TR(m_pwaIndices[2], count[2], count2[2], ucNumLineWarp);
					else
						ADDLvl2T(m_pwaIndices[2], count[2], count2[2], ucNumLineWarp);
				}
				else if ((TERRAIN_PATCHSIZE - 4) == y)
				{
					if (0 == x)
						ADDLvl2BL(m_pwaIndices[2], count[2], count2[2], ucNumLineWarp);
					else if ((TERRAIN_PATCHSIZE - 4) == x)
						ADDLvl2BR(m_pwaIndices[2], count[2], count2[2], ucNumLineWarp);
					else
						ADDLvl2B(m_pwaIndices[2], count[2], count2[2], ucNumLineWarp);
				}
				else if (0 == y%4)
				{
					if (0 == x)
						ADDLvl2L(m_pwaIndices[2], count[2], count2[2], ucNumLineWarp);
					else if ((TERRAIN_PATCHSIZE - 4) == x)
						ADDLvl2R(m_pwaIndices[2], count[2], count2[2], ucNumLineWarp);
					else
						ADDLvl2M(m_pwaIndices[2], count[2], count2[2], ucNumLineWarp);
				}
				count2[2] += 4;
			}
		}
		
		if (y < TERRAIN_PATCHSIZE-1)
		{
			m_pwaIndices[0][(count[0])++] = (uint16_t) (count2[0]+ucNumLineWarp);
			m_pwaIndices[0][(count[0])++] = (uint16_t) (count2[0]+ucNumLineWarp);
			count2[0] += ucNumLineWarp;
			if (0 == y%2)
				count2[1] += 2;
			if (0 == y%4)
				count2[2] += 4;
		}
	}

	for (uci = 0; uci < TERRAINPATCH_LODMAX; ++uci)
	{
		m_wNumIndices[uci] = count[uci];
		if( !m_IndexBuffer[uci].Lock((void **) &pIndices) )
			TraceError("CMapOutdoor::SetIndexBuffer() IndexBuffer Unlock Error");
		memcpy(pIndices, m_pwaIndices[uci], count[uci] * sizeof(uint16_t));
		m_IndexBuffer[uci].Unlock();
		delete [] m_pwaIndices[uci];
		m_pwaIndices[uci] = nullptr;
	}
#endif
}

void CMapOutdoor::ADDLvl1TL(uint16_t * pIndices, uint16_t & rwCount, const uint16_t & c_rwCurCount, const uint8_t & c_rucNumLineWarp)
{
	pIndices[rwCount++] = c_rwCurCount;
	pIndices[rwCount++] = c_rwCurCount + c_rucNumLineWarp;
	pIndices[rwCount++] = c_rwCurCount + c_rucNumLineWarp + 1;

	pIndices[rwCount++] = c_rwCurCount + c_rucNumLineWarp;
	pIndices[rwCount++] = c_rwCurCount + c_rucNumLineWarp * 2;
	pIndices[rwCount++] = c_rwCurCount + c_rucNumLineWarp + 1;

	pIndices[rwCount++] = c_rwCurCount + c_rucNumLineWarp + 1;
	pIndices[rwCount++] = c_rwCurCount + c_rucNumLineWarp * 2;
	pIndices[rwCount++] = c_rwCurCount + c_rucNumLineWarp * 2 + 2;

	pIndices[rwCount++] = c_rwCurCount + c_rucNumLineWarp + 1;
	pIndices[rwCount++] = c_rwCurCount + c_rucNumLineWarp * 2 + 2;
	pIndices[rwCount++] = c_rwCurCount + 2;

	pIndices[rwCount++] = c_rwCurCount + c_rucNumLineWarp + 1;
	pIndices[rwCount++] = c_rwCurCount + 2;
	pIndices[rwCount++] = c_rwCurCount + 1;

	pIndices[rwCount++] = c_rwCurCount + c_rucNumLineWarp + 1;
	pIndices[rwCount++] = c_rwCurCount + 1;
	pIndices[rwCount++] = c_rwCurCount;
}

void CMapOutdoor::ADDLvl1T(uint16_t * pIndices, uint16_t & rwCount, const uint16_t & c_rwCurCount, const uint8_t & c_rucNumLineWarp)
{
	pIndices[rwCount++] = c_rwCurCount;
	pIndices[rwCount++] = c_rwCurCount + c_rucNumLineWarp * 2;
	pIndices[rwCount++] = c_rwCurCount + c_rucNumLineWarp + 1;
	
	pIndices[rwCount++] = c_rwCurCount + c_rucNumLineWarp * 2;
	pIndices[rwCount++] = c_rwCurCount + c_rucNumLineWarp * 2 + 2;
	pIndices[rwCount++] = c_rwCurCount + c_rucNumLineWarp + 1;
	
	pIndices[rwCount++] = c_rwCurCount + c_rucNumLineWarp * 2 + 2;
	pIndices[rwCount++] = c_rwCurCount + 2;
	pIndices[rwCount++] = c_rwCurCount + c_rucNumLineWarp + 1;
	
	pIndices[rwCount++] = c_rwCurCount + 2;
	pIndices[rwCount++] = c_rwCurCount + 1;
	pIndices[rwCount++] = c_rwCurCount + c_rucNumLineWarp + 1;
	
	pIndices[rwCount++] = c_rwCurCount + 1;
	pIndices[rwCount++] = c_rwCurCount;
	pIndices[rwCount++] = c_rwCurCount + c_rucNumLineWarp + 1;
}

void CMapOutdoor::ADDLvl1TR(uint16_t * pIndices, uint16_t & rwCount, const uint16_t & c_rwCurCount, const uint8_t & c_rucNumLineWarp)
{
	pIndices[rwCount++] = c_rwCurCount;
	pIndices[rwCount++] = c_rwCurCount + c_rucNumLineWarp * 2;
	pIndices[rwCount++] = c_rwCurCount + c_rucNumLineWarp + 1;
	
	pIndices[rwCount++] = c_rwCurCount + c_rucNumLineWarp * 2;
	pIndices[rwCount++] = c_rwCurCount + c_rucNumLineWarp * 2 + 2;
	pIndices[rwCount++] = c_rwCurCount + c_rucNumLineWarp + 1;
	
	pIndices[rwCount++] = c_rwCurCount + c_rucNumLineWarp * 2 + 2;
	pIndices[rwCount++] = c_rwCurCount + c_rucNumLineWarp + 2;
	pIndices[rwCount++] = c_rwCurCount + c_rucNumLineWarp + 1;
	
	pIndices[rwCount++] = c_rwCurCount + c_rucNumLineWarp + 2;
	pIndices[rwCount++] = c_rwCurCount + 2;
	pIndices[rwCount++] = c_rwCurCount + c_rucNumLineWarp + 1;
	
	pIndices[rwCount++] = c_rwCurCount + 2;
	pIndices[rwCount++] = c_rwCurCount + 1;
	pIndices[rwCount++] = c_rwCurCount + c_rucNumLineWarp + 1;

	pIndices[rwCount++] = c_rwCurCount + 1;
	pIndices[rwCount++] = c_rwCurCount;
	pIndices[rwCount++] = c_rwCurCount + c_rucNumLineWarp + 1;
}

void CMapOutdoor::ADDLvl1L(uint16_t * pIndices, uint16_t & rwCount, const uint16_t & c_rwCurCount, const uint8_t & c_rucNumLineWarp)
{
	pIndices[rwCount++] = c_rwCurCount;
	pIndices[rwCount++] = c_rwCurCount + c_rucNumLineWarp;
	pIndices[rwCount++] = c_rwCurCount + c_rucNumLineWarp + 1;
	
	pIndices[rwCount++] = c_rwCurCount + c_rucNumLineWarp;
	pIndices[rwCount++] = c_rwCurCount + c_rucNumLineWarp * 2;
	pIndices[rwCount++] = c_rwCurCount + c_rucNumLineWarp + 1;
	
	pIndices[rwCount++] = c_rwCurCount + c_rucNumLineWarp * 2;
	pIndices[rwCount++] = c_rwCurCount + c_rucNumLineWarp * 2 + 2;
	pIndices[rwCount++] = c_rwCurCount + c_rucNumLineWarp + 1;
	
	pIndices[rwCount++] = c_rwCurCount + c_rucNumLineWarp * 2 + 2;
	pIndices[rwCount++] = c_rwCurCount + 2;
	pIndices[rwCount++] = c_rwCurCount + c_rucNumLineWarp + 1;
	
	pIndices[rwCount++] = c_rwCurCount + 2;
	pIndices[rwCount++] = c_rwCurCount;
	pIndices[rwCount++] = c_rwCurCount + c_rucNumLineWarp + 1;
}

void CMapOutdoor::ADDLvl1R(uint16_t * pIndices, uint16_t & rwCount, const uint16_t & c_rwCurCount, const uint8_t & c_rucNumLineWarp)
{
	pIndices[rwCount++] = c_rwCurCount;
	pIndices[rwCount++] = c_rwCurCount + c_rucNumLineWarp * 2;
	pIndices[rwCount++] = c_rwCurCount + c_rucNumLineWarp + 1;
	
	pIndices[rwCount++] = c_rwCurCount + c_rucNumLineWarp * 2;
	pIndices[rwCount++] = c_rwCurCount + c_rucNumLineWarp * 2 + 2;
	pIndices[rwCount++] = c_rwCurCount + c_rucNumLineWarp + 1;
	
	pIndices[rwCount++] = c_rwCurCount + c_rucNumLineWarp * 2 + 2;
	pIndices[rwCount++] = c_rwCurCount + c_rucNumLineWarp + 2;
	pIndices[rwCount++] = c_rwCurCount + c_rucNumLineWarp + 1;
	
	pIndices[rwCount++] = c_rwCurCount + c_rucNumLineWarp + 2;
	pIndices[rwCount++] = c_rwCurCount + 2;
	pIndices[rwCount++] = c_rwCurCount + c_rucNumLineWarp + 1;
	
	pIndices[rwCount++] = c_rwCurCount + 2;
	pIndices[rwCount++] = c_rwCurCount;
	pIndices[rwCount++] = c_rwCurCount + c_rucNumLineWarp + 1;
}

void CMapOutdoor::ADDLvl1BL(uint16_t * pIndices, uint16_t & rwCount, const uint16_t & c_rwCurCount, const uint8_t & c_rucNumLineWarp)
{
	pIndices[rwCount++] = c_rwCurCount;
	pIndices[rwCount++] = c_rwCurCount + c_rucNumLineWarp;
	pIndices[rwCount++] = c_rwCurCount + c_rucNumLineWarp + 1;
	
	pIndices[rwCount++] = c_rwCurCount + c_rucNumLineWarp;
	pIndices[rwCount++] = c_rwCurCount + c_rucNumLineWarp * 2;
	pIndices[rwCount++] = c_rwCurCount + c_rucNumLineWarp + 1;
	
	pIndices[rwCount++] = c_rwCurCount + c_rucNumLineWarp + 1;
	pIndices[rwCount++] = c_rwCurCount + c_rucNumLineWarp * 2;
	pIndices[rwCount++] = c_rwCurCount + c_rucNumLineWarp * 2 + 1;
	
	pIndices[rwCount++] = c_rwCurCount + c_rucNumLineWarp + 1;
	pIndices[rwCount++] = c_rwCurCount + c_rucNumLineWarp * 2 + 1;
	pIndices[rwCount++] = c_rwCurCount + c_rucNumLineWarp * 2 + 2;
	
	pIndices[rwCount++] = c_rwCurCount + c_rucNumLineWarp * 2 + 2;
	pIndices[rwCount++] = c_rwCurCount + 2;
	pIndices[rwCount++] = c_rwCurCount + c_rucNumLineWarp + 1;
	
	pIndices[rwCount++] = c_rwCurCount + 2;
	pIndices[rwCount++] = c_rwCurCount;
	pIndices[rwCount++] = c_rwCurCount + c_rucNumLineWarp + 1;
}

void CMapOutdoor::ADDLvl1B(uint16_t * pIndices, uint16_t & rwCount, const uint16_t & c_rwCurCount, const uint8_t & c_rucNumLineWarp)
{
	pIndices[rwCount++] = c_rwCurCount;
	pIndices[rwCount++] = c_rwCurCount + c_rucNumLineWarp * 2;
	pIndices[rwCount++] = c_rwCurCount + c_rucNumLineWarp + 1;
	
	pIndices[rwCount++] = c_rwCurCount + c_rucNumLineWarp * 2;
	pIndices[rwCount++] = c_rwCurCount + c_rucNumLineWarp * 2 + 1;
	pIndices[rwCount++] = c_rwCurCount + c_rucNumLineWarp + 1;
	
	pIndices[rwCount++] = c_rwCurCount + c_rucNumLineWarp * 2 + 1;
	pIndices[rwCount++] = c_rwCurCount + c_rucNumLineWarp * 2 + 2;
	pIndices[rwCount++] = c_rwCurCount + c_rucNumLineWarp + 1;
	
	pIndices[rwCount++] = c_rwCurCount + c_rucNumLineWarp * 2 + 2;
	pIndices[rwCount++] = c_rwCurCount + 2;
	pIndices[rwCount++] = c_rwCurCount + c_rucNumLineWarp + 1;
	
	pIndices[rwCount++] = c_rwCurCount + 2;
	pIndices[rwCount++] = c_rwCurCount;
	pIndices[rwCount++] = c_rwCurCount + c_rucNumLineWarp + 1;
}

void CMapOutdoor::ADDLvl1BR(uint16_t * pIndices, uint16_t & rwCount, const uint16_t & c_rwCurCount, const uint8_t & c_rucNumLineWarp)
{
	pIndices[rwCount++] = c_rwCurCount;
	pIndices[rwCount++] = c_rwCurCount + c_rucNumLineWarp * 2;
	pIndices[rwCount++] = c_rwCurCount + c_rucNumLineWarp + 1;
	
	pIndices[rwCount++] = c_rwCurCount + c_rucNumLineWarp * 2;
	pIndices[rwCount++] = c_rwCurCount + c_rucNumLineWarp * 2 + 1;
	pIndices[rwCount++] = c_rwCurCount + c_rucNumLineWarp + 1;
	
	pIndices[rwCount++] = c_rwCurCount + c_rucNumLineWarp * 2 + 1;
	pIndices[rwCount++] = c_rwCurCount + c_rucNumLineWarp * 2 + 2;
	pIndices[rwCount++] = c_rwCurCount + c_rucNumLineWarp + 1;
	
	pIndices[rwCount++] = c_rwCurCount + c_rucNumLineWarp * 2 + 2;
	pIndices[rwCount++] = c_rwCurCount + c_rucNumLineWarp + 2;
	pIndices[rwCount++] = c_rwCurCount + c_rucNumLineWarp + 1;
	
	pIndices[rwCount++] = c_rwCurCount + c_rucNumLineWarp + 2;
	pIndices[rwCount++] = c_rwCurCount + 2;
	pIndices[rwCount++] = c_rwCurCount + c_rucNumLineWarp + 1;
	
	pIndices[rwCount++] = c_rwCurCount + 2;
	pIndices[rwCount++] = c_rwCurCount;
	pIndices[rwCount++] = c_rwCurCount + c_rucNumLineWarp + 1;
}

void CMapOutdoor::ADDLvl1M(uint16_t * pIndices, uint16_t & rwCount, const uint16_t & c_rwCurCount, const uint8_t & c_rucNumLineWarp)
{
	pIndices[rwCount++] = c_rwCurCount;
	pIndices[rwCount++] = c_rwCurCount + c_rucNumLineWarp * 2;
	pIndices[rwCount++] = c_rwCurCount + 2;
	
	pIndices[rwCount++] = c_rwCurCount + c_rucNumLineWarp * 2;
	pIndices[rwCount++] = c_rwCurCount + c_rucNumLineWarp * 2 + 2;
	pIndices[rwCount++] = c_rwCurCount + 2;
}

void CMapOutdoor::ADDLvl2TL(uint16_t * pIndices, uint16_t & rwCount, const uint16_t & c_rwCurCount, const uint8_t & c_rucNumLineWarp)
{
	ADDLvl1TL(pIndices, rwCount, c_rwCurCount, c_rucNumLineWarp);
	ADDLvl1T(pIndices, rwCount, c_rwCurCount + 2, c_rucNumLineWarp);
	ADDLvl1L(pIndices, rwCount, c_rwCurCount + c_rucNumLineWarp * 2, c_rucNumLineWarp);
	ADDLvl1M(pIndices, rwCount, c_rwCurCount + c_rucNumLineWarp * 2 + 2, c_rucNumLineWarp);
}

void CMapOutdoor::ADDLvl2T(uint16_t * pIndices, uint16_t & rwCount, const uint16_t & c_rwCurCount, const uint8_t & c_rucNumLineWarp)
{
	ADDLvl1T(pIndices, rwCount, c_rwCurCount, c_rucNumLineWarp);
	ADDLvl1T(pIndices, rwCount, c_rwCurCount + 2, c_rucNumLineWarp);
	
	pIndices[rwCount++] = c_rwCurCount + c_rucNumLineWarp * 2;
	pIndices[rwCount++] = c_rwCurCount + c_rucNumLineWarp * 4;
	pIndices[rwCount++] = c_rwCurCount + c_rucNumLineWarp * 2 + 2;

	pIndices[rwCount++] = c_rwCurCount + c_rucNumLineWarp * 4;
	pIndices[rwCount++] = c_rwCurCount + c_rucNumLineWarp * 4 + 4;
	pIndices[rwCount++] = c_rwCurCount + c_rucNumLineWarp * 2 + 2;
	
	pIndices[rwCount++] = c_rwCurCount + c_rucNumLineWarp * 4 + 4;
	pIndices[rwCount++] = c_rwCurCount + c_rucNumLineWarp * 2 + 4;
	pIndices[rwCount++] = c_rwCurCount + c_rucNumLineWarp * 2 + 2;
}

void CMapOutdoor::ADDLvl2TR(uint16_t * pIndices, uint16_t & rwCount, const uint16_t & c_rwCurCount, const uint8_t & c_rucNumLineWarp)
{
	ADDLvl1T(pIndices, rwCount, c_rwCurCount, c_rucNumLineWarp);
	ADDLvl1TR(pIndices, rwCount, c_rwCurCount + 2, c_rucNumLineWarp);
	ADDLvl1M(pIndices, rwCount, c_rwCurCount + c_rucNumLineWarp * 2, c_rucNumLineWarp);
	ADDLvl1R(pIndices, rwCount, c_rwCurCount + c_rucNumLineWarp * 2 + 2, c_rucNumLineWarp);
}

void CMapOutdoor::ADDLvl2L(uint16_t * pIndices, uint16_t & rwCount, const uint16_t & c_rwCurCount, const uint8_t & c_rucNumLineWarp)
{
	ADDLvl1L(pIndices, rwCount, c_rwCurCount, c_rucNumLineWarp);
	ADDLvl1L(pIndices, rwCount, c_rwCurCount + c_rucNumLineWarp * 2, c_rucNumLineWarp);

	pIndices[rwCount++] = c_rwCurCount + 2;
	pIndices[rwCount++] = c_rwCurCount + c_rucNumLineWarp * 2 + 2;
	pIndices[rwCount++] = c_rwCurCount + 4;
	
	pIndices[rwCount++] = c_rwCurCount + 4;
	pIndices[rwCount++] = c_rwCurCount + c_rucNumLineWarp * 2 + 2;
	pIndices[rwCount++] = c_rwCurCount + c_rucNumLineWarp * 4 + 4;
	
	pIndices[rwCount++] = c_rwCurCount + c_rucNumLineWarp * 4 + 4;
	pIndices[rwCount++] = c_rwCurCount + c_rucNumLineWarp * 2 + 2;
	pIndices[rwCount++] = c_rwCurCount + c_rucNumLineWarp * 4 + 2;
}

void CMapOutdoor::ADDLvl2R(uint16_t * pIndices, uint16_t & rwCount, const uint16_t & c_rwCurCount, const uint8_t & c_rucNumLineWarp)
{
	ADDLvl1R(pIndices, rwCount, c_rwCurCount + 2, c_rucNumLineWarp);
	ADDLvl1R(pIndices, rwCount, c_rwCurCount + c_rucNumLineWarp * 2 + 2, c_rucNumLineWarp);
	
	pIndices[rwCount++] = c_rwCurCount + 2;
	pIndices[rwCount++] = c_rwCurCount;
	pIndices[rwCount++] = c_rwCurCount + c_rucNumLineWarp * 2 + 2;
	
	pIndices[rwCount++] = c_rwCurCount;
	pIndices[rwCount++] = c_rwCurCount + c_rucNumLineWarp * 4;
	pIndices[rwCount++] = c_rwCurCount + c_rucNumLineWarp * 2 + 2;
	
	pIndices[rwCount++] = c_rwCurCount + c_rucNumLineWarp * 2 + 2;
	pIndices[rwCount++] = c_rwCurCount + c_rucNumLineWarp * 4;
	pIndices[rwCount++] = c_rwCurCount + c_rucNumLineWarp * 4 + 2;
}

void CMapOutdoor::ADDLvl2BL(uint16_t * pIndices, uint16_t & rwCount, const uint16_t & c_rwCurCount, const uint8_t & c_rucNumLineWarp)
{
	ADDLvl1L(pIndices, rwCount, c_rwCurCount, c_rucNumLineWarp);
	ADDLvl1M(pIndices, rwCount, c_rwCurCount + 2, c_rucNumLineWarp);
	ADDLvl1BL(pIndices, rwCount, c_rwCurCount + c_rucNumLineWarp * 2, c_rucNumLineWarp);
	ADDLvl1B(pIndices, rwCount, c_rwCurCount + c_rucNumLineWarp * 2 + 2, c_rucNumLineWarp);
}

void CMapOutdoor::ADDLvl2B(uint16_t * pIndices, uint16_t & rwCount, const uint16_t & c_rwCurCount, const uint8_t & c_rucNumLineWarp)
{
	pIndices[rwCount++] = c_rwCurCount + c_rucNumLineWarp * 2;
	pIndices[rwCount++] = c_rwCurCount + c_rucNumLineWarp * 2 + 2;
	pIndices[rwCount++] = c_rwCurCount;
	
	pIndices[rwCount++] = c_rwCurCount;
	pIndices[rwCount++] = c_rwCurCount + c_rucNumLineWarp * 2 + 2;
	pIndices[rwCount++] = c_rwCurCount + 4;
	
	pIndices[rwCount++] = c_rwCurCount + 4;
	pIndices[rwCount++] = c_rwCurCount + c_rucNumLineWarp * 2 + 2;
	pIndices[rwCount++] = c_rwCurCount + c_rucNumLineWarp * 2 + 4;

	ADDLvl1B(pIndices, rwCount, c_rwCurCount + c_rucNumLineWarp * 2, c_rucNumLineWarp);
	ADDLvl1B(pIndices, rwCount, c_rwCurCount + c_rucNumLineWarp * 2 + 2, c_rucNumLineWarp);
}

void CMapOutdoor::ADDLvl2BR(uint16_t * pIndices, uint16_t & rwCount, const uint16_t & c_rwCurCount, const uint8_t & c_rucNumLineWarp)
{
	ADDLvl1M(pIndices, rwCount, c_rwCurCount, c_rucNumLineWarp);
	ADDLvl1R(pIndices, rwCount, c_rwCurCount + 2, c_rucNumLineWarp);
	ADDLvl1B(pIndices, rwCount, c_rwCurCount + c_rucNumLineWarp * 2, c_rucNumLineWarp);
	ADDLvl1BR(pIndices, rwCount, c_rwCurCount + c_rucNumLineWarp * 2 + 2, c_rucNumLineWarp);
}

void CMapOutdoor::ADDLvl2M(uint16_t * pIndices, uint16_t & rwCount, const uint16_t & c_rwCurCount, const uint8_t & c_rucNumLineWarp)
{
	pIndices[rwCount++] = c_rwCurCount;
	pIndices[rwCount++] = c_rwCurCount + c_rucNumLineWarp * 4;
	pIndices[rwCount++] = c_rwCurCount + 4;
	
	pIndices[rwCount++] = c_rwCurCount + c_rucNumLineWarp * 4;
	pIndices[rwCount++] = c_rwCurCount + c_rucNumLineWarp * 4 + 4;
	pIndices[rwCount++] = c_rwCurCount + 4;
}
