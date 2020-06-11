#pragma once

#include "../eterLib/FuncObject.h"
#include "../eterlib/NetStream.h"
#include "../eterLib/NetPacketHeaderMap.h"
#include "../eterSecurity/CheatQueueManager.h"

#include "InsultChecker.h"
#include "Locale_inc.h"
#include "packet.h"

class CInstanceBase;
class CNetworkActorManager;
struct SNetworkActorData;
struct SNetworkUpdateActorData;

class CPythonNetworkStream : public CNetworkStream, public CSingleton<CPythonNetworkStream>
{
	public:
		enum
		{
			SERVER_COMMAND_LOG_OUT = 0,
			SERVER_COMMAND_RETURN_TO_SELECT_CHARACTER = 1,
			SERVER_COMMAND_QUIT = 2,

			MAX_ACCOUNT_PLAYER
		};
		
		enum
		{
			ERROR_NONE,
			ERROR_UNKNOWN,
			ERROR_CONNECT_MARK_SERVER,			
			ERROR_LOAD_MARK,
			ERROR_MARK_WIDTH,
			ERROR_MARK_HEIGHT,

			// MARK_BUG_FIX
			ERROR_MARK_UPLOAD_NEED_RECONNECT,
			ERROR_MARK_CHECK_NEED_RECONNECT,
			// END_OF_MARK_BUG_FIX
		};

		enum
		{
			ACCOUNT_CHARACTER_SLOT_ID,
			ACCOUNT_CHARACTER_SLOT_NAME,
			ACCOUNT_CHARACTER_SLOT_RACE,
			ACCOUNT_CHARACTER_SLOT_LEVEL,
			ACCOUNT_CHARACTER_SLOT_STR,
			ACCOUNT_CHARACTER_SLOT_DEX,
			ACCOUNT_CHARACTER_SLOT_HTH,
			ACCOUNT_CHARACTER_SLOT_INT,
			ACCOUNT_CHARACTER_SLOT_PLAYTIME,
			ACCOUNT_CHARACTER_SLOT_FORM,
			ACCOUNT_CHARACTER_SLOT_ADDR,
			ACCOUNT_CHARACTER_SLOT_PORT,
			ACCOUNT_CHARACTER_SLOT_GUILD_ID,
			ACCOUNT_CHARACTER_SLOT_GUILD_NAME,
			ACCOUNT_CHARACTER_SLOT_CHANGE_NAME_FLAG,
			ACCOUNT_CHARACTER_SLOT_HAIR,
#ifdef ENABLE_ACCE_SYSTEM
			ACCOUNT_CHARACTER_SLOT_ACCE,
#endif
		};

		enum
		{
			PHASE_WINDOW_LOGO,
			PHASE_WINDOW_LOGIN,
			PHASE_WINDOW_SELECT,
			PHASE_WINDOW_CREATE,
			PHASE_WINDOW_LOAD,
			PHASE_WINDOW_GAME,
			PHASE_WINDOW_EMPIRE,
			PHASE_WINDOW_NUM,
		};

	public:
		CPythonNetworkStream();
		virtual ~CPythonNetworkStream();
		
		void StartGame();
		void Warp(int32_t lGlobalX, int32_t lGlobalY);
		
		void SetWaitFlag();

		void SendEmoticon(uint32_t eEmoticon);

		void ExitApplication();
		void ExitGame();
		void LogOutGame();
		void AbsoluteExitGame();
		void AbsoluteExitApplication();

		void EnableChatInsultFilter(bool isEnable);		
		bool IsChatInsultIn(const char* c_szMsg);
		bool IsInsultIn(const char* c_szMsg);

		uint32_t GetGuildID();

		uint32_t UploadMark(const char* c_szImageFileName);
		uint32_t UploadSymbol(const char* c_szImageFileName);

		bool LoadInsultList(const char* c_szInsultListFileName);
		bool LoadConvertTable(uint32_t dwEmpireID, const char* c_szFileName);

		uint32_t		GetAccountCharacterSlotDatau(uint32_t iSlot, uint32_t eType);
		const char* GetAccountCharacterSlotDataz(uint32_t iSlot, uint32_t eType);

