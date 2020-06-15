#include "stdafx.h"
#include "config.h"
#include "utils.h"
#include "vector.h"
#include "char.h"
#include "sectree_manager.h"
#include "char_manager.h"
#include "mob_manager.h"
#include "pet_system.h"
#include "../../common/VnumHelper.h"
#include "packet.h"
#include "item_manager.h"
#include "item.h"

EVENTINFO(petsystem_event_info)
{
	CPetSystem* pPetSystem;
};

// PetSystem�� update ���ִ� event.
// PetSystem�� CHRACTER_MANAGER���� ���� FSM���� update ���ִ� ���� chracters�� �޸�,
// Owner�� STATE�� update �� �� _UpdateFollowAI �Լ��� update ���ش�.
// �׷��� owner�� state�� update�� CHRACTER_MANAGER���� ���ֱ� ������,
// petsystem�� update�ϴٰ� pet�� unsummon�ϴ� �κп��� ������ �����.
// (CHRACTER_MANAGER���� update �ϸ� chracter destroy�� pending�Ǿ�, CPetSystem������ dangling �����͸� ������ �ְ� �ȴ�.)
// ���� PetSystem�� ������Ʈ ���ִ� event�� �߻���Ŵ.
EVENTFUNC(petsystem_update_event)
{
	petsystem_event_info* info = dynamic_cast<petsystem_event_info*>( event->info );
	if ( info == nullptr )
	{
		sys_err("petsystem_update_event> <Factor> Null pointer");
		return 0;
	}

	CPetSystem*	pPetSystem = info->pPetSystem;

	if (nullptr == pPetSystem)
		return 0;


	pPetSystem->Update(0);
	// 0.25�ʸ��� ����.
	return PASSES_PER_SEC(1) / 4;
}

///////////////////////////////////////////////////////////////////////////////////////
//  CPetActor
///////////////////////////////////////////////////////////////////////////////////////

CPetActor::CPetActor(LPCHARACTER owner, uint32_t vnum, uint32_t options)
{
	m_dwVnum = vnum;
	m_dwVID = 0;
	m_dwOptions = options;
	m_dwLastActionTime = 0;

	m_pkChar = 0;
	m_pkOwner = owner;

	m_originalMoveSpeed = 0;

	m_dwSummonItemVID = 0;
	m_dwSummonItemVnum = 0;
}

CPetActor::~CPetActor()
{
	this->Unsummon();

	m_pkOwner = 0;
}

void CPetActor::SetName(const char* name)
{
	/*
	std::string petName = m_pkOwner->GetName();

	if (0 != m_pkOwner &&
		0 == name &&
		0 != m_pkOwner->GetName())
	{
		petName += "'s Pet";
	}
	else
		//petName += name;
		petName = name;
	*/

	if (IsSummoned())
		m_pkChar->SetName(name);

	m_name = name;
}

void CPetActor::Unsummon()
{
	if (!this->IsSummoned())
		return;

	if (m_dwSummonItemVnum)
		this->ClearBuff();

	this->SetSummonItem(nullptr);

	if (m_pkOwner)
		m_pkOwner->ComputePoints();

	if (m_pkChar)
		M2_DESTROY_CHARACTER(m_pkChar);

	m_pkChar = 0;
	m_dwVID = 0;
}

uint32_t CPetActor::Summon(const char* petName, LPITEM pSummonItem, bool bSpawnFar)
{
	int32_t x = m_pkOwner->GetX();
	int32_t y = m_pkOwner->GetY();
	int32_t z = m_pkOwner->GetZ();

	if (bSpawnFar)
	{
		x += (number(0, 1) * 2 - 1) * number(2000, 2500);
		y += (number(0, 1) * 2 - 1) * number(2000, 2500);
	}
	else
	{
		x += number(-100, 100);
		y += number(-100, 100);
	}

	if (m_pkChar)
	{
		m_pkChar->Show (m_pkOwner->GetMapIndex(), x, y);
		m_dwVID = m_pkChar->GetVID();

		return m_dwVID;
	}

	m_pkChar = CHARACTER_MANAGER::instance().SpawnMob(
				m_dwVnum,
				m_pkOwner->GetMapIndex(),
				x, y, z,
				false, (int32_t)(m_pkOwner->GetRotation()+180), false);

	if (0 == m_pkChar)
	{
		sys_err("[CPetSystem::Summon] Failed to summon the pet. (vnum: %d)", m_dwVnum);
		return 0;
	}

	m_pkChar->SetPet();

//	m_pkOwner->DetailLog();
//	m_pkChar->DetailLog();

	//���� ������ ������ ������ ������.
	m_pkChar->SetEmpire(m_pkOwner->GetEmpire());

	m_dwVID = m_pkChar->GetVID();

	this->SetName(petName);

	// SetSummonItem(pSummonItem)�� �θ� �Ŀ� ComputePoints�� �θ��� ���� �����.
	this->SetSummonItem(pSummonItem);
	m_pkOwner->ComputePoints();
	m_pkChar->Show(m_pkOwner->GetMapIndex(), x, y, z);

	return m_dwVID;
}

