#include "stdafx.h"
#include "input.h"
#include "constants.h"
#include "config.h"
#include "utils.h"
#include "desc_client.h"
#include "desc_manager.h"
#include "buffer_manager.h"
#include "protocol.h"
#include "char.h"
#include "char_manager.h"
#include "item.h"
#include "item_manager.h"
#include "cmd.h"
#include "shop.h"
#include "shop_manager.h"
#include "safebox.h"
#include "regen.h"
#include "battle.h"
#include "exchange.h"
#include "quest_manager.h"
#include "profiler.h"
#include "messenger_manager.h"
#include "party.h"
#include "p2p.h"
#include "affect.h"
#include "guild.h"
#include "guild_manager.h"
#include "log.h"
#include "banword.h"
#include "unique_item.h"
#include "building.h"
#include "locale_service.h"
#include "gm.h"
#include "spam.h"
#include "ani.h"
#include "motion.h"
#include "ox_event.h"
#include "locale_service.h"
#include "dragon_soul.h"
#include "belt_inventory_helper.h"
#include "battleground.h"

static char	__escape_string[1024];
static char	__escape_string2[1024];

static int32_t __deposit_limit()
{
	return (1000*10000); // 1천만
}

void SendBlockChatInfo(LPCHARACTER ch, int32_t sec)
{
	if (sec <= 0)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("채팅 금지 상태입니다."));
		return;
	}

	int32_t hour = sec / 3600;
	sec -= hour * 3600;

	int32_t min = (sec / 60);
	sec -= min * 60;

	char buf[128+1];

	if (hour > 0 && min > 0)
		snprintf(buf, sizeof(buf), LC_TEXT("%d 시간 %d 분 %d 초 동안 채팅금지 상태입니다"), hour, min, sec);
	else if (hour > 0 && min == 0)
		snprintf(buf, sizeof(buf), LC_TEXT("%d 시간 %d 초 동안 채팅금지 상태입니다"), hour, sec);
	else if (hour == 0 && min > 0)
		snprintf(buf, sizeof(buf), LC_TEXT("%d 분 %d 초 동안 채팅금지 상태입니다"), min, sec);
	else
		snprintf(buf, sizeof(buf), LC_TEXT("%d 초 동안 채팅금지 상태입니다"), sec);

	ch->ChatPacket(CHAT_TYPE_INFO, buf);
}

EVENTINFO(spam_event_info)
{
	char host[MAX_HOST_LENGTH+1];

	spam_event_info()
	{
		::memset( host, 0, MAX_HOST_LENGTH+1 );
	}
};

typedef std::unordered_map<std::string, std::pair<uint32_t, LPEVENT> > spam_score_of_ip_t;
spam_score_of_ip_t spam_score_of_ip;

EVENTFUNC(block_chat_by_ip_event)
{
	spam_event_info* info = dynamic_cast<spam_event_info*>( event->info );

	if ( info == nullptr )
	{
		sys_err( "block_chat_by_ip_event> <Factor> Null pointer" );
		return 0;
	}

	const char * host = info->host;

	spam_score_of_ip_t::iterator it = spam_score_of_ip.find(host);

	if (it != spam_score_of_ip.end())
	{
		it->second.first = 0;
		it->second.second = nullptr;
	}

	return 0;
}

bool SpamBlockCheck(LPCHARACTER ch, const char* const buf, const size_t buflen)
{
	if (ch->GetLevel() < g_iSpamBlockMaxLevel)
	{
		spam_score_of_ip_t::iterator it = spam_score_of_ip.find(ch->GetDesc()->GetHostName());

		if (it == spam_score_of_ip.end())
		{
			spam_score_of_ip.insert(std::make_pair(ch->GetDesc()->GetHostName(), std::make_pair(0, (LPEVENT) nullptr)));
			it = spam_score_of_ip.find(ch->GetDesc()->GetHostName());
		}

		if (it->second.second)
		{
			SendBlockChatInfo(ch, event_time(it->second.second) / passes_per_sec);
			return true;
		}

		uint32_t score;
		const char * word = SpamManager::Instance().GetSpamScore(buf, buflen, score);

		it->second.first += score;

		if (word)
			sys_log(0, "SPAM_SCORE: %s text: %s score: %u total: %u word: %s", ch->GetName(), buf, score, it->second.first, word);

		if (it->second.first >= g_uiSpamBlockScore)
		{
			spam_event_info* info = AllocEventInfo<spam_event_info>();
			strlcpy(info->host, ch->GetDesc()->GetHostName(), sizeof(info->host));

			it->second.second = event_create(block_chat_by_ip_event, info, PASSES_PER_SEC(g_uiSpamBlockDuration));
			sys_log(0, "SPAM_IP: %s for %u seconds", info->host, g_uiSpamBlockDuration);

			LogManager::Instance().CharLog(ch, 0, "SPAM", word);

			SendBlockChatInfo(ch, event_time(it->second.second) / passes_per_sec);

			return true;
		}
	}

	return false;
}

enum
{
	TEXT_TAG_PLAIN,
	TEXT_TAG_TAG, // ||
	TEXT_TAG_COLOR, // |cffffffff
	TEXT_TAG_HYPERLINK_START, // |H
	TEXT_TAG_HYPERLINK_END, // |h ex) |Hitem:1234:1:1:1|h
	TEXT_TAG_RESTORE_COLOR,
};

int32_t GetTextTag(const char * src, int32_t maxLen, int32_t & tagLen, std::string & extraInfo)
{
	tagLen = 1;

	if (maxLen < 2 || *src != '|')
		return TEXT_TAG_PLAIN;

	const char * cur = ++src;

	if (*cur == '|') // ||는 |로 표시한다.
	{
		tagLen = 2;
		return TEXT_TAG_TAG;
	}
	else if (*cur == 'c') // color |cffffffffblahblah|r
	{
		tagLen = 2;
		return TEXT_TAG_COLOR;
	}
	else if (*cur == 'H') // hyperlink |Hitem:10000:0:0:0:0|h[이름]|h
	{
		tagLen = 2;
		return TEXT_TAG_HYPERLINK_START;
	}
	else if (*cur == 'h') // end of hyperlink
	{
		tagLen = 2;
		return TEXT_TAG_HYPERLINK_END;
	}

	return TEXT_TAG_PLAIN;
}

void GetTextTagInfo(const char * src, int32_t src_len, int32_t & hyperlinks, bool & colored)
{
	colored = false;
	hyperlinks = 0;

	int32_t len;
	std::string extraInfo;

	for (int32_t i = 0; i < src_len;)
	{
		int32_t tag = GetTextTag(&src[i], src_len - i, len, extraInfo);

		if (tag == TEXT_TAG_HYPERLINK_START)
			++hyperlinks;

		if (tag == TEXT_TAG_COLOR)
			colored = true;

		i += len;
	}
}

int32_t ProcessTextTag(LPCHARACTER ch, const char * c_pszText, size_t len)
{
	//2012.05.17 김용욱
	//0 : 정상적으로 사용
	//1 : 금강경 부족
	//2 : 금강경이 있으나, 개인상점에서 사용중
	//3 : 교환중
	//4 : 에러
	int32_t hyperlinks;
	bool colored = false;
	
	GetTextTagInfo(c_pszText, len, hyperlinks, colored);

	if (colored == true && hyperlinks == 0)
		return 4;

	if (g_bDisablePrismNeed)
		return 0;

	int32_t nPrismCount = ch->CountSpecifyItem(ITEM_PRISM);

	if (nPrismCount < hyperlinks)
		return 1;

	if (!ch->GetMyShop())
	{
		ch->RemoveSpecifyItem(ITEM_PRISM, hyperlinks);
		return 0;
	}
	else
	{
		int32_t sellingNumber = ch->GetMyShop()->GetNumberByVnum(ITEM_PRISM);
		if(nPrismCount - sellingNumber < hyperlinks)
		{
			return 2;
		}
		else
		{
			ch->RemoveSpecifyItem(ITEM_PRISM, hyperlinks);
			return 0;
		}
	}
}

int32_t CInputMain::Whisper(LPCHARACTER ch, const char * data, size_t uiBytes)
{
	const SPacketCGWhisper* pinfo = reinterpret_cast<const SPacketCGWhisper*>(data);

	if (uiBytes < pinfo->wSize)
		return -1;

	int32_t iExtraLen = pinfo->wSize - sizeof(SPacketCGWhisper);

	if (iExtraLen < 0)
	{
		sys_err("invalid packet length (len %d size %u buffer %u)", iExtraLen, pinfo->wSize, uiBytes);
		ch->GetDesc()->SetPhase(PHASE_CLOSE);
		return -1;
	}

	if (ch->IncreaseChatCounter() >= 10 && !ch->IsGM())
	{
		if (ch->GetChatCounter() == 10)
		{
			ch->GetDesc()->DelayedDisconnect(0);
			return (iExtraLen);
		}
	}

	if (ch->FindAffect(AFFECT_BLOCK_CHAT))
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("채팅 금지 상태입니다."));
		return (iExtraLen);
	}

	LPCHARACTER pkChr = CHARACTER_MANAGER::Instance().FindPC(pinfo->szNameTo);

	if (pkChr == ch)
		return (iExtraLen);

	LPDESC pkDesc = nullptr;

	uint8_t bOpponentEmpire = 0;
	uint32_t dwOpponentPID = 0;
	const char* szOpponentName = "";

	if (g_bIsTestServer)
	{
		if (!pkChr)
			sys_log(0, "Whisper to %s(%s) from %s", "Null", pinfo->szNameTo, ch->GetName());
		else
			sys_log(0, "Whisper to %s(%s) from %s", pkChr->GetName(), pinfo->szNameTo, ch->GetName());
	}
		
	if (ch->IsBlockMode(BLOCK_WHISPER) && !pkChr->IsGM())
	{
		if (ch->GetDesc())
		{
			SPacketGCWhisper pack;
			pack.bType = WHISPER_TYPE_SENDER_BLOCKED;
			pack.wSize = sizeof(SPacketGCWhisper);
			strlcpy(pack.szNameFrom, pinfo->szNameTo, sizeof(pack.szNameFrom));
			ch->GetDesc()->Packet(&pack, sizeof(pack));
		}
		return iExtraLen;
	}

	if (!pkChr)
	{
		CCI * pkCCI = P2P_MANAGER::Instance().Find(pinfo->szNameTo);

		if (pkCCI)
		{
			pkDesc = pkCCI->pkDesc;
			pkDesc->SetRelay(pinfo->szNameTo);
			bOpponentEmpire = pkCCI->bEmpire;
			szOpponentName = pkChr->GetName();
			dwOpponentPID = pkChr->GetPlayerID();

			if (g_bIsTestServer)
			{
				sys_log (0, "Whisper out to %s from %s (Channel %d Mapindex %d)", pkCCI->szName, ch->GetName(), pkCCI->bChannel, pkCCI->lMapIndex);
			}
		}
	}
	else
	{
		pkDesc = pkChr->GetDesc();
		bOpponentEmpire = pkChr->GetEmpire();
		szOpponentName = pkChr->GetName();
		dwOpponentPID = pkChr->GetPlayerID();
	}

	if (!pkDesc)
	{
		if (ch->GetDesc())
		{
			SPacketGCWhisper pack;
			pack.bType = WHISPER_TYPE_NOT_EXIST;
			pack.wSize = sizeof(SPacketGCWhisper);
			strlcpy(pack.szNameFrom, pinfo->szNameTo, sizeof(pack.szNameFrom));
			ch->GetDesc()->Packet(&pack, sizeof(SPacketGCWhisper));
			sys_log(0, "WHISPER: no player");
		}
	}
	else
	{
		if (ch->IsBlockMode(BLOCK_WHISPER))
		{
			if (ch->GetDesc())
			{
				SPacketGCWhisper pack;
				pack.bType = WHISPER_TYPE_SENDER_BLOCKED;
				pack.wSize = sizeof(SPacketGCWhisper);
				strlcpy(pack.szNameFrom, pinfo->szNameTo, sizeof(pack.szNameFrom));
				ch->GetDesc()->Packet(&pack, sizeof(pack));
			}
		}
		else if (pkChr && pkChr->IsBlockMode(BLOCK_WHISPER))
		{
			if (ch->GetDesc())
			{
				SPacketGCWhisper pack;
				pack.bType = WHISPER_TYPE_TARGET_BLOCKED;
				pack.wSize = sizeof(SPacketGCWhisper);
				strlcpy(pack.szNameFrom, pinfo->szNameTo, sizeof(pack.szNameFrom));
				ch->GetDesc()->Packet(&pack, sizeof(pack));
			}
		}
		else
		{
			uint8_t bType = WHISPER_TYPE_NORMAL;

			char buf[CHAT_MAX_LEN + 1];
			strlcpy(buf, data + sizeof(SPacketCGWhisper), MIN(iExtraLen + 1, sizeof(buf)));
			const size_t buflen = strlen(buf);

			if (SpamBlockCheck(ch, buf, buflen))
			{
				if (!pkChr)
				{
					CCI * pkCCI = P2P_MANAGER::Instance().Find(pinfo->szNameTo);

					if (pkCCI)
					{
						pkDesc->SetRelay("");
					}
				}
				return iExtraLen;
			}

			CBanwordManager::Instance().ConvertString(buf, buflen);

			int32_t processReturn = ProcessTextTag(ch, buf, buflen);
			if (0!=processReturn)
			{
				if (ch->GetDesc())
				{
					SItemTable_Server * pTable = ITEM_MANAGER::Instance().GetTable(ITEM_PRISM);

					if (pTable)
					{
						char buf[128];
						int32_t len;
						if (3==processReturn) //교환중
							len = snprintf(buf, sizeof(buf), LC_TEXT("사용할수 없습니다."), pTable->szLocaleName);
						else
							len = snprintf(buf, sizeof(buf), LC_TEXT("%s이 필요합니다."), pTable->szLocaleName);

						if (len < 0 || len >= (int32_t) sizeof(buf))
							len = sizeof(buf) - 1;

						++len;  // \0 문자 포함

						SPacketGCWhisper pack;
						pack.bType = WHISPER_TYPE_ERROR;
						pack.wSize = sizeof(SPacketGCWhisper) + len;
						strlcpy(pack.szNameFrom, pinfo->szNameTo, sizeof(pack.szNameFrom));

						ch->GetDesc()->BufferedPacket(&pack, sizeof(pack));
						ch->GetDesc()->Packet(buf, len);

						sys_log(0, "WHISPER: not enough %s: char: %s", pTable->szLocaleName, ch->GetName());
					}
				}

				// 릴래이 상태일 수 있으므로 릴래이를 풀어준다.
				pkDesc->SetRelay("");
				return (iExtraLen);
			}

			if (ch->IsGM())
				bType = (bType & 0xF0) | WHISPER_TYPE_GM;

			if (buflen > 0)
			{
				SPacketGCWhisper pack;
				pack.wSize = sizeof(SPacketGCWhisper) + buflen;
				pack.bType = bType;
				strlcpy(pack.szNameFrom, ch->GetName(), sizeof(pack.szNameFrom));

				// desc->BufferedPacket을 하지 않고 버퍼에 써야하는 이유는 
				// P2P relay되어 패킷이 캡슐화 될 수 있기 때문이다.
				TEMP_BUFFER tmpbuf;

				tmpbuf.write(&pack, sizeof(pack));
				tmpbuf.write(buf, buflen);

				pkDesc->Packet(tmpbuf.read_peek(), tmpbuf.size());

				// sys_log(0, "WHISPER: %s -> %s : %s", ch->GetName(), pinfo->szNameTo, buf);

				if (ch->IsGM())
				{
					LogManager::Instance().EscapeString(__escape_string, sizeof(__escape_string), buf, buflen);
					LogManager::Instance().EscapeString(__escape_string2, sizeof(__escape_string2), pinfo->szNameTo, sizeof(pack.szNameFrom));
					LogManager::Instance().ChatLog(ch->GetMapIndex(), ch->GetPlayerID(), ch->GetName(), 0, __escape_string2, "WHISPER", __escape_string, ch->GetDesc() ? ch->GetDesc()->GetHostName() : "");
				}
			}
		}
	}
	if(pkDesc)
		pkDesc->SetRelay("");

	return (iExtraLen);
}

