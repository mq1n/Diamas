#include "stdafx.h"
#include "constants.h"
#include "config.h"
#include "quest_manager.h"
#include "start_position.h"
#include "packet.h"
#include "buffer_manager.h"
#include "log.h"
#include "char.h"
#include "char_manager.h"
#include "ox_event.h"
#include "desc.h"
#include "affect.h"

bool COXEventManager::Initialize()
{
	m_timedEvent = nullptr;
	m_map_char.clear();
	m_map_attender.clear();
	m_vec_quiz.clear();
	m_map_ban.clear();

	SetStatus(OXEVENT_FINISH);

	return true;
}

void COXEventManager::Destroy()
{
	CloseEvent();

	m_map_char.clear();
	m_map_attender.clear();
	m_vec_quiz.clear();
	m_map_ban.clear();

	SetStatus(OXEVENT_FINISH);
}

OXEventStatus COXEventManager::GetStatus()
{
	uint8_t ret = quest::CQuestManager::instance().GetEventFlag("oxevent_status");

	switch (ret)
	{
		case 0 :
			return OXEVENT_FINISH;
			
		case 1 :
			return OXEVENT_OPEN;
			
		case 2 :
			return OXEVENT_CLOSE;
			
		case 3 :
			return OXEVENT_QUIZ;
			
		default :
			return OXEVENT_ERR;
	}

	return OXEVENT_ERR;
}

void COXEventManager::SetStatus(OXEventStatus status)
{
	uint8_t val = 0;
	
	switch (status)
	{
		case OXEVENT_OPEN :
			val = 1;
			break;
			
		case OXEVENT_CLOSE :
			val = 2;
			break;
			
		case OXEVENT_QUIZ :
			val = 3;
			break;
		
		case OXEVENT_FINISH :
		case OXEVENT_ERR :
		default :
			val = 0;
			break;
	}
	quest::CQuestManager::instance().RequestSetEventFlag("oxevent_status", val);
}


void COXEventManager::ProcessPlayer(LPCHARACTER pChar)
{
	if (pChar->IsHorseRiding() || pChar->GetMountVnum())
	{
		pChar->RemoveAffect(AFFECT_MOUNT);
		pChar->RemoveAffect(AFFECT_MOUNT_BONUS);

		pChar->StopRiding();
	}

	if (pChar->GetHorse())
		pChar->HorseSummon(false);

	if (pChar->IsPolymorphed()) 
	{
		pChar->SetPolymorph(0);
		pChar->RemoveAffect(AFFECT_POLYMORPH);
	}

	pChar->RemoveBadAffect();
	pChar->RemoveGoodAffect();
}


bool COXEventManager::CheckIpAddress(LPCHARACTER ch)
{
	if (!ch || !ch->GetDesc())
		return false;

	const char * c_szCurrIpAddress = ch->GetDesc()->GetHostName();
	if (!c_szCurrIpAddress || !*c_szCurrIpAddress || !strlen(c_szCurrIpAddress))
		return false;

	for (auto it = m_map_attender.begin(); it != m_map_attender.end(); ++it)
	{
		LPCHARACTER tch = CHARACTER_MANAGER::Instance().FindByPID(it->second);
		if (!tch || !tch->GetDesc())
			continue;

		if (!strcmp(c_szCurrIpAddress, tch->GetDesc()->GetHostName()))
		{
			ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("Multi IP detected"));
			LogManager::Instance().HackLog("MULTI_IP_OX", ch);
			ch->GoHome();
			return false;
		}
	}

	return true;
}


void COXEventManager::RemoveFromAttenderList(uint32_t dwPID)
{
	if (m_map_attender.find(dwPID) == m_map_attender.end())
		return;
	m_map_attender.erase(dwPID);
}


bool COXEventManager::IsBanned(LPCHARACTER ch) 
{
	return m_map_ban.find(ch->GetAID()) != m_map_ban.end();
}
void COXEventManager::AddBan(LPCHARACTER ch) 
{
	m_map_ban.insert(std::make_pair(ch->GetAID(), ch->GetAID()));
}
void COXEventManager::ClearBanList()
{
	m_map_ban.clear();
}


