#include "stdafx.h"
#include "constants.h"
#include "packet_info.h"

CPacketInfo::CPacketInfo() :
	m_pCurrentPacket(nullptr), m_dwStartTime(0)
{
}

CPacketInfo::~CPacketInfo()
{
	for (auto it = m_pPacketMap.begin(); it != m_pPacketMap.end(); ++it) 
	{
		M2_DELETE(it->second);
	}
}

void CPacketInfo::Set(int32_t header, int32_t iSize, const char * c_pszName)
{
	if (m_pPacketMap.find(header) != m_pPacketMap.end())
		return;

	TPacketElement * element = M2_NEW TPacketElement;

	element->iSize = iSize;
	element->stName.assign(c_pszName);
	element->iCalled = 0;
	element->dwLoad = 0;

	m_pPacketMap.insert(std::map<int32_t, TPacketElement *>::value_type(header, element));
}

bool CPacketInfo::Get(int32_t header, int32_t * size, const char ** c_ppszName)
{
	auto it = m_pPacketMap.find(header);
	if (it == m_pPacketMap.end())
		return false;

	*size = it->second->iSize;
	*c_ppszName = it->second->stName.c_str();

	m_pCurrentPacket = it->second;
	return true;
}

TPacketElement * CPacketInfo::GetElement(int32_t header)
{
	auto it = m_pPacketMap.find(header);
	if (it == m_pPacketMap.end())
		return nullptr;

	return it->second;
}

void CPacketInfo::Start()
{
	assert(m_pCurrentPacket != nullptr);
	m_dwStartTime = get_unix_ms_time();
}

void CPacketInfo::End()
{
	++m_pCurrentPacket->iCalled;
	m_pCurrentPacket->dwLoad += get_unix_ms_time() - m_dwStartTime;
}

void CPacketInfo::Log(const char * c_pszFileName)
{
	auto fp = fopen(c_pszFileName, "w");
	if (!fp)
		return;

	fprintf(fp, "Name             Called     Load       Ratio\n");

	auto it = m_pPacketMap.begin();
	while (it != m_pPacketMap.end())
	{
		TPacketElement * p = it->second;

		fprintf(fp, "%-16s %-10d %-10u %.2f\n",
				p->stName.c_str(),
				p->iCalled,
				p->dwLoad,
				p->iCalled != 0 ? (float) p->dwLoad / p->iCalled : 0.0f);

		++it;
	}

	fclose(fp);
}
///---------------------------------------------------------