struct RawPacketToCharacterFunc
{
	const void * m_buf;
	int32_t	m_buf_len;

	RawPacketToCharacterFunc(const void * buf, int32_t buf_len) : m_buf(buf), m_buf_len(buf_len)
	{
	}

	void operator () (LPCHARACTER c)
	{
		if (!c->GetDesc())
			return;

		c->GetDesc()->Packet(m_buf, m_buf_len);
	}
};

struct FEmpireChatPacket
{
	SPacketGCChat& p;
	const char* orig_msg;
	int32_t orig_len;

	uint8_t bEmpire;
	int32_t iMapIndex;
	int32_t namelen;

	FEmpireChatPacket(SPacketGCChat& p, const char* chat_msg, int32_t len, uint8_t bEmpire, int32_t iMapIndex, int32_t iNameLen)
		: p(p), orig_msg(chat_msg), orig_len(len), bEmpire(bEmpire), iMapIndex(iMapIndex), namelen(iNameLen)
	{
	}

	void operator () (LPDESC d)
	{
		if (!d->GetCharacter())
			return;

		if (d->GetCharacter()->GetMapIndex() != iMapIndex)
			return;

		d->BufferedPacket(&p, sizeof(SPacketGCChat));
		d->Packet(orig_msg, orig_len);
	}
};

int32_t CInputMain::Chat(LPCHARACTER ch, const char * data, size_t uiBytes)
{
	const SPacketCGChat* pinfo = reinterpret_cast<const SPacketCGChat*>(data);

	if (uiBytes < pinfo->length)
		return -1;

	const int32_t iExtraLen = pinfo->length - sizeof(SPacketCGChat);

	if (iExtraLen < 0)
	{
		sys_err("invalid packet length (len %d size %u buffer %u)", iExtraLen, pinfo->length, uiBytes);
		ch->GetDesc()->SetPhase(PHASE_CLOSE);
		return -1;
	}

	char buf[CHAT_MAX_LEN - (CHARACTER_NAME_MAX_LEN + 3) + 1];
	strlcpy(buf, data + sizeof(SPacketCGChat), MIN(iExtraLen + 1, sizeof(buf)));
	const size_t buflen = strlen(buf);

	if (buflen > 1 && *buf == '/')
	{
		interpret_command(ch, buf + 1, buflen - 1);
		return iExtraLen;
	}
	
	if (ch && CBattlegroundManager::Instance().IsEventMap(ch->GetMapIndex()))
		return iExtraLen;
		
	if (ch->IncreaseChatCounter() >= 6 && !ch->IsGM())
	{
		if (ch->GetChatCounter() == 10)
			ch->GetDesc()->DelayedDisconnect(0);
		return iExtraLen;
	}

	// 채팅 금지 Affect 처리
	const CAffect* pAffect = ch->FindAffect(AFFECT_BLOCK_CHAT);

	if (pAffect != nullptr)
	{
		SendBlockChatInfo(ch, pAffect->lDuration);
		return iExtraLen;
	}

//	if (true == SpamBlockCheck(ch, buf, buflen))
//	{
//		return iExtraLen;
//	}

	CBanwordManager::Instance().ConvertString(buf, buflen);

	int32_t processReturn = ProcessTextTag(ch, buf, buflen);
	if (processReturn)
	{
		const SItemTable_Server* pTable = ITEM_MANAGER::Instance().GetTable(ITEM_PRISM);

		if (nullptr != pTable)
		{
			if (3==processReturn) 
				ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("사용할수 없습니다."), pTable->szLocaleName);
			else
				ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("%s이 필요합니다."), pTable->szLocaleName);
						
		}

		return iExtraLen;
	}

	char chatbuf[CHAT_MAX_LEN + 1];
	int32_t len;

	char szHyperLinkedName[120];
	sprintf(szHyperLinkedName, "|Hmsg:%s|h%s|h|r", ch->GetName(), ch->GetName());

	switch (pinfo->type)
	{
		case CHAT_TYPE_SHOUT:
		{
			static const char* colorbuf[] = { "|cFFffa200|H|h[Yonetici]", "|cFFff2727|H|h", "|cFFfff989|H|h", "|cFF1c67e9|H|h" };
			len = snprintf(chatbuf, sizeof(chatbuf), "%s%s|h|r : %s", (ch->IsGM() ? colorbuf[0] : colorbuf[MINMAX(0, ch->GetEmpire(), 3)]), szHyperLinkedName, buf);
		}
		break;

		default:
			len = snprintf(chatbuf, sizeof(chatbuf), "%s : %s", ch->GetName(), buf);
			break;
	}

	if (CHAT_TYPE_SHOUT == pinfo->type)
	{
		LogManager::Instance().ShoutLog(g_bChannel, ch->GetEmpire(), chatbuf);
	}

	if (len < 0 || len >= (int32_t) sizeof(chatbuf))
		len = sizeof(chatbuf) - 1;

	if (pinfo->type == CHAT_TYPE_SHOUT)
	{
		// const int32_t SHOUT_LIMIT_LEVEL = 15;

		if (ch->GetLevel() < g_iShoutLimitLevel)
		{
			ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("외치기는 레벨 %d 이상만 사용 가능 합니다."), g_iShoutLimitLevel);
			return (iExtraLen);
		}

		if (thecore_heart->pulse - (int32_t) ch->GetLastShoutPulse() < passes_per_sec * 15)
			return (iExtraLen);

		ch->SetLastShoutPulse(thecore_heart->pulse);

		TPacketGGShout p;

		p.bHeader = HEADER_GG_SHOUT;
		p.bEmpire = ch->GetEmpire();
		strlcpy(p.szText, chatbuf, sizeof(p.szText));

		P2P_MANAGER::Instance().Send(&p, sizeof(TPacketGGShout));

		SendShout(chatbuf, ch->GetEmpire());

		return (iExtraLen);
	}

	SPacketGCChat pack_chat;
	pack_chat.size = sizeof(SPacketGCChat) + len;
	pack_chat.type = pinfo->type;
	pack_chat.dwVID = ch->GetVID();

	switch (pinfo->type)
	{
		case CHAT_TYPE_TALKING:
			{
				const int32_t NORMAL_CHAT_LIMIT = 5;
				if (ch->GetLevel() < NORMAL_CHAT_LIMIT)
				{
					ch->ChatPacket(CHAT_TYPE_INFO, "5.Seviyeden once bunu yapamazsin");
					return (iExtraLen);
				}

				const DESC_MANAGER::DESC_SET & c_ref_set = DESC_MANAGER::Instance().GetClientSet();


				{
					std::for_each(c_ref_set.begin(), c_ref_set.end(), 
							FEmpireChatPacket(pack_chat,
								chatbuf,
								len, 
								(ch->GetGMLevel() > GM_PLAYER ||
								 ch->IsEquipUniqueGroup(UNIQUE_GROUP_RING_OF_LANGUAGE)) ? 0 : ch->GetEmpire(), 
								ch->GetMapIndex(), strlen(ch->GetName())));

					if (ch->IsGM())
					{
						LogManager::Instance().EscapeString(__escape_string, sizeof(__escape_string), chatbuf, len);
						LogManager::Instance().ChatLog(ch->GetMapIndex(), ch->GetPlayerID(), ch->GetName(), 0, "", "NORMAL", __escape_string, ch->GetDesc() ? ch->GetDesc()->GetHostName() : "");
					}
				}
			}
			break;

		case CHAT_TYPE_PARTY:
			{
				if (!ch->GetParty())
					ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("파티 중이 아닙니다."));
				else
				{
					TEMP_BUFFER tbuf;
					
					tbuf.write(&pack_chat, sizeof(pack_chat));
					tbuf.write(chatbuf, len);

					RawPacketToCharacterFunc f(tbuf.read_peek(), tbuf.size());
					ch->GetParty()->ForEachOnlineMember(f);

					if (ch->IsGM())
					{
						LogManager::Instance().EscapeString(__escape_string, sizeof(__escape_string), chatbuf, len);
						LogManager::Instance().ChatLog(ch->GetMapIndex(), ch->GetPlayerID(), ch->GetName(), ch->GetParty()->GetLeaderPID(), "", "PARTY", __escape_string, ch->GetDesc() ? ch->GetDesc()->GetHostName() : "");
					}
				}
			}
			break;

		case CHAT_TYPE_GUILD:
			{
				if (!ch->GetGuild())
					ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("길드에 가입하지 않았습니다."));
				else
				{
					ch->GetGuild()->Chat(chatbuf);
					if (ch->IsGM())
					{
						LogManager::Instance().EscapeString(__escape_string, sizeof(__escape_string), chatbuf, len);
						LogManager::Instance().ChatLog(ch->GetMapIndex(), ch->GetPlayerID(), ch->GetName(), ch->GetGuild()->GetID(), ch->GetGuild()->GetName(), "GUILD", __escape_string, ch->GetDesc() ? ch->GetDesc()->GetHostName() : "");
					}
				}
			}
			break;

		default:
			sys_err("Unknown chat type %d", pinfo->type);
			break;
	}

	return (iExtraLen);
}

void CInputMain::ItemUse(LPCHARACTER ch, const char * data)
{
	ch->UseItem(((SPacketCGItemUse*) data)->pos);
}

void CInputMain::ItemToItem(LPCHARACTER ch, const char * pcData)
{
	SPacketCGItemUseToItem * p = (SPacketCGItemUseToItem *) pcData;
	if (ch)
		ch->UseItem(p->source_pos, p->target_pos);
}

void CInputMain::ItemDrop2(LPCHARACTER ch, const char * data)
{
	if (ch && CBattlegroundManager::Instance().IsEventMap(ch->GetMapIndex()))
		return;

	SPacketCGItemDrop2 * pinfo = (SPacketCGItemDrop2 *) data;

	// 엘크가 0보다 크면 엘크를 버리는 것 이다.
	
	if (!ch)
		return;
	if (pinfo->gold > 0)
		ch->DropGold(pinfo->gold);
	else
		ch->DropItem(pinfo->pos, pinfo->count);
}

void CInputMain::ItemMove(LPCHARACTER ch, const char * data)
{
	SPacketCGItemMove* pinfo = (SPacketCGItemMove*) data;

	if (ch)
		ch->MoveItem(pinfo->pos, pinfo->change_pos, pinfo->num);
}

void CInputMain::ItemPickup(LPCHARACTER ch, const char * data)
{
	SPacketCGItemPickUp* pinfo = (SPacketCGItemPickUp*) data;
	if (ch)
		ch->PickupItem(pinfo->vid);
}

void CInputMain::QuickslotAdd(LPCHARACTER ch, const char * data)
{
	SPacketCGQuickSlotAdd* pinfo = (SPacketCGQuickSlotAdd*) data;
	if (!ch || !pinfo)
		return;

    if (pinfo->slot.Type == QUICKSLOT_TYPE_ITEM)
    {
        LPITEM item = nullptr;
        TItemPos srcCell(INVENTORY, pinfo->slot.Position);
        if (!(item = ch->GetItem(srcCell)))
            return;
        if (item->GetType() != ITEM_USE && item->GetType() != ITEM_QUEST)
            return;
    }
	ch->SetQuickslot(pinfo->pos, pinfo->slot);
}

void CInputMain::QuickslotDelete(LPCHARACTER ch, const char * data)
{
	SPacketCGQuickSlotDel* pinfo = (SPacketCGQuickSlotDel*) data;
	if (ch)
		ch->DelQuickslot(pinfo->pos);
}

void CInputMain::QuickslotSwap(LPCHARACTER ch, const char * data)
{
	SPacketCGQuickSlotSwap* pinfo = (SPacketCGQuickSlotSwap*) data;
	if (ch)
		ch->SwapQuickslot(pinfo->pos, pinfo->change_pos);
}

