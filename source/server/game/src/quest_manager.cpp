#include "stdafx.h"
#include "quest_manager.h"
#include "constants.h"
#include "buffer_manager.h"
#include "desc_client.h"
#include "desc_manager.h"
#include "char.h"
#include "char_manager.h"
#include "text_file_loader.h"
#include "lzo_manager.h"
#include "item.h"
#include "config.h"
#include "xmas_event.h"
#include "target.h"
#include "party.h"
#include "locale_service.h"
#include "dungeon.h"

uint32_t g_GoldDropTimeLimitValue = 0;
uint32_t g_ItemDropTimeLimitValue = 0;
uint32_t g_BoxUseTimeLimitValue = 0;
uint32_t g_BuySellTimeLimitValue = 0;

bool g_NoDropMetinStone = false;
bool g_NoMountAtGuildWar = false;
bool g_NoPotionsOnPVP = false;

extern bool DropEvent_CharStone_SetValue(const std::string& name, int32_t value);
extern bool DropEvent_RefineBox_SetValue (const std::string& name, int32_t value);

namespace quest
{

	CQuestManager::CQuestManager() :
	m_pSelectedDungeon(nullptr), m_dwServerTimerArg(0), m_iRunningEventIndex(0), L(nullptr), m_bNoSend (false),
		m_CurrentRunningState(nullptr), m_pCurrentCharacter(nullptr), m_pCurrentNPCCharacter(nullptr), m_pCurrentPartyMember(nullptr),
		m_pCurrentPC(nullptr), m_bQuestInfoFlag(false),  m_iCurrentSkin(0), m_bError(false), m_pOtherPCBlockRootPC(nullptr)
	{
	}

	CQuestManager::~CQuestManager()
	{
		Destroy();
	}

	void CQuestManager::Destroy()
	{
		if (L)
		{
			lua_close(L);
			L = nullptr;
		}
	}	

	bool CQuestManager::Initialize()
	{
		if (g_bAuthServer)
			return true;

		if (!InitializeLua())
			return false;

		m_pSelectedDungeon = nullptr;

		m_mapEventName.insert({"click", QUEST_CLICK_EVENT});		// NPC를 클릭
		m_mapEventName.insert({"kill", QUEST_KILL_EVENT});		// Mob을 사냥
		m_mapEventName.insert({"timer", QUEST_TIMER_EVENT});		// 미리 지정해둔 시간이 지남
		m_mapEventName.insert({"levelup", QUEST_LEVELUP_EVENT});	// 레벨업을 함
		m_mapEventName.insert({"login", QUEST_LOGIN_EVENT});		// 로그인 시
		m_mapEventName.insert({"logout", QUEST_LOGOUT_EVENT});		// 로그아웃 시
		m_mapEventName.insert({"button", QUEST_BUTTON_EVENT});		// 퀘스트 버튼을 누름
		m_mapEventName.insert({"info", QUEST_INFO_EVENT});		// 퀘스트 정보창을 염
		m_mapEventName.insert({"chat", QUEST_CHAT_EVENT});		// 특정 키워드로 대화를 함
		m_mapEventName.insert({"in", QUEST_ATTR_IN_EVENT});		// 맵의 특정 속성에 들어감
		m_mapEventName.insert({"out", QUEST_ATTR_OUT_EVENT});		// 맵의 특정 속성에서 나옴
		m_mapEventName.insert({"use", QUEST_ITEM_USE_EVENT});		// 퀘스트 아이템을 사용
		m_mapEventName.insert({"server_timer", QUEST_SERVER_TIMER_EVENT});	// 서버 타이머 (아직 테스트 안됐음)
		m_mapEventName.insert({"enter", QUEST_ENTER_STATE_EVENT});	// 현재 스테이트가 됨
		m_mapEventName.insert({"leave", QUEST_LEAVE_STATE_EVENT});	// 현재 스테이트에서 다른 스테이트로 바뀜
		m_mapEventName.insert({"letter", QUEST_LETTER_EVENT});		// 로긴 하거나 스테이트가 바껴 새로 정보를 세팅해줘야함
		m_mapEventName.insert({"take", QUEST_ITEM_TAKE_EVENT});	// 아이템을 받음
		m_mapEventName.insert({"target", QUEST_TARGET_EVENT});		// 타겟
		m_mapEventName.insert({"party_kill", QUEST_PARTY_KILL_EVENT});	// 파티 멤버가 몬스터를 사냥 (리더에게 옴)
		m_mapEventName.insert({"unmount", QUEST_UNMOUNT_EVENT});
		m_mapEventName.insert({"pick", QUEST_ITEM_PICK_EVENT});	// 떨어져있는 아이템을 습득함.
		m_mapEventName.insert({"sig_use", QUEST_SIG_USE_EVENT});		// Special item group에 속한 아이템을 사용함.
		m_mapEventName.insert({"item_informer", QUEST_ITEM_INFORMER_EVENT});	// 독일선물기능테스트
		m_mapEventName.insert({"die", QUEST_DIE_EVENT});

		m_bNoSend = false;

		m_iCurrentSkin = QUEST_SKIN_NORMAL;

		{
			std::ifstream inf((g_stQuestDir + "/questnpc.txt").c_str());
			int32_t line = 0;

			if (!inf.is_open())
				sys_err( "QUEST Cannot open 'questnpc.txt'");
			else
				sys_log(0, "QUEST can open 'questnpc.txt' (%s)", g_stQuestDir.c_str() );

			while (1)
			{
				uint32_t vnum;

				inf >> vnum;

				line++;

				if (inf.fail())
					break;

				std::string s;
				getline(inf, s);
				uint32_t li = 0, ri = s.size()-1;
				while (li < s.size() && isspace(s[li])) li++;
				while (ri > 0 && isspace(s[ri])) ri--;

				if (ri < li) 
				{
					sys_err("QUEST questnpc.txt:%d:npc name error",line);
					continue;
				}

				s = s.substr(li, ri-li+1);

				int32_t	n = 0;
				str_to_number(n, s.c_str());
				if (n)
					continue;

				if(vnum == 0)
					continue;
				//cout << '-' << s << '-' << endl;
				if ( g_bIsTestServer )
					sys_log(0, "QUEST reading script of %s(%d)", s.c_str(), vnum);
				m_mapNPC[vnum].Set(vnum, s);
				m_mapNPCNameID[s] = vnum;
			}

			// notarget quest
			m_mapNPC[0].Set(0, "notarget");
		}

		SetEventFlag("guild_withdraw_delay", 1);
		SetEventFlag("guild_disband_delay", 1);
		return true;
	}

	uint32_t CQuestManager::FindNPCIDByName(const std::string& name)
	{
		auto it = m_mapNPCNameID.find(name);
		return it != m_mapNPCNameID.end() ? it->second : 0;
	}

	void CQuestManager::SelectItem(uint32_t pc, uint32_t selection)
	{
		PC* pPC = GetPC(pc);
		if (pPC && pPC->IsRunning() && pPC->GetRunningQuestState()->suspend_state == SUSPEND_STATE_SELECT_ITEM)
		{
			pPC->SetSendDoneFlag();
			pPC->GetRunningQuestState()->args=1;
			lua_pushnumber(pPC->GetRunningQuestState()->co,selection);

			if (!RunState(*pPC->GetRunningQuestState()))
			{
				CloseState(*pPC->GetRunningQuestState());
				pPC->EndRunning();
			}
		}
	}

