#include "stdafx.h"
#include "config.h"
#include "utils.h"
#include "vector.h"
#include "char.h"
#include "battle.h"
#include "char_manager.h"
#include "motion.h"
#include "party.h"
#include "affect.h"
#include "buffer_manager.h"
#include "quest_manager.h"
#include "p2p.h"
#include "item_manager.h"
#include "mob_manager.h"
#include "exchange.h"
#include "sectree_manager.h"
#include "xmas_event.h"
#include "guild_manager.h"
#include "war_map.h"
#include "locale_service.h"
#include "blue_dragon.h"
#include "battleground.h"

extern LPCHARACTER FindMobVictim(LPCHARACTER pkChr, int32_t iMaxDistance);
extern LPCHARACTER FindBattlegroundMobVictim(LPCHARACTER pkChr);

namespace
{
	class FuncFindChrForFlag
	{
		public:
			FuncFindChrForFlag(LPCHARACTER pkChr) :
				m_pkChr(pkChr), m_pkChrFind(nullptr), m_iMinDistance(INT_MAX)
				{
				}

			void operator () (LPENTITY ent)
			{
				if (!ent->IsType(ENTITY_CHARACTER))
					return;

				if (ent->IsObserverMode())
					return;

				LPCHARACTER pkChr = (LPCHARACTER) ent;

				if (!pkChr->IsPC())
					return;

				if (!pkChr->GetGuild())
					return;

				if (pkChr->IsDead())
					return;

		        if (!m_pkChr || !m_pkChr->IsPC() || !m_pkChr->GetGuild() || m_pkChr->IsDead())
		            return;

				int32_t iDist = DISTANCE_APPROX(pkChr->GetX()-m_pkChr->GetX(), pkChr->GetY()-m_pkChr->GetY());

				if (iDist <= 500 && m_iMinDistance > iDist &&
						!pkChr->IsAffectFlag(AFF_WAR_FLAG1) &&
						!pkChr->IsAffectFlag(AFF_WAR_FLAG2) &&
						!pkChr->IsAffectFlag(AFF_WAR_FLAG3))
				{
					// 우리편 깃발일 경우
					if ((uint32_t) m_pkChr->GetGuild()->GetID() == pkChr->GetGuild()->GetID())
					{
						CWarMap * pMap = pkChr->GetWarMap();
						uint8_t idx;

						if (!pMap || !pMap->GetTeamIndex(pkChr->GetGuild()->GetID(), idx))
							return;

						// 우리편 기지에 깃발이 없을 때만 깃발을 뽑는다. 안그러면 기지에 있는 깃발을
						// 가만히 두고 싶은데도 뽑힐수가 있으므로..
						if (!pMap->IsFlagOnBase(idx))
						{
							m_pkChrFind = pkChr;
							m_iMinDistance = iDist;
						}
					}
					else
					{
						// 상대편 깃발인 경우 무조건 뽑는다.
						m_pkChrFind = pkChr;
						m_iMinDistance = iDist;
					}
				}
			}

			LPCHARACTER	m_pkChr;
			LPCHARACTER m_pkChrFind;
			int32_t		m_iMinDistance;
	};

	class FuncFindChrForFlagBase
	{
		public:
			FuncFindChrForFlagBase(LPCHARACTER pkChr) : m_pkChr(pkChr)
			{
			}

			void operator () (LPENTITY ent)
			{
				if (!ent->IsType(ENTITY_CHARACTER))
					return;

				if (ent->IsObserverMode())
					return;

				LPCHARACTER pkChr = (LPCHARACTER) ent;

				if (!pkChr->IsPC())
					return;

				CGuild * pkGuild = pkChr->GetGuild();

				if (!pkGuild)
					return;

		        if (!m_pkChr || !m_pkChr->IsPC() || !m_pkChr->GetGuild())
		            return;
        
				int32_t iDist = DISTANCE_APPROX(pkChr->GetX()-m_pkChr->GetX(), pkChr->GetY()-m_pkChr->GetY());

				if (iDist <= 500 &&
						(pkChr->IsAffectFlag(AFF_WAR_FLAG1) || 
						 pkChr->IsAffectFlag(AFF_WAR_FLAG2) ||
						 pkChr->IsAffectFlag(AFF_WAR_FLAG3)))
				{
					CAffect * pkAff = pkChr->FindAffect(AFFECT_WAR_FLAG);

					sys_log(0, "FlagBase %s dist %d aff %p flag gid %d chr gid %u",
							pkChr->GetName(), iDist, pkAff, m_pkChr->GetGuild()->GetID(),
							pkChr->GetGuild()->GetID());

					if (pkAff)
					{
						if ((uint32_t) m_pkChr->GetGuild()->GetID() == pkGuild->GetID() &&
								m_pkChr->GetGuild()->GetID() != pkAff->lApplyValue)
						{
							CWarMap * pMap = pkChr->GetWarMap();
							uint8_t idx;

							if (!pMap || !pMap->GetTeamIndex(pkGuild->GetID(), idx))
								return;

							//if (pMap->IsFlagOnBase(idx))
							{
								uint8_t idx_opp = idx == 0 ? 1 : 0;

								SendGuildWarScore(m_pkChr->GetGuild()->GetID(), pkAff->lApplyValue, 1);
								//SendGuildWarScore(pkAff->lApplyValue, m_pkChr->GetGuild()->GetID(), -1);

								pMap->ResetFlag();
								//pMap->AddFlag(idx_opp);
								//pkChr->RemoveAffect(AFFECT_WAR_FLAG);

								char buf[256];
								snprintf(buf, sizeof(buf), LC_TEXT("%s 길드가 %s 길드의 깃발을 빼앗았습니다!"), pMap->GetGuild(idx)->GetName(), pMap->GetGuild(idx_opp)->GetName());
								pMap->Notice(buf);
							}
						}
					}
				}
			}

			LPCHARACTER m_pkChr;
	};

	class FuncFindGuardVictim
	{
		public:
			FuncFindGuardVictim(LPCHARACTER pkChr, int32_t iMaxDistance) :
				m_pkChr(pkChr),
			m_iMinDistance(INT_MAX),
			m_iMaxDistance(iMaxDistance),
			m_lx(pkChr->GetX()),
			m_ly(pkChr->GetY()),
			m_pkChrVictim(nullptr)
			{
			};