CPacketInfoCG::CPacketInfoCG()
{
//	packetsdeki gibi düzenle
	Set(HEADER_CG_HANDSHAKE, sizeof(SPacketHandshake), "Handshake");
	Set(HEADER_CG_TIME_SYNC, sizeof(SPacketHandshake), "TimeSync");
	Set(HEADER_CG_MARK_LOGIN, sizeof(SPacketCGMarkLogin), "MarkLogin");
	Set(HEADER_CG_MARK_IDXLIST, sizeof(SPacketCGMarkIDXList), "MarkIdxList");
	Set(HEADER_CG_MARK_CRCLIST, sizeof(SPacketCGMarkCRCList), "MarkCrcList");
	Set(HEADER_CG_MARK_UPLOAD, sizeof(SPacketCGMarkUpload), "MarkUpload");
#ifdef _IMPROVED_PACKET_ENCRYPTION_
	Set(HEADER_CG_KEY_AGREEMENT, sizeof(SPacketKeyAgreement), "KeyAgreement");
#endif

	Set(HEADER_CG_GUILD_SYMBOL_UPLOAD, sizeof(SPacketCGGuildSymbolUpload), "SymbolUpload");
	Set(HEADER_CG_GUILD_SYMBOL_CRC, sizeof(SPacketCGGuildSymbolCRC), "SymbolCRC");
	Set(HEADER_CG_LOGIN, sizeof(SPacketCGLogin), "Login");
	Set(HEADER_CG_LOGIN2, sizeof(SPacketCGLogin2), "Login2");
	Set(HEADER_CG_LOGIN3, sizeof(SPacketCGLogin3), "Login3");
	Set(HEADER_CG_ATTACK, sizeof(SPacketCGAttack), "Attack");
	Set(HEADER_CG_CHAT, sizeof(SPacketCGChat), "Chat");
	Set(HEADER_CG_WHISPER, sizeof(SPacketCGWhisper), "Whisper");

	Set(HEADER_CG_PLAYER_SELECT, sizeof(SPacketCGSelectCharacter), "Select");
	Set(HEADER_CG_PLAYER_CREATE, sizeof(SPacketCGCreateCharacter), "Create");
	Set(HEADER_CG_PLAYER_DESTROY, sizeof(SPacketCGDestroyCharacter), "Delete");
	Set(HEADER_CG_ENTERGAME, sizeof(SPacketCGEnterFrontGame), "EnterGame");

	Set(HEADER_CG_ITEM_USE, sizeof(SPacketCGItemUse), "ItemUse");
	Set(HEADER_CG_ITEM_DROP, sizeof(SPacketCGItemDrop), "ItemDrop");
	Set(HEADER_CG_ITEM_DROP2, sizeof(SPacketCGItemDrop2), "ItemDrop2");
	Set(HEADER_CG_ITEM_MOVE, sizeof(SPacketCGItemMove), "ItemMove");
	Set(HEADER_CG_ITEM_PICKUP, sizeof(SPacketCGItemPickUp), "ItemPickup");

	Set(HEADER_CG_QUICKSLOT_ADD, sizeof(SPacketCGQuickSlotAdd), "QuickslotAdd");
	Set(HEADER_CG_QUICKSLOT_DEL, sizeof(SPacketCGQuickSlotDel), "QuickslotDel");
	Set(HEADER_CG_QUICKSLOT_SWAP, sizeof(SPacketCGQuickSlotSwap), "QuickslotSwap");

	Set(HEADER_CG_SHOP, sizeof(SPacketCGShop), "Shop");

	Set(HEADER_CG_ON_CLICK, sizeof(SPacketCGOnClick), "OnClick");
	Set(HEADER_CG_EXCHANGE, sizeof(SPacketCGExchange), "Exchange");
	Set(HEADER_CG_CHARACTER_POSITION, sizeof(SPacketCGPosition), "Position");
	Set(HEADER_CG_SCRIPT_ANSWER, sizeof(SPacketCGScriptAnswer), "ScriptAnswer");
	Set(HEADER_CG_SCRIPT_BUTTON, sizeof(SPacketCGScriptButton), "ScriptButton");
	Set(HEADER_CG_QUEST_INPUT_STRING, sizeof(SPacketCGQuestInputString), "QuestInputString");
	Set(HEADER_CG_QUEST_CONFIRM, sizeof(SPacketCGQuestConfirm), "QuestConfirm");

	Set(HEADER_CG_CHARACTER_MOVE, sizeof(SPacketCGMove), "Move");
	Set(HEADER_CG_SYNC_POSITION, sizeof(SPacketCGSyncPosition), "SyncPosition");

	Set(HEADER_CG_FLY_TARGETING, sizeof(SPacketCGFlyTargeting), "FlyTarget");
	Set(HEADER_CG_ADD_FLY_TARGETING, sizeof(SPacketCGFlyTargeting), "AddFlyTarget");
	Set(HEADER_CG_SHOOT, sizeof(SPacketCGShoot), "Shoot");

	Set(HEADER_CG_USE_SKILL, sizeof(SPacketCGUseSkill), "UseSkill");

	Set(HEADER_CG_ITEM_USE_TO_ITEM, sizeof(SPacketCGItemUseToItem), "UseItemToItem");
	Set(HEADER_CG_TARGET, sizeof(SPacketCGTarget), "Target");
	Set(HEADER_CG_MESSENGER, sizeof(SPacketCGMessenger), "Messenger");

	Set(HEADER_CG_PARTY_REMOVE, sizeof(SPacketCGPartyRemove), "PartyRemove");
	Set(HEADER_CG_PARTY_INVITE, sizeof(SPacketCGPartyInvite), "PartyInvite");
	Set(HEADER_CG_PARTY_INVITE_ANSWER, sizeof(SPacketCGPartyInviteAnswer), "PartyInviteAnswer");
	Set(HEADER_CG_PARTY_SET_STATE, sizeof(SPacketCGPartySetState), "PartySetState");
	Set(HEADER_CG_PARTY_USE_SKILL, sizeof(SPacketCGPartyUseSkill), "PartyUseSkill");
	Set(HEADER_CG_PARTY_PARAMETER, sizeof(SPacketCGPartyParameter), "PartyParam");

	Set(HEADER_CG_EMPIRE, sizeof(SPacketCGEmpire), "Empire");
	Set(HEADER_CG_SAFEBOX_CHECKOUT, sizeof(SPacketCGSafeboxCheckout), "SafeboxCheckout");
	Set(HEADER_CG_SAFEBOX_CHECKIN, sizeof(SPacketCGSafeboxCheckin), "SafeboxCheckin");

	Set(HEADER_CG_SAFEBOX_ITEM_MOVE, sizeof(SPacketCGItemMove), "SafeboxItemMove");

	Set(HEADER_CG_GUILD, sizeof(SPacketCGGuild), "Guild");
	Set(HEADER_CG_ANSWER_MAKE_GUILD, sizeof(SPacketCGAnswerMakeGuild), "AnswerMakeGuild");

	Set(HEADER_CG_FISHING, sizeof(SPacketCGFishing), "Fishing");
	Set(HEADER_CG_GIVE_ITEM, sizeof(SPacketCGGiveItem), "ItemGive");
	Set(HEADER_CG_HACK, sizeof(SPacketCGHack), "Hack");
	Set(HEADER_CG_MYSHOP, sizeof(SPacketCGMyShop), "MyShop");

	Set(HEADER_CG_REFINE, sizeof(SPacketCGRefine), "Refine");
	Set(HEADER_CG_CHANGE_NAME, sizeof(SPacketCGChangeName), "ChangeName");

	Set(HEADER_CG_PONG, sizeof(uint8_t), "Pong");
	Set(HEADER_CG_MALL_CHECKOUT, sizeof(SPacketCGSafeboxCheckout), "MallCheckout");

	Set(HEADER_CG_SCRIPT_SELECT_ITEM, sizeof(SPacketCGScriptSelectItem), "ScriptSelectItem");

	Set(HEADER_CG_DRAGON_SOUL_REFINE, sizeof(SPacketCGDragonSoulRefine), "DragonSoulRefine");
	Set(HEADER_CG_STATE_CHECKER, sizeof(uint8_t), "ServerStateCheck");
#ifdef ENABLE_ACCE_SYSTEM
	Set(HEADER_CG_ACCE, sizeof(TPacketAcce), "Acce");
#endif
}