	void CQuestManager::Confirm(uint32_t pc, EQuestConfirmType confirm, uint32_t pc2)
	{
		PC* pPC = GetPC(pc);
		if (!pPC)
		{
			sys_err("null pc pointer! pc: %u", pc);
			return;
		}

		if (!pPC->IsRunning())
		{
			sys_err("no quest running for pc, cannot process input : %u", pc);
			return;
		}

		if (pPC->GetRunningQuestState()->suspend_state != SUSPEND_STATE_CONFIRM)
		{
			sys_err("not wait for a confirm : %u %d", pc, pPC->GetRunningQuestState()->suspend_state);
			return;
		}

		if (pc2 && !pPC->IsConfirmWait(pc2))
		{
			sys_err("not wait for a confirm : %u %d", pc, pPC->GetRunningQuestState()->suspend_state);
			return;
		}

		pPC->ClearConfirmWait();

		pPC->SetSendDoneFlag();

		pPC->GetRunningQuestState()->args=1;
		lua_pushnumber(pPC->GetRunningQuestState()->co, confirm);

		AddScript("[END_CONFIRM_WAIT]");
		SetSkinStyle(QUEST_SKIN_NOWINDOW);
		SendScript();

		if (!RunState(*pPC->GetRunningQuestState()))
		{
			CloseState(*pPC->GetRunningQuestState());
			pPC->EndRunning();
		}

	}

	void CQuestManager::Input(uint32_t pc, const char* msg)
	{
		PC* pPC = GetPC(pc);
		if (!pPC)
		{
			sys_err("no pc! : %u",pc);
			return;
		}

		if (!pPC->IsRunning())
		{
			sys_err("no quest running for pc, cannot process input : %u", pc);
			return;
		}

		if (pPC->GetRunningQuestState()->suspend_state != SUSPEND_STATE_INPUT)
		{
			sys_err("not wait for a input : %u %d", pc, pPC->GetRunningQuestState()->suspend_state);
			return;
		}

		pPC->SetSendDoneFlag();

		pPC->GetRunningQuestState()->args=1;
		lua_pushstring(pPC->GetRunningQuestState()->co,msg);

		if (!RunState(*pPC->GetRunningQuestState()))
		{
			CloseState(*pPC->GetRunningQuestState());
			pPC->EndRunning();
		}
	}

	void CQuestManager::Select(uint32_t pc, uint32_t selection)
	{
		PC* pPC;

		if (((pPC = GetPC(pc))) && pPC->IsRunning() && pPC->GetRunningQuestState()->suspend_state==SUSPEND_STATE_SELECT)
		{
			pPC->SetSendDoneFlag();

			if (!pPC->GetRunningQuestState()->chat_scripts.empty())
			{
				// 채팅 이벤트인 경우
				// 현재 퀘스트는 어느 퀘스트를 실행할 것인가를 고르는 퀘스트 이므로
				// 끝내고 선택된 퀘스트를 실행한다.
				QuestState& old_qs = *pPC->GetRunningQuestState();
				CloseState(old_qs);

				if (selection >= pPC->GetRunningQuestState()->chat_scripts.size())
				{
					pPC->SetSendDoneFlag();
					GotoEndState(old_qs);
					pPC->EndRunning();
				}
				else
				{
					AArgScript* pas = pPC->GetRunningQuestState()->chat_scripts[selection];
					ExecuteQuestScript(*pPC, pas->quest_index, pas->state_index, pas->script.GetCode(), pas->script.GetSize());
				}
			}
			else
			{
				// on default 
				pPC->GetRunningQuestState()->args=1;
				lua_pushnumber(pPC->GetRunningQuestState()->co,selection+1);

				if (!RunState(*pPC->GetRunningQuestState()))
				{
					CloseState(*pPC->GetRunningQuestState());
					pPC->EndRunning();
				}
			}
		}
		else
		{
			sys_err("wrong QUEST_SELECT request! : %d",pc);
		}
	}

	void CQuestManager::Resume(uint32_t pc)
	{
		PC * pPC;

		if ((pPC = GetPC(pc)) && pPC->IsRunning() && pPC->GetRunningQuestState()->suspend_state == SUSPEND_STATE_PAUSE)
		{
			pPC->SetSendDoneFlag();
			pPC->GetRunningQuestState()->args = 0;

			if (!RunState(*pPC->GetRunningQuestState()))
			{
				CloseState(*pPC->GetRunningQuestState());
				pPC->EndRunning();
			}
		}
		else
		{
			//cerr << pPC << endl;
			//cerr << pPC->IsRunning() << endl;
			//cerr << pPC->GetRunningQuestState()->suspend_state;
			//cerr << SUSPEND_STATE_WAIT << endl;
			//cerr << "wrong QUEST_WAIT request! : " << pc << endl;
			sys_err("wrong QUEST_WAIT request! : %d",pc);
		}
	}

	void CQuestManager::EnterState(uint32_t pc, uint32_t quest_index, int32_t state)
	{
		PC* pPC;
		if ((pPC = GetPC(pc)))
		{
			if (!CheckQuestLoaded(pPC))
				return;

			m_mapNPC[QUEST_NO_NPC].OnEnterState(*pPC, quest_index, state);
		}
		else
			sys_err("QUEST no such pc id : %d", pc);
	}

	void CQuestManager::LeaveState(uint32_t pc, uint32_t quest_index, int32_t state)
	{
		PC* pPC;
		if ((pPC = GetPC(pc)))
		{
			if (!CheckQuestLoaded(pPC))
				return;

			m_mapNPC[QUEST_NO_NPC].OnLeaveState(*pPC, quest_index, state);
		}
		else
			sys_err("QUEST no such pc id : %d", pc);
	}

	void CQuestManager::Letter(uint32_t pc, uint32_t quest_index, int32_t state)
	{
		PC* pPC;
		if ((pPC = GetPC(pc)))
		{
			if (!CheckQuestLoaded(pPC))
				return;

			m_mapNPC[QUEST_NO_NPC].OnLetter(*pPC, quest_index, state);
		}
		else
			sys_err("QUEST no such pc id : %d", pc);
	}

	void CQuestManager::LogoutPC(LPCHARACTER ch)
	{
		if (!ch)
		{
			sys_err("no ch!");
			return;
		}
		PC * pPC = GetPC(ch->GetPlayerID());

		if (pPC && pPC->IsRunning())
		{
			CloseState(*pPC->GetRunningQuestState());
			pPC->CancelRunning();
		}

		// 지우기 전에 로그아웃 한다.
		Logout(ch->GetPlayerID());

		if (ch == m_pCurrentCharacter)
		{
			m_pCurrentCharacter = nullptr;
			m_pCurrentPC = nullptr;
		}
	}