bool CPetActor::_UpdatAloneActionAI(float fMinDist, float fMaxDist)
{
	float fDist = number(fMinDist, fMaxDist);
	float r = (float)number (0, 359);
	float dest_x = GetOwner()->GetX() + fDist * cos(r);
	float dest_y = GetOwner()->GetY() + fDist * sin(r);

	//m_pkChar->SetRotation(number(0, 359));        // ������ �������� ����

	//GetDeltaByDegree(m_pkChar->GetRotation(), fDist, &fx, &fy);

	// ������ ���� �Ӽ� üũ; ���� ��ġ�� �߰� ��ġ�� �������ٸ� ���� �ʴ´�.
	//if (!(SECTREE_MANAGER::instance().IsMovablePosition(m_pkChar->GetMapIndex(), m_pkChar->GetX() + (int32_t) fx, m_pkChar->GetY() + (int32_t) fy)
	//			&& SECTREE_MANAGER::instance().IsMovablePosition(m_pkChar->GetMapIndex(), m_pkChar->GetX() + (int32_t) fx/2, m_pkChar->GetY() + (int32_t) fy/2)))
	//	return true;

	m_pkChar->SetNowWalking(true);

	//if (m_pkChar->Goto(m_pkChar->GetX() + (int32_t) fx, m_pkChar->GetY() + (int32_t) fy))
	//	m_pkChar->SendMovePacket(FUNC_WAIT, 0, 0, 0, 0);
	if (!m_pkChar->IsStateMove() && m_pkChar->Goto(dest_x, dest_y))
		m_pkChar->SendMovePacket(FUNC_WAIT, 0, 0, 0, 0);

	m_dwLastActionTime = get_dword_time();

	return true;
}

// char_state.cpp StateHorse�Լ� �׳� C&P -_-;
bool CPetActor::_UpdateFollowAI()
{
	if (0 == m_pkChar->m_pkMobData)
	{
		//sys_err("[CPetActor::_UpdateFollowAI] m_pkChar->m_pkMobData is nullptr");
		return false;
	}

	// NOTE: ĳ����(��)�� ���� �̵� �ӵ��� �˾ƾ� �ϴµ�, �ش� ��(m_pkChar->m_pkMobData->m_table.sMovingSpeed)�� ���������� �����ؼ� �˾Ƴ� ���� ������
	// m_pkChar->m_pkMobData ���� invalid�� ��찡 ���� �߻���. ���� �ð������ ������ ������ �ľ��ϰ� �ϴ��� m_pkChar->m_pkMobData ���� �ƿ� ������� �ʵ��� ��.
	// ���⼭ �Ź� �˻��ϴ� ������ ���� �ʱ�ȭ �� �� ���� ���� ����� �������� ��쵵 ����.. -_-;; �ФФФФФФФФ�
	if (0 == m_originalMoveSpeed)
	{
		const CMob* mobData = CMobManager::Instance().Get(m_dwVnum);

		if (0 != mobData)
			m_originalMoveSpeed = mobData->m_table.sMovingSpeed;
	}
	float	START_FOLLOW_DISTANCE = 300.0f;		// �� �Ÿ� �̻� �������� �Ѿư��� ������
	float	START_RUN_DISTANCE = 1000.0f;		// �� �Ÿ� �̻� �������� �پ �Ѿư�.

	float	RESPAWN_DISTANCE = 4500.f;			// �� �Ÿ� �̻� �־����� ���� ������ ��ȯ��.
	int32_t		APPROACH = 200;						// ���� �Ÿ�

	//bool bDoMoveAlone = true;					// ĳ���Ϳ� ������ ���� �� ȥ�� �������� �����ϰ��� ���� -_-;
	bool bRun = false;							// �پ�� �ϳ�?

	uint32_t currentTime = get_dword_time();

	int32_t ownerX = m_pkOwner->GetX();		int32_t ownerY = m_pkOwner->GetY();
	int32_t charX = m_pkChar->GetX();			int32_t charY = m_pkChar->GetY();

	float fDist = DISTANCE_APPROX(charX - ownerX, charY - ownerY);

	if (fDist >= RESPAWN_DISTANCE)
	{
		float fOwnerRot = m_pkOwner->GetRotation() * 3.141592f / 180.f;
		float fx = -APPROACH * cos(fOwnerRot);
		float fy = -APPROACH * sin(fOwnerRot);
		if (m_pkChar->Show(m_pkOwner->GetMapIndex(), ownerX + fx, ownerY + fy))
		{
			return true;
		}
	}


	if (fDist >= START_FOLLOW_DISTANCE)
	{
		if( fDist >= START_RUN_DISTANCE)
		{
			bRun = true;
		}

		m_pkChar->SetNowWalking(!bRun);		// NOTE: �Լ� �̸����� ���ߴ°��� �˾Ҵµ� SetNowWalking(false) �ϸ� �ٴ°���.. -_-;

		Follow(APPROACH);

		m_pkChar->SetLastAttacked(currentTime);
		m_dwLastActionTime = currentTime;
	}
	//else
	//{
	//	if (fabs(m_pkChar->GetRotation() - GetDegreeFromPositionXY(charX, charY, ownerX, ownerX)) > 10.f || fabs(m_pkChar->GetRotation() - GetDegreeFromPositionXY(charX, charY, ownerX, ownerX)) < 350.f)
	//	{
	//		m_pkChar->Follow(m_pkOwner, APPROACH);
	//		m_pkChar->SetLastAttacked(currentTime);
	//		m_dwLastActionTime = currentTime;
	//	}
	//}
	// Follow �������� ���ΰ� ���� �Ÿ� �̳��� ��������ٸ� ����
	else
		m_pkChar->SendMovePacket(FUNC_WAIT, 0, 0, 0, 0);
	//else if (currentTime - m_dwLastActionTime > number(5000, 12000))
	//{
	//	this->_UpdatAloneActionAI(START_FOLLOW_DISTANCE / 2, START_FOLLOW_DISTANCE);
	//}

	return true;
}