int32_t CInputMain::Messenger(LPCHARACTER ch, const char* c_pData, size_t uiBytes)
{
	SPacketCGMessenger* p = (SPacketCGMessenger*) c_pData;
	
	if (uiBytes < sizeof(SPacketCGMessenger))
		return -1;

	c_pData += sizeof(SPacketCGMessenger);
	uiBytes -= sizeof(SPacketCGMessenger);

	switch (p->subheader)
	{
		case MESSENGER_SUBHEADER_CG_ADD_BY_VID:
			{
				if (uiBytes < sizeof(TPacketMessengerAddByVID))
					return -1;

				TPacketMessengerAddByVID* p2 = (TPacketMessengerAddByVID*) c_pData;
				LPCHARACTER ch_companion = CHARACTER_MANAGER::Instance().Find(p2->vid);

				if (!ch_companion)
					return sizeof(TPacketMessengerAddByVID);

				if (ch->IsObserverMode())
					return sizeof(TPacketMessengerAddByVID);

				if (ch_companion->IsBlockMode(BLOCK_MESSENGER_INVITE))
				{
					ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("상대방이 메신져 추가 거부 상태입니다."));
					return sizeof(TPacketMessengerAddByVID);
				}

				LPDESC d = ch_companion->GetDesc();

				if (!d)
					return sizeof(TPacketMessengerAddByVID);

				if (ch->GetGMLevel() == GM_PLAYER && ch_companion->GetGMLevel() != GM_PLAYER)
				{
					ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("<메신져> 운영자는 메신져에 추가할 수 없습니다."));
					return sizeof(TPacketMessengerAddByVID);
				}

				if (ch->GetDesc() == d) // 자신은 추가할 수 없다.
					return sizeof(TPacketMessengerAddByVID);

				MessengerManager::Instance().RequestToAdd(ch, ch_companion);
				//MessengerManager::Instance().AddToList(ch->GetName(), ch_companion->GetName());
			}
			return sizeof(TPacketMessengerAddByVID);

		case MESSENGER_SUBHEADER_CG_ADD_BY_NAME:
			{
				if (uiBytes < CHARACTER_NAME_MAX_LEN)
					return -1;

				char name[CHARACTER_NAME_MAX_LEN + 1];
				strlcpy(name, c_pData, sizeof(name));

				if (ch->GetGMLevel() == GM_PLAYER && GM::get_level(name) != GM_PLAYER)
				{
					ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("<메신져> 운영자는 메신져에 추가할 수 없습니다."));
					return CHARACTER_NAME_MAX_LEN;
				}

				LPCHARACTER tch = CHARACTER_MANAGER::Instance().FindPC(name);

				if (!tch)
					ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("%s 님은 접속되 있지 않습니다."), name);
				else
				{
					if (tch == ch) // 자신은 추가할 수 없다.
						return CHARACTER_NAME_MAX_LEN;

					if (tch->IsBlockMode(BLOCK_MESSENGER_INVITE) == true)
					{
						ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("상대방이 메신져 추가 거부 상태입니다."));
					}
					else
					{
						// 메신저가 캐릭터단위가 되면서 변경
						MessengerManager::Instance().RequestToAdd(ch, tch);
						//MessengerManager::Instance().AddToList(ch->GetName(), tch->GetName());
					}
				}
			}
			return CHARACTER_NAME_MAX_LEN;

		case MESSENGER_SUBHEADER_CG_REMOVE:
			{
				if (uiBytes < CHARACTER_NAME_MAX_LEN)
					return -1;

				char char_name[CHARACTER_NAME_MAX_LEN + 1];
				strlcpy(char_name, c_pData, sizeof(char_name));
				MessengerManager::Instance().RemoveFromList(ch->GetName(), char_name);
				MessengerManager::Instance().RemoveFromList(char_name, ch->GetName());
			}
			return CHARACTER_NAME_MAX_LEN;

		default:
			sys_err("CInputMain::Messenger : Unknown subheader %d : %s", p->subheader, ch->GetName());
			break;
	}

	return 0;
}

int32_t CInputMain::Shop(LPCHARACTER ch, const char * data, size_t uiBytes)
{
	SPacketCGShop * p = (SPacketCGShop *) data;

	if (uiBytes < sizeof(SPacketCGShop))
		return -1;

	if (g_bIsTestServer)
		sys_log(0, "CInputMain::Shop() ==> SubHeader %d", p->subheader);
	if (!ch)
		return -1;
	if (!ch->GetDesc())
		return -1;

	const char * c_pData = data + sizeof(SPacketCGShop);
	uiBytes -= sizeof(SPacketCGShop);

	switch (p->subheader)
	{
		case SHOP_SUBHEADER_CG_END:
			sys_log(1, "INPUT: %s SHOP: END", ch->GetName());
			CShopManager::Instance().StopShopping(ch);
			return 0;

		case SHOP_SUBHEADER_CG_BUY:
			{
				if (uiBytes < sizeof(uint8_t) + sizeof(uint8_t))
					return -1;

				uint8_t bPos = *(c_pData + 1);
				sys_log(1, "INPUT: %s SHOP: BUY %d", ch->GetName(), bPos);
				CShopManager::Instance().Buy(ch, bPos);
				return (sizeof(uint8_t) + sizeof(uint8_t));
			}

		case SHOP_SUBHEADER_CG_SELL:
			{
				if (uiBytes < sizeof(uint8_t))
					return -1;

				uint8_t pos = *c_pData;

				sys_log(0, "INPUT: %s SHOP: SELL", ch->GetName());
				CShopManager::Instance().Sell(ch, pos);
				return sizeof(uint8_t);
			}

		case SHOP_SUBHEADER_CG_SELL2:
			{
				if (uiBytes < sizeof(uint8_t) + sizeof(uint8_t))
					return -1;

				uint8_t pos = *(c_pData++);
				uint8_t count = *(c_pData);

				sys_log(0, "INPUT: %s SHOP: SELL2", ch->GetName());
				CShopManager::Instance().Sell(ch, pos, count);
				return sizeof(uint8_t) + sizeof(uint8_t);
			}

		default:
			sys_err("CInputMain::Shop : Unknown subheader %d : %s", p->subheader, ch->GetName());
			break;
	}

	return 0;
}

void CInputMain::OnClick(LPCHARACTER ch, const char * data)
{
	SPacketCGOnClick*	pinfo = (SPacketCGOnClick*) data;
	LPCHARACTER			victim;

	if ((victim = CHARACTER_MANAGER::Instance().Find(pinfo->vid)))
		victim->OnClick(ch);
	else if (g_bIsTestServer)
	{
		sys_err("CInputMain::OnClick %s.Click.NOT_EXIST_VID[%d]", ch->GetName(), pinfo->vid);
	}
}

void CInputMain::Exchange(LPCHARACTER ch, const char * data)
{
	if (ch && CBattlegroundManager::Instance().IsEventMap(ch->GetMapIndex()))
		return;

	SPacketCGExchange* pinfo = (SPacketCGExchange*) data;
	LPCHARACTER	to_ch = nullptr;

	if (!ch->CanHandleItem())
		return;

	int32_t iPulse = thecore_pulse(); 
	
	if ((to_ch = CHARACTER_MANAGER::Instance().Find(pinfo->arg1)))
	{
		if (iPulse - to_ch->GetSafeboxLoadTime() < PASSES_PER_SEC(g_nPortalLimitTime))
		{
			to_ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("거래 후 %d초 이내에 창고를 열수 없습니다."), g_nPortalLimitTime);
			return;
		}

		if( true == to_ch->IsDead() )
		{
			return;
		}
	}

	sys_log(0, "CInputMain()::Exchange()  SubHeader %d ", pinfo->subheader);

	if (iPulse - ch->GetSafeboxLoadTime() < PASSES_PER_SEC(g_nPortalLimitTime))
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("거래 후 %d초 이내에 창고를 열수 없습니다."), g_nPortalLimitTime);
		return;
	}


	switch (pinfo->subheader)
	{
		case EXCHANGE_SUBHEADER_CG_START:	// arg1 == vid of target character
			if (!ch->GetExchange())
			{
				if ((to_ch = CHARACTER_MANAGER::Instance().Find(pinfo->arg1)))
				{
					if (iPulse - ch->GetSafeboxLoadTime() < PASSES_PER_SEC(g_nPortalLimitTime))
					{
						ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("창고를 연후 %d초 이내에는 거래를 할수 없습니다."), g_nPortalLimitTime);

						if (g_bIsTestServer)
							ch->ChatPacket(CHAT_TYPE_INFO, "[TestOnly][Safebox]Pulse %d LoadTime %d PASS %d", iPulse, ch->GetSafeboxLoadTime(), PASSES_PER_SEC(g_nPortalLimitTime));
						return; 
					}

					if (iPulse - to_ch->GetSafeboxLoadTime() < PASSES_PER_SEC(g_nPortalLimitTime))
					{
						to_ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("창고를 연후 %d초 이내에는 거래를 할수 없습니다."), g_nPortalLimitTime);


						if (g_bIsTestServer)
							to_ch->ChatPacket(CHAT_TYPE_INFO, "[TestOnly][Safebox]Pulse %d LoadTime %d PASS %d", iPulse, to_ch->GetSafeboxLoadTime(), PASSES_PER_SEC(g_nPortalLimitTime));
						return; 
					}

					if (ch->GetGold() >= GOLD_MAX)
					{	
						ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("액수가 20억 냥을 초과하여 거래를 할수가 없습니다.."));

						sys_err("[OVERFLOG_GOLD] START (%u) id %u name %s ", ch->GetGold(), ch->GetPlayerID(), ch->GetName());
						return;
					}

					if (to_ch->IsPC())
					{
						if (quest::CQuestManager::Instance().GiveItemToPC(ch->GetPlayerID(), to_ch))
						{
							sys_log(0, "Exchange canceled by quest %s %s", ch->GetName(), to_ch->GetName());
							return;
						}
					}


					if (ch->GetMyShop() || ch->IsOpenSafebox() || ch->GetShopOwner() || ch->IsCubeOpen())
					{
						ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("다른 거래중일경우 개인상점을 열수가 없습니다."));
						return;
					}

					ch->ExchangeStart(to_ch);
				}
			}
			break;

		case EXCHANGE_SUBHEADER_CG_ITEM_ADD:	// arg1 == position of item, arg2 == position in exchange window
			if (ch->GetExchange())
			{
				if (ch->GetExchange()->GetCompany()->GetAcceptStatus() != true)
					ch->GetExchange()->AddItem(pinfo->Pos, pinfo->arg2);
			}
			break;

		case EXCHANGE_SUBHEADER_CG_ITEM_DEL:	// arg1 == position of item
			if (ch->GetExchange())
			{
				if (ch->GetExchange()->GetCompany()->GetAcceptStatus() != true)
					ch->GetExchange()->RemoveItem(pinfo->arg1);
			}
			break;

		case EXCHANGE_SUBHEADER_CG_ELK_ADD:	// arg1 == amount of gold
			if (ch->GetExchange())
			{
				const int64_t nTotalGold = static_cast<int64_t>(ch->GetExchange()->GetCompany()->GetOwner()->GetGold()) + static_cast<int64_t>(pinfo->arg1);

				if (GOLD_MAX <= nTotalGold)
				{
					ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("상대방의 총금액이 20억 냥을 초과하여 거래를 할수가 없습니다.."));

					sys_err("[OVERFLOW_GOLD] ELK_ADD (%u) id %u name %s ",
							ch->GetExchange()->GetCompany()->GetOwner()->GetGold(),
							ch->GetExchange()->GetCompany()->GetOwner()->GetPlayerID(),
						   	ch->GetExchange()->GetCompany()->GetOwner()->GetName());

					return;
				}

				if (ch->GetExchange()->GetCompany()->GetAcceptStatus() != true)
					ch->GetExchange()->AddGold(pinfo->arg1);
			}
			break;

		case EXCHANGE_SUBHEADER_CG_ACCEPT:	// arg1 == not used
			if (ch->GetExchange())
			{
				sys_log(0, "CInputMain()::Exchange() ==> ACCEPT "); 
				ch->GetExchange()->Accept(true);
			}

			break;

		case EXCHANGE_SUBHEADER_CG_CANCEL:	// arg1 == not used
			if (ch->GetExchange())
				ch->GetExchange()->Cancel();
			break;
	}
}

void CInputMain::Position(LPCHARACTER ch, const char * data)
{
	SPacketCGPosition* pinfo = (SPacketCGPosition*) data;

	switch (pinfo->position)
	{
		case POSITION_GENERAL:
			ch->Standup();
			break;

		case POSITION_SITTING_CHAIR:
			ch->Sitdown(0);
			break;

		case POSITION_SITTING_GROUND:
			ch->Sitdown(1);
			break;
	}
}

static const int32_t ComboSequenceBySkillLevel[3][8] = 
{
	// 0   1   2   3   4   5   6   7
	{ 14, 15, 16, 17,  0,  0,  0,  0 },
	{ 14, 15, 16, 18, 20,  0,  0,  0 },
	{ 14, 15, 16, 18, 19, 17,  0,  0 },
};

#define COMBO_HACK_ALLOWABLE_MS	100

int32_t ClacValidComboInterval(LPCHARACTER ch, uint8_t bArg)
{
	int32_t nInterval = 300;
	float fAdjustNum = 1.5f; // ??쨔캅 ?짱?첬째징 speed hack 쩔징 째?쨍짰쨈? 째??쨩 쨍쨌짹창 ?짠??. 2013.09.10 CYH

	if (!ch)
	{
		sys_err("ClacValidComboInterval() ch is nullptr");
		return nInterval;
	}

	if (bArg == 13)
	{
		float normalAttackDuration = CMotionManager::Instance().GetNormalAttackDuration(ch->GetRaceNum());
		nInterval = (int32_t)(normalAttackDuration / (((float)ch->GetPoint(POINT_ATT_SPEED) / 100.f) * 900.f) + fAdjustNum);
	}
	else if (bArg == 14)
	{
		nInterval = (int32_t)(ani_combo_speed(ch, 1) / ((ch->GetPoint(POINT_ATT_SPEED) / 100.f) + fAdjustNum));
	}
	else if (bArg > 14 && bArg < 22)
	{
		nInterval = (int32_t)(ani_combo_speed(ch, bArg - 13) / ((ch->GetPoint(POINT_ATT_SPEED) / 100.f) + fAdjustNum));
	}
	else
	{
		sys_err("ClacValidComboInterval() Invalid bArg(%d) ch(%s)", bArg, ch->GetName());
	}

	return nInterval;
}

