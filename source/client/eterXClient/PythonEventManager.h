#pragma once

#include "../eterLib/parser.h"

#include <string>
#include <map>

class CPythonEventManager : public CSingleton<CPythonEventManager>
{
	public:
		typedef struct STextLine
		{
			int32_t ixLocal, iyLocal;
			CGraphicTextInstance * pInstance;
		} TTextLine;

		using TScriptTextLineList = std::list<TTextLine>;
		using TEventAnswerMap = std::map<int32_t, std::string>;

		enum
		{
			EVENT_POSITION_START = 0,
			EVENT_POSITION_END = 1,

			BOX_VISIBLE_LINE_COUNT = 5,
		};

		enum EButtonType
		{
			BUTTON_TYPE_NEXT,
			BUTTON_TYPE_DONE,
			BUTTON_TYPE_CANCEL,
		};

		enum EEventType
		{
			EVENT_TYPE_LETTER,
			EVENT_TYPE_COLOR,
			EVENT_TYPE_DELAY,
			EVENT_TYPE_ENTER,
			EVENT_TYPE_WAIT,
			EVENT_TYPE_CLEAR,
			EVENT_TYPE_QUESTION,
			EVENT_TYPE_NEXT,
			EVENT_TYPE_DONE,

			EVENT_TYPE_LEFT_IMAGE,
			EVENT_TYPE_TOP_IMAGE,
			EVENT_TYPE_BACKGROUND_IMAGE,
			EVENT_TYPE_IMAGE,

			EVENT_TYPE_ADD_MAP_SIGNAL,
			EVENT_TYPE_CLEAR_MAP_SIGNAL,
			EVENT_TYPE_SET_MESSAGE_POSITION,
			EVENT_TYPE_ADJUST_MESSAGE_POSITION,
			EVENT_TYPE_SET_CENTER_MAP_POSITION,

			EVENT_TYPE_QUEST_BUTTON,

			// HIDE_QUEST_LETTER
			EVENT_TYPE_QUEST_BUTTON_CLOSE,
			// END_OF_HIDE_QUEST_LETTER

			EVENT_TYPE_SLEEP,
			EVENT_TYPE_SET_CAMERA,
			EVENT_TYPE_BLEND_CAMERA,
			EVENT_TYPE_RESTORE_CAMERA,
			EVENT_TYPE_FADE_OUT,
			EVENT_TYPE_FADE_IN,
			EVENT_TYPE_WHITE_OUT,
			EVENT_TYPE_WHITE_IN,
			EVENT_TYPE_CLEAR_TEXT,
			EVENT_TYPE_TEXT_HORIZONTAL_ALIGN_CENTER,
			EVENT_TYPE_TITLE_IMAGE,

			EVENT_TYPE_RUN_CINEMA,
			EVENT_TYPE_DUNGEON_RESULT,

			EVENT_TYPE_ITEM_NAME,
			EVENT_TYPE_MONSTER_NAME,

			EVENT_TYPE_COLOR256,
			EVENT_TYPE_WINDOW_SIZE,

			EVENT_TYPE_INPUT,
			EVENT_TYPE_CONFIRM_WAIT,
			EVENT_TYPE_END_CONFIRM_WAIT,

			EVENT_TYPE_INSERT_IMAGE,

			EVENT_TYPE_SELECT_ITEM,
		};

	private:
		std::map<std::string, int32_t> EventTypeMap;

