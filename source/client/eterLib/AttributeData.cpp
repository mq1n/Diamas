#include "StdAfx.h"
#include "../eterBase/Utils.h"
#include "AttributeData.h"

const char c_szAttributeDataFileHeader[] = "AttributeData";
const int32_t c_iAttributeDataFileHeaderLength = 13;

/*uint32_t CAttributeData::GetCollisionDataCount() const
{
	return m_CollisionDataVector.size();
}

BOOL CAttributeData::GetCollisionDataPointer(uint32_t dwIndex, const TCollisionData ** c_ppCollisionData) const
{
	if (dwIndex >= GetCollisionDataCount())
		return FALSE;

	*c_ppCollisionData = &m_CollisionDataVector[dwIndex];

	return TRUE;
}
*/

const CStaticCollisionDataVector & CAttributeData::GetCollisionDataVector() const
{
	return m_StaticCollisionDataVector;
}

const THeightDataVector & CAttributeData::GetHeightDataVector() const
{
	return m_HeightDataVector;
}

uint32_t CAttributeData::GetHeightDataCount() const
{
	return m_HeightDataVector.size();
}

BOOL CAttributeData::GetHeightDataPointer(uint32_t dwIndex, const THeightData ** c_ppHeightData) const
{
	if (dwIndex >= GetHeightDataCount())
		return FALSE;

	*c_ppHeightData = &m_HeightDataVector[dwIndex];

	return TRUE;
}

float CAttributeData::GetMaximizeRadius()
{
	return m_fMaximizeRadius;
}

size_t CAttributeData::AddCollisionData(const CStaticCollisionData& data)
{
	m_StaticCollisionDataVector.emplace_back(data);
	return m_StaticCollisionDataVector.size();
}

bool CAttributeData::OnLoad(int32_t /*iSize*/, const void * c_pvBuf)
{
	if (!c_pvBuf)
	{
		// NOTE: 파일이 존재하지 않으면 다른곳에서 그래픽 모델을 기반으로 충돌 데이터를 생성하니 리소스를 파괴하지 않고 유지시킴.
		return true;
	}

	const auto * c_pbBuf = static_cast<const uint8_t *>(c_pvBuf);

	char szHeader[c_iAttributeDataFileHeaderLength+1];
	memcpy(szHeader, c_pbBuf, c_iAttributeDataFileHeaderLength+1);
	c_pbBuf += c_iAttributeDataFileHeaderLength+1;
	if (strcmp(szHeader, c_szAttributeDataFileHeader) != 0)
		return FALSE;

	uint32_t dwCollisionDataCount;
	uint32_t dwHeightDataCount;
	memcpy(&dwCollisionDataCount, c_pbBuf, sizeof(uint32_t));
	c_pbBuf += sizeof(uint32_t);
	memcpy(&dwHeightDataCount, c_pbBuf, sizeof(uint32_t));
	c_pbBuf += sizeof(uint32_t);

	m_StaticCollisionDataVector.clear();
	m_StaticCollisionDataVector.resize(dwCollisionDataCount);
	m_HeightDataVector.clear();
	m_HeightDataVector.resize(dwHeightDataCount);

	for (uint32_t i = 0; i < dwCollisionDataCount; ++i)
	{
		CStaticCollisionData & rCollisionData = m_StaticCollisionDataVector[i];
		memcpy(&rCollisionData.dwType, c_pbBuf, sizeof(uint32_t));
		c_pbBuf += sizeof(uint32_t);
		memcpy(rCollisionData.szName, c_pbBuf, 32);
		c_pbBuf += 32;
		memcpy(&rCollisionData.v3Position, c_pbBuf, sizeof(D3DXVECTOR3));
		c_pbBuf += sizeof(D3DXVECTOR3);

		switch(rCollisionData.dwType)
		{
			case COLLISION_TYPE_PLANE:
				memcpy(rCollisionData.fDimensions, c_pbBuf, 2*sizeof(float));
				c_pbBuf += 2*sizeof(float);
				break;
			case COLLISION_TYPE_BOX:
				memcpy(rCollisionData.fDimensions, c_pbBuf, 3*sizeof(float));
				c_pbBuf += 3*sizeof(float);
				break;
			case COLLISION_TYPE_SPHERE:
				memcpy(rCollisionData.fDimensions, c_pbBuf, sizeof(float));
				c_pbBuf += sizeof(float);
				break;
			case COLLISION_TYPE_CYLINDER:
				memcpy(rCollisionData.fDimensions, c_pbBuf, 2*sizeof(float));
				c_pbBuf += 2*sizeof(float);
				break;
			case COLLISION_TYPE_AABB:
				memcpy(rCollisionData.fDimensions, c_pbBuf, 3*sizeof(float));
				c_pbBuf += 3*sizeof(float);
				break;
			case COLLISION_TYPE_OBB:
				memcpy(rCollisionData.fDimensions, c_pbBuf, 3*sizeof(float));
				c_pbBuf += 3*sizeof(float);
				break;
		}

		memcpy(rCollisionData.quatRotation, c_pbBuf, sizeof(D3DXQUATERNION));
		c_pbBuf += sizeof(D3DXQUATERNION);
	}

	for (uint32_t j = 0; j < dwHeightDataCount; ++j)
	{
		THeightData & rHeightData = m_HeightDataVector[j];
		memcpy(rHeightData.szName, c_pbBuf, 32);
		c_pbBuf += 32;

		uint32_t dwPrimitiveCount;
		memcpy(&dwPrimitiveCount, c_pbBuf, sizeof(uint32_t));
		c_pbBuf += sizeof(uint32_t);

		rHeightData.v3VertexVector.clear();
		rHeightData.v3VertexVector.resize(dwPrimitiveCount);
		memcpy(&rHeightData.v3VertexVector[0], c_pbBuf, dwPrimitiveCount*sizeof(D3DXVECTOR3));
		c_pbBuf += dwPrimitiveCount*sizeof(D3DXVECTOR3);

		// Getting Maximize Radius
		for (auto & k : rHeightData.v3VertexVector)
		{
			m_fMaximizeRadius = fMAX(m_fMaximizeRadius, fabs(k.x) + 50.0f);
			m_fMaximizeRadius = fMAX(m_fMaximizeRadius, fabs(k.y) + 50.0f);
			m_fMaximizeRadius = fMAX(m_fMaximizeRadius, fabs(k.z) + 50.0f);
		}
		// Getting Maximize Radius
	}

	return true;
}

void CAttributeData::OnClear()
{
	m_StaticCollisionDataVector.clear();
	m_HeightDataVector.clear();
}

bool CAttributeData::OnIsEmpty() const
{
	if (!m_StaticCollisionDataVector.empty())
		return false;
	if (!m_HeightDataVector.empty())
		return false;

	return true;
}

bool CAttributeData::OnIsType(TType type)
{
	if (CAttributeData::Type() == type)
		return true;

	return CResource::OnIsType(type);
}

CAttributeData::TType CAttributeData::Type()
{
	static TType s_type = StringToType("CAttributeData");
	return s_type;
}

void CAttributeData::OnSelfDestruct()
{
	Clear();
}

CAttributeData::CAttributeData(const FileSystem::CFileName& filename) : CResource(filename)
{
	m_fMaximizeRadius = 0.0f;
}

CAttributeData::~CAttributeData()
{
}