bool CheckComboHack(LPCHARACTER ch, uint8_t bArg, uint32_t dwTime, bool CheckSpeedHack)
{
	if (!gHackCheckEnable)
		return false;
	
	//	if (IsStun() || IsDead()) return false;
	//	를 추가하는게 맞다고 생각하나,
	//	이미 다른 부분에서 CanMove()는 IsStun(), IsDead()과
	//	독립적으로 체크하고 있기 때문에 수정에 의한 영향을
	//	최소화하기 위해 이렇게 땜빵 코드를 써놓는다.
	if (ch->IsStun() || ch->IsDead())
		return false;
	int32_t ComboInterval = dwTime - ch->GetLastComboTime();
	int32_t HackScalar = 0; // 기본 스칼라 단위 1
#if 0	
	sys_log(0, "COMBO: %s arg:%u seq:%u delta:%d checkspeedhack:%d",
			ch->GetName(), bArg, ch->GetComboSequence(), ComboInterval - ch->GetValidComboInterval(), CheckSpeedHack);
#endif
	// bArg 14 ~ 21번 까지 총 8콤보 가능
	// 1. 첫 콤보(14)는 일정 시간 이후에 반복 가능
	// 2. 15 ~ 21번은 반복 불가능
	// 3. 차례대로 증가한다.
	if (bArg == 14)
	{
		if (CheckSpeedHack && ComboInterval > 0 && ComboInterval < ch->GetValidComboInterval() - COMBO_HACK_ALLOWABLE_MS)
		{
			// FIXME 첫번째 콤보는 이상하게 빨리 올 수가 있어서 300으로 나눔 -_-;
			// 다수의 몬스터에 의해 다운되는 상황에서 공격을 하면
			// 첫번째 콤보가 매우 적은 인터벌로 들어오는 상황 발생.
			// 이로 인해 콤보핵으로 튕기는 경우가 있어 다음 코드 비 활성화.
			//HackScalar = 1 + (ch->GetValidComboInterval() - ComboInterval) / 300;

			//sys_log(0, "COMBO_HACK: 2 %s arg:%u interval:%d valid:%u atkspd:%u riding:%s",
			//		ch->GetName(),
			//		bArg,
			//		ComboInterval,
			//		ch->GetValidComboInterval(),
			//		ch->GetPoint(POINT_ATT_SPEED),
			//	    ch->IsRiding() ? "yes" : "no");
		}

		ch->SetComboSequence(1);
		// 2013 09 11 CYH edited
		//ch->SetValidComboInterval((int32_t) (ani_combo_speed(ch, 1) / (ch->GetPoint(POINT_ATT_SPEED) / 100.f)));
		ch->SetValidComboInterval(ClacValidComboInterval(ch, bArg));
		ch->SetLastComboTime(dwTime);
	}
	else if (bArg > 14 && bArg < 22)
	{
		int32_t idx = MIN(2, ch->GetComboIndex());

		if (ch->GetComboSequence() > 5) // 현재 6콤보 이상은 없다.
		{
			HackScalar = 1;
			ch->SetValidComboInterval(300);
			sys_log(0, "COMBO_HACK: 5 %s combo_seq:%d", ch->GetName(), ch->GetComboSequence());
		}
		// 자객 쌍수 콤보 예외처리
		else if (bArg == 21 &&
				 idx == 2 &&
				 ch->GetComboSequence() == 5 &&
				 ch->GetJob() == JOB_ASSASSIN &&
				 ch->GetWear(WEAR_WEAPON) &&
				 ch->GetWear(WEAR_WEAPON)->GetSubType() == WEAPON_DAGGER)
			ch->SetValidComboInterval(300);
#ifdef ENABLE_WOLFMAN_CHARACTER
		else if (bArg == 21 && idx == 2 && ch->GetComboSequence() == 5 && ch->GetJob() == JOB_WOLFMAN && ch->GetWear(WEAR_WEAPON) && ch->GetWear(WEAR_WEAPON)->GetSubType() == WEAPON_CLAW)
			ch->SetValidComboInterval(300);
#endif
		else if (ComboSequenceBySkillLevel[idx][ch->GetComboSequence()] != bArg)
		{
			HackScalar = 1;
			ch->SetValidComboInterval(300);

			sys_log(0, "COMBO_HACK: 3 %s arg:%u valid:%u combo_idx:%d combo_seq:%d",
					ch->GetName(),
					bArg,
					ComboSequenceBySkillLevel[idx][ch->GetComboSequence()],
					idx,
					ch->GetComboSequence());
		}
		else
		{
			if (CheckSpeedHack && ComboInterval < ch->GetValidComboInterval() - COMBO_HACK_ALLOWABLE_MS)
			{
				HackScalar = 1 + (ch->GetValidComboInterval() - ComboInterval) / 100;

				sys_log(0, "COMBO_HACK: 2 %s arg:%u interval:%d valid:%u atkspd:%u riding:%s",
						ch->GetName(),
						bArg,
						ComboInterval,
						ch->GetValidComboInterval(),
						ch->GetPoint(POINT_ATT_SPEED),
						ch->IsRiding() ? "yes" : "no");
			}

			// 말을 탔을 때는 15번 ~ 16번을 반복한다
			//if (ch->IsHorseRiding())
			if (ch->IsRiding())
				ch->SetComboSequence(ch->GetComboSequence() == 1 ? 2 : 1);
			else
				ch->SetComboSequence(ch->GetComboSequence() + 1);

			// 2013 09 11 CYH edited
			//ch->SetValidComboInterval((int32_t) (ani_combo_speed(ch, bArg - 13) / (ch->GetPoint(POINT_ATT_SPEED) / 100.f)));
			ch->SetValidComboInterval(ClacValidComboInterval(ch, bArg));
			ch->SetLastComboTime(dwTime);
		}
	}
	else if (bArg == 13) // 기본 공격 (둔갑(Polymorph)했을 때 온다)
	{
		if (CheckSpeedHack && ComboInterval > 0 && ComboInterval < ch->GetValidComboInterval() - COMBO_HACK_ALLOWABLE_MS)
		{
			// 다수의 몬스터에 의해 다운되는 상황에서 공격을 하면
			// 첫번째 콤보가 매우 적은 인터벌로 들어오는 상황 발생.
			// 이로 인해 콤보핵으로 튕기는 경우가 있어 다음 코드 비 활성화.
			//HackScalar = 1 + (ch->GetValidComboInterval() - ComboInterval) / 100;

			//sys_log(0, "COMBO_HACK: 6 %s arg:%u interval:%d valid:%u atkspd:%u",
			//		ch->GetName(),
			//		bArg,
			//		ComboInterval,
			//		ch->GetValidComboInterval(),
			//		ch->GetPoint(POINT_ATT_SPEED));
		}

		if (ch->GetRaceNum() >= MAIN_RACE_MAX_NUM)
		{
			// POLYMORPH_BUG_FIX
			
			// DELETEME
			/*
			const CMotion * pkMotion = CMotionManager::Instance().GetMotion(ch->GetRaceNum(), MAKE_MOTION_KEY(MOTION_MODE_GENERAL, MOTION_NORMAL_ATTACK));

			if (!pkMotion)
				sys_err("cannot find motion by race %u", ch->GetRaceNum());
			else
			{
				// 정상적 계산이라면 1000.f를 곱해야 하지만 클라이언트가 애니메이션 속도의 90%에서
				// 다음 애니메이션 블렌딩을 허용하므로 900.f를 곱한다.
				int32_t k = (int32_t) (pkMotion->GetDuration() / ((float) ch->GetPoint(POINT_ATT_SPEED) / 100.f) * 900.f);
				ch->SetValidComboInterval(k);
				ch->SetLastComboTime(dwTime);
			}
			*/
			// 2013 09 11 CYH edited
			//float normalAttackDuration = CMotionManager::Instance().GetNormalAttackDuration(ch->GetRaceNum());
			//int32_t k = (int32_t) (normalAttackDuration / ((float) ch->GetPoint(POINT_ATT_SPEED) / 100.f) * 900.f);			
			//ch->SetValidComboInterval(k);
			ch->SetValidComboInterval(ClacValidComboInterval(ch, bArg));
			ch->SetLastComboTime(dwTime);
			// END_OF_POLYMORPH_BUG_FIX
		}
		else
		{
			// 말이 안되는 콤보가 왔다 해커일 가능성?
			//if (ch->GetDesc()->DelayedDisconnect(number(2, 9)))
			//{
			//	LogManager::Instance().HackLog("Hacker", ch);
			//	sys_log(0, "HACKER: %s arg %u", ch->GetName(), bArg);
			//}

			// 위 코드로 인해, 폴리모프를 푸는 중에 공격 하면,
			// 가끔 핵으로 인식하는 경우가 있다.

			// 자세히 말혀면,
			// 서버에서 poly 0를 처리했지만,
			// 클라에서 그 패킷을 받기 전에, 몹을 공격. <- 즉, 몹인 상태에서 공격.
			//
			// 그러면 클라에서는 서버에 몹 상태로 공격했다는 커맨드를 보내고 (arg == 13)
			//
			// 서버에서는 race는 인간인데 공격형태는 몹인 놈이다! 라고 하여 핵체크를 했다.

			// 사실 공격 패턴에 대한 것은 클라이언트에서 판단해서 보낼 것이 아니라,
			// 서버에서 판단해야 할 것인데... 왜 이렇게 해놨을까...
			// by rtsummit
		}
	}
	else
	{
		// 말이 안되는 콤보가 왔다 해커일 가능성?
		if (ch->GetDesc()->DelayedDisconnect(number(2, 9)))
		{
			LogManager::Instance().HackLog("Hacker", ch);
			sys_log(0, "HACKER: %s arg %u", ch->GetName(), bArg);
		}

		HackScalar = 10;
		ch->SetValidComboInterval(300);
	}

	if (HackScalar)
	{
		// 말에 타거나 내렸을 때 1.5초간 공격은 핵으로 간주하지 않되 공격력은 없게 하는 처리
		if (get_unix_ms_time() - ch->GetLastMountTime() > 1500)
			ch->IncreaseComboHackCount(1 + HackScalar);

		ch->SkipComboAttackByTime(ch->GetValidComboInterval());
	}

	return HackScalar;
}

void CInputMain::Move(LPCHARACTER ch, const char * data)
{
	if (!ch->CanMove())
		return;

	SPacketCGMove* pinfo = (SPacketCGMove*) data;

	if (pinfo->bFunc >= FUNC_MAX_NUM && !(pinfo->bFunc & 0x80))
	{
		sys_err("invalid move type: %s", ch->GetName());
		return;
	}

	LPDESC descStore = ch->GetDesc();


	//enum EMoveFuncType
	//{   
	//	FUNC_WAIT,
	//	FUNC_MOVE,
	//	FUNC_ATTACK,
	//	FUNC_COMBO,
	//	FUNC_MOB_SKILL,
	//	_FUNC_SKILL,
	//	FUNC_MAX_NUM,
	//	FUNC_SKILL = 0x80,
	//};  

	// 텔레포트 핵 체크

	{
		const float fDist = DISTANCE_SQRT((ch->GetX() - pinfo->lX) / 100, (ch->GetY() - pinfo->lY) / 100);
		if (((false == ch->IsRiding() && fDist > 25) || fDist > 60) && OXEVENT_MAP_INDEX != ch->GetMapIndex())
		{
			const GPOS & warpPos = ch->GetWarpPosition();

			if (warpPos.x == 0 && warpPos.y == 0)
				LogManager::Instance().HackLog("Teleport", ch); 
			sys_log(0, "MOVE: %s trying to move too far (dist: %.1fm) Riding(%d)", ch->GetName(), fDist, ch->IsRiding());

			ch->Show(ch->GetMapIndex(), ch->GetX(), ch->GetY(), ch->GetZ());
			ch->Stop();
			return;
		}

		if (ch->IsPC() && ch->IsDead())
		{
			sys_log(0, "MOVE: %s trying to move as dead", ch->GetName());

			ch->Show(ch->GetMapIndex(), ch->GetX(), ch->GetY(), ch->GetZ());
			ch->Stop();
			return;
		}

		//
		// 스피드핵(SPEEDHACK) Check
		//
		uint32_t dwCurTime = get_unix_ms_time();
		// 시간을 Sync하고 7초 후 부터 검사한다. (20090702 이전엔 5초였음)
		if (descStore)
		{
			bool CheckSpeedHack = (false == descStore->IsHandshaking() && dwCurTime - descStore->GetClientTime() > 7000);

		if (CheckSpeedHack)
		{
			int32_t iDelta = (int32_t) (pinfo->dwTime - ch->GetDesc()->GetClientTime());
			int32_t iServerDelta = (int32_t) (dwCurTime - ch->GetDesc()->GetClientTime());

			iDelta = (int32_t) (dwCurTime - pinfo->dwTime);

			// 시간이 늦게간다. 일단 로그만 해둔다. 진짜 이런 사람들이 많은지 체크해야함. TODO
			if (iDelta >= 30000)
			{
				sys_log(0, "SPEEDHACK: slow timer name %s delta %d", ch->GetName(), iDelta);
				ch->GetDesc()->DelayedDisconnect(3);
			}
			// 1초에 20msec 빨리 가는거 까지는 이해한다.
			else if (iDelta < -(iServerDelta / 50))
			{
				sys_log(0, "SPEEDHACK: DETECTED! %s (delta %d %d)", ch->GetName(), iDelta, iServerDelta);
				ch->GetDesc()->DelayedDisconnect(3);
				}
			}
		}

		//
		// 콤보핵 및 스피드핵 체크
		//
		if (pinfo->bFunc == FUNC_COMBO && g_bCheckMultiHack)
		{
			if (descStore)
			{
				bool CheckSpeedHack = (false == descStore->IsHandshaking() && dwCurTime - descStore->GetClientTime() > 7000);
			CheckComboHack(ch, pinfo->bArg, pinfo->dwTime, CheckSpeedHack); // 콤보 체크
			}
		}
	}

	if (pinfo->bFunc == FUNC_MOVE)
	{
		if (ch->GetLimitPoint(POINT_MOV_SPEED) == 0)
			return;

		ch->SetRotation(pinfo->rot);	// 중복 코드
		ch->ResetStopTime();				// ""

		ch->Goto(pinfo->lX, pinfo->lY);
	}
	else
	{
		if (pinfo->bFunc == FUNC_ATTACK || pinfo->bFunc == FUNC_COMBO)
        {
			ch->OnMove(true);
			ch->GetAbuseController()->ReceiveMoveAttackPacket();
		}
		else if (pinfo->bFunc & FUNC_SKILL)
		{
			const int32_t MASK_SKILL_MOTION = 0x7F;
			uint32_t motion = pinfo->bFunc & MASK_SKILL_MOTION;

			if (!ch->IsUsableSkillMotion(motion))
			{
				const char* name = ch->GetName();
				uint32_t job = ch->GetJob();
				uint32_t group = ch->GetSkillGroup();

				char szBuf[256];
				snprintf(szBuf, sizeof(szBuf), "SKILL_HACK: name=%s, job=%d, group=%d, motion=%d", name, job, group, motion);
				LogManager::Instance().HackLog(szBuf, ch->GetDesc()->GetAccountTable().login, ch->GetName(), ch->GetDesc()->GetHostName());
				sys_log(0, "%s", szBuf);

				if (g_bIsTestServer)
					ch->ChatPacket(CHAT_TYPE_INFO, szBuf);

				ch->GetDesc()->DelayedDisconnect(number(2, 8));
			}

			ch->GetAbuseController()->ReceiveMoveAttackPacket();

			ch->OnMove();
		}

		ch->SetRotation(pinfo->rot);	// 중복 코드
		ch->ResetStopTime();				// ""

		ch->Move(pinfo->lX, pinfo->lY);
		ch->Stop();
		ch->StopStaminaConsume();
	}

	SPacketGCMove pack;
	pack.bFunc        = pinfo->bFunc;
	pack.bArg         = pinfo->bArg;
	pack.rot          = pinfo->rot;
	pack.dwVID        = ch->GetVID();
	pack.lX           = pinfo->lX;
	pack.lY           = pinfo->lY;
	pack.dwTime       = pinfo->dwTime;
	pack.dwDuration   = (pinfo->bFunc == FUNC_MOVE) ? ch->GetCurrentMoveDuration() : 0;

	ch->PacketAround(&pack, sizeof(SPacketGCMove), ch);
/*
	if (pinfo->dwTime == 10653691) // 디버거 발견
	{
		if (ch->GetDesc()->DelayedDisconnect(number(15, 30)))
			LogManager::Instance().HackLog("Debugger", ch);

	}
	else if (pinfo->dwTime == 10653971) // Softice 발견
	{
		if (ch->GetDesc()->DelayedDisconnect(number(15, 30)))
			LogManager::Instance().HackLog("Softice", ch);
	}
*/
	/*
	sys_log(0, 
			"MOVE: %s Func:%u Arg:%u Pos:%dx%d Time:%u Dist:%.1f",
			ch->GetName(),
			pinfo->bFunc,
			pinfo->bArg,
			pinfo->lX / 100,
			pinfo->lY / 100,
			pinfo->dwTime,
			fDist);
	*/
}