			void operator () (LPENTITY ent)
			{
				if (!ent->IsType(ENTITY_CHARACTER))
					return;

				LPCHARACTER pkChr = (LPCHARACTER) ent;

				// 일단 PC 공격안함
				if (pkChr->IsPC())
					return;


				if (pkChr->IsNPC() && !pkChr->IsMonster())
					return;

				if (pkChr->IsDead())
					return;

				if (pkChr->IsAffectFlag(AFF_EUNHYUNG) || 
						pkChr->IsAffectFlag(AFF_INVISIBILITY) ||
						pkChr->IsAffectFlag(AFF_REVIVE_INVISIBLE))
					return;

				// 왜구는 패스
				if (pkChr->GetRaceNum() == 5001)
					return;

				int32_t iDistance = DISTANCE_APPROX(m_lx - pkChr->GetX(), m_ly - pkChr->GetY());

				if (iDistance < m_iMinDistance && iDistance <= m_iMaxDistance)
				{
					m_pkChrVictim = pkChr;
					m_iMinDistance = iDistance;
				}
			}

			LPCHARACTER GetVictim()
			{
				return (m_pkChrVictim);
			}

		private:
			LPCHARACTER	m_pkChr;

			int32_t		m_iMinDistance;
			int32_t		m_iMaxDistance;
			int32_t	m_lx;
			int32_t	m_ly;

			LPCHARACTER	m_pkChrVictim;
	};

	class FuncFindPlayerVictim
	{
	public:
		FuncFindPlayerVictim(int32_t lBaseX, int32_t lBaseY, int32_t iMaxDistance) :
			m_iMinDistance(INT_MAX),
			m_iMaxDistance(iMaxDistance),
			m_lx(lBaseX),
			m_ly(lBaseY),
			m_pkChrVictim(nullptr)
		{
		};

		void operator () (LPENTITY ent)
		{
			if (!ent->IsType(ENTITY_CHARACTER))
				return;

			LPCHARACTER pkChr = (LPCHARACTER)ent;

			if (!pkChr->IsPC())
				return;

			if (pkChr->IsDead())
				return;

			if (pkChr->IsAffectFlag(AFF_EUNHYUNG) ||
				pkChr->IsAffectFlag(AFF_INVISIBILITY) ||
				pkChr->IsAffectFlag(AFF_REVIVE_INVISIBLE))
				return;

			int32_t iDistance = DISTANCE_APPROX(m_lx - pkChr->GetX(), m_ly - pkChr->GetY());

			if (iDistance < m_iMinDistance && iDistance <= m_iMaxDistance)
			{
				m_pkChrVictim = pkChr;
				m_iMinDistance = iDistance;
			}
		}

		LPCHARACTER GetVictim()
		{
			return (m_pkChrVictim);
		}

	private:
		int32_t		m_iMinDistance;
		int32_t		m_iMaxDistance;
		int32_t	m_lx;
		int32_t	m_ly;

		LPCHARACTER	m_pkChrVictim;
	};
}

bool CHARACTER::IsAggressive() const
{
	return IS_SET(m_pointsInstant.dwAIFlag, AIFLAG_AGGRESSIVE);
}

void CHARACTER::SetAggressive()
{
	SET_BIT(m_pointsInstant.dwAIFlag, AIFLAG_AGGRESSIVE);
}

bool CHARACTER::IsCoward() const
{
	return IS_SET(m_pointsInstant.dwAIFlag, AIFLAG_COWARD);
}

void CHARACTER::SetCoward()
{
	SET_BIT(m_pointsInstant.dwAIFlag, AIFLAG_COWARD);
}

bool CHARACTER::IsBerserker() const
{
	return IS_SET(m_pointsInstant.dwAIFlag, AIFLAG_BERSERK);
}

bool CHARACTER::IsStoneSkinner() const
{
	return IS_SET(m_pointsInstant.dwAIFlag, AIFLAG_STONESKIN);
}

bool CHARACTER::IsGodSpeeder() const
{
	return IS_SET(m_pointsInstant.dwAIFlag, AIFLAG_GODSPEED);
}

bool CHARACTER::IsDeathBlower() const
{
	return IS_SET(m_pointsInstant.dwAIFlag, AIFLAG_DEATHBLOW);
}

bool CHARACTER::IsReviver() const
{
	return IS_SET(m_pointsInstant.dwAIFlag, AIFLAG_REVIVE);
}

void CHARACTER::CowardEscape()
{
	int32_t iDist[4] = {500, 1000, 3000, 5000};

	for (int32_t iDistIdx = 2; iDistIdx >= 0; --iDistIdx)
		for (int32_t iTryCount = 0; iTryCount < 8; ++iTryCount)
		{
			SetRotation(number(0, 359));        // 방향은 랜덤으로 설정

			float fx, fy;
			float fDist = number(iDist[iDistIdx], iDist[iDistIdx+1]);

			GetDeltaByDegree(GetRotation(), fDist, &fx, &fy);

			bool bIsWayBlocked = false;
			for (int32_t j = 1; j <= 100; ++j)
			{
				if (!SECTREE_MANAGER::Instance().IsMovablePosition(GetMapIndex(), GetX() + (int32_t) fx*j/100, GetY() + (int32_t) fy*j/100))
				{
					bIsWayBlocked = true;
					break;
				}
			}

			if (bIsWayBlocked)
				continue;

			m_dwStateDuration = PASSES_PER_SEC(1);

			int32_t iDestX = GetX() + (int32_t) fx;
			int32_t iDestY = GetY() + (int32_t) fy;

			if (Goto(iDestX, iDestY))
				SendMovePacket(FUNC_WAIT, 0, 0, 0, 0);

			sys_log(0, "WAEGU move to %d %d (far)", iDestX, iDestY);
			return;
		}
}

void  CHARACTER::SetNoAttackShinsu()
{
	SET_BIT(m_pointsInstant.dwAIFlag, AIFLAG_NOATTACKSHINSU);
}
bool CHARACTER::IsNoAttackShinsu() const
{
	return IS_SET(m_pointsInstant.dwAIFlag, AIFLAG_NOATTACKSHINSU);
}

void CHARACTER::SetNoAttackChunjo()
{
	SET_BIT(m_pointsInstant.dwAIFlag, AIFLAG_NOATTACKCHUNJO);
}

bool CHARACTER::IsNoAttackChunjo() const
{
	return IS_SET(m_pointsInstant.dwAIFlag, AIFLAG_NOATTACKCHUNJO);
}

void CHARACTER::SetNoAttackJinno()
{
	SET_BIT(m_pointsInstant.dwAIFlag, AIFLAG_NOATTACKJINNO);
}

bool CHARACTER::IsNoAttackJinno() const
{
	return IS_SET(m_pointsInstant.dwAIFlag, AIFLAG_NOATTACKJINNO);
}

