#include "stdafx.h"
#include "utils.h"
#include "config.h"
#include "desc_client.h"
#include "desc_manager.h"
#include "char.h"
#include "char_manager.h"
#include "motion.h"
#include "affect.h"
#include "pvp.h"
#include "start_position.h"
#include "party.h"
#include "guild_manager.h"
#include "p2p.h"
#include "dungeon.h"
#include "messenger_manager.h"
#include "war_map.h"
#include "quest_manager.h"
#include "item_manager.h"
#include "mob_manager.h"
#include "dev_log.h"
#include "item.h"
#include "arena.h"
#include "buffer_manager.h"
#include "unique_item.h"
#include "threeway_war.h"
#include "log.h"
#include "gm.h"
#include "crypt_helper.h"
#include "gposition.h"
#include "shop.h"
#include "shop_manager.h"
#include "battleground.h"

ACMD(do_user_horse_ride)
{
	if (ch && CBattlegroundManager::Instance().HasBattleground())
		return;

	if (ch->IsObserverMode())
		return;

	if (ch->IsDead() || ch->IsStun())
		return;

	if (ch->IsHorseRiding() == false)
	{
		// 말이 아닌 다른탈것을 타고있다.
		if (ch->GetMountVnum())
		{
			ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("이미 탈것을 이용중입니다."));
			return;
		}

		if (ch->GetHorse() == nullptr)
		{
			ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("말을 먼저 소환해주세요."));
			return;
		}

		ch->StartRiding();
	}
	else
	{
		ch->StopRiding();
	}
}

ACMD(do_user_horse_back)
{
	if (ch->GetHorse() != nullptr)
	{
		ch->HorseSummon(false);
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("말을 돌려보냈습니다."));
	}
	else if (ch->IsHorseRiding() == true)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("말에서 먼저 내려야 합니다."));
	}
	else
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("말을 먼저 소환해주세요."));
	}
}

ACMD(do_user_horse_feed)
{
	// 개인상점을 연 상태에서는 말 먹이를 줄 수 없다.
	if (ch->GetMyShop())
		return;

	if (ch->GetHorse() == nullptr)
	{
		if (ch->IsHorseRiding() == false)
			ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("말을 먼저 소환해주세요."));
		else
			ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("말을 탄 상태에서는 먹이를 줄 수 없습니다."));
		return;
	}

	uint32_t dwFood = ch->GetHorseGrade() + 50054 - 1;

	if (ch->CountSpecifyItem(dwFood) > 0)
	{
		ch->RemoveSpecifyItem(dwFood, 1);
		ch->FeedHorse();
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("말에게 %s%s 주었습니다."), 
			ITEM_MANAGER::Instance().GetTable(dwFood)->szLocaleName,
			""
		);
	}
	else
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("%s 아이템이 필요합니다"), ITEM_MANAGER::Instance().GetTable(dwFood)->szLocaleName);
	}
}

#define MAX_REASON_LEN		128

EVENTINFO(TimedEventInfo)
{
	DynamicCharacterPtr ch;
	int32_t		subcmd;
	int32_t         	left_second;
	char		szReason[MAX_REASON_LEN];

	TimedEventInfo() 
	: ch()
	, subcmd( 0 )
	, left_second( 0 )
	{
		::memset( szReason, 0, MAX_REASON_LEN );
	}
};

struct SendDisconnectFunc
{
	void operator () (LPDESC d)
	{
		if (d->GetCharacter())
		{
			if (d->GetCharacter()->GetGMLevel() == GM_PLAYER)
				d->GetCharacter()->ChatPacket(CHAT_TYPE_COMMAND, "quit Shutdown(SendDisconnectFunc)");
		}
	}
};

struct DisconnectFunc
{
	void operator () (LPDESC d)
	{
		if (d->GetType() == DESC_TYPE_CONNECTOR)
			return;

		if (d->IsPhase(PHASE_P2P))
			return;

		if (d->GetCharacter())
			d->GetCharacter()->Disconnect("Shutdown(DisconnectFunc)");

		d->SetPhase(PHASE_CLOSE);
	}
};

EVENTINFO(shutdown_event_data)
{
	int32_t seconds;

	shutdown_event_data()
	: seconds( 0 )
	{
	}
};

EVENTFUNC(shutdown_event)
{
	shutdown_event_data* info = dynamic_cast<shutdown_event_data*>( event->info );

	if ( info == nullptr )
	{
		sys_err( "shutdown_event> <Factor> Null pointer" );
		return 0;
	}

	int32_t * pSec = & (info->seconds);

	if (*pSec < 0)
	{
		sys_log(0, "shutdown_event sec %d", *pSec);

		if (--*pSec == -10)
		{
			const DESC_MANAGER::DESC_SET & c_set_desc = DESC_MANAGER::Instance().GetClientSet();
			std::for_each(c_set_desc.begin(), c_set_desc.end(), DisconnectFunc());
			return passes_per_sec;
		}
		else if (*pSec < -10)
			return 0;

		return passes_per_sec;
	}
	else if (*pSec == 0)
	{
		const DESC_MANAGER::DESC_SET & c_set_desc = DESC_MANAGER::Instance().GetClientSet();
		std::for_each(c_set_desc.begin(), c_set_desc.end(), SendDisconnectFunc());
		g_bNoMoreClient = true;
		--*pSec;
		return passes_per_sec;
	}
	else
	{
		char buf[64];
		snprintf(buf, sizeof(buf), LC_TEXT("셧다운이 %d초 남았습니다."), *pSec);
		SendNotice(buf);

		--*pSec;
		return passes_per_sec;
	}
}

void Shutdown(int32_t iSec)
{
	if (g_bNoMoreClient)
	{
		thecore_shutdown();
		return;
	}

	CBattlegroundManager::Instance().Destroy();
	CWarMapManager::Instance().OnShutdown();

	char buf[64];
	snprintf(buf, sizeof(buf), LC_TEXT("%d초 후 게임이 셧다운 됩니다."), iSec);

	SendNotice(buf);

	shutdown_event_data* info = AllocEventInfo<shutdown_event_data>();
	info->seconds = iSec;

	event_create(shutdown_event, info, 1);
}

ACMD(do_shutdown)
{
	char arg1[256];
	one_argument(argument, arg1, sizeof(arg1));

	if (!*arg1 || (*arg1 && strcmp(arg1, "force")))
	{
		if (ch && CBattlegroundManager::Instance().HasBattleground())
		{
			ch->ChatPacket(CHAT_TYPE_INFO, "You can not shutdown while running any battleground! Use force param.");
			CBattlegroundManager::Instance().BlockCreateBattleground();
			return;
		}
	}

	if (ch)
	{
		sys_err("Accept shutdown command from %s.", ch->GetName());

		TPacketGGShutdown p;
		p.bHeader = HEADER_GG_SHUTDOWN;
		P2P_MANAGER::Instance().Send(&p, sizeof(TPacketGGShutdown));

		Shutdown(10);
	}
}

EVENTFUNC(timed_event)
{
	TimedEventInfo * info = dynamic_cast<TimedEventInfo *>( event->info );
	
	if ( info == nullptr )
	{
		sys_err( "timed_event> <Factor> Null pointer" );
		return 0;
	}

	LPCHARACTER	ch = info->ch;
	if (ch == nullptr) { // <Factor>
		return 0;
	}
	LPDESC d = ch->GetDesc();

	if (info->left_second <= 0)
	{
		ch->m_pkTimedEvent = nullptr;

		switch (info->subcmd)
		{
			case SCMD_LOGOUT:
			case SCMD_QUIT:
			case SCMD_PHASE_SELECT:
				{
					TPacketNeedLoginLogInfo acc_info;
					acc_info.dwPlayerID = ch->GetDesc()->GetAccountTable().id;

					db_clientdesc->DBPacket( HEADER_GD_VALID_LOGOUT, 0, &acc_info, sizeof(acc_info) );

					LogManager::Instance().DetailLoginLog( false, ch );
				}
				break;
		}

		switch (info->subcmd)
		{
			case SCMD_LOGOUT:
				if (d)
					d->SetPhase(PHASE_CLOSE);
				break;

			case SCMD_QUIT:
				ch->ChatPacket(CHAT_TYPE_COMMAND, "quit");
				break;

			case SCMD_PHASE_SELECT:
				{
					ch->Disconnect("timed_event - SCMD_PHASE_SELECT");

					if (d)
					{
						d->SetPhase(PHASE_SELECT);
					}
				}
				break;
		}

		return 0;
	}
	else
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("%d초 남았습니다."), info->left_second);
		--info->left_second;
	}

	return PASSES_PER_SEC(1);
}

ACMD(do_cmd)
{
	/* RECALL_DELAY
	   if (ch->m_pkRecallEvent != nullptr)
	   {
	   ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("취소 되었습니다."));
	   event_cancel(&ch->m_pkRecallEvent);
	   return;
	   }
	// END_OF_RECALL_DELAY */

	if (ch->m_pkTimedEvent)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("취소 되었습니다."));
		event_cancel(&ch->m_pkTimedEvent);
		return;
	}

	switch (subcmd)
	{
		case SCMD_LOGOUT:
			ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("로그인 화면으로 돌아 갑니다. 잠시만 기다리세요."));
			break;

		case SCMD_QUIT:
			ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("게임을 종료 합니다. 잠시만 기다리세요."));
			break;

		case SCMD_PHASE_SELECT:
			ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("캐릭터를 전환 합니다. 잠시만 기다리세요."));
			break;
	}

	int32_t nExitLimitTime = 10;

	if (ch->IsHack(false, true, nExitLimitTime) &&
		false == CThreeWayWar::Instance().IsSungZiMapIndex(ch->GetMapIndex()) &&
	   	(!ch->GetWarMap() || ch->GetWarMap()->GetType() == GUILD_WAR_TYPE_FLAG))
	{
		return;
	}
	
	switch (subcmd)
	{
		case SCMD_LOGOUT:
		case SCMD_QUIT:
		case SCMD_PHASE_SELECT:
			{
				TimedEventInfo* info = AllocEventInfo<TimedEventInfo>();

				{
					if (ch->IsPosition(POS_FIGHTING))
						info->left_second = 10;
					else
						info->left_second = 3;
				}

				info->ch		= ch;
				info->subcmd		= subcmd;
				strlcpy(info->szReason, argument, sizeof(info->szReason));

				ch->m_pkTimedEvent	= event_create(timed_event, info, 1);
			}
			break;
	}
}

