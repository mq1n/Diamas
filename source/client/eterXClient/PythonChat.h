#pragma once
#include "AbstractChat.h"

class CWhisper
{
	public:
		typedef struct SChatLine
		{
			CGraphicTextInstance Instance;

			SChatLine() = default;
			~SChatLine()
			{
				Instance.Destroy();
			}

			static void DestroySystem();

			static SChatLine* New();
			static void Delete(SChatLine* pkChatLine);

			static CDynamicPool<SChatLine> ms_kPool;
		} TChatLine;

		using TChatLineDeque = std::deque<TChatLine *>;
		using TChatLineList = std::list<TChatLine *>;

	public:
		CWhisper();
		~CWhisper();

		void Destroy();

		void SetPosition(float fPosition);
		void SetBoxSize(float fWidth, float fHeight);
		void AppendChat(int32_t iType, const char* c_szChat);
		void Render(float fx, float fy);
	const char * GetLastWhisper();

	protected:
		void __Initialize();
		void __ArrangeChat();

	protected:
		float m_fLineStep;
		float m_fWidth;
		float m_fHeight;
		float m_fcurPosition;
	const char * m_lastWhisper;

		TChatLineDeque m_ChatLineDeque;
		TChatLineList m_ShowingChatLineList;

	public:
		static void DestroySystem();

		static CWhisper* New();
		static void Delete(CWhisper* pkWhisper);

		static CDynamicPool<CWhisper>		ms_kPool;
};

class CPythonChat : public CSingleton<CPythonChat>, public IAbstractChat
{
	public:
		enum EWhisperType
		{
			WHISPER_TYPE_CHAT               = 0,
			WHISPER_TYPE_NOT_EXIST          = 1,
			WHISPER_TYPE_TARGET_BLOCKED     = 2,
			WHISPER_TYPE_SENDER_BLOCKED     = 3,
			WHISPER_TYPE_ERROR              = 4,
			WHISPER_TYPE_GM                 = 5,
			WHISPER_TYPE_SYSTEM             = 0xFF
		};

		enum EBoardState
		{
			BOARD_STATE_VIEW,
			BOARD_STATE_EDIT,
		BOARD_STATE_LOG
		};

		enum
		{
			CHAT_LINE_MAX_NUM = 300,
		CHAT_LINE_COLOR_ARRAY_MAX_NUM = 3
		};

		typedef struct SChatLine
		{
			int32_t iType;
			float fAppendedTime;
			D3DXCOLOR aColor[CHAT_LINE_COLOR_ARRAY_MAX_NUM];
			CGraphicTextInstance Instance;

			SChatLine();
			virtual ~SChatLine();

			void SetColor(uint32_t dwID, uint32_t dwColor);
			void SetColorAll(uint32_t dwColor);
			D3DXCOLOR & GetColorRef(uint32_t dwID);
			static void DestroySystem();

			static SChatLine* New();
			static void Delete(SChatLine* pkChatLine);

			static CDynamicPool<SChatLine> ms_kPool;
		} TChatLine;

		typedef struct SWaitChat
		{
			int32_t iType;
			std::string strChat;

			uint32_t dwAppendingTime;
		} TWaitChat;

		using TChatLineDeque = std::deque<TChatLine *>;
		using TChatLineList = std::list<TChatLine *>;

		using TWhisperMap = std::map<std::string, CWhisper *>;
		using TIgnoreCharacterSet = std::set<std::string>;
		using TWaitChatList = std::list<TWaitChat>;

		typedef struct SChatSet
		{
			int32_t					m_ix;
			int32_t					m_iy;
			int32_t					m_iHeight;
			int32_t					m_iStep;
			float				m_fEndPos;

			int32_t					m_iBoardState;
			std::vector<int32_t>	m_iMode;

			TChatLineList		m_ShowingChatLineList;

			bool CheckMode(uint32_t dwType)
			{
				if (dwType >= m_iMode.size())
					return false;

				return m_iMode[dwType] ? true : false;
			}

			SChatSet()
			{
				m_iBoardState = BOARD_STATE_VIEW;

				m_ix = 0;
				m_iy = 0;
				m_fEndPos = 1.0f;
				m_iHeight = 0;
				m_iStep = 15;

				m_iMode.clear();
				m_iMode.resize(ms_iChatModeSize, 1);
			}

			static int32_t ms_iChatModeSize;
		} TChatSet;

		typedef std::map<int32_t, TChatSet> TChatSetMap;

	public:
		CPythonChat();
		virtual ~CPythonChat();

		void SetChatColor(uint32_t eType, uint32_t r, uint32_t g, uint32_t b);

		void Destroy();
		void Close();

		int32_t CreateChatSet(uint32_t dwID);
		void Update(uint32_t dwID);
		void Render(uint32_t dwID);
		void RenderWhisper(const char * c_szName, float fx, float fy);

		void SetBoardState(uint32_t dwID, int32_t iState);
		void SetPosition(uint32_t dwID, int32_t ix, int32_t iy);
		void SetHeight(uint32_t dwID, int32_t iHeight);
		void SetStep(uint32_t dwID, int32_t iStep);
		void ToggleChatMode(uint32_t dwID, int32_t iMode);
		void EnableChatMode(uint32_t dwID, int32_t iMode);
		void DisableChatMode(uint32_t dwID, int32_t iMode);
		void SetEndPos(uint32_t dwID, float fPos);

		int32_t  GetVisibleLineCount(uint32_t dwID);
		int32_t  GetEditableLineCount(uint32_t dwID);
		int32_t  GetLineCount(uint32_t dwID);
		int32_t  GetLineStep(uint32_t dwID);

		// Chat
		void AppendChat(int32_t iType, const char * c_szChat);
		void AppendChatWithDelay(int32_t iType, const char * c_szChat, int32_t iDelay);
		void ArrangeShowingChat(uint32_t dwID);

		// Ignore
		void IgnoreCharacter(const char * c_szName);
		BOOL IsIgnoreCharacter(const char * c_szName);

		// Whisper
		CWhisper * CreateWhisper(const char * c_szName);
		void AppendWhisper(int32_t iType, const char * c_szName, const char * c_szChat);
		void ClearWhisper(const char * c_szName);
		BOOL GetWhisper(const char * c_szName, CWhisper ** ppWhisper);
		void InitWhisper(PyObject * ppyObject);

	protected:
		void __Initialize();
		void __DestroyWhisperMap();

		TChatLineList * GetChatLineListPtr(uint32_t dwID);
		TChatSet * GetChatSetPtr(uint32_t dwID);

		void UpdateViewMode(uint32_t dwID);
		void UpdateEditMode(uint32_t dwID);
		void UpdateLogMode(uint32_t dwID);

		uint32_t GetChatColor(int32_t iType);

	protected:
		TChatLineDeque						m_ChatLineDeque;
		TChatLineList						m_ShowingChatLineList;
		TChatSetMap							m_ChatSetMap;
		TWhisperMap							m_WhisperMap;
		TIgnoreCharacterSet					m_IgnoreCharacterSet;
		TWaitChatList						m_WaitChatList;

		D3DXCOLOR m_akD3DXClrChat[CHAT_TYPE_MAX_NUM];
};