void CHARACTER::SetAttackMob()
{
	SET_BIT(m_pointsInstant.dwAIFlag, AIFLAG_ATTACKMOB);
}

bool CHARACTER::IsAttackMob() const
{
	return IS_SET(m_pointsInstant.dwAIFlag, AIFLAG_ATTACKMOB);
}

void CHARACTER::SetNoMove()
{
	SET_BIT(m_pointsInstant.dwAIFlag, AIFLAG_NOMOVE);
}

bool CHARACTER::CanNPCFollowTarget(LPCHARACTER pkTarget)
{
	if (!IsNPC() && !IsMonster())
		return true;

	if (IS_SET(GetAIFlag(), AIFLAG_NOMOVE))
	{
		if (GetBattleground())
			return false;

		if (!IS_SET(GetAIFlag(), AIFLAG_AGGRESSIVE))
			return false;

		if (DISTANCE_APPROX(pkTarget->GetX() - m_posExit.x, pkTarget->GetY() - m_posExit.y) > GetMobTable().wAggressiveSight)
			return false;
	}

	return true;
}

// STATE_IDLE_REFACTORING
void CHARACTER::StateIdle()
{
	if (IsStone())
	{
		__StateIdle_Stone();
		return;
	}
	else if (IsWarp() || IsGoto())
	{
		// 워프는 이벤트로 처리
		m_dwStateDuration = 60 * passes_per_sec;
		return;
	}

	if (IsPC())
		return;

	// NPC 처리
	if (!IsMonster())
	{
		__StateIdle_NPC();
		return;
	}

	__StateIdle_Monster();
}

void CHARACTER::__StateIdle_Stone()
{
	m_dwStateDuration = PASSES_PER_SEC(1);

	int32_t iPercent = 0;
	if (GetMaxHP() >= 0)
		iPercent = (GetHP() * 100) / GetMaxHP();
	uint32_t dwVnum = number(MIN(GetMobTable().sAttackSpeed, GetMobTable().sMovingSpeed ), MAX(GetMobTable().sAttackSpeed, GetMobTable().sMovingSpeed));

	if (iPercent <= 10 && GetMaxSP() < 10)
	{
		SetMaxSP(10);
		SendMovePacket(FUNC_ATTACK, 0, GetX(), GetY(), 0);

		CHARACTER_MANAGER::Instance().SelectStone(this);
		CHARACTER_MANAGER::Instance().SpawnGroup(dwVnum, GetMapIndex(), GetX() - 500, GetY() - 500, GetX() + 500, GetY() + 500);
		CHARACTER_MANAGER::Instance().SpawnGroup(dwVnum, GetMapIndex(), GetX() - 1000, GetY() - 1000, GetX() + 1000, GetY() + 1000);
		CHARACTER_MANAGER::Instance().SpawnGroup(dwVnum, GetMapIndex(), GetX() - 1500, GetY() - 1500, GetX() + 1500, GetY() + 1500);
		CHARACTER_MANAGER::Instance().SelectStone(nullptr);
	}
	else if (iPercent <= 20 && GetMaxSP() < 9)
	{
		SetMaxSP(9);
		SendMovePacket(FUNC_ATTACK, 0, GetX(), GetY(), 0);

		CHARACTER_MANAGER::Instance().SelectStone(this);
		CHARACTER_MANAGER::Instance().SpawnGroup(dwVnum, GetMapIndex(), GetX() - 500, GetY() - 500, GetX() + 500, GetY() + 500);
		CHARACTER_MANAGER::Instance().SpawnGroup(dwVnum, GetMapIndex(), GetX() - 1000, GetY() - 1000, GetX() + 1000, GetY() + 1000);
		CHARACTER_MANAGER::Instance().SpawnGroup(dwVnum, GetMapIndex(), GetX() - 1500, GetY() - 1500, GetX() + 1500, GetY() + 1500);
		CHARACTER_MANAGER::Instance().SelectStone(nullptr);
	}
	else if (iPercent <= 30 && GetMaxSP() < 8)
	{
		SetMaxSP(8);
		SendMovePacket(FUNC_ATTACK, 0, GetX(), GetY(), 0);

		CHARACTER_MANAGER::Instance().SelectStone(this);
		CHARACTER_MANAGER::Instance().SpawnGroup(dwVnum, GetMapIndex(), GetX() - 500, GetY() - 500, GetX() + 500, GetY() + 500);
		CHARACTER_MANAGER::Instance().SpawnGroup(dwVnum, GetMapIndex(), GetX() - 1000, GetY() - 1000, GetX() + 1000, GetY() + 1000);
		CHARACTER_MANAGER::Instance().SpawnGroup(dwVnum, GetMapIndex(), GetX() - 1000, GetY() - 1000, GetX() + 1000, GetY() + 1000);
		CHARACTER_MANAGER::Instance().SelectStone(nullptr);
	}
	else if (iPercent <= 40 && GetMaxSP() < 7)
	{
		SetMaxSP(7);
		SendMovePacket(FUNC_ATTACK, 0, GetX(), GetY(), 0);

		CHARACTER_MANAGER::Instance().SelectStone(this);
		CHARACTER_MANAGER::Instance().SpawnGroup(dwVnum, GetMapIndex(), GetX() - 1000, GetY() - 1000, GetX() + 1000, GetY() + 1000);
		CHARACTER_MANAGER::Instance().SpawnGroup(dwVnum, GetMapIndex(), GetX() - 1000, GetY() - 1000, GetX() + 1000, GetY() + 1000);
		CHARACTER_MANAGER::Instance().SpawnGroup(dwVnum, GetMapIndex(), GetX() - 1000, GetY() - 1000, GetX() + 1000, GetY() + 1000);
		CHARACTER_MANAGER::Instance().SelectStone(nullptr);
	}
	else if (iPercent <= 50 && GetMaxSP() < 6)
	{
		SetMaxSP(6);
		SendMovePacket(FUNC_ATTACK, 0, GetX(), GetY(), 0);

		CHARACTER_MANAGER::Instance().SelectStone(this);
		CHARACTER_MANAGER::Instance().SpawnGroup(dwVnum, GetMapIndex(), GetX() - 1000, GetY() - 1000, GetX() + 1000, GetY() + 1000);
		CHARACTER_MANAGER::Instance().SpawnGroup(dwVnum, GetMapIndex(), GetX() - 1000, GetY() - 1000, GetX() + 1000, GetY() + 1000);
		CHARACTER_MANAGER::Instance().SelectStone(nullptr);
	}
	else if (iPercent <= 60 && GetMaxSP() < 5)
	{
		SetMaxSP(5);
		SendMovePacket(FUNC_ATTACK, 0, GetX(), GetY(), 0);

		CHARACTER_MANAGER::Instance().SelectStone(this);
		CHARACTER_MANAGER::Instance().SpawnGroup(dwVnum, GetMapIndex(), GetX() - 1000, GetY() - 1000, GetX() + 1000, GetY() + 1000);
		CHARACTER_MANAGER::Instance().SpawnGroup(dwVnum, GetMapIndex(), GetX() - 500, GetY() - 500, GetX() + 500, GetY() + 500);
		CHARACTER_MANAGER::Instance().SelectStone(nullptr);
	}
	else if (iPercent <= 70 && GetMaxSP() < 4)
	{
		SetMaxSP(4);
		SendMovePacket(FUNC_ATTACK, 0, GetX(), GetY(), 0);

		CHARACTER_MANAGER::Instance().SelectStone(this);
		CHARACTER_MANAGER::Instance().SpawnGroup(dwVnum, GetMapIndex(), GetX() - 500, GetY() - 500, GetX() + 500, GetY() + 500);
		CHARACTER_MANAGER::Instance().SpawnGroup(dwVnum, GetMapIndex(), GetX() - 1000, GetY() - 1000, GetX() + 1000, GetY() + 1000);
		CHARACTER_MANAGER::Instance().SelectStone(nullptr);
	}
	else if (iPercent <= 80 && GetMaxSP() < 3)
	{
		SetMaxSP(3);
		SendMovePacket(FUNC_ATTACK, 0, GetX(), GetY(), 0);

		CHARACTER_MANAGER::Instance().SelectStone(this);
		CHARACTER_MANAGER::Instance().SpawnGroup(dwVnum, GetMapIndex(), GetX() - 1000, GetY() - 1000, GetX() + 1000, GetY() + 1000);
		CHARACTER_MANAGER::Instance().SpawnGroup(dwVnum, GetMapIndex(), GetX() - 1000, GetY() - 1000, GetX() + 1000, GetY() + 1000);
		CHARACTER_MANAGER::Instance().SelectStone(nullptr);
	}
	else if (iPercent <= 90 && GetMaxSP() < 2)
	{
		SetMaxSP(2);
		SendMovePacket(FUNC_ATTACK, 0, GetX(), GetY(), 0);

		CHARACTER_MANAGER::Instance().SelectStone(this);
		CHARACTER_MANAGER::Instance().SpawnGroup(dwVnum, GetMapIndex(), GetX() - 500, GetY() - 500, GetX() + 500, GetY() + 500);
		CHARACTER_MANAGER::Instance().SelectStone(nullptr);
	}
	else if (iPercent <= 99 && GetMaxSP() < 1)
	{
		SetMaxSP(1);
		SendMovePacket(FUNC_ATTACK, 0, GetX(), GetY(), 0);

		CHARACTER_MANAGER::Instance().SelectStone(this);
		CHARACTER_MANAGER::Instance().SpawnGroup(dwVnum, GetMapIndex(), GetX() - 1000, GetY() - 1000, GetX() + 1000, GetY() + 1000);
		CHARACTER_MANAGER::Instance().SelectStone(nullptr);
	}
	else
		return;

	UpdatePacket();
	return;
}

