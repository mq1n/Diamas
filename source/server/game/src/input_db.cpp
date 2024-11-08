#include "stdafx.h" 
#include "constants.h"
#include "config.h"
#include "utils.h"
#include "desc_manager.h"
#include "char.h"
#include "char_manager.h"
#include "item.h"
#include "item_manager.h"
#include "protocol.h"
#include "mob_manager.h"
#include "shop_manager.h"
#include "sectree_manager.h"
#include "skill.h"
#include "quest_manager.h"
#include "p2p.h"
#include "guild.h"
#include "guild_manager.h"
#include "start_position.h"
#include "party.h"
#include "refine.h"
#include "banword.h"
#include "priv_manager.h"
#include "db.h"
#include "building.h"
#include "wedding.h"
#include "login_data.h"
#include "unique_item.h"
#include "affect.h"
#include "motion.h"
#include "dev_log.h"
#include "log.h"
#include "gm.h"
#include "map_location.h"
#include "dragon_soul.h"
#include "desc_client.h"
#include "anticheat_manager.h"
#include "safebox.h"

#define MAPNAME_DEFAULT	"none"

bool GetServerLocation(TAccountTable & rTab, uint8_t bEmpire)
{
	bool bFound = false;

	for (int32_t i = 0; i < PLAYER_PER_ACCOUNT; ++i)
	{
		if (0 == rTab.players[i].dwID)
			continue;

		bFound = true;
		int32_t lIndex = 0;

		if (!CMapLocation::Instance().Get(rTab.players[i].x,
					rTab.players[i].y,
					lIndex,
					rTab.players[i].lAddr,
					rTab.players[i].wPort))
		{
			sys_err("location error name %s mapindex %d %d x %d empire %d",
					rTab.players[i].szName, lIndex, rTab.players[i].x, rTab.players[i].y, rTab.bEmpire);

			rTab.players[i].x = EMPIRE_START_X(rTab.bEmpire);
			rTab.players[i].y = EMPIRE_START_Y(rTab.bEmpire);

			lIndex = 0;

			if (!CMapLocation::Instance().Get(rTab.players[i].x, rTab.players[i].y, lIndex, rTab.players[i].lAddr, rTab.players[i].wPort))
			{
				sys_err("cannot find server for mapindex %d %d x %d (name %s)", 
						lIndex,
						rTab.players[i].x,
						rTab.players[i].y,
						rTab.players[i].szName);
#ifdef ENABLE_NEWSTUFF
				if (!g_stProxyIP.empty())
					rTab.players[i].lAddr=inet_addr(g_stProxyIP.c_str());
#endif
				continue;
			}
		}
#ifdef ENABLE_NEWSTUFF
		if (!g_stProxyIP.empty())
			rTab.players[i].lAddr=inet_addr(g_stProxyIP.c_str());
#endif
		struct in_addr in;
		in.s_addr = rTab.players[i].lAddr;
		sys_log(0, "success to %s:%d", inet_ntoa(in), rTab.players[i].wPort);
	}

	return bFound;
}

void CInputDB::LoginSuccess(uint32_t dwHandle, const char *data)
{
	sys_log(0, "LoginSuccess");

	TAccountTable * pTab = (TAccountTable *) data;

	LPDESC d = DESC_MANAGER::Instance().FindByHandle(dwHandle);

	if (!d)
	{
		sys_log(0, "CInputDB::LoginSuccess - cannot find handle [%s]", pTab->login);

		TLogoutPacket pack;

		strlcpy(pack.login, pTab->login, sizeof(pack.login));
		db_clientdesc->DBPacket(HEADER_GD_LOGOUT, dwHandle, &pack, sizeof(pack));
		return;
	}

	if (strcmp(pTab->status, "OK")) // OK가 아니면
	{
		sys_log(0, "CInputDB::LoginSuccess - status[%s] is not OK [%s]", pTab->status, pTab->login);

		TLogoutPacket pack;

		strlcpy(pack.login, pTab->login, sizeof(pack.login));
		db_clientdesc->DBPacket(HEADER_GD_LOGOUT, dwHandle, &pack, sizeof(pack));

		LoginFailure(d, pTab->status);
		return;
	}

	for (int32_t i = 0; i != PLAYER_PER_ACCOUNT; ++i)
	{
		TSimplePlayerInformation& player = pTab->players[i];
		sys_log(0, "\tplayer(%s).job(%d)", player.szName, player.byJob);
	}

	bool bFound = GetServerLocation(*pTab, pTab->bEmpire);

	d->BindAccountTable(pTab);

	if (!bFound) // 캐릭터가 없으면 랜덤한 제국으로 보낸다.. -_-
	{
		SPacketGCEmpire pe;
		pe.bEmpire = number(1, 3);
		d->Packet(&pe, sizeof(pe));
	}
	else
	{
		SPacketGCEmpire pe;
		pe.bEmpire = d->GetEmpire();
		d->Packet(&pe, sizeof(pe));
	}

	d->SetPhase(PHASE_SELECT);
	d->SendLoginSuccessPacket();

	sys_log(0, "InputDB::login_success: %s", pTab->login);
}

void CInputDB::PlayerCreateFailure(LPDESC d, uint8_t bType)
{
	if (!d)
		return;

	SPacketGCCreateFailure pack;
	pack.bType	= bType;

	d->Packet(&pack, sizeof(pack));
}

void CInputDB::PlayerCreateSuccess(LPDESC d, const char * data)
{
	if (!d)
		return;

	TPacketDGCreateSuccess * pPacketDB = (TPacketDGCreateSuccess *) data;

	if (pPacketDB->bAccountCharacterIndex >= PLAYER_PER_ACCOUNT)
	{
		d->Packet(encode_byte(HEADER_GC_PLAYER_CREATE_FAILURE), 1);
		return;
	}

	int32_t lIndex = 0;

	if (!CMapLocation::Instance().Get(pPacketDB->player.x,
				pPacketDB->player.y,
				lIndex,
				pPacketDB->player.lAddr,
				pPacketDB->player.wPort))
	{
		sys_err("InputDB::PlayerCreateSuccess: cannot find server for mapindex %d %d x %d (name %s)", 
				lIndex,
				pPacketDB->player.x,
				pPacketDB->player.y,
				pPacketDB->player.szName);
	}

	TAccountTable & r_Tab = d->GetAccountTable();
	r_Tab.players[pPacketDB->bAccountCharacterIndex] = pPacketDB->player;

	SPacketGCPlayerCreateSuccess pack;
	pack.bAccountCharacterSlot = pPacketDB->bAccountCharacterIndex;
	pack.kSimplePlayerInfomation = pPacketDB->player;
#ifdef ENABLE_NEWSTUFF
	if (!g_stProxyIP.empty())
		pack.kSimplePlayerInfomation.lAddr=inet_addr(g_stProxyIP.c_str());
#endif
	d->Packet(&pack, sizeof(SPacketGCPlayerCreateSuccess));

	LogManager::Instance().CharLog(pack.kSimplePlayerInfomation.dwID, 0, 0, 0, "CREATE PLAYER", "", d->GetHostName());
}

void CInputDB::PlayerDeleteSuccess(LPDESC d, const char * data)
{
	if (!d)
		return;

	uint8_t account_index;
	account_index = decode_byte(data);
	d->BufferedPacket(encode_byte(HEADER_GC_PLAYER_DELETE_SUCCESS),	1);
	d->Packet(encode_byte(account_index), 1);

	d->GetAccountTable().players[account_index].dwID = 0;
}

void CInputDB::PlayerDeleteFail(LPDESC d)
{
	if (!d)
		return;

	d->Packet(encode_byte(HEADER_GC_PLAYER_DELETE_WRONG_SOCIAL_ID),	1);
	//d->Packet(encode_byte(account_index),			1);

	//d->GetAccountTable().players[account_index].dwID = 0;
}

void CInputDB::ChangeName(LPDESC d, const char * data)
{
	if (!d)
		return;

	TPacketDGChangeName * p = (TPacketDGChangeName *) data;

	TAccountTable & r = d->GetAccountTable();

	if (!r.id)
		return;

	for (size_t i = 0; i < PLAYER_PER_ACCOUNT; ++i)
		if (r.players[i].dwID == p->pid)
		{
			strlcpy(r.players[i].szName, p->name, sizeof(r.players[i].szName));
			r.players[i].bChangeName = 0;

			SPacketGCChangeName pgc;
			pgc.pid = p->pid;
			strlcpy(pgc.name, p->name, sizeof(pgc.name));

			d->Packet(&pgc, sizeof(SPacketGCChangeName));
			break;
		}
}