		// SUPPORT_BGM
		const char*		GetFieldMusicFileName();
		float			GetFieldMusicVolume();
		// END_OF_SUPPORT_BGM

		bool IsSelectedEmpire();

		void ToggleGameDebugInfo();

		void SetMarkServer(const char* c_szAddr, uint32_t uPort);
		void ConnectLoginServer(const char* c_szAddr, uint32_t uPort);
		void ConnectGameServer(uint32_t iChrSlot);

		void SetLoginInfo(const char* c_szID, const char* c_szPassword);
		void SetLoginKey(uint32_t dwLoginKey);
		void ClearLoginInfo( void );

		void SetHandler(PyObject* poHandler);
		void SetPhaseWindow(uint32_t ePhaseWnd, PyObject* poPhaseWnd);
		void ClearPhaseWindow(uint32_t ePhaseWnd, PyObject* poPhaseWnd);
		void SetServerCommandParserWindow(PyObject* poPhaseWnd);

		bool SendSyncPositionElementPacket(uint32_t dwVictimVID, uint32_t dwVictimX, uint32_t dwVictimY);

		bool SendAttackPacket(uint32_t uMotAttack, uint32_t dwVIDVictim);
		bool SendCharacterStatePacket(const TPixelPosition& c_rkPPosDst, float fDstRot, uint32_t eFunc, uint32_t uArg);
		bool SendUseSkillPacket(uint32_t dwSkillIndex, uint32_t dwTargetVID=0);
		bool SendTargetPacket(uint32_t dwVID);

		// OLDCODE:
		bool SendCharacterStartWalkingPacket(float fRotation, int32_t lx, int32_t ly);
		bool SendCharacterEndWalkingPacket(float fRotation, int32_t lx, int32_t ly);
		bool SendCharacterCheckWalkingPacket(float fRotation, int32_t lx, int32_t ly);

		bool SendCharacterPositionPacket(uint8_t iPosition);

		bool SendItemUsePacket(TItemPos pos);
		bool SendItemUseToItemPacket(TItemPos source_pos, TItemPos target_pos);
		bool SendItemDropPacket(TItemPos pos, uint32_t elk);
		bool SendItemDropPacketNew(TItemPos pos, uint32_t elk, uint32_t count);
		bool SendItemMovePacket(TItemPos pos, TItemPos change_pos, uint8_t num);
		bool SendItemPickUpPacket(uint32_t vid);

		bool SendQuickSlotAddPacket(uint8_t wpos, uint8_t type, uint8_t pos);
		bool SendQuickSlotDelPacket(uint8_t wpos);
		bool SendQuickSlotMovePacket(uint8_t wpos, uint8_t change_pos);

		// PointReset 개 임시
		bool SendPointResetPacket();

		// Shop
		bool SendShopEndPacket();
		bool SendShopBuyPacket(uint8_t byCount);
		bool SendShopSellPacket(uint8_t bySlot);
		bool SendShopSellPacketNew(uint8_t bySlot, uint8_t byCount);

		// Exchange
		bool SendExchangeStartPacket(uint32_t vid);
		bool SendExchangeItemAddPacket(TItemPos ItemPos, uint8_t byDisplayPos);
		bool SendExchangeElkAddPacket(uint32_t elk);
		bool SendExchangeItemDelPacket(uint8_t pos);
		bool SendExchangeAcceptPacket();
		bool SendExchangeExitPacket();

		// Quest
		bool SendScriptAnswerPacket(int32_t iAnswer);
		bool SendScriptButtonPacket(uint32_t iIndex);
		bool SendAnswerMakeGuildPacket(const char * c_szName);
		bool SendQuestInputStringPacket(const char * c_szString);
		bool SendQuestConfirmPacket(uint8_t byAnswer, uint32_t dwPID);

		// Event
		bool SendOnClickPacket(uint32_t vid);

		// Fly
		bool SendFlyTargetingPacket(uint32_t dwTargetVID, const TPixelPosition& kPPosTarget);
		bool SendAddFlyTargetingPacket(uint32_t dwTargetVID, const TPixelPosition& kPPosTarget);
		bool SendShootPacket(uint32_t uSkill);