void CInputMain::Attack(LPCHARACTER ch, const uint8_t header, const char* data)
{
	if (nullptr == ch)
		return;

	struct type_identifier
	{
		uint8_t header;
		uint8_t type;
	};

	const struct type_identifier* const type = reinterpret_cast<const struct type_identifier*>(data);

	if (type->type > 0)
	{
		if (false == ch->CanUseSkill(type->type))
		{
			return;
		}

		switch (type->type)
		{
			case SKILL_GEOMPUNG:
			case SKILL_SANGONG:
			case SKILL_YEONSA:
			case SKILL_KWANKYEOK:
			case SKILL_HWAJO:
			case SKILL_GIGUNG:
			case SKILL_PABEOB:
			case SKILL_MARYUNG:
			case SKILL_TUSOK:
			case SKILL_MAHWAN:
			case SKILL_BIPABU:
			case SKILL_NOEJEON:
			case SKILL_CHAIN:
			case SKILL_HORSE_WILDATTACK_RANGE:
				if (HEADER_CG_SHOOT != type->header)
				{
					if (g_bIsTestServer) 
						ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("Attack :name[%s] Vnum[%d] can't use skill by attack(warning)"), type->type);
					return;
				}
				break;
		}
	}

	switch (header)
	{
		case HEADER_CG_ATTACK:
			{
				if (nullptr == ch->GetDesc())
					return;

				const SPacketCGAttack* packMelee = reinterpret_cast<const SPacketCGAttack*>(data);

				LPCHARACTER	victim = CHARACTER_MANAGER::Instance().Find(packMelee->dwVictimVID);

				if (nullptr == victim || ch == victim)
					return;

				switch (victim->GetCharType())
				{
					case CHAR_TYPE_NPC:
					case CHAR_TYPE_WARP:
					case CHAR_TYPE_GOTO:
						return;
				}

				if (packMelee->bType > 0)
				{
					if (false == ch->CheckSkillHitCount(packMelee->bType, victim->GetVID()))
					{
						return;
					}
				}

				ch->Attack(victim, packMelee->bType);
			}
			break;

		case HEADER_CG_SHOOT:
			{
				const SPacketCGShoot* const packShoot = reinterpret_cast<const SPacketCGShoot*>(data);

				ch->Shoot(packShoot->bType);
			}
			break;
	}
}

int32_t CInputMain::SyncPosition(LPCHARACTER ch, const char * c_pcData, size_t uiBytes)
{
	const SPacketCGSyncPosition* pinfo = reinterpret_cast<const SPacketCGSyncPosition*>( c_pcData );

	if (uiBytes < pinfo->wSize)
		return -1;

	int32_t iExtraLen = pinfo->wSize - sizeof(SPacketCGSyncPosition);

	if (iExtraLen < 0)
	{
		sys_err("invalid packet length (len %d size %u buffer %u)", iExtraLen, pinfo->wSize, uiBytes);
		ch->GetDesc()->SetPhase(PHASE_CLOSE);
		return -1;
	}

	if (0 != (iExtraLen % sizeof(SPacketSyncPositionElement)))
	{
		sys_err("invalid packet length %d (name: %s)", pinfo->wSize, ch->GetName());
		return iExtraLen;
	}

	int32_t iCount = iExtraLen / sizeof(SPacketSyncPositionElement);

	if (iCount <= 0)
		return iExtraLen;

	static const int32_t nCountLimit = 16;

	if( iCount > nCountLimit )
	{
		//LogManager::Instance().HackLog( "SYNC_POSITION_HACK", ch );
		sys_err( "Too many SyncPosition Count(%d) from Name(%s)", iCount, ch ? ch->GetName() : "" );
		//ch->GetDesc()->SetPhase(PHASE_CLOSE);
		//return -1;
		iCount = nCountLimit;
	}

	TEMP_BUFFER tbuf;
	LPBUFFER lpBuf = tbuf.getptr();

	SPacketGCSyncPosition * pHeader = (SPacketGCSyncPosition *) buffer_write_peek(lpBuf);
	buffer_write_proceed(lpBuf, sizeof(SPacketGCSyncPosition));

	auto e = reinterpret_cast<const SPacketSyncPositionElement*>(c_pcData + sizeof(SPacketCGSyncPosition));

	timeval tvCurTime;
	gettimeofday(&tvCurTime, nullptr);

	for (int32_t i = 0; i < iCount; ++i, ++e)
	{
		LPCHARACTER victim = CHARACTER_MANAGER::Instance().Find(e->dwVID);

		if (!victim)
			continue;

		switch (victim->GetCharType())
		{
			case CHAR_TYPE_NPC:
			case CHAR_TYPE_WARP:
			case CHAR_TYPE_GOTO:
				continue;
		}

		// 소유권 검사
		if (!victim->SetSyncOwner(ch))
			continue;

		const float fDistWithSyncOwner = DISTANCE_SQRT( (victim->GetX() - ch->GetX()) / 100, (victim->GetY() - ch->GetY()) / 100 );
		static const float fLimitDistWithSyncOwner = 2500.f + 1000.f;
		// victim과의 거리가 2500 + a 이상이면 핵으로 간주.
		//	거리 참조 : 클라이언트의 __GetSkillTargetRange, __GetBowRange 함수
		//	2500 : 스킬 proto에서 가장 사거리가 긴 스킬의 사거리, 또는 활의 사거리
		//	a = POINT_BOW_DISTANCE 값... 인데 실제로 사용하는 값인지는 잘 모르겠음. 아이템이나 포션, 스킬, 퀘스트에는 없는데...
		//		그래도 혹시나 하는 마음에 버퍼로 사용할 겸해서 1000.f 로 둠...
		if (fDistWithSyncOwner > fLimitDistWithSyncOwner)
		{
			if (ch->GetSyncHackCount() < g_iSyncHackLimitCount)
			{
				ch->UpdateSyncHackCount(victim->IsPC() ? "player" : "monster", true);
				ch->SetNoHackCount(0);
				continue;
			}

			LogManager::Instance().HackLog("SYNC_DIST", ch);

			sys_err("Too far SyncPosition DistanceWithSyncOwner(%f)(%s) from Name(%s) CH(%d,%d) VICTIM(%d,%d) SYNC(%d,%d)",
				fDistWithSyncOwner, victim->GetName(), ch->GetName(), ch->GetX(), ch->GetY(), victim->GetX(), victim->GetY(),
				e->lX, e->lY);

			ch->GetDesc()->SetCloseReason("HACKER_DIST");
			ch->GetDesc()->SetPhase(PHASE_CLOSE);

			return -1;
		}

		const float fDist = DISTANCE_SQRT((victim->GetX() - e->lX) / 100, (victim->GetY() - e->lY) / 100);
		static const int32_t g_lValidSyncInterval = 20 * 1000; // 100ms
		static const int32_t g_minNoHackCount = 10; // 10 valid sync positions reduce 1 in the hack counter
		const timeval & tvLastSyncTime = victim->GetLastSyncTime();
		timeval * tvDiff = timediff(&tvCurTime, &tvLastSyncTime);

		if (tvDiff->tv_sec == 0 && tvDiff->tv_usec < g_lValidSyncInterval)
		{
			if (ch->GetSyncHackCount("player") < g_iSyncHackLimitCount
				&& ch->GetSyncHackCount("monster") < g_iSyncHackLimitCount * 2)
			{
				std::string s_targetType = victim->IsPC() ? "player" : "monster";

				ch->UpdateSyncHackCount(s_targetType, true);
				ch->SetNoHackCount(0);
				continue;
			}
			else
			{
				LogManager::Instance().HackLog("SYNC_INTERVAL", ch);

				sys_err("Too often SyncPosition Interval(%dms)(%s) from Name(%s) VICTIM(%d,%d) SYNC(%d,%d)",
					tvDiff->tv_sec * 1000 + tvDiff->tv_usec / 1000, victim->GetName(), ch->GetName(), victim->GetX(), victim->GetY(),
					e->lX, e->lY);

				ch->GetDesc()->SetCloseReason("HACKER_INTERVAL");
				ch->GetDesc()->SetPhase(PHASE_CLOSE);

				return -1;
			}
		}
		else if (fDist > 25.0f)
		{
			LogManager::Instance().HackLog("SYNC_VDIST", ch);

			sys_err("Too far SyncPosition Distance(%f)(%s) from Name(%s) CH(%d,%d) VICTIM(%d,%d) SYNC(%d,%d)",
				fDist, victim->GetName(), ch->GetName(), ch->GetX(), ch->GetY(), victim->GetX(), victim->GetY(),
				e->lX, e->lY);

			ch->GetDesc()->SetCloseReason("HACKER_VDIST");
			ch->GetDesc()->SetPhase(PHASE_CLOSE);

			return -1;
		}
		else
		{
			//Reduce the counter after some valid syncs (reduce false positives)
			if (ch->GetSyncHackCount("monster") > 0)
			{
				int32_t i_curCount = ch->GetNoHackCount();
				ch->SetNoHackCount(++i_curCount);

				if (i_curCount > g_minNoHackCount) {
					ch->SetNoHackCount(0);
					ch->UpdateSyncHackCount("monster", false);
				}
			}
			victim->SetLastSyncTime(tvCurTime);
			victim->Sync(e->lX, e->lY);
			buffer_write(lpBuf, e, sizeof(SPacketSyncPositionElement));
		}
	}

	if (buffer_size(lpBuf) != sizeof(SPacketGCSyncPosition))
	{
		pHeader->header = HEADER_GC_SYNC_POSITION;
		pHeader->wSize = buffer_size(lpBuf);

		ch->PacketAround(buffer_read_peek(lpBuf), buffer_size(lpBuf), ch);
	}

	return iExtraLen;
}

void CInputMain::FlyTarget(LPCHARACTER ch, const char * pcData, uint8_t bHeader)
{
	SPacketCGFlyTargeting * p = (SPacketCGFlyTargeting *) pcData;
	ch->FlyTarget(p->dwTargetVID, p->lX, p->lY, bHeader);
}

void CInputMain::UseSkill(LPCHARACTER ch, const char * pcData)
{
	SPacketCGUseSkill * p = (SPacketCGUseSkill *) pcData;
	if (!ch->CanUseSkill(p->dwVnum))
		return;

	ch->UseSkill(p->dwVnum, CHARACTER_MANAGER::Instance().Find(p->dwTargetVID));
}

void CInputMain::ScriptButton(LPCHARACTER ch, const void* c_pData)
{
	SPacketCGScriptButton * p = (SPacketCGScriptButton *) c_pData;
	sys_log(0, "QUEST ScriptButton pid %d idx %u", ch->GetPlayerID(), p->idx);

	quest::PC* pc = quest::CQuestManager::Instance().GetPCForce(ch->GetPlayerID());
	if (pc && pc->IsConfirmWait())
		quest::CQuestManager::Instance().Confirm(ch->GetPlayerID(), quest::CONFIRM_TIMEOUT);
	else if (p->idx & 0x80000000)
		quest::CQuestManager::Instance().QuestInfo(ch->GetPlayerID(), p->idx & 0x7fffffff);
	else
		quest::CQuestManager::Instance().QuestButton(ch->GetPlayerID(), p->idx);
}

void CInputMain::ScriptAnswer(LPCHARACTER ch, const void* c_pData)
{
	SPacketCGScriptAnswer * p = (SPacketCGScriptAnswer *) c_pData;
	sys_log(0, "QUEST ScriptAnswer pid %d answer %d", ch->GetPlayerID(), p->answer);

	if (p->answer > 250) // 다음 버튼에 대한 응답으로 온 패킷인 경우
	{
		quest::CQuestManager::Instance().Resume(ch->GetPlayerID());
	}
	else // 선택 버튼을 골라서 온 패킷인 경우
	{
		quest::CQuestManager::Instance().Select(ch->GetPlayerID(),  p->answer);
	}
}


// SCRIPT_SELECT_ITEM
void CInputMain::ScriptSelectItem(LPCHARACTER ch, const void* c_pData)
{
	SPacketCGScriptSelectItem* p = (SPacketCGScriptSelectItem*) c_pData;
	sys_log(0, "QUEST ScriptSelectItem pid %d answer %d", ch->GetPlayerID(), p->selection);
	quest::CQuestManager::Instance().SelectItem(ch->GetPlayerID(), p->selection);
}
// END_OF_SCRIPT_SELECT_ITEM

void CInputMain::QuestInputString(LPCHARACTER ch, const void* c_pData)
{
	SPacketCGQuestInputString * p = (SPacketCGQuestInputString*) c_pData;

	char msg[65];
	strlcpy(msg, p->szString, sizeof(msg));
	sys_log(0, "QUEST InputString pid %u msg %s", ch->GetPlayerID(), msg);

	quest::CQuestManager::Instance().Input(ch->GetPlayerID(), msg);
}

void CInputMain::QuestConfirm(LPCHARACTER ch, const void* c_pData)
{
	SPacketCGQuestConfirm* p = (SPacketCGQuestConfirm*) c_pData;
	LPCHARACTER ch_wait = CHARACTER_MANAGER::Instance().FindByPID(p->requestPID);
	if (p->answer)
		p->answer = quest::CONFIRM_YES;
	sys_log(0, "QuestConfirm from %s pid %u name %s answer %d", ch->GetName(), p->requestPID, (ch_wait)?ch_wait->GetName():"", p->answer);
	if (ch_wait)
	{
		quest::CQuestManager::Instance().Confirm(ch_wait->GetPlayerID(), (quest::EQuestConfirmType) p->answer, ch->GetPlayerID());
	}
}

void CInputMain::Target(LPCHARACTER ch, const char * pcData)
{
	SPacketCGTarget * p = (SPacketCGTarget *) pcData;

	building::LPOBJECT pkObj = building::CManager::Instance().FindObjectByVID(p->dwVID);

	if (pkObj)
	{
		SPacketGCTarget pckTarget;
		pckTarget.dwVID = p->dwVID;
		ch->GetDesc()->Packet(&pckTarget, sizeof(SPacketGCTarget));
	}
	else
		ch->SetTarget(CHARACTER_MANAGER::Instance().Find(p->dwVID));
}

void CInputMain::TargetDrop(LPCHARACTER ch, const char * pcData)
{
	ch->SendTargetDrop();
}

