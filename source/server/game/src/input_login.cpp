#include "stdafx.h"
#include "constants.h"
#include "config.h"
#include "utils.h"
#include "input.h"
#include "desc_client.h"
#include "desc_manager.h"
#include "char.h"
#include "char_manager.h"
#include "cmd.h"
#include "buffer_manager.h"
#include "protocol.h"
#include "pvp.h"
#include "start_position.h"
#include "messenger_manager.h"
#include "guild_manager.h"
#include "party.h"
#include "dungeon.h"
#include "war_map.h"
#include "quest_manager.h"
#include "building.h"
#include "wedding.h"
#include "affect.h"
#include "arena.h"
#include "ox_event.h"
#include "priv_manager.h"
#include "dev_log.h"
#include "log.h"
#include "mark_manager.h"
#include "gm.h"
#include "item.h"
#include "battleground.h"

#ifdef ENABLE_WOLFMAN_CHARACTER

// #define USE_LYCAN_CREATE_POSITION
#ifdef USE_LYCAN_CREATE_POSITION

uint32_t g_lycan_create_position[4][2] =
{
	{		0,		0 },
	{ 768000+38300, 896000+35500 },
	{ 819200+38300, 896000+35500 },
	{ 870400+38300, 896000+35500 },
};

inline uint32_t LYCAN_CREATE_START_X(uint8_t e, uint8_t job)
{
	if (1 <= e && e <= 3)
		return (job==JOB_WOLFMAN)?g_lycan_create_position[e][0]:g_create_position[e][0];
	return 0;
}

inline uint32_t LYCAN_CREATE_START_Y(uint8_t e, uint8_t job)
{
	if (1 <= e && e <= 3)
		return (job==JOB_WOLFMAN)?g_lycan_create_position[e][1]:g_create_position[e][1];
	return 0;
}

#endif


#endif

static void _send_bonus_info(LPCHARACTER ch)
{
	int32_t	item_drop_bonus = 0;
	int32_t gold_drop_bonus = 0;
	int32_t gold10_drop_bonus	= 0;
	int32_t exp_bonus		= 0;

	item_drop_bonus		= CPrivManager::Instance().GetPriv(ch, PRIV_ITEM_DROP);
	gold_drop_bonus		= CPrivManager::Instance().GetPriv(ch, PRIV_GOLD_DROP);
	gold10_drop_bonus	= CPrivManager::Instance().GetPriv(ch, PRIV_GOLD10_DROP);
	exp_bonus			= CPrivManager::Instance().GetPriv(ch, PRIV_EXP_PCT);

	if (item_drop_bonus)
	{
		ch->ChatPacket(CHAT_TYPE_NOTICE, 
				LC_TEXT("아이템 드롭률  %d%% 추가 이벤트 중입니다."), item_drop_bonus);
	}
	if (gold_drop_bonus)
	{
		ch->ChatPacket(CHAT_TYPE_NOTICE, 
				LC_TEXT("골드 드롭률 %d%% 추가 이벤트 중입니다."), gold_drop_bonus);
	}
	if (gold10_drop_bonus)
	{
		ch->ChatPacket(CHAT_TYPE_NOTICE, 
				LC_TEXT("대박골드 드롭률 %d%% 추가 이벤트 중입니다."), gold10_drop_bonus);
	}
	if (exp_bonus)
	{
		ch->ChatPacket(CHAT_TYPE_NOTICE, 
				LC_TEXT("경험치 %d%% 추가 획득 이벤트 중입니다."), exp_bonus);
	}
}

static bool FN_is_battle_zone(LPCHARACTER ch)
{
	switch (ch->GetMapIndex())
	{
		case 1:         // 신수 1차 마을
		case 2:         // 신수 2차 마을
		case 21:        // 천조 1차 마을
		case 23:        // 천조 2차 마을
		case 41:        // 진노 1차 마을
		case 43:        // 진노 2차 마을
		case 113:       // OX 맵
			return false;
	}

	return true;
}