bool CPetActor::Update(uint32_t deltaTime)
{
	bool bResult = true;

	// �� ������ �׾��ų�, ��ȯ�� ���� ���°� �̻��ϴٸ� ���� ����. (NOTE: �������� �̷� ���� ������ ��ȯ�� ���� DEAD ���¿� ������ ��찡 ����-_-;)
	// ���� ��ȯ�� �������� ���ų�, ���� ���� ���°� �ƴ϶�� ���� ����.
	if (m_pkOwner->IsDead() || (IsSummoned() && m_pkChar->IsDead())
		|| nullptr == ITEM_MANAGER::instance().FindByVID(this->GetSummonItemVID())
		|| ITEM_MANAGER::instance().FindByVID(this->GetSummonItemVID())->GetOwner() != this->GetOwner()
		)
	{
		this->Unsummon();
		return true;
	}

	if (this->IsSummoned() && HasOption(EPetOption_Followable))
		bResult = bResult && this->_UpdateFollowAI();

	return bResult;
}

//NOTE : ����!!! MinDistance�� ũ�� ������ �� ������ŭ�� ��ȭ������ follow���� �ʴ´�,
bool CPetActor::Follow(float fMinDistance)
{
	// ������ ��ġ�� �ٶ���� �Ѵ�.
	if( !m_pkOwner || !m_pkChar)
		return false;

	float fOwnerX = m_pkOwner->GetX();
	float fOwnerY = m_pkOwner->GetY();

	float fPetX = m_pkChar->GetX();
	float fPetY = m_pkChar->GetY();

	float fDist = DISTANCE_SQRT(fOwnerX - fPetX, fOwnerY - fPetY);
	if (fDist <= fMinDistance)
		return false;

	m_pkChar->SetRotationToXY(fOwnerX, fOwnerY);

	float fx, fy;

	float fDistToGo = fDist - fMinDistance;
	GetDeltaByDegree(m_pkChar->GetRotation(), fDistToGo, &fx, &fy);

	if (!m_pkChar->Goto((int32_t)(fPetX+fx+0.5f), (int32_t)(fPetY+fy+0.5f)) )
		return false;

	m_pkChar->SendMovePacket(FUNC_WAIT, 0, 0, 0, 0, 0);

	return true;
}