ACMD(do_fishing)
{
	if (ch && CBattlegroundManager::Instance().IsEventMap(ch->GetMapIndex()))
		return;
		
	char arg1[256];
	one_argument(argument, arg1, sizeof(arg1));

	if (!*arg1)
		return;

	ch->SetRotation(atof(arg1));
	ch->fishing();
}

ACMD(do_console)
{
	ch->ChatPacket(CHAT_TYPE_COMMAND, "ConsoleEnable");
}

ACMD(do_restart)
{
	static const int32_t iRestartHereSeconds = 170;
	static const int32_t iRestartTownSeconds = 173;

	if (false == ch->IsDead())
	{
		ch->ChatPacket(CHAT_TYPE_COMMAND, "CloseRestartWindow");
		ch->StartRecoveryEvent();
		return;
	}

	if (nullptr == ch->m_pkDeadEvent)
		return;

	int32_t iTimeToDead = (event_time(ch->m_pkDeadEvent) / passes_per_sec);

	if (CBattlegroundManager::Instance().IsEventMap(ch->GetMapIndex()))
	{
		CBattlegroundManager::Instance().OnRevive(ch, iTimeToDead);
		return;
	}

	if (iTimeToDead - (180 - g_nPortalLimitTime) < (g_nPortalLimitTime - 180 + 2))
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("You will be ported in your city soon."));
		return;
	}

	if (subcmd == SCMD_RESTART_BASE)
		return;

	if (iTimeToDead - (180 - g_nPortalLimitTime) > 0)
	{
		if (subcmd == SCMD_RESTART_HERE && (!ch->GetWarMap() || ch->GetWarMap()->GetType() == GUILD_WAR_TYPE_FLAG))
	{
		if (!g_bIsTestServer)
		{
			if (ch->IsHack())
			{
				//성지 맵일경우에는 체크 하지 않는다.
				if (false == CThreeWayWar::Instance().IsSungZiMapIndex(ch->GetMapIndex()))
				{
					ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("아직 재시작 할 수 없습니다. (%d초 남음)"), iTimeToDead - (180 - g_nPortalLimitTime));
					return;
				}
			}
				if (iTimeToDead > iRestartHereSeconds)
				{
					ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("아직 재시작 할 수 없습니다. (%d초 남음)"), iTimeToDead - iRestartHereSeconds);
					return;
				}
			}
		}
	}

	//PREVENT_HACK
	//DESC : 창고, 교환 창 후 포탈을 사용하는 버그에 이용될수 있어서
	//		쿨타임을 추가 
	if (subcmd == SCMD_RESTART_TOWN)
	{
		if (ch->IsHack())
		{
			//길드맵, 성지맵에서는 체크 하지 않는다.
			if ((!ch->GetWarMap() || ch->GetWarMap()->GetType() == GUILD_WAR_TYPE_FLAG) ||
			   	false == CThreeWayWar::Instance().IsSungZiMapIndex(ch->GetMapIndex()))
			{
				ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("아직 재시작 할 수 없습니다. (%d초 남음)"), iTimeToDead - (180 - g_nPortalLimitTime));
				return;
			}
		}

		if (iTimeToDead > iRestartTownSeconds)
		{
			ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("아직 마을에서 재시작 할 수 없습니다. (%d 초 남음)"), iTimeToDead - iRestartTownSeconds);
			return;
		}
	}
	//END_PREVENT_HACK

	ch->ChatPacket(CHAT_TYPE_COMMAND, "CloseRestartWindow");

	ch->GetDesc()->SetPhase(PHASE_GAME);
	ch->SetPosition(POS_STANDING);
	ch->StartRecoveryEvent();

	//FORKED_LOAD
	//DESC: 삼거리 전투시 부활을 할경우 맵의 입구가 아닌 삼거리 전투의 시작지점으로 이동하게 된다.
	if (1 == quest::CQuestManager::Instance().GetEventFlag("threeway_war"))
	{
		if (subcmd == SCMD_RESTART_TOWN || subcmd == SCMD_RESTART_HERE)
		{
			if (true == CThreeWayWar::Instance().IsThreeWayWarMapIndex(ch->GetMapIndex()) &&
					false == CThreeWayWar::Instance().IsSungZiMapIndex(ch->GetMapIndex()))
			{
				ch->WarpSet(EMPIRE_START_X(ch->GetEmpire()), EMPIRE_START_Y(ch->GetEmpire()));

				ch->ReviveInvisible(5);
				ch->PointChange(POINT_HP, ch->GetMaxHP() - ch->GetHP());
				ch->PointChange(POINT_SP, ch->GetMaxSP() - ch->GetSP());

				return;
			}

			//성지 
			if (true == CThreeWayWar::Instance().IsSungZiMapIndex(ch->GetMapIndex()))
			{
				if (CThreeWayWar::Instance().GetReviveTokenForPlayer(ch->GetPlayerID()) <= 0)
				{
					ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("성지에서 부활 기회를 모두 잃었습니다! 마을로 이동합니다!"));
					ch->WarpSet(EMPIRE_START_X(ch->GetEmpire()), EMPIRE_START_Y(ch->GetEmpire()));
				}
				else
				{
					ch->Show(ch->GetMapIndex(), GetSungziStartX(ch->GetEmpire()), GetSungziStartY(ch->GetEmpire()));
				}

				ch->PointChange(POINT_HP, ch->GetMaxHP() - ch->GetHP());
				ch->PointChange(POINT_SP, ch->GetMaxSP() - ch->GetSP());
				ch->ReviveInvisible(5);

				return;
			}
		}
	}
	//END_FORKED_LOAD

	if (ch->GetDungeon())
		ch->GetDungeon()->UseRevive(ch);

	if (ch->GetWarMap() && !ch->IsObserverMode())
	{
		CWarMap * pMap = ch->GetWarMap();
		uint32_t dwGuildOpponent = pMap ? pMap->GetGuildOpponent(ch) : 0;

		if (dwGuildOpponent)
		{
			switch (subcmd)
			{
				case SCMD_RESTART_TOWN:
				{
					sys_log(0, "do_restart: restart town");
					GPOS pos;

					if (CWarMapManager::Instance().GetStartPosition(ch->GetMapIndex(), ch->GetGuild()->GetID() < dwGuildOpponent ? 0 : 1, pos))
						ch->Show(ch->GetMapIndex(), pos.x, pos.y);
					else
						ch->ExitToSavedLocation();

					ch->PointChange(POINT_HP, ch->GetMaxHP() - ch->GetHP());
					ch->PointChange(POINT_SP, ch->GetMaxSP() - ch->GetSP());
					ch->ReviveInvisible(5);
				} break;

				case SCMD_RESTART_HERE:
				{
					sys_log(0, "do_restart: restart here");
					ch->RestartAtSamePos();
					//ch->Show(ch->GetMapIndex(), ch->GetX(), ch->GetY());
					ch->PointChange(POINT_HP, ch->GetMaxHP() - ch->GetHP());
					ch->PointChange(POINT_SP, ch->GetMaxSP() - ch->GetSP());
					ch->ReviveInvisible(5);
				} break;
			}

			return;
		}
	}

	switch (subcmd)
	{
		case SCMD_RESTART_TOWN:
		{
			sys_log(0, "do_restart: restart town");
			GPOS pos;

			if (SECTREE_MANAGER::Instance().GetRecallPositionByEmpire(ch->GetMapIndex(), ch->GetEmpire(), pos))
				ch->WarpSet(pos.x, pos.y);
			else
				ch->GoHome();

			ch->PointChange(POINT_HP, 50 - ch->GetHP());
			ch->DeathPenalty(1);
		} break;

		case SCMD_RESTART_HERE:
		{
			sys_log(0, "do_restart: restart here");

			ch->RestartAtSamePos();
			//ch->Show(ch->GetMapIndex(), ch->GetX(), ch->GetY());

			ch->PointChange(POINT_HP, 50 - ch->GetHP());
			ch->DeathPenalty(0);
			ch->ReviveInvisible(5);
		} break;
	}
}

#define MAX_STAT g_iStatusPointSetMaxValue

ACMD(do_stat_reset)
{
	ch->PointChange(POINT_STAT_RESET_COUNT, 12 - ch->GetPoint(POINT_STAT_RESET_COUNT));
}