void CInputMain::ChestDropInfo(LPCHARACTER ch, const char* c_pData)
{
	SPacketCGChestDropInfo* p = (SPacketCGChestDropInfo*)c_pData;

	if (p->wInventoryCell >= INVENTORY_MAX_NUM)
		return;

	if (!ch)
		return;

	LPDESC pkDesc = ch->GetDesc();

	if (!pkDesc)
		return;

	LPITEM pkItem = ch->GetInventoryItem(p->wInventoryCell);

	if (!pkItem)
		return;

	std::vector<TChestDropInfoTable> vec_ItemList;
	ITEM_MANAGER::Instance().GetChestItemList(pkItem->GetVnum(), vec_ItemList);

	if (vec_ItemList.size() == 0)
		return;

	SPacketGCChestDropInfo packet;
	packet.wSize = sizeof(packet) + sizeof(TChestDropInfoTable) * vec_ItemList.size();
	packet.dwChestVnum = pkItem->GetVnum();

	pkDesc->BufferedPacket(&packet, sizeof(packet));
	pkDesc->Packet(&vec_ItemList[0], sizeof(TChestDropInfoTable) * vec_ItemList.size());
}

void CInputMain::SafeboxCheckin(LPCHARACTER ch, const char * c_pData)
{
	if (ch && CBattlegroundManager::Instance().IsEventMap(ch->GetMapIndex()))
		return;

	SPacketCGSafeboxCheckin * p = (SPacketCGSafeboxCheckin *)c_pData;

	quest::PC* pc = quest::CQuestManager::Instance().GetPCForce(ch->GetPlayerID());
	if (pc && pc->IsRunning() == true)
		return;

	if (!ch->CanHandleItem())
		return;

	CSafebox * pkSafebox = ch->GetSafebox();
	LPITEM pkItem = ch->GetItem(p->ItemPos);

	if (!pkSafebox || !pkItem)
		return;
		
	if (pkItem->GetType() == ITEM_BELT && pkItem->IsEquipped()) 
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("PLEASE_BE_EMPTY_BELT_INVENTORY_WINDOW"));
		return;
	}

	if (pkItem->GetCell() >= INVENTORY_MAX_NUM && IS_SET(pkItem->GetFlag(), ITEM_FLAG_IRREMOVABLE))
	{
	    ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("<창고> 창고로 옮길 수 없는 아이템 입니다."));
	    return;
	}

	if (!pkSafebox->IsEmpty(p->bSafePos, pkItem->GetSize()))
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("<창고> 옮길 수 없는 위치입니다."));
		return;
	}

	if (pkItem->IsEquipped())
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("Giyili item koyulamaz"));
		return;
	}

	if (pkItem->GetVnum() == UNIQUE_ITEM_SAFEBOX_EXPAND)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("<창고> 이 아이템은 넣을 수 없습니다."));
		return;
	}

	if( IS_SET(pkItem->GetAntiFlag(), ITEM_ANTIFLAG_SAFEBOX) )
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("<창고> 이 아이템은 넣을 수 없습니다."));
		return;
	}

	if (true == pkItem->isLocked())
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("<창고> 이 아이템은 넣을 수 없습니다."));
		return;
	}

#ifdef ENABLE_WEAPON_COSTUME_SYSTEM
	if (pkItem->IsEquipped())
	{
		int32_t iWearCell = pkItem->FindEquipCell(ch);
		if (iWearCell == WEAR_WEAPON)
		{
			LPITEM costumeWeapon = ch->GetWear(WEAR_COSTUME_WEAPON);
			if (costumeWeapon && !ch->UnequipItem(costumeWeapon))
			{
				ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("You cannot unequip the costume weapon. Not enough space."));
				return;
			}
		}
	}
#endif

	if (ITEM_BELT == pkItem->GetType() && CBeltInventoryHelper::IsExistItemInBeltInventory(ch))
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("벨트 인벤토리에 아이템이 존재하면 해제할 수 없습니다."));
		return;
	}

	pkItem->RemoveFromCharacter();
	if (!pkItem->IsDragonSoul())
		ch->SyncQuickslot(QUICKSLOT_TYPE_ITEM, p->ItemPos.cell, 255);
	pkSafebox->Add(p->bSafePos, pkItem);
	
	char szHint[128];
	snprintf(szHint, sizeof(szHint), "%s %u", pkItem->GetName(), pkItem->GetCount());
	LogManager::Instance().ItemLog(ch, pkItem, "SAFEBOX PUT", szHint);
}

void CInputMain::SafeboxCheckout(LPCHARACTER ch, const char * c_pData, bool bMall)
{
	if (ch && CBattlegroundManager::Instance().IsEventMap(ch->GetMapIndex()))
		return;

	SPacketCGSafeboxCheckout * p = (SPacketCGSafeboxCheckout *) c_pData;

	if (!ch->CanHandleItem())
		return;

	CSafebox * pkSafebox;

	if (bMall)
		pkSafebox = ch->GetMall();
	else
		pkSafebox = ch->GetSafebox();

	if (!pkSafebox)
		return;

	LPITEM pkItem = pkSafebox->Get(p->bSafePos);

	if (!pkItem)
		return;
	
	if (!ch->IsEmptyItemGrid(p->ItemPos, pkItem->GetSize()))
		return;

	// 아이템 몰에서 인벤으로 옮기는 부분에서 용혼석 특수 처리
	// (몰에서 만드는 아이템은 item_proto에 정의된대로 속성이 붙기 때문에,
	//  용혼석의 경우, 이 처리를 하지 않으면 속성이 하나도 붙지 않게 된다.)
	if (pkItem->IsDragonSoul())
	{
		if (bMall)
		{
			DSManager::Instance().DragonSoulItemInitialize(pkItem);
		}

		if (DRAGON_SOUL_INVENTORY != p->ItemPos.window_type)
		{
			ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("<창고> 옮길 수 없는 위치입니다."));
			return;
		}
		
		TItemPos DestPos = p->ItemPos;
		if (!DSManager::Instance().IsValidCellForThisItem(pkItem, DestPos))
		{
			int32_t iCell = ch->GetEmptyDragonSoulInventory(pkItem);
			if (iCell < 0)
			{
				ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("<창고> 옮길 수 없는 위치입니다."));
				return ;
			}
			DestPos = TItemPos (DRAGON_SOUL_INVENTORY, iCell);
		}

		pkSafebox->Remove(p->bSafePos);
		pkItem->AddToCharacter(ch, DestPos);
		ITEM_MANAGER::Instance().FlushDelayedSave(pkItem);
	}
	else
	{
		if (DRAGON_SOUL_INVENTORY == p->ItemPos.window_type)
		{
			ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("<창고> 옮길 수 없는 위치입니다."));
			return;
		}

		if (p->ItemPos.IsBeltInventoryPosition() && false == CBeltInventoryHelper::CanMoveIntoBeltInventory(pkItem))
		{
			ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("이 아이템은 벨트 인벤토리로 옮길 수 없습니다."));
			return;
		}

		pkSafebox->Remove(p->bSafePos);
		pkItem->AddToCharacter(ch, p->ItemPos);
		ITEM_MANAGER::Instance().FlushDelayedSave(pkItem);
	}

	uint32_t dwID = pkItem->GetID();
	db_clientdesc->DBPacketHeader(HEADER_GD_ITEM_FLUSH, 0, sizeof(uint32_t));
	db_clientdesc->Packet(&dwID, sizeof(uint32_t));

	char szHint[128];
	snprintf(szHint, sizeof(szHint), "%s %u", pkItem->GetName(), pkItem->GetCount());
	if (bMall)
		LogManager::Instance().ItemLog(ch, pkItem, "MALL GET", szHint);
	else
		LogManager::Instance().ItemLog(ch, pkItem, "SAFEBOX GET", szHint);
}

void CInputMain::SafeboxItemMove(LPCHARACTER ch, const char * data)
{
	SPacketCGSafeboxItemMove * pinfo = (SPacketCGSafeboxItemMove*) data;

	if (!ch->CanHandleItem())
		return;

	if (!ch->GetSafebox())
		return;

	ch->GetSafebox()->MoveItem(pinfo->pos.cell, pinfo->change_pos.cell, pinfo->num);
}

// PARTY_JOIN_BUG_FIX
void CInputMain::PartyInvite(LPCHARACTER ch, const char * c_pData)
{
	if (!ch)
		return; 
		
	if (ch && CBattlegroundManager::Instance().IsEventMap(ch->GetMapIndex()))
		return;

	if (ch->GetArena())
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("대련장에서 사용하실 수 없습니다."));
		return;
	}

	SPacketCGPartyInvite * p = (SPacketCGPartyInvite*) c_pData;

	LPCHARACTER pInvitee = CHARACTER_MANAGER::Instance().Find(p->vid);

	if (!pInvitee || !ch->GetDesc() || !pInvitee->GetDesc() || !pInvitee->IsPC() || !ch->IsPC())
	{
		sys_err("PARTY Cannot find invited character");
		return;
	}

	ch->PartyInvite(pInvitee);
}

void CInputMain::PartyInviteAnswer(LPCHARACTER ch, const char * c_pData)
{
	if (!ch)
		return;
		
	if (ch && CBattlegroundManager::Instance().IsEventMap(ch->GetMapIndex()))
		return;

	if (ch->GetArena())
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("대련장에서 사용하실 수 없습니다."));
		return;
	}

	SPacketCGPartyInviteAnswer * p = (SPacketCGPartyInviteAnswer*) c_pData;

	LPCHARACTER pInviter = CHARACTER_MANAGER::Instance().Find(p->leader_vid);

	// pInviter 가 ch 에게 파티 요청을 했었다.

	if (!pInviter || !pInviter->IsPC()) 
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("<파티> 파티요청을 한 캐릭터를 찾을수 없습니다."));
	else if (!p->accept)
		pInviter->PartyInviteDeny(ch->GetPlayerID());
	else
		pInviter->PartyInviteAccept(ch);
}
// END_OF_PARTY_JOIN_BUG_FIX

void CInputMain::PartySetState(LPCHARACTER ch, const char* c_pData)
{
	if (!CPartyManager::Instance().IsEnablePCParty())
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("<파티> 서버 문제로 파티 관련 처리를 할 수 없습니다."));
		return;
	}

	SPacketCGPartySetState* p = (SPacketCGPartySetState*) c_pData;

	if (!ch->GetParty())
		return;

	if (ch->GetParty()->GetLeaderPID() != ch->GetPlayerID())
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("<파티> 리더만 변경할 수 있습니다."));
		return;
	}

	if (!ch->GetParty()->IsMember(p->dwPID))
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("<파티> 상태를 변경하려는 사람이 파티원이 아닙니다."));
		return;
	}

	uint32_t pid = p->dwPID;
	sys_log(0, "PARTY SetRole pid %d to role %d state %s", pid, p->byState, p->byFlag ? "on" : "off");

	switch (p->byState)
	{
		case PARTY_ROLE_NORMAL:
			break;

		case PARTY_ROLE_ATTACKER: 
		case PARTY_ROLE_TANKER: 
		case PARTY_ROLE_BUFFER:
		case PARTY_ROLE_SKILL_MASTER:
		case PARTY_ROLE_HASTE:
		case PARTY_ROLE_DEFENDER:
			if (ch->GetParty()->SetRole(pid, p->byState, p->byFlag))
			{
				TPacketPartyStateChange pack;
				pack.dwLeaderPID = ch->GetPlayerID();
				pack.dwPID = p->dwPID;
				pack.bRole = p->byState;
				pack.bFlag = p->byFlag;
				db_clientdesc->DBPacket(HEADER_GD_PARTY_STATE_CHANGE, 0, &pack, sizeof(pack));
			}
			/* else
			   ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("<파티> 어태커 설정에 실패하였습니다.")); */
			break;

		default:
			sys_err("wrong byRole in PartySetState Packet name %s state %d", ch->GetName(), p->byState);
			break;
	}
}

void CInputMain::PartyRemove(LPCHARACTER ch, const char* c_pData)
{
	if (!ch->GetParty())
		return;

	if (!CPartyManager::Instance().IsEnablePCParty())
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("<파티> 서버 문제로 파티 관련 처리를 할 수 없습니다."));
		return;
	}
	
	if (ch->GetArena())
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("대련장에서 사용하실 수 없습니다."));
		return;
	}

	if (ch->GetDungeon())
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("<파티> 던전 안에서는 파티에서 추방할 수 없습니다."));
		return;
	}

	SPacketCGPartyRemove* p = (SPacketCGPartyRemove*) c_pData;

	LPPARTY pParty = ch->GetParty();
	if (pParty->GetLeaderPID() == ch->GetPlayerID())
	{
		if (ch->GetDungeon())
		{
			ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("<파티> 던젼내에서는 파티원을 추방할 수 없습니다."));
		}
		else
		{
			
			if(pParty->IsPartyInDungeon(351))
			{
				ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("<파티>던전 안에 파티원이 있어 파티를 해산 할 수 없습니다."));
				return;
			}

			// leader can remove any member
			if (p->pid == ch->GetPlayerID() || pParty->GetMemberCount() == 2)
			{
				// party disband
				CPartyManager::Instance().DeleteParty(pParty);

				if (ch)
					ch->ComputePoints();
			}
			else
			{
				LPCHARACTER B = CHARACTER_MANAGER::Instance().FindByPID(p->pid);
				if (B)
				{
					//pParty->SendPartyRemoveOneToAll(B);
					B->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("<파티> 파티에서 추방당하셨습니다."));
					//pParty->Unlink(B);
					//CPartyManager::Instance().SetPartyMember(B->GetPlayerID(), nullptr);
				}
				pParty->Quit(p->pid);

				if (B)
					B->ComputePoints();
			}
		}
	}
	else
	{
		// otherwise, only remove itself
		if (p->pid == ch->GetPlayerID())
		{
			if (ch->GetDungeon())
			{
				ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("<파티> 던젼내에서는 파티를 나갈 수 없습니다."));
			}
			else
			{
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
					//CPartyManager::Instance().SetPartyMember(ch->GetPlayerID(), nullptr);
				}
			}
		}
		else
		{
			ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("<파티> 다른 파티원을 탈퇴시킬 수 없습니다."));
		}
	}
}

void CInputMain::AnswerMakeGuild(LPCHARACTER ch, const char* c_pData)
{
	SPacketCGAnswerMakeGuild* p = (SPacketCGAnswerMakeGuild*) c_pData;

	if (ch->GetGold() < 200000 || ch->GetLevel() < 40)
		return;

	if (get_unix_time() - ch->GetQuestFlag("guild_manage.new_disband_time") <
			CGuildManager::Instance().GetDisbandDelay())
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("<길드> 해산한 후 %d일 이내에는 길드를 만들 수 없습니다."), 
				quest::CQuestManager::Instance().GetEventFlag("guild_disband_delay"));
		return;
	}

	if (get_unix_time() - ch->GetQuestFlag("guild_manage.new_withdraw_time") <
			CGuildManager::Instance().GetWithdrawDelay())
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("<길드> 탈퇴한 후 %d일 이내에는 길드를 만들 수 없습니다."), 
				quest::CQuestManager::Instance().GetEventFlag("guild_withdraw_delay"));
		return;
	}

	if (ch->GetGuild())
		return;

	CGuildManager& gm = CGuildManager::Instance();

	TGuildCreateParameter cp;
	memset(&cp, 0, sizeof(cp));

	cp.master = ch;
	strlcpy(cp.name, p->guild_name, sizeof(cp.name));

	if (cp.name[0] == 0 || !check_name(cp.name))
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("적합하지 않은 길드 이름 입니다."));
		return;
	}

	uint32_t dwGuildID = gm.CreateGuild(cp);

	if (dwGuildID)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("<길드> [%s] 길드가 생성되었습니다."), cp.name);

		int32_t GuildCreateFee = 200000;

		ch->PointChange(POINT_GOLD, -GuildCreateFee);
		LogManager::Instance().MoneyLog(MONEY_LOG_GUILD, ch->GetPlayerID(), -GuildCreateFee);

		char Log[128];
		snprintf(Log, sizeof(Log), "GUILD_NAME %s MASTER %s", cp.name, ch->GetName());
		LogManager::Instance().CharLog(ch, 0, "MAKE_GUILD", Log);

		// ch->RemoveSpecifyItem(GUILD_CREATE_ITEM_VNUM, 1); // kader kitabi kullanilmiyor
		//ch->SendGuildName(dwGuildID);
	}
	else
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("<길드> 길드 생성에 실패하였습니다."));
}

