#include "stdafx.h"
#include "constants.h"
#include "config.h"
#include "char.h"
#include "char_manager.h"
#include "affect.h"
#include "locale_service.h"

// 독
const int32_t poison_damage_rate[MOB_RANK_MAX_NUM] = 
{
	80, 50, 40, 30, 25, 1
};

int32_t GetPoisonDamageRate(LPCHARACTER ch)
{
	int32_t iRate;

	if (ch->IsPC())
		iRate = 50;
	else
		iRate = poison_damage_rate[ch->GetMobRank()];

	iRate = MAX(0, iRate - ch->GetPoint(POINT_POISON_REDUCE));
	return iRate;
}

EVENTINFO(TPoisonEventInfo)
{
	DynamicCharacterPtr ch;
	int32_t		count;
	uint32_t	attacker_pid;

	TPoisonEventInfo()
	: ch()
	, count(0)
	, attacker_pid(0)
	{
	}
};

EVENTFUNC(poison_event)
{
	TPoisonEventInfo * info = dynamic_cast<TPoisonEventInfo *>( event->info );
	
	if ( info == nullptr )
	{
		sys_err( "poison_event> <Factor> Null pointer" );
		return 0;
	}

	LPCHARACTER ch = info->ch;

	if (ch == nullptr) { // <Factor>
		return 0;
	}
	LPCHARACTER pkAttacker = CHARACTER_MANAGER::instance().FindByPID(info->attacker_pid);

	int32_t dam = ch->GetMaxHP() * GetPoisonDamageRate(ch) / 1000;
	if (g_bIsTestServer) ch->ChatPacket(CHAT_TYPE_NOTICE, "Poison Damage %d", dam);

	if (ch->Damage(pkAttacker, dam, DAMAGE_TYPE_POISON))
	{
		ch->m_pkPoisonEvent = nullptr;
		return 0;
	}

	--info->count;

	if (info->count)
		return PASSES_PER_SEC(3);
	else
	{
		ch->m_pkPoisonEvent = nullptr;
		return 0;
	}
}

#ifdef ENABLE_WOLFMAN_CHARACTER
const int32_t bleeding_damage_rate[MOB_RANK_MAX_NUM] =
{
	80, 50, 40, 30, 25, 1
};

int32_t GetBleedingDamageRate(LPCHARACTER ch)
{
	int32_t iRate;

	if (ch->IsPC())
		iRate = 50;
	else
		iRate = bleeding_damage_rate[ch->GetMobRank()];

	iRate = MAX(0, iRate - ch->GetPoint(POINT_BLEEDING_REDUCE));
#if defined(ENABLE_WOLFMAN_CHARACTER) && defined(USE_ITEM_BLEEDING_AS_POISON)
	iRate = MAX(0, iRate - ch->GetPoint(POINT_POISON_REDUCE));
#endif
	return iRate;
}

EVENTINFO(TBleedingEventInfo)
{
	DynamicCharacterPtr ch;
	int32_t		count;
	uint32_t	attacker_pid;

	TBleedingEventInfo()
	: ch()
	, count(0)
	, attacker_pid(0)
	{
	}
};

EVENTFUNC(bleeding_event)
{
	TBleedingEventInfo * info = dynamic_cast<TBleedingEventInfo *>( event->info );

	if ( info == nullptr )
	{
		sys_err( "bleeding_event> <Factor> Null pointer" );
		return 0;
	}

	LPCHARACTER ch = info->ch;

	if (ch == nullptr) { // <Factor>
		return 0;
	}
	LPCHARACTER pkAttacker = CHARACTER_MANAGER::instance().FindByPID(info->attacker_pid);

	int32_t dam = ch->GetMaxHP() * GetBleedingDamageRate(ch) / 1000;
	if (g_bIsTestServer) ch->ChatPacket(CHAT_TYPE_NOTICE, "Bleeding Damage %d", dam);

	if (ch->Damage(pkAttacker, dam, DAMAGE_TYPE_BLEEDING))
	{
		ch->m_pkBleedingEvent = nullptr;
		return 0;
	}

	--info->count;

	if (info->count)
		return PASSES_PER_SEC(3);
	else
	{
		ch->m_pkBleedingEvent = nullptr;
		return 0;
	}
}
#endif

EVENTINFO(TFireEventInfo)
{
	DynamicCharacterPtr ch;
	int32_t		count;
	int32_t		amount;
	uint32_t	attacker_pid;

	TFireEventInfo()
	: ch()
	, count(0)
	, amount(0)
	, attacker_pid(0)
	{
	}
};