bool COXEventManager::Enter(LPCHARACTER pkChar)
{
	if (GetStatus() == OXEVENT_FINISH && !pkChar->IsGM())
	{
		sys_log(0, "OXEVENT : map finished. but char enter. %s", pkChar->GetName());
		return false;
	}

	if (pkChar->GetLevel() < 15)
		return false;

	ProcessPlayer(pkChar);

	GPOS pos = pkChar->GetXYZ();

	if (pos.x == 896500 && pos.y == 24600)
	{
		return EnterAttender(pkChar);
	}
	else if ((pos.x == 896300 && pos.y == 28900) || pkChar->IsGM())
	{
		return EnterAudience(pkChar);
	}

	sys_log(0, "OXEVENT : wrong pos enter %d %d", pos.x, pos.y);
	return false;
}

bool COXEventManager::EnterAttender(LPCHARACTER pkChar)
{
	if (GetStatus() != OXEVENT_OPEN && !pkChar->IsGM())
	{
		sys_err("cannot join ox %u %s (oxevent is not open)", pkChar->GetPlayerID(), pkChar->GetName());
		AddBan(pkChar);
		return false;
	}

	uint32_t pid = pkChar->GetPlayerID();

	if (CheckIpAddress(pkChar))
	{
		m_map_char.insert(std::make_pair(pid, pid));
		m_map_attender.insert(std::make_pair(pid, pid));
		return true;
	}

	return false;
}


bool COXEventManager::EnterAudience(LPCHARACTER pkChar)
{
	uint32_t pid = pkChar->GetPlayerID();

	m_map_char.insert(std::make_pair(pid, pid));

	return true;
}


#if 0
bool AddQuizEx(uint8_t level, const char* pszQuestion, bool answer)
{
	if (m_vec_quiz.size() < (size_t)level + 1)
		m_vec_quiz.resize(level + 1);

	struct tag_Quiz tmpQuiz;

	tmpQuiz.level = level;
	strlcpy(tmpQuiz.Quiz, pszQuestion, sizeof(tmpQuiz.Quiz));
	tmpQuiz.answer = answer;

	m_vec_quiz[level].push_back(tmpQuiz);
	return true;
}

void CreateQuizList()
{
	const char c_szQuizArray[200 /* element count of array */][3 /* sub element count on elements */][255 /* max length of sub elements */] = {
		{ "1", "kirmizi orman perisinin seviyesi 77'dir?", "1" },
		{ "1", "Olum metni 70 leveldir?", "0" },
		{ "1", "1.Koyde Siyah Metin duser?", "1" },
	};

	int32_t i;
	for (i = 0; i < _countof(c_szQuizArray); i++) {
		sys_log(0, "Reading-%d: %s -> %s -> %s", i, c_szQuizArray[i][0], c_szQuizArray[i][1], c_szQuizArray[i][2]);

		int32_t iLevelOfQuestion = atoi(c_szQuizArray[i][0]);
		const char* c_szQuestion = c_szQuizArray[i][1];
		bool bAnswer = atoi(c_szQuizArray[i][2]) ? true : false;

		sys_log(0, "Processed-%d: %d -> %s -> %d", i, iLevelOfQuestion, c_szQuestion, bAnswer ? 1 : 0);

		if (AddQuizEx(iLevelOfQuestion, c_szQuestion, bAnswer) == false)
			sys_err("OXEVENT : Cannot add quiz. %d) %s", i, c_szQuestion);
	}

	sys_log(0, "Completed: %d", i);
}
#endif

bool COXEventManager::AddQuiz(uint8_t level, const char* pszQuestion, bool answer)
{
	if (m_vec_quiz.size() < (size_t) level + 1)
		m_vec_quiz.resize(level + 1);

	struct tag_Quiz tmpQuiz;

	tmpQuiz.level = level;
	strlcpy(tmpQuiz.Quiz, pszQuestion, sizeof(tmpQuiz.Quiz));
	tmpQuiz.answer = answer;

	m_vec_quiz[level].push_back(tmpQuiz);
	return true;
}

bool COXEventManager::ShowQuizList(LPCHARACTER pkChar)
{
	int32_t c = 0;
	
	for (size_t i = 0; i < m_vec_quiz.size(); ++i)
	{
		for (size_t j = 0; j < m_vec_quiz[i].size(); ++j, ++c)
		{
			pkChar->ChatPacket(CHAT_TYPE_INFO, "%d %s %s", m_vec_quiz[i][j].level, m_vec_quiz[i][j].Quiz, m_vec_quiz[i][j].answer ? LC_TEXT("참") : LC_TEXT("거짓"));
		}
	}

	pkChar->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("총 퀴즈 수: %d"), c);	
	return true;
}

void COXEventManager::ClearQuiz()
{
	for (uint32_t i = 0; i < m_vec_quiz.size(); ++i)
	{
		m_vec_quiz[i].clear();
	}

	m_vec_quiz.clear();
}