void CHARACTER::__StateIdle_NPC()
{
	MonsterChat(MONSTER_CHAT_WAIT);
	m_dwStateDuration = PASSES_PER_SEC(5);

	// 펫 시스템의 Idle 처리는 기존 거의 모든 종류의 캐릭터들이 공유해서 사용하는 상태머신이 아닌 CPetActor::Update에서 처리함.
	if (IsPet())
		return;
	else if (IsGuardNPC())
	{
		if (!quest::CQuestManager::Instance().GetEventFlag("noguard"))
		{
			FuncFindGuardVictim f(this, 50000);

			if (GetSectree())
				GetSectree()->ForEachAround(f);

			LPCHARACTER victim = f.GetVictim();

			if (victim)
			{
				m_dwStateDuration = passes_per_sec/2;

				if (CanBeginFight())
					BeginFight(victim);
			}
		}
	}
	else
	{
		if (GetRaceNum() == xmas::MOB_SANTA_VNUM) // 산타
		{
			if (get_unix_ms_time() > m_dwPlayStartTime)
			{
				int32_t	next_warp_time = 2 * 1000;	// 2초

				m_dwPlayStartTime = get_unix_ms_time() + next_warp_time;

				// 시간이 넘었으니 워프합시다.
				/*
				 * 산타용
				const int32_t WARP_MAP_INDEX_NUM = 4;
				static const int32_t c_lWarpMapIndexs[WARP_MAP_INDEX_NUM] = {61, 62, 63, 64};
				*/
				// 신선자 노해용
				const int32_t WARP_MAP_INDEX_NUM = 7;
				static const int32_t c_lWarpMapIndexs[WARP_MAP_INDEX_NUM] = { 61, 62, 63, 64, 3, 23, 43 };
				int32_t lNextMapIndex;
				lNextMapIndex = c_lWarpMapIndexs[number(1, WARP_MAP_INDEX_NUM) - 1];

				if (map_allow_find(lNextMapIndex))
				{
					// 이곳입니다.
					M2_DESTROY_CHARACTER(this);
					int32_t iNextSpawnDelay = 50 * 60;

					xmas::SpawnSanta(lNextMapIndex, iNextSpawnDelay);
				}
				else
				{
					// 다른 서버 입니다.
					TPacketGGXmasWarpSanta p;
					p.bHeader   = HEADER_GG_XMAS_WARP_SANTA;
					p.bChannel  = g_bChannel;
					p.lMapIndex = lNextMapIndex;
					P2P_MANAGER::Instance().Send(&p, sizeof(TPacketGGXmasWarpSanta));
				}
				return;
			}
		}

		if (!IS_SET(m_pointsInstant.dwAIFlag, AIFLAG_NOMOVE))
		{
			if (IS_SET(m_pointsInstant.dwAIFlag, AIFLAG_AGGRESSIVE) && IS_SET(m_pointsInstant.dwAIFlag, AIFLAG_ATTACKMOB))
			{
				FuncFindGuardVictim f(this, 50000);

				if (GetSectree())
					GetSectree()->ForEachAround(f);

				LPCHARACTER victim = f.GetVictim();

				if (victim)
				{
					m_dwStateDuration = passes_per_sec / 2;

					if (CanBeginFight())
						BeginFight(victim);
				}
			}

			//
			// 이 곳 저 곳 이동한다.
			// 
			LPCHARACTER pkChrProtege = GetProtege();

			if (pkChrProtege)
			{
				if (DISTANCE_APPROX(GetX() - pkChrProtege->GetX(), GetY() - pkChrProtege->GetY()) > 500)
				{
					if (Follow(pkChrProtege, number(100, 300)))
						return;
				}
			}

			if (!number(0, 6))
			{
				SetRotation(number(0, 359));        // 방향은 랜덤으로 설정

				float fx, fy;
				float fDist = number(200, 400);

				GetDeltaByDegree(GetRotation(), fDist, &fx, &fy);

				// 느슨한 못감 속성 체크; 최종 위치와 중간 위치가 갈수없다면 가지 않는다.
				if (!(SECTREE_MANAGER::Instance().IsMovablePosition(GetMapIndex(), GetX() + (int32_t) fx, GetY() + (int32_t) fy) 
					&& SECTREE_MANAGER::Instance().IsMovablePosition(GetMapIndex(), GetX() + (int32_t) fx / 2, GetY() + (int32_t) fy / 2)))
					return;

				SetNowWalking(true);

				if (Goto(GetX() + (int32_t) fx, GetY() + (int32_t) fy))
					SendMovePacket(FUNC_WAIT, 0, 0, 0, 0);

				return;
			}
		}
		else if (IS_SET(GetAIFlag(), AIFLAG_AGGRESSIVE))
		{
			FuncFindPlayerVictim f(m_posExit.x, m_posExit.y, GetMobTable().wAggressiveSight);

			if (GetSectree())
				GetSectree()->ForEachAround(f);

			LPCHARACTER victim = f.GetVictim();

			if (victim)
			{
				m_dwStateDuration = passes_per_sec / 2;

				if (CanBeginFight())
					BeginFight(victim);
			}
		}
	}
}