	///////////////////////////////////////////////////////////////////////////////////////////
	//
	// Quest Event 관련
	//
	///////////////////////////////////////////////////////////////////////////////////////////
	void CQuestManager::Login(uint32_t pc, const char * c_pszQuest)
	{
		PC * pPC;

		if ((pPC = GetPC(pc)))
		{
			if (!CheckQuestLoaded(pPC))
				return;

			m_mapNPC[QUEST_NO_NPC].OnLogin(*pPC, c_pszQuest);
		}
		else
		{
			sys_err("QUEST no such pc id : %u", pc);
		}
	}

	void CQuestManager::Logout(uint32_t pc)
	{
		PC * pPC;

		if ((pPC = GetPC(pc)))
		{
			if (!CheckQuestLoaded(pPC))
				return;

			m_mapNPC[QUEST_NO_NPC].OnLogout(*pPC);
		}
		else
			sys_err("QUEST no such pc id : %u", pc);
	}

	void CQuestManager::Kill(uint32_t pc, uint32_t npc)
	{
		//m_CurrentNPCRace = npc;
		PC * pPC;

		sys_log(0, "CQuestManager::Kill QUEST_KILL_EVENT (pc=%u, npc=%u)", pc, npc);

		if ((pPC = GetPC(pc)))
		{
			if (!CheckQuestLoaded(pPC))
				return;

			// kill call script
			if (npc >= MAIN_RACE_MAX_NUM)
				m_mapNPC[npc].OnKill(*pPC);
			m_mapNPC[QUEST_NO_NPC].OnKill(*pPC);

			// party_kill call script
			LPCHARACTER ch = GetCurrentCharacterPtr();
			LPPARTY pParty = ch->GetParty();
			LPCHARACTER leader = pParty ? pParty->GetLeaderCharacter() : ch;
			if (leader)
			{
				m_pCurrentPartyMember = ch;
				if (npc >= MAIN_RACE_MAX_NUM)
					m_mapNPC[npc].OnPartyKill(*GetPC(leader->GetPlayerID()));
				m_mapNPC[QUEST_NO_NPC].OnPartyKill(*GetPC(leader->GetPlayerID()));

				pPC = GetPC(pc);
			}
		}
		else
			sys_err("QUEST: no such pc id : %d", pc);
	}

	void CQuestManager::Die(uint32_t pc, uint32_t npc)
	{
		PC * pPC;

		sys_log(0, "CQuestManager::Kill QUEST_DIE_EVENT (pc=%d, npc=%d)", pc, npc);

		if ((pPC = GetPC(pc)))
		{
			if (!CheckQuestLoaded(pPC))
			{
				sys_err("No quest loaded for pid %d", pc);
				return;
			}

			m_mapNPC[QUEST_NO_NPC].OnDie(*pPC);
		}
		else
			sys_err("QUEST: no such pc id : %u", pc);
	}

	bool CQuestManager::ServerTimer(uint32_t npc, uint32_t arg)
	{
		SetServerTimerArg(arg);
		sys_log(0, "XXX ServerTimer Call NPC %p vnum %u arg %u", GetPCForce(0), npc, arg);
		m_pCurrentPC = GetPCForce(0);
		m_pCurrentCharacter = nullptr;
		m_pSelectedDungeon = nullptr;
		return m_mapNPC[npc].OnServerTimer(*m_pCurrentPC);
	}

	bool CQuestManager::Timer(uint32_t pc, uint32_t npc)
	{
		PC* pPC;

		if ((pPC = GetPC(pc)))
		{
			if (!CheckQuestLoaded(pPC))
			{
				return false;
			}
			// call script
			return m_mapNPC[npc].OnTimer(*pPC);
		}

		sys_err("QUEST TIMER_EVENT no such pc id : %u", pc);
		return false;
	}

	void CQuestManager::LevelUp(uint32_t pc)
	{
		PC * pPC;

		if ((pPC = GetPC(pc)))
		{
			if (!CheckQuestLoaded(pPC))
				return;

			m_mapNPC[QUEST_NO_NPC].OnLevelUp(*pPC);
		}
		else
		{
			sys_err("QUEST LEVELUP_EVENT no such pc id : %u", pc);
		}
	}

	void CQuestManager::AttrIn(uint32_t pc, LPCHARACTER ch, int32_t attr)
	{
		PC* pPC;
		if ((pPC = GetPC(pc)))
		{
			m_pCurrentPartyMember = ch;
			if (!CheckQuestLoaded(pPC))
				return;

			// call script
			m_mapNPC[attr+QUEST_ATTR_NPC_START].OnAttrIn(*pPC);
		}
		else
		{
			//cout << "no such pc id : " << pc;
			sys_err("QUEST no such pc id : %u", pc);
		}
	}

	void CQuestManager::AttrOut(uint32_t pc, LPCHARACTER ch, int32_t attr)
	{
		PC* pPC;
		if ((pPC = GetPC(pc)))
		{
			//m_pCurrentCharacter = ch;
			m_pCurrentPartyMember = ch;
			if (!CheckQuestLoaded(pPC))
				return;

			// call script
			m_mapNPC[attr+QUEST_ATTR_NPC_START].OnAttrOut(*pPC);
		}
		else
		{
			//cout << "no such pc id : " << pc;
			sys_err("QUEST no such pc id : %u", pc);
		}
	}

	bool CQuestManager::Target(uint32_t pc, uint32_t dwQuestIndex, const char * c_pszTargetName, const char * c_pszVerb)
	{
		PC * pPC;

		if ((pPC = GetPC(pc)))
		{
			if (!CheckQuestLoaded(pPC))
				return false;

			bool bRet;
			return m_mapNPC[QUEST_NO_NPC].OnTarget(*pPC, dwQuestIndex, c_pszTargetName, c_pszVerb, bRet);
		}

		return false;
	}

