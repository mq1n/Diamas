
#include "stdafx.h"
#include "config.h"

#include "BlueDragon.h"

#include "vector.h"
#include "utils.h"
#include "char.h"
#include "mob_manager.h"
#include "sectree_manager.h"
#include "battle.h"
#include "affect.h"
#include "BlueDragon_Binder.h"
#include "BlueDragon_Skill.h"
#include "packet.h"
#include "motion.h"

time_t UseBlueDragonSkill(LPCHARACTER pChar, uint32_t idx)
{
	LPSECTREE_MAP pSecMap = SECTREE_MANAGER::instance().GetMap( pChar->GetMapIndex() );

	if (nullptr == pSecMap)
		return 0;

	int32_t nextUsingTime = 0;

	switch (idx)
	{
		case 0:
			{
				sys_log(0, "BlueDragon: Using Skill Breath");

				FSkillBreath f(pChar);

				pSecMap->for_each( f );

				nextUsingTime = number(BlueDragon_GetSkillFactor(3, "Skill0", "period", "min"), BlueDragon_GetSkillFactor(3, "Skill0", "period", "max"));
			}
			break;

		case 1:
			{
				sys_log(0, "BlueDragon: Using Skill Weak Breath");

				FSkillWeakBreath f(pChar);

				pSecMap->for_each( f );

				nextUsingTime = number(BlueDragon_GetSkillFactor(3, "Skill1", "period", "min"), BlueDragon_GetSkillFactor(3, "Skill1", "period", "max"));
			}
			break;

		case 2:
			{
				sys_log(0, "BlueDragon: Using Skill EarthQuake");

				FSkillEarthQuake f(pChar);

				pSecMap->for_each( f );

				nextUsingTime = number(BlueDragon_GetSkillFactor(3, "Skill2", "period", "min"), BlueDragon_GetSkillFactor(3, "Skill2", "period", "max"));

				if (nullptr != f.pFarthestChar)
				{
					pChar->BeginFight( f.pFarthestChar );
				}
			}
			break;

		default:
			sys_err("BlueDragon: Wrong Skill Index: %d", idx);
			return 0;
	}

	int32_t addPct = BlueDragon_GetRangeFactor("hp_period", pChar->GetHPPct());

	nextUsingTime += (nextUsingTime * addPct) / 100;

	return nextUsingTime;
}

int32_t BlueDragon_StateBattle(LPCHARACTER pChar)
{
	if (pChar->GetHPPct() > 98)
		return PASSES_PER_SEC(1);

	const int32_t SkillCount = 3;
	int32_t SkillPriority[SkillCount];
	static time_t timeSkillCanUseTime[SkillCount];

	if (pChar->GetHPPct() > 76)
	{
		SkillPriority[0] = 1;
		SkillPriority[1] = 0;
		SkillPriority[2] = 2;
	}
	else if (pChar->GetHPPct() > 31)
	{
		SkillPriority[0] = 0;
		SkillPriority[1] = 1;
		SkillPriority[2] = 2;
	}
	else
	{
		SkillPriority[0] = 0;
		SkillPriority[1] = 2;
		SkillPriority[2] = 1;
	}

	time_t timeNow = static_cast<time_t>(get_dword_time());

	for (int32_t i=0 ; i < SkillCount ; ++i)
	{
		const int32_t SkillIndex = SkillPriority[i];

		if (timeSkillCanUseTime[SkillIndex] < timeNow)
		{
			int32_t SkillUsingDuration =
				static_cast<int32_t>(CMotionManager::instance().GetMotionDuration( pChar->GetRaceNum(), MAKE_MOTION_KEY(MOTION_MODE_GENERAL, MOTION_SPECIAL_1 + SkillIndex) ));

			timeSkillCanUseTime[SkillIndex] = timeNow + (UseBlueDragonSkill( pChar, SkillIndex ) * 1000) + SkillUsingDuration + 3000;

			pChar->SendMovePacket(FUNC_MOB_SKILL, SkillIndex, pChar->GetX(), pChar->GetY(), 0, timeNow);

			return 0 == SkillUsingDuration ? PASSES_PER_SEC(1) : PASSES_PER_SEC(SkillUsingDuration);
		}
	}

	return PASSES_PER_SEC(1);
}

int32_t BlueDragon_Damage (LPCHARACTER me, LPCHARACTER pAttacker, int32_t dam)
{
	if (nullptr == me || nullptr == pAttacker)
		return dam;

	if (true == pAttacker->IsMonster() && 2493 == pAttacker->GetMobTable().dwVnum)
	{
		for (int32_t i=1 ; i <= 4 ; ++i)
		{
			if (ATK_BONUS == BlueDragon_GetIndexFactor("DragonStone", i, "effect_type"))
			{
				uint32_t dwDragonStoneID = BlueDragon_GetIndexFactor("DragonStone", i, "vnum");
				size_t val = BlueDragon_GetIndexFactor("DragonStone", i, "val");
				size_t cnt = SECTREE_MANAGER::instance().GetMonsterCountInMap( pAttacker->GetMapIndex(), dwDragonStoneID );

				dam += (dam * (val*cnt))/100;

				break;
			}
		}
	}

	if (true == me->IsMonster() && 2493 == me->GetMobTable().dwVnum)
	{
		for (int32_t i=1 ; i <= 4 ; ++i)
		{
			if (DEF_BONUS == BlueDragon_GetIndexFactor("DragonStone", i, "effect_type"))
			{
				uint32_t dwDragonStoneID = BlueDragon_GetIndexFactor("DragonStone", i, "vnum");
				size_t val = BlueDragon_GetIndexFactor("DragonStone", i, "val");
				size_t cnt = SECTREE_MANAGER::instance().GetMonsterCountInMap( me->GetMapIndex(), dwDragonStoneID );

				dam -= (dam * (val*cnt))/100;

				if (dam <= 0)
					dam = 1;

				break;
			}
		}
	}

	if (true == me->IsStone() && 0 != pAttacker->GetMountVnum())
	{
		for (int32_t i=1 ; i <= 4 ; ++i)
		{
			if (me->GetMobTable().dwVnum == BlueDragon_GetIndexFactor("DragonStone", i, "vnum"))
			{
				if (pAttacker->GetMountVnum() == BlueDragon_GetIndexFactor("DragonStone", i, "enemy"))
				{
					size_t val = BlueDragon_GetIndexFactor("DragonStone", i, "enemy_val");

					dam *= val;

					break;
				}
			}
		}
	}

	return dam;
}