void CHARACTER::__StateIdle_Monster()
{
	if (IsStun())
		return;

	if (!CanMove())
		return;

	if (IsCoward())
	{
		// 겁쟁이 몬스터는 도망만 다닙니다.
		if (!IsDead())
			CowardEscape();

		return;
	}

	if (IsBerserker())
		if (IsBerserk())
			SetBerserk(false);

	if (IsGodSpeeder())
		if (IsGodSpeed())
			SetGodSpeed(false);

	LPCHARACTER victim = GetVictim();

	if (!victim || victim->IsDead())
	{
		SetVictim(nullptr);
		victim = nullptr;
		m_dwStateDuration = PASSES_PER_SEC(1);
	}

	if (!victim || victim->IsBuilding())
	{
		// 돌 보호 처리
		if (m_pkChrStone)
		{
			victim = m_pkChrStone->GetNearestVictim(m_pkChrStone);
		}
		else if (GetBattleground())
		{
			switch (GetRaceNum())
			{
				case MINNION_SOLDIER:
				case MINNION_ARCHER:
				case MINNION_COMMANDER:
				{
					victim = FindBattlegroundMobVictim(this);
				} break;
				
				default:
					break;
			}			
		}
		// 선공 몬스터 처리
		else if (!no_wander && IsAggressive())
		{
			if (GetMapIndex() == 61 && quest::CQuestManager::Instance().GetEventFlag("xmas_tree"));
			// 서한산에서 나무가 있으면 선공하지않는다.
			else
				victim = FindMobVictim(this, m_pkMobData->m_table.wAggressiveSight);
		}
	}

	if (victim && !victim->IsDead())
	{
		if (CanBeginFight())
			BeginFight(victim);

		return;
	}

	if (IsAggressive() && !victim)
		m_dwStateDuration = PASSES_PER_SEC(number(1, 3));
	else
		m_dwStateDuration = PASSES_PER_SEC(number(3, 5));

	LPCHARACTER pkChrProtege = GetProtege();

	// 보호할 것(돌, 파티장)에게로 부터 멀다면 따라간다.
	if (pkChrProtege)
	{
		if (DISTANCE_APPROX(GetX() - pkChrProtege->GetX(), GetY() - pkChrProtege->GetY()) > 1000)
		{
			if (Follow(pkChrProtege, number(150, 400)))
			{
				return;
			}
		}
	}

	//
	// 그냥 왔다리 갔다리 한다.
	//
	if (!no_wander && !IS_SET(m_pointsInstant.dwAIFlag, AIFLAG_NOMOVE))
	{
		if (!number(0, 6))
		{
			SetRotation(number(0, 359));        // 방향은 랜덤으로 설정

			float fx, fy;
			float fDist = number(300, 700);

			GetDeltaByDegree(GetRotation(), fDist, &fx, &fy);

			// 느슨한 못감 속성 체크; 최종 위치와 중간 위치가 갈수없다면 가지 않는다.
			if (!(SECTREE_MANAGER::Instance().IsMovablePosition(GetMapIndex(), GetX() + (int32_t) fx, GetY() + (int32_t) fy) 
						&& SECTREE_MANAGER::Instance().IsMovablePosition(GetMapIndex(), GetX() + (int32_t) fx/2, GetY() + (int32_t) fy/2)))
				return;

			// NOTE: 몬스터가 IDLE 상태에서 주변을 서성거릴 때, 현재 무조건 뛰어가게 되어 있음. (절대로 걷지 않음)
			// 그래픽 팀에서 몬스터가 걷는 모습도 보고싶다고 해서 임시로 특정확률로 걷거나 뛰게 함. (게임의 전반적인 느낌이 틀려지기 때문에 일단 테스트 모드에서만 작동)
			if (g_bIsTestServer)
			{
				if (number(0, 100) < 60)
					SetNowWalking(false);
				else
					SetNowWalking(true);
			}

			if (Goto(GetX() + (int32_t) fx, GetY() + (int32_t) fy))
				SendMovePacket(FUNC_WAIT, 0, 0, 0, 0);

			return;
		}
	}

	MonsterChat(MONSTER_CHAT_WAIT);
}
// END_OF_STATE_IDLE_REFACTORING

bool __CHARACTER_GotoNearTarget(LPCHARACTER self, LPCHARACTER victim)
{
	if (!self->CanNPCFollowTarget(victim))
		return false;

	switch (self->GetMobBattleType())
	{
		case BATTLE_TYPE_RANGE:
		case BATTLE_TYPE_MAGIC:
			// 마법사나 궁수는 공격 거리의 80%까지 가서 공격을 시작한다.
			if (self->Follow(victim, self->GetMobAttackRange() * 8 / 10))
				return true;
			break;

		default:
			// 나머지는 90%?
			if (self->Follow(victim, self->GetMobAttackRange() * 9 / 10))
				return true;
	}

	return false;
}