	void CQuestManager::QuestInfo(uint32_t pc, uint32_t quest_index)
	{
		PC* pPC;

		if ((pPC = GetPC(pc)))
		{
			// call script
			if (!CheckQuestLoaded(pPC))
			{
				LPCHARACTER ch = CHARACTER_MANAGER::Instance().FindByPID(pc);

				if (ch)
					ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("퀘스트를 로드하는 중입니다. 잠시만 기다려 주십시오."));

				return;
			}

			m_bQuestInfoFlag = 1;
			m_mapNPC[QUEST_NO_NPC].OnInfo(*pPC, quest_index);
		}
		else
		{
			//cout << "no such pc id : " << pc;
			sys_err("QUEST INFO_EVENT no such pc id : %u", pc);
		}
	}

	void CQuestManager::QuestButton(uint32_t pc, uint32_t quest_index)
	{
		PC* pPC;
		if ((pPC = GetPC(pc)))
		{
			// call script
			if (!CheckQuestLoaded(pPC))
			{
				LPCHARACTER ch = CHARACTER_MANAGER::Instance().FindByPID(pc);
				if (ch)
				{
					ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("퀘스트를 로드하는 중입니다. 잠시만 기다려 주십시오."));
				}
				return;
			}
			m_mapNPC[QUEST_NO_NPC].OnButton(*pPC, quest_index);
		}
		else
		{
			//cout << "no such pc id : " << pc;
			sys_err("QUEST CLICK_EVENT no such pc id : %u", pc);
		}
	}

	bool CQuestManager::TakeItem(uint32_t pc, uint32_t npc, LPITEM item)
	{
		//m_CurrentNPCRace = npc;
		PC* pPC;

		if ((pPC = GetPC(pc)))
		{
			if (!CheckQuestLoaded(pPC))
			{
				LPCHARACTER ch = CHARACTER_MANAGER::Instance().FindByPID(pc);
				if (ch)
				{
					ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("퀘스트를 로드하는 중입니다. 잠시만 기다려 주십시오."));
				}
				return false;
			}
			// call script
			SetCurrentItem(item);
			return m_mapNPC[npc].OnTakeItem(*pPC);
		}
		sys_err("QUEST USE_ITEM_EVENT no such pc id : %u", pc);
		return false;
	}

	bool CQuestManager::UseItem(uint32_t pc, LPITEM item, bool bReceiveAll)
	{
		if (g_bIsTestServer)
			sys_log( 0, "questmanager::UseItem Start : itemVnum : %d PC : %d", item->GetOriginalVnum(), pc);
		PC* pPC;
		if ((pPC = GetPC(pc)))
		{
			if (!CheckQuestLoaded(pPC))
			{
				LPCHARACTER ch = CHARACTER_MANAGER::Instance().FindByPID(pc);
				if (ch)
				{
					ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("퀘스트를 로드하는 중입니다. 잠시만 기다려 주십시오."));
				}
				return false;
			}
			// call script
			SetCurrentItem(item);
			/*
			if (g_bIsTestServer)
			{
				sys_log( 0, "Quest UseItem Start : itemVnum : %d PC : %d", item->GetOriginalVnum(), pc);
				auto it = m_mapNPC.begin();
				auto end = m_mapNPC.end();
				for( ; it != end ; ++it)
				{
					sys_log( 0, "Quest UseItem : vnum : %d item Vnum : %d", it->first, item->GetOriginalVnum());
				}
			}
			if(g_bIsTestServer)
			sys_log( 0, "questmanager:useItem: mapNPCVnum : %d\n", m_mapNPC[item->GetVnum()].GetVnum());
			*/

			return m_mapNPC[item->GetVnum()].OnUseItem(*pPC, bReceiveAll);
		}

		sys_err("QUEST USE_ITEM_EVENT no such pc id : %d", pc);
		return false;
	}

	// Speical Item Group에 정의된 Group Use
	bool CQuestManager::SIGUse(uint32_t pc, uint32_t sig_vnum, LPITEM item, bool bReceiveAll)
	{
		if (g_bIsTestServer)
			sys_log( 0, "questmanager::SIGUse Start : itemVnum : %d PC : %u", item->GetOriginalVnum(), pc);
		PC* pPC;
		if ((pPC = GetPC(pc)))
		{
			if (!CheckQuestLoaded(pPC))
			{
				LPCHARACTER ch = CHARACTER_MANAGER::Instance().FindByPID(pc);
				if (ch)
				{
					ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("퀘스트를 로드하는 중입니다. 잠시만 기다려 주십시오."));
				}
				return false;
			}
			// call script
			SetCurrentItem(item);

			return m_mapNPC[sig_vnum].OnSIGUse(*pPC, bReceiveAll);
		}

		sys_err("QUEST USE_ITEM_EVENT no such pc id : %u", pc);
		return false;
	}

	bool CQuestManager::GiveItemToPC(uint32_t pc, LPCHARACTER pkChr)
	{
		if (!pkChr->IsPC())
			return false;

		PC * pPC = GetPC(pc);

		if (pPC)
		{
			if (!CheckQuestLoaded(pPC))
				return false;

			TargetInfo * pInfo = CTargetManager::Instance().GetTargetInfo(pc, TARGET_TYPE_VID, pkChr->GetVID());

			if (pInfo)
			{
				bool bRet;

				if (m_mapNPC[QUEST_NO_NPC].OnTarget(*pPC, pInfo->dwQuestIndex, pInfo->szTargetName, "click", bRet))
					return true;
			}
		}

		return false;
	}

	bool CQuestManager::Click(uint32_t pc, LPCHARACTER pkChrTarget)
	{
		PC * pPC = GetPC(pc);

		if (pPC)
		{
			if (!CheckQuestLoaded(pPC))
			{
				LPCHARACTER ch = CHARACTER_MANAGER::Instance().FindByPID(pc);

				if (ch)
					ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("퀘스트를 로드하는 중입니다. 잠시만 기다려 주십시오."));

				return false;
			}

			TargetInfo * pInfo = CTargetManager::Instance().GetTargetInfo(pc, TARGET_TYPE_VID, pkChrTarget->GetVID());
			if (g_bIsTestServer)
			{
				sys_log(0, "CQuestManager::Click(pid=%d, npc_name=%s) - target_info(%x)", pc, pkChrTarget->GetName(), pInfo);
			}

			if (pInfo)
			{
				bool bRet;
				if (m_mapNPC[QUEST_NO_NPC].OnTarget(*pPC, pInfo->dwQuestIndex, pInfo->szTargetName, "click", bRet))
					return bRet;
			}

			uint32_t dwCurrentNPCRace = pkChrTarget->GetRaceNum();

			if (pkChrTarget->IsNPC())
			{
				auto it = m_mapNPC.find(dwCurrentNPCRace);

				if (it == m_mapNPC.end())
				{
					if (g_bIsTestServer)
					{
						sys_log(0, "CQuestManager::Click(pid=%d, target_npc_name=%s) - NOT EXIST NPC RACE VNUM[%d]",
								pc,
								pkChrTarget->GetName(),
								dwCurrentNPCRace);
					}
					return false;
				}

				// call script
				if (it->second.HasChat())
				{
					// if have chat, give chat
					if (g_bIsTestServer)
						sys_log(0, "CQuestManager::Click->OnChat");

					if (!it->second.OnChat(*pPC))
					{
						if (g_bIsTestServer)
							sys_log(0, "CQuestManager::Click->OnChat Failed");

						return it->second.OnClick(*pPC);
					}

					return true;
				}
				else
				{
					// else click
					return it->second.OnClick(*pPC);
				}
			}
			return false;
		}

		sys_err("QUEST CLICK_EVENT no such pc id : %d", pc);
		return false;
	}

	void CQuestManager::Unmount(uint32_t pc)
	{
		PC * pPC;

		if ((pPC = GetPC(pc)))
		{
			if (!CheckQuestLoaded(pPC))
				return;

			m_mapNPC[QUEST_NO_NPC].OnUnmount(*pPC);
		}
		else
			sys_err("QUEST no such pc id : %u", pc);
	}
	//독일 선물 기능 테스트
	void CQuestManager::ItemInformer(uint32_t pc,uint32_t vnum)
	{
		PC* pPC = GetPC(pc);
		if (!pPC)
			return;
		m_mapNPC[QUEST_NO_NPC].OnItemInformer(*pPC,vnum);
	}
	///////////////////////////////////////////////////////////////////////////////////////////
	// END OF 퀘스트 이벤트 처리
	///////////////////////////////////////////////////////////////////////////////////////////

	void CQuestManager::UpdateStateItem(LPCHARACTER ch, LPITEM item)
	{
		PC * pPC = GetPC(ch->GetPlayerID());

		if (pPC && pPC->IsRunning() && pPC->GetRunningQuestState())
		{
			pPC->GetRunningQuestState()->_item = item;
		}
		else
			sys_err("item state update fail!");
	}

	///////////////////////////////////////////////////////////////////////////////////////////
	void CQuestManager::LoadStartQuest(const std::string& quest_name, uint32_t idx)
	{
		for (const auto & stQuestObjectDir : g_setQuestObjectDir)
		{
			std::string full_name = stQuestObjectDir + "/begin_condition/" + quest_name;
			std::ifstream inf(full_name.c_str());

			if (inf.is_open())
			{
				sys_log(0, "QUEST loading begin condition for %s", quest_name.c_str());

				std::istreambuf_iterator<char> ib(inf), ie;
				copy(ib, ie, back_inserter(m_hmQuestStartScript[idx]));
			}
		}
	}

	bool CQuestManager::CanStartQuest(uint32_t quest_index, const PC& pc)
	{
		return CanStartQuest(quest_index);
	}

	bool CQuestManager::CanStartQuest(uint32_t quest_index)
	{
		THashMapQuestStartScript::iterator it;

		if ((it = m_hmQuestStartScript.find(quest_index)) == m_hmQuestStartScript.end())
			return true;

		int32_t x = lua_gettop(L);
		lua_dobuffer(L, &(it->second[0]), it->second.size(), "StartScript");
		int32_t bStart = lua_toboolean(L, -1);
		lua_settop(L, x);
		return bStart != 0;
	}

	bool CQuestManager::CanEndQuestAtState(const std::string& quest_name, const std::string& state_name)
	{
		return false;
	}

	void CQuestManager::DisconnectPC(LPCHARACTER ch)
	{
		m_mapPC.erase(ch->GetPlayerID());
	}

	PC * CQuestManager::GetPCForce(uint32_t pc)
	{
		PCMap::iterator it;

		if ((it = m_mapPC.find(pc)) == m_mapPC.end())
		{
			PC * pPC = &m_mapPC[pc];
			pPC->SetID(pc);
			return pPC;
		}

		return &it->second;
	}

	PC * CQuestManager::GetPC(uint32_t pc)
	{
		LPCHARACTER pkChr = CHARACTER_MANAGER::Instance().FindByPID(pc);

		if (!pkChr)
			return nullptr;

		m_pCurrentPC = GetPCForce(pc);
		m_pCurrentCharacter = pkChr;
		m_pSelectedDungeon = nullptr;
		return (m_pCurrentPC);
	}

	void CQuestManager::ClearScript()
	{
		m_strScript.clear();
		m_iCurrentSkin = QUEST_SKIN_NORMAL;
	}

	void CQuestManager::AddScript(const std::string& str)
	{
		m_strScript+=str;
	}

	void CQuestManager::SendScript()
	{
		if (m_bNoSend)
		{
			m_bNoSend = false;
			ClearScript();
			return;
		}

		if (m_strScript=="[DONE]" || m_strScript == "[NEXT]")
		{
			if (m_pCurrentPC && !m_pCurrentPC->GetAndResetDoneFlag() && m_strScript=="[DONE]" && m_iCurrentSkin == QUEST_SKIN_NORMAL && !IsError())
			{
				ClearScript();
				return;
			}
			m_iCurrentSkin = QUEST_SKIN_NOWINDOW;
		}

		//sys_log(0, "Send Quest Script to %s", GetCurrentCharacterPtr()->GetName());
		//send -_-!
		SPacketGCScript packet_script;

		packet_script.skin = m_iCurrentSkin;
		packet_script.src_size = static_cast<uint16_t>(m_strScript.size());
		packet_script.size = packet_script.src_size + sizeof(SPacketGCScript);

		TEMP_BUFFER buf;
		buf.write(&packet_script, sizeof(SPacketGCScript));
		buf.write(&m_strScript[0], m_strScript.size());

		CHARACTER * ch = GetCurrentCharacterPtr();
		LPDESC desc = ch ? ch->GetDesc() : nullptr;

		if (!ch || !desc)
		{
			ClearScript();
			m_bQuestInfoFlag = 0;
			return;
		}
		
		desc->Packet(buf.read_peek(), buf.size());
		if (g_bIsTestServer)
		{
			sys_log(0, "m_strScript %s size %d", m_strScript.c_str(), buf.size());
		}

		ClearScript();
		m_bQuestInfoFlag = 0;
	}

	const char* CQuestManager::GetQuestStateName(const std::string& quest_name, const int32_t state_index)
	{
		if (!L)
		{
			sys_err("Lua state is null pointer!!!");
		}
		int32_t x = lua_gettop(L);
		lua_getglobal(L, quest_name.c_str());
		if (lua_isnil(L,-1))
		{
			sys_err("QUEST wrong quest state file %s.%d", quest_name.c_str(), state_index);

			LPCHARACTER ch = GetCurrentCharacterPtr();
			if (ch)
				sys_err("QUEST wrong quest state file from %s", ch->GetName());

			LPCHARACTER npc = GetCurrentNPCCharacterPtr();
			if (npc)
				sys_err("QUEST wrong quest state file for %s", npc->GetName());

			lua_settop(L,x);
			return "";
		}
		lua_pushnumber(L, state_index);
		lua_gettable(L, -2);

		const char* str = lua_tostring(L, -1);
		lua_settop(L, x);
		return str;
	}

	int32_t CQuestManager::GetQuestStateIndex(const std::string& quest_name, const std::string& state_name)
	{
		if (!L)
		{
			sys_err("Lua state is null pointer!!!");
		}
		int32_t x = lua_gettop(L);
		lua_getglobal(L, quest_name.c_str());
		if (lua_isnil(L,-1))
		{
			sys_err("QUEST wrong quest state file %s.%s",quest_name.c_str(),state_name.c_str()  );

			LPCHARACTER ch = GetCurrentCharacterPtr();
			if (ch)
				sys_err("QUEST wrong quest state file from %s", ch->GetName());

			LPCHARACTER npc = GetCurrentNPCCharacterPtr();
			if (npc)
				sys_err("QUEST wrong quest state file for %s", npc->GetName());

			lua_settop(L,x);
			return 0;
		}
		lua_pushstring(L, state_name.c_str());
		lua_gettable(L, -2);

		int32_t v = (int32_t)rint(lua_tonumber(L,-1));
		lua_settop(L, x);
		if ( g_bIsTestServer )
			sys_log( 0,"[QUESTMANAGER] GetQuestStateIndex x(%d) v(%d) %s %s", v,x, quest_name.c_str(), state_name.c_str() );
		return v;
	}

	void CQuestManager::SetSkinStyle(int32_t iStyle)
	{
		if (iStyle<0 || iStyle >= QUEST_SKIN_COUNT)
		{
			m_iCurrentSkin = QUEST_SKIN_NORMAL;
		}
		else
			m_iCurrentSkin = iStyle;
	}

	uint32_t CQuestManager::LoadTimerScript(const std::string& name)
	{
		std::map<std::string, uint32_t>::iterator it;
		if ((it = m_mapTimerID.find(name)) == m_mapTimerID.end())
		{
			auto new_id = std::numeric_limits<uint32_t>::max() - m_mapTimerID.size();

			m_mapNPC[new_id].Set(new_id, name);
			m_mapTimerID.insert(std::make_pair(name, new_id));

			return new_id;
		}
		return it->second;
	}

	uint32_t CQuestManager::GetCurrentNPCRace()
	{
		return GetCurrentNPCCharacterPtr() ? GetCurrentNPCCharacterPtr()->GetRaceNum() : 0;
	}

	LPITEM CQuestManager::GetCurrentItem()
	{
		return (GetCurrentCharacterPtr() && GetCurrentState()) ? GetCurrentState()->_item : nullptr;
	}

	void CQuestManager::ClearCurrentItem()
	{
		if (GetCurrentCharacterPtr())
			GetCurrentCharacterPtr()->ClearQuestItemPtr();
	}

	void CQuestManager::SetCurrentItem(LPITEM item)
	{
		LPCHARACTER ch = GetCurrentCharacterPtr();
		if (ch)
			ch->SetQuestItemPtr(item);
	}

	LPCHARACTER CQuestManager::GetCurrentNPCCharacterPtr()
	{ 
		return GetCurrentCharacterPtr() ? GetCurrentCharacterPtr()->GetQuestNPC() : nullptr; 
	}

	const std::string & CQuestManager::GetCurrentQuestName()
	{
		return GetCurrentPC()->GetCurrentQuestName();
	}

	LPDUNGEON CQuestManager::GetCurrentDungeon()
	{
		LPCHARACTER ch = GetCurrentCharacterPtr();

		if (!ch)
		{
			if (m_pSelectedDungeon)
				return m_pSelectedDungeon;
			return nullptr;
		}

		return ch->GetDungeonForce();
	}

	void CQuestManager::RegisterQuest(const std::string & stQuestName, uint32_t idx)
	{
		assert(idx > 0);

		auto it = m_hmQuestName.find(stQuestName);
		if (it != m_hmQuestName.end())
			return;

		m_hmQuestName.insert(std::make_pair(stQuestName, idx));
		LoadStartQuest(stQuestName, idx);
		m_mapQuestNameByIndex.insert(std::make_pair(idx, stQuestName));

		sys_log(0, "QUEST: Register %4u %s", idx, stQuestName.c_str());
	}

	uint32_t CQuestManager::GetQuestIndexByName(const std::string& name)
	{
		auto it = m_hmQuestName.find(name);

		if (it == m_hmQuestName.end())
			return 0; // RESERVED

		return it->second;
	}

	const std::string & CQuestManager::GetQuestNameByIndex(uint32_t idx)
	{
		if (idx == 0) 
		{
			static std::string stint = "SPECIAL_INTERNAL_QUEST";
			return stint;
		}
		auto it = m_mapQuestNameByIndex.find(idx);
		if (it == m_mapQuestNameByIndex.end())
		{
			sys_err("cannot find quest name by index %u", idx);
			assert(!"cannot find quest name by index");

			static std::string st = "";
			return st;
		}

		return it->second;
	}

	void CQuestManager::SendEventFlagList(LPCHARACTER ch, std::string filter)
	{
		bool found = false;
		for (const auto it : m_mapEventFlag)
		{
			const std::string& flagname = it.first;
			int32_t value = it.second;

			if (filter != "" && flagname.find(filter) == std::string::npos)
				continue;
			if (!g_bIsTestServer && value == 1 && flagname == "valentine_drop")
				ch->ChatPacket(CHAT_TYPE_INFO, "%s %d prob 800", flagname.c_str(), value);
			else if (!g_bIsTestServer && value == 1 && flagname == "newyear_wonso")
				ch->ChatPacket(CHAT_TYPE_INFO, "%s %d prob 500", flagname.c_str(), value);
			else if (!g_bIsTestServer && value == 1 && flagname == "newyear_fire")
				ch->ChatPacket(CHAT_TYPE_INFO, "%s %d prob 1000", flagname.c_str(), value);
			else
				ch->ChatPacket(CHAT_TYPE_INFO, "%s %d", flagname.c_str(), value);
			if (filter != "")
				found = true;
		}

		if (filter != "" && !found) 
		{
			ch->ChatPacket(CHAT_TYPE_INFO, "NO_EVENTFLAG_MATCHED_FILTER_%s", filter.c_str());
		}
	}

	void CQuestManager::RequestSetEventFlag(const std::string& name, int32_t value)
	{
		TPacketSetEventFlag p;
		strlcpy(p.szFlagName, name.c_str(), sizeof(p.szFlagName));
		p.lValue = value;
		db_clientdesc->DBPacket(HEADER_GD_SET_EVENT_FLAG, 0, &p, sizeof(TPacketSetEventFlag));
	}

	void CQuestManager::SetEventFlag(const std::string& name, int32_t value)
	{
		int32_t prev_value = m_mapEventFlag[name];

		sys_log(0, "QUEST eventflag %s %d prev_value %d", name.c_str(), value, m_mapEventFlag[name]);
		m_mapEventFlag[name] = value;

		if (name == "mob_item")
		{
			CHARACTER_MANAGER::Instance().SetMobItemRate(value);
		}
		else if (name == "mob_dam")
		{
			CHARACTER_MANAGER::Instance().SetMobDamageRate(value);
		}
		else if (name == "mob_gold")
		{
			CHARACTER_MANAGER::Instance().SetMobGoldAmountRate(value);
		}
		else if (name == "mob_gold_pct")
		{
			CHARACTER_MANAGER::Instance().SetMobGoldDropRate(value);
		}
		else if (name == "user_dam")
		{
			CHARACTER_MANAGER::Instance().SetUserDamageRate(value);
		}
		else if (name == "user_dam_buyer")
		{
			CHARACTER_MANAGER::Instance().SetUserDamageRatePremium(value);
		}
		else if (name == "mob_exp")
		{
			CHARACTER_MANAGER::Instance().SetMobExpRate(value);
		}
		else if (name == "mob_item_buyer")
		{
			CHARACTER_MANAGER::Instance().SetMobItemRatePremium(value);
		}
		else if (name == "mob_exp_buyer")
		{
			CHARACTER_MANAGER::Instance().SetMobExpRatePremium(value);
		}
		else if (name == "mob_gold_buyer")
		{
			CHARACTER_MANAGER::Instance().SetMobGoldAmountRatePremium(value);
		}
		else if (name == "mob_gold_pct_buyer")
		{
			CHARACTER_MANAGER::Instance().SetMobGoldDropRatePremium(value);
		}

		else if (name == "gold_drop_limit_time")
		{
			g_GoldDropTimeLimitValue = value * 1000;
		}
		else if (name == "item_drop_limit_time")
		{
			g_ItemDropTimeLimitValue = value * 1000;
		}
		else if (name == "box_use_limit_time")
		{
			g_BoxUseTimeLimitValue = value * 1000;
		}
		else if (name == "buysell_limit_time")
		{
			g_BuySellTimeLimitValue = value * 1000;
		}
		else if (name == "no_drop_metin_stone")
		{
			g_NoDropMetinStone = !!value;
		}
		else if (name == "no_mount_at_guild_war")
		{
			g_NoMountAtGuildWar = !!value;
		}
		else if (name == "no_potions_on_pvp")
		{
			g_NoPotionsOnPVP = !!value;
		}
		else if (name == "newyear_boom")
		{
			DESC_MANAGER::Instance().BroadcastCommand(std::string("newyear_boom ") + std::to_string(value));
		}
		else if ( name == "eclipse" )
		{
			std::string mode = value == 1 ? "dark" : "light";
			DESC_MANAGER::Instance().BroadcastCommand(std::string("DayMode ") + mode);
		}
		else if (name == "new_xmas_event")
		{
			static uint32_t new_santa = 20126;
			if (value != 0)
			{
				auto map1_santa_exist = false;
				auto map21_santa_exist = false;
				auto map41_santa_exist = false;

				const auto chars = CHARACTER_MANAGER::Instance().GetCharactersByRaceNum(new_santa);
				for (auto ch : chars) {
					if (ch->GetMapIndex() == 1)
						map1_santa_exist = true;
					else if (ch->GetMapIndex() == 21)
						map21_santa_exist = true;
					else if (ch->GetMapIndex() == 41)
						map41_santa_exist = true;
				}

				if (map_allow_find(1) && !map1_santa_exist)
				{
					auto pkSectreeMap = SECTREE_MANAGER::Instance().GetMap(1);
					CHARACTER_MANAGER::Instance().SpawnMob(new_santa, 1, 60800, 61700, 0, false, 90, true);
				}
				if (map_allow_find(21) && !map21_santa_exist)
				{
					auto pkSectreeMap = SECTREE_MANAGER::Instance().GetMap(21);
					CHARACTER_MANAGER::Instance().SpawnMob(new_santa, 21, 59600, 61000, 0, false, 110, true);
				}
				if (map_allow_find(41) && !map41_santa_exist)
				{
					auto pkSectreeMap = SECTREE_MANAGER::Instance().GetMap(41);
					CHARACTER_MANAGER::Instance().SpawnMob(new_santa, 41, 35700, 74300, 0, false, 140, true);
				}
			}
			else
			{
				const auto chars = CHARACTER_MANAGER::Instance().GetCharactersByRaceNum(new_santa);
				for (auto ch : chars)
					M2_DESTROY_CHARACTER(ch);
			}
		}
	}

	int32_t	CQuestManager::GetEventFlag(const std::string& name)
	{
		auto it = m_mapEventFlag.find(name);

		if (it == m_mapEventFlag.end())
			return 0;

		return it->second;
	}

	void CQuestManager::BroadcastEventFlagOnLogin(LPCHARACTER ch)
	{
		int32_t iEventFlagValue;

		//Generic flag behaviour
		std::vector<std::string> flags = { { "xmas_snow", "xmas_boom", "xmas_tree", "newyear_boom", "halloween_box", "mini_game_okey", "valentine_drop", "e_easter_monsterback", "carnival_event", "mini_game_okey_normal", "e_summer_event" } };
		for (const auto flag : flags) 
		{
			iEventFlagValue = quest::CQuestManager::Instance().GetEventFlag(flag);
			if (iEventFlagValue) {
				ch->ChatPacket(CHAT_TYPE_COMMAND, "%s %d", flag.c_str(), iEventFlagValue);
			}
		}


		if ( (iEventFlagValue = quest::CQuestManager::Instance().GetEventFlag("eclipse")) )
		{
			ch->ChatPacket(CHAT_TYPE_COMMAND, "DayMode %s", iEventFlagValue == 1 ? "dark" : "light");
		}
	}

	void CQuestManager::Reload()
	{
		if (!InitializeLua())
		{
			sys_err("Unable to reload quest(InitializeLua)");
			return;
		}

		lua_close(L);
		m_mapNPC.clear();
		m_mapNPCNameID.clear();
		m_hmQuestName.clear();
		m_mapTimerID.clear();
		//Clear all server timers
		for (auto it = m_mapServerTimer.begin(); it != m_mapServerTimer.end();) 
		{
			LPEVENT event = it->second;
			event_cancel(&event);
			it = m_mapServerTimer.erase(it);
		}
		m_hmQuestStartScript.clear();
		m_mapEventName.clear();
		L = nullptr;
		if (!Initialize())
		{
			sys_err("Unable to reload quest(Initialize)");
			return;
		}

		for (auto dwVnum : m_registeredNPCVnum)
		{
			char buf[256] = { 0 };
			snprintf(buf, sizeof(buf), "%u", dwVnum);
			m_mapNPC[dwVnum].Set(dwVnum, buf);
		}
	}

	bool CQuestManager::ExecuteQuestScript(PC& pc, uint32_t quest_index, const int32_t state, const char* code, const int32_t code_size, std::vector<AArgScript*>* pChatScripts, bool bUseCache)
	{
		return ExecuteQuestScript(pc, CQuestManager::Instance().GetQuestNameByIndex(quest_index), state, code, code_size, pChatScripts, bUseCache);
	}

	bool CQuestManager::ExecuteQuestScript(PC& pc, const std::string& quest_name, const int32_t state, const char* code, const int32_t code_size, std::vector<AArgScript*>* pChatScripts, bool bUseCache)
	{
		// 실행공간을 생성
		QuestState qs = CQuestManager::Instance().OpenState(quest_name, state);
		if (pChatScripts)
			qs.chat_scripts.swap(*pChatScripts);

		// 코드를 읽어들임
		if (bUseCache)
		{
			lua_getglobal(qs.co, "__codecache");
			// stack : __codecache
			lua_pushnumber(qs.co, (uintptr_t)code);
			// stack : __codecache (codeptr)
			lua_rawget(qs.co, -2);
			// stack : __codecache (compiled-code)
			if (lua_isnil(qs.co, -1))
			{
				// cache miss

				// load code to lua,
				// save it to cache
				// and only function remain in stack
				lua_pop(qs.co, 1);
				// stack : __codecache
				luaL_loadbuffer(qs.co, code, code_size, quest_name.c_str());
				// stack : __codecache (compiled-code)
				lua_pushnumber(qs.co, (uintptr_t)code);
				// stack : __codecache (compiled-code) (codeptr)
				lua_pushvalue(qs.co, -2);
				// stack : __codecache (compiled-code) (codeptr) (compiled_code)
				lua_rawset(qs.co, -4);
				// stack : __codecache (compiled-code)
				lua_remove(qs.co, -2);
				// stack : (compiled-code)
			}
			else
			{
				// cache hit
				lua_remove(qs.co, -2);
				// stack : (compiled-code)
			}
		}
		else
			luaL_loadbuffer(qs.co, code, code_size, quest_name.c_str());

		// 플레이어와 연결
		pc.SetQuest(quest_name, qs);

		// 실행
		QuestState& rqs = *pc.GetRunningQuestState();
		if (!CQuestManager::Instance().RunState(rqs))
		{
			CQuestManager::Instance().CloseState(rqs);
			pc.EndRunning();
			return false;
		}
		return true;
	}

	void CQuestManager::RegisterNPCVnum(uint32_t dwVnum)
	{
		if (m_registeredNPCVnum.find(dwVnum) != m_registeredNPCVnum.end())
			return;

		m_registeredNPCVnum.insert(dwVnum);

		for (const auto& stQuestObjectDir : g_setQuestObjectDir)
		{
			std::string directory = string_format("%s/%u", stQuestObjectDir.c_str(), dwVnum);
			
			if (std::filesystem::exists(directory) && std::filesystem::is_directory(directory))
			{
				auto vnum = std::to_string(dwVnum);
				sys_log(0, "%s", vnum.c_str());

				m_mapNPC[dwVnum].Set(dwVnum, vnum);
			}
		}
	}

	void CQuestManager::WriteRunningStateToSyserr()
	{
		const char * state_name = GetQuestStateName(GetCurrentQuestName(), GetCurrentState()->st);

		std::string event_index_name = "";

		for (auto& event : m_mapEventName)
		{
			if (event.second == m_iRunningEventIndex)
			{
				event_index_name = event.first;
				break;
			}
		}

		sys_err("LUA_ERROR: quest %s.%s %s", GetCurrentQuestName().c_str(), state_name, event_index_name.c_str() );
		if (GetCurrentCharacterPtr() && g_bIsTestServer)
			GetCurrentCharacterPtr()->ChatPacket(CHAT_TYPE_PARTY, "LUA_ERROR: quest %s.%s %s", GetCurrentQuestName().c_str(), state_name, event_index_name.c_str() );
	}