ACMD(do_stat_minus)
{
	char arg1[256];
	one_argument(argument, arg1, sizeof(arg1));

	if (!*arg1)
		return;

	if (ch->IsPolymorphed())
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("둔갑 중에는 능력을 올릴 수 없습니다."));
		return;
	}

	if (ch->GetPoint(POINT_STAT_RESET_COUNT) <= 0)
		return;

	if (!strcmp(arg1, "st"))
	{
		if (ch->GetRealPoint(POINT_ST) <= JobInitialPoints[ch->GetJob()].st)
			return;

		ch->SetRealPoint(POINT_ST, ch->GetRealPoint(POINT_ST) - 1);
		ch->SetPoint(POINT_ST, ch->GetPoint(POINT_ST) - 1);
		ch->ComputePoints();
		ch->PointChange(POINT_ST, 0);
	}
	else if (!strcmp(arg1, "dx"))
	{
		if (ch->GetRealPoint(POINT_DX) <= JobInitialPoints[ch->GetJob()].dx)
			return;

		ch->SetRealPoint(POINT_DX, ch->GetRealPoint(POINT_DX) - 1);
		ch->SetPoint(POINT_DX, ch->GetPoint(POINT_DX) - 1);
		ch->ComputePoints();
		ch->PointChange(POINT_DX, 0);
	}
	else if (!strcmp(arg1, "ht"))
	{
		if (ch->GetRealPoint(POINT_HT) <= JobInitialPoints[ch->GetJob()].ht)
			return;

		ch->SetRealPoint(POINT_HT, ch->GetRealPoint(POINT_HT) - 1);
		ch->SetPoint(POINT_HT, ch->GetPoint(POINT_HT) - 1);
		ch->ComputePoints();
		ch->PointChange(POINT_HT, 0);
		ch->PointChange(POINT_MAX_HP, 0);
	}
	else if (!strcmp(arg1, "iq"))
	{
		if (ch->GetRealPoint(POINT_IQ) <= JobInitialPoints[ch->GetJob()].iq)
			return;

		ch->SetRealPoint(POINT_IQ, ch->GetRealPoint(POINT_IQ) - 1);
		ch->SetPoint(POINT_IQ, ch->GetPoint(POINT_IQ) - 1);
		ch->ComputePoints();
		ch->PointChange(POINT_IQ, 0);
		ch->PointChange(POINT_MAX_SP, 0);
	}
	else
		return;

	ch->PointChange(POINT_STAT, +1);
	ch->PointChange(POINT_STAT_RESET_COUNT, -1);
	ch->ComputePoints();
}


ACMD(do_search_bg)
{
	char arg1[256], arg2[256], arg3[256];

	auto line = two_arguments(argument, arg1, sizeof(arg1), arg2, sizeof(arg2));
	one_argument(line, arg3, sizeof(arg3));

	if (!*arg1 || !*arg2 || !*arg3)
		return;

	uint8_t nGameMode = 0;
	str_to_number(nGameMode, arg1);

	uint8_t nGameType = 0;
	str_to_number(nGameType, arg2);

	uint8_t nQueueType = 0;
	str_to_number(nQueueType, arg3);


	CBattlegroundManager::Instance().SearchBattleground(ch, nGameMode, nGameType, nQueueType);
}

ACMD(do_recv_bg_details)
{
	if (CBattlegroundManager::Instance().IsStarted() == false)
	{
		ch->ChatPacket(CHAT_TYPE_COMMAND, "bg_details 0");
		ch->ChatPacket(CHAT_TYPE_INFO, "Battleground is not active yet");
		return;
	}
	ch->ChatPacket(CHAT_TYPE_COMMAND, "bg_details %u", CBattlegroundManager::Instance().GetBattlegroundJoinType());
}

ACMD(do_recv_create_bg_details)
{
	auto arrReqItems = CBattlegroundManager::Instance().GetBattlegroundRequiredItems();

	ch->ChatPacket(CHAT_TYPE_COMMAND, "bg_create_ret %u %u %u", arrReqItems.at(0), arrReqItems.at(1), arrReqItems.at(2));
}

ACMD(do_create_battleground)
{
	char arg1[256], arg2[256], arg3[256];

	auto line = two_arguments(argument, arg1, sizeof(arg1), arg2, sizeof(arg2));
	one_argument(line, arg3, sizeof(arg3));

	if (!*arg1 || !*arg2 || !*arg3)
		return;

	uint8_t nGameMode = 0;
	str_to_number(nGameMode, arg1);

	uint8_t nGameType = 0;
	str_to_number(nGameType, arg2);

	uint8_t nQueueType = 0;
	str_to_number(nQueueType, arg3);

	if (CBattlegroundManager::Instance().IsBattlegroundCreateBlocked())
	{
		ch->ChatPacket(CHAT_TYPE_INFO, "Battleground create blocked by server administration");
		return;		
	}

	auto arrReqItems = CBattlegroundManager::Instance().GetBattlegroundRequiredItems();
	auto dwReqItem1 = arrReqItems.at(0);
	auto dwReqItem2 = arrReqItems.at(1);
	auto dwReqItem3 = arrReqItems.at(2);
	
	if (dwReqItem1 && 0 == ch->CountSpecifyItem(dwReqItem1))
	{
		ch->ChatPacket(CHAT_TYPE_INFO, "Yetersiz esya: %u", dwReqItem1);
		return;
	}
	if (dwReqItem2 && 0 == ch->CountSpecifyItem(dwReqItem2))
	{
		ch->ChatPacket(CHAT_TYPE_INFO, "Yetersiz esya: %u", dwReqItem2);
		return;
	}
	if (dwReqItem3 && 0 == ch->CountSpecifyItem(dwReqItem3))
	{
		ch->ChatPacket(CHAT_TYPE_INFO, "Yetersiz esya: %u", dwReqItem3);
		return;
	}

	if (dwReqItem1)
		ch->RemoveSpecifyItem(dwReqItem1);
	if (dwReqItem2)
		ch->RemoveSpecifyItem(dwReqItem2);
	if (dwReqItem3)
		ch->RemoveSpecifyItem(dwReqItem3);


	auto dwRoomID = CBattlegroundManager::Instance().CreateBattlegroundRoom(ch, nGameMode, nGameType, nQueueType);
	if (!dwRoomID)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, "Battleground room can not created!");
		return;
	}

	auto pkBgRoom = CBattlegroundManager::Instance().FindBattlegroundRoom(dwRoomID);
	if (pkBgRoom)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, "Battleground room succesfully created!");
		pkBgRoom->SendRoomDetails(ch);
	}
}

ACMD(do_join_bg_queue)
{
	char arg1[256], arg2[256], arg3[256];

	auto line = two_arguments(argument, arg1, sizeof(arg1), arg2, sizeof(arg2));
	one_argument(line, arg3, sizeof(arg3));

	if (!*arg1 || !*arg2 || !*arg3)
		return;

	uint8_t nGameMode = 0;
	str_to_number(nGameMode, arg1);

	uint8_t nGameType = 0;
	str_to_number(nGameType, arg2);

	uint8_t nQueueType = 0;
	str_to_number(nQueueType, arg3);


	auto bRet = CBattlegroundManager::Instance().JoinBattlegroundQueue(ch->GetPlayerID(), nGameMode, nGameType, nQueueType);
	ch->ChatPacket(CHAT_TYPE_COMMAND, "bg_queue_ret %d", bRet ? 1 : 0);
}

ACMD(do_bg_start)
{
	auto pkRoom = CBattlegroundManager::Instance().FindBattlegroundRoomByLeader(ch->GetPlayerID());
	if (pkRoom)
	{
		pkRoom->StartBattleground();
	}
}

ACMD(do_bg_join_room)
{
	char arg1[256];
	one_argument(argument, arg1, sizeof(arg1));

	if (!*arg1)
		return;

	uint32_t dwRoomID = 0;
	str_to_number(dwRoomID, arg1);

	auto pkRoom = CBattlegroundManager::Instance().FindBattlegroundRoom(dwRoomID);
	if (pkRoom)
	{
		if (pkRoom->JoinToRoom(ch->GetPlayerID(), number(BG_TEAM_BLUE, BG_TEAM_RED)))
			pkRoom->SendRoomDetails(ch);
	}
}

ACMD(do_bg_join_team)
{
	char arg1[256];
	one_argument(argument, arg1, sizeof(arg1));

	if (!*arg1)
		return;

	uint8_t nTeamID = 0;
	str_to_number(nTeamID, arg1);

	auto pkRoom = CBattlegroundManager::Instance().FindBattlegroundRoomByAttender(ch->GetPlayerID());
	if (pkRoom)
	{
		pkRoom->SetTeamID(ch->GetPlayerID(), nTeamID);
	}
}

ACMD(do_bg_change_state)
{
	char arg1[256];
	one_argument(argument, arg1, sizeof(arg1));

	if (!*arg1)
		return;

	uint8_t nNewState = 0;
	str_to_number(nNewState, arg1);

	auto pkRoom = CBattlegroundManager::Instance().FindBattlegroundRoomByAttender(ch->GetPlayerID());
	if (pkRoom)
	{
		pkRoom->SetAttenderState(ch->GetPlayerID(), nNewState);
	}
}

ACMD(do_bg_leaved)
{
	auto pkRoom = CBattlegroundManager::Instance().FindBattlegroundRoomByAttender(ch->GetPlayerID());
	if (pkRoom)
	{
		pkRoom->LeaveFromRoom(ch->GetPlayerID());
	}	
}

ACMD(do_bg_kick)
{
	char arg1[256];
	one_argument(argument, arg1, sizeof(arg1));

	if (!*arg1)
		return;

	auto pkRoom = CBattlegroundManager::Instance().FindBattlegroundRoomByLeader(ch->GetPlayerID());
	if (pkRoom)
	{
		pkRoom->KickPlayer(arg1);
	}
}