void CInputLogin::Login(LPDESC d, const char * data)
{
	SPacketCGLogin * pinfo = (SPacketCGLogin *) data;

	char login[LOGIN_MAX_LEN + 1];
	trim_and_lower(pinfo->login, login, sizeof(login));

	sys_log(0, "InputLogin::Login : %s", login);

	SPacketGCLoginFailure failurePacket;

	if (!g_bIsTestServer)
	{
		strlcpy(failurePacket.szStatus, "VERSION", sizeof(failurePacket.szStatus));
		d->Packet(&failurePacket, sizeof(SPacketGCLoginFailure));
		sys_log(0, "LOGIN_FAIL: VERSION | %s", login);
		return;
	}

	if (g_bNoMoreClient)
	{
		strlcpy(failurePacket.szStatus, "SHUTDOWN", sizeof(failurePacket.szStatus));
		d->Packet(&failurePacket, sizeof(SPacketGCLoginFailure));
		sys_log(0, "LOGIN_FAIL: SHUTDOWN | %s", login);
		return;
	}

	if (g_iUserLimit > 0)
	{
		int32_t iTotal;
		int32_t * paiEmpireUserCount;
		int32_t iLocal;

		DESC_MANAGER::Instance().GetUserCount(iTotal, &paiEmpireUserCount, iLocal);

		if (g_iUserLimit <= iTotal)
		{
			strlcpy(failurePacket.szStatus, "FULL", sizeof(failurePacket.szStatus));
			d->Packet(&failurePacket, sizeof(SPacketGCLoginFailure));
			return;
		}
	}

	TLoginPacket login_packet;

	strlcpy(login_packet.login, login, sizeof(login_packet.login));
	strlcpy(login_packet.passwd, pinfo->passwd, sizeof(login_packet.passwd));

	db_clientdesc->DBPacket(HEADER_GD_LOGIN, d->GetHandle(), &login_packet, sizeof(TLoginPacket)); 
}

void CInputLogin::LoginByKey(LPDESC d, const char * data)
{
	SPacketCGLogin2 * pinfo = (SPacketCGLogin2 *) data;

	char login[LOGIN_MAX_LEN + 1];
	trim_and_lower(pinfo->name, login, sizeof(login));

	if (g_bNoMoreClient)
	{
		SPacketGCLoginFailure failurePacket;
		strlcpy(failurePacket.szStatus, "SHUTDOWN", sizeof(failurePacket.szStatus));
		d->Packet(&failurePacket, sizeof(SPacketGCLoginFailure));
		return;
	}

	if (g_iUserLimit > 0)
	{
		int32_t iTotal;
		int32_t * paiEmpireUserCount;
		int32_t iLocal;

		DESC_MANAGER::Instance().GetUserCount(iTotal, &paiEmpireUserCount, iLocal);

		if (g_iUserLimit <= iTotal)
		{
			SPacketGCLoginFailure failurePacket;
			strlcpy(failurePacket.szStatus, "FULL", sizeof(failurePacket.szStatus));

			d->Packet(&failurePacket, sizeof(SPacketGCLoginFailure));
			return;
		}
	}

	sys_log(0, "LOGIN_BY_KEY: %s key %u", login, pinfo->login_key);

	d->SetLoginKey(pinfo->login_key);
#ifndef _IMPROVED_PACKET_ENCRYPTION_
	d->SetSecurityKey(pinfo->adwClientKey);
#endif

	TPacketGDLoginByKey ptod;

	strlcpy(ptod.szLogin, login, sizeof(ptod.szLogin));
	ptod.dwLoginKey = pinfo->login_key;
	memcpy(ptod.adwClientKey, pinfo->adwClientKey, sizeof(uint32_t) * 4);
	strlcpy(ptod.szIP, d->GetHostName(), sizeof(ptod.szIP));

	db_clientdesc->DBPacket(HEADER_GD_LOGIN_BY_KEY, d->GetHandle(), &ptod, sizeof(TPacketGDLoginByKey));
}

void CInputLogin::ChangeName(LPDESC d, const char * data)
{
	SPacketCGChangeName * p = (SPacketCGChangeName *) data;
	const TAccountTable & c_r = d->GetAccountTable();

	if (!c_r.id)
	{
		sys_err("no account table");
		return;
	}

	if (!c_r.players[p->index].bChangeName)
		return;

	if (!check_name(p->name))
	{
		SPacketGCCreateFailure pack;
		pack.bType = 0;
		d->Packet(&pack, sizeof(pack));
		return;
	}

	TPacketGDChangeName pdb;

	pdb.pid = c_r.players[p->index].dwID;
	strlcpy(pdb.name, p->name, sizeof(pdb.name));
	db_clientdesc->DBPacket(HEADER_GD_CHANGE_NAME, d->GetHandle(), &pdb, sizeof(TPacketGDChangeName));
}

void CInputLogin::CharacterSelect(LPDESC d, const char * data)
{
	SPacketCGSelectCharacter* pinfo = (SPacketCGSelectCharacter*) data;
	const TAccountTable & c_r = d->GetAccountTable();

	sys_log(0, "player_select: login: %s index: %d", c_r.login, pinfo->player_index);

	if (!c_r.id)
	{
		sys_err("no account table");
		return;
	}

	if (pinfo->player_index >= PLAYER_PER_ACCOUNT)
	{
		sys_err("index overflow %d, login: %s", pinfo->player_index, c_r.login);
		return;
	}

	if (c_r.players[pinfo->player_index].bChangeName)
	{
		sys_err("name must be changed idx %d, login %s, name %s", 
				pinfo->player_index, c_r.login, c_r.players[pinfo->player_index].szName);
		return;
	}

	TPlayerLoadPacket player_load_packet;

	player_load_packet.account_id	= c_r.id;
	player_load_packet.player_id	= c_r.players[pinfo->player_index].dwID;
	player_load_packet.account_index	= pinfo->player_index;

	db_clientdesc->DBPacket(HEADER_GD_PLAYER_LOAD, d->GetHandle(), &player_load_packet, sizeof(TPlayerLoadPacket));
}

