#pragma once
#include "../../../common/defines.h"
#include "blue_dragon_binder.h"

struct FSkillBreath
{
	EJobs Set1;
	EJobs Set2;
	ESex gender;
	LPCHARACTER pAttacker;

	FSkillBreath(LPCHARACTER p)
	{
		pAttacker = p;

		Set1 = static_cast<EJobs>(number(0,3));
		Set2 = static_cast<EJobs>(number(0,3));
		gender = static_cast<ESex>(number(0,2));
	}

	void operator()(LPENTITY ent)
	{
		if (nullptr != ent)
		{
			if (true == ent->IsType(ENTITY_CHARACTER))
			{
				LPCHARACTER ch = static_cast<LPCHARACTER>(ent);

				if (true == ch->IsPC() && false == ch->IsDead())
				{
					if (nullptr != ch->FindAffect(AFFECT_REVIVE_INVISIBLE, APPLY_NONE))
						return;

					if (static_cast<int32_t>(BlueDragon_GetSkillFactor(2, "Skill0", "damage_area")) < DISTANCE_APPROX(pAttacker->GetX()-ch->GetX(), pAttacker->GetY()-ch->GetY()))
					{
						sys_log(0, "BlueDragon: Breath too far (%d)", DISTANCE_APPROX(pAttacker->GetX()-ch->GetX(), pAttacker->GetY()-ch->GetY()) );
						return;
					}

					int32_t overlapDamageCount = 0;

					int32_t pct = 0;
					if (ch->GetJob() == Set1)
					{
						const char* ptr = nullptr;

						switch ( Set1 )
						{
							case JOB_WARRIOR:	ptr = "musa";	break;
							case JOB_ASSASSIN:	ptr = "assa";	break;
							case JOB_SURA:		ptr = "sura";	break;
							case JOB_SHAMAN:	ptr = "muda";	break;
#ifdef ENABLE_WOLFMAN_CHARACTER
							case JOB_WOLFMAN:	ptr = "wolf";	break;
#endif

							default:
							case JOB_MAX_NUM:	return;
						}


						int32_t firstDamagePercent =  number(BlueDragon_GetSkillFactor(4, "Skill0", "damage", ptr, "min"), BlueDragon_GetSkillFactor(4, "Skill0", "damage", ptr, "max"));
						pct += firstDamagePercent;

						if (firstDamagePercent > 0)
							overlapDamageCount++;
					}

					if (ch->GetJob() == Set2)
					{
						const char* ptr = nullptr;

						switch ( Set2 )
						{
							case JOB_WARRIOR:	ptr = "musa";	break;
							case JOB_ASSASSIN:	ptr = "assa";	break;
							case JOB_SURA:		ptr = "sura";	break;
							case JOB_SHAMAN:	ptr = "muda";	break;
#ifdef ENABLE_WOLFMAN_CHARACTER
							case JOB_WOLFMAN:	ptr = "wolf";	break;
#endif

							default:
							case JOB_MAX_NUM:	return;
						}

						int32_t secondDamagePercent = number(BlueDragon_GetSkillFactor(4, "Skill0", "damage", ptr, "min"), BlueDragon_GetSkillFactor(4, "Skill0", "damage", ptr, "max"));
						pct += secondDamagePercent;

						if (secondDamagePercent > 0)
							overlapDamageCount++;
					}

					if (GET_SEX(ch) == gender)
					{
						const char* ptr = nullptr;

						switch (gender)
						{
							case SEX_MALE:		ptr = "male";	break;
							case SEX_FEMALE:	ptr = "female";	break;
							default:			return;
						}

						int32_t thirdDamagePercent = number(BlueDragon_GetSkillFactor(4, "Skill0", "gender", ptr, "min"), BlueDragon_GetSkillFactor(4, "Skill0", "gender", ptr, "max"));
						pct += thirdDamagePercent;

						if (thirdDamagePercent > 0)
							overlapDamageCount++;
					}

					switch (overlapDamageCount)
					{
						case 1:
							ch->EffectPacket(SE_PERCENT_DAMAGE1);
							break;
						case 2:
							ch->EffectPacket(SE_PERCENT_DAMAGE2);
							break;
						case 3:
							ch->EffectPacket(SE_PERCENT_DAMAGE3);
							break;
					}

					int32_t addPct = BlueDragon_GetRangeFactor("hp_damage", pAttacker->GetHPPct());

					pct += addPct;

					int32_t dam = number(BlueDragon_GetSkillFactor(3, "Skill0", "default_damage", "min"), BlueDragon_GetSkillFactor(3, "Skill0", "default_damage", "max"));

					dam += (dam * addPct) / 100;
					dam += (ch->GetMaxHP() * pct) / 100;

					ch->Damage( pAttacker, dam, DAMAGE_TYPE_ICE );

					sys_log(0, "BlueDragon: Breath to %s pct(%d) dam(%d) overlap(%d)", ch->GetName(), pct, dam, overlapDamageCount);
				}
			}
		}
	}
};