ACMD(do_bg_test)
{
	char arg1[256], arg2[256], arg3[256];

	auto line = two_arguments(argument, arg1, sizeof(arg1), arg2, sizeof(arg2));
	one_argument(line, arg3, sizeof(arg3));

	uint32_t narg1 = 0;
	str_to_number(narg1, arg1);

	uint32_t narg2 = 0;
	str_to_number(narg2, arg2);

	uint32_t narg3 = 0;
	str_to_number(narg3, arg3);

	switch (narg1)
	{
		case 1: // create map index
		{
			auto lMapIndex = SECTREE_MANAGER::Instance().CreatePrivateMap(BATTLEGROUND_NORMAL_MAP_INDEX);
			if (!lMapIndex)
			{
				sys_err("Battleground dungeon can not created!");
				return;
			}
			ch->ChatPacket(CHAT_TYPE_INFO, "Map index %d", lMapIndex);
		} break;

		case 2: // create battleground
		{

		} break;

		case 3: // enter to queue
		{

		} break;

		case 4: // manually close queue
		{

		} break;

		case 5: // start event
		{
			if (CBattlegroundManager::Instance().StartEvent(narg2) == false)
			{
				sys_err("Battleground can not started!");
				return;			
			}
		} break;

		case 6: // close event & battleground
		{
			CBattlegroundManager::Instance().CloseEvent(narg2);
			CBattlegroundManager::Instance().Destroy();
		} break;

#if 0
		case 1:
		{
			auto mob1 = CHARACTER_MANAGER::Instance().SpawnMob(301, 218, 9516500, 9524000, 0, true);
			if (mob1)
				ch->ChatPacket(CHAT_TYPE_INFO, "vmob1 id: %u", mob1->GetVID());		
		} break;

		case 2:
		{
			sys_err("%u -> %u", narg2, narg3);

			auto attacker = CHARACTER_MANAGER::Instance().Find(narg2);
			auto victim = CHARACTER_MANAGER::Instance().Find(narg3);

			attacker->Attack(victim);
		} break;

		case 3:
		{
			/*
			typedef struct SNPCMovingPosition
{
	int32_t	lX;
	int32_t	lY;
	bool	bRun;
} TNPCMovingPosition;
			 */
			// 		void			SetMovingWay(const TNPCMovingPosition* pWay, int32_t iMaxNum, bool bRepeat = false, bool bLocal = false);
			auto charr = CHARACTER_MANAGER::Instance().Find(narg2);

			///*
			TNPCMovingPosition cmd;
			cmd.lX = 120;
			cmd.lY = 120;
			cmd.bRun = true;

			charr->SetMovingWay(&cmd, 1, false, true);
			//*/

			//charr->SetNowWalking(false);
			//if (charr->Goto(lX * 100, lY * 100))
			//	charr->SendMovePacket(FUNC_WAIT, 0, 0, 0, 0);
		} break;
#endif

		default:
			break;
	};
}

ACMD(do_stat)
{
	char arg1[256];
	one_argument(argument, arg1, sizeof(arg1));

	if (!*arg1)
		return;

	if (ch->IsPolymorphed())
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("둔갑 중에는 능력을 올릴 수 없습니다."));
		return;
	}

	if (ch->GetPoint(POINT_STAT) <= 0)
		return;

	uint8_t idx = 0;
	
	if (!strcmp(arg1, "st"))
		idx = POINT_ST;
	else if (!strcmp(arg1, "dx"))
		idx = POINT_DX;
	else if (!strcmp(arg1, "ht"))
		idx = POINT_HT;
	else if (!strcmp(arg1, "iq"))
		idx = POINT_IQ;
	else
		return;

	if (ch->GetRealPoint(idx) >= MAX_STAT)
		return;

	ch->SetRealPoint(idx, ch->GetRealPoint(idx) + 1);
	ch->SetPoint(idx, ch->GetPoint(idx) + 1);
	ch->ComputePoints();
	ch->PointChange(idx, 0);

	if (idx == POINT_IQ)
	{
		ch->PointChange(POINT_MAX_HP, 0);
	}
	else if (idx == POINT_HT)
	{
		ch->PointChange(POINT_MAX_SP, 0);
	}

	ch->PointChange(POINT_STAT, -1);
	ch->ComputePoints();
}

ACMD(do_pvp)
{
	if (ch && CBattlegroundManager::Instance().IsEventMap(ch->GetMapIndex()))
		return;

	if (ch->GetArena() != nullptr || CArenaManager::Instance().IsArenaMap(ch->GetMapIndex()) == true)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("대련장에서 사용하실 수 없습니다."));
		return;
	}

	char arg1[256];
	one_argument(argument, arg1, sizeof(arg1));

	uint32_t vid = 0;
	str_to_number(vid, arg1);
	LPCHARACTER pkVictim = CHARACTER_MANAGER::Instance().Find(vid);

	if (!pkVictim)
		return;

	if (pkVictim->IsNPC())
		return;

	if (pkVictim->GetArena() != nullptr)
	{
		pkVictim->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("상대방이 대련중입니다."));
		return;
	}

	CPVPManager::Instance().Insert(ch, pkVictim);
}

ACMD(do_deny_pvp)
{
	char arg1[256];
	one_argument(argument, arg1, sizeof(arg1));

	if (!*arg1)
		return;

	uint32_t vid = 0;
	str_to_number(vid, arg1);
	LPCHARACTER pkVictim = CHARACTER_MANAGER::Instance().Find(vid);

	if (!pkVictim)
		return;

	if (!pkVictim->IsPC())
		return;

	CPVPManager::Instance().Reject(ch, pkVictim);
}

ACMD(do_guildskillup)
{
	char arg1[256];
	one_argument(argument, arg1, sizeof(arg1));

	if (!*arg1)
		return;

	if (!ch->GetGuild())
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("<길드> 길드에 속해있지 않습니다."));
		return;
	}

	CGuild* g = ch->GetGuild();
	TGuildMember* gm = g->GetMember(ch->GetPlayerID());
	if (gm->grade == GUILD_LEADER_GRADE)
	{
		uint32_t vnum = 0;
		str_to_number(vnum, arg1);
		g->SkillLevelUp(vnum);
	}
	else
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("<길드> 길드 스킬 레벨을 변경할 권한이 없습니다."));
	}
}

ACMD(do_skillup)
{
	char arg1[256];
	one_argument(argument, arg1, sizeof(arg1));

	if (!*arg1)
		return;

	uint32_t vnum = 0;
	str_to_number(vnum, arg1);

	if (true == ch->CanUseSkill(vnum))
	{
		ch->SkillLevelUp(vnum);
	}
	else
	{
		switch(vnum)
		{
			case SKILL_HORSE_WILDATTACK:
			case SKILL_HORSE_CHARGE:
			case SKILL_HORSE_ESCAPE:
			case SKILL_HORSE_WILDATTACK_RANGE:

				/*
			case SKILL_ADD_HP:
			case SKILL_RESIST_PENETRATE:
				*/
				ch->SkillLevelUp(vnum);
				break;
		}
	}
}

//
// @version	05/06/20 Bang2ni - 커맨드 처리 Delegate to CHARACTER class
//
ACMD(do_safebox_close)
{
	ch->CloseSafebox();
}

//
// @version	05/06/20 Bang2ni - 커맨드 처리 Delegate to CHARACTER class
//
ACMD(do_safebox_password)
{
	if (ch && CBattlegroundManager::Instance().IsEventMap(ch->GetMapIndex()))
		return;

	char arg1[256];
	one_argument(argument, arg1, sizeof(arg1));
	ch->ReqSafeboxLoad(arg1);
}

ACMD(do_safebox_change_password)
{
	if (!ch || !ch->GetDesc())
		return;

	char arg1[256];
	char arg2[256];

	two_arguments(argument, arg1, sizeof(arg1), arg2, sizeof(arg2));

	if (!*arg1 || strlen(arg1)>6)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("<창고> 잘못된 암호를 입력하셨습니다."));
		return;
	}

	if (!*arg2 || strlen(arg2)>6)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("<창고> 잘못된 암호를 입력하셨습니다."));
		return;
	}

	TSafeboxChangePasswordPacket p;

	p.dwID = ch->GetDesc()->GetAccountTable().id;
	strlcpy(p.szOldPassword, arg1, sizeof(p.szOldPassword));
	strlcpy(p.szNewPassword, arg2, sizeof(p.szNewPassword));

	db_clientdesc->DBPacket(HEADER_GD_SAFEBOX_CHANGE_PASSWORD, ch->GetDesc()->GetHandle(), &p, sizeof(p));
}

ACMD(do_mall_password)
{
	if (ch && CBattlegroundManager::Instance().IsEventMap(ch->GetMapIndex()))
		return;

	if (!ch || !ch->GetDesc())
		return;

	char arg1[256];
	one_argument(argument, arg1, sizeof(arg1));

	if (!*arg1 || strlen(arg1) > 6)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("<창고> 잘못된 암호를 입력하셨습니다."));
		return;
	}

	int32_t iPulse = thecore_pulse();

	if (ch->GetMall())
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("<창고> 창고가 이미 열려있습니다."));
		return;
	}

	if (iPulse - ch->GetMallLoadTime() < passes_per_sec * 10) // 10초에 한번만 요청 가능
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("<창고> 창고를 닫은지 10초 안에는 열 수 없습니다."));
		return;
	}

	ch->SetMallLoadTime(iPulse);

	TSafeboxLoadPacket p;
	p.dwID = ch->GetDesc()->GetAccountTable().id;
	strlcpy(p.szLogin, ch->GetDesc()->GetAccountTable().login, sizeof(p.szLogin));
	strlcpy(p.szPassword, arg1, sizeof(p.szPassword));

	db_clientdesc->DBPacket(HEADER_GD_MALL_LOAD, ch->GetDesc()->GetHandle(), &p, sizeof(p));
}

ACMD(do_mall_close)
{
	if (ch->GetMall())
	{
		ch->SetMallLoadTime(thecore_pulse());
		ch->CloseMall();
		ch->Save();
	}
}

ACMD(do_ungroup)
{
	if (!ch->GetParty())
		return;

	if (!CPartyManager::Instance().IsEnablePCParty())
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("<파티> 서버 문제로 파티 관련 처리를 할 수 없습니다."));
		return;
	}

	if (ch->GetDungeon())
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("<파티> 던전 안에서는 파티에서 나갈 수 없습니다."));
		return;
	}

	LPPARTY pParty = ch->GetParty();

	if (pParty->GetMemberCount() == 2)
	{
		// party disband
		CPartyManager::Instance().DeleteParty(pParty);
	}
	else
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("<파티> 파티에서 나가셨습니다."));
		//pParty->SendPartyRemoveOneToAll(ch);
		pParty->Quit(ch->GetPlayerID());
		//pParty->SendPartyRemoveAllToOne(ch);
	}
}

ACMD(do_close_shop)
{
	if (ch->GetMyShop())
	{
		ch->CloseMyShop();
		return;
	}
}