EVENTINFO(OXEventInfoData)
{
	bool answer;

	OXEventInfoData()
	: answer( false )
	{
	}
};

EVENTFUNC(oxevent_timer)
{
	static uint8_t flag = 0;
	OXEventInfoData* info = dynamic_cast<OXEventInfoData*>(event->info);

	if ( info == nullptr )
	{
		sys_err( "oxevent_timer> <Factor> Null pointer" );
		return 0;
	}

	switch (flag)
	{
		case 0:
			SendNoticeMap(LC_TEXT("10초뒤 판정하겠습니다."), OXEVENT_MAP_INDEX, true);
			flag++;
			return PASSES_PER_SEC(10);
			
		case 1:
			SendNoticeMap(LC_TEXT("정답은"), OXEVENT_MAP_INDEX, true);

			if (info->answer == true)
			{
				COXEventManager::instance().CheckAnswer(true);
				SendNoticeMap(LC_TEXT("O 입니다"), OXEVENT_MAP_INDEX, true);
			}
			else
			{
				COXEventManager::instance().CheckAnswer(false);
				SendNoticeMap(LC_TEXT("X 입니다"), OXEVENT_MAP_INDEX, true);
			}

			SendNoticeMap(LC_TEXT("5초 뒤 틀리신 분들을 바깥으로 이동 시키겠습니다."), OXEVENT_MAP_INDEX, true);

			flag++;
			return PASSES_PER_SEC(5);

		case 2:
			COXEventManager::instance().WarpToAudience();
			COXEventManager::instance().SetStatus(OXEVENT_CLOSE);
			SendNoticeMap(LC_TEXT("다음 문제 준비해주세요."), OXEVENT_MAP_INDEX, true);
			flag = 0;
			break;
	}
	return 0;
}

bool COXEventManager::Quiz(uint8_t level, int32_t timelimit)
{
	if (m_vec_quiz.size() == 0)
		return false;
	if (level > m_vec_quiz.size())
		level = static_cast<uint8_t>(m_vec_quiz.size() - 1);
	if (m_vec_quiz[level].size() <= 0)
		return false;

	if (timelimit < 0) timelimit = 30;

	int32_t idx = number(0, m_vec_quiz[level].size()-1);

	SendNoticeMap(LC_TEXT("문제 입니다."), OXEVENT_MAP_INDEX, true);
	SendNoticeMap(m_vec_quiz[level][idx].Quiz, OXEVENT_MAP_INDEX, true);
	SendNoticeMap(LC_TEXT("맞으면 O, 틀리면 X로 이동해주세요"), OXEVENT_MAP_INDEX, true);

	if (m_timedEvent != nullptr) {
		event_cancel(&m_timedEvent);
	}

	OXEventInfoData* answer = AllocEventInfo<OXEventInfoData>();

	answer->answer = m_vec_quiz[level][idx].answer;

	timelimit -= 15;
	m_timedEvent = event_create(oxevent_timer, answer, PASSES_PER_SEC(timelimit));

	SetStatus(OXEVENT_QUIZ);

	m_vec_quiz[level].erase(m_vec_quiz[level].begin()+idx);
	return true;
}