void CHARACTER::StateMove()
{
	uint32_t dwElapsedTime = get_unix_ms_time() - m_dwMoveStartTime;
	float fRate = (float) dwElapsedTime / (float) m_dwMoveDuration;

	if (fRate > 1.0f)
		fRate = 1.0f;

	int32_t x = (int32_t) ((float) (m_posDest.x - m_posStart.x) * fRate + m_posStart.x);
	int32_t y = (int32_t) ((float) (m_posDest.y - m_posStart.y) * fRate + m_posStart.y);

	Move(x, y);

	if ((IsPC() || GetBattleground()) && (thecore_pulse() & 15) == 0)
	{
		UpdateSectree();

		if (IsPC() && GetExchange())
		{
			LPCHARACTER victim = GetExchange()->GetCompany()->GetOwner();
			int32_t iDist = DISTANCE_APPROX(GetX() - victim->GetX(), GetY() - victim->GetY());

			// 거리 체크
			if (iDist >= EXCHANGE_MAX_DISTANCE)
			{
				GetExchange()->Cancel();
			}
		}
	}

	// 스테미나가 0 이상이어야 한다.
	if (IsPC())
	{
    	// Cooldown attacks
		GetAbuseController()->SuspiciousAttackCooldown();

		if (IsWalking() && GetStamina() < GetMaxStamina())
		{
			// 5초 후 부터 스테미너 증가
			if (get_unix_ms_time() - GetWalkStartTime() > 5000)
				PointChange(POINT_STAMINA, GetMaxStamina());
		}

		// 전투 중이면서 뛰는 중이면
		if (!IsWalking() && !IsRiding())
		{
			if ((get_unix_ms_time() - GetLastAttackTime()) < 20000)
			{
				StartAffectEvent();

				if (IsStaminaHalfConsume())
				{
					if (thecore_pulse()&1)
						PointChange(POINT_STAMINA, -STAMINA_PER_STEP);
				}
				else
					PointChange(POINT_STAMINA, -STAMINA_PER_STEP);

				StartStaminaConsume();

				if (GetStamina() <= 0)
				{
					// 스테미나가 모자라 걸어야함
					SetStamina(0);
					SetNowWalking(true);
					StopStaminaConsume();
				}
			}
			else if (IsStaminaConsume())
			{
				StopStaminaConsume();
			}
	}
	}
	else
	{
		// XXX AGGRO 
		if (IsMonster() && GetVictim())
		{
			LPCHARACTER victim = GetVictim();
			UpdateAggrPoint(victim, DAMAGE_TYPE_NORMAL, -(victim->GetLevel() / 3 + 1));
		}

		if (IsMonster() && GetMobRank() >= MOB_RANK_BOSS && GetVictim())
		{
			LPCHARACTER victim = GetVictim();

			// 거대 거북
			if ((GetRaceNum() == 2191 || GetRaceNum() == 2192) && number(1, 20) == 1 && get_unix_ms_time() - m_pkMobInst->m_dwLastWarpTime > 1000)
			{
				// 워프 테스트
				float fx, fy;
				GetDeltaByDegree(victim->GetRotation(), 400, &fx, &fy);
				int32_t new_x = victim->GetX() + (int32_t)fx;
				int32_t new_y = victim->GetY() + (int32_t)fy;
				SetRotation(GetDegreeFromPositionXY(new_x, new_y, victim->GetX(), victim->GetY()));
				Show(victim->GetMapIndex(), new_x, new_y, 0, true);
				GotoState(m_stateBattle);
				m_dwStateDuration = 1;
				ResetMobSkillCooltime();
				m_pkMobInst->m_dwLastWarpTime = get_unix_ms_time();
				return;
			}

			// TODO 방향전환을 해서 덜 바보가 되자!
			if (number(0, 3) == 0)
			{
				if (__CHARACTER_GotoNearTarget(this, victim))
					return;
			}
		}
	}

	if (m_nBGTeamID)
	{
		// check has near enemy 
		auto pkCharNearEnemy = FindBattlegroundMobVictim(this);

		if (pkCharNearEnemy && !pkCharNearEnemy->IsDead())
		{
			if (CanBeginFight())
			{
				BeginFight(pkCharNearEnemy);

				if (!IsState(m_stateBattle))
				{
					GotoState(m_stateBattle);
					m_dwStateDuration = 1;
				}

				// Attack(pkCharNearEnemy);
				return;
			}
		}
	}

	if (fRate >= 0.93f)
	{
		if (DoMovingWay())
			return;

		if (DoMoveBattlegroundMinnion())
			return;
	}

	if (1.0f == fRate)
	{
		if (IsPC())
		{
			sys_log(1, "도착 %s %d %d", GetName(), x, y);
			GotoState(m_stateIdle);
			StopStaminaConsume();
		}
		else
		{
			if (GetVictim() && !IsCoward())
			{
				GotoState(m_stateBattle);
				m_dwStateDuration = 1;
			}
			else
			{
				GotoState(m_stateIdle);

				//LPCHARACTER rider = GetRider();

				if (GetBattlegroundTeamID() && IsMonster())
					CBattlegroundManager::Instance().OnSwitchIdle(this);

				m_dwStateDuration = PASSES_PER_SEC(number(1, 3));
			}
		}
	}
}

