#include "StdAfx.h"
#include "PythonNetworkStream.h"
#include "PythonApplication.h"
#include "Packet.h"
#include "NetworkActorManager.h"
#include "GuildMarkDownloader.h"
#include "GuildMarkUploader.h"
#include "MarkManager.h"

#include "../eterLib/NetPacketHeaderMap.h"
#include "../eterSecurity/CheatQueueManager.h"

// Packet ---------------------------------------------------------------------------
class CMainPacketHeaderMap : public CNetworkPacketHeaderMap
{
	public:
		enum
		{
			STATIC_SIZE_PACKET = false,
			DYNAMIC_SIZE_PACKET = true
		};

	public:
		CMainPacketHeaderMap()
		{
//			packetsdeki gibi düzenle
			Set(HEADER_GC_EMPIRE, TPacketType(sizeof(SPacketGCEmpire), STATIC_SIZE_PACKET));
			Set(HEADER_GC_WARP, TPacketType(sizeof(SPacketGCWarp), STATIC_SIZE_PACKET));
			Set(HEADER_GC_QUEST_INFO, TPacketType(sizeof(SPacketGCQuestInfo), DYNAMIC_SIZE_PACKET));
			Set(HEADER_GC_REQUEST_MAKE_GUILD, TPacketType(sizeof(SPacketGCRequestGuildMake), STATIC_SIZE_PACKET));
			Set(HEADER_GC_PVP, TPacketType(sizeof(SPacketGCPVP), STATIC_SIZE_PACKET));
			Set(HEADER_GC_DUEL_START, TPacketType(sizeof(SPacketGCDuelStart), DYNAMIC_SIZE_PACKET));
			Set(HEADER_GC_CHARACTER_ADD, TPacketType(sizeof(SPacketGCCharacterAdd), STATIC_SIZE_PACKET));
			Set(HEADER_GC_CHAR_ADDITIONAL_INFO, TPacketType(sizeof(SPacketGCCharacterAdditionalInfo), STATIC_SIZE_PACKET));
			Set(HEADER_GC_CHARACTER_UPDATE, TPacketType(sizeof(SPacketGCCharacterUpdate), STATIC_SIZE_PACKET));
			Set(HEADER_GC_CHARACTER_DEL, TPacketType(sizeof(SPacketGCCharacterDelete), STATIC_SIZE_PACKET));
			Set(HEADER_GC_CHARACTER_MOVE, TPacketType(sizeof(SPacketGCMove), STATIC_SIZE_PACKET));
			Set(HEADER_GC_CHAT, TPacketType(sizeof(SPacketGCChat), DYNAMIC_SIZE_PACKET));
			Set(HEADER_GC_SYNC_POSITION, TPacketType(sizeof(SPacketGCSyncPosition), DYNAMIC_SIZE_PACKET));

			Set(HEADER_GC_LOGIN_SUCCESS, TPacketType(sizeof(SPacketGCLoginSuccess), STATIC_SIZE_PACKET));
			Set(HEADER_GC_LOGIN_FAILURE, TPacketType(sizeof(SPacketGCLoginFailure), STATIC_SIZE_PACKET));

			Set(HEADER_GC_PLAYER_CREATE_SUCCESS, TPacketType(sizeof(SPacketGCPlayerCreateSuccess), STATIC_SIZE_PACKET));
			Set(HEADER_GC_PLAYER_CREATE_FAILURE, TPacketType(sizeof(SPacketGCCreateFailure), STATIC_SIZE_PACKET));
			Set(HEADER_GC_PLAYER_DELETE_SUCCESS, TPacketType(sizeof(SPacketGCPlayerDeleteSuccess), STATIC_SIZE_PACKET));
			Set(HEADER_GC_PLAYER_DELETE_WRONG_SOCIAL_ID, TPacketType(sizeof(SPacketGCPlayerDeleteWrongSocialID), STATIC_SIZE_PACKET));

			Set(HEADER_GC_STUN, TPacketType(sizeof(SPacketGCStun), STATIC_SIZE_PACKET));
			Set(HEADER_GC_DEAD, TPacketType(sizeof(SPacketGCDead), STATIC_SIZE_PACKET));

			Set(HEADER_GC_MAIN_CHARACTER, TPacketType(sizeof(SPacketGCMainCharacter), STATIC_SIZE_PACKET));
			Set(HEADER_GC_MAIN_CHARACTER3_BGM, TPacketType(sizeof(SPacketGCMainCharacter3_BGM), STATIC_SIZE_PACKET));
			Set(HEADER_GC_MAIN_CHARACTER4_BGM_VOL, TPacketType(sizeof(SPacketGCMainCharacter4_BGM_VOL), STATIC_SIZE_PACKET));

			Set(HEADER_GC_PLAYER_POINTS, TPacketType(sizeof(SPacketGCPoints), STATIC_SIZE_PACKET));
			Set(HEADER_GC_PLAYER_POINT_CHANGE, TPacketType(sizeof(SPacketGCPointChange), STATIC_SIZE_PACKET));

			Set(HEADER_GC_ITEM_SET, TPacketType(sizeof(SPacketGCItemSet), STATIC_SIZE_PACKET));
			Set(HEADER_GC_ITEM_SET2, TPacketType(sizeof(SPacketGCItemSet2), STATIC_SIZE_PACKET));

			Set(HEADER_GC_ITEM_USE, TPacketType(sizeof(SPacketGCItemUse), STATIC_SIZE_PACKET));
			Set(HEADER_GC_ITEM_UPDATE, TPacketType(sizeof(SPacketGCItemUpdate), STATIC_SIZE_PACKET));

			Set(HEADER_GC_ITEM_GROUND_ADD, TPacketType(sizeof(SPacketGCItemGroundAdd), STATIC_SIZE_PACKET));
			Set(HEADER_GC_ITEM_GROUND_DEL, TPacketType(sizeof(SPacketGCItemGroundDel), STATIC_SIZE_PACKET));
			Set(HEADER_GC_ITEM_OWNERSHIP, TPacketType(sizeof(SPacketGCItemOwnership), STATIC_SIZE_PACKET));

			Set(HEADER_GC_QUICKSLOT_ADD, TPacketType(sizeof(SPacketGCQuickSlotAdd), STATIC_SIZE_PACKET));
			Set(HEADER_GC_QUICKSLOT_DEL, TPacketType(sizeof(SPacketGCQuickSlotDel), STATIC_SIZE_PACKET));
			Set(HEADER_GC_QUICKSLOT_SWAP, TPacketType(sizeof(SPacketGCQuickSlotSwap), STATIC_SIZE_PACKET));

			Set(HEADER_GC_WHISPER, TPacketType(sizeof(SPacketGCWhisper), STATIC_SIZE_PACKET));

			Set(HEADER_GC_CHARACTER_POSITION, TPacketType(sizeof(SPacketGCPosition), STATIC_SIZE_PACKET));
			Set(HEADER_GC_MOTION, TPacketType(sizeof(SPacketGCMotion), STATIC_SIZE_PACKET));

			Set(HEADER_GC_SHOP, TPacketType(sizeof(SPacketGCShop), DYNAMIC_SIZE_PACKET));
			Set(HEADER_GC_SHOP_SIGN, TPacketType(sizeof(SPacketGCShopSign), STATIC_SIZE_PACKET));
			Set(HEADER_GC_EXCHANGE, TPacketType(sizeof(SPacketGCExchange), STATIC_SIZE_PACKET));

			Set(HEADER_GC_PING, TPacketType(sizeof(SPacketGCPing), STATIC_SIZE_PACKET));

			Set(HEADER_GC_SCRIPT, TPacketType(sizeof(SPacketGCScript), DYNAMIC_SIZE_PACKET));
			Set(HEADER_GC_QUEST_CONFIRM, TPacketType(sizeof(SPacketGCQuestConfirm), STATIC_SIZE_PACKET));

			Set(HEADER_GC_TARGET, TPacketType(sizeof(SPacketGCTarget), STATIC_SIZE_PACKET));
			Set(HEADER_GC_TARGET_DROP, CNetworkPacketHeaderMap::TPacketType(sizeof(SPacketGCTargetDrop), STATIC_SIZE_PACKET));
			Set(HEADER_GC_CHEST_DROP_INFO, CNetworkPacketHeaderMap::TPacketType(sizeof(SPacketGCChestDropInfo), DYNAMIC_SIZE_PACKET));

			Set(HEADER_GC_CHANGE_SPEED, TPacketType(sizeof(SPacketGCChangeSpeed), STATIC_SIZE_PACKET));

			Set(HEADER_GC_HANDSHAKE, TPacketType(sizeof(SPacketHandshake), STATIC_SIZE_PACKET));
			Set(HEADER_GC_HANDSHAKE_OK, TPacketType(sizeof(SPacketGCHandshakeOK), STATIC_SIZE_PACKET));
			Set(HEADER_GC_OWNERSHIP, TPacketType(sizeof(SPacketGCOwnership), STATIC_SIZE_PACKET));
			Set(HEADER_GC_CREATE_FLY, TPacketType(sizeof(SPacketGCCreateFly), STATIC_SIZE_PACKET));
#ifdef _IMPROVED_PACKET_ENCRYPTION_
			Set(HEADER_GC_KEY_AGREEMENT, TPacketType(sizeof(SPacketKeyAgreement), STATIC_SIZE_PACKET));
			Set(HEADER_GC_KEY_AGREEMENT_COMPLETED, TPacketType(sizeof(SPacketGCKeyAgreementCompleted), STATIC_SIZE_PACKET));
#endif
			Set(HEADER_GC_ADD_FLY_TARGETING, TPacketType(sizeof(SPacketGCFlyTargeting), STATIC_SIZE_PACKET));
			Set(HEADER_GC_FLY_TARGETING, TPacketType(sizeof(SPacketGCFlyTargeting), STATIC_SIZE_PACKET));

			Set(HEADER_GC_PHASE, TPacketType(sizeof(SPacketGCPhase), STATIC_SIZE_PACKET));
			Set(HEADER_GC_SKILL_LEVEL, TPacketType(sizeof(SPacketGCSkillLevel), STATIC_SIZE_PACKET));

			Set(HEADER_GC_MESSENGER, TPacketType(sizeof(SPacketGCMessenger), DYNAMIC_SIZE_PACKET));
			Set(HEADER_GC_GUILD, TPacketType(sizeof(SPacketGCGuild), DYNAMIC_SIZE_PACKET));

			Set(HEADER_GC_PARTY_INVITE, TPacketType(sizeof(SPacketGCPartyInvite), STATIC_SIZE_PACKET));
			Set(HEADER_GC_PARTY_ADD, TPacketType(sizeof(SPacketGCPartyAdd), STATIC_SIZE_PACKET));
			Set(HEADER_GC_PARTY_UPDATE, TPacketType(sizeof(SPacketGCPartyUpdate), STATIC_SIZE_PACKET));
			Set(HEADER_GC_PARTY_REMOVE, TPacketType(sizeof(SPacketGCPartyRemove), STATIC_SIZE_PACKET));
			Set(HEADER_GC_PARTY_LINK, TPacketType(sizeof(SPacketGCPartyLink), STATIC_SIZE_PACKET));
			Set(HEADER_GC_PARTY_UNLINK, TPacketType(sizeof(SPacketGCPartyUnlink), STATIC_SIZE_PACKET));
			Set(HEADER_GC_PARTY_PARAMETER, TPacketType(sizeof(SPacketGCPartyParameter), STATIC_SIZE_PACKET));

			Set(HEADER_GC_SAFEBOX_SET, TPacketType(sizeof(SPacketGCSafeboxSet), STATIC_SIZE_PACKET));
			Set(HEADER_GC_SAFEBOX_DEL, TPacketType(sizeof(SPacketGCSafeboxDel), STATIC_SIZE_PACKET));
			Set(HEADER_GC_SAFEBOX_WRONG_PASSWORD, TPacketType(sizeof(SPacketGCSafeboxWrongPassword), STATIC_SIZE_PACKET));
			Set(HEADER_GC_SAFEBOX_SIZE, TPacketType(sizeof(SPacketGCSafeboxSize), STATIC_SIZE_PACKET));
			Set(HEADER_GC_FISHING, TPacketType(sizeof(SPacketGCFishing), STATIC_SIZE_PACKET));
			Set(HEADER_GC_DUNGEON, TPacketType(sizeof(SPacketGCDungeon), DYNAMIC_SIZE_PACKET));
			Set(HEADER_GC_TIME, TPacketType(sizeof(SPacketGCTime), STATIC_SIZE_PACKET));
			Set(HEADER_GC_WALK_MODE, TPacketType(sizeof(SPacketGCWalkMode), STATIC_SIZE_PACKET));
			Set(HEADER_GC_CHANGE_SKILL_GROUP, TPacketType(sizeof(SPacketGCChangeSkillGroup), STATIC_SIZE_PACKET));
			Set(HEADER_GC_REFINE_INFORMATION, TPacketType(sizeof(SPacketGCRefineInformation), STATIC_SIZE_PACKET));
			Set(HEADER_GC_SPECIAL_EFFECT, TPacketType(sizeof(SPacketGCSpecialEffect), STATIC_SIZE_PACKET));
			Set(HEADER_GC_NPC_POSITION, TPacketType(sizeof(SPacketGCNPCPosition), DYNAMIC_SIZE_PACKET));
			Set(HEADER_GC_CHANGE_NAME, TPacketType(sizeof(SPacketGCChangeName), STATIC_SIZE_PACKET));

			Set(HEADER_GC_LOGIN_KEY, TPacketType(sizeof(SPacketGCLoginKey), STATIC_SIZE_PACKET));

			Set(HEADER_GC_AUTH_SUCCESS, TPacketType(sizeof(SPacketGCAuthSuccess), STATIC_SIZE_PACKET));
			Set(HEADER_GC_CHANNEL, TPacketType(sizeof(SPacketGCChannel), STATIC_SIZE_PACKET));
			Set(HEADER_GC_VIEW_EQUIP, TPacketType(sizeof(SPacketGCViewEquip), STATIC_SIZE_PACKET));
			Set(HEADER_GC_LAND_LIST, TPacketType(sizeof(SPacketGCLandList), DYNAMIC_SIZE_PACKET));

			Set(HEADER_GC_TARGET_UPDATE, TPacketType(sizeof(SPacketGCTargetUpdate), STATIC_SIZE_PACKET));
			Set(HEADER_GC_TARGET_DELETE, TPacketType(sizeof(SPacketGCTargetDelete), STATIC_SIZE_PACKET));
			Set(HEADER_GC_TARGET_CREATE, TPacketType(sizeof(SPacketGCTargetCreate), STATIC_SIZE_PACKET));

			Set(HEADER_GC_AFFECT_ADD, TPacketType(sizeof(SPacketGCAffectAdd), STATIC_SIZE_PACKET));
			Set(HEADER_GC_AFFECT_REMOVE, TPacketType(sizeof(SPacketGCAffectRemove), STATIC_SIZE_PACKET));

			Set(HEADER_GC_MALL_OPEN, TPacketType(sizeof(SPacketGCMallOpen), STATIC_SIZE_PACKET));
			Set(HEADER_GC_MALL_SET, TPacketType(sizeof(SPacketGCMallSet), STATIC_SIZE_PACKET));
			Set(HEADER_GC_MALL_DEL, TPacketType(sizeof(SPacketGCMallDel), STATIC_SIZE_PACKET));

			Set(HEADER_GC_LOVER_INFO, TPacketType(sizeof(SPacketGCLoverInfo), STATIC_SIZE_PACKET));
			Set(HEADER_GC_LOVE_POINT_UPDATE, TPacketType(sizeof(SPacketGCLovePointUpdate), STATIC_SIZE_PACKET));

			Set(HEADER_GC_DIG_MOTION, TPacketType(sizeof(SPacketGCDigMotion), STATIC_SIZE_PACKET));
			Set(HEADER_GC_DAMAGE_INFO, TPacketType(sizeof(SPacketGCDamageInfo), STATIC_SIZE_PACKET));

			Set(HEADER_GC_SPECIFIC_EFFECT, TPacketType(sizeof(SPacketGCSpecificEffect), STATIC_SIZE_PACKET));
			Set(HEADER_GC_DRAGON_SOUL_REFINE, TPacketType(sizeof(SPacketGCDragonSoulRefine), STATIC_SIZE_PACKET));
		
			Set(HEADER_GC_CHEAT_BLACKLIST, TPacketType(sizeof(SPacketGCCheatBlacklist), DYNAMIC_SIZE_PACKET));

#ifdef ENABLE_ACCE_SYSTEM
			Set(HEADER_GC_ACCE, TPacketType(sizeof(SPacketAcce), STATIC_SIZE_PACKET));
#endif
		}
};

