#include "stdafx.h"
#include "utils.h"
#include "config.h"
#include "char.h"
#include "sectree_manager.h"
#include "battle.h"
#include "affect.h"
#include "shop_manager.h"
#include "item.h"
#include "Battleground.h"

int32_t	OnClickShop(TRIGGERPARAM);
int32_t	OnClickTalk(TRIGGERPARAM);

int32_t	OnIdleDefault(TRIGGERPARAM);
int32_t	OnAttackDefault(TRIGGERPARAM);

typedef struct STriggerFunction
{
	int32_t (*func) (TRIGGERPARAM);
} TTriggerFunction;

TTriggerFunction OnClickTriggers[ON_CLICK_MAX_NUM] =
{
	{ nullptr,          	},	// ON_CLICK_NONE,
	{ OnClickShop,	},	// ON_CLICK_SHOP,
};

void CHARACTER::AssignTriggers(const TMobTable * table)
{
	if (table->bOnClickType >= ON_CLICK_MAX_NUM)
	{
		sys_err("%s has invalid OnClick value %d", GetName(), table->bOnClickType);
		abort();
	}

	m_triggerOnClick.bType = table->bOnClickType;
	m_triggerOnClick.pFunc = OnClickTriggers[table->bOnClickType].func;
}

/*
 * ON_CLICK
 */
int32_t OnClickShop(TRIGGERPARAM)
{
	CShopManager::instance().StartShopping(causer, ch);
	return 1;
}

/*
 * 몬스터 AI 함수들을 BattleAI 클래스로 수정
 */
int32_t OnIdleDefault(TRIGGERPARAM)
{
	if (ch->OnIdle())
		return PASSES_PER_SEC(1);

	return PASSES_PER_SEC(1);
}

class FuncFindMobVictim
{
	public:
		FuncFindMobVictim(LPCHARACTER pkChr, int32_t iMaxDistance) :
			m_pkChr(pkChr),
			m_iMinDistance(~(1 << 31)),
			m_iMaxDistance(iMaxDistance),
			m_lx(pkChr->GetX()),
			m_ly(pkChr->GetY()),
			m_pkChrVictim(nullptr),
			m_pkChrBuilding(nullptr)
	{
	};

		bool operator () (LPENTITY ent)
		{
			if (!ent->IsType(ENTITY_CHARACTER))
				return false;

			LPCHARACTER pkChr = (LPCHARACTER) ent;

			if (pkChr->IsBuilding() && 
				(pkChr->IsAffectFlag(AFF_BUILDING_CONSTRUCTION_SMALL) ||
				 pkChr->IsAffectFlag(AFF_BUILDING_CONSTRUCTION_LARGE) ||
				 pkChr->IsAffectFlag(AFF_BUILDING_UPGRADE)))
			{
				m_pkChrBuilding = pkChr;
			}

			if (pkChr->IsNPC())
			{
				if ( !pkChr->IsMonster() || !m_pkChr->IsAttackMob() || m_pkChr->IsAggressive()  )
					return false;
					
			}

			if (pkChr->IsDead())
				return false;

			if (pkChr->IsAffectFlag(AFF_EUNHYUNG) || 
					pkChr->IsAffectFlag(AFF_INVISIBILITY) ||
					pkChr->IsAffectFlag(AFF_REVIVE_INVISIBLE))
				return false;

			if (pkChr->IsAffectFlag(AFF_TERROR) && m_pkChr->IsImmune(IMMUNE_TERROR) == false )	// 공포 처리
			{
				if ( pkChr->GetLevel() >= m_pkChr->GetLevel() )
					return false;
			}

		 	if ( m_pkChr->IsNoAttackShinsu() )
			{
				if ( pkChr->GetEmpire() == 1 )
					return false;
			}

			if ( m_pkChr->IsNoAttackChunjo() )
			{
				if ( pkChr->GetEmpire() == 2 )
					return false;
			}
			

			if ( m_pkChr->IsNoAttackJinno() )
			{
				if ( pkChr->GetEmpire() == 3 )
					return false;
			}

			int32_t iDistance = DISTANCE_APPROX(m_lx - pkChr->GetX(), m_ly - pkChr->GetY());

			if (iDistance < m_iMinDistance && iDistance <= m_iMaxDistance)
			{
				m_pkChrVictim = pkChr;
				m_iMinDistance = iDistance;
			}
			return true;
		}

		LPCHARACTER GetVictim()
		{
			// 근처에 건물이 있고 피가 많이 있다면 건물을 공격한다. 건물만 있어도 건물을 공격
			if ((m_pkChrBuilding && ((m_pkChr->GetHP() * 2) > m_pkChr->GetMaxHP())) || !m_pkChrVictim)
			{
				return m_pkChrBuilding;
			}

			return (m_pkChrVictim);
		}

	private:
		LPCHARACTER	m_pkChr;

		int32_t		m_iMinDistance;
		int32_t		m_iMaxDistance;
		int32_t		m_lx;
		int32_t		m_ly;

		LPCHARACTER	m_pkChrVictim;
		LPCHARACTER	m_pkChrBuilding;
};

LPCHARACTER FindMobVictim(LPCHARACTER pkChr, int32_t iMaxDistance)
{
	FuncFindMobVictim f(pkChr, iMaxDistance);
	if (pkChr->GetSectree() != nullptr) {
		pkChr->GetSectree()->ForEachAround(f);
	}
	return f.GetVictim();
}

