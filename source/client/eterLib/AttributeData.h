#pragma once

#include "Resource.h"
#include "Ref.h"
#include "CollisionData.h"

typedef struct SHeightData
{
	char szName[32+1];
	std::vector<D3DXVECTOR3> v3VertexVector;
} THeightData;

typedef std::vector<THeightData> THeightDataVector;

class CAttributeData : public CResource
{
	public:
		typedef CRef<CAttributeData> TRef;

		/*
		enum ECollisionType
		{
			COLLISION_TYPE_PLANE,
			COLLISION_TYPE_BOX,
			COLLISION_TYPE_SPHERE,
			COLLISION_TYPE_CYLINDER,
		};

		typedef struct SCollisionData
		{
			uint32_t dwType;
			char szName[32+1];

			D3DXVECTOR3 v3Position;
			float fDimensions[3];
			D3DXQUATERNION quatRotation;
		} TCollisionData;*/

	public:
		static TType Type();

	public:
		CAttributeData(const FileSystem::CFileName& filename);
		virtual ~CAttributeData();

		//uint32_t GetCollisionDataCount() const;
		//BOOL GetCollisionDataPointer(uint32_t dwIndex, const TCollisionData ** c_ppCollisionData) const;
		const CStaticCollisionDataVector & GetCollisionDataVector() const;
		const THeightDataVector & GetHeightDataVector() const;

		size_t AddCollisionData(const CStaticCollisionData& collisionData);	// return m_StaticCollisionDataVector.size();

		uint32_t GetHeightDataCount() const;
		BOOL GetHeightDataPointer(uint32_t dwIndex, const THeightData ** c_ppHeightData) const;

		float GetMaximizeRadius();

	protected:
		bool OnLoad(int32_t iSize, const void * c_pvBuf);
		void OnClear();
		bool OnIsEmpty() const;
		bool OnIsType(TType type);
		void OnSelfDestruct();

	protected:
		float m_fMaximizeRadius;

		//std::vector<TCollisionData> m_CollisionDataVector;
		CStaticCollisionDataVector m_StaticCollisionDataVector;
		THeightDataVector m_HeightDataVector;
};