int32_t g_iLastPacket[2] = { 0, 0 };

void CPythonNetworkStream::ExitApplication()
{
	if (__IsNotPing())
		AbsoluteExitApplication();
	else
		SendChatPacket("/quit");
}

void CPythonNetworkStream::ExitGame()
{
	if (__IsNotPing())
		LogOutGame();
	else
		SendChatPacket("/phase_select");
}


void CPythonNetworkStream::LogOutGame()
{
	if (__IsNotPing())
		AbsoluteExitGame();
	else
		SendChatPacket("/logout");
}

void CPythonNetworkStream::AbsoluteExitGame()
{
	if (!IsOnline())
		return;

	OnRemoteDisconnect();		
	Disconnect();
}

void CPythonNetworkStream::AbsoluteExitApplication()
{
	PostQuitMessage(0);
}

bool CPythonNetworkStream::__IsNotPing()
{
	// ¿ø·¡´Â ÇÎÀÌ ¾È¿Ã¶§ Ã¼Å©ÀÌ³ª ¼­¹ö¶û Á¤È®È÷ ¸ÂÃß¾î¾ß ÇÑ´Ù.
	return false;
}

uint32_t CPythonNetworkStream::GetGuildID()
{
	return m_dwGuildID;
}

uint32_t CPythonNetworkStream::UploadMark(const char * c_szImageFileName)
{
	// MARK_BUG_FIX
	// ±æµå¸¦ ¸¸µç Á÷ÈÄ´Â ±æµå ¾ÆÀÌµð°¡ 0ÀÌ´Ù.
	if (0 == m_dwGuildID)
		return ERROR_MARK_UPLOAD_NEED_RECONNECT;

	m_nextDownloadMarkTime = 0;
	// END_OF_MARK_BUG_FIX

	uint32_t uError=ERROR_UNKNOWN;
	CGuildMarkUploader& rkGuildMarkUploader=CGuildMarkUploader::Instance();
	if (!rkGuildMarkUploader.Connect(m_kMarkAuth.m_kNetAddr, m_kMarkAuth.m_dwHandle, m_kMarkAuth.m_dwRandomKey, m_dwGuildID, c_szImageFileName, &uError))
	{
		switch (uError)
		{
		case CGuildMarkUploader::ERROR_CONNECT:
			return ERROR_CONNECT_MARK_SERVER;
		case CGuildMarkUploader::ERROR_LOAD:
			return ERROR_LOAD_MARK;
		case CGuildMarkUploader::ERROR_WIDTH:
			return ERROR_MARK_WIDTH;
		case CGuildMarkUploader::ERROR_HEIGHT:
			return ERROR_MARK_HEIGHT;
			default:
				return ERROR_UNKNOWN;
		}
	}

	// MARK_BUG_FIX	
	__DownloadMark(true);
	// END_OF_MARK_BUG_FIX
	
	if (CGuildMarkManager::INVALID_MARK_ID == CGuildMarkManager::Instance().GetMarkID(m_dwGuildID))
		return ERROR_MARK_CHECK_NEED_RECONNECT;

	return ERROR_NONE;
}