ACMD(do_set_walk_mode)
{
	ch->SetNowWalking(true);
	ch->SetWalking(true);
}

ACMD(do_set_run_mode)
{
	ch->SetNowWalking(false);
	ch->SetWalking(false);
}

ACMD(do_war)
{
	if (ch && CBattlegroundManager::Instance().IsEventMap(ch->GetMapIndex()))
		return;

	//내 길드 정보를 얻어오고
	CGuild * g = ch->GetGuild();

	if (!g)
		return;

	//전쟁중인지 체크한번!
	if (g->UnderAnyWar())
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("<길드> 이미 다른 전쟁에 참전 중 입니다."));
		return;
	}

	//파라메터를 두배로 나누고
	char arg1[256], arg2[256];
	uint8_t type = GUILD_WAR_TYPE_FIELD; //fixme102 base int32_t modded uint
	two_arguments(argument, arg1, sizeof(arg1), arg2, sizeof(arg2));

	if (!*arg1)
		return;

	if (*arg2)
	{
		str_to_number(type, arg2);

		if (type >= GUILD_WAR_TYPE_MAX_NUM)
			type = GUILD_WAR_TYPE_FIELD;
	}

	//마스터인지 체크(길전은 길드장만이 가능)
	if (g->GetMasterPID() != ch->GetPlayerID())
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("<길드> 길드전에 대한 권한이 없습니다."));
		return;
	}

	//상대 길드를 얻어오고
	CGuild * opp_g = CGuildManager::Instance().FindGuildByName(arg1);

	if (!opp_g)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("<길드> 그런 길드가 없습니다."));
		return;
	}

	//상대길드와의 상태 체크
	switch (g->GetGuildWarState(opp_g->GetID()))
	{
		case GUILD_WAR_NONE:
			{
				if (opp_g->UnderAnyWar())
				{
					ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("<길드> 상대방 길드가 이미 전쟁 중 입니다."));
					return;
				}

				int32_t iWarPrice = KOR_aGuildWarInfo[type].iWarPrice;

				if (g->GetGuildMoney() < iWarPrice)
				{
					ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("<길드> 전비가 부족하여 길드전을 할 수 없습니다."));
					return;
				}

				if (opp_g->GetGuildMoney() < iWarPrice)
				{
					ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("<길드> 상대방 길드의 전비가 부족하여 길드전을 할 수 없습니다."));
					return;
				}
			}
			break;

		case GUILD_WAR_SEND_DECLARE:
			{
				ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("이미 선전포고 중인 길드입니다."));
				return;
			}
			break;

		case GUILD_WAR_RECV_DECLARE:
			{
				if (opp_g->UnderAnyWar())
				{
					ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("<길드> 상대방 길드가 이미 전쟁 중 입니다."));
					g->RequestRefuseWar(opp_g->GetID());
					return;
				}
			}
			break;

		case GUILD_WAR_RESERVE:
			{
				ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("<길드> 이미 전쟁이 예약된 길드 입니다."));
				return;
			}
			break;

		case GUILD_WAR_END:
			return;

		default:
			ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("<길드> 이미 전쟁 중인 길드입니다."));
			g->RequestRefuseWar(opp_g->GetID());
			return;
	}

	if (!g->CanStartWar(type))
	{
		// 길드전을 할 수 있는 조건을 만족하지않는다.
		if (g->GetLadderPoint() == 0)
		{
			ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("<길드> 레더 점수가 모자라서 길드전을 할 수 없습니다."));
			sys_log(0, "GuildWar.StartError.NEED_LADDER_POINT");
		}
		else if (g->GetMemberCount() < GUILD_WAR_MIN_MEMBER_COUNT)
		{
			ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("<길드> 길드전을 하기 위해선 최소한 %d명이 있어야 합니다."), GUILD_WAR_MIN_MEMBER_COUNT);
			sys_log(0, "GuildWar.StartError.NEED_MINIMUM_MEMBER[%d]", GUILD_WAR_MIN_MEMBER_COUNT);
		}
		else
		{
			sys_log(0, "GuildWar.StartError.UNKNOWN_ERROR");
		}
		return;
	}

	// 필드전 체크만 하고 세세한 체크는 상대방이 승낙할때 한다.
	if (!opp_g->CanStartWar(GUILD_WAR_TYPE_FIELD))
	{
		if (opp_g->GetLadderPoint() == 0)
			ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("<길드> 상대방 길드의 레더 점수가 모자라서 길드전을 할 수 없습니다."));
		else if (opp_g->GetMemberCount() < GUILD_WAR_MIN_MEMBER_COUNT)
			ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("<길드> 상대방 길드의 길드원 수가 부족하여 길드전을 할 수 없습니다."));
		return;
	}

	do
	{
		if (g->GetMasterCharacter() != nullptr)
			break;

		CCI *pCCI = P2P_MANAGER::Instance().FindByPID(g->GetMasterPID());

		if (pCCI != nullptr)
			break;

		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("<길드> 상대방 길드의 길드장이 접속중이 아닙니다."));
		g->RequestRefuseWar(opp_g->GetID());
		return;

	} while (false);

	do
	{
		if (opp_g->GetMasterCharacter() != nullptr)
			break;
		
		CCI *pCCI = P2P_MANAGER::Instance().FindByPID(opp_g->GetMasterPID());
		
		if (pCCI != nullptr)
			break;

		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("<길드> 상대방 길드의 길드장이 접속중이 아닙니다."));
		g->RequestRefuseWar(opp_g->GetID());
		return;

	} while (false);

	g->RequestDeclareWar(opp_g->GetID(), type);
}

ACMD(do_nowar)
{
	CGuild* g = ch->GetGuild();
	if (!g)
		return;

	char arg1[256];
	one_argument(argument, arg1, sizeof(arg1));

	if (!*arg1)
		return;

	uint32_t gm_pid = g->GetMasterPID();

	if (gm_pid != ch->GetPlayerID())
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("<길드> 길드전에 대한 권한이 없습니다."));
		return;
	}

	CGuild* opp_g = CGuildManager::Instance().FindGuildByName(arg1);

	if (!opp_g)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("<길드> 그런 길드가 없습니다."));
		return;
	}

	g->RequestRefuseWar(opp_g->GetID());
}

ACMD(do_pkmode)
{
	if (ch && CBattlegroundManager::Instance().IsEventMap(ch->GetMapIndex()))
		return;

	char arg1[256];
	one_argument(argument, arg1, sizeof(arg1));

	if (!*arg1)
		return;

	uint8_t mode = 0;
	str_to_number(mode, arg1);

	if (mode == PK_MODE_PROTECT)
		return;

	if (ch->GetLevel() < g_bPKProtectLevel && mode != 0)
		return;

	ch->SetPKMode(mode);
}

ACMD(do_disband_gwar)
{
	char arg1[256];
	one_argument(argument, arg1, sizeof(arg1));

	if (!*arg1)
		return;

	if (!ch->GetWarMap())
		return;

	LPDESC d = DESC_MANAGER::Instance().FindByCharacterName(arg1);
	LPCHARACTER	tch = d ? d->GetCharacter() : nullptr;

	if (!tch || !tch->IsPC())
		return;

	if (ch == tch)
		return;

	CGuild * g = ch->GetGuild();
	CGuild * tg = tch->GetGuild();

	if (!g || !tg)
		return;

	if (tg->GetID() != g->GetID())
		return;

	if (g->GetMasterPID() != ch->GetPlayerID())
		return;

	if (!g->UnderAnyWar())
		return;

	tch->SetQuestFlag("guild_war_join.savasengeli", get_unix_time() + 3600);

	DESC_MANAGER::Instance().DestroyDesc(d);
}

ACMD(do_messenger_auth)
{
	if (ch && CBattlegroundManager::Instance().IsEventMap(ch->GetMapIndex()))
		return;

	if (ch->GetArena())
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("대련장에서 사용하실 수 없습니다."));
		return;
	}

	char arg1[256], arg2[256];
	two_arguments(argument, arg1, sizeof(arg1), arg2, sizeof(arg2));

	if (!*arg1 || !*arg2)
		return;

	char answer = LOWER(*arg1);

	bool bIsDenied = answer != 'y';
	bool bIsAdded = MessengerManager::Instance().AuthToAdd(ch->GetName(), arg2, bIsDenied); // DENY
	if (bIsAdded && bIsDenied)
	{
		LPCHARACTER tch = CHARACTER_MANAGER::Instance().FindPC(arg2);

		if (tch)
			tch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("%s 님으로 부터 친구 등록을 거부 당했습니다."), ch->GetName());
		else if (CCI* pCCI = P2P_MANAGER::Instance().Find(arg2))
		{
			if (pCCI->pkDesc)
			{
				TPacketGGMessengerRequest packet;
				packet.bHeader = HEADER_GG_MESSENGER_REQUEST_FAIL;
				strlcpy(packet.szRequestor, arg2, sizeof(packet.szRequestor));
				packet.dwTargetPID = ch->GetPlayerID();
				pCCI->pkDesc->Packet(&packet, sizeof(TPacketGGMessengerRequest));
			}
		}
	}

}

ACMD(do_setblockmode)
{
	char arg1[256];
	one_argument(argument, arg1, sizeof(arg1));

	if (*arg1)
	{
		uint8_t flag = 0;
		str_to_number(flag, arg1);
		ch->SetBlockMode(flag);
	}
}

ACMD(do_unmount)
{
	if (true == ch->UnEquipSpecialRideUniqueItem())
	{
		ch->RemoveAffect(AFFECT_MOUNT);
		ch->RemoveAffect(AFFECT_MOUNT_BONUS);

		if (ch->IsHorseRiding())
		{
			ch->StopRiding(); 
		}
	}
	else
	{
		ch->ChatPacket( CHAT_TYPE_INFO, LC_TEXT("인벤토리가 꽉 차서 내릴 수 없습니다."));
	}

}