bool NewPlayerTable(TPlayerTable * table,
		const char * name,
		uint8_t job,
		uint8_t shape,
		uint8_t bEmpire,
		uint8_t bCon,
		uint8_t bInt,
		uint8_t bStr,
		uint8_t bDex)
{
	if (job >= JOB_MAX_NUM)
		return false;

	memset(table, 0, sizeof(TPlayerTable));

	strlcpy(table->name, name, sizeof(table->name));

	table->level = 1;
	table->job = job;
	table->voice = 0;
	table->part_base = shape;
	
	table->st = JobInitialPoints[job].st;
	table->dx = JobInitialPoints[job].dx;
	table->ht = JobInitialPoints[job].ht;
	table->iq = JobInitialPoints[job].iq;

	table->hp = JobInitialPoints[job].max_hp + table->ht * JobInitialPoints[job].hp_per_ht;
	table->sp = JobInitialPoints[job].max_sp + table->iq * JobInitialPoints[job].sp_per_iq;
	table->stamina = JobInitialPoints[job].max_stamina;

#if defined(ENABLE_WOLFMAN_CHARACTER) && defined(USE_LYCAN_CREATE_POSITION)
	table->x 	= LYCAN_CREATE_START_X(bEmpire, job) + number(-300, 300);
	table->y 	= LYCAN_CREATE_START_Y(bEmpire, job) + number(-300, 300);
#else
	table->x 	= CREATE_START_X(bEmpire) + number(-300, 300);
	table->y 	= CREATE_START_Y(bEmpire) + number(-300, 300);
#endif
	table->z	= 0;
	table->dir	= 0;
	table->playtime = 0;
	table->gold 	= 0;

	table->skill_group = 0;

	return true;
}

bool RaceToJob(uint32_t race, uint32_t* ret_job)
{
	*ret_job = 0;

	if (race >= MAIN_RACE_MAX_NUM)
		return false;

	switch (race)
	{
		case MAIN_RACE_WARRIOR_M:
			*ret_job = JOB_WARRIOR;
			break;

		case MAIN_RACE_WARRIOR_W:
			*ret_job = JOB_WARRIOR;
			break;

		case MAIN_RACE_ASSASSIN_M:
			*ret_job = JOB_ASSASSIN;
			break;

		case MAIN_RACE_ASSASSIN_W:
			*ret_job = JOB_ASSASSIN;
			break;

		case MAIN_RACE_SURA_M:
			*ret_job = JOB_SURA;
			break;

		case MAIN_RACE_SURA_W:
			*ret_job = JOB_SURA;
			break;

		case MAIN_RACE_SHAMAN_M:
			*ret_job = JOB_SHAMAN;
			break;

		case MAIN_RACE_SHAMAN_W:
			*ret_job = JOB_SHAMAN;
			break;
#ifdef ENABLE_WOLFMAN_CHARACTER
		case MAIN_RACE_WOLFMAN_M:
			*ret_job = JOB_WOLFMAN;
			break;
#endif
		default:
			return false;
			break;
	}
	return true;
}

// 신규 캐릭터 지원
bool NewPlayerTable2(TPlayerTable * table, const char * name, uint8_t race, uint8_t shape, uint8_t bEmpire)
{
	if (race >= MAIN_RACE_MAX_NUM)
	{
		sys_err("NewPlayerTable2.OUT_OF_RACE_RANGE(%d >= max(%d))\n", race, MAIN_RACE_MAX_NUM);
		return false;
	}

	uint32_t job;

	if (!RaceToJob(race, &job))
	{	
		sys_err("NewPlayerTable2.RACE_TO_JOB_ERROR(%d)\n", race);
		return false;
	}

	sys_log(0, "NewPlayerTable2(name=%s, race=%d, job=%d)", name, race, job); 

	memset(table, 0, sizeof(TPlayerTable));

	strlcpy(table->name, name, sizeof(table->name));

	table->level		= 1;
	table->job			= race;	// 직업대신 종족을 넣는다
	table->voice		= 0;
	table->part_base	= shape;

	table->st		= JobInitialPoints[job].st;
	table->dx		= JobInitialPoints[job].dx;
	table->ht		= JobInitialPoints[job].ht;
	table->iq		= JobInitialPoints[job].iq;

	table->hp		= JobInitialPoints[job].max_hp + table->ht * JobInitialPoints[job].hp_per_ht;
	table->sp		= JobInitialPoints[job].max_sp + table->iq * JobInitialPoints[job].sp_per_iq;
	table->stamina	= JobInitialPoints[job].max_stamina;

#if defined(ENABLE_WOLFMAN_CHARACTER) && defined(USE_LYCAN_CREATE_POSITION)
	table->x 		= LYCAN_CREATE_START_X(bEmpire, job) + number(-300, 300);
	table->y 		= LYCAN_CREATE_START_Y(bEmpire, job) + number(-300, 300);
#else
	table->x		= CREATE_START_X(bEmpire) + number(-300, 300);
	table->y		= CREATE_START_Y(bEmpire) + number(-300, 300);
#endif
	table->z		= 0;
	table->dir		= 0;
	table->playtime = 0;
	table->gold 	= 0;

	table->skill_group = 0;

	return true;
}