void CPetActor::SetSummonItem (LPITEM pItem)
{
	if (nullptr == pItem)
	{
		m_dwSummonItemVID = 0;
		m_dwSummonItemVnum = 0;
		return;
	}

	m_dwSummonItemVID = pItem->GetVID();
	m_dwSummonItemVnum = pItem->GetVnum();
}

bool __PetCheckBuff(const CPetActor* pPetActor)
{
	bool bMustHaveBuff = true;
	switch (pPetActor->GetVnum())
	{
		case 34004:
		case 34009:
			if (nullptr == pPetActor->GetOwner()->GetDungeon())
				bMustHaveBuff = false;
		default:
			break;
	}
	return bMustHaveBuff;
}

void CPetActor::GiveBuff()
{
	// ��Ȳ �� ������ ���������� �߻���.
	if (!__PetCheckBuff(this))
		return;
	LPITEM item = ITEM_MANAGER::instance().FindByVID(m_dwSummonItemVID);
	if (nullptr != item)
		item->ModifyPoints(true);
	return ;
}

void CPetActor::ClearBuff()
{
	if (nullptr == m_pkOwner)
		return;

	TItemTable* item_proto = ITEM_MANAGER::instance().GetTable(m_dwSummonItemVnum);
	if (nullptr == item_proto)
		return;

	if (!__PetCheckBuff(this))
		return;

	for (int32_t i = 0; i < ITEM_APPLY_MAX_NUM; i++)
	{
		if (item_proto->aApplies[i].bType == APPLY_NONE)
			continue;
		m_pkOwner->ApplyPoint(item_proto->aApplies[i].bType, -item_proto->aApplies[i].lValue);
	}

	return ;
}

///////////////////////////////////////////////////////////////////////////////////////
//  CPetSystem
///////////////////////////////////////////////////////////////////////////////////////

CPetSystem::CPetSystem(LPCHARACTER owner)
{
//	assert(0 != owner && "[CPetSystem::CPetSystem] Invalid owner");

	m_pkOwner = owner;
	m_dwUpdatePeriod = 400;

	m_dwLastUpdateTime = 0;
}

CPetSystem::~CPetSystem()
{
	Destroy();
}

void CPetSystem::Destroy()
{
	for (TPetActorMap::iterator iter = m_petActorMap.begin(); iter != m_petActorMap.end(); ++iter)
	{
		CPetActor* petActor = iter->second;

		if (0 != petActor)
		{
			delete petActor;
		}
	}
	event_cancel(&m_pkPetSystemUpdateEvent);
	m_petActorMap.clear();
}

/// �� �ý��� ������Ʈ. ��ϵ� ����� AI ó�� ���� ��.
bool CPetSystem::Update(uint32_t deltaTime)
{
	bool bResult = true;

	uint32_t currentTime = get_dword_time();

	// CHARACTER_MANAGER���� ĳ���ͷ� Update�� �� �Ű������� �ִ� (Pulse��� �Ǿ��ִ�)���� ���� �����Ӱ��� �ð��������� �˾Ҵµ�
	// ���� �ٸ� ���̶�-_-; ���⿡ �Է����� ������ deltaTime�� �ǹ̰� �����Ф�

	if (m_dwUpdatePeriod > currentTime - m_dwLastUpdateTime)
		return true;

	std::vector <CPetActor*> v_garbageActor;

	for (TPetActorMap::iterator iter = m_petActorMap.begin(); iter != m_petActorMap.end(); ++iter)
	{
		CPetActor* petActor = iter->second;

		if (0 != petActor && petActor->IsSummoned())
		{
			LPCHARACTER pPet = petActor->GetCharacter();

			if (nullptr == CHARACTER_MANAGER::instance().Find(pPet->GetVID()))
			{
				v_garbageActor.push_back(petActor);
			}
			else
			{
				bResult = bResult && petActor->Update(deltaTime);
			}
		}
	}
	for (std::vector<CPetActor*>::iterator it = v_garbageActor.begin(); it != v_garbageActor.end(); it++)
		DeletePet(*it);

	m_dwLastUpdateTime = currentTime;

	return bResult;
}

/// ���� ��Ͽ��� ���� ����
void CPetSystem::DeletePet(uint32_t mobVnum)
{
	TPetActorMap::iterator iter = m_petActorMap.find(mobVnum);

	if (m_petActorMap.end() == iter)
	{
		sys_err("[CPetSystem::DeletePet] Can't find pet on my list (VNUM: %d)", mobVnum);
		return;
	}

	CPetActor* petActor = iter->second;

	if (!petActor)
		sys_err("[CPetSystem::DeletePet] Null Pointer (petActor)");
	else
		delete petActor;

	m_petActorMap.erase(iter);
}