struct FSkillWeakBreath
{
	LPCHARACTER pAttacker;

	FSkillWeakBreath(LPCHARACTER p)
	{
		pAttacker = p;
	}

	void operator()(LPENTITY ent)
	{
		if (nullptr != ent)
		{
			if (true == ent->IsType(ENTITY_CHARACTER))
			{
				LPCHARACTER ch = static_cast<LPCHARACTER>(ent);

				if (true == ch->IsPC() && false == ch->IsDead())
				{
					if (nullptr != ch->FindAffect(AFFECT_REVIVE_INVISIBLE, APPLY_NONE))
						return;

					if (static_cast<int32_t>(BlueDragon_GetSkillFactor(2, "Skill1", "damage_area")) < DISTANCE_APPROX(pAttacker->GetX()-ch->GetX(), pAttacker->GetY()-ch->GetY()))
					{
						sys_log(0, "BlueDragon: Breath too far (%d)", DISTANCE_APPROX(pAttacker->GetX()-ch->GetX(), pAttacker->GetY()-ch->GetY()) );
						return;
					}

					int32_t addPct = BlueDragon_GetRangeFactor("hp_damage", pAttacker->GetHPPct());

					int32_t dam = number( BlueDragon_GetSkillFactor(3, "Skill1", "default_damage", "min"), BlueDragon_GetSkillFactor(3, "Skill1", "default_damage", "max") );
					dam += (dam * addPct) / 100;

					ch->Damage( pAttacker, dam, DAMAGE_TYPE_ICE );

					sys_log(0, "BlueDragon: WeakBreath to %s addPct(%d) dam(%d)", ch->GetName(), addPct, dam);
				}
			}
		}
	}
};

struct FSkillEarthQuake
{
	EJobs Set1;
	EJobs Set2;
	ESex gender;
	int32_t MaxDistance;
	LPCHARACTER pAttacker;
	LPCHARACTER pFarthestChar;

	FSkillEarthQuake(LPCHARACTER p)
	{
		pAttacker = p;

		MaxDistance = 0;
		pFarthestChar = nullptr;

		Set1 = static_cast<EJobs>(number(0,3));
		Set2 = static_cast<EJobs>(number(0,3));
		gender = static_cast<ESex>(number(0,2));
	}