void CInputLogin::CharacterCreate(LPDESC d, const char * data)
{
	SPacketCGCreateCharacter* pinfo = (SPacketCGCreateCharacter *) data;
	TPlayerCreatePacket player_create_packet;

	sys_log(0, "PlayerCreate: name %s pos %d job %d shape %d",
			pinfo->name, 
			pinfo->index, 
			pinfo->job, 
			pinfo->shape);

	SPacketGCLoginFailure packFailure;

	if (g_BlockCharCreation)
	{
		d->Packet(&packFailure, sizeof(packFailure));
		return;
	}

	if (strlen(pinfo->name) > 12)
	{
		d->Packet(&packFailure, sizeof(packFailure));
		return;
	}
	
	if (!GM::check_account_allow(d->GetAccountTable().login, GM_ALLOW_CREATE_PLAYER))
	{
		sys_err("gm may not create a character");
		d->Packet(&packFailure, sizeof(packFailure));
		return;
	}

	if (!check_name(pinfo->name) || pinfo->shape > 1)
	{
		d->Packet(&packFailure, sizeof(packFailure));
		return;
	}

	const TAccountTable & c_rAccountTable = d->GetAccountTable();

	if (0 == strcmp(c_rAccountTable.login, pinfo->name))
	{
		SPacketGCCreateFailure pack;
		pack.bType = 1;

		d->Packet(&pack, sizeof(pack));
		return;
	}

	memset(&player_create_packet, 0, sizeof(TPlayerCreatePacket));

	if (!NewPlayerTable2(&player_create_packet.player_table, pinfo->name, pinfo->job, pinfo->shape, d->GetEmpire()))
	{
		sys_err("player_prototype error: job %d face %d ", pinfo->job);
		d->Packet(&packFailure, sizeof(packFailure));
		return;
	}

	trim_and_lower(c_rAccountTable.login, player_create_packet.login, sizeof(player_create_packet.login));
	strlcpy(player_create_packet.passwd, c_rAccountTable.passwd, sizeof(player_create_packet.passwd));

	player_create_packet.account_id	= c_rAccountTable.id;
	player_create_packet.account_index	= pinfo->index;

	sys_log(0, "PlayerCreate: name %s account_id %d, TPlayerCreatePacketSize(%d), Packet->Gold %d",
			pinfo->name, 
			pinfo->index, 
			sizeof(TPlayerCreatePacket),
			player_create_packet.player_table.gold);

	db_clientdesc->DBPacket(HEADER_GD_PLAYER_CREATE, d->GetHandle(), &player_create_packet, sizeof(TPlayerCreatePacket));
}

void CInputLogin::CharacterDelete(LPDESC d, const char * data)
{
	SPacketCGDestroyCharacter* pinfo = (SPacketCGDestroyCharacter*) data;
	const TAccountTable & c_rAccountTable = d->GetAccountTable();

	if (!c_rAccountTable.id)
	{
		sys_err("PlayerDelete: no login data");
		return;
	}

	sys_log(0, "PlayerDelete: login: %s index: %d, social_id %s", c_rAccountTable.login, pinfo->index, pinfo->szPrivateCode);

	if (pinfo->index >= PLAYER_PER_ACCOUNT)
	{
		sys_err("PlayerDelete: index overflow %d, login: %s", pinfo->index, c_rAccountTable.login);
		return;
	}

	if (!c_rAccountTable.players[pinfo->index].dwID)
	{
		sys_err("PlayerDelete: Wrong Social ID index %d, login: %s", pinfo->index, c_rAccountTable.login);
		d->Packet(encode_byte(HEADER_GC_PLAYER_DELETE_WRONG_SOCIAL_ID), 1);
		return;
	}


	if (!GM::check_allow(GM::get_level(c_rAccountTable.players[pinfo->index].szName, c_rAccountTable.login), GM_ALLOW_DELETE_PLAYER))
	{
		sys_err("PlayerDelete: cannot delete gm character [%s]", c_rAccountTable.players[pinfo->index].szName);
		d->Packet(encode_byte(HEADER_GC_PLAYER_DELETE_WRONG_SOCIAL_ID), 1);
		return;
	}

	TPlayerDeletePacket	player_delete_packet;

	trim_and_lower(c_rAccountTable.login, player_delete_packet.login, sizeof(player_delete_packet.login));
	player_delete_packet.player_id	= c_rAccountTable.players[pinfo->index].dwID;
	player_delete_packet.account_index	= pinfo->index;
	strlcpy(player_delete_packet.private_code, pinfo->szPrivateCode, sizeof(player_delete_packet.private_code));

	db_clientdesc->DBPacket(HEADER_GD_PLAYER_DELETE, d->GetHandle(), &player_delete_packet, sizeof(TPlayerDeletePacket));
}