uint32_t CPythonNetworkStream::UploadSymbol(const char* c_szImageFileName)
{
	uint32_t uError=ERROR_UNKNOWN;
	CGuildMarkUploader& rkGuildMarkUploader=CGuildMarkUploader::Instance();
	if (!rkGuildMarkUploader.ConnectToSendSymbol(m_kMarkAuth.m_kNetAddr, m_kMarkAuth.m_dwHandle, m_kMarkAuth.m_dwRandomKey, m_dwGuildID, c_szImageFileName, &uError))
	{
		switch (uError)
		{
		case CGuildMarkUploader::ERROR_CONNECT:
			return ERROR_CONNECT_MARK_SERVER;
		case CGuildMarkUploader::ERROR_LOAD:
			return ERROR_LOAD_MARK;
		case CGuildMarkUploader::ERROR_WIDTH:
			return ERROR_MARK_WIDTH;
		case CGuildMarkUploader::ERROR_HEIGHT:
			return ERROR_MARK_HEIGHT;
			default:
				return ERROR_UNKNOWN;
		}
	}
	std::set<uint32_t> guildIDSet;
	guildIDSet.insert(m_dwGuildID);

	__DownloadSymbol(guildIDSet);
	return ERROR_NONE;
}

void CPythonNetworkStream::__DownloadMark(bool forceDownload)
{
	// 3ºÐ ¾È¿¡´Â ´Ù½Ã Á¢¼ÓÇÏÁö ¾Ê´Â´Ù.
	uint32_t curTime = ELTimer_GetMSec();

	if (!forceDownload && (curTime < m_nextDownloadMarkTime))
		return;

	m_nextDownloadMarkTime = curTime + 60000 * 3; // 3ºÐ

	CGuildMarkDownloader& rkGuildMarkDownloader = CGuildMarkDownloader::Instance();
	rkGuildMarkDownloader.Connect(m_kMarkAuth.m_kNetAddr, m_kMarkAuth.m_dwHandle, m_kMarkAuth.m_dwRandomKey);
}

