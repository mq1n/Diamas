#include "StdAfx.h"
#include "ActorInstance.h"
#include "RaceData.h"



void CActorInstance::__Push(int32_t x, int32_t y)
{
	//uint32_t dwVID=GetVirtualID();
	//Tracenf("VID %d SyncPixelPosition %d %d", dwVID, x, y);		

	const D3DXVECTOR3& c_rv3Src=GetPosition();
	const D3DXVECTOR3 c_v3Dst=D3DXVECTOR3(x, -y, c_rv3Src.z);
	const D3DXVECTOR3 c_v3Delta=c_v3Dst-c_rv3Src;
	
	const int32_t LoopValue = 100;
	const D3DXVECTOR3 inc=c_v3Delta / LoopValue;
	
	D3DXVECTOR3 v3Movement(0.0f, 0.0f, 0.0f);

	IPhysicsWorld* pWorld = IPhysicsWorld::GetPhysicsWorld();
			
	if (!pWorld)
	{
		return;
	}

	for(int32_t i = 0; i < LoopValue; ++i)
	{
		if (pWorld->isPhysicalCollision(c_rv3Src + v3Movement))
		{
			ResetBlendingPosition();
			return;
		}
		v3Movement += inc;
	}

	SetBlendingPosition(c_v3Dst);

	if (IsResistFallen() || IsPushing())
		return;

	if (!IsUsingSkill())
	{
		int32_t len=sqrt(c_v3Delta.x*c_v3Delta.x+c_v3Delta.y*c_v3Delta.y);
		if (len>150.0f)
		{
			InterceptOnceMotion(CRaceMotionData::NAME_DAMAGE_FLYING);
			PushOnceMotion(CRaceMotionData::NAME_STAND_UP);
		}
	}
}

void CActorInstance::TEMP_Push(int32_t x, int32_t y)
{
	__Push(x, y);
}

bool CActorInstance::__IsSyncing()
{
	if (IsDead())
		return TRUE;

	if (IsStun())
		return TRUE;

	if (IsPushing())
		return TRUE;

	return FALSE;
}

bool CActorInstance::IsPushing()
{
	return m_PhysicsObject.isBlending();
}