#ifndef _WIN32
	void CQuestManager::QuestError(const char* func, int32_t line, const char* fmt, ...)
	{
		char szMsg[4096];
		va_list args;

		va_start(args, fmt);
		vsnprintf(szMsg, sizeof(szMsg), fmt, args);
		va_end(args);

		_sys_err(func, line, "%s", szMsg);
		if (g_bIsTestServer)
		{
			LPCHARACTER ch = GetCurrentCharacterPtr();
			if (ch)
			{
				ch->ChatPacket(CHAT_TYPE_PARTY, "error occurred on [%s:%d]", func,line);
				ch->ChatPacket(CHAT_TYPE_PARTY, "%s", szMsg);
			}
		}
	}
#else
	void CQuestManager::QuestError(const char* func, int32_t line, const char* fmt, ...)
	{
		char szMsg[4096];
		va_list args;

		va_start(args, fmt);
		vsnprintf(szMsg, sizeof(szMsg), fmt, args);
		va_end(args);

		_sys_err(func, line, "%s", szMsg);
		if (g_bIsTestServer)
		{
			LPCHARACTER ch = GetCurrentCharacterPtr();
			if (ch)
			{
				ch->ChatPacket(CHAT_TYPE_PARTY, "error occurred on [%s:%d]", func,line);
				ch->ChatPacket(CHAT_TYPE_PARTY, "%s", szMsg);
			}
		}
	}