		// Command
		bool ClientCommand(const char * c_szCommand);
		void ServerCommand(char * c_szCommand);

		// Emoticon
		void RegisterEmoticonString(const char * pcEmoticonString);

		// Party
		bool SendPartyInvitePacket(uint32_t dwVID);
		bool SendPartyInviteAnswerPacket(uint32_t dwLeaderVID, uint8_t byAccept);
		bool SendPartyRemovePacket(uint32_t dwPID);
		bool SendPartySetStatePacket(uint32_t dwVID, uint8_t byState, uint8_t byFlag);
		bool SendPartyUseSkillPacket(uint8_t bySkillIndex, uint32_t dwVID);
		bool SendPartyParameterPacket(uint8_t byDistributeMode);

		// SafeBox
		bool SendSafeBoxMoneyPacket(uint8_t byState, uint32_t dwMoney);
		bool SendSafeBoxCheckinPacket(TItemPos InventoryPos, uint8_t bySafeBoxPos);
		bool SendSafeBoxCheckoutPacket(uint8_t bySafeBoxPos, TItemPos InventoryPos);
		bool SendSafeBoxItemMovePacket(uint8_t bySourcePos, uint8_t byTargetPos, uint8_t byCount);

		// Mall
		bool SendMallCheckoutPacket(uint8_t byMallPos, TItemPos InventoryPos);

		// Guild
		bool SendGuildAddMemberPacket(uint32_t dwVID);
		bool SendGuildRemoveMemberPacket(uint32_t dwPID);
		bool SendGuildChangeGradeNamePacket(uint8_t byGradeNumber, const char * c_szName);
		bool SendGuildChangeGradeAuthorityPacket(uint8_t byGradeNumber, uint8_t byAuthority);
		bool SendGuildOfferPacket(uint32_t dwExperience);
		bool SendGuildPostCommentPacket(const char * c_szMessage);
		bool SendGuildDeleteCommentPacket(uint32_t dwIndex);
		bool SendGuildRefreshCommentsPacket(uint32_t dwHighestIndex);
		bool SendGuildChangeMemberGradePacket(uint32_t dwPID, uint8_t byGrade);
		bool SendGuildUseSkillPacket(uint32_t dwSkillID, uint32_t dwTargetVID);
		bool SendGuildChangeMemberGeneralPacket(uint32_t dwPID, uint8_t byFlag);
		bool SendGuildInvitePacket(uint32_t dwVID);
		bool SendGuildInviteAnswerPacket(uint32_t dwGuildID, uint8_t byAnswer);
		bool SendGuildChargeGSPPacket(uint32_t dwMoney);
		bool SendGuildDepositMoneyPacket(uint32_t dwMoney);
		bool SendGuildWithdrawMoneyPacket(uint32_t dwMoney);

		// Mall
		bool RecvMallOpenPacket();
		bool RecvMallItemSetPacket();
		bool RecvMallItemDelPacket();

		// Lover
		bool RecvLoverInfoPacket();
		bool RecvLovePointUpdatePacket();

		// Dig
		bool RecvDigMotionPacket();

		// Fishing
		bool SendFishingPacket(int32_t iRotation);
		bool SendGiveItemPacket(uint32_t dwTargetVID, TItemPos ItemPos, int32_t iItemCount);

		// Private Shop
		bool SendBuildPrivateShopPacket(const char * c_szName, const std::vector<TShopItemTable> & c_rSellingItemStock);

		// Refine
		bool SendRefinePacket(uint8_t byPos, uint8_t byType);
		bool SendSelectItemPacket(uint32_t dwItemPos);

		// 용홍석 강화
		bool SendDragonSoulRefinePacket(uint8_t bRefineType, TItemPos* pos);