EVENTFUNC(fire_event)
{
	TFireEventInfo * info = dynamic_cast<TFireEventInfo *>( event->info );

	if ( info == nullptr )
	{
		sys_err( "fire_event> <Factor> Null pointer" );
		return 0;
	}

	LPCHARACTER ch = info->ch;
	if (ch == nullptr) { // <Factor>
		return 0;
	}
	LPCHARACTER pkAttacker = CHARACTER_MANAGER::instance().FindByPID(info->attacker_pid);

	int32_t dam = info->amount;
	if (g_bIsTestServer) ch->ChatPacket(CHAT_TYPE_NOTICE, "Fire Damage %d", dam);

	if (ch->Damage(pkAttacker, dam, DAMAGE_TYPE_FIRE))
	{
		ch->m_pkFireEvent = nullptr;
		return 0;
	}

	--info->count;

	if (info->count)
		return PASSES_PER_SEC(3);
	else
	{
		ch->m_pkFireEvent = nullptr;
		return 0;
	}
}

/*

   LEVEL에 의한..

   +8   0%
   +7   5%
   +6  10%
   +5  30%
   +4  50%
   +3  70%
   +2  80%
   +1  90%
   +0 100%
   -1 100%
   -2 100%
   -3 100%
   -4 100%
   -5 100%
   -6 100%
   -7 100%
   -8 100%

 */

static int32_t poison_level_adjust[9] =
{
	100, 90, 80, 70, 50, 30, 10, 5, 0
};

#ifdef ENABLE_WOLFMAN_CHARACTER
static int32_t bleeding_level_adjust[9] =
{
	100, 90, 80, 70, 50, 30, 10, 5, 0
};
#endif

void CHARACTER::AttackedByFire(LPCHARACTER pkAttacker, int32_t amount, int32_t count)
{
	if (m_pkFireEvent)
		return;

	AddAffect(AFFECT_FIRE, POINT_NONE, 0, AFF_FIRE, count*3+1, 0, true);

	TFireEventInfo* info = AllocEventInfo<TFireEventInfo>();

	info->ch = this;
	info->count = count;
	info->amount = amount;
	info->attacker_pid = pkAttacker->GetPlayerID();

	m_pkFireEvent = event_create(fire_event, info, 1);
}

void CHARACTER::AttackedByPoison(LPCHARACTER pkAttacker)
{
	if (m_pkPoisonEvent)
		return;

	if (m_bHasPoisoned && !IsPC()) // 몬스터는 독이 한번만 걸린다.
		return;

#ifdef ENABLE_WOLFMAN_CHARACTER
	if (m_pkBleedingEvent)
		return;

	if (m_bHasBled && !IsPC()) 
		return;
#endif

	if (pkAttacker && pkAttacker->GetLevel() < GetLevel())
	{
		int32_t delta = GetLevel() - pkAttacker->GetLevel();

		if (delta > 8)
			delta = 8;

		if (number(1, 100) > poison_level_adjust[delta])
			return;
	}

	/*if (IsImmune(IMMUNE_POISON))
	  return;*/

	// 독 내성 굴림 실패, 독에 걸렸다!
	m_bHasPoisoned = true;

	AddAffect(AFFECT_POISON, POINT_NONE, 0, AFF_POISON, POISON_LENGTH + 1, 0, true);

	TPoisonEventInfo* info = AllocEventInfo<TPoisonEventInfo>();

	info->ch = this;
	info->count = 10;
	info->attacker_pid = pkAttacker?pkAttacker->GetPlayerID():0;

	m_pkPoisonEvent = event_create(poison_event, info, 1);

	if (g_bIsTestServer && pkAttacker)
	{
		char buf[256];
		snprintf(buf, sizeof(buf), "POISON %s -> %s", pkAttacker->GetName(), GetName());
		pkAttacker->ChatPacket(CHAT_TYPE_INFO, "%s", buf);
	}
}