#endif

	void CQuestManager::AddServerTimer(const std::string& name, uint32_t arg, LPEVENT event)
	{
		sys_log(0, "XXX AddServerTimer %s %d %p", name.c_str(), arg, get_pointer(event));
		if (m_mapServerTimer.find(make_pair(name, arg)) != m_mapServerTimer.end())
		{
			sys_err("already registered server timer name:%s arg:%u", name.c_str(), arg);
			return;
		}
		m_mapServerTimer.insert(make_pair(make_pair(name, arg), event));
	}

	void CQuestManager::ClearServerTimerNotCancel(const std::string& name, uint32_t arg)
	{
		m_mapServerTimer.erase(make_pair(name, arg));
	}

	void CQuestManager::ClearServerTimer(const std::string& name, uint32_t arg)
	{
		auto it = m_mapServerTimer.find(make_pair(name, arg));
		if (it != m_mapServerTimer.end())
		{
			LPEVENT event = it->second;
			event_cancel(&event);
			m_mapServerTimer.erase(it);
		}
	}

	void CQuestManager::CancelServerTimers(uint32_t arg)
	{
		for (auto it = m_mapServerTimer.begin(); it != m_mapServerTimer.end();)
		{
			if (it->first.second == arg)
			{
				LPEVENT event = it->second;
				event_cancel(&event);

				m_mapServerTimer.erase(it++);
			}
			else
			{
				++it;
			}
		}
	}

	void CQuestManager::SetServerTimerArg(uint32_t dwArg)
	{
		m_dwServerTimerArg = dwArg;
	}

	uint32_t CQuestManager::GetServerTimerArg()
	{
		return m_dwServerTimerArg;
	}

	void CQuestManager::SelectDungeon(LPDUNGEON pDungeon)
	{
		m_pSelectedDungeon = pDungeon;
	}
	
	bool CQuestManager::PickupItem(uint32_t pc, LPITEM item)
	{
		if (g_bIsTestServer)
			sys_log( 0, "questmanager::PickupItem Start : itemVnum : %u PC : %u", item->GetOriginalVnum(), pc);
		PC* pPC;
		if ((pPC = GetPC(pc)))
		{
			if (!CheckQuestLoaded(pPC))
			{
				LPCHARACTER ch = CHARACTER_MANAGER::Instance().FindByPID(pc);
				if (ch)
				{
					ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("퀘스트를 로드하는 중입니다. 잠시만 기다려 주십시오."));
				}
				return false;
			}
			// call script
			SetCurrentItem(item);

			return m_mapNPC[item->GetVnum()].OnPickupItem(*pPC);
		}
		else
		{
			sys_err("QUEST PICK_ITEM_EVENT no such pc id : %d", pc);
			return false;
		}
	}
	void CQuestManager::BeginOtherPCBlock(uint32_t pid)
	{
		LPCHARACTER ch = GetCurrentCharacterPtr();
		if (nullptr == ch)
		{
			sys_err("nullptr?");
			return;
		}
		/*
		# 1. current pid = pid0 <- It will be m_pOtherPCBlockRootPC.
		begin_other_pc_block(pid1)
			# 2. current pid = pid1
			begin_other_pc_block(pid2)
				# 3. current_pid = pid2
			end_other_pc_block()
		end_other_pc_block()
		*/
		// when begin_other_pc_block(pid1)
		if (m_vecPCStack.empty())
		{
			m_pOtherPCBlockRootPC = GetCurrentPC();
		}
		m_vecPCStack.push_back(GetCurrentCharacterPtr()->GetPlayerID());
		GetPC(pid);
	}

	void CQuestManager::EndOtherPCBlock()
	{
		if (m_vecPCStack.empty())
		{
			sys_err("m_vecPCStack is alread empty. CurrentQuest{Name(%s), State(%s)}", GetCurrentQuestName().c_str(), GetCurrentState()->_title.c_str());
			return;
		}
		uint32_t pc = m_vecPCStack.back();
		m_vecPCStack.pop_back();
		GetPC(pc);

		if (m_vecPCStack.empty())
		{
			m_pOtherPCBlockRootPC = nullptr;
		}
	}

	bool CQuestManager::IsInOtherPCBlock()
	{
		return !m_vecPCStack.empty();
	}

	PC*	CQuestManager::GetOtherPCBlockRootPC()
	{
		return m_pOtherPCBlockRootPC;
	}
}