void CInputMain::PartyUseSkill(LPCHARACTER ch, const char* c_pData)
{
	SPacketCGPartyUseSkill* p = (SPacketCGPartyUseSkill*) c_pData; 
	if (!ch->GetParty())
		return;

	if (ch->GetPlayerID() != ch->GetParty()->GetLeaderPID())
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("<파티> 파티 기술은 파티장만 사용할 수 있습니다."));
		return;
	}

	switch (p->bySkillIndex)
	{
		case PARTY_SKILL_HEAL:
			ch->GetParty()->HealParty();
			break;
		case PARTY_SKILL_WARP:
			{
				LPCHARACTER pch = CHARACTER_MANAGER::Instance().Find(p->dwTargetVID);
				if (pch && pch->IsPC() && ch->GetParty() == pch->GetParty())
				{
					if (ch->GetWarMap())
					{
						ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("Bu haritada yanina oyuncu cekemezsin!"));
						return;
					}
					if (pch->GetMapIndex() != ch->GetMapIndex())
					{
						ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("Yanina getirmek istedigin oyuncu ile ayni haritada olmalisin!"));
						return;
					}
					ch->GetParty()->SummonToLeader(pch->GetPlayerID());
				}
				else
					ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("<파티> 소환하려는 대상을 찾을 수 없습니다."));
			}
			break;
	}
}

void CInputMain::PartyParameter(LPCHARACTER ch, const char * c_pData)
{
	SPacketCGPartyParameter * p = (SPacketCGPartyParameter *) c_pData;

	if (ch->GetParty())
		ch->GetParty()->SetParameter(p->bDistributeMode);
}

size_t GetSubPacketSize(const EPacketCGGuildSubHeaderType& header)
{
	switch (header)
	{
		case GUILD_SUBHEADER_CG_DEPOSIT_MONEY:				return sizeof(int32_t);
		case GUILD_SUBHEADER_CG_WITHDRAW_MONEY:				return sizeof(int32_t);
		case GUILD_SUBHEADER_CG_ADD_MEMBER:					return sizeof(uint32_t);
		case GUILD_SUBHEADER_CG_REMOVE_MEMBER:				return sizeof(uint32_t);
		case GUILD_SUBHEADER_CG_CHANGE_GRADE_NAME:			return 10;
		case GUILD_SUBHEADER_CG_CHANGE_GRADE_AUTHORITY:		return sizeof(uint8_t) + sizeof(uint8_t);
		case GUILD_SUBHEADER_CG_OFFER:						return sizeof(uint32_t);
		case GUILD_SUBHEADER_CG_CHARGE_GSP:					return sizeof(int32_t);
		case GUILD_SUBHEADER_CG_POST_COMMENT:				return 1;
		case GUILD_SUBHEADER_CG_DELETE_COMMENT:				return sizeof(uint32_t);
		case GUILD_SUBHEADER_CG_REFRESH_COMMENT:			return 0;
		case GUILD_SUBHEADER_CG_CHANGE_MEMBER_GRADE:		return sizeof(uint32_t) + sizeof(uint8_t);
		case GUILD_SUBHEADER_CG_USE_SKILL:					return sizeof(TPacketGuildUseSkill);
		case GUILD_SUBHEADER_CG_CHANGE_MEMBER_GENERAL:		return sizeof(uint32_t) + sizeof(uint8_t);
		case GUILD_SUBHEADER_CG_GUILD_INVITE_ANSWER:		return sizeof(uint32_t) + sizeof(uint8_t);
	}

	return 0;
}

int32_t CInputMain::Guild(LPCHARACTER ch, const char * data, size_t uiBytes)
{
	if (uiBytes < sizeof(SPacketCGGuild))
		return -1;

	const SPacketCGGuild* p = reinterpret_cast<const SPacketCGGuild*>(data);
	const char* c_pData = data + sizeof(SPacketCGGuild);

	uiBytes -= sizeof(SPacketCGGuild);

	const EPacketCGGuildSubHeaderType SubHeader = static_cast<EPacketCGGuildSubHeaderType>(p->bySubHeader);
	const size_t SubPacketLen = GetSubPacketSize(SubHeader);

	if (uiBytes < SubPacketLen)
	{
		return -1;
	}

	CGuild* pGuild = ch->GetGuild();

	if (nullptr == pGuild)
	{
		if (SubHeader != GUILD_SUBHEADER_CG_GUILD_INVITE_ANSWER)
		{
			ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("<길드> 길드에 속해있지 않습니다."));
			return SubPacketLen;
		}
	}

	switch (SubHeader)
	{
		case GUILD_SUBHEADER_CG_DEPOSIT_MONEY:
			{
				return SubPacketLen;



				const int32_t gold = MIN(*reinterpret_cast<const int32_t*>(c_pData), __deposit_limit());

				if (gold < 0)
				{
					ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("<길드> 잘못된 금액입니다."));
					return SubPacketLen;
				}

				if (ch->GetGold() < gold)
				{
					ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("<길드> 가지고 있는 돈이 부족합니다."));
					return SubPacketLen;
				}

				pGuild->RequestDepositMoney(ch, gold);
			}
			return SubPacketLen;

		case GUILD_SUBHEADER_CG_WITHDRAW_MONEY:
			{
				return SubPacketLen;



				const int32_t gold = MIN(*reinterpret_cast<const int32_t*>(c_pData), 500000);

				if (gold < 0)
				{
					ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("<길드> 잘못된 금액입니다."));
					return SubPacketLen;
				}

				pGuild->RequestWithdrawMoney(ch, gold);
			}
			return SubPacketLen;

		case GUILD_SUBHEADER_CG_ADD_MEMBER:
			{
				const uint32_t vid = *reinterpret_cast<const uint32_t*>(c_pData);
				LPCHARACTER newmember = CHARACTER_MANAGER::Instance().Find(vid);

				if (!newmember)
				{
					ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("<길드> 그러한 사람을 찾을 수 없습니다."));
					return SubPacketLen;
				}

				if (!ch->IsPC() || !newmember->IsPC())
					return SubPacketLen;

				pGuild->Invite(ch, newmember);
			}
			return SubPacketLen;

		case GUILD_SUBHEADER_CG_REMOVE_MEMBER:
			{
				if (pGuild->UnderAnyWar() != 0)
				{
					ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("<길드> 길드전 중에는 길드원을 탈퇴시킬 수 없습니다."));
					return SubPacketLen;
				}

				const uint32_t pid = *reinterpret_cast<const uint32_t*>(c_pData);
				const TGuildMember* m = pGuild->GetMember(ch->GetPlayerID());

				if (nullptr == m)
					return -1;

				LPCHARACTER member = CHARACTER_MANAGER::Instance().FindByPID(pid);

				if (member)
				{
					if (member->GetGuild() != pGuild)
					{
						ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("<길드> 상대방이 같은 길드가 아닙니다."));
						return SubPacketLen;
					}

					if (!pGuild->HasGradeAuth(m->grade, GUILD_AUTH_REMOVE_MEMBER))
					{
						ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("<길드> 길드원을 강제 탈퇴 시킬 권한이 없습니다."));
						return SubPacketLen;
					}

					member->SetQuestFlag("guild_manage.new_withdraw_time", get_unix_time());
					pGuild->RequestRemoveMember(member->GetPlayerID());
				}
				else
				{
					if (!pGuild->HasGradeAuth(m->grade, GUILD_AUTH_REMOVE_MEMBER))
					{
						ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("<길드> 길드원을 강제 탈퇴 시킬 권한이 없습니다."));
						return SubPacketLen;
					}

					if (pGuild->RequestRemoveMember(pid))
						ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("<길드> 길드원을 강제 탈퇴 시켰습니다."));
					else
						ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("<길드> 그러한 사람을 찾을 수 없습니다."));
				}
			}
			return SubPacketLen;

		case GUILD_SUBHEADER_CG_CHANGE_GRADE_NAME:
			{
				char gradename[GUILD_GRADE_NAME_MAX_LEN + 1];
				strlcpy(gradename, c_pData + 1, sizeof(gradename));

				const TGuildMember * m = pGuild->GetMember(ch->GetPlayerID());

				if (nullptr == m)
					return -1;

				if (m->grade != GUILD_LEADER_GRADE)
				{
					ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("<길드> 직위 이름을 변경할 권한이 없습니다."));
				}
				else if (*c_pData == GUILD_LEADER_GRADE)
				{
					ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("<길드> 길드장의 직위 이름은 변경할 수 없습니다."));
				}
				else if (!check_name(gradename))
				{
					ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("<길드> 적합하지 않은 직위 이름 입니다."));
				}
				else
				{
					pGuild->ChangeGradeName(*c_pData, gradename);
				}
			}
			return SubPacketLen;

		case GUILD_SUBHEADER_CG_CHANGE_GRADE_AUTHORITY:
			{
				const TGuildMember* m = pGuild->GetMember(ch->GetPlayerID());

				if (nullptr == m)
					return -1;

				if (m->grade != GUILD_LEADER_GRADE)
				{
					ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("<길드> 직위 권한을 변경할 권한이 없습니다."));
				}
				else if (*c_pData == GUILD_LEADER_GRADE)
				{
					ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("<길드> 길드장의 권한은 변경할 수 없습니다."));
				}
				else
				{
					pGuild->ChangeGradeAuth(*c_pData, *(c_pData + 1));
				}
			}
			return SubPacketLen;

		case GUILD_SUBHEADER_CG_OFFER:
			{
				uint32_t offer = *reinterpret_cast<const uint32_t*>(c_pData);

				if (pGuild->GetLevel() >= GUILD_MAX_LEVEL)
				{
					ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("<길드> 길드가 이미 최고 레벨입니다."));
				}
				else
				{
					offer /= 100;
					offer *= 100;

					if (pGuild->OfferExp(ch, offer))
					{
						ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("<길드> %u의 경험치를 투자하였습니다."), offer);
					}
					else
					{
						ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("<길드> 경험치 투자에 실패하였습니다."));
					}
				}
			}
			return SubPacketLen;

		case GUILD_SUBHEADER_CG_CHARGE_GSP:
			{
				const int32_t offer = *reinterpret_cast<const int32_t*>(c_pData);
				const int32_t gold = offer * 100;

				if (offer < 0 || gold < offer || gold < 0 || ch->GetGold() < gold)
				{
					ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("<길드> 돈이 부족합니다."));
					return SubPacketLen;
				}

				if (!pGuild->ChargeSP(ch, offer))
				{
					ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("<길드> 용신력 회복에 실패하였습니다."));
				}
			}
			return SubPacketLen;

		case GUILD_SUBHEADER_CG_POST_COMMENT:
			{
				const size_t length = *c_pData;

				if (length > GUILD_COMMENT_MAX_LEN)
				{
					// 잘못된 길이.. 끊어주자.
					sys_err("POST_COMMENT: %s comment too int32_t (length: %u)", ch->GetName(), length);
					ch->GetDesc()->SetPhase(PHASE_CLOSE);
					return -1;
				}

				if (uiBytes < 1 + length)
					return -1;

				const TGuildMember* m = pGuild->GetMember(ch->GetPlayerID());

				if (nullptr == m)
					return -1;

				if (length && !pGuild->HasGradeAuth(m->grade, GUILD_AUTH_NOTICE) && *(c_pData + 1) == '!')
				{
					ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("<길드> 공지글을 작성할 권한이 없습니다."));
				}
				else
				{
					std::string str(c_pData + 1, length);
					pGuild->AddComment(ch, str);
				}

				return (1 + length);
			}

		case GUILD_SUBHEADER_CG_DELETE_COMMENT:
			{
				const uint32_t comment_id = *reinterpret_cast<const uint32_t*>(c_pData);

				pGuild->DeleteComment(ch, comment_id);
			}
			return SubPacketLen;

		case GUILD_SUBHEADER_CG_REFRESH_COMMENT:
			pGuild->RefreshComment(ch);
			return SubPacketLen;

		case GUILD_SUBHEADER_CG_CHANGE_MEMBER_GRADE:
			{
				const uint32_t pid = *reinterpret_cast<const uint32_t*>(c_pData);
				const uint8_t grade = *(c_pData + sizeof(uint32_t));
				const TGuildMember* m = pGuild->GetMember(ch->GetPlayerID());

				if (nullptr == m)
					return -1;

				if (m->grade != GUILD_LEADER_GRADE)
					ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("<길드> 직위를 변경할 권한이 없습니다."));
				else if (ch->GetPlayerID() == pid)
					ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("<길드> 길드장의 직위는 변경할 수 없습니다."));
				else if (grade == 1)
					ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("<길드> 길드장으로 직위를 변경할 수 없습니다."));
				else
					pGuild->ChangeMemberGrade(pid, grade);
			}
			return SubPacketLen;

		case GUILD_SUBHEADER_CG_USE_SKILL:
			{
				const TPacketCGGuildUseSkill* p2 = reinterpret_cast<const TPacketCGGuildUseSkill*>(c_pData);

				pGuild->UseSkill(p2->dwVnum, ch, p2->dwPID);
			}
			return SubPacketLen;

		case GUILD_SUBHEADER_CG_CHANGE_MEMBER_GENERAL:
			{
				const uint32_t pid = *reinterpret_cast<const uint32_t*>(c_pData);
				const uint8_t is_general = *(c_pData + sizeof(uint32_t));
				const TGuildMember* m = pGuild->GetMember(ch->GetPlayerID());

				if (nullptr == m)
					return -1;

				if (m->grade != GUILD_LEADER_GRADE)
				{
					ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("<길드> 장군을 지정할 권한이 없습니다."));
				}
				else
				{
					if (!pGuild->ChangeMemberGeneral(pid, is_general))
					{
						ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("<길드> 더이상 장수를 지정할 수 없습니다."));
					}
				}
			}
			return SubPacketLen;

		case GUILD_SUBHEADER_CG_GUILD_INVITE_ANSWER:
			{
				const uint32_t guild_id = *reinterpret_cast<const uint32_t*>(c_pData);
				const uint8_t accept = *(c_pData + sizeof(uint32_t));

				CGuild * g = CGuildManager::Instance().FindGuild(guild_id);

				if (g && ch && ch->IsPC())
				{
					if (accept)
						g->InviteAccept(ch);
					else
						g->InviteDeny(ch->GetPlayerID());
				}
			}
			return SubPacketLen;

	}

	return 0;
}