	public:
		typedef struct SEventSet
		{
			int32_t ix, iy;
			int32_t iWidth;
			int32_t iyLocal;

			// State
			bool isLock;

			int32_t lLastDelayTime;

			int32_t iCurrentLetter;
			
			int32_t iTotalLineCount;

			D3DXCOLOR CurrentColor;
			std::string strCurrentLine;

			CGraphicTextInstance * pCurrentTextLine;
			TScriptTextLineList ScriptTextLineList;

			BOOL isConfirmWait;
			CGraphicTextInstance * pConfirmTimeTextLine;
			int32_t iConfirmEndTime;

			// Group Data
			script::Group ScriptGroup;

			// Static Data
			char szFileName[32+1];

			int32_t iVisibleStartLine;
			int32_t iVisibleLineCount;

			int32_t iAdjustLine;

			D3DXCOLOR DiffuseColor;
			int32_t lWaitingTime;
			int32_t iRestrictedCharacterCount;

			int32_t nAnswer;

			bool isTextCenterMode;
			bool isWaitFlag;

			PyObject * poEventHandler;

			bool isQuestInfo;

			SEventSet() : ix(0), iy(0), iWidth(0), iyLocal(0), isLock(false), lLastDelayTime(0), iCurrentLetter(0), pCurrentTextLine(nullptr),
						isConfirmWait(0), pConfirmTimeTextLine(nullptr), iConfirmEndTime(0), iVisibleStartLine(0), iVisibleLineCount(0),
						iAdjustLine(0), lWaitingTime(0), iRestrictedCharacterCount(0), nAnswer(0), isTextCenterMode(false), isWaitFlag(false), poEventHandler(nullptr), isQuestInfo(false)
			{}
			virtual ~SEventSet() {}
		} TEventSet;

		using TEventSetVector = std::vector<TEventSet *>;


	public:
		CPythonEventManager();
		virtual ~CPythonEventManager();

		void Destroy();
		
		int32_t GetTotalLineCount(int32_t iIndex);

		void AllProcessEventSet(int32_t iIndex);

		int32_t GetProcessedLineCount(int32_t iIndex);

		void SetYPosition(int32_t iIndex, int32_t iY);

		int32_t	GetLineHeight(int32_t iIndex);

		void SetVisibleLineCount(int32_t iIndex, int32_t iLineCount);

		int32_t RegisterEventSet(const char * c_szFileName);
		int32_t RegisterEventSetFromString(const std::string& strScript, bool bIsQuestInfo = false);
		void ClearEventSeti(int32_t iIndex);
		void __ClearEventSetp(TEventSet * pEventSet);

		void SetEventHandler(int32_t iIndex, PyObject * poEventHandler);
		void SetRestrictedCount(int32_t iIndex, int32_t iCount);

		int32_t GetEventSetLocalYPosition(int32_t iIndex);
		void AddEventSetLocalYPosition(int32_t iIndex, int32_t iAddValue);
		void InsertText(int32_t iIndex, const char * c_szText, int32_t iX_pos = 0);

		void UpdateEventSet(int32_t iIndex, int32_t ix, int32_t iy);
		void RenderEventSet(int32_t iIndex);
		void SetEventSetWidth(int32_t iIndex, int32_t iWidth);

		void Skip(int32_t iIndex);
		bool IsWait(int32_t iIndex);
		void EndEventProcess(int32_t iIndex);

		void SelectAnswer(int32_t iIndex, int32_t iAnswer);
		void SetVisibleStartLine(int32_t iIndex, int32_t iStartLine);
		int32_t GetVisibleStartLine(int32_t iIndex);
		int32_t GetLineCount(int32_t iIndex);

		void SetInterfaceWindow(PyObject * poInterface);
		void SetLeftTimeString(const char * c_szString);
		void SetFontColor(int32_t iIndex, float r, float g, float b);

	protected:
		void __InitEventSet(TEventSet& rEventSet);
		void __InsertLine(TEventSet& rEventSet, BOOL isCenter=FALSE, int32_t iX_pos=0);
		void __AddSpace(TEventSet& pEventSet, int32_t iSpace);

		uint32_t GetEmptyEventSetSlot();

		bool CheckEventSetIndex(int32_t iIndex);

		bool ProcessEventSet(TEventSet * pEventSet);

		void ClearLine(TEventSet * pEventSet);		
		void RefreshLinePosition(TEventSet * pEventSet);		
		bool GetScriptEventIndex(const char * c_szName, int32_t * pEventPosition, int32_t * pEventType);

		void MakeQuestion(TEventSet * pEventSet, script::TArgList & rArgumentList);
		void MakeNextButton(TEventSet * pEventSet, int32_t iButtonType);

	protected:
		TEventSetVector m_EventSetVector;
		BOOL m_isQuestConfirmWait;

	private:
		CDynamicPool<TEventSet> m_EventSetPool;
		CDynamicPool<CGraphicTextInstance> m_ScriptTextLinePool;
		PyObject * m_poInterface;
		std::string m_strLeftTimeString;
};