void CInputDB::PlayerLoad(LPDESC d, const char * data)
{
	TPlayerTable * pTab = (TPlayerTable *) data;

	if (!d)
		return;

	int32_t lMapIndex = pTab->lMapIndex;
	GPOS pos;

	if (lMapIndex == 0)
	{
		lMapIndex = SECTREE_MANAGER::Instance().GetMapIndex(pTab->x, pTab->y);

		if (lMapIndex == 0) // 좌표를 찾을 수 없다.
		{
			lMapIndex = EMPIRE_START_MAP(d->GetAccountTable().bEmpire);
			pos.x = EMPIRE_START_X(d->GetAccountTable().bEmpire);
			pos.y = EMPIRE_START_Y(d->GetAccountTable().bEmpire);
		}
		else
		{
			pos.x = pTab->x;
			pos.y = pTab->y;
		}
	}
	pTab->lMapIndex = lMapIndex;

	// Private 맵에 있었는데, Private 맵이 사라진 상태라면 출구로 돌아가야 한다.
	// ----
	// 근데 출구로 돌아가야 한다면서... 왜 출구가 아니라 private map 상에 대응되는 pulic map의 위치를 찾냐고...
	// 역사를 모르니... 또 하드코딩 한다.
	// 아귀동굴이면, 출구로...
	// by rtsummit
	if (!SECTREE_MANAGER::Instance().GetValidLocation(pTab->lMapIndex, pTab->x, pTab->y, lMapIndex, pos, d->GetEmpire()))
	{
		sys_err("InputDB::PlayerLoad : cannot find valid location %d x %d (name: %s)", pTab->x, pTab->y, pTab->name);
		lMapIndex = EMPIRE_START_MAP(d->GetAccountTable().bEmpire);
		pos.x = EMPIRE_START_X(d->GetAccountTable().bEmpire);
		pos.y = EMPIRE_START_Y(d->GetAccountTable().bEmpire);
	}

	pTab->x = pos.x;
	pTab->y = pos.y;
	pTab->lMapIndex = lMapIndex;

	if (d->GetCharacter() || d->IsPhase(PHASE_GAME))
	{
		LPCHARACTER p = d->GetCharacter();
		sys_err("login state already has main state (character %s %p)", p->GetName(), get_pointer(p));
		return;
	}

	if (nullptr != CHARACTER_MANAGER::Instance().FindPC(pTab->name))
	{
		sys_err("InputDB: PlayerLoad : %s already exist in game", pTab->name);
		return;
	}

	LPCHARACTER ch = CHARACTER_MANAGER::Instance().CreateCharacter(pTab->name, pTab->id);

	ch->BindDesc(d);
	ch->SetPlayerProto(pTab);
	ch->SetEmpire(d->GetEmpire());

	d->BindCharacter(ch);
	
	{
		// P2P Login
		TPacketGGLogin p;

		p.bHeader = HEADER_GG_LOGIN;
		strlcpy(p.szName, ch->GetName(), sizeof(p.szName));
		p.dwPID = ch->GetPlayerID();
		p.bEmpire = ch->GetEmpire();
		p.lMapIndex = SECTREE_MANAGER::Instance().GetMapIndex(ch->GetX(), ch->GetY());
		p.bChannel = g_bChannel;
		p.iLevel = ch->GetLevel();

		P2P_MANAGER::Instance().Send(&p, sizeof(TPacketGGLogin));

		char buf[51];
		snprintf(buf, sizeof(buf), "%s %d %d %d %d", 
				inet_ntoa(ch->GetDesc()->GetAddr().sin_addr), ch->GetGold(), g_bChannel, ch->GetMapIndex(), ch->GetAlignment());
		LogManager::Instance().CharLog(ch, 0, "LOGIN", buf);
	}

	d->SetPhase(PHASE_LOADING);
	ch->MainCharacterPacket();

	int32_t lPublicMapIndex = lMapIndex >= 10000 ? lMapIndex / 10000 : lMapIndex;

	//if (!map_allow_find(lMapIndex >= 10000 ? lMapIndex / 10000 : lMapIndex) || !CheckEmpire(ch, lMapIndex))
	if (!map_allow_find(lPublicMapIndex))
	{
		sys_err("InputDB::PlayerLoad : entering %d map is not allowed here (name: %s, empire %u)", 
				lMapIndex, pTab->name, d->GetEmpire());

		ch->SetWarpLocation(EMPIRE_START_MAP(d->GetEmpire()),
				EMPIRE_START_X(d->GetEmpire()) / 100,
				EMPIRE_START_Y(d->GetEmpire()) / 100);

		d->SetPhase(PHASE_CLOSE);
		return;
	}

	quest::CQuestManager::Instance().BroadcastEventFlagOnLogin(ch);

	for (int32_t i = 0; i < QUICKSLOT_MAX_NUM; ++i)
		ch->SetQuickslot(i, pTab->quickslot[i]);

	ch->PointsPacket();
	ch->SkillLevelPacket();

	sys_log(0, "InputDB: player_load %s %dx%dx%d LEVEL %d MOV_SPEED %d JOB %d ATG %d DFG %d GMLv %d",
			pTab->name, 
			ch->GetX(), ch->GetY(), ch->GetZ(),
			ch->GetLevel(),
			ch->GetPoint(POINT_MOV_SPEED),
			ch->GetJob(),
			ch->GetPoint(POINT_ATT_GRADE),
			ch->GetPoint(POINT_DEF_GRADE),
			ch->GetGMLevel());

	ch->QuerySafeboxSize();

	CAnticheatManager::Instance().CreateClientHandle(ch);
}