void CInputLogin::Entergame(LPDESC d, const char * data)
{
	LPCHARACTER ch;

	if (!(ch = d->GetCharacter()))
	{
		d->SetPhase(PHASE_CLOSE);
		return;
	}

	GPOS pos = ch->GetXYZ();

	if (!SECTREE_MANAGER::Instance().GetMovablePosition(ch->GetMapIndex(), pos.x, pos.y, pos))
	{
		GPOS pos2;
		SECTREE_MANAGER::Instance().GetRecallPositionByEmpire(ch->GetMapIndex(), ch->GetEmpire(), pos2);

		sys_err("!GetMovablePosition (name %s %dx%d map %d changed to %dx%d)", 
				ch->GetName(),
				pos.x, pos.y,
				ch->GetMapIndex(),
				pos2.x, pos2.y);
		pos = pos2;
	}

	CGuildManager::Instance().LoginMember(ch);

	// 캐릭터를 맵에 추가 
	ch->Show(ch->GetMapIndex(), pos.x, pos.y, pos.z);

	SECTREE_MANAGER::Instance().SendNPCPosition(ch);

	d->SetPhase(PHASE_GAME);

	sys_log(0, "ENTERGAME: %s %dx%dx%d %s map_index %d",
			ch->GetName(), ch->GetX(), ch->GetY(), ch->GetZ(), d->GetHostName(), ch->GetMapIndex());

	if (ch->GetItemAward_cmd())																		//게임페이즈 들어가면
		quest::CQuestManager::Instance().ItemInformer(ch->GetPlayerID(),ch->GetItemAward_vnum());	//questmanager 호출

	if (ch->GetHorseLevel() > 0)
	{
		ch->EnterHorse();
	}

	// 플레이시간 레코딩 시작
	ch->ResetPlayTime();

	// 자동 저장 이벤트 추가
	ch->StartSaveEvent();
	ch->StartRecoveryEvent();
	ch->StartCheckSpeedHackEvent();

	ch->SetLastMoveAblePosition(ch->GetXYZ());
	ch->SetLastMoveableMapIndex();

	ch->StartCheckWallhack();

	CPVPManager::Instance().Connect(ch);
	CPVPManager::Instance().SendList(d);

	MessengerManager::Instance().Login(ch->GetName());

	CPartyManager::Instance().SetParty(ch);
	CGuildManager::Instance().SendGuildWar(ch);

	building::CManager::Instance().SendLandList(d, ch->GetMapIndex());

	marriage::CManager::Instance().Login(ch);

	SPacketGCTime p;
	p.time = get_global_time();
	d->Packet(&p, sizeof(p));

	SPacketGCChannel p2;
	p2.channel = g_bChannel;
	d->Packet(&p2, sizeof(p2));

	if (!ch->IsGMInvisible())
		ch->ReviveInvisible(5);
	else
		ch->ReviveInvisible(INFINITE_AFFECT_DURATION);

	_send_bonus_info(ch);
	
	for (int32_t i = 0; i <= PREMIUM_MAX_NUM; ++i)
	{
		int32_t remain = ch->GetPremiumRemainSeconds(i);

		if (remain <= 0)
			continue;

		ch->AddAffect(AFFECT_PREMIUM_START + i, POINT_NONE, 0, 0, remain, 0, true);
		sys_log(0, "PREMIUM: %s type %d %dmin", ch->GetName(), i, remain);
	}

	if (ch->IsGM())
		ch->ChatPacket(CHAT_TYPE_COMMAND, "ConsoleEnable");

	if (ch->GetMapIndex() >= 10000)
	{
		if (CWarMapManager::Instance().IsWarMap(ch->GetMapIndex()))
			ch->SetWarMap(CWarMapManager::Instance().Find(ch->GetMapIndex()));
		else if (marriage::WeddingManager::Instance().IsWeddingMap(ch->GetMapIndex()))
			ch->SetWeddingMap(marriage::WeddingManager::Instance().Find(ch->GetMapIndex()));
		else {
			ch->SetDungeon(CDungeonManager::Instance().FindByMapIndex(ch->GetMapIndex()));
		}
	}
	else if (CArenaManager::Instance().IsArenaMap(ch->GetMapIndex()) == true)
	{
		int32_t memberFlag = CArenaManager::Instance().IsMember(ch->GetMapIndex(), ch->GetPlayerID());
		if (memberFlag == MEMBER_OBSERVER)
		{
			ch->SetObserverMode(true);
			ch->SetArenaObserverMode(true);
			if (CArenaManager::Instance().RegisterObserverPtr(ch, ch->GetMapIndex(), ch->GetX()/100, ch->GetY()/100))
			{
				sys_log(0, "ARENA : Observer add failed");
			}

			if (ch->IsHorseRiding() == true)
			{
				ch->StopRiding();
				ch->HorseSummon(false);
			}
		}
		else if (memberFlag == MEMBER_DUELIST)
		{
			SPacketGCDuelStart duelStart;
			duelStart.wSize = sizeof(SPacketGCDuelStart);

			ch->GetDesc()->Packet(&duelStart, sizeof(SPacketGCDuelStart));

			if (ch->IsHorseRiding() == true)
			{
				ch->StopRiding();
				ch->HorseSummon(false);
			}

			LPPARTY pParty = ch->GetParty();
			if (pParty != nullptr)
			{
				if (pParty->GetMemberCount() == 2)
				{
					CPartyManager::Instance().DeleteParty(pParty);
				}
				else
				{
					pParty->Quit(ch->GetPlayerID());
				}
			}
		}
		else if (memberFlag == MEMBER_NO)		
		{
			sys_err("input_login.cpp: memberFlag == MEMBER_NO)");
			if (ch->GetGMLevel() == GM_PLAYER)
				ch->WarpSet(EMPIRE_START_X(ch->GetEmpire()), EMPIRE_START_Y(ch->GetEmpire()));
		}
		else
		{
			sys_err("input_login.cpp: Unknown member flag: %d Member: %s", memberFlag, ch->GetName());
			// wtf
		}
	}
	else if (ch->GetMapIndex() == OXEVENT_MAP_INDEX)
	{
		// ox 이벤트 맵
		if (COXEventManager::Instance().Enter(ch) == false)
		{
			// ox 맵 진입 허가가 나지 않음. 플레이어면 마을로 보내자
			if (ch->GetGMLevel() == GM_PLAYER)
				ch->WarpSet(EMPIRE_START_X(ch->GetEmpire()), EMPIRE_START_Y(ch->GetEmpire()));
		}
	}
	else
	{
		if (CWarMapManager::Instance().IsWarMap(ch->GetMapIndex()) ||
				marriage::WeddingManager::Instance().IsWeddingMap(ch->GetMapIndex()))
		{
			if (!g_bIsTestServer)
				ch->WarpSet(EMPIRE_START_X(ch->GetEmpire()), EMPIRE_START_Y(ch->GetEmpire()));
		}
	}

	// 중립맵에 들어갔을때 안내하기
	if (g_noticeBattleZone)
	{
		if (FN_is_battle_zone(ch))
		{
			ch->ChatPacket(CHAT_TYPE_NOTICE, LC_TEXT("이 맵에선 강제적인 대전이 있을수 도 있습니다."));
			ch->ChatPacket(CHAT_TYPE_NOTICE, LC_TEXT("이 조항에 동의하지 않을시"));
			ch->ChatPacket(CHAT_TYPE_NOTICE, LC_TEXT("본인의 주성 및 부성으로 돌아가시기 바랍니다."));
		}
	}

	if (CBattlegroundManager::Instance().IsEventMap(ch->GetMapIndex()))
		CBattlegroundManager::Instance().OnLogin(ch);

	if (g_bIsTestServer && ch->IsGM())
		ch->ChatPacket(CHAT_TYPE_BIG_NOTICE, "Test Server Aktif");

	if (ch->GetPoint(POINT_ST) > 150 || ch->GetPoint(POINT_HT) > 150 || ch->GetPoint(POINT_DX) > 150 || ch->GetPoint(POINT_IQ) > 150)
		LogManager::Instance().HackLog("STAT_OVERFLOW", ch);

	// Compute Points
	// ch->ComputePoints();
}