void CHARACTER::StateBattle()
{
	if (IsStone())
	{
		sys_err("Stone must not use battle state (name %s)", GetName());
		return;
	}

	if (IsPC())
    {
		// Cooldown attacks
		GetAbuseController()->SuspiciousAttackCooldown();
		return;
	}

	if (!CanMove())
		return;

	if (IsStun())
		return;

	LPCHARACTER victim = GetVictim();

	if (IsCoward())
	{
		if (IsDead())
			return;

		SetVictim(nullptr);

		if (number(1, 50) != 1)
		{
			GotoState(m_stateIdle);
			m_dwStateDuration = 1;
		}
		else
			CowardEscape();

		return;
	}

	if (!victim || (victim->IsStun() && IsGuardNPC()) || victim->IsDead())
	{
		if (victim && victim->IsDead() &&
				!no_wander && IsAggressive() && (!GetParty() || GetParty()->GetLeader() == this))
		{
			LPCHARACTER new_victim = FindMobVictim(this, m_pkMobData->m_table.wAggressiveSight);

			SetVictim(new_victim);
			m_dwStateDuration = PASSES_PER_SEC(1);

			if (!new_victim)
			{
				if (IsMonster())
			{
				switch (GetMobBattleType())
				{
					case BATTLE_TYPE_MELEE:
					case BATTLE_TYPE_SUPER_POWER:
					case BATTLE_TYPE_SUPER_TANKER:
					case BATTLE_TYPE_POWER:
					case BATTLE_TYPE_TANKER:
						{
							float fx, fy;
							float fDist = number(400, 1500);

							GetDeltaByDegree(number(0, 359), fDist, &fx, &fy);

							if (SECTREE_MANAGER::Instance().IsMovablePosition(victim->GetMapIndex(),
										victim->GetX() + (int32_t) fx, 
										victim->GetY() + (int32_t) fy) && 
									SECTREE_MANAGER::Instance().IsMovablePosition(victim->GetMapIndex(),
										victim->GetX() + (int32_t) fx/2,
										victim->GetY() + (int32_t) fy/2))
							{
								float dx = victim->GetX() + fx;
								float dy = victim->GetY() + fy;

								SetRotation(GetDegreeFromPosition(dx, dy));

								if (Goto((int32_t) dx, (int32_t) dy))
								{
									sys_log(0, "KILL_AND_GO: %s distance %.1f", GetName(), fDist);
									SendMovePacket(FUNC_WAIT, 0, 0, 0, 0);
								}
							}
						}
				}
			}
			}
			return;
		}

		SetVictim(nullptr);

		if (IsGuardNPC())
			Return();

		if (IS_SET(GetAIFlag(), AIFLAG_NOMOVE) && IS_SET(GetAIFlag(), AIFLAG_AGGRESSIVE))
		{
			if (GetX() != m_posExit.x || GetY() != m_posExit.y)
			{
				SetRotationToXY(m_posExit.x, m_posExit.y);
				if (Goto(m_posExit.x, m_posExit.y))
					SendMovePacket(FUNC_WAIT, 0, 0, 0, 0);
			}
		}

		m_dwStateDuration = PASSES_PER_SEC(1);
		return;
	}

	if (IsSummonMonster() && !IsDead() && !IsStun())
	{
		if (!GetParty())
		{
			// 서몬해서 채워둘 파티를 만들어 둡니다.
			CPartyManager::Instance().CreateParty(this);
		}

		LPPARTY pParty = GetParty();
		bool bPct = !number(0, 3);

		if (bPct && pParty->CountMemberByVnum(GetSummonVnum()) < SUMMON_MONSTER_COUNT)
		{
			// 모자라는 녀석을 불러내 채웁시다.
			int32_t sx = GetX() - 300;
			int32_t sy = GetY() - 300;
			int32_t ex = GetX() + 300;
			int32_t ey = GetY() + 300;

			LPCHARACTER tch = CHARACTER_MANAGER::Instance().SpawnMobRange(GetSummonVnum(), GetMapIndex(), sx, sy, ex, ey, true, true);

			if (tch)
			{
				pParty->Join(tch->GetVID());
				pParty->Link(tch);
			}
		}
	}

	float fDist = DISTANCE_APPROX(GetX() - victim->GetX(), GetY() - victim->GetY());

	if (IS_SET(GetAIFlag(), AIFLAG_NOMOVE) && IS_SET(GetAIFlag(), AIFLAG_AGGRESSIVE))
	{
		float fDistFromBase = DISTANCE_APPROX(GetX() - m_posExit.x, GetY() - m_posExit.y);

		if (fDistFromBase > GetMobTable().wAggressiveSight || fDist >= 4000.0f)
		{
			SetVictim(nullptr);

			SetRotation(GetDegreeFromPositionXY(GetX(), GetY(), m_posExit.x, m_posExit.y));
			if (Goto(m_posExit.x, m_posExit.y))
				SendMovePacket(FUNC_WAIT, 0, 0, 0, 0);

			return;
		}
	}

#if 0
	// yerini degistir, idleye gecince victimi nexus yap ve nexusa olan uzunluk x ise nexusa yurut
	if (m_nBGTeamID)
	{
		if (fDist > 400.f)
		{
//			sys_err("far victim. dist %f", fDist);

			auto pkEnemyNexus = GetEnemyNexus();
			if (pkEnemyNexus)
			{
				if (GetVictim() != pkEnemyNexus)
					SetVictim(pkEnemyNexus);

				return;
			}
			else
			{
				SetVictim(nullptr);
			}

			LPCHARACTER pkChrProtege = GetProtege();
			if (pkChrProtege)
				if (DISTANCE_APPROX(GetX() - pkChrProtege->GetX(), GetY() - pkChrProtege->GetY()) > 1000)
					Follow(pkChrProtege, number(150, 400));		
		}
//		return;
	}
#endif

	if (!m_nBGTeamID && fDist >= 4000.0f)   // 40미터 이상 멀어지면 포기
	{
		SetVictim(nullptr);

		LPCHARACTER pkChrProtege = GetProtege();

		// 보호할 것(돌, 파티장) 주변으로 간다.
		if (pkChrProtege)
			if (DISTANCE_APPROX(GetX() - pkChrProtege->GetX(), GetY() - pkChrProtege->GetY()) > 1000)
				Follow(pkChrProtege, number(150, 400));

		return;
	}

	if (!m_nBGTeamID && fDist >= GetMobAttackRange() * 1.15)
	{
		__CHARACTER_GotoNearTarget(this, victim);
		return;
	}

	if (m_pkParty)
		m_pkParty->SendMessage(this, PM_ATTACKED_BY, 0, 0);

	if (2493 == m_pkMobData->m_table.dwVnum)
	{
		// 수룡(2493) 특수 처리
		m_dwStateDuration = BlueDragon_StateBattle(this);
		return;
	}

	uint32_t dwCurTime = get_unix_ms_time();
	uint32_t dwDuration = CalculateDuration(GetLimitPoint(POINT_ATT_SPEED), 2000);

	if ((dwCurTime - m_dwLastAttackTime) < dwDuration) // 2초 마다 공격해야 한다.
	{
		m_dwStateDuration = MAX(1, (passes_per_sec * (dwDuration - (dwCurTime - m_dwLastAttackTime)) / 1000));
		return;
	}

	if (IsBerserker() == true)
		if (GetHPPct() < m_pkMobData->m_table.bBerserkPoint)
			if (IsBerserk() != true)
				SetBerserk(true);

	if (IsGodSpeeder() == true)
		if (GetHPPct() < m_pkMobData->m_table.bGodSpeedPoint)
			if (IsGodSpeed() != true)
				SetGodSpeed(true);

	//
	// 몹 스킬 처리
	//
	if (HasMobSkill())
	{
		for (uint32_t iSkillIdx = 0; iSkillIdx < MOB_SKILL_MAX_NUM; ++iSkillIdx)
		{
			if (CanUseMobSkill(iSkillIdx))
			{
				SetRotationToXY(victim->GetX(), victim->GetY());

				if (UseMobSkill(iSkillIdx))
				{
					SendMovePacket(FUNC_MOB_SKILL, iSkillIdx, GetX(), GetY(), 0, dwCurTime);

					float fDuration = CMotionManager::Instance().GetMotionDuration(GetRaceNum(), MAKE_MOTION_KEY(MOTION_MODE_GENERAL, MOTION_SPECIAL_1 + iSkillIdx));
					m_dwStateDuration = (uint32_t) (fDuration == 0.0f ? PASSES_PER_SEC(2) : PASSES_PER_SEC(fDuration));

					if (g_bIsTestServer)
						sys_log(0, "USE_MOB_SKILL: %s idx %u motion %u duration %.0f", GetName(), iSkillIdx, MOTION_SPECIAL_1 + iSkillIdx, fDuration);

					return;
				}
			}
		}
	}

	if (!Attack(victim))    // 공격 실패라면? 왜 실패했지? TODO
		m_dwStateDuration = passes_per_sec / 2;
	else
	{
		// 적을 바라보게 만든다.
		SetRotationToXY(victim->GetX(), victim->GetY());

		SendMovePacket(FUNC_ATTACK, 0, GetX(), GetY(), 0, dwCurTime);

		float fDuration = CMotionManager::Instance().GetMotionDuration(GetRaceNum(), MAKE_MOTION_KEY(MOTION_MODE_GENERAL, MOTION_NORMAL_ATTACK));
		m_dwStateDuration = (uint32_t) (fDuration == 0.0f ? PASSES_PER_SEC(2) : PASSES_PER_SEC(fDuration));
	}
}