void CInputDB::Boot(const char* data)
{
	signal_timer_disable();

	// 패킷 사이즈 체크
	uint32_t dwPacketSize = decode_4bytes(data);
	data += 4;

	// 패킷 버전 체크
	uint8_t bVersion = decode_byte(data);
	data += 1;

	sys_log(0, "BOOT: PACKET: %d", dwPacketSize);
	sys_log(0, "BOOT: VERSION: %d", bVersion);
	if (bVersion != 6)
	{
		sys_err("boot version error");
		thecore_shutdown();
	}

	sys_log(0, "sizeof(TMobTable) = %d", sizeof(TMobTable));
	sys_log(0, "sizeof(SItemTable_Server) = %d", sizeof(SItemTable_Server));
	sys_log(0, "sizeof(TShopTable) = %d", sizeof(TShopTable));
	sys_log(0, "sizeof(TSkillTable) = %d", sizeof(TSkillTable));
	sys_log(0, "sizeof(TRefineTable) = %d", sizeof(TRefineTable));
	sys_log(0, "sizeof(TItemAttrTable) = %d", sizeof(TItemAttrTable));
	sys_log(0, "sizeof(TItemRareTable) = %d", sizeof(TItemAttrTable));
	sys_log(0, "sizeof(TBanwordTable) = %d", sizeof(TBanwordTable));
	sys_log(0, "sizeof(TLand) = %d", sizeof(building::TLand));
	sys_log(0, "sizeof(TObjectProto) = %d", sizeof(building::TObjectProto));
	sys_log(0, "sizeof(TObject) = %d", sizeof(building::TObject));
	//ADMIN_MANAGER
	sys_log(0, "sizeof(TAdminManager) = %d", sizeof (TAdminInfo) );
	//END_ADMIN_MANAGER

	uint16_t size;

	/*
	 * MOB
	 */

	if (decode_2bytes(data)!=sizeof(TMobTable))
	{
		sys_err("mob table size error");
		thecore_shutdown();
		return;
	}
	data += 2;

	size = decode_2bytes(data);
	data += 2;
	sys_log(0, "BOOT: MOB: %d", size);

	if (size)
	{
		CMobManager::Instance().Initialize((TMobTable *) data, size);
		data += size * sizeof(TMobTable);
	}

	/*
	 * ITEM
	 */

	if (decode_2bytes(data) != sizeof(SItemTable_Server))
	{
		sys_err("item table size error");
		thecore_shutdown();
		return;
	}
	data += 2;

	size = decode_2bytes(data);
	data += 2;
	sys_log(0, "BOOT: ITEM: %d", size);


	if (size)
	{
		ITEM_MANAGER::Instance().Initialize((SItemTable_Server *) data, size);
		data += size * sizeof(SItemTable_Server);
	}

	/*
	 * SHOP
	 */

	if (decode_2bytes(data) != sizeof(TShopTable))
	{
		sys_err("shop table size error");
		thecore_shutdown();
		return;
	}
	data += 2;

	size = decode_2bytes(data);
	data += 2;
	sys_log(0, "BOOT: SHOP: %d", size);


	if (size)
	{
		if (!CShopManager::Instance().Initialize((TShopTable *) data, size))
		{
			sys_err("shop table Initialize error");
			thecore_shutdown();
			return;
		}
		data += size * sizeof(TShopTable);
	}

	/*
	 * SKILL
	 */

	if (decode_2bytes(data) != sizeof(TSkillTable))
	{
		sys_err("skill table size error");
		thecore_shutdown();
		return;
	}
	data += 2;

	size = decode_2bytes(data);
	data += 2;
	sys_log(0, "BOOT: SKILL: %d", size);

	if (size)
	{
		if (!CSkillManager::Instance().Initialize((TSkillTable *) data, size))
		{
			sys_err("cannot initialize skill table");
			thecore_shutdown();
			return;
		}

		data += size * sizeof(TSkillTable);
	}
	/*
	 * REFINE RECIPE
	 */
	if (decode_2bytes(data) != sizeof(TRefineTable))
	{
		sys_err("refine table size error");
		thecore_shutdown();
		return;
	}
	data += 2;

	size = decode_2bytes(data);
	data += 2;
	sys_log(0, "BOOT: REFINE: %d", size);

	if (size)
	{
		CRefineManager::Instance().Initialize((TRefineTable*) data, size);
		data += size * sizeof(TRefineTable);
	}

	/*
	 * ITEM ATTR
	 */
	if (decode_2bytes(data) != sizeof(TItemAttrTable))
	{
		sys_err("item attr table size error");
		thecore_shutdown();
		return;
	}
	data += 2;

	size = decode_2bytes(data);
	data += 2;
	sys_log(0, "BOOT: ITEM_ATTR: %d", size);

	if (size)
	{
		TItemAttrTable * p = (TItemAttrTable *) data;

		for (int32_t i = 0; i < size; ++i, ++p)
		{
			if (p->dwApplyIndex >= MAX_APPLY_NUM)
				continue;

			g_map_itemAttr[p->dwApplyIndex] = *p;
			sys_log(0, "ITEM_ATTR[%d]: %s %u", p->dwApplyIndex, p->szApply, p->dwProb);
		}
	}

	data += size * sizeof(TItemAttrTable);


	/*
     * ITEM RARE
     */
	if (decode_2bytes(data) != sizeof(TItemAttrTable))
	{
		sys_err("item rare table size error");
		thecore_shutdown();
		return;
	}
	data += 2;

	size = decode_2bytes(data);
	data += 2;
	sys_log(0, "BOOT: ITEM_RARE: %d", size);

	if (size)
	{
		TItemAttrTable * p = (TItemAttrTable *) data;

		for (int32_t i = 0; i < size; ++i, ++p)
		{
			if (p->dwApplyIndex >= MAX_APPLY_NUM)
				continue;

			g_map_itemRare[p->dwApplyIndex] = *p;
			sys_log(0, "ITEM_RARE[%d]: %s %u", p->dwApplyIndex, p->szApply, p->dwProb);
		}
	}

	data += size * sizeof(TItemAttrTable);


	/*
	 * BANWORDS
	 */

	if (decode_2bytes(data) != sizeof(TBanwordTable))
	{
		sys_err("ban word table size error");
		thecore_shutdown();
		return;
	}
	data += 2;

	size = decode_2bytes(data);
	data += 2;

	if (size)
	{
		CBanwordManager::Instance().Initialize((TBanwordTable *) data, size);
		data += size * sizeof(TBanwordTable);
	}
	sys_log(0, "[BANWORD] Size %d Count %d", sizeof(TBanwordTable), size);

	{
		using namespace building;

		/*
		 * LANDS
		 */

		if (decode_2bytes(data) != sizeof(TLand))
		{
			sys_err("land table size error");
			thecore_shutdown();
			return;
		}
		data += 2;

		size = decode_2bytes(data);
		data += 2;

		if (size)
		{
			TLand * kLand = (TLand *) data;
			data += size * sizeof(TLand);

			for (uint16_t i = 0; i < size; ++i, ++kLand)
				CManager::Instance().LoadLand(kLand);
		}
		sys_log (0, "[LAND] Size %d Count %d", sizeof (TLand), size);
		/*
		 * OBJECT PROTO
		 */

		if (decode_2bytes(data) != sizeof(TObjectProto))
		{
			sys_err("object proto table size error");
			thecore_shutdown();
			return;
		}
		data += 2;

		size = decode_2bytes(data);
		data += 2;

		CManager::Instance().LoadObjectProto((TObjectProto *) data, size);
		data += size * sizeof(TObjectProto);

		/*
		 * OBJECT 
		 */
		if (decode_2bytes(data) != sizeof(TObject))
		{
			sys_err("object table size error");
			thecore_shutdown();
			return;
		}
		data += 2;

		size = decode_2bytes(data);
		data += 2;

		TObject * kObj = (TObject *) data;
		data += size * sizeof(TObject);

		for (uint16_t i = 0; i < size; ++i, ++kObj)
			CManager::Instance().LoadObject(kObj, true);
	}

	set_global_time(*(time_t *) data);
	data += sizeof(time_t);

	if (decode_2bytes(data) != sizeof(TItemIDRangeTable) )
	{
		sys_err("ITEM ID RANGE size error");
		thecore_shutdown();
		return;
	}
	data += 2;

	size = decode_2bytes(data);
	data += 2;

	TItemIDRangeTable* range = (TItemIDRangeTable*) data;
	data += size * sizeof(TItemIDRangeTable);

	TItemIDRangeTable* rangespare = (TItemIDRangeTable*) data;
	data += size * sizeof(TItemIDRangeTable);

	//ADMIN_MANAGER
	data += 2;
	int32_t adminsize = decode_2bytes(data);
	data += 2;

	for (int32_t n = 0; n < adminsize; ++n)
	{
		tAdminInfo& rAdminInfo = *(tAdminInfo*)data;

		GM::insert(rAdminInfo);

		data += sizeof(rAdminInfo);
	}

	GM::init((uint32_t*)data);
	data += sizeof(uint32_t) * GM_MAX_NUM;

	//END_ADMIN_MANAGER

	uint16_t endCheck=decode_2bytes(data);
	if (endCheck != 0xffff)
	{
		sys_err("boot packet end check error [%x]!=0xffff", endCheck);
		thecore_shutdown();
		return;
	}
	else
		sys_log(0, "boot packet end check ok [%x]==0xffff", endCheck );
	data +=2; 

	if (!ITEM_MANAGER::Instance().SetMaxItemID(*range))
	{
		sys_err("not enough item id contact your administrator!");
		thecore_shutdown();
		return;
	}

	if (!ITEM_MANAGER::Instance().SetMaxSpareItemID(*rangespare))
	{
		sys_err("not enough item id for spare contact your administrator!");
		thecore_shutdown();
		return;
	}



	// LOCALE_SERVICE
	const int32_t FILE_NAME_LEN = 256;
	char szCommonDropItemFileName[FILE_NAME_LEN];
	char szETCDropItemFileName[FILE_NAME_LEN];
	char szMOBDropItemFileName[FILE_NAME_LEN];
	char szDropItemGroupFileName[FILE_NAME_LEN];
	char szSpecialItemGroupFileName[FILE_NAME_LEN];
	char szMapIndexFileName[FILE_NAME_LEN];
	char szItemVnumMaskTableFileName[FILE_NAME_LEN];
	char szDragonSoulTableFileName[FILE_NAME_LEN];

	snprintf(szCommonDropItemFileName, sizeof(szCommonDropItemFileName),
			"%s/common_drop_item.txt", LocaleService_GetBasePath().c_str());
	snprintf(szETCDropItemFileName, sizeof(szETCDropItemFileName),
			"%s/etc_drop_item.txt", LocaleService_GetBasePath().c_str());
	snprintf(szMOBDropItemFileName, sizeof(szMOBDropItemFileName),
			"%s/mob_drop_item.txt", LocaleService_GetBasePath().c_str());
	snprintf(szSpecialItemGroupFileName, sizeof(szSpecialItemGroupFileName),
			"%s/special_item_group.txt", LocaleService_GetBasePath().c_str());
	snprintf(szDropItemGroupFileName, sizeof(szDropItemGroupFileName),
			"%s/drop_item_group.txt", LocaleService_GetBasePath().c_str());
	snprintf(szMapIndexFileName, sizeof(szMapIndexFileName),
			"%s/index", LocaleService_GetMapPath().c_str());
	snprintf(szItemVnumMaskTableFileName, sizeof(szItemVnumMaskTableFileName),
			"%s/ori_to_new_table.txt", LocaleService_GetBasePath().c_str());
	snprintf(szDragonSoulTableFileName, sizeof(szDragonSoulTableFileName),
			"%s/dragon_soul_table.txt", LocaleService_GetBasePath().c_str());

	sys_log(0, "Initializing Informations of Cube System");
	if (!Cube_InformationInitialize())
	{
		sys_err("cannot init cube infomation.");
		thecore_shutdown();
		return;
	}

	sys_log(0, "LoadLocaleFile: CommonDropItem: %s", szCommonDropItemFileName);
	if (!ITEM_MANAGER::Instance().ReadCommonDropItemFile(szCommonDropItemFileName))
	{
		sys_err("cannot load CommonDropItem: %s", szCommonDropItemFileName);
		thecore_shutdown();
		return;
	}

	sys_log(0, "LoadLocaleFile: ETCDropItem: %s", szETCDropItemFileName);
	if (!ITEM_MANAGER::Instance().ReadEtcDropItemFile(szETCDropItemFileName))
	{
		sys_err("cannot load ETCDropItem: %s", szETCDropItemFileName);
		thecore_shutdown();
		return;
	}

	sys_log(0, "LoadLocaleFile: DropItemGroup: %s", szDropItemGroupFileName);
	if (!ITEM_MANAGER::Instance().ReadDropItemGroup(szDropItemGroupFileName))
	{
		sys_err("cannot load DropItemGroup: %s", szDropItemGroupFileName);
		thecore_shutdown();
		return;
	}

	sys_log(0, "LoadLocaleFile: SpecialItemGroup: %s", szSpecialItemGroupFileName);
	if (!ITEM_MANAGER::Instance().ReadSpecialDropItemFile(szSpecialItemGroupFileName))
	{
		sys_err("cannot load SpecialItemGroup: %s", szSpecialItemGroupFileName);
		thecore_shutdown();
		return;
	}

	sys_log(0, "LoadLocaleFile: ItemVnumMaskTable : %s", szItemVnumMaskTableFileName);
	if (!ITEM_MANAGER::Instance().ReadItemVnumMaskTable(szItemVnumMaskTableFileName))
	{
		sys_log(0, "Could not open MaskItemTable");
	}

	sys_log(0, "LoadLocaleFile: MOBDropItemFile: %s", szMOBDropItemFileName);
	if (!ITEM_MANAGER::Instance().ReadMonsterDropItemGroup(szMOBDropItemFileName))
	{
		sys_err("cannot load MOBDropItemFile: %s", szMOBDropItemFileName);
		thecore_shutdown();
		return;
	}

	sys_log(0, "LoadLocaleFile: MapIndex: %s", szMapIndexFileName);
	if (!SECTREE_MANAGER::Instance().Build(szMapIndexFileName, LocaleService_GetMapPath().c_str()))
	{
		sys_err("cannot load MapIndex: %s", szMapIndexFileName);
		thecore_shutdown();
		return;
	}

	sys_log(0, "LoadLocaleFile: DragonSoulTable: %s", szDragonSoulTableFileName);
	if (!DSManager::Instance().ReadDragonSoulTableFile(szDragonSoulTableFileName))
	{
		sys_err("cannot load DragonSoulTable: %s", szDragonSoulTableFileName);
		//thecore_shutdown();
		//return;
	}

	// END_OF_LOCALE_SERVICE


	building::CManager::Instance().FinalizeBoot();

	CMotionManager::Instance().Build();

	signal_timer_enable(30);

	if (g_bIsTestServer)
	{
		CMobManager::Instance().DumpRegenCount("mob_count");
	}
}