ACMD(do_observer_exit)
{
	if (ch->IsObserverMode())
	{
		if (ch->GetWarMap())
			ch->SetWarMap(nullptr);

		if (ch->GetArena() != nullptr || ch->GetArenaObserverMode() == true)
		{
			ch->SetArenaObserverMode(false);

			if (ch->GetArena() != nullptr)
				ch->GetArena()->RemoveObserver(ch->GetPlayerID());

			ch->SetArena(nullptr);
			ch->WarpSet(ARENA_RETURN_POINT_X(ch->GetEmpire()), ARENA_RETURN_POINT_Y(ch->GetEmpire()));
		}
		else
		{
			ch->ExitToSavedLocation();
		}
		ch->SetObserverMode(false);
	}
}

ACMD(do_view_equip)
{
	if (ch && CBattlegroundManager::Instance().IsEventMap(ch->GetMapIndex()))
		return;

	if (!ch)
		return;

	if (ch->GetLevel() <= 14)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("Bu fonksiyonu kulanabilecek seviyede degilsin."));
		return;
	}

	char arg1[256];
	one_argument(argument, arg1, sizeof(arg1));

	if (*arg1)
	{
		uint32_t vid = 0;
		str_to_number(vid, arg1);
		LPCHARACTER tch = CHARACTER_MANAGER::Instance().Find(vid);

		if (!tch)
			return;

		if (!tch->IsPC())
			return;
		/*
		   int32_t iSPCost = ch->GetMaxSP() / 3;

		   if (ch->GetSP() < iSPCost)
		   {
		   ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("정신력이 부족하여 다른 사람의 장비를 볼 수 없습니다."));
		   return;
		   }
		   ch->PointChange(POINT_SP, -iSPCost);
		 */
		tch->SendEquipment(ch);
	}
}

ACMD(do_party_request)
{
	if (ch && CBattlegroundManager::Instance().IsEventMap(ch->GetMapIndex()))
		return;

	if (ch->GetArena())
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("대련장에서 사용하실 수 없습니다."));
		return;
	}

	if (ch->GetParty())
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("이미 파티에 속해 있으므로 가입신청을 할 수 없습니다."));
		return;
	}

	char arg1[256];
	one_argument(argument, arg1, sizeof(arg1));

	if (!*arg1)
		return;

	uint32_t vid = 0;
	str_to_number(vid, arg1);
	LPCHARACTER tch = CHARACTER_MANAGER::Instance().Find(vid);

	if (tch)
		if (!ch->RequestToParty(tch))
			ch->ChatPacket(CHAT_TYPE_COMMAND, "PartyRequestDenied");
}

ACMD(do_party_request_accept)
{
	if (ch && CBattlegroundManager::Instance().IsEventMap(ch->GetMapIndex()))
		return;
		
	char arg1[256];
	one_argument(argument, arg1, sizeof(arg1));

	if (!*arg1)
		return;

	uint32_t vid = 0;
	str_to_number(vid, arg1);
	LPCHARACTER tch = CHARACTER_MANAGER::Instance().Find(vid);

	if (tch)
		ch->AcceptToParty(tch);
}

ACMD(do_party_request_deny)
{
	char arg1[256];
	one_argument(argument, arg1, sizeof(arg1));

	if (!*arg1)
		return;

	uint32_t vid = 0;
	str_to_number(vid, arg1);
	LPCHARACTER tch = CHARACTER_MANAGER::Instance().Find(vid);

	if (tch)
		ch->DenyToParty(tch);
}

static const char* FN_point_string(int32_t apply_number)
{
	switch (apply_number)
	{
		case POINT_MAX_HP:	return LC_TEXT("최대 생명력 +%d");
		case POINT_MAX_SP:	return LC_TEXT("최대 정신력 +%d");
		case POINT_HT:		return LC_TEXT("체력 +%d");
		case POINT_IQ:		return LC_TEXT("지능 +%d");
		case POINT_ST:		return LC_TEXT("근력 +%d");
		case POINT_DX:		return LC_TEXT("민첩 +%d");
		case POINT_ATT_SPEED:	return LC_TEXT("공격속도 +%d");
		case POINT_MOV_SPEED:	return LC_TEXT("이동속도 %d");
		case POINT_CASTING_SPEED:	return LC_TEXT("쿨타임 -%d");
		case POINT_HP_REGEN:	return LC_TEXT("생명력 회복 +%d");
		case POINT_SP_REGEN:	return LC_TEXT("정신력 회복 +%d");
		case POINT_POISON_PCT:	return LC_TEXT("독공격 %d");
#ifdef ENABLE_WOLFMAN_CHARACTER
		case POINT_BLEEDING_PCT:	return LC_TEXT("독공격 %d");
#endif
		case POINT_STUN_PCT:	return LC_TEXT("스턴 +%d");
		case POINT_SLOW_PCT:	return LC_TEXT("슬로우 +%d");
		case POINT_CRITICAL_PCT:	return LC_TEXT("%d%% 확률로 치명타 공격");
		case POINT_RESIST_CRITICAL:	return LC_TEXT("상대의 치명타 확률 %d%% 감소");
		case POINT_PENETRATE_PCT:	return LC_TEXT("%d%% 확률로 관통 공격");
		case POINT_RESIST_PENETRATE: return LC_TEXT("상대의 관통 공격 확률 %d%% 감소");
		case POINT_ATTBONUS_HUMAN:	return LC_TEXT("인간류 몬스터 타격치 +%d%%");
		case POINT_ATTBONUS_ANIMAL:	return LC_TEXT("동물류 몬스터 타격치 +%d%%");
		case POINT_ATTBONUS_ORC:	return LC_TEXT("웅귀족 타격치 +%d%%");
		case POINT_ATTBONUS_MILGYO:	return LC_TEXT("밀교류 타격치 +%d%%");
		case POINT_ATTBONUS_UNDEAD:	return LC_TEXT("시체류 타격치 +%d%%");
		case POINT_ATTBONUS_DEVIL:	return LC_TEXT("악마류 타격치 +%d%%");
		case POINT_STEAL_HP:		return LC_TEXT("타격치 %d%% 를 생명력으로 흡수");
		case POINT_STEAL_SP:		return LC_TEXT("타력치 %d%% 를 정신력으로 흡수");
		case POINT_MANA_BURN_PCT:	return LC_TEXT("%d%% 확률로 타격시 상대 전신력 소모");
		case POINT_DAMAGE_SP_RECOVER:	return LC_TEXT("%d%% 확률로 피해시 정신력 회복");
		case POINT_BLOCK:			return LC_TEXT("물리타격시 블럭 확률 %d%%");
		case POINT_DODGE:			return LC_TEXT("활 공격 회피 확률 %d%%");
		case POINT_RESIST_SWORD:	return LC_TEXT("한손검 방어 %d%%");
		case POINT_RESIST_TWOHAND:	return LC_TEXT("양손검 방어 %d%%");
		case POINT_RESIST_DAGGER:	return LC_TEXT("두손검 방어 %d%%");
		case POINT_RESIST_BELL:		return LC_TEXT("방울 방어 %d%%");
		case POINT_RESIST_FAN:		return LC_TEXT("부채 방어 %d%%");
		case POINT_RESIST_BOW:		return LC_TEXT("활공격 저항 %d%%");
#ifdef ENABLE_WOLFMAN_CHARACTER
		case POINT_RESIST_CLAW:		return LC_TEXT("두손검 방어 %d%%");
#endif
		case POINT_RESIST_FIRE:		return LC_TEXT("화염 저항 %d%%");
		case POINT_RESIST_ELEC:		return LC_TEXT("전기 저항 %d%%");
		case POINT_RESIST_MAGIC:	return LC_TEXT("마법 저항 %d%%");
		case POINT_RESIST_MAGIC_REDUCTION:	return LC_TEXT("마법 저항 %d%%");
		case POINT_RESIST_WIND:		return LC_TEXT("바람 저항 %d%%");
		case POINT_RESIST_ICE:		return LC_TEXT("냉기 저항 %d%%");
		case POINT_RESIST_EARTH:	return LC_TEXT("대지 저항 %d%%");
		case POINT_RESIST_DARK:		return LC_TEXT("어둠 저항 %d%%");
		case POINT_REFLECT_MELEE:	return LC_TEXT("직접 타격치 반사 확률 : %d%%");
		case POINT_REFLECT_CURSE:	return LC_TEXT("저주 되돌리기 확률 %d%%");
		case POINT_POISON_REDUCE:	return LC_TEXT("독 저항 %d%%");
#ifdef ENABLE_WOLFMAN_CHARACTER
		case POINT_BLEEDING_REDUCE:	return LC_TEXT("독 저항 %d%%");
#endif
		case POINT_KILL_SP_RECOVER:	return LC_TEXT("%d%% 확률로 적퇴치시 정신력 회복");
		case POINT_EXP_DOUBLE_BONUS:	return LC_TEXT("%d%% 확률로 적퇴치시 경험치 추가 상승");
		case POINT_GOLD_DOUBLE_BONUS:	return LC_TEXT("%d%% 확률로 적퇴치시 돈 2배 드롭");
		case POINT_ITEM_DROP_BONUS:	return LC_TEXT("%d%% 확률로 적퇴치시 아이템 2배 드롭");
		case POINT_POTION_BONUS:	return LC_TEXT("물약 사용시 %d%% 성능 증가");
		case POINT_KILL_HP_RECOVERY:	return LC_TEXT("%d%% 확률로 적퇴치시 생명력 회복");
//		case POINT_IMMUNE_STUN:	return LC_TEXT("기절하지 않음 %d%%");
//		case POINT_IMMUNE_SLOW:	return LC_TEXT("느려지지 않음 %d%%");
//		case POINT_IMMUNE_FALL:	return LC_TEXT("넘어지지 않음 %d%%");
//		case POINT_SKILL:	return LC_TEXT("");
//		case POINT_BOW_DISTANCE:	return LC_TEXT("");
		case POINT_ATT_GRADE_BONUS:	return LC_TEXT("공격력 +%d");
		case POINT_DEF_GRADE_BONUS:	return LC_TEXT("방어력 +%d");
		case POINT_MAGIC_ATT_GRADE:	return LC_TEXT("마법 공격력 +%d");
		case POINT_MAGIC_DEF_GRADE:	return LC_TEXT("마법 방어력 +%d");
//		case POINT_CURSE_PCT:	return LC_TEXT("");
		case POINT_MAX_STAMINA:	return LC_TEXT("최대 지구력 +%d");
		case POINT_ATTBONUS_WARRIOR:	return LC_TEXT("무사에게 강함 +%d%%");
		case POINT_ATTBONUS_ASSASSIN:	return LC_TEXT("자객에게 강함 +%d%%");
		case POINT_ATTBONUS_SURA:		return LC_TEXT("수라에게 강함 +%d%%");
		case POINT_ATTBONUS_SHAMAN:		return LC_TEXT("무당에게 강함 +%d%%");
#ifdef ENABLE_WOLFMAN_CHARACTER
		case POINT_ATTBONUS_WOLFMAN:	return LC_TEXT("무당에게 강함 +%d%%");
#endif
		case POINT_ATTBONUS_MONSTER:	return LC_TEXT("몬스터에게 강함 +%d%%");
		case POINT_MALL_ATTBONUS:		return LC_TEXT("공격력 +%d%%");
		case POINT_MALL_DEFBONUS:		return LC_TEXT("방어력 +%d%%");
		case POINT_MALL_EXPBONUS:		return LC_TEXT("경험치 %d%%");
		case POINT_MALL_ITEMBONUS:		return LC_TEXT("아이템 드롭율 %.1f배");
		case POINT_MALL_GOLDBONUS:		return LC_TEXT("돈 드롭율 %.1f배");
		case POINT_MAX_HP_PCT:			return LC_TEXT("최대 생명력 +%d%%");
		case POINT_MAX_SP_PCT:			return LC_TEXT("최대 정신력 +%d%%");
		case POINT_SKILL_DAMAGE_BONUS:	return LC_TEXT("스킬 데미지 %d%%");
		case POINT_NORMAL_HIT_DAMAGE_BONUS:	return LC_TEXT("평타 데미지 %d%%");
		case POINT_SKILL_DEFEND_BONUS:		return LC_TEXT("스킬 데미지 저항 %d%%");
		case POINT_NORMAL_HIT_DEFEND_BONUS:	return LC_TEXT("평타 데미지 저항 %d%%");
//		case POINT_PC_BANG_EXP_BONUS:	return LC_TEXT("");
//		case POINT_PC_BANG_DROP_BONUS:	return LC_TEXT("");
//		case POINT_EXTRACT_HP_PCT:	return LC_TEXT("");
		case POINT_RESIST_WARRIOR:	return LC_TEXT("무사공격에 %d%% 저항");
		case POINT_RESIST_ASSASSIN:	return LC_TEXT("자객공격에 %d%% 저항");
		case POINT_RESIST_SURA:		return LC_TEXT("수라공격에 %d%% 저항");
		case POINT_RESIST_SHAMAN:	return LC_TEXT("무당공격에 %d%% 저항");
#ifdef ENABLE_WOLFMAN_CHARACTER
		case POINT_RESIST_WOLFMAN:	return LC_TEXT("무당공격에 %d%% 저항");
#endif
		default:                    return "error %d";
	}
}