CPacketInfoCG::~CPacketInfoCG()
{
	Log("packet_info.txt");
}

////////////////////////////////////////////////////////////////////////////////
CPacketInfoGG::CPacketInfoGG()
{
	Set(HEADER_GG_SETUP,		sizeof(TPacketGGSetup),		"Setup");
	Set(HEADER_GG_LOGIN,		sizeof(TPacketGGLogin),		"Login");
	Set(HEADER_GG_LOGOUT,		sizeof(TPacketGGLogout),	"Logout");
	Set(HEADER_GG_RELAY,		sizeof(TPacketGGRelay),		"Relay");
	Set(HEADER_GG_NOTICE,		sizeof(TPacketGGNotice),	"Notice");
#ifdef ENABLE_FULL_NOTICE
	Set(HEADER_GG_BIG_NOTICE,	sizeof(TPacketGGNotice),	"BigNotice");
#endif
	Set(HEADER_GG_SHUTDOWN,		sizeof(TPacketGGShutdown),	"Shutdown");
	Set(HEADER_GG_GUILD,		sizeof(TPacketGGGuild),		"Guild");
	Set(HEADER_GG_SHOUT,		sizeof(TPacketGGShout),		"Shout");
	Set(HEADER_GG_DISCONNECT,	    	sizeof(TPacketGGDisconnect),	"Disconnect");
	Set(HEADER_GG_MESSENGER_ADD,	sizeof(TPacketGGMessenger),	"MessengerAdd");
	Set(HEADER_GG_MESSENGER_REMOVE,	sizeof(TPacketGGMessenger),	"MessengerRemove");
	Set(HEADER_GG_FIND_POSITION,	sizeof(TPacketGGFindPosition),	"FindPosition");
	Set(HEADER_GG_WARP_CHARACTER,	sizeof(TPacketGGWarpCharacter),	"WarpCharacter");
	Set(HEADER_GG_GUILD_WAR_ZONE_MAP_INDEX, sizeof(TPacketGGGuildWarMapIndex), "GuildWarMapIndex");
	Set(HEADER_GG_TRANSFER,		sizeof(TPacketGGTransfer),	"Transfer");
	Set(HEADER_GG_XMAS_WARP_SANTA,	sizeof(TPacketGGXmasWarpSanta),	"XmasWarpSanta");
	Set(HEADER_GG_XMAS_WARP_SANTA_REPLY, sizeof(TPacketGGXmasWarpSantaReply), "XmasWarpSantaReply");
	Set(HEADER_GG_LOGIN_PING,		sizeof(TPacketGGLoginPing),	"LoginPing");
	Set(HEADER_GG_BLOCK_CHAT,		sizeof(TPacketGGBlockChat),	"BlockChat");
	Set(HEADER_GG_CHECK_AWAKENESS,	sizeof(TPacketGGCheckAwakeness), "CheckAwakeness");
}

CPacketInfoGG::~CPacketInfoGG()
{
	Log("p2p_packet_info.txt");
}