class FuncFindBattlegroundMobVictim
{
	public:
		FuncFindBattlegroundMobVictim(LPCHARACTER pkChr, int32_t iMaxDistance) :
			m_pkChr(pkChr),
			m_iMinDistance(~(1 << 31)),
			m_iMaxDistance(iMaxDistance),
			m_lx(pkChr->GetX()),
			m_ly(pkChr->GetY()),
			m_pkChrVictim(nullptr)
		{
		};

		bool operator() (LPENTITY ent)
		{
			if (!ent->IsType(ENTITY_CHARACTER))
				return false;

			LPCHARACTER pkChr = (LPCHARACTER) ent;

			if (pkChr->IsBuilding())
				return false;

			if (pkChr->IsDead())
				return false;

			auto nMyTeamID = m_pkChr->GetBattlegroundTeamID();
			auto nCurrTeamID = pkChr->GetBattlegroundTeamID();

			if (nCurrTeamID == nMyTeamID)
				return false;			

			if (pkChr->IsNPC())
			{
				if (!pkChr->IsMonster())
					return false;
			}
			else if (pkChr->IsPC())
			{
				if (pkChr->IsAffectFlag(AFF_EUNHYUNG) || pkChr->IsAffectFlag(AFF_INVISIBILITY) || pkChr->IsAffectFlag(AFF_REVIVE_INVISIBLE))
					return false;

				if (pkChr->IsAffectFlag(AFF_TERROR) && m_pkChr->IsImmune(IMMUNE_TERROR) == false )	// 공포 처리
				{
					if ( pkChr->GetLevel() >= m_pkChr->GetLevel() )
						return false;
				}							
			}

			int32_t iDistance = DISTANCE_APPROX(m_lx - pkChr->GetX(), m_ly - pkChr->GetY());

			if ( /*iDistance < m_iMinDistance &&  */ iDistance <= m_iMaxDistance)
			{
				m_pkChrVictim = pkChr;
				m_iMinDistance = iDistance;
			}
			return true;
		}

		LPCHARACTER GetVictim()
		{
			return m_pkChrVictim;
		}

	private:
		LPCHARACTER	m_pkChr;

		int32_t		m_iMinDistance;
		int32_t		m_iMaxDistance;
		int32_t		m_lx;
		int32_t		m_ly;

		LPCHARACTER	m_pkChrVictim;
};

LPCHARACTER FindBattlegroundMobVictim(LPCHARACTER pkChr)
{
	FuncFindBattlegroundMobVictim f(pkChr, 2000);
	if (pkChr->GetSectree() != nullptr) {
		pkChr->GetSectree()->ForEachAround(f);
	}
	return f.GetVictim();
}


class FuncFindItemVictim
{
public:
	FuncFindItemVictim(LPCHARACTER pkChr, int32_t iMaxDistance, bool bNeedOwner = false) :
		m_pkChr(pkChr),
		m_iMinItemDistance(~(1L << 31)),
		m_iMinGoldDistance(~(1L << 31)),
		m_iMaxDistance(iMaxDistance),
		m_lx(pkChr->GetX()),
		m_ly(pkChr->GetY()),
		m_bNeedOwner(bNeedOwner),
		m_pkItemVictim(nullptr),
		m_pkGoldVictim(nullptr)
	{
	};

	bool operator () (LPENTITY ent)
	{
		if (!ent->IsType(ENTITY_ITEM))
			return false;

		LPITEM pkItem = (LPITEM)ent;

		if (pkItem->GetWindow() != GROUND)
			return false;

		if (m_bNeedOwner && !pkItem->GetOwner())
			return false;

		if (pkItem->GetOwner()->GetPlayerID() != m_pkChr->GetPlayerID())
			return false;

		int32_t iDistance = DISTANCE_APPROX(m_lx - pkItem->GetX(), m_ly - pkItem->GetY());

		int32_t* iMinDistance;
		LPITEM* pkItemVictim;

		if (pkItem->GetType() == ITEM_ELK)
		{
			iMinDistance = &m_iMinGoldDistance;
			pkItemVictim = &m_pkGoldVictim;
		}
		else
		{
			iMinDistance = &m_iMinItemDistance;
			pkItemVictim = &m_pkItemVictim;
		}

		if (iDistance < *iMinDistance && iDistance <= m_iMaxDistance)
		{
			*iMinDistance = iDistance;
			*pkItemVictim = pkItem;
		}

		return true;
	}

	LPITEM GetVictim()
	{
		return (m_pkItemVictim ? m_pkItemVictim : m_pkGoldVictim);
	}

private:
	LPCHARACTER	m_pkChr;

	int32_t		m_iMinItemDistance;
	int32_t		m_iMinGoldDistance;
	int32_t		m_iMaxDistance;
	int32_t		m_lx;
	int32_t		m_ly;

	bool	m_bNeedOwner;

	LPITEM	m_pkItemVictim;
	LPITEM	m_pkGoldVictim;
};

LPITEM FindItemVictim(LPCHARACTER pkChr, int32_t iMaxDistance, int32_t bNeedOwner)
{
	FuncFindItemVictim f(pkChr, iMaxDistance, bNeedOwner);
	if (pkChr->GetSectree())
		pkChr->GetSectree()->ForEachAround(f);

	return f.GetVictim();
}

LPITEM FindItemVictim(LPCHARACTER pkChr, int32_t iMaxDistance)
{
	FuncFindItemVictim f(pkChr, iMaxDistance);
	if (pkChr->GetSectree()) 
		pkChr->GetSectree()->ForEachAround(f);

	return f.GetVictim();
}