void CInputMain::Fishing(LPCHARACTER ch, const char* c_pData)
{
	if (ch && CBattlegroundManager::Instance().IsEventMap(ch->GetMapIndex()))
		return;

	SPacketCGFishing* p = (SPacketCGFishing*)c_pData;
	ch->SetRotation(p->dir * 5);
	ch->fishing();
	return;
}

void CInputMain::ItemGive(LPCHARACTER ch, const char* c_pData)
{
	if (ch && CBattlegroundManager::Instance().IsEventMap(ch->GetMapIndex()))
		return;

	SPacketCGGiveItem* p = (SPacketCGGiveItem*) c_pData;
	LPCHARACTER to_ch = CHARACTER_MANAGER::Instance().Find(p->dwTargetVID);

	if (to_ch)
		ch->GiveItem(to_ch, p->ItemPos);
	else
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("아이템을 건네줄 수 없습니다."));
}

void CInputMain::Hack(LPCHARACTER ch, const char * c_pData)
{
	SPacketCGHack * p = (SPacketCGHack *) c_pData;
	if (!p || p->szBuf == '\0')
		return;

	char buf[sizeof(p->szBuf)];
	strlcpy(buf, p->szBuf, sizeof(buf));

	char __escape_string[1024];
	DBManager::Instance().EscapeString(__escape_string, sizeof(__escape_string), buf, strlen(p->szBuf));

	char __escape_string2[1024];
	if (strlen(p->szInfo))
	{
		char info[sizeof(p->szInfo)];
		strlcpy(info, p->szInfo, sizeof(info));

		DBManager::Instance().EscapeString(__escape_string2, sizeof(__escape_string2), info, strlen(p->szInfo));
	}
	else
		strcpy(__escape_string2, "-");

	char szFinalStr[2096];
	sprintf(szFinalStr, "%s|%s", __escape_string, __escape_string2);
	
	LogManager::Instance().HackLog(szFinalStr, ch);

	//if (ch->GetDesc())
	//	ch->GetDesc()->DelayedDisconnect(3);
}

int32_t CInputMain::MyShop(LPCHARACTER ch, const char * c_pData, size_t uiBytes)
{
	SPacketCGMyShop * p = (SPacketCGMyShop *) c_pData;
	int32_t iExtraLen = p->bCount * sizeof(TShopItemTable);

	if (uiBytes < sizeof(SPacketCGMyShop) + iExtraLen)
		return -1;

	if (ch->GetGold() >= GOLD_MAX)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("소유 돈이 20억냥을 넘어 거래를 핼수가 없습니다."));
		sys_log(0, "MyShop ==> OverFlow Gold id %u name %s ", ch->GetPlayerID(), ch->GetName());
		return (iExtraLen);
	}

	if (ch->IsStun() || ch->IsDead())
		return (iExtraLen);

	if (ch->GetExchange() || ch->IsOpenSafebox() || ch->GetShopOwner() || ch->IsCubeOpen())
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("다른 거래중일경우 개인상점을 열수가 없습니다."));
		return (iExtraLen);
	}

	sys_log(0, "MyShop count %d", p->bCount);
	ch->OpenMyShop(p->szSign, (TShopItemTable *) (c_pData + sizeof(SPacketCGMyShop)), p->bCount);
	return (iExtraLen);
}

void CInputMain::Refine(LPCHARACTER ch, const char* c_pData)
{
	const SPacketCGRefine* p = reinterpret_cast<const SPacketCGRefine*>(c_pData);

	if (ch->GetExchange() || ch->IsOpenSafebox() || ch->GetShopOwner() || ch->GetMyShop() || ch->IsCubeOpen())
	{
		ch->ChatPacket(CHAT_TYPE_INFO,  LC_TEXT("창고,거래창등이 열린 상태에서는 개량을 할수가 없습니다"));
		ch->ClearRefineMode();
		return;
	}

	if (p->type == 255)
	{
		// DoRefine Cancel
		ch->ClearRefineMode();
		return;
	}

	if (p->pos >= INVENTORY_MAX_NUM)
	{
		ch->ClearRefineMode();
		return;
	}

	LPITEM item = ch->GetInventoryItem(p->pos);

	if (!item)
	{
		ch->ClearRefineMode();
		return;
	}

	ch->SetRefineTime();

	if (p->type == REFINE_TYPE_NORMAL)
	{
		sys_log (0, "refine_type_normal");
		ch->DoRefine(item);
	}
	else if (p->type == REFINE_TYPE_SCROLL || p->type == REFINE_TYPE_HYUNIRON || p->type == REFINE_TYPE_MUSIN || p->type == REFINE_TYPE_BDRAGON)
	{
		sys_log (0, "refine_type_scroll, ...");
		ch->DoRefineWithScroll(item);
	}
	else if (p->type == REFINE_TYPE_MONEY_ONLY)
	{
		const LPITEM item2 = ch->GetInventoryItem(p->pos);

		if (nullptr != item2)
		{
			if (500 <= item2->GetRefineSet())
			{
				LogManager::Instance().HackLog("DEVIL_TOWER_REFINE_HACK", ch);
			}
			else
			{
				if (ch->GetQuestFlag("deviltower_zone.can_refine") > 0)
				{
					if (ch->DoRefine(item2, true))
						ch->SetQuestFlag("deviltower_zone.can_refine", ch->GetQuestFlag("deviltower_zone.can_refine") - 1);
				}
				else
				{
					ch->ChatPacket(CHAT_TYPE_INFO, "사귀 타워 완료 보상은 한번까지 사용가능합니다.");
				}
			}
		}
	}

	ch->ClearRefineMode();
}


#ifdef ENABLE_ACCE_SYSTEM
void CInputMain::Acce(LPCHARACTER pkChar, const char* c_pData)
{
	quest::PC * pPC = quest::CQuestManager::Instance().GetPCForce(pkChar->GetPlayerID());
	if (pPC->IsRunning())
		return;

	TPacketAcce * sPacket = (TPacketAcce*) c_pData;
	switch (sPacket->subheader)
	{
	case ACCE_SUBHEADER_CG_CLOSE:
	{
		pkChar->CloseAcce();
	}
	break;
	case ACCE_SUBHEADER_CG_ADD:
	{
		pkChar->AddAcceMaterial(sPacket->tPos, sPacket->bPos);
	}
	break;
	case ACCE_SUBHEADER_CG_REMOVE:
	{
		pkChar->RemoveAcceMaterial(sPacket->bPos);
	}
	break;
	case ACCE_SUBHEADER_CG_REFINE:
	{
		pkChar->RefineAcceMaterials();
	}
	break;
	default:
		break;
	}
}
#endif


int32_t CInputMain::Analyze(LPDESC d, uint8_t bHeader, const char * c_pData)
{
	LPCHARACTER ch;

	if (!(ch = d->GetCharacter()))
	{
		sys_err("no character on desc");
		d->SetPhase(PHASE_CLOSE);
		return (0);
	}

	int32_t iExtraLen = 0;
	
	if (g_bIsTestServer && bHeader != HEADER_CG_CHARACTER_MOVE)
		sys_log(0, "CInputMain::Analyze() ==> Header [%d] ", bHeader);

	switch (bHeader)
	{
		case HEADER_CG_PONG:
			Pong(d); 
			break;

		case HEADER_CG_TIME_SYNC:
			Handshake(d, c_pData);
			break;

		case HEADER_CG_CHAT:
			if (g_bIsTestServer)
			{
				char* pBuf = (char*)c_pData;
				sys_log(0, "%s", pBuf + sizeof(SPacketCGChat));
			}
	
			if ((iExtraLen = Chat(ch, c_pData, m_iBufferLeft)) < 0)
				return -1;
			break;

		case HEADER_CG_WHISPER:
			if ((iExtraLen = Whisper(ch, c_pData, m_iBufferLeft)) < 0)
				return -1;
			break;

		case HEADER_CG_CHARACTER_MOVE:
			Move(ch, c_pData);
			break;

		case HEADER_CG_CHARACTER_POSITION:
			Position(ch, c_pData);
			break;

		case HEADER_CG_ITEM_USE:
			if (!ch->IsObserverMode())
				ItemUse(ch, c_pData);
			break;
			
#ifdef ENABLE_ACCE_SYSTEM
		case HEADER_CG_ACCE:
			Acce(ch, c_pData);
			break;
#endif

		case HEADER_CG_ITEM_DROP2:
			if (!ch->IsObserverMode())
				ItemDrop2(ch, c_pData);
			break;

		case HEADER_CG_ITEM_MOVE:
			if (!ch->IsObserverMode())
				ItemMove(ch, c_pData);
			break;

		case HEADER_CG_ITEM_PICKUP:
			if (!ch->IsObserverMode())
				ItemPickup(ch, c_pData);
			break;

		case HEADER_CG_ITEM_USE_TO_ITEM:
			if (!ch->IsObserverMode())
				ItemToItem(ch, c_pData);
			break;

		case HEADER_CG_GIVE_ITEM:
			if (!ch->IsObserverMode())
				ItemGive(ch, c_pData);
			break;

		case HEADER_CG_EXCHANGE:
			if (!ch->IsObserverMode())
				Exchange(ch, c_pData);
			break;

		case HEADER_CG_ATTACK:
		case HEADER_CG_SHOOT:
			if (!ch->IsObserverMode())
			{
				Attack(ch, bHeader, c_pData);
			}
			break;

		case HEADER_CG_USE_SKILL:
			if (!ch->IsObserverMode())
				UseSkill(ch, c_pData);
			break;

		case HEADER_CG_QUICKSLOT_ADD:
			QuickslotAdd(ch, c_pData);
			break;

		case HEADER_CG_QUICKSLOT_DEL:
			QuickslotDelete(ch, c_pData);
			break;

		case HEADER_CG_QUICKSLOT_SWAP:
			QuickslotSwap(ch, c_pData);
			break;

		case HEADER_CG_SHOP:
			if ((iExtraLen = Shop(ch, c_pData, m_iBufferLeft)) < 0)
				return -1;
			break;

		case HEADER_CG_MESSENGER:
			if ((iExtraLen = Messenger(ch, c_pData, m_iBufferLeft))<0)
				return -1;
			break;

		case HEADER_CG_ON_CLICK:
			OnClick(ch, c_pData);
			break;

		case HEADER_CG_SYNC_POSITION:
			if ((iExtraLen = SyncPosition(ch, c_pData, m_iBufferLeft)) < 0)
				return -1;
			break;

		case HEADER_CG_ADD_FLY_TARGETING:
		case HEADER_CG_FLY_TARGETING:
			FlyTarget(ch, c_pData, bHeader);
			break;

		case HEADER_CG_SCRIPT_BUTTON:
			ScriptButton(ch, c_pData);
			break;

			// SCRIPT_SELECT_ITEM
		case HEADER_CG_SCRIPT_SELECT_ITEM:
			ScriptSelectItem(ch, c_pData);
			break;
			// END_OF_SCRIPT_SELECT_ITEM

		case HEADER_CG_SCRIPT_ANSWER:
			ScriptAnswer(ch, c_pData);
			break;

		case HEADER_CG_QUEST_INPUT_STRING:
			QuestInputString(ch, c_pData);
			break;

		case HEADER_CG_QUEST_CONFIRM:
			QuestConfirm(ch, c_pData);
			break;

		case HEADER_CG_TARGET:
			Target(ch, c_pData);
			break;

		case HEADER_CG_TARGET_DROP:
			TargetDrop(ch, c_pData);
			break;

		case HEADER_CG_CHEST_DROP_INFO:
			ChestDropInfo(ch, c_pData);
			break;

		case HEADER_CG_SAFEBOX_CHECKIN:
			SafeboxCheckin(ch, c_pData);
			break;

		case HEADER_CG_SAFEBOX_CHECKOUT:
			SafeboxCheckout(ch, c_pData, false);
			break;

		case HEADER_CG_SAFEBOX_ITEM_MOVE:
			SafeboxItemMove(ch, c_pData);
			break;

		case HEADER_CG_MALL_CHECKOUT:
			SafeboxCheckout(ch, c_pData, true);
			break;

		case HEADER_CG_PARTY_INVITE:
			PartyInvite(ch, c_pData);
			break;

		case HEADER_CG_PARTY_REMOVE:
			PartyRemove(ch, c_pData);
			break;

		case HEADER_CG_PARTY_INVITE_ANSWER:
			PartyInviteAnswer(ch, c_pData);
			break;

		case HEADER_CG_PARTY_SET_STATE:
			PartySetState(ch, c_pData);
			break;

		case HEADER_CG_PARTY_USE_SKILL:
			PartyUseSkill(ch, c_pData);
			break;

		case HEADER_CG_PARTY_PARAMETER:
			PartyParameter(ch, c_pData);
			break;

		case HEADER_CG_ANSWER_MAKE_GUILD:
			AnswerMakeGuild(ch, c_pData);
			break;

		case HEADER_CG_GUILD:
			if ((iExtraLen = Guild(ch, c_pData, m_iBufferLeft)) < 0)
				return -1;
			break;

		case HEADER_CG_FISHING:
			Fishing(ch, c_pData);
			break;

		case HEADER_CG_HACK:
			Hack(ch, c_pData);
			break;

		case HEADER_CG_MYSHOP:
			if ((iExtraLen = MyShop(ch, c_pData, m_iBufferLeft)) < 0)
				return -1;
			break;

		case HEADER_CG_REFINE:
			Refine(ch, c_pData);
			break;

		case HEADER_CG_DRAGON_SOUL_REFINE:
			{
				SPacketCGDragonSoulRefine* p = reinterpret_cast <SPacketCGDragonSoulRefine*>((void*)c_pData);
				switch(p->bSubType)
				{
				case DS_SUB_HEADER_CLOSE:
					ch->DragonSoul_RefineWindow_Close();
					break;
				case DS_SUB_HEADER_DO_UPGRADE:
					{
						DSManager::Instance().DoRefineGrade(ch, p->ItemGrid);
					}
					break;
				case DS_SUB_HEADER_DO_IMPROVEMENT:
					{
						DSManager::Instance().DoRefineStep(ch, p->ItemGrid);
					}
					break;
				case DS_SUB_HEADER_DO_REFINE:
					{
						DSManager::Instance().DoRefineStrength(ch, p->ItemGrid);
					}
					break;
				}
			}

			break;
	}
	return (iExtraLen);
}