void CPythonNetworkStream::__DownloadSymbol(const std::set<uint32_t> & c_rkSet_dwGuildID)
{
	CGuildMarkDownloader& rkGuildMarkDownloader=CGuildMarkDownloader::Instance();
	rkGuildMarkDownloader.ConnectToRecvSymbol(m_kMarkAuth.m_kNetAddr, m_kMarkAuth.m_dwHandle, m_kMarkAuth.m_dwRandomKey, c_rkSet_dwGuildID);
}

void CPythonNetworkStream::SetPhaseWindow(uint32_t ePhaseWnd, PyObject* poPhaseWnd)
{
	if (ePhaseWnd>=PHASE_WINDOW_NUM)
		return;

	m_apoPhaseWnd[ePhaseWnd]=poPhaseWnd;
}

void CPythonNetworkStream::ClearPhaseWindow(uint32_t ePhaseWnd, PyObject* poPhaseWnd)
{
	if (ePhaseWnd>=PHASE_WINDOW_NUM)
		return;

	if (poPhaseWnd != m_apoPhaseWnd[ePhaseWnd])
		return;

	m_apoPhaseWnd[ePhaseWnd] = nullptr;
}

void CPythonNetworkStream::SetServerCommandParserWindow(PyObject* poWnd)
{
	m_poSerCommandParserWnd = poWnd;
}