/// ���� ��Ͽ��� ���� ����
void CPetSystem::DeletePet(CPetActor* petActor)
{
	for (TPetActorMap::iterator iter = m_petActorMap.begin(); iter != m_petActorMap.end(); ++iter)
	{
		if (iter->second == petActor)
		{
			delete petActor;
			m_petActorMap.erase(iter);

			return;
		}
	}

	sys_err("[CPetSystem::DeletePet] Can't find petActor(0x%x) on my list(size: %d) ", petActor, m_petActorMap.size());
}

void CPetSystem::Unsummon(uint32_t vnum, bool bDeleteFromList)
{
	CPetActor* actor = this->GetByVnum(vnum);

	if (0 == actor)
	{
		sys_err("[CPetSystem::GetByVnum(%d)] Null Pointer (petActor)", vnum);
		return;
	}
	actor->Unsummon();

	if (true == bDeleteFromList)
		this->DeletePet(actor);

	bool bActive = false;
	for (TPetActorMap::iterator it = m_petActorMap.begin(); it != m_petActorMap.end(); it++)
	{
		bActive |= it->second->IsSummoned();
	}
	if (false == bActive)
	{
		event_cancel(&m_pkPetSystemUpdateEvent);
		m_pkPetSystemUpdateEvent = nullptr;
	}
}


CPetActor* CPetSystem::Summon(uint32_t mobVnum, LPITEM pSummonItem, const char* petName, bool bSpawnFar, uint32_t options)
{
	CPetActor* petActor = this->GetByVnum(mobVnum);

	// ��ϵ� ���� �ƴ϶�� ���� ���� �� ���� ��Ͽ� �����.
	if (0 == petActor)
	{
		petActor = M2_NEW CPetActor(m_pkOwner, mobVnum, options);
		m_petActorMap.insert(std::make_pair(mobVnum, petActor));
	}

	uint32_t petVID = petActor->Summon(petName, pSummonItem, bSpawnFar);
	if (!petVID && m_pkOwner)
		sys_err("[CPetSystem::Summon(%d)] Null Pointer (petVID) Owner: %u", pSummonItem, m_pkOwner->GetPlayerID());
	else if (!petVID)
		sys_err("[CPetSystem::Summon(%d)] Null Pointer (petVID)", pSummonItem);

	if (nullptr == m_pkPetSystemUpdateEvent)
	{
		petsystem_event_info* info = AllocEventInfo<petsystem_event_info>();

		info->pPetSystem = this;

		m_pkPetSystemUpdateEvent = event_create(petsystem_update_event, info, PASSES_PER_SEC(1) / 4);	// 0.25��
	}

	return petActor;
}


CPetActor* CPetSystem::GetByVID(uint32_t vid) const
{
	CPetActor* petActor = 0;

	bool bFound = false;

	for (TPetActorMap::const_iterator iter = m_petActorMap.begin(); iter != m_petActorMap.end(); ++iter)
	{
		petActor = iter->second;

		if (!petActor)
		{
			sys_err("[CPetSystem::GetByVID(%d)] Null Pointer (petActor)", vid);
			continue;
		}

		bFound = petActor->GetVID() == vid;

		if (bFound)
			break;
	}

	return bFound ? petActor : 0;
}

/// ��� �� �� �߿��� �־��� �� VNUM�� ���� ���͸� ��ȯ�ϴ� �Լ�.
CPetActor* CPetSystem::GetByVnum(uint32_t vnum) const
{
	CPetActor* petActor = 0;

	TPetActorMap::const_iterator iter = m_petActorMap.find(vnum);

	if (iter != m_petActorMap.end())
		petActor = iter->second;

	return petActor;
}

size_t CPetSystem::CountSummoned() const
{
	size_t count = 0;

	for (TPetActorMap::const_iterator iter = m_petActorMap.begin(); iter != m_petActorMap.end(); ++iter)
	{
		CPetActor* petActor = iter->second;

		if (petActor)
		{
			if (petActor->IsSummoned())
				++count;
		}
	}

	return count;
}

void CPetSystem::RefreshBuff()
{
	for (TPetActorMap::const_iterator iter = m_petActorMap.begin(); iter != m_petActorMap.end(); ++iter)
	{
		CPetActor* petActor = iter->second;

		if (petActor)
		{
			if (petActor->IsSummoned())
			{
				petActor->GiveBuff();
			}
		}
	}
}