EVENTINFO(quest_login_event_info)
{
	uint32_t dwPID;

	quest_login_event_info() 
	: dwPID( 0 )
	{
	}
};

EVENTFUNC(quest_login_event)
{
	quest_login_event_info* info = dynamic_cast<quest_login_event_info*>( event->info );

	if ( info == nullptr )
	{
		sys_err( "quest_login_event> <Factor> Null pointer" );
		return 0;
	}

	uint32_t dwPID = info->dwPID;

	LPCHARACTER ch = CHARACTER_MANAGER::Instance().FindByPID(dwPID);

	if (!ch)
		return 0;

	LPDESC d = ch->GetDesc();

	if (!d)
		return 0;

	if (d->IsPhase(PHASE_HANDSHAKE) ||
		d->IsPhase(PHASE_LOGIN) ||
		d->IsPhase(PHASE_SELECT) ||
		d->IsPhase(PHASE_DEAD) ||
		d->IsPhase(PHASE_LOADING))
	{
		return PASSES_PER_SEC(1);
	}
	else if (d->IsPhase(PHASE_CLOSE))
	{
		return 0;
	}
	else if (d->IsPhase(PHASE_GAME))
	{
		sys_log(0, "QUEST_LOAD: Login pc %d by event", ch->GetPlayerID());
		quest::CQuestManager::Instance().Login(ch->GetPlayerID());
		return 0;
	}
	else
	{
		sys_err(0, "input_db.cpp:quest_login_event INVALID PHASE pid %d", ch->GetPlayerID());
		return 0;
	}
}

void CInputDB::QuestLoad(LPDESC d, const char * c_pData)
{
	if (nullptr == d)
		return;

	LPCHARACTER ch = d->GetCharacter();

	if (nullptr == ch)
		return;

	const uint32_t dwCount = decode_4bytes(c_pData);

	const TQuestTable* pQuestTable = reinterpret_cast<const TQuestTable*>(c_pData+4);

	if (nullptr != pQuestTable)
	{
		if (dwCount != 0)
		{
			if (ch->GetPlayerID() != pQuestTable[0].dwPID)
			{
				sys_err("PID differs %u %u", ch->GetPlayerID(), pQuestTable[0].dwPID);
				return;
			}
		}

		sys_log(0, "QUEST_LOAD: count %d", dwCount);

		quest::PC * pkPC = quest::CQuestManager::Instance().GetPCForce(ch->GetPlayerID());

		if (!pkPC)
		{
			sys_err("null quest::PC with id %u", pQuestTable[0].dwPID);
			return;
		}

		if (pkPC->IsLoaded())
			return;

		for (uint32_t i = 0; i < dwCount; ++i)
		{
			std::string st(pQuestTable[i].szName);

			st += ".";
			st += pQuestTable[i].szState;

			sys_log(0,  "            %s %d", st.c_str(), pQuestTable[i].lValue);
			pkPC->SetFlag(st.c_str(), pQuestTable[i].lValue, false);
		}

		pkPC->SetLoaded();
		pkPC->Build();

		if (ch->GetDesc()->IsPhase(PHASE_GAME))
		{
			sys_log(0, "QUEST_LOAD: Login pc %d", pQuestTable[0].dwPID);
			quest::CQuestManager::Instance().Login(pQuestTable[0].dwPID);
		}
		else
		{
			quest_login_event_info* info = AllocEventInfo<quest_login_event_info>();
			info->dwPID = ch->GetPlayerID();

			event_create(quest_login_event, info, PASSES_PER_SEC(1));
		}
	}	
}

void CInputDB::SafeboxLoad(LPDESC d, const char * c_pData)
{
	if (!d)
		return;

	TSafeboxTable * p = (TSafeboxTable *) c_pData;

	if (d->GetAccountTable().id != p->dwID)
	{
		sys_err("SafeboxLoad: safebox has different id %u != %u", d->GetAccountTable().id, p->dwID);
		return;
	}

	if (!d->GetCharacter())
		return;

	uint8_t bSize = 1;

	LPCHARACTER ch = d->GetCharacter();

	//PREVENT_TRADE_WINDOW
	if (ch->GetShopOwner() || ch->GetExchange() || ch->GetMyShop() || ch->IsCubeOpen() )
	{
		d->GetCharacter()->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("다른거래창이 열린상태에서는 창고를 열수가 없습니다." ) );
		d->GetCharacter()->CancelSafeboxLoad();
		return;
	}
	//END_PREVENT_TRADE_WINDOW

	// ADD_PREMIUM
	if (d->GetCharacter()->GetPremiumRemainSeconds(PREMIUM_SAFEBOX) > 0 ||
			d->GetCharacter()->IsEquipUniqueGroup(UNIQUE_GROUP_LARGE_SAFEBOX))
		bSize = 3;
	// END_OF_ADD_PREMIUM

	//if (d->GetCharacter()->IsEquipUniqueItem(UNIQUE_ITEM_SAFEBOX_EXPAND))
	//bSize = 3; // 창고확장권

	//d->GetCharacter()->LoadSafebox(p->bSize * SAFEBOX_PAGE_SIZE, p->dwGold, p->wItemCount, (TPlayerItem *) (c_pData + sizeof(TSafeboxTable)));
	d->GetCharacter()->LoadSafebox(bSize * SAFEBOX_PAGE_SIZE, p->dwGold, p->wItemCount, (TPlayerItem *) (c_pData + sizeof(TSafeboxTable)));
}

void CInputDB::SafeboxChangeSize(LPDESC d, const char * c_pData)
{
	if (!d)
		return;

	uint8_t bSize = *(uint8_t *) c_pData;

	if (!d->GetCharacter())
		return;

	d->GetCharacter()->ChangeSafeboxSize(bSize);
}

//
// @version	05/06/20 Bang2ni - ReqSafeboxLoad 의 취소
//
void CInputDB::SafeboxWrongPassword(LPDESC d)
{
	if (!d)
		return;

	if (!d->GetCharacter())
		return;

	SPacketGCSafeboxWrongPassword p;
	d->Packet(&p, sizeof(p));

	d->GetCharacter()->CancelSafeboxLoad();
}

void CInputDB::SafeboxChangePasswordAnswer(LPDESC d, const char* c_pData)
{
	if (!d)
		return;

	if (!d->GetCharacter())
		return;

	TSafeboxChangePasswordPacketAnswer* p = (TSafeboxChangePasswordPacketAnswer*) c_pData;
	if (p->flag)
	{
		d->GetCharacter()->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("<창고> 창고 비밀번호가 변경되었습니다."));
	}
	else
	{
		d->GetCharacter()->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("<창고> 기존 비밀번호가 틀렸습니다."));
	}
}

void CInputDB::MallLoad(LPDESC d, const char * c_pData)
{
	if (!d)
		return;

	TSafeboxTable * p = (TSafeboxTable *) c_pData;

	if (d->GetAccountTable().id != p->dwID)
	{
		sys_err("safebox has different id %u != %u", d->GetAccountTable().id, p->dwID);
		return;
	}

	if (!d->GetCharacter())
		return;

	d->GetCharacter()->LoadMall(p->wItemCount, (TPlayerItem *) (c_pData + sizeof(TSafeboxTable)));
}

void CInputDB::LoginAlready(LPDESC d, const char * c_pData)
{
	if (!d)
		return;

	// INTERNATIONAL_VERSION 이미 접속중이면 접속 끊음
	{ 
		TPacketDGLoginAlready * p = (TPacketDGLoginAlready *) c_pData;

		LPDESC d2 = DESC_MANAGER::Instance().FindByLoginName(p->szLogin);

		if (d2)
			d2->DisconnectOfSameLogin();
		else
		{
			TPacketGGDisconnect pgg;

			pgg.bHeader = HEADER_GG_DISCONNECT;
			strlcpy(pgg.szLogin, p->szLogin, sizeof(pgg.szLogin));

			P2P_MANAGER::Instance().Send(&pgg, sizeof(TPacketGGDisconnect));
		}
	}
	// END_OF_INTERNATIONAL_VERSION

	LoginFailure(d, "ALREADY");
}