bool CPythonNetworkStream::IsSelectedEmpire()
{
	if (m_dwEmpireID)
		return true;
	
	return false;
}

uint32_t CPythonNetworkStream::GetAccountCharacterSlotDatau(uint32_t iSlot, uint32_t eType)
{
	if (iSlot >= PLAYER_PER_ACCOUNT)
		return 0;
		
	TSimplePlayerInformation&	rkSimplePlayerInfo=m_akSimplePlayerInfo[iSlot];
	
	switch (eType)
	{
		case ACCOUNT_CHARACTER_SLOT_ID:
			return rkSimplePlayerInfo.dwID;
		case ACCOUNT_CHARACTER_SLOT_RACE:
			return rkSimplePlayerInfo.byJob;
		case ACCOUNT_CHARACTER_SLOT_LEVEL:
			return rkSimplePlayerInfo.byLevel;
		case ACCOUNT_CHARACTER_SLOT_STR:
			return rkSimplePlayerInfo.byST;
		case ACCOUNT_CHARACTER_SLOT_DEX:
			return rkSimplePlayerInfo.byDX;
		case ACCOUNT_CHARACTER_SLOT_HTH:
			return rkSimplePlayerInfo.byHT;
		case ACCOUNT_CHARACTER_SLOT_INT:			
			return rkSimplePlayerInfo.byIQ;
		case ACCOUNT_CHARACTER_SLOT_PLAYTIME:
			return rkSimplePlayerInfo.dwPlayMinutes;
		case ACCOUNT_CHARACTER_SLOT_FORM:
//			return rkSimplePlayerInfo.wParts[CRaceData::PART_MAIN];
			return rkSimplePlayerInfo.wMainPart;
		case ACCOUNT_CHARACTER_SLOT_PORT:
			return rkSimplePlayerInfo.wPort;
		case ACCOUNT_CHARACTER_SLOT_GUILD_ID:
			return m_adwGuildID[iSlot];
		case ACCOUNT_CHARACTER_SLOT_CHANGE_NAME_FLAG:
			return rkSimplePlayerInfo.bChangeName;
		case ACCOUNT_CHARACTER_SLOT_HAIR:
			return rkSimplePlayerInfo.wHairPart;

#ifdef ENABLE_ACCE_SYSTEM
		case ACCOUNT_CHARACTER_SLOT_ACCE:
			return rkSimplePlayerInfo.dwAccePart;
#endif
	}
	return 0;
}

