#ifndef QUEST_NPC
#define QUEST_NPC

#include "quest_pc.h"

extern int32_t g_bIsTestServer;

namespace quest
{
	enum EQuestModes
	{
		QUEST_START_STATE_INDEX = 0,
		QUEST_CHAT_STATE_INDEX = -1,
		QUEST_FISH_REFINE_STATE_INDEX = -2,
	};

	class PC;

	class NPC
	{
		public:
			// 인자가 없는 스크립트들
			// first: state number
			typedef std::map<int32_t, AStateScriptType> AQuestScriptType;
			// first: quest number
			typedef std::map<uint32_t, AQuestScriptType> QuestMapType;

			// 인자가 있는 스크립트들
			// first: state number
			typedef std::map<int32_t, std::vector<AArgScript> > AArgQuestScriptType;
			// first: quest number
			typedef std::map<uint32_t, AArgQuestScriptType> ArgQuestMapType;

			NPC();
			~NPC();

			void	Set(uint32_t vnum, const std::string & script_name);

			static bool HasStartState(const AQuestScriptType & q)
			{
				return q.find(0) != q.end();
			}

			static bool HasStartState(const AArgQuestScriptType& q)
			{
				return q.find(0) != q.end();
			}

			bool	OnServerTimer(PC& pc);

			bool	OnClick(PC& pc);
			bool	OnKill(PC& pc);
			bool	OnPartyKill(PC& pc);
			bool	OnDie(PC& pc);
			bool	OnTimer(PC& pc);
			bool	OnLevelUp(PC& pc);
			bool	OnLogin(PC& pc, const char * c_pszQuestName = nullptr);
			bool	OnLogout(PC& pc);
			bool	OnButton(PC& pc, uint32_t quest_index);
			bool	OnInfo(PC& pc, uint32_t quest_index);
			bool	OnAttrIn(PC& pc);
			bool	OnAttrOut(PC& pc);
			bool	OnUseItem(PC& pc, bool bReceiveAll);
			bool	OnTakeItem(PC& pc);
			bool	OnEnterState(PC& pc, uint32_t quest_index, int32_t state);
			bool	OnLeaveState(PC& pc, uint32_t quest_index, int32_t state);
			bool	OnLetter(PC& pc, uint32_t quest_index, int32_t state);
			bool	OnChat(PC& pc);
			bool	HasChat();

			bool	OnItemInformer(PC& pc,uint32_t vnum);	// 독일 선물 기능 테스트

			bool	OnTarget(PC& pc, uint32_t dwQuestIndex, const char * c_pszTargetName, const char * c_pszVerb, bool & bRet);
			bool	OnUnmount(PC& pc);

			// ITEM_PICK EVENT
			bool	OnPickupItem(PC& pc);
			// Special item group USE EVENT
			bool	OnSIGUse(PC& pc, bool bReceiveAll);


			bool	HandleEvent(PC& pc, int32_t EventIndex);
			bool	HandleReceiveAllEvent(PC& pc, int32_t EventIndex);
			bool	HandleReceiveAllNoWaitEvent(PC& pc, int32_t EventIndex);

			bool	ExecuteEventScript(PC& pc, int32_t EventIndex, uint32_t dwQuestIndex, int32_t iState);

			uint32_t GetVnum() { return m_vnum; }


		protected:
			template <typename TQuestMapType, typename FuncMatch, typename FuncMiss>
				void MatchingQuest(PC& pc, TQuestMapType& QuestMap, FuncMatch& fMatch, FuncMiss& fMiss);

			// true if quest still running, false if ended

			void LoadStateScript(int32_t idx, const char* filename, const char* script_name);

			uint32_t m_vnum;
			QuestMapType m_mapOwnQuest[QUEST_EVENT_COUNT];
			ArgQuestMapType m_mapOwnArgQuest[QUEST_EVENT_COUNT];
			//map<string, AStartConditionScriptType> m_mapStartCondition[QUEST_EVENT_COUNT];
	};

	template <typename TQuestMapType, typename FuncMatch, typename FuncMiss>
		void NPC::MatchingQuest(PC& pc, TQuestMapType& QuestMap, FuncMatch& fMatch, FuncMiss& fMiss)
		{
			if (g_bIsTestServer)
				sys_log(0, "Click Quest : MatchingQuest");

			PC::QuestInfoIterator itPCQuest = pc.quest_begin();
			typename TQuestMapType::iterator itQuestMap = QuestMap.begin();

			while (itQuestMap != QuestMap.end())
			{
				if (itPCQuest == pc.quest_end() || itPCQuest->first > itQuestMap->first)
				{
					fMiss(itPCQuest, itQuestMap);
					++itQuestMap;
				}
				else if (itPCQuest->first < itQuestMap->first)
				{
					++itPCQuest;
				}
				else
				{
					fMatch(itPCQuest, itQuestMap);
					++itPCQuest;
					++itQuestMap;
				}
			}
		}
}
#endif