		// Handshake
		bool RecvHandshakePacket();
		bool RecvHandshakeOKPacket();

#ifdef _IMPROVED_PACKET_ENCRYPTION_
		bool RecvKeyAgreementPacket();
		bool RecvKeyAgreementCompletedPacket();

#endif
		// ETC
		uint32_t GetMainActorVID();
		uint32_t GetMainActorRace();
		uint32_t GetMainActorEmpire();
		uint32_t GetMainActorSkillGroup();
		void SetEmpireID(uint32_t dwEmpireID);
		uint32_t GetEmpireID();
		void __TEST_SetSkillGroupFake(int32_t iIndex);
#ifdef ENABLE_ACCE_SYSTEM
		bool	SendAcceClosePacket();
		bool	SendAcceAddPacket(TItemPos tPos, uint8_t bPos);
		bool	SendAcceRemovePacket(uint8_t bPos);
		bool	SendAcceRefinePacket();
#endif

	//////////////////////////////////////////////////////////////////////////
	// Phase 관련
	//////////////////////////////////////////////////////////////////////////
	public:
		void SetOffLinePhase();
		void SetHandShakePhase();
		void SetLoginPhase();
		void SetSelectPhase();
		void SetLoadingPhase();
		void SetGamePhase();
		void ClosePhase();

		// Login Phase
		bool SendLoginPacket(const char * c_szName, const char * c_szPassword);
		bool SendLoginPacketNew(const char * c_szName, const char * c_szPassword);
		bool SendDirectEnterPacket(const char * c_szName, const char * c_szPassword, uint32_t uChrSlot);

		bool SendEnterGame();

		// Select Phase
		bool SendSelectEmpirePacket(uint32_t dwEmpireID);
		bool SendSelectCharacterPacket(uint8_t account_Index);
		bool SendChangeNamePacket(uint8_t index, const char *name);
		bool SendCreateCharacterPacket(uint8_t index, const char *name, uint8_t job, uint8_t shape, uint8_t byStat1, uint8_t byStat2, uint8_t byStat3, uint8_t byStat4);
		bool SendDestroyCharacterPacket(uint8_t index, const char * szPrivateCode);

		// Main Game Phase
		bool SendChatPacket(const char * c_szChat, uint8_t byType = CHAT_TYPE_TALKING);
		bool SendWhisperPacket(const char * name, const char * c_szChat);
		bool SendMessengerAddByVIDPacket(uint32_t vid);
		bool SendMessengerAddByNamePacket(const char * c_szName);
		bool SendMessengerRemovePacket(const char * c_szKey, const char * c_szName);

		bool SendHackNotification(const char* c_szMsg, const char* c_szInfo);

	protected:
		bool OnProcess();	// State들을 실제로 실행한다.
		void OffLinePhase();
		void HandShakePhase();
		void LoginPhase();
		void SelectPhase();
		void LoadingPhase();
		void GamePhase();

		bool __IsNotPing();

		void __DownloadMark();
		void __DownloadSymbol(const std::set<uint32_t> & c_rkSet_dwGuildID);

		void __PlayInventoryItemUseSound(TItemPos uSlotPos);
		void __PlayInventoryItemDropSound(TItemPos uSlotPos);
		//void __PlayShopItemDropSound(uint32_t uSlotPos);
		void __PlaySafeBoxItemDropSound(uint32_t uSlotPos);
		void __PlayMallItemDropSound(uint32_t uSlotPos);

		bool __CanActMainInstance();

		enum REFRESH_WINDOW_TYPE
		{
			RefreshStatus = (1 << 0),
			RefreshAlignmentWindow = (1 << 1),
			RefreshCharacterWindow = (1 << 2),
			RefreshEquipmentWindow = (1 << 3), 
			RefreshInventoryWindow = (1 << 4),
			RefreshExchangeWindow = (1 << 5),
			RefreshSkillWindow = (1 << 6),
			RefreshSafeboxWindow  = (1 << 7),
			RefreshMessengerWindow = (1 << 8),
			RefreshGuildWindowInfoPage = (1 << 9),
			RefreshGuildWindowBoardPage = (1 << 10),
			RefreshGuildWindowMemberPage = (1 << 11), 
			RefreshGuildWindowMemberPageGradeComboBox = (1 << 12),
			RefreshGuildWindowSkillPage = (1 << 13),
			RefreshGuildWindowGradePage = (1 << 14),
			RefreshTargetBoard = (1 << 15),
			RefreshMallWindow = (1 << 16),
		};