void CInputDB::EmpireSelect(LPDESC d, const char * c_pData)
{
	sys_log(0, "EmpireSelect %p", get_pointer(d));

	if (!d)
		return;

	TAccountTable & rTable = d->GetAccountTable();
	rTable.bEmpire = *(uint8_t *) c_pData;

	SPacketGCEmpire pe;
	pe.bEmpire = rTable.bEmpire;
	d->Packet(&pe, sizeof(pe));

	for (int32_t i = 0; i < PLAYER_PER_ACCOUNT; ++i)
	{
		if (rTable.players[i].dwID)
		{
			rTable.players[i].x = EMPIRE_START_X(rTable.bEmpire);
			rTable.players[i].y = EMPIRE_START_Y(rTable.bEmpire);
		}
	}

	GetServerLocation(d->GetAccountTable(), rTable.bEmpire);

	d->SendLoginSuccessPacket();
}

void CInputDB::MapLocations(const char * c_pData)
{
	uint8_t bCount = *(uint8_t *) (c_pData++);

	sys_log(0, "InputDB::MapLocations %d", bCount);

	TMapLocation * pLoc = (TMapLocation *) c_pData;

	while (bCount--)
	{
		for (int32_t i = 0; i < MAP_ALLOW_LIMIT; ++i)
		{
			if (0 == pLoc->alMaps[i])
				break;

			CMapLocation::Instance().Insert(pLoc->alMaps[i], pLoc->szHost, pLoc->wPort);
		}

		pLoc++;
	}
}

void CInputDB::P2P(const char * c_pData)
{
	extern LPFDWATCH main_fdw;

	TPacketDGP2P * p = (TPacketDGP2P *) c_pData;

	P2P_MANAGER& mgr = P2P_MANAGER::Instance();

	if (false == DESC_MANAGER::Instance().IsP2PDescExist(p->szHost, p->wPort))
	{
	    LPCLIENT_DESC pkDesc = nullptr;
		sys_log(0, "InputDB:P2P %s:%u", p->szHost, p->wPort);
	    pkDesc = DESC_MANAGER::Instance().CreateConnectionDesc(main_fdw, p->szHost, p->wPort, PHASE_P2P, false);
		mgr.RegisterConnector(pkDesc);
		pkDesc->SetP2P(p->szHost, p->wPort, p->bChannel);
		pkDesc->SetListenPort(p->wListenPort);
	}
}

void CInputDB::GuildLoad(const char * c_pData)
{
	CGuildManager::Instance().LoadGuild(*(uint32_t *) c_pData);
}

void CInputDB::GuildSkillUpdate(const char* c_pData)
{
	TPacketGuildSkillUpdate * p = (TPacketGuildSkillUpdate *) c_pData;

	CGuild * g = CGuildManager::Instance().TouchGuild(p->guild_id);

	if (g)
	{
		g->UpdateSkill(p->skill_point, p->skill_levels);
		g->GuildPointChange(POINT_SP, p->amount, p->save?true:false);
	}
}

void CInputDB::GuildWar(const char* c_pData)
{
	TPacketGuildWar * p = (TPacketGuildWar*) c_pData;

	sys_log(0, "InputDB::GuildWar %u %u state %d", p->dwGuildFrom, p->dwGuildTo, p->bWar);

	switch (p->bWar)
	{
		case GUILD_WAR_SEND_DECLARE:
		case GUILD_WAR_RECV_DECLARE:
			CGuildManager::Instance().DeclareWar(p->dwGuildFrom, p->dwGuildTo, p->bType);
			break;

		case GUILD_WAR_REFUSE:
			CGuildManager::Instance().RefuseWar(p->dwGuildFrom, p->dwGuildTo);
			break;

		case GUILD_WAR_WAIT_START:
			CGuildManager::Instance().WaitStartWar(p->dwGuildFrom, p->dwGuildTo);
			break;

		case GUILD_WAR_CANCEL:
			CGuildManager::Instance().CancelWar(p->dwGuildFrom, p->dwGuildTo);
			break;

		case GUILD_WAR_ON_WAR:
			CGuildManager::Instance().StartWar(p->dwGuildFrom, p->dwGuildTo);
			break;

		case GUILD_WAR_END:
			CGuildManager::Instance().EndWar(p->dwGuildFrom, p->dwGuildTo);
			break;

		case GUILD_WAR_OVER:
			CGuildManager::Instance().WarOver(p->dwGuildFrom, p->dwGuildTo, p->bType);
			break;

		case GUILD_WAR_RESERVE:
			CGuildManager::Instance().ReserveWar(p->dwGuildFrom, p->dwGuildTo, p->bType);
			break;

		default:
			sys_err("Unknown guild war state");
			break;
	}
}

void CInputDB::GuildWarScore(const char* c_pData)
{
	TPacketGuildWarScore* p = (TPacketGuildWarScore*) c_pData;
	CGuild * g = CGuildManager::Instance().TouchGuild(p->dwGuildGainPoint);
	g->SetWarScoreAgainstTo(p->dwGuildOpponent, p->lScore);
}

void CInputDB::GuildSkillRecharge()
{
	CGuildManager::Instance().SkillRecharge();
}

void CInputDB::GuildExpUpdate(const char* c_pData)
{
	TPacketGuildSkillUpdate * p = (TPacketGuildSkillUpdate *) c_pData;
	sys_log(1, "GuildExpUpdate %d", p->amount);

	CGuild * g = CGuildManager::Instance().TouchGuild(p->guild_id);

	if (g)
		g->GuildPointChange(POINT_EXP, p->amount);
}

void CInputDB::GuildAddMember(const char* c_pData)
{
	TPacketDGGuildMember * p = (TPacketDGGuildMember *) c_pData;
	CGuild * g = CGuildManager::Instance().TouchGuild(p->dwGuild);

	if (g)
		g->AddMember(p);
}

void CInputDB::GuildRemoveMember(const char* c_pData)
{
	TPacketGuild* p=(TPacketGuild*)c_pData;
	CGuild* g = CGuildManager::Instance().TouchGuild(p->dwGuild);

	if (g)
		g->RemoveMember(p->dwInfo);
}

void CInputDB::GuildChangeGrade(const char* c_pData)
{
	TPacketGuild* p=(TPacketGuild*)c_pData;
	CGuild* g = CGuildManager::Instance().TouchGuild(p->dwGuild);

	if (g)
		g->P2PChangeGrade((uint8_t)p->dwInfo);
}

void CInputDB::GuildChangeMemberData(const char* c_pData)
{
	sys_log(0, "Recv GuildChangeMemberData");
	TPacketGuildChangeMemberData * p = (TPacketGuildChangeMemberData *) c_pData;
	CGuild * g = CGuildManager::Instance().TouchGuild(p->guild_id);

	if (g)
		g->ChangeMemberData(p->pid, p->offer, p->level, p->grade);
}

void CInputDB::GuildDisband(const char* c_pData)
{
	TPacketGuild * p = (TPacketGuild*) c_pData;
	CGuildManager::Instance().DisbandGuild(p->dwGuild);
}

void CInputDB::GuildLadder(const char* c_pData)
{
	TPacketGuildLadder* p = (TPacketGuildLadder*) c_pData;
	sys_log(0, "Recv GuildLadder %u %d / w %d d %d l %d", p->dwGuild, p->lLadderPoint, p->lWin, p->lDraw, p->lLoss);
	CGuild * g = CGuildManager::Instance().TouchGuild(p->dwGuild);

	g->SetLadderPoint(p->lLadderPoint);
	g->SetWarData(p->lWin, p->lDraw, p->lLoss);
}

void CInputDB::ItemLoad(LPDESC d, const char * c_pData)
{
	LPCHARACTER ch;

	if (!d || !(ch = d->GetCharacter()))
		return;

	if (ch->IsItemLoaded())
		return;

	uint32_t dwCount = decode_4bytes(c_pData);
	c_pData += sizeof(uint32_t);

	sys_log(0, "ITEM_LOAD: COUNT %s %u", ch->GetName(), dwCount);

	std::vector<LPITEM> v;

	TPlayerItem * p = (TPlayerItem *) c_pData;

	for (uint32_t i = 0; i < dwCount; ++i, ++p)
	{
		LPITEM item = ITEM_MANAGER::Instance().CreateItem(p->vnum, p->count, p->id);

		if (!item)
		{
			sys_err("cannot create item by vnum %u (name %s id %u)", p->vnum, ch->GetName(), p->id);
			continue;
		}

		item->SetSkipSave(true);
		item->SetSockets(p->alSockets);
		item->SetAttributes(p->aAttr);
		item->SetGMOwner(p->is_gm_owner);

#ifdef ENABLE_HIGHLIGHT_NEW_ITEM
		item->SetLastOwnerPID(p->owner);
#endif

		if (p->window == BELT_INVENTORY)
		{
			p->window = INVENTORY;
			p->pos = p->pos + BELT_INVENTORY_SLOT_START;
		}

		if ((p->window == INVENTORY && ch->GetInventoryItem(p->pos)) ||
				(p->window == EQUIPMENT && ch->GetWear(p->pos)))
		{
			sys_log(0, "ITEM_RESTORE: %s %s", ch->GetName(), item->GetName());
			v.push_back(item);
		}
		else
		{
			switch (p->window)
			{
				case INVENTORY:
				case DRAGON_SOUL_INVENTORY:
					item->AddToCharacter(ch, TItemPos(p->window, p->pos));
					break;

				case EQUIPMENT:
					if (item->CheckItemUseLevel(ch->GetLevel()) == true )
					{
						if (item->EquipTo(ch, p->pos) == false )
						{
							v.push_back(item);
						}
					}
					else
					{
						v.push_back(item);
					}
					break;
			}
		}

		if (false == item->OnAfterCreatedItem())
			sys_err("Failed to call ITEM::OnAfterCreatedItem (vnum: %d, id: %d)", item->GetVnum(), item->GetID());

		item->SetSkipSave(false);
	}

	auto it = v.begin();

	while (it != v.end())
	{
		LPITEM item = *(it++);

		int32_t pos = ch->GetEmptyInventory(item->GetSize());

		if (pos < 0)
		{
			GPOS coord;
			coord.x = ch->GetX();
			coord.y = ch->GetY();

			item->AddToGround(ch->GetMapIndex(), coord);
			item->SetOwnership(ch, 180);
			item->StartDestroyEvent();
		}
		else
			item->AddToCharacter(ch, TItemPos(INVENTORY, pos));
	}

	ch->CheckMaximumPoints();
	ch->PointsPacket();

	ch->SetItemLoaded();
}