#ifdef ENABLE_WOLFMAN_CHARACTER
void CHARACTER::AttackedByBleeding(LPCHARACTER pkAttacker)
{
	if (m_pkBleedingEvent)
		return;

	if (m_bHasBled && !IsPC()) 
		return;

	if (m_pkPoisonEvent)
		return;

	if (m_bHasPoisoned && !IsPC()) 
		return;

	if (pkAttacker && pkAttacker->GetLevel() < GetLevel())
	{
		int32_t delta = GetLevel() - pkAttacker->GetLevel();

		if (delta > 8)
			delta = 8;

		if (number(1, 100) > bleeding_level_adjust[delta])
			return;
	}

	/*if (IsImmune(IMMUNE_BLEEDING))
	  return;*/

	
	m_bHasBled = true;

	AddAffect(AFFECT_BLEEDING, POINT_NONE, 0, AFF_BLEEDING, BLEEDING_LENGTH + 1, 0, true);

	TBleedingEventInfo* info = AllocEventInfo<TBleedingEventInfo>();

	info->ch = this;
	info->count = 10;
	info->attacker_pid = pkAttacker?pkAttacker->GetPlayerID():0;

	m_pkBleedingEvent = event_create(bleeding_event, info, 1);

	if (g_bIsTestServer && pkAttacker)
	{
		char buf[256];
		snprintf(buf, sizeof(buf), "BLEEDING %s -> %s", pkAttacker->GetName(), GetName());
		pkAttacker->ChatPacket(CHAT_TYPE_INFO, "%s", buf);
	}
}
#endif
void CHARACTER::RemoveFire()
{
	RemoveAffect(AFFECT_FIRE);
	event_cancel(&m_pkFireEvent);
}

void CHARACTER::RemovePoison()
{
	RemoveAffect(AFFECT_POISON);
	event_cancel(&m_pkPoisonEvent);
}

#ifdef ENABLE_WOLFMAN_CHARACTER
void CHARACTER::RemoveBleeding()
{
	RemoveAffect(AFFECT_BLEEDING);
	event_cancel(&m_pkBleedingEvent);
}
#endif

void CHARACTER::ApplyMobAttribute(const TMobTable* table)
{
	for (int32_t i = 0; i < MOB_ENCHANTS_MAX_NUM; ++i)
	{
		if (table->cEnchants[i] != 0)
			ApplyPoint(aiMobEnchantApplyIdx[i], table->cEnchants[i]);
	}
#if defined(ENABLE_WOLFMAN_CHARACTER) && defined(USE_MOB_BLEEDING_AS_POISON)
	if (table->cEnchants[MOB_ENCHANT_POISON] != 0)
		ApplyPoint(APPLY_BLEEDING_PCT, table->cEnchants[MOB_ENCHANT_POISON]/50);
#endif

	for (int32_t i = 0; i < MOB_RESISTS_MAX_NUM; ++i)
	{
		if (table->cResists[i] != 0)
			ApplyPoint(aiMobResistsApplyIdx[i], table->cResists[i]);
	}
#if defined(ENABLE_WOLFMAN_CHARACTER) && defined(USE_MOB_CLAW_AS_DAGGER)
	if (table->cResists[MOB_RESIST_DAGGER] != 0)
		ApplyPoint(APPLY_RESIST_CLAW, table->cResists[MOB_RESIST_DAGGER]);
#endif
#if defined(ENABLE_WOLFMAN_CHARACTER) && defined(USE_MOB_BLEEDING_AS_POISON)
	if (table->cResists[MOB_RESIST_POISON] != 0)
		ApplyPoint(APPLY_BLEEDING_REDUCE, table->cResists[MOB_RESIST_POISON]);
#endif
}

// #define ENABLE_IMMUNE_PERC
bool CHARACTER::IsImmune(uint32_t dwImmuneFlag)
{
	// 1 stun, 2 slow, 4 fall = 7 all == X
	// ChatPacket(CHAT_TYPE_PARTY, "<IMMUNE_IS> (%u == %u)", m_pointsInstant.dwImmuneFlag, dwImmuneFlag);
	if (IS_SET(m_pointsInstant.dwImmuneFlag, dwImmuneFlag))
	{
		int32_t immune_pct = 90;
		int32_t	percent = number(1, 100);

		if (percent <= immune_pct)	// 90% Immune
		{
			if (g_bIsTestServer && IsPC())
				ChatPacket(CHAT_TYPE_PARTY, "<IMMUNE_SUCCESS> (%s)", GetName()); 

			return true;
		}
		else
		{
			if (g_bIsTestServer && IsPC())
				ChatPacket(CHAT_TYPE_PARTY, "<IMMUNE_FAIL> (%s)", GetName());

			return false;
		}
	}

	if (g_bIsTestServer && IsPC())
		ChatPacket(CHAT_TYPE_PARTY, "<IMMUNE_FAIL> (%s) NO_IMMUNE_FLAG", GetName());

	return false;
}