		void __RefreshStatus();
		void __RefreshAlignmentWindow();
		void __RefreshCharacterWindow();
		void __RefreshEquipmentWindow();
		void __RefreshInventoryWindow();
		void __RefreshExchangeWindow();
		void __RefreshSkillWindow();
		void __RefreshSafeboxWindow();
		void __RefreshMessengerWindow();
		void __RefreshGuildWindowInfoPage();
		void __RefreshGuildWindowBoardPage();
		void __RefreshGuildWindowMemberPage();
		void __RefreshGuildWindowMemberPageGradeComboBox();
		void __RefreshGuildWindowSkillPage();
		void __RefreshGuildWindowGradePage();
		void __RefreshTargetBoardByVID(uint32_t dwVID);
		void __RefreshTargetBoardByName(const char * c_szName);
		void __RefreshTargetBoard();
		void __RefreshMallWindow();

		bool __SendHack(const char* c_szMsg);

	protected:
		bool RecvObserverAddPacket();
		bool RecvObserverRemovePacket();
		bool RecvObserverMovePacket();

		// Common
		bool RecvErrorPacket(int32_t header);
		bool RecvPingPacket();
		bool RecvDefaultPacket(int32_t header);
		bool RecvPhasePacket();

		// Login Phase
		bool __RecvLoginSuccessPacket3();
		bool __RecvLoginSuccessPacket4();
		bool __RecvLoginFailurePacket();
		bool __RecvEmpirePacket();
		bool __RecvLoginKeyPacket();

		// Select Phase
		bool __RecvPlayerCreateSuccessPacket();
		bool __RecvPlayerCreateFailurePacket();
		bool __RecvPlayerDestroySuccessPacket();
		bool __RecvPlayerDestroyFailurePacket();
		bool __RecvPreserveItemPacket();
		bool __RecvPlayerPoints();
		bool __RecvChangeName();

		// Loading Phase
		bool RecvMainCharacter();		
		bool RecvMainCharacter2_EMPIRE();
		bool RecvMainCharacter3_BGM();
		bool RecvMainCharacter4_BGM_VOL();

		void __SetFieldMusicFileName(const char* musicName);
		void __SetFieldMusicFileInfo(const char* musicName, float vol);
		// END_OF_SUPPORT_BGM

		// Main Game Phase
		bool RecvWarpPacket();
		bool RecvPVPPacket();
		bool RecvDuelStartPacket();
        bool RecvGlobalTimePacket();
		bool RecvCharacterAppendPacket();
		bool RecvCharacterAdditionalInfo();
		bool RecvCharacterAppendPacketNew();
		bool RecvCharacterUpdatePacket();
		bool RecvCharacterUpdatePacketNew();
		bool RecvCharacterDeletePacket();
		bool RecvChatPacket();
		bool RecvOwnerShipPacket();
		bool RecvSyncPositionPacket();
		bool RecvWhisperPacket();
		bool RecvPointChange();					// Alarm to python
		bool RecvChangeSpeedPacket();

		bool RecvStunPacket();
		bool RecvDeadPacket();
		bool RecvCharacterMovePacket();

		bool RecvItemSetPacket();					// Alarm to python
		bool RecvItemSetPacket2();					// Alarm to python
		bool RecvItemUsePacket();					// Alarm to python
		bool RecvItemUpdatePacket();				// Alarm to python
		bool RecvItemGroundAddPacket();
		bool RecvItemGroundDelPacket();
		bool RecvItemOwnership();

		bool RecvQuickSlotAddPacket();				// Alarm to python
		bool RecvQuickSlotDelPacket();				// Alarm to python
		bool RecvQuickSlotMovePacket();				// Alarm to python

		bool RecvCharacterPositionPacket();
		bool RecvMotionPacket();

		bool RecvShopPacket();
		bool RecvShopSignPacket();
		bool RecvExchangePacket();

		// Quest
		bool RecvScriptPacket();
		bool RecvQuestInfoPacket();
		bool RecvQuestConfirmPacket();
		bool RecvRequestMakeGuild();

		// Skill
		bool RecvSkillLevel();
		bool RecvSkillLevelNew();
		bool RecvSkillCoolTimeEnd();