	void operator()(LPENTITY ent)
	{
		if (nullptr != ent)
		{
			if (true == ent->IsType(ENTITY_CHARACTER))
			{
				LPCHARACTER ch = static_cast<LPCHARACTER>(ent);

				if (true == ch->IsPC() && false == ch->IsDead())
				{
					if (nullptr != ch->FindAffect(AFFECT_REVIVE_INVISIBLE, APPLY_NONE))
						return;

					if (static_cast<int32_t>(BlueDragon_GetSkillFactor(2, "Skill2", "damage_area")) < DISTANCE_APPROX(pAttacker->GetX()-ch->GetX(), pAttacker->GetY()-ch->GetY()))
					{
						sys_log(0, "BlueDragon: Breath too far (%d)", DISTANCE_APPROX(pAttacker->GetX()-ch->GetX(), pAttacker->GetY()-ch->GetY()) );
						return;
					}

					int32_t sec = number(BlueDragon_GetSkillFactor(4, "Skill2", "stun_time", "default", "min"), BlueDragon_GetSkillFactor(4, "Skill2", "stun_time", "default", "max"));

					if (ch->GetJob() == Set1)
					{
						const char* ptr = nullptr;

						switch ( Set1 )
						{
							case JOB_WARRIOR:	ptr = "musa";	break;
							case JOB_ASSASSIN:	ptr = "assa";	break;
							case JOB_SURA:		ptr = "sura";	break;
							case JOB_SHAMAN:	ptr = "muda";	break;
#ifdef ENABLE_WOLFMAN_CHARACTER
							case JOB_WOLFMAN:	ptr = "wolf";	break;
#endif

							default:
							case JOB_MAX_NUM:	return;
						}

						sec += number(BlueDragon_GetSkillFactor(4, "Skill2", "stun_time", ptr, "min"), BlueDragon_GetSkillFactor(4, "Skill2", "stun_time", ptr, "max"));
					}

					if (ch->GetJob() == Set2)
					{
						const char* ptr = nullptr;

						switch ( Set2 )
						{
							case JOB_WARRIOR:	ptr = "musa";	break;
							case JOB_ASSASSIN:	ptr = "assa";	break;
							case JOB_SURA:		ptr = "sura";	break;
							case JOB_SHAMAN:	ptr = "muda";	break;
#ifdef ENABLE_WOLFMAN_CHARACTER
							case JOB_WOLFMAN:	ptr = "wolf";	break;	
#endif

							default:
							case JOB_MAX_NUM:	return;
						}

						sec += number(BlueDragon_GetSkillFactor(4, "Skill2", "stun_time", ptr, "min"), BlueDragon_GetSkillFactor(4, "Skill2", "stun_time", ptr, "max"));
					}

					if (GET_SEX(ch) == gender)
					{
						const char* ptr = nullptr;

						switch (gender)
						{
							case SEX_MALE:		ptr = "male";	break;
							case SEX_FEMALE:	ptr = "female";	break;
							default:			return;
						}

						sec += number(BlueDragon_GetSkillFactor(4, "Skill2", "gender", ptr, "min"), BlueDragon_GetSkillFactor(4, "Skill2", "gender", ptr, "max"));
					}

					int32_t addPct = BlueDragon_GetRangeFactor("hp_damage", pAttacker->GetHPPct());

					int32_t dam = number( BlueDragon_GetSkillFactor(3, "Skill2", "default_damage", "min"), BlueDragon_GetSkillFactor(3, "Skill2", "default_damage", "max") );
					dam += (dam * addPct) / 100;

					ch->Damage( pAttacker, dam, DAMAGE_TYPE_ICE);

					SkillAttackAffect( ch, 1000, IMMUNE_STUN, AFFECT_STUN, POINT_NONE, 0, AFF_STUN, sec, "BDRAGON_STUN" );

					sys_log(0, "BlueDragon: EarthQuake to %s addPct(%d) dam(%d) sec(%d)", ch->GetName(), addPct, dam, sec);


					VECTOR vec;
					vec.x = static_cast<float>(pAttacker->GetX() - ch->GetX());
					vec.y = static_cast<float>(pAttacker->GetY() - ch->GetY());
					vec.z = 0.0f;

					Normalize( &vec, &vec );

					const int32_t nFlyDistance = 1000;

					int32_t tx = (int32_t)(ch->GetX() + vec.x * nFlyDistance);
					int32_t ty = (int32_t)(ch->GetY() + vec.y * nFlyDistance);

					for (int32_t i=0 ; i < 5 ; ++i)
					{
						if (true == SECTREE_MANAGER::Instance().IsMovablePosition( ch->GetMapIndex(), tx, ty ))
						{
							break;
						}

						switch( i )
						{
							case 0:
								tx = ch->GetX() + vec.x * nFlyDistance * -1;
								ty = ch->GetY() + vec.y * nFlyDistance * -1;
								break;
							case 1:
								tx = ch->GetX() + vec.x * nFlyDistance * -1;
								ty = ch->GetY() + vec.y * nFlyDistance;
								break;
							case 2:
								tx = ch->GetX() + vec.x * nFlyDistance;
								ty = ch->GetY() + vec.y * nFlyDistance * -1;
								break;
							case 3:
								tx = ch->GetX() + vec.x * number(1,100);
								ty = ch->GetY() + vec.y * number(1,100);
								break;
							case 4:
								tx = ch->GetX() + vec.x * number(1,10);
								ty = ch->GetY() + vec.y * number(1,10);
								break;
						}
					}

					if (ch->CanFall())
					{
						ch->Sync(tx, ty);
						ch->Goto(tx, ty);
						ch->CalculateMoveDuration();

						ch->SyncPacket();
					}

					int32_t dist = DISTANCE_APPROX( pAttacker->GetX() - ch->GetX(), pAttacker->GetY() - ch->GetY() );

					if (dist > MaxDistance)
					{
						MaxDistance = dist;
						pFarthestChar = ch;
					}
				}
			}
		}
	}
};