const char* CPythonNetworkStream::GetAccountCharacterSlotDataz(uint32_t iSlot, uint32_t eType)
{
	static const char* sc_szEmpty="";

	if (iSlot >= PLAYER_PER_ACCOUNT)
		return sc_szEmpty;
		
	TSimplePlayerInformation&	rkSimplePlayerInfo=m_akSimplePlayerInfo[iSlot];
	
	switch (eType)
	{
		case ACCOUNT_CHARACTER_SLOT_ADDR:
		{				
		uint8_t ip[4];

		const int32_t LEN = 4;
		for (int32_t i = 0; i < LEN; i++)
		{
			ip[i] = uint8_t(rkSimplePlayerInfo.lAddr&0xff);
			rkSimplePlayerInfo.lAddr>>=8;
		}

		static char s_szAddr[256];
		sprintf_s(s_szAddr, "%d.%d.%d.%d", ip[0], ip[1], ip[2], ip[3]);
		return s_szAddr;
	}
	case ACCOUNT_CHARACTER_SLOT_NAME:
		return rkSimplePlayerInfo.szName;
	case ACCOUNT_CHARACTER_SLOT_GUILD_NAME:
		return m_astrGuildName[iSlot].c_str();
	}
	return sc_szEmpty;
}

void CPythonNetworkStream::ConnectLoginServer(const char* c_szAddr, uint32_t uPort)
{
	Connect(c_szAddr, uPort);
}

void CPythonNetworkStream::SetMarkServer(const char* c_szAddr, uint32_t uPort)
{
	m_kMarkAuth.m_kNetAddr.Set(c_szAddr, uPort);
}

void CPythonNetworkStream::ConnectGameServer(uint32_t iChrSlot)
{
	if (iChrSlot >= PLAYER_PER_ACCOUNT)
		return;

	m_dwSelectedCharacterIndex = iChrSlot;

	__DirectEnterMode_Set(iChrSlot);

	TSimplePlayerInformation&	rkSimplePlayerInfo=m_akSimplePlayerInfo[iChrSlot];	
	Connect(static_cast<uint32_t>(rkSimplePlayerInfo.lAddr), rkSimplePlayerInfo.wPort);
}

void CPythonNetworkStream::SetLoginInfo(const char* c_szID, const char* c_szPassword)
{
	m_stID=c_szID;
	m_stPassword=c_szPassword;
}

void CPythonNetworkStream::ClearLoginInfo()
{
	m_stPassword.clear();
}

void CPythonNetworkStream::SetLoginKey(uint32_t dwLoginKey)
{
	m_dwLoginKey = dwLoginKey;
}


bool CPythonNetworkStream::CheckPacket(TPacketHeader* pRetHeader)
{
	*pRetHeader = 0;

	static CMainPacketHeaderMap s_packetHeaderMap;

	TPacketHeader header;

	size_t packetHeaderSize = sizeof(TPacketHeader);

	if (!Peek(packetHeaderSize, &header))
		return false;

	if (0 == header)
	{
		if (!Recv(packetHeaderSize, &header))
			return false;

		while (Peek(packetHeaderSize, &header))
		{
			if (0 == header)
			{
				if (!Recv(packetHeaderSize, &header))
					return false;
			}
			else
			{
				break;
			}
		}

		if (0 == header)
			return false;
	}

	CNetworkPacketHeaderMap::TPacketType PacketType;

	if (!s_packetHeaderMap.Get(header, &PacketType))
	{
		TraceError("Unknown packet header: %d, last: %d %d recv: %d", header, g_iLastPacket[0], g_iLastPacket[1], GetRecvBufferSize());
		ClearRecvBuffer();

		PostQuitMessage(0);
		return false;
	}

	// Code for dynamic size packet
	if (PacketType.isDynamicSizePacket)
	{
		SDynamicSizePacketHeader DynamicSizePacketHeader;

		if (!Peek(sizeof(DynamicSizePacketHeader), &DynamicSizePacketHeader))
			return false;

		if (!Peek(DynamicSizePacketHeader.size))
		{
			Tracef("CPythonNetworkStream::CheckPacket - Not enough dynamic packet size: header %d packet size: %d\n",
				DynamicSizePacketHeader.header,
				DynamicSizePacketHeader.size);
			return false;
		}
	}
	else
	{
		if (!Peek(PacketType.iPacketSize))
		{
			Tracef("Not enough packet size: header %d packet size: %d, recv buffer size: %d",
				header,
				PacketType.iPacketSize,
				GetRecvBufferSize());
			return false;
		}
	}

	if (!header)
		return false;

	*pRetHeader = header;

	g_iLastPacket[0] = g_iLastPacket[1];
	g_iLastPacket[1] = header;
	//Tracenf("header %d size %d", header, PacketType.iPacketSize);
	//Tracenf("header %d size %d outputpos[%d] security %u", header, PacketType.iPacketSize, m_recvBufOutputPos, IsSecurityMode());
	return true;
}