void CHARACTER::StateFlag()
{
	m_dwStateDuration = (uint32_t) PASSES_PER_SEC(0.5);

	CWarMap * pMap = GetWarMap();

	if (!pMap)
		return;

	FuncFindChrForFlag f(this);
	GetSectree()->ForEachAround(f);

	if (!f.m_pkChrFind)
		return;

	if (nullptr == f.m_pkChrFind->GetGuild())
		return;

	char buf[256];
	uint8_t idx;

	if (!pMap->GetTeamIndex(GetGuild()->GetID(), idx))
		return;

	f.m_pkChrFind->AddAffect(AFFECT_WAR_FLAG, POINT_NONE, GetGuild()->GetID(), idx == 0 ? AFF_WAR_FLAG1 : AFF_WAR_FLAG2, INFINITE_AFFECT_DURATION, 0, false);
	f.m_pkChrFind->AddAffect(AFFECT_WAR_FLAG, POINT_MOV_SPEED, 50 - f.m_pkChrFind->GetPoint(POINT_MOV_SPEED), 0, INFINITE_AFFECT_DURATION, 0, false);

	pMap->RemoveFlag(idx);

	snprintf(buf, sizeof(buf), LC_TEXT("%s 길드의 깃발을 %s 님이 획득하였습니다."), pMap->GetGuild(idx)->GetName(), f.m_pkChrFind->GetName());
	pMap->Notice(buf);
}

void CHARACTER::StateFlagBase()
{
	m_dwStateDuration = (uint32_t) PASSES_PER_SEC(0.5);

	FuncFindChrForFlagBase f(this);
	GetSectree()->ForEachAround(f);
}

void CHARACTER::StateHorse()
{
	float	START_FOLLOW_DISTANCE = 400.0f;		// 이 거리 이상 떨어지면 쫓아가기 시작함
	float	START_RUN_DISTANCE = 700.0f;		// 이 거리 이상 떨어지면 뛰어서 쫓아감.
	int32_t		MIN_APPROACH = 150;					// 최소 접근 거리
	int32_t		MAX_APPROACH = 300;					// 최대 접근 거리	

	uint32_t	STATE_DURATION = (uint32_t)PASSES_PER_SEC(0.5);	// 상태 지속 시간

	bool bDoMoveAlone = true;					// 캐릭터와 가까이 있을 때 혼자 여기저기 움직일건지 여부 -_-;
	bool bRun = true;							// 뛰어야 하나?

	if (IsDead())
		return;

	m_dwStateDuration = STATE_DURATION;

	LPCHARACTER victim = GetRider();

	// ! 아님 // 대상이 없는 경우 소환자가 직접 나를 클리어할 것임
	if (!victim)
	{
		M2_DESTROY_CHARACTER(this);
		return;
	}

	m_pkMobInst->m_posLastAttacked = GetXYZ();

	float fDist = DISTANCE_APPROX(GetX() - victim->GetX(), GetY() - victim->GetY());

	if (fDist >= START_FOLLOW_DISTANCE)
	{
		if (fDist > START_RUN_DISTANCE)
			SetNowWalking(!bRun);		// NOTE: 함수 이름보고 멈추는건줄 알았는데 SetNowWalking(false) 하면 뛰는거임.. -_-;

		Follow(victim, number(MIN_APPROACH, MAX_APPROACH));

		m_dwStateDuration = STATE_DURATION;
	}
	else if (bDoMoveAlone && (get_unix_ms_time() > m_dwLastAttackTime))
	{
		// wondering-.-
		m_dwLastAttackTime = get_unix_ms_time() + number(5000, 12000);

		SetRotation(number(0, 359));        // 방향은 랜덤으로 설정

		float fx, fy;
		float fDist = number(200, 400);

		GetDeltaByDegree(GetRotation(), fDist, &fx, &fy);

		// 느슨한 못감 속성 체크; 최종 위치와 중간 위치가 갈수없다면 가지 않는다.
		if (!(SECTREE_MANAGER::Instance().IsMovablePosition(GetMapIndex(), GetX() + (int32_t) fx, GetY() + (int32_t) fy) 
					&& SECTREE_MANAGER::Instance().IsMovablePosition(GetMapIndex(), GetX() + (int32_t) fx/2, GetY() + (int32_t) fy/2)))
			return;

		SetNowWalking(true);

		if (Goto(GetX() + (int32_t) fx, GetY() + (int32_t) fy))
			SendMovePacket(FUNC_WAIT, 0, 0, 0, 0);
	}
}