void CInputDB::AffectLoad(LPDESC d, const char * c_pData)
{
	if (!d)
		return;

	if (!d->GetCharacter())
		return;

	LPCHARACTER ch = d->GetCharacter();

	uint32_t dwPID = decode_4bytes(c_pData);
	c_pData += sizeof(uint32_t);

	uint32_t dwCount = decode_4bytes(c_pData);
	c_pData += sizeof(uint32_t);

	if (ch->GetPlayerID() != dwPID)
		return;

	ch->LoadAffect(dwCount, (TPacketAffectElement *) c_pData);
	
}

void CInputDB::ActivityLoad(LPDESC d, const char* c_pData)
{
	if (!d || !d->GetCharacter())
		return;

	LPCHARACTER ch = d->GetCharacter();
	if (!ch)
		return;

	uint32_t dwPID = decode_4bytes(c_pData);
	c_pData += sizeof(uint32_t);

	if (ch->GetPlayerID() != dwPID)
		return;

	ch->LoadActivity((TActivityTable*)c_pData);
}

void CInputDB::PartyCreate(const char* c_pData)
{
	TPacketPartyCreate* p = (TPacketPartyCreate*) c_pData;
	CPartyManager::Instance().P2PCreateParty(p->dwLeaderPID);
}

void CInputDB::PartyDelete(const char* c_pData)
{
	TPacketPartyDelete* p = (TPacketPartyDelete*) c_pData;
	CPartyManager::Instance().P2PDeleteParty(p->dwLeaderPID);
}

void CInputDB::PartyAdd(const char* c_pData)
{
	TPacketPartyAdd* p = (TPacketPartyAdd*) c_pData;
	CPartyManager::Instance().P2PJoinParty(p->dwLeaderPID, p->dwPID, p->bState);
}

void CInputDB::PartyRemove(const char* c_pData)
{
	TPacketPartyRemove* p = (TPacketPartyRemove*) c_pData;
	CPartyManager::Instance().P2PQuitParty(p->dwPID);
}

void CInputDB::PartyStateChange(const char* c_pData)
{
	TPacketPartyStateChange * p = (TPacketPartyStateChange *) c_pData;
	LPPARTY pParty = CPartyManager::Instance().P2PCreateParty(p->dwLeaderPID);

	if (!pParty)
		return;

	pParty->SetRole(p->dwPID, p->bRole, p->bFlag);
}

void CInputDB::PartySetMemberLevel(const char* c_pData)
{
	TPacketPartySetMemberLevel* p = (TPacketPartySetMemberLevel*) c_pData;
	LPPARTY pParty = CPartyManager::Instance().P2PCreateParty(p->dwLeaderPID);

	if (!pParty)
		return;

	pParty->P2PSetMemberLevel(p->dwPID, p->bLevel);
}

void CInputDB::Time(const char * c_pData)
{
	set_global_time(*(time_t *) c_pData);
}

void CInputDB::ReloadProto(const char * c_pData)
{
	uint16_t wSize;

	/*
	 * Skill
	 */
	wSize = decode_2bytes(c_pData);
	c_pData += sizeof(uint16_t);
	if (wSize) CSkillManager::Instance().Initialize((TSkillTable *) c_pData, wSize);
	c_pData += sizeof(TSkillTable) * wSize;

	/*
	 * Banwords
	 */

	wSize = decode_2bytes(c_pData);
	c_pData += sizeof(uint16_t);
	CBanwordManager::Instance().Initialize((TBanwordTable *) c_pData, wSize);
	c_pData += sizeof(TBanwordTable) * wSize;

	/*
	 * ITEM
	 */
	wSize = decode_2bytes(c_pData);
	c_pData += 2;
	sys_log(0, "RELOAD: ITEM: %d", wSize);

	if (wSize)
	{
		ITEM_MANAGER::Instance().Initialize((SItemTable_Server *) c_pData, wSize);
		c_pData += wSize * sizeof(SItemTable_Server);
	}

	/*
	 * MONSTER
	 */
	wSize = decode_2bytes(c_pData);
	c_pData += 2;
	sys_log(0, "RELOAD: MOB: %d", wSize);

	if (wSize)
	{
		CMobManager::Instance().Initialize((TMobTable *) c_pData, wSize);
		c_pData += wSize * sizeof(TMobTable);
	}

	CMotionManager::Instance().Build();

	CHARACTER_MANAGER::Instance().for_each_pc(std::mem_fn(&CHARACTER::ComputePoints));
}

void CInputDB::GuildSkillUsableChange(const char* c_pData)
{
	TPacketGuildSkillUsableChange* p = (TPacketGuildSkillUsableChange*) c_pData;

	CGuild* g = CGuildManager::Instance().TouchGuild(p->dwGuild);

	g->SkillUsableChange(p->dwSkillVnum, p->bUsable?true:false);
}

void CInputDB::AuthLogin(LPDESC d, const char * c_pData)
{
	if (!d)
		return;

	uint8_t bResult = *(uint8_t *) c_pData;

	SPacketGCAuthSuccess ptoc;

	if (bResult)
	{
		ptoc.dwLoginKey = d->GetLoginKey();
	}
	else
	{
		ptoc.dwLoginKey = 0;
	}

	ptoc.bResult = bResult;

	d->Packet(&ptoc, sizeof(SPacketGCAuthSuccess));
	sys_log(0, "AuthLogin result %u key %u", bResult, d->GetLoginKey());
}

void CInputDB::ChangeEmpirePriv(const char* c_pData)
{
	TPacketDGChangeEmpirePriv* p = (TPacketDGChangeEmpirePriv*) c_pData;

	// ADD_EMPIRE_PRIV_TIME
	CPrivManager::Instance().GiveEmpirePriv(p->empire, p->type, p->value, p->bLog, p->end_time_sec);
	// END_OF_ADD_EMPIRE_PRIV_TIME
}

/**
 * @version 05/06/08	Bang2ni - 지속시간 추가
 */
void CInputDB::ChangeGuildPriv(const char* c_pData)
{
	TPacketDGChangeGuildPriv* p = (TPacketDGChangeGuildPriv*) c_pData;

	// ADD_GUILD_PRIV_TIME
	CPrivManager::Instance().GiveGuildPriv(p->guild_id, p->type, p->value, p->bLog, p->end_time_sec);
	// END_OF_ADD_GUILD_PRIV_TIME
}

void CInputDB::ChangeCharacterPriv(const char* c_pData)
{
	TPacketDGChangeCharacterPriv* p = (TPacketDGChangeCharacterPriv*) c_pData;
	CPrivManager::Instance().GiveCharacterPriv(p->pid, p->type, p->value, p->bLog);
}

void CInputDB::GuildMoneyChange(const char* c_pData)
{
	TPacketDGGuildMoneyChange* p = (TPacketDGGuildMoneyChange*) c_pData;

	CGuild* g = CGuildManager::Instance().TouchGuild(p->dwGuild);
	if (g)
	{
		g->RecvMoneyChange(p->iTotalGold);
	}
}

void CInputDB::GuildWithdrawMoney(const char* c_pData)
{
	TPacketDGGuildMoneyWithdraw* p = (TPacketDGGuildMoneyWithdraw*) c_pData;

	CGuild* g = CGuildManager::Instance().TouchGuild(p->dwGuild);
	if (g)
	{
		g->RecvWithdrawMoneyGive(p->iChangeGold);
	}
}

void CInputDB::SetEventFlag(const char* c_pData)
{
	TPacketSetEventFlag* p = (TPacketSetEventFlag*) c_pData;
	quest::CQuestManager::Instance().SetEventFlag(p->szFlagName, p->lValue);
}

void CInputDB::CreateObject(const char * c_pData)
{
	using namespace building;
	CManager::Instance().LoadObject((TObject *) c_pData);
}

void CInputDB::DeleteObject(const char * c_pData)
{
	using namespace building;
	CManager::Instance().DeleteObject(*(uint32_t *) c_pData);
}