static bool FN_hair_affect_string(LPCHARACTER ch, char *buf, size_t bufsiz)
{
	if (nullptr == ch || nullptr == buf)
		return false;

	CAffect* aff = nullptr;
	time_t expire = 0;
	struct tm ltm;
	int32_t	year, mon, day;
	int32_t	offset = 0;

	aff = ch->FindAffect(AFFECT_HAIR);

	if (nullptr == aff)
		return false;

	expire = ch->GetQuestFlag("hair.limit_time");

	if (expire < get_unix_time())
		return false;

	// set apply string
	offset = snprintf(buf, bufsiz, FN_point_string(aff->bApplyOn), aff->lApplyValue);

	if (offset < 0 || offset >= (int32_t) bufsiz)
		offset = bufsiz - 1;

	localtime_r(&expire, &ltm);

	year	= ltm.tm_year + 1900;
	mon		= ltm.tm_mon + 1;
	day		= ltm.tm_mday;

	snprintf(buf + offset, bufsiz - offset, LC_TEXT(" (만료일 : %d년 %d월 %d일)"), year, mon, day);

	return true;
}

ACMD(do_costume)
{
	char buf[768];

	const size_t bufferSize = sizeof(buf);

	char arg1[256];
	one_argument(argument, arg1, sizeof(arg1));

	CItem* pBody = ch->GetWear(WEAR_COSTUME_BODY);
	CItem* pHair = ch->GetWear(WEAR_COSTUME_HAIR);
#ifdef ENABLE_MOUNT_COSTUME_SYSTEM
	CItem* pMount = ch->GetWear(WEAR_COSTUME_MOUNT);
#endif
#ifdef ENABLE_ACCE_SYSTEM
	CItem* pAcce = ch->GetWear(WEAR_COSTUME_ACCE);
#endif
#ifdef ENABLE_WEAPON_COSTUME_SYSTEM
	CItem* pWeapon = ch->GetWear(WEAR_COSTUME_WEAPON);
#endif

	ch->ChatPacket(CHAT_TYPE_INFO, "COSTUME status:");

	if (pHair)
	{
		const char* itemName = pHair->GetName();
		ch->ChatPacket(CHAT_TYPE_INFO, "  HAIR : %s", itemName);

		for (int32_t i = 0; i < pHair->GetAttributeCount(); ++i)
		{
			const TPlayerItemAttribute& attr = pHair->GetAttribute(i);
			if (0 < attr.bType)
			{
				snprintf(buf, bufferSize, FN_point_string(attr.bType), attr.sValue);
				ch->ChatPacket(CHAT_TYPE_INFO, "     %s", buf);
			}
		}

		if (pHair->IsEquipped() && arg1[0] == 'h')
			ch->UnequipItem(pHair);
	}

	if (pBody)
	{
		const char* itemName = pBody->GetName();
		ch->ChatPacket(CHAT_TYPE_INFO, "  BODY : %s", itemName);

		if (pBody->IsEquipped() && arg1[0] == 'b')
			ch->UnequipItem(pBody);
	}

#ifdef ENABLE_MOUNT_COSTUME_SYSTEM
	if (pMount)
	{
		const char* itemName = pMount->GetName();
		ch->ChatPacket(CHAT_TYPE_INFO, "  MOUNT : %s", itemName);

		if (pMount->IsEquipped() && arg1[0] == 'm')
			ch->UnequipItem(pMount);
	}
#endif

#ifdef ENABLE_ACCE_SYSTEM
	if (pAcce)
	{
		const char* itemName = pAcce->GetName();
		ch->ChatPacket(CHAT_TYPE_INFO, "  ACCE : %s", itemName);

		if (pAcce->IsEquipped() && arg1[0] == 'a')
			ch->UnequipItem(pAcce);
	}
#endif

#ifdef ENABLE_WEAPON_COSTUME_SYSTEM
	if (pWeapon)
	{
		const char* itemName = pWeapon->GetName();
		ch->ChatPacket(CHAT_TYPE_INFO, "  WEAPON : %s", itemName);

		if (pWeapon->IsEquipped() && arg1[0] == 'w')
			ch->UnequipItem(pWeapon);
	}
#endif
}

ACMD(do_hair)
{
	char buf[256];

	if (false == FN_hair_affect_string(ch, buf, sizeof(buf)))
		return;

	ch->ChatPacket(CHAT_TYPE_INFO, buf);
}

ACMD(do_inventory)
{
	int32_t	index = 0;
	int32_t	count		= 1;

	char arg1[256];
	char arg2[256];

	LPITEM	item;

	two_arguments(argument, arg1, sizeof(arg1), arg2, sizeof(arg2));

	if (!*arg1)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, "Usage: inventory <start_index> <count>");
		return;
	}

	if (!*arg2)
	{
		index = 0;
		str_to_number(count, arg1);
	}
	else
	{
		str_to_number(index, arg1); index = MIN(index, INVENTORY_MAX_NUM);
		str_to_number(count, arg2); count = MIN(count, INVENTORY_MAX_NUM);
	}

	for (int32_t i = 0; i < count; ++i)
	{
		if (index >= INVENTORY_MAX_NUM)
			break;

		item = ch->GetInventoryItem(index);

		if (item)
		{
			ch->ChatPacket(CHAT_TYPE_INFO, "inventory [%d] = %s [%u] stone %d, %d, %d attr %d %d, %d %d, %d %d, %d %d, %d %d, %d %d, %d %d",
				index, item->GetName(), item->GetID(), item->GetSocket(0), item->GetSocket(1), item->GetSocket(2),
				item->GetAttributeType(0), item->GetAttributeValue(0),
				item->GetAttributeType(1), item->GetAttributeValue(1),
				item->GetAttributeType(2), item->GetAttributeValue(2),
				item->GetAttributeType(3), item->GetAttributeValue(3),
				item->GetAttributeType(4), item->GetAttributeValue(4),
				item->GetAttributeType(5), item->GetAttributeValue(5),
				item->GetAttributeType(6), item->GetAttributeValue(6));
		}

		++index;
	}
}

ACMD(do_block_exp)
{
	ch->ChatPacket(CHAT_TYPE_INFO, "Anti exp aktif");
	ch->BlockExp();
}
ACMD(do_unblock_exp)
{
	ch->ChatPacket(CHAT_TYPE_INFO, "Anti exp pasif");
	ch->UnblockExp();
}