void CInputLogin::Empire(LPDESC d, const char * c_pData)
{
	const SPacketCGEmpire* p = reinterpret_cast<const SPacketCGEmpire*>(c_pData);

	if (EMPIRE_MAX_NUM <= p->bEmpire)
	{
		d->SetPhase(PHASE_CLOSE);
		return;
	}

	const TAccountTable& r = d->GetAccountTable();

	if (r.bEmpire != 0)
	{
		for (int32_t i = 0; i < PLAYER_PER_ACCOUNT; ++i)
		{
			if (0 != r.players[i].dwID)
			{
				sys_err("EmpireSelectFailed %d", r.players[i].dwID);
				return;
			}
		}
	}

	TEmpireSelectPacket pd;

	pd.dwAccountID = r.id;
	pd.bEmpire = p->bEmpire;

	db_clientdesc->DBPacket(HEADER_GD_EMPIRE_SELECT, d->GetHandle(), &pd, sizeof(pd));
}

int32_t CInputLogin::GuildSymbolUpload(LPDESC d, const char* c_pData, size_t uiBytes)
{
	if (uiBytes < sizeof(SPacketCGGuildSymbolUpload))
		return -1;

	sys_log(0, "GuildSymbolUpload uiBytes %u", uiBytes);

	SPacketCGGuildSymbolUpload* p = (SPacketCGGuildSymbolUpload*) c_pData;

	if (uiBytes < p->size)
		return -1;

	int32_t iSymbolSize = p->size - sizeof(SPacketCGGuildSymbolUpload);

	if (iSymbolSize <= 0 || iSymbolSize > 64 * 1024)
	{
		// 64k 보다 큰 길드 심볼은 올릴수없다
		// 접속을 끊고 무시
		d->SetPhase(PHASE_CLOSE);
		return 0;
	}

	// 땅을 소유하지 않은 길드인 경우.
	if (!g_bIsTestServer)
		if (!building::CManager::Instance().FindLandByGuild(p->guild_id))
		{
			d->SetPhase(PHASE_CLOSE);
			return 0;
		}

	sys_log(0, "GuildSymbolUpload Do Upload %02X%02X%02X%02X %d", c_pData[7], c_pData[8], c_pData[9], c_pData[10], sizeof(*p));

	CGuildMarkManager::Instance().UploadSymbol(p->guild_id, iSymbolSize, (const uint8_t*)(c_pData + sizeof(*p)));
	CGuildMarkManager::Instance().SaveSymbol(GUILD_SYMBOL_FILENAME);
	return iSymbolSize;
}

void CInputLogin::GuildSymbolCRC(LPDESC d, const char* c_pData)
{
	const SPacketCGGuildSymbolCRC& CGPacket = *((SPacketCGGuildSymbolCRC*) c_pData);

	sys_log(0, "GuildSymbolCRC %u %u %u", CGPacket.dwGuildID, CGPacket.dwCRC, CGPacket.dwSize);

	const CGuildMarkManager::TGuildSymbol * pkGS = CGuildMarkManager::Instance().GetGuildSymbol(CGPacket.dwGuildID);

	if (!pkGS)
		return;

	sys_log(0, "  Server %u %u", pkGS->crc, pkGS->raw.size());

	if (pkGS->raw.size() != CGPacket.dwSize || pkGS->crc != CGPacket.dwCRC)
	{
		SPacketGCGuildSymbolData GCPacket;
		GCPacket.size = sizeof(GCPacket) + pkGS->raw.size();
		GCPacket.guild_id = CGPacket.dwGuildID;

		d->BufferedPacket(&GCPacket, sizeof(GCPacket));
		d->Packet(&pkGS->raw[0], pkGS->raw.size());

		sys_log(0, "SendGuildSymbolHead %02X%02X%02X%02X Size %d", 
				pkGS->raw[0], pkGS->raw[1], pkGS->raw[2], pkGS->raw[3], pkGS->raw.size());
	}
}

void CInputLogin::GuildMarkUpload(LPDESC d, const char* c_pData)
{
	SPacketCGMarkUpload * p = (SPacketCGMarkUpload *) c_pData;
	CGuildManager& rkGuildMgr = CGuildManager::Instance();
	CGuild * pkGuild;

	if (!(pkGuild = rkGuildMgr.FindGuild(p->gid)))
	{
		sys_err("MARK_SERVER: GuildMarkUpload: no guild. gid %u", p->gid);
		return;
	}

	if (pkGuild->GetLevel() < guild_mark_min_level)
	{
		sys_log(0, "MARK_SERVER: GuildMarkUpload: level < %u (%u)", guild_mark_min_level, pkGuild->GetLevel());
		return;
	}

	CGuildMarkManager & rkMarkMgr = CGuildMarkManager::Instance();

	sys_log(0, "MARK_SERVER: GuildMarkUpload: gid %u", p->gid);

	bool isEmpty = true;

	for (uint32_t iPixel = 0; iPixel < SGuildMark::SIZE; ++iPixel)
		if (*((uint32_t *) p->image + iPixel) != 0x00000000)
			isEmpty = false;

	if (isEmpty)
		rkMarkMgr.DeleteMark(p->gid);
	else
		rkMarkMgr.SaveMark(p->gid, p->image);
}

void CInputLogin::GuildMarkIDXList(LPDESC d, const char* c_pData)
{
	CGuildMarkManager & rkMarkMgr = CGuildMarkManager::Instance();
	
	uint32_t bufSize = sizeof(uint16_t) * 2 * rkMarkMgr.GetMarkCount();
	char * buf = nullptr;

	if (bufSize > 0)
	{
		buf = (char *) malloc(bufSize);
		rkMarkMgr.CopyMarkIdx(buf);
	}

	SPacketGCMarkIDXList p;
	p.bufSize = sizeof(p) + bufSize;
	p.count = rkMarkMgr.GetMarkCount();

	if (buf)
	{
		d->BufferedPacket(&p, sizeof(p));
		d->LargePacket(buf, bufSize);
		free(buf);
	}
	else
		d->Packet(&p, sizeof(p));

	sys_log(0, "MARK_SERVER: GuildMarkIDXList %d bytes sent.", p.bufSize);
}

void CInputLogin::GuildMarkCRCList(LPDESC d, const char* c_pData)
{
	SPacketCGMarkCRCList * pCG = (SPacketCGMarkCRCList *) c_pData;

	std::map<uint8_t, const SGuildMarkBlock *> mapDiffBlocks;
	CGuildMarkManager::Instance().GetDiffBlocks(pCG->imgIdx, pCG->crclist, mapDiffBlocks);

	uint32_t blockCount = 0;
	TEMP_BUFFER buf(1024 * 1024); // 1M 버퍼

	for (auto it = mapDiffBlocks.begin(); it != mapDiffBlocks.end(); ++it)
	{
		uint8_t posBlock = it->first;
		const SGuildMarkBlock & rkBlock = *it->second;

		buf.write(&posBlock, sizeof(uint8_t));
		buf.write(&rkBlock.m_sizeCompBuf, sizeof(uint32_t));
		buf.write(rkBlock.m_abCompBuf, rkBlock.m_sizeCompBuf);

		++blockCount;
	}

	SPacketGCMarkBlock pGC;
	pGC.imgIdx = pCG->imgIdx;
	pGC.bufSize = buf.size() + sizeof(SPacketGCMarkBlock);
	pGC.count = blockCount;

	sys_log(0, "MARK_SERVER: Sending blocks. (imgIdx %u diff %u size %u)", pCG->imgIdx, mapDiffBlocks.size(), pGC.bufSize);

	if (buf.size() > 0)
	{
		d->BufferedPacket(&pGC, sizeof(SPacketGCMarkBlock));
		d->LargePacket(buf.read_peek(), buf.size());
	}
	else
		d->Packet(&pGC, sizeof(SPacketGCMarkBlock));
}

int32_t CInputLogin::Analyze(LPDESC d, uint8_t bHeader, const char * c_pData)
{
	int32_t iExtraLen = 0;

	switch (bHeader)
	{
		case HEADER_CG_PONG:
			Pong(d);
			break;

		case HEADER_CG_TIME_SYNC:
			Handshake(d, c_pData);
			break;

		case HEADER_CG_LOGIN:
			Login(d, c_pData);
			break;

		case HEADER_CG_LOGIN2:
			LoginByKey(d, c_pData);
			break;

		case HEADER_CG_PLAYER_SELECT:
			CharacterSelect(d, c_pData);
			break;

		case HEADER_CG_PLAYER_CREATE:
			CharacterCreate(d, c_pData);
			break;

		case HEADER_CG_PLAYER_DESTROY:
			CharacterDelete(d, c_pData);
			break;

		case HEADER_CG_ENTERGAME:
			Entergame(d, c_pData);
			break;

		case HEADER_CG_EMPIRE:
			Empire(d, c_pData);
			break;

		case HEADER_CG_CHARACTER_MOVE:
			break;

			///////////////////////////////////////
			// Guild Mark
			/////////////////////////////////////
		case HEADER_CG_MARK_LOGIN:
			if (!guild_mark_server)
			{
				sys_err("Guild Mark login requested but i'm not a mark server!");
				d->SetPhase(PHASE_CLOSE);
				break;
			}

			sys_log(0, "MARK_SERVER: Login");
			d->SetPhase(PHASE_LOGIN);
			break;

		case HEADER_CG_MARK_CRCLIST:
			GuildMarkCRCList(d, c_pData);
			break;

		case HEADER_CG_MARK_IDXLIST:
			GuildMarkIDXList(d, c_pData);
			break;

		case HEADER_CG_MARK_UPLOAD:
			GuildMarkUpload(d, c_pData);
			break;

			//////////////////////////////////////
			// Guild Symbol
			/////////////////////////////////////
		case HEADER_CG_GUILD_SYMBOL_UPLOAD:
			if ((iExtraLen = GuildSymbolUpload(d, c_pData, m_iBufferLeft)) < 0)
				return -1;
			break;

		case HEADER_CG_GUILD_SYMBOL_CRC:
			GuildSymbolCRC(d, c_pData);
			break;
			/////////////////////////////////////

		case HEADER_CG_HACK:
		case HEADER_CG_CHAT:
			break;

		case HEADER_CG_CHANGE_NAME:
			ChangeName(d, c_pData);
			break;

		case HEADER_CG_ITEM_USE:
		case HEADER_CG_TARGET:
			break;

		default:
			sys_err("login phase does not handle this packet! header %d", bHeader);
			//d->SetPhase(PHASE_CLOSE);
			return (0);
	}

	return (iExtraLen);
}