bool COXEventManager::CheckAnswer(bool answer)
{
	if (m_map_attender.size() <= 0) return true;
	
	auto iter = m_map_attender.begin();
	auto iter_tmp = iter;
	
	m_map_miss.clear();

	int32_t rect[4];
	if (answer != true)
	{
		rect[0] = 892600;
		rect[1] = 22900;
		rect[2] = 896300;
		rect[3] = 26400;
	}
	else
	{
		rect[0] = 896600;
		rect[1] = 22900;
		rect[2] = 900300;
		rect[3] = 26400;
	}
	
	LPCHARACTER pkChar = nullptr;
	GPOS pos;
	for (; iter != m_map_attender.end();)
	{
		pkChar = CHARACTER_MANAGER::instance().FindByPID(iter->second);
		if (pkChar != nullptr)
		{
			pos = pkChar->GetXYZ();

			if (pos.x < rect[0] || pos.x > rect[2] || pos.y < rect[1] || pos.y > rect[3])
			{
				pkChar->EffectPacket(SE_FAIL);
				pkChar->ChatPacket(CHAT_TYPE_BIG_NOTICE, LC_TEXT("Elendin!"));
				pkChar->AutoGiveItem(50011);

				iter_tmp = iter;
				iter++;
				m_map_attender.erase(iter_tmp);
				m_map_miss.insert(std::make_pair(pkChar->GetPlayerID(), pkChar->GetPlayerID()));
			}
			else
			{
				pkChar->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("정답입니다!"));
				// pkChar->CreateFly(number(FLY_FIREWORK1, FLY_FIREWORK6), pkChar);
				char chatbuf[256];
				int32_t len = snprintf(chatbuf, sizeof(chatbuf), 
						"%s %u %u", number(0, 1) == 1 ? "cheer1" : "cheer2", (uint32_t)pkChar->GetVID(), 0);

				// 리턴값이 sizeof(chatbuf) 이상일 경우 truncate되었다는 뜻..
				if (len < 0 || len >= (int32_t) sizeof(chatbuf))
					len = sizeof(chatbuf) - 1;

				// \0 문자 포함
				++len;

				SPacketGCChat pack_chat;
				pack_chat.header = HEADER_GC_CHAT;
				pack_chat.size = sizeof(SPacketGCChat) + len;
				pack_chat.type = CHAT_TYPE_COMMAND;
				pack_chat.dwVID = 0;

				TEMP_BUFFER buf;
				buf.write(&pack_chat, sizeof(SPacketGCChat));
				buf.write(chatbuf, len);

				pkChar->PacketAround(buf.read_peek(), buf.size());
				pkChar->EffectPacket(SE_SUCCESS);

				++iter;
			}
		}
		else
		{
			auto err = m_map_char.find(iter->first);
			if (err != m_map_char.end()) m_map_char.erase(err);

			auto err2 = m_map_miss.find(iter->first);
			if (err2 != m_map_miss.end()) m_map_miss.erase(err2);

			iter_tmp = iter;
			++iter;
			m_map_attender.erase(iter_tmp);
		}
	}
	return true;
}

void COXEventManager::WarpToAudience()
{
	if (m_map_miss.size() <= 0) return;

	auto iter = m_map_miss.begin();
	LPCHARACTER pkChar = nullptr;
	
	for (; iter != m_map_miss.end(); ++iter)
	{
		pkChar = CHARACTER_MANAGER::instance().FindByPID(iter->second);

		if (pkChar != nullptr)
		{
			switch ( number(0, 3))
			{
				case 0 : pkChar->Show(OXEVENT_MAP_INDEX, 896300, 28900); break;
				case 1 : pkChar->Show(OXEVENT_MAP_INDEX, 890900, 28100); break;
				case 2 : pkChar->Show(OXEVENT_MAP_INDEX, 896600, 20500); break;
				case 3 : pkChar->Show(OXEVENT_MAP_INDEX, 902500, 28100); break;
				default : pkChar->Show(OXEVENT_MAP_INDEX, 896300, 28900); break;
			}
		}
	}

	m_map_miss.clear();
}

bool COXEventManager::CloseEvent()
{
	if (m_timedEvent != nullptr) {
		event_cancel(&m_timedEvent);
	}

	auto iter = m_map_char.begin();

	LPCHARACTER pkChar = nullptr;
	for (; iter != m_map_char.end(); ++iter)
	{
		pkChar = CHARACTER_MANAGER::instance().FindByPID(iter->second);

		if (pkChar != nullptr)
			pkChar->WarpSet(EMPIRE_START_X(pkChar->GetEmpire()), EMPIRE_START_Y(pkChar->GetEmpire()));
	}

	Initialize();
	return true;
}

bool COXEventManager::LogWinner()
{
	auto iter = m_map_attender.begin();
	
	for (; iter != m_map_attender.end(); ++iter)
	{
		LPCHARACTER pkChar = CHARACTER_MANAGER::instance().FindByPID(iter->second);

		if (pkChar)
			LogManager::instance().CharLog(pkChar, 0, "OXEVENT", "LastManStanding");
	}

	return true;
}

bool COXEventManager::GiveItemToAttender(uint32_t dwItemVnum, uint8_t count)
{
	auto iter = m_map_attender.begin();

	for (; iter != m_map_attender.end(); ++iter)
	{
		LPCHARACTER pkChar = CHARACTER_MANAGER::instance().FindByPID(iter->second);

		if (pkChar)
		{
			pkChar->AutoGiveItem(dwItemVnum, count);
			LogManager::instance().ItemLog(pkChar->GetPlayerID(), 0, count, dwItemVnum, "OXEVENT_REWARD", "", pkChar->GetDesc()->GetHostName(), dwItemVnum);
		}
	}

	return true;
}