		// Target
		bool RecvTargetPacket();
		bool RecvViewEquipPacket();
		bool RecvDamageInfoPacket();

		// Mount
		bool RecvMountPacket();

		// Fly
		bool RecvCreateFlyPacket();
		bool RecvFlyTargetingPacket();
		bool RecvAddFlyTargetingPacket();

		// Messenger
		bool RecvMessenger();

		// Guild
		bool RecvGuild();

		// Party
		bool RecvPartyInvite();
		bool RecvPartyAdd();
		bool RecvPartyUpdate();
		bool RecvPartyRemove();
		bool RecvPartyLink();
		bool RecvPartyUnlink();
		bool RecvPartyParameter();

		// SafeBox
		bool RecvSafeBoxSetPacket();
		bool RecvSafeBoxDelPacket();
		bool RecvSafeBoxWrongPasswordPacket();
		bool RecvSafeBoxSizePacket();
		bool RecvSafeBoxMoneyChangePacket();

		// Fishing
		bool RecvFishing();

		// Dungeon
		bool RecvDungeon();

		// Time
		bool RecvTimePacket();

		// WalkMode
		bool RecvWalkModePacket();

		// ChangeSkillGroup
		bool RecvChangeSkillGroupPacket();

		// Refine
		bool RecvRefineInformationPacket();
		bool RecvRefineInformationPacketNew();

		// Use Potion
		bool RecvSpecialEffect();

		// 서버에서 지정한 이팩트 발동 패킷.
		bool RecvSpecificEffect();
		
		// 용혼석 관련
		bool RecvDragonSoulRefine();

		// MiniMap Info
		bool RecvNPCList();
		bool RecvLandPacket();
		bool RecvTargetCreatePacket();
		bool RecvTargetCreatePacketNew();
		bool RecvTargetUpdatePacket();
		bool RecvTargetDeletePacket();

		// Affect
		bool RecvAffectAddPacket();
		bool RecvAffectRemovePacket();

		// Channel
		bool RecvChannelPacket();

#ifdef ENABLE_ACCE_SYSTEM
		bool	RecvAccePacket(bool bReturn = false);
#endif

		// @fixme007
		bool RecvUnk213();

	protected:
		// 이모티콘
		bool ParseEmoticon(const char * pChatMsg, uint32_t * pdwEmoticon);

		// 파이썬으로 보내는 콜들
		void OnConnectFailure();
		void OnScriptEventStart(int32_t iSkin, int32_t iIndex);
		
		void OnRemoteDisconnect();
		void OnDisconnect();

		void SetGameOnline();
		void SetGameOffline();
		BOOL IsGameOnline();

	protected:
		bool CheckPacket(TPacketHeader * pRetHeader);
		
		void __InitializeGamePhase();
		void __InitializeMarkAuth();
		void __GlobalPositionToLocalPosition(int32_t& rGlobalX, int32_t& rGlobalY);
		void __LocalPositionToGlobalPosition(int32_t& rLocalX, int32_t& rLocalY);

		bool __IsPlayerAttacking();
		bool __IsEquipItemInSlot(TItemPos Cell);

		void __ShowMapName(int32_t lLocalX, int32_t lLocalY);

		void __LeaveOfflinePhase() {}
		void __LeaveHandshakePhase() {}
		void __LeaveLoginPhase() {}
		void __LeaveSelectPhase() {}
		void __LeaveLoadingPhase() {}
		void __LeaveGamePhase();

		void __ClearNetworkActorManager();

		void __ClearSelectCharacterData();

		// DELETEME
		//void __SendWarpPacket();

		void __RecvCharacterAppendPacket(SNetworkActorData * pkNetActorData);
		void __RecvCharacterUpdatePacket(SNetworkUpdateActorData * pkNetUpdateActorData);

		void __FilterInsult(char* szLine, uint32_t uLineLen);

		void __SetGuildID(uint32_t id);