void CInputDB::Notice(const char * c_pData)
{
	char szBuf[256+1];
	strlcpy(szBuf, c_pData, sizeof(szBuf));

	sys_log(0, "InputDB:: Notice: %s", szBuf);

	//SendNotice(LC_TEXT(szBuf));
	SendNotice(szBuf);
}

void CInputDB::GuildWarReserveAdd(TGuildWarReserve * p)
{
	CGuildManager::Instance().ReserveWarAdd(p);
}

void CInputDB::GuildWarReserveDelete(uint32_t dwID)
{
	CGuildManager::Instance().ReserveWarDelete(dwID);
}

void CInputDB::GuildWarBet(TPacketGDGuildWarBet * p)
{
	CGuildManager::Instance().ReserveWarBet(p);
}

void CInputDB::MarriageAdd(TPacketMarriageAdd * p)
{
	sys_log(0, "MarriageAdd %u %u %u %s %s", p->dwPID1, p->dwPID2, (uint32_t)p->tMarryTime, p->szName1, p->szName2);
	marriage::CManager::Instance().Add(p->dwPID1, p->dwPID2, p->tMarryTime, p->szName1, p->szName2);
}

void CInputDB::MarriageUpdate(TPacketMarriageUpdate * p)
{
	sys_log(0, "MarriageUpdate %u %u %d %d", p->dwPID1, p->dwPID2, p->iLovePoint, p->byMarried);
	marriage::CManager::Instance().Update(p->dwPID1, p->dwPID2, p->iLovePoint, p->byMarried);
}

void CInputDB::MarriageRemove(TPacketMarriageRemove * p)
{
	sys_log(0, "MarriageRemove %u %u", p->dwPID1, p->dwPID2);
	marriage::CManager::Instance().Remove(p->dwPID1, p->dwPID2);
}

void CInputDB::WeddingRequest(TPacketWeddingRequest* p)
{
	marriage::WeddingManager::Instance().Request(p->dwPID1, p->dwPID2);
}

void CInputDB::WeddingReady(TPacketWeddingReady* p)
{
	sys_log(0, "WeddingReady %u %u %u", p->dwPID1, p->dwPID2, p->dwMapIndex);
	marriage::CManager::Instance().WeddingReady(p->dwPID1, p->dwPID2, p->dwMapIndex);
}

void CInputDB::WeddingStart(TPacketWeddingStart* p)
{
	sys_log(0, "WeddingStart %u %u", p->dwPID1, p->dwPID2);
	marriage::CManager::Instance().WeddingStart(p->dwPID1, p->dwPID2);
}

void CInputDB::WeddingEnd(TPacketWeddingEnd* p)
{
	sys_log(0, "WeddingEnd %u %u", p->dwPID1, p->dwPID2);
	marriage::CManager::Instance().WeddingEnd(p->dwPID1, p->dwPID2);
}

// MYSHOP_PRICE_LIST
void CInputDB::MyshopPricelistRes(LPDESC d, const TPacketMyshopPricelistHeader* p )
{
	LPCHARACTER ch;

	if (!d || !(ch = d->GetCharacter()) )
		return;

	sys_log(0, "RecvMyshopPricelistRes name[%s]", ch->GetName());
	ch->UseSilkBotaryReal(p );

}
// END_OF_MYSHOP_PRICE_LIST


//RELOAD_ADMIN
void CInputDB::ReloadAdmin(const char * c_pData )
{
	GM::clear();

	int32_t size = decode_2bytes(c_pData);
	c_pData += 2;

	for (int32_t n = 0; n < size; ++n)
	{
		tAdminInfo& rAdminInfo = *(tAdminInfo*)c_pData;

		GM::insert(rAdminInfo);

		c_pData += sizeof(tAdminInfo);

		LPCHARACTER pChar = CHARACTER_MANAGER::Instance().FindPC(rAdminInfo.m_szName);
		if (pChar)
		{
			pChar->SetGMLevel();
		}
	}

	GM::init((uint32_t*)c_pData);
}
//END_RELOAD_ADMIN

