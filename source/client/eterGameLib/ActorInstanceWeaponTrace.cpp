#include "StdAfx.h"
#include "ActorInstance.h"
#include "WeaponTrace.h"

void CActorInstance::TraceProcess()
{
	if (!m_WeaponTraceVector.empty())
	{
		for (const auto & weaponTrace : m_WeaponTraceVector) {
			weaponTrace->SetPosition(m_x, m_y, m_z);
			weaponTrace->SetRotation(m_fcurRotation);
			weaponTrace->Update(__GetReachScale());
		}
	}
}

void CActorInstance::RenderTrace()
{
	for (const auto & weaponTrace : m_WeaponTraceVector)
		weaponTrace->Render();
}

void CActorInstance::__DestroyWeaponTrace()
{
	for (const auto & weaponTrace : m_WeaponTraceVector)
		weaponTrace->Delete(weaponTrace);

	m_WeaponTraceVector.clear();
}

void CActorInstance::__ShowWeaponTrace()
{
	for (const auto & weaponTrace : m_WeaponTraceVector)
		weaponTrace->TurnOn();
}

void CActorInstance::__HideWeaponTrace()
{
	for (const auto & weaponTrace : m_WeaponTraceVector)
		weaponTrace->TurnOff();
}