	protected:
		TPacketGCHandshake m_HandshakeData;
		uint32_t m_dwChangingPhaseTime;
		uint32_t m_dwBindupRetryCount;
		uint32_t m_dwMainActorVID;
		uint32_t m_dwMainActorRace;
		uint32_t m_dwMainActorEmpire;
		uint32_t m_dwMainActorSkillGroup;
		BOOL m_isGameOnline;
		BOOL m_isStartGame;

		uint32_t m_dwGuildID;
		uint32_t m_dwEmpireID;
		
		struct SServerTimeSync
		{
			uint32_t m_dwChangeServerTime;
			uint32_t m_dwChangeClientTime;
		} m_kServerTimeSync;

		void __ServerTimeSync_Initialize();
		//uint32_t m_dwBaseServerTime;
		//uint32_t m_dwBaseClientTime;

		uint32_t m_dwLastGamePingTime;

		std::string	m_stID;
		std::string	m_stPassword;
		std::string	m_strLastCommand;
		std::string	m_strPhase;
		uint32_t m_dwLoginKey;
		BOOL m_isWaitLoginKey;

		std::string m_stMarkIP;

		CFuncObject<CPythonNetworkStream>	m_phaseProcessFunc;
		CFuncObject<CPythonNetworkStream>	m_phaseLeaveFunc;

		PyObject*							m_poHandler;
		PyObject*							m_apoPhaseWnd[PHASE_WINDOW_NUM];
		PyObject*							m_poSerCommandParserWnd;

		CCheatDetectQueueMgr		m_kCheatQueueManager;

		TSimplePlayerInformation			m_akSimplePlayerInfo[PLAYER_PER_ACCOUNT4];
		uint32_t								m_adwGuildID[PLAYER_PER_ACCOUNT4];
		std::string							m_astrGuildName[PLAYER_PER_ACCOUNT4];
		bool m_bSimplePlayerInfo;

		CRef<CNetworkActorManager>			m_rokNetActorMgr;

		bool m_isRefreshStatus;
		bool m_isRefreshCharacterWnd;
		bool m_isRefreshEquipmentWnd;
		bool m_isRefreshInventoryWnd;
		bool m_isRefreshExchangeWnd;
		bool m_isRefreshSkillWnd;
		bool m_isRefreshSafeboxWnd;
		bool m_isRefreshMallWnd;
		bool m_isRefreshMessengerWnd;
		bool m_isRefreshGuildWndInfoPage;
		bool m_isRefreshGuildWndBoardPage;
		bool m_isRefreshGuildWndMemberPage;
		bool m_isRefreshGuildWndMemberPageGradeComboBox;
		bool m_isRefreshGuildWndSkillPage;
		bool m_isRefreshGuildWndGradePage;

		// Emoticon
		std::vector<std::string> m_EmoticonStringVector;

		struct STextConvertTable 
		{
			char acUpper[26];
			char acLower[26];
			uint8_t aacHan[5000][2];
		} m_aTextConvTable[3];



		struct SMarkAuth
		{
			CNetworkAddress m_kNetAddr;
			uint32_t m_dwHandle;
			uint32_t m_dwRandomKey;
		} m_kMarkAuth;



		uint32_t m_dwSelectedCharacterIndex;

		CInsultChecker m_kInsultChecker;

		bool m_isEnableChatInsultFilter;
		bool m_bComboSkillFlag;

	private:
		struct SDirectEnterMode
		{
			bool m_isSet;
			uint32_t m_dwChrSlotIndex;
		} m_kDirectEnterMode;

		void __DirectEnterMode_Initialize();
		void __DirectEnterMode_Set(uint32_t uChrSlotIndex);
		bool __DirectEnterMode_IsSet();

	public:
		uint32_t EXPORT_GetBettingGuildWarValue(const char* c_szValueName);
		std::string GetPhase() { return m_strPhase; };

	private:
		struct SBettingGuildWar
		{
			uint32_t m_dwBettingMoney;
			uint32_t m_dwObserverCount;
		} m_kBettingGuildWar;

		CInstanceBase * m_pInstTarget;

		void __BettingGuildWar_Initialize();
		void __BettingGuildWar_SetObserverCount(uint32_t uObserverCount);
		void __BettingGuildWar_SetBettingMoney(uint32_t uBettingMoney);
};