////////////////////////////////////////////////////////////////////
// Analyze
// @version	05/06/10 Bang2ni - 아이템 가격정보 리스트 패킷(HEADER_DG_MYSHOP_PRICELIST_RES) 처리루틴 추가.
////////////////////////////////////////////////////////////////////
int32_t CInputDB::Analyze(LPDESC d, uint8_t bHeader, const char * c_pData)
{
	switch (bHeader)
	{
	case HEADER_DG_BOOT:
		Boot(c_pData);
		break;

	case HEADER_DG_LOGIN_SUCCESS:
		LoginSuccess(m_dwHandle, c_pData);
		break;

	case HEADER_DG_LOGIN_NOT_EXIST:
		LoginFailure(DESC_MANAGER::Instance().FindByHandle(m_dwHandle), "NOID");
		break;

	case HEADER_DG_LOGIN_WRONG_PASSWD:
		LoginFailure(DESC_MANAGER::Instance().FindByHandle(m_dwHandle), "WRONGPWD");
		break;

	case HEADER_DG_LOGIN_ALREADY:
		LoginAlready(DESC_MANAGER::Instance().FindByHandle(m_dwHandle), c_pData);
		break;

	case HEADER_DG_PLAYER_LOAD_SUCCESS:
		PlayerLoad(DESC_MANAGER::Instance().FindByHandle(m_dwHandle), c_pData);
		break;

	case HEADER_DG_PLAYER_CREATE_SUCCESS:
		PlayerCreateSuccess(DESC_MANAGER::Instance().FindByHandle(m_dwHandle), c_pData);
		break;

	case HEADER_DG_PLAYER_CREATE_FAILED:
		PlayerCreateFailure(DESC_MANAGER::Instance().FindByHandle(m_dwHandle), 0);
		break;

	case HEADER_DG_PLAYER_CREATE_ALREADY:
		PlayerCreateFailure(DESC_MANAGER::Instance().FindByHandle(m_dwHandle), 1);
		break;

	case HEADER_DG_PLAYER_DELETE_SUCCESS:
		PlayerDeleteSuccess(DESC_MANAGER::Instance().FindByHandle(m_dwHandle), c_pData);
		break;

	case HEADER_DG_PLAYER_LOAD_FAILED:
		sys_log(0, "PLAYER_LOAD_FAILED");
		break;

	case HEADER_DG_PLAYER_DELETE_FAILED:
		sys_log(0, "PLAYER_DELETE_FAILED");
		PlayerDeleteFail(DESC_MANAGER::Instance().FindByHandle(m_dwHandle));
		break;

	case HEADER_DG_ITEM_LOAD:
		ItemLoad(DESC_MANAGER::Instance().FindByHandle(m_dwHandle), c_pData);
		break;

	case HEADER_DG_QUEST_LOAD:
		QuestLoad(DESC_MANAGER::Instance().FindByHandle(m_dwHandle), c_pData);
		break;

	case HEADER_DG_AFFECT_LOAD:
		AffectLoad(DESC_MANAGER::Instance().FindByHandle(m_dwHandle), c_pData);
		break;

	case HEADER_DG_ACTIVITY_LOAD:
		ActivityLoad(DESC_MANAGER::Instance().FindByHandle(m_dwHandle), c_pData);
		break;

	case HEADER_DG_SAFEBOX_LOAD:
		SafeboxLoad(DESC_MANAGER::Instance().FindByHandle(m_dwHandle), c_pData);
		break;

	case HEADER_DG_SAFEBOX_CHANGE_SIZE:
		SafeboxChangeSize(DESC_MANAGER::Instance().FindByHandle(m_dwHandle), c_pData);
		break;

	case HEADER_DG_SAFEBOX_WRONG_PASSWORD:
		SafeboxWrongPassword(DESC_MANAGER::Instance().FindByHandle(m_dwHandle));
		break;

	case HEADER_DG_SAFEBOX_CHANGE_PASSWORD_ANSWER:
		SafeboxChangePasswordAnswer(DESC_MANAGER::Instance().FindByHandle(m_dwHandle), c_pData);
		break;

	case HEADER_DG_MALL_LOAD:
		MallLoad(DESC_MANAGER::Instance().FindByHandle(m_dwHandle), c_pData);
		break;

	case HEADER_DG_EMPIRE_SELECT:
		EmpireSelect(DESC_MANAGER::Instance().FindByHandle(m_dwHandle), c_pData);
		break;

	case HEADER_DG_MAP_LOCATIONS:
		MapLocations(c_pData);
		break;

	case HEADER_DG_P2P:
		P2P(c_pData);
		break;

	case HEADER_DG_GUILD_SKILL_UPDATE:
		GuildSkillUpdate(c_pData);
		break;

	case HEADER_DG_GUILD_LOAD:
		GuildLoad(c_pData);
		break;

	case HEADER_DG_GUILD_SKILL_RECHARGE:
		GuildSkillRecharge();
		break;

	case HEADER_DG_GUILD_EXP_UPDATE:
		GuildExpUpdate(c_pData);
		break;

	case HEADER_DG_PARTY_CREATE:
		PartyCreate(c_pData);
		break;

	case HEADER_DG_PARTY_DELETE:
		PartyDelete(c_pData);
		break;

	case HEADER_DG_PARTY_ADD:
		PartyAdd(c_pData);
		break;

	case HEADER_DG_PARTY_REMOVE:
		PartyRemove(c_pData);
		break;

	case HEADER_DG_PARTY_STATE_CHANGE:
		PartyStateChange(c_pData);
		break;

	case HEADER_DG_PARTY_SET_MEMBER_LEVEL:
		PartySetMemberLevel(c_pData);    
		break;

	case HEADER_DG_TIME:
		Time(c_pData);
		break;

	case HEADER_DG_GUILD_ADD_MEMBER:
		GuildAddMember(c_pData);
		break;

	case HEADER_DG_GUILD_REMOVE_MEMBER:
		GuildRemoveMember(c_pData);
		break;

	case HEADER_DG_GUILD_CHANGE_GRADE:
		GuildChangeGrade(c_pData);
		break;

	case HEADER_DG_GUILD_CHANGE_MEMBER_DATA:
		GuildChangeMemberData(c_pData);
		break;

	case HEADER_DG_GUILD_DISBAND:
		GuildDisband(c_pData);
		break;

	case HEADER_DG_RELOAD_PROTO:
		ReloadProto(c_pData);
		break;

	case HEADER_DG_GUILD_WAR:
		GuildWar(c_pData);
		break;

	case HEADER_DG_GUILD_WAR_SCORE:
		GuildWarScore(c_pData);
		break;

	case HEADER_DG_GUILD_LADDER:
		GuildLadder(c_pData);
		break;

	case HEADER_DG_GUILD_SKILL_USABLE_CHANGE:
		GuildSkillUsableChange(c_pData);
		break;

	case HEADER_DG_CHANGE_NAME:
		ChangeName(DESC_MANAGER::Instance().FindByHandle(m_dwHandle), c_pData);
		break;

	case HEADER_DG_AUTH_LOGIN:
		AuthLogin(DESC_MANAGER::Instance().FindByHandle(m_dwHandle), c_pData);
		break;

	case HEADER_DG_CHANGE_EMPIRE_PRIV:
		ChangeEmpirePriv(c_pData);
		break;

	case HEADER_DG_CHANGE_GUILD_PRIV:
		ChangeGuildPriv(c_pData);
		break;

	case HEADER_DG_CHANGE_CHARACTER_PRIV:
		ChangeCharacterPriv(c_pData);
		break;

	case HEADER_DG_GUILD_WITHDRAW_MONEY_GIVE:
		GuildWithdrawMoney(c_pData);
		break;

	case HEADER_DG_GUILD_MONEY_CHANGE:
		GuildMoneyChange(c_pData);
		break;

	case HEADER_DG_SET_EVENT_FLAG:
		SetEventFlag(c_pData);
		break;

	case HEADER_DG_CREATE_OBJECT:
		CreateObject(c_pData);
		break;

	case HEADER_DG_DELETE_OBJECT:
		DeleteObject(c_pData);
		break;

	case HEADER_DG_NOTICE:
		Notice(c_pData);
		break;

	case HEADER_DG_GUILD_WAR_RESERVE_ADD:
		GuildWarReserveAdd((TGuildWarReserve *) c_pData);
		break;

	case HEADER_DG_GUILD_WAR_RESERVE_DEL:
		GuildWarReserveDelete(*(uint32_t *) c_pData);
		break;

	case HEADER_DG_GUILD_WAR_BET:
		GuildWarBet((TPacketGDGuildWarBet *) c_pData);
		break;

	case HEADER_DG_MARRIAGE_ADD:
		MarriageAdd((TPacketMarriageAdd*) c_pData);
		break;

	case HEADER_DG_MARRIAGE_UPDATE:
		MarriageUpdate((TPacketMarriageUpdate*) c_pData);
		break;

	case HEADER_DG_MARRIAGE_REMOVE:
		MarriageRemove((TPacketMarriageRemove*) c_pData);
		break;

	case HEADER_DG_WEDDING_REQUEST:
		WeddingRequest((TPacketWeddingRequest*) c_pData);
		break;

	case HEADER_DG_WEDDING_READY:
		WeddingReady((TPacketWeddingReady*) c_pData);
		break;

	case HEADER_DG_WEDDING_START:
		WeddingStart((TPacketWeddingStart*) c_pData);
		break;

	case HEADER_DG_WEDDING_END:
		WeddingEnd((TPacketWeddingEnd*) c_pData);
		break;

		// MYSHOP_PRICE_LIST
	case HEADER_DG_MYSHOP_PRICELIST_RES:
		MyshopPricelistRes(DESC_MANAGER::Instance().FindByHandle(m_dwHandle), (TPacketMyshopPricelistHeader*) c_pData );
		break;
		// END_OF_MYSHOP_PRICE_LIST
		//
	// RELOAD_ADMIN
	case HEADER_DG_RELOAD_ADMIN:
		ReloadAdmin(c_pData );		
		break;
	//END_RELOAD_ADMIN

	case HEADER_DG_ACK_CHANGE_GUILD_MASTER :
		this->GuildChangeMaster((TPacketChangeGuildMaster*) c_pData);
		break;	

	case HEADER_DG_ACK_SPARE_ITEM_ID_RANGE :
		ITEM_MANAGER::Instance().SetMaxSpareItemID(*((TItemIDRangeTable*)c_pData) );
		break;

	case HEADER_DG_NEED_LOGIN_LOG:
		DetailLog( (TPacketNeedLoginLogInfo*) c_pData );
		break;
	// 독일 선물 기능 테스트
	case HEADER_DG_ITEMAWARD_INFORMER:
		ItemAwardInformer((TPacketItemAwardInfromer*) c_pData);
		break;
	case HEADER_DG_RESPOND_CHANNELSTATUS:
		RespondChannelStatus(DESC_MANAGER::Instance().FindByHandle(m_dwHandle), c_pData);
		break;
	default:
		return (-1);
	}

	return 0;
}

bool CInputDB::Process(LPDESC d, const void * orig, int32_t bytes, int32_t & r_iBytesProceed)
{
	const char *	c_pData = (const char *) orig;
	uint8_t		bHeader, bLastHeader = 0;
	int32_t			iSize;
	int32_t			iLastPacketLen = 0;

	for (m_iBufferLeft = bytes; m_iBufferLeft > 0;)
	{
		if (m_iBufferLeft < 9)
			return true;

		bHeader		= *((uint8_t *) (c_pData));	// 1
		m_dwHandle	= *((uint32_t *) (c_pData + 1));	// 4
		iSize		= *((uint32_t *) (c_pData + 5));	// 4

		sys_log(1, "DBCLIENT: header %d handle %d size %d bytes %d", bHeader, m_dwHandle, iSize, bytes); 

		if (m_iBufferLeft - 9 < iSize)
			return true;

		const char * pRealData = (c_pData + 9);

		if (Analyze(d, bHeader, pRealData) < 0)
		{
			sys_err("in InputDB: UNKNOWN HEADER: %d, LAST HEADER: %d(%d), REMAIN BYTES: %d, DESC: %d",
					bHeader, bLastHeader, iLastPacketLen, m_iBufferLeft, d->GetSocket());

			//printdata((uint8_t*) orig, bytes);
			//d->SetPhase(PHASE_CLOSE);
		}

		c_pData		+= 9 + iSize;
		m_iBufferLeft	-= 9 + iSize;
		r_iBytesProceed	+= 9 + iSize;

		iLastPacketLen	= 9 + iSize;
		bLastHeader	= bHeader;
	}

	return true;
}

void CInputDB::GuildChangeMaster(TPacketChangeGuildMaster* p)
{
	CGuildManager::Instance().ChangeMaster(p->dwGuildID);
}

void CInputDB::DetailLog(const TPacketNeedLoginLogInfo* info)
{
	LPCHARACTER pChar = CHARACTER_MANAGER::Instance().FindByPID( info->dwPlayerID );

	if (nullptr != pChar)
	{
		LogManager::Instance().DetailLoginLog(true, pChar);
	}
}

void CInputDB::ItemAwardInformer(TPacketItemAwardInfromer *data)
{	
	LPDESC d = DESC_MANAGER::Instance().FindByLoginName(data->login);	//login정보
	
	if(d == nullptr)
		return;
	else
	{
		if (d->GetCharacter())
		{
			LPCHARACTER ch = d->GetCharacter();	
			ch->SetItemAward_vnum(data->vnum);	// ch 에 임시 저장해놨다가 QuestLoad 함수에서 처리
			ch->SetItemAward_cmd(data->command);		

			if(d->IsPhase(PHASE_GAME))			//게임페이즈일때
			{
				quest::CQuestManager::Instance().ItemInformer(ch->GetPlayerID(),ch->GetItemAward_vnum());	//questmanager 호출
			}
		}
	}
}

void CInputDB::RespondChannelStatus(LPDESC desc, const char* pcData) 
{
	if (!desc) {
		return;
	}
	const int32_t nSize = decode_4bytes(pcData);
	pcData += sizeof(nSize);

	uint8_t bHeader = HEADER_GC_RESPOND_CHANNELSTATUS;
	desc->BufferedPacket(&bHeader, sizeof(uint8_t));
	desc->BufferedPacket(&nSize, sizeof(nSize));
	if (0 < nSize) {
		desc->BufferedPacket(pcData, sizeof(TChannelStatus)*nSize);
	}
	uint8_t bSuccess = 1;
	desc->Packet(&bSuccess, sizeof(bSuccess));
	desc->SetChannelStatusRequested(false);
}