bool CPythonNetworkStream::RecvErrorPacket(int32_t header)
{
	TraceError("Phase %s does not handle this header (header: %d, last: %d, %d)",
		m_strPhase.c_str(), header, g_iLastPacket[0], g_iLastPacket[1]);

	ClearRecvBuffer();
	return true;
}

bool CPythonNetworkStream::RecvPhasePacket()
{
	SPacketGCPhase packet_phase;

	if (!Recv(sizeof(SPacketGCPhase), &packet_phase))
		return false;

	switch (packet_phase.phase)
	{
		case PHASE_CLOSE:				// ²÷±â´Â »óÅÂ (¶Ç´Â ²÷±â Àü »óÅÂ)
			ClosePhase();
			m_isChatEnable = FALSE;
			break;

		case PHASE_HANDSHAKE:			// ¾Ç¼ö..;;
			SetHandShakePhase();
			m_isChatEnable = FALSE;
			break;

		case PHASE_LOGIN:				// ·Î±×ÀÎ Áß
			SetLoginPhase();
			m_isChatEnable = FALSE;
			break;

		case PHASE_SELECT:				// Ä³¸¯ÅÍ ¼±ÅÃ È­¸é
			SetSelectPhase();
			m_isChatEnable = FALSE;
	
			// MARK_BUG_FIX
			__DownloadMark();
			// END_OF_MARK_BUG_FIX
			break;

		case PHASE_LOADING:				// ¼±ÅÃ ÈÄ ·Îµù È­¸é
			SetLoadingPhase();
			m_isChatEnable = FALSE;
			break;

		case PHASE_GAME:				// °ÔÀÓ È­¸é
			SetGamePhase();
			m_isChatEnable = TRUE;
			break;

		case PHASE_DEAD:
			break;
	}

	CPythonApplication::Instance().SetGameStage(packet_phase.stage);
	return true;
}

bool CPythonNetworkStream::RecvPingPacket()
{
	Tracef("recv ping packet. (securitymode %u)\n", IsSecurityMode());

	SPacketGCPing kPacketPing;

	if (!Recv(sizeof(SPacketGCPing), &kPacketPing))
		return false;

	m_dwLastGamePingTime = ELTimer_GetMSec();

	SPacketCGPong kPacketPong;
	if (!Send(sizeof(SPacketCGPong), &kPacketPong))
		return false;

	return true;
}

bool CPythonNetworkStream::RecvDefaultPacket(int32_t header)
{
	if (!header)
		return true;

	TraceError("Unprocessed packet header %d, state %s\n", header, m_strPhase.c_str());
	ClearRecvBuffer();
	return true;
}

bool CPythonNetworkStream::OnProcess()
{
	if (m_isStartGame)
	{
		m_isStartGame = FALSE;

		PyCallClassMemberFunc(m_poHandler, "SetGamePhase", Py_BuildValue("()"));
//		PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "StartGame", Py_BuildValue("()"));
	}
	
	m_rokNetActorMgr->Update();

	if (m_phaseProcessFunc.IsEmpty())
		return true;

	//SPacketHeader header;
	//while(CheckPacket(&header))
	{
		m_phaseProcessFunc.Run();
	}

	return true;
}


// Set
void CPythonNetworkStream::SetOffLinePhase()
{
	if ("OffLine" != m_strPhase)
		m_phaseLeaveFunc.Run();

	m_strPhase = "OffLine";

	Tracen("");
	Tracen("## Network - OffLine Phase ##");	
	Tracen("");

	m_dwChangingPhaseTime = ELTimer_GetMSec();
	m_phaseProcessFunc.Set(this, &CPythonNetworkStream::OffLinePhase);
	m_phaseLeaveFunc.Set(this, &CPythonNetworkStream::__LeaveOfflinePhase);

	SetGameOffline();

	m_dwSelectedCharacterIndex = 0;

	__DirectEnterMode_Initialize();
	__BettingGuildWar_Initialize();
}


void CPythonNetworkStream::ClosePhase()
{
	PyCallClassMemberFunc(m_poHandler, "SetLoginPhase", Py_BuildValue("()"));
}

// Game Online
void CPythonNetworkStream::SetGameOnline()
{
	m_isGameOnline = TRUE;
}

void CPythonNetworkStream::SetGameOffline()
{
	m_isGameOnline = FALSE;
}

BOOL CPythonNetworkStream::IsGameOnline()
{
	return m_isGameOnline;
}

// Handler
void CPythonNetworkStream::SetHandler(PyObject* poHandler)
{
	m_poHandler = poHandler;
}

// ETC
uint32_t CPythonNetworkStream::GetMainActorVID()
{
	return m_dwMainActorVID;
}

uint32_t CPythonNetworkStream::GetMainActorRace()
{
	return m_dwMainActorRace;
}

uint32_t CPythonNetworkStream::GetMainActorEmpire()
{
	return m_dwMainActorEmpire;
}

uint32_t CPythonNetworkStream::GetMainActorSkillGroup()
{
	return m_dwMainActorSkillGroup;
}

void CPythonNetworkStream::SetEmpireID(uint32_t dwEmpireID)
{
	m_dwEmpireID = dwEmpireID;
}

uint32_t CPythonNetworkStream::GetEmpireID()
{
	return m_dwEmpireID;
}

void CPythonNetworkStream::__ClearSelectCharacterData()
{
	memset(&m_akSimplePlayerInfo, 0, sizeof(m_akSimplePlayerInfo));

	for (int32_t i = 0; i < PLAYER_PER_ACCOUNT; ++i)
	{
		m_adwGuildID[i] = 0;
		m_astrGuildName[i].clear();
	}
}

void CPythonNetworkStream::__DirectEnterMode_Initialize()
{
	m_kDirectEnterMode.m_isSet=false;
	m_kDirectEnterMode.m_dwChrSlotIndex=0;	
}

void CPythonNetworkStream::__DirectEnterMode_Set(uint32_t uChrSlotIndex)
{
	m_kDirectEnterMode.m_isSet=true;
	m_kDirectEnterMode.m_dwChrSlotIndex=uChrSlotIndex;
}

bool CPythonNetworkStream::__DirectEnterMode_IsSet()
{
	return m_kDirectEnterMode.m_isSet;
}

void CPythonNetworkStream::__InitializeMarkAuth()
{
	m_kMarkAuth.m_dwHandle=0;
	m_kMarkAuth.m_dwRandomKey=0;
}

void CPythonNetworkStream::__BettingGuildWar_Initialize()
{
	m_kBettingGuildWar.m_dwBettingMoney=0;
	m_kBettingGuildWar.m_dwObserverCount=0;
}

void CPythonNetworkStream::__BettingGuildWar_SetObserverCount(uint32_t uObserverCount)
{
	m_kBettingGuildWar.m_dwObserverCount=uObserverCount;
}

void CPythonNetworkStream::__BettingGuildWar_SetBettingMoney(uint32_t uBettingMoney)
{
	m_kBettingGuildWar.m_dwBettingMoney=uBettingMoney;
}

uint32_t CPythonNetworkStream::EXPORT_GetBettingGuildWarValue(const char* c_szValueName)
{
	if (stricmp(c_szValueName, "OBSERVER_COUNT") == 0)
		return m_kBettingGuildWar.m_dwObserverCount;

	if (stricmp(c_szValueName, "BETTING_MONEY") == 0)
		return m_kBettingGuildWar.m_dwBettingMoney;

	return 0;
}

void CPythonNetworkStream::__ServerTimeSync_Initialize()
{
	m_kServerTimeSync.m_dwChangeClientTime=0;
	m_kServerTimeSync.m_dwChangeServerTime=0;
}

void CPythonNetworkStream::SetWaitFlag()
{
	m_isWaitLoginKey = TRUE;
}

void CPythonNetworkStream::SendEmoticon(uint32_t eEmoticon)
{
	if(eEmoticon < m_EmoticonStringVector.size())
		SendChatPacket(m_EmoticonStringVector[eEmoticon].c_str());
	else
		assert(false && "SendEmoticon Error");
}

CPythonNetworkStream::CPythonNetworkStream()
{
	m_rokNetActorMgr=new CNetworkActorManager;

	memset(m_akSimplePlayerInfo, 0, sizeof(m_akSimplePlayerInfo));

	m_phaseProcessFunc.Clear();

	m_dwEmpireID = 0;
	m_dwGuildID = 0;

	m_dwMainActorVID = 0;
	m_dwMainActorRace = 0;
	m_dwMainActorEmpire = 0;
	m_dwMainActorSkillGroup = 0;
	m_poHandler = nullptr;

	m_dwLastGamePingTime = 0;

	m_dwLoginKey = 0;
	m_isWaitLoginKey = FALSE;
	m_isStartGame = FALSE;
	m_isChatEnable = TRUE;
	m_isEnableChatInsultFilter = FALSE;
	m_bComboSkillFlag = FALSE;
	m_strPhase = "OffLine";
	
	__InitializeGamePhase();
	__InitializeMarkAuth();

	__DirectEnterMode_Initialize();
	__BettingGuildWar_Initialize();

	std::fill(m_apoPhaseWnd, m_apoPhaseWnd+PHASE_WINDOW_NUM, (PyObject*)nullptr);
	m_poSerCommandParserWnd = nullptr;

	SetOffLinePhase();

	m_nextDownloadMarkTime = 0;
	
	CCheatDetectQueueMgr::Instance().RegisterReportFunction(
		std::bind(&CPythonNetworkStream::SendHackNotification, this, std::placeholders::_1, std::placeholders::_2)
	);
}

CPythonNetworkStream::~CPythonNetworkStream()
{
	Tracen("PythonNetworkMainStream Clear");
}