ACMD(do_remove_polymorph)
{
	if (!ch)
		return;

	if (ch->GetExchange() || ch->GetMyShop() || ch->GetShopOwner() || ch->IsOpenSafebox() || ch->IsCubeOpen())
		return;

	if (ch->IsPolymorphed()) 
	{
		ch->RemoveAffect(AFFECT_POLYMORPH);
		ch->SetPolymorph(0);
	}
}

//gift notify quest command
ACMD(do_gift)
{
	ch->ChatPacket(CHAT_TYPE_COMMAND, "gift");
}

ACMD(do_cube)
{
	if (!ch->CanDoCube())
		return;

	dev_log(LOG_DEB0, "CUBE COMMAND <%s>: %s", ch->GetName(), argument);
	int32_t cube_index = 0, inven_index = 0;
	const char *line;

	char arg1[256], arg2[256], arg3[256];

	line = two_arguments(argument, arg1, sizeof(arg1), arg2, sizeof(arg2));
	one_argument(line, arg3, sizeof(arg3));

	if (0 == arg1[0])
	{
		// print usage
		ch->ChatPacket(CHAT_TYPE_INFO, "Usage: cube open");
		ch->ChatPacket(CHAT_TYPE_INFO, "       cube close");
		ch->ChatPacket(CHAT_TYPE_INFO, "       cube add <inventory_index>");
		ch->ChatPacket(CHAT_TYPE_INFO, "       cube delete <cube_index>");
		ch->ChatPacket(CHAT_TYPE_INFO, "       cube list");
		ch->ChatPacket(CHAT_TYPE_INFO, "       cube cancel");
		ch->ChatPacket(CHAT_TYPE_INFO, "       cube make [all]");
		return;
	}

	const std::string& strArg1 = std::string(arg1);

	// r_info (request information)
	// /cube r_info     ==> (Client -> Server) 현재 NPC가 만들 수 있는 레시피 요청
	//					    (Server -> Client) /cube r_list npcVNUM resultCOUNT 123,1/125,1/128,1/130,5
	//
	// /cube r_info 3   ==> (Client -> Server) 현재 NPC가 만들수 있는 레시피 중 3번째 아이템을 만드는 데 필요한 정보를 요청
	// /cube r_info 3 5 ==> (Client -> Server) 현재 NPC가 만들수 있는 레시피 중 3번째 아이템부터 이후 5개의 아이템을 만드는 데 필요한 재료 정보를 요청
	//					   (Server -> Client) /cube m_info startIndex count 125,1|126,2|127,2|123,5&555,5&555,4/120000@125,1|126,2|127,2|123,5&555,5&555,4/120000
	//
	if (strArg1 == "r_info")
	{
		if (0 == arg2[0])
			Cube_request_result_list(ch);
		else
		{
			if (isdigit(*arg2))
			{
				int32_t listIndex = 0, requestCount = 1;
				str_to_number(listIndex, arg2);

				if (0 != arg3[0] && isdigit(*arg3))
					str_to_number(requestCount, arg3);

				Cube_request_material_info(ch, listIndex, requestCount);
			}
		}

		return;
	}

	switch (LOWER(arg1[0]))
	{
		case 'o':	// open
			Cube_open(ch);
			break;

		case 'c':	// close
			Cube_close(ch);
			break;

		case 'l':	// list
			Cube_show_list(ch);
			break;

		case 'a':	// add cue_index inven_index
			{
				if (0 == arg2[0] || !isdigit(*arg2) ||
					0 == arg3[0] || !isdigit(*arg3))
					return;

				str_to_number(cube_index, arg2);
				str_to_number(inven_index, arg3);
				Cube_add_item (ch, cube_index, inven_index);
			}
			break;

		case 'd':	// delete
			{
				if (0 == arg2[0] || !isdigit(*arg2))
					return;

				str_to_number(cube_index, arg2);
				Cube_delete_item (ch, cube_index);
			}
			break;

		case 'm':	// make
			if (0 != arg2[0])
			{
				while (true == Cube_make(ch))
					dev_log (LOG_DEB0, "cube make success");
			}
			else
				Cube_make(ch);
			break;

		default:
			return;
	}
}

ACMD(do_in_game_mall)
{
	ch->ChatPacket(CHAT_TYPE_COMMAND, "TODOOOO");

#if 0
	std::string country_code = "tr";

	const char sas_key[] = "GF9001";
	std::string sas = str(boost::format("%u%u%s") % ch->GetPlayerID() % ch->GetAID() % sas_key);

	auto md5Helper = std::make_unique<CMd5>();
	auto sas_output = md5Helper->BuildHash(sas.data(), sas.size());

	std::string buf = str(boost::format("mall http://%s/ishop?pid=%u&c=%s&sid=%d&sas=%s") % g_strWebMallURL.c_str() % ch->GetPlayerID() % country_code % g_server_id % sas_output);

	ch->ChatPacket(CHAT_TYPE_COMMAND, buf.c_str());
#endif
}

// 주사위
ACMD(do_dice) 
{
	char arg1[256], arg2[256];
	int32_t start = 1, end = 100;

	two_arguments(argument, arg1, sizeof(arg1), arg2, sizeof(arg2));

	if (*arg1 && *arg2)
	{
		start = atoi(arg1);
		end = atoi(arg2);
	}
	else if (*arg1 && !*arg2)
	{
		start = 1;
		end = atoi(arg1);
	}

	end = MAX(start, end);
	start = MIN(start, end);

	int32_t n = number(start, end);
	
#ifdef ENABLE_DICE_SYSTEM
	if (ch->GetParty())
		ch->GetParty()->ChatPacketToAllMember(CHAT_TYPE_DICE_INFO, LC_TEXT("%s님이 주사위를 굴려 %d가 나왔습니다. (%d-%d)"), ch->GetName(), n, start, end);
	else
		ch->ChatPacket(CHAT_TYPE_DICE_INFO, LC_TEXT("당신이 주사위를 굴려 %d가 나왔습니다. (%d-%d)"), n, start, end);
#else
	if (ch->GetParty())
		ch->GetParty()->ChatPacketToAllMember(CHAT_TYPE_INFO, LC_TEXT("%s님이 주사위를 굴려 %d가 나왔습니다. (%d-%d)"), ch->GetName(), n, start, end);
	else
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("당신이 주사위를 굴려 %d가 나왔습니다. (%d-%d)"), n, start, end);
#endif
}

ACMD(do_click_safebox)
{
	if ((ch->GetGMLevel() <= GM_PLAYER) && (ch->GetDungeon() || ch->GetWarMap()))
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("You cannot open the safebox in dungeon or at war."));
		return;
	}

	ch->SetSafeboxOpenPosition();
	ch->ChatPacket(CHAT_TYPE_COMMAND, "ShowMeSafeboxPassword");
}
ACMD(do_force_logout)
{
	LPDESC pDesc=DESC_MANAGER::Instance().FindByCharacterName(ch->GetName());
	if (!pDesc)
		return;
	pDesc->DelayedDisconnect(0);
}

ACMD(do_click_mall)
{
	ch->ChatPacket(CHAT_TYPE_COMMAND, "ShowMeMallPassword");
}

ACMD(do_ride)
{
    dev_log(LOG_DEB0, "[DO_RIDE] start");
    if (ch->IsDead() || ch->IsStun())
	return;

	if (ch && CBattlegroundManager::Instance().IsEventMap(ch->GetMapIndex()) && !ch->IsGM())
		return;

	/*
	if ((get_unix_ms_time() - ch->GetLastMoveTime()) < 1000) {
		ch->ChatPacket(CHAT_TYPE_INFO, "Hareket ederken bu eylemi gerceklestiremezsin");
		return;
	}
	*/

	if (ch->IsHorseRiding())
	{
	    dev_log(LOG_DEB0, "[DO_RIDE] stop riding");
	    ch->StopRiding(); 
	    return;
	}

	if (ch->GetMountVnum())
	{
	    dev_log(LOG_DEB0, "[DO_RIDE] unmount");
	    do_unmount(ch, nullptr, 0, 0);
	    return;
	}
  

	if (ch->GetHorse() != nullptr)
	{
	    dev_log(LOG_DEB0, "[DO_RIDE] start riding");
	    ch->StartRiding();
	    return;
	}

	for (uint8_t i=0; i<INVENTORY_MAX_NUM; ++i)
	{
	    LPITEM item = ch->GetInventoryItem(i);
	    if (nullptr == item)
		continue;

	    // 유니크 탈것 아이템
		if (item->IsRideItem())
		{
			if (
				nullptr==ch->GetWear(WEAR_UNIQUE1)
				|| nullptr==ch->GetWear(WEAR_UNIQUE2)
#ifdef ENABLE_MOUNT_COSTUME_SYSTEM
				|| nullptr==ch->GetWear(WEAR_COSTUME_MOUNT)
#endif
			)
			{
				dev_log(LOG_DEB0, "[DO_RIDE] USE UNIQUE ITEM");
				//ch->EquipItem(item);
				ch->UseItem(TItemPos (INVENTORY, i));
				return;
			}
		}

	    // 일반 탈것 아이템
	    // TODO : 탈것용 SubType 추가
	    switch (item->GetVnum())
	    {
		case 71114:	// 저신이용권
		case 71116:	// 산견신이용권
		case 71118:	// 투지범이용권
		case 71120:	// 사자왕이용권
		    dev_log(LOG_DEB0, "[DO_RIDE] USE QUEST ITEM");
		    ch->UseItem(TItemPos (INVENTORY, i));
		    return;
	    }

		// GF mantis #113524, 52001~52090 번 탈것
		if( (item->GetVnum() > 52000) && (item->GetVnum() < 52091) )
		{
			dev_log(LOG_DEB0, "[DO_RIDE] USE QUEST ITEM");
			ch->UseItem(TItemPos (INVENTORY, i));
		    return;
		}
	}

    // 타거나 내릴 수 없을때
    ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("말을 먼저 소환해주세요."));
}
