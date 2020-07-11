#pragma once
#include "packet_info.h"
#include "typedef.h"
#include "../../common/service.h"

enum EInprocState
{
	INPROC_CLOSE,
	INPROC_HANDSHAKE,
	INPROC_LOGIN,
	INPROC_MAIN,
	INPROC_DB,
	INPROC_P2P,
	INPROC_AUTH,
};

void LoginFailure(LPDESC d, const char * c_pszStatus);
extern void SendShout(const char * szText, uint8_t bEmpire);

class CInputProcessor
{
	public:
		CInputProcessor();
		virtual ~CInputProcessor() {};

		virtual bool Process(LPDESC d, const void * c_pvOrig, int32_t iBytes, int32_t & r_iBytesProceed);
		virtual uint8_t GetType() = 0;

		void BindPacketInfo(CPacketInfo * pPacketInfo);
		void Pong(LPDESC d);
		void Handshake(LPDESC d, const char * c_pData);

	protected:
		virtual int32_t	Analyze(LPDESC d, uint8_t bHeader, const char * c_pData) = 0;

		CPacketInfo * m_pPacketInfo;
		int32_t	m_iBufferLeft;

		CPacketInfoCG 	m_packetInfoCG;
};

class CInputClose : public CInputProcessor
{
	public:
		virtual uint8_t	GetType() { return INPROC_CLOSE; }

	protected:
		virtual int32_t	Analyze(LPDESC d, uint8_t bHeader, const char * c_pData) { return m_iBufferLeft; }
};

class CInputHandshake : public CInputProcessor
{
	public:
		CInputHandshake();
		virtual ~CInputHandshake();

		virtual uint8_t	GetType() { return INPROC_HANDSHAKE; }

	protected:
		virtual int32_t	Analyze(LPDESC d, uint8_t bHeader, const char * c_pData);

	protected:
		void		GuildMarkLogin(LPDESC d, const char* c_pData);

		CPacketInfo *	m_pMainPacketInfo;
};

class CInputLogin : public CInputProcessor
{
	public:
		virtual uint8_t	GetType() { return INPROC_LOGIN; }

	protected:
		virtual int32_t	Analyze(LPDESC d, uint8_t bHeader, const char * c_pData);

	protected:
		void		Login(LPDESC d, const char * data);
		void		LoginByKey(LPDESC d, const char * data);

		void		CharacterSelect(LPDESC d, const char * data);
		void		CharacterCreate(LPDESC d, const char * data);
		void		CharacterDelete(LPDESC d, const char * data);
		void		Entergame(LPDESC d, const char * data);
		void		Empire(LPDESC d, const char * c_pData);
		void		GuildMarkCRCList(LPDESC d, const char* c_pData);
		// MARK_BUG_FIX
		void		GuildMarkIDXList(LPDESC d, const char* c_pData);
		// END_OF_MARK_BUG_FIX
		void		GuildMarkUpload(LPDESC d, const char* c_pData);
		int32_t			GuildSymbolUpload(LPDESC d, const char* c_pData, size_t uiBytes);
		void		GuildSymbolCRC(LPDESC d, const char* c_pData);
		void		ChangeName(LPDESC d, const char * data);
};

class CInputMain : public CInputProcessor
{
	public:
		virtual uint8_t	GetType() { return INPROC_MAIN; }

	protected:
		virtual int32_t	Analyze(LPDESC d, uint8_t bHeader, const char * c_pData);

	protected:
		void		Attack(LPCHARACTER ch, const uint8_t header, const char* data);

		int32_t			Whisper(LPCHARACTER ch, const char * data, size_t uiBytes);
		int32_t			Chat(LPCHARACTER ch, const char * data, size_t uiBytes);
		void		ItemUse(LPCHARACTER ch, const char * data);
		void		ItemDrop(LPCHARACTER ch, const char * data);
		void		ItemDrop2(LPCHARACTER ch, const char * data);
		void		ItemMove(LPCHARACTER ch, const char * data);
		void		ItemPickup(LPCHARACTER ch, const char * data);
		void		ItemToItem(LPCHARACTER ch, const char * pcData);
		void		QuickslotAdd(LPCHARACTER ch, const char * data);
		void		QuickslotDelete(LPCHARACTER ch, const char * data);
		void		QuickslotSwap(LPCHARACTER ch, const char * data);
		int32_t			Shop(LPCHARACTER ch, const char * data, size_t uiBytes);
		void		OnClick(LPCHARACTER ch, const char * data);
		void		Exchange(LPCHARACTER ch, const char * data);
		void		Position(LPCHARACTER ch, const char * data);
		void		Move(LPCHARACTER ch, const char * data);
		int32_t			SyncPosition(LPCHARACTER ch, const char * data, size_t uiBytes);
		void		FlyTarget(LPCHARACTER ch, const char * pcData, uint8_t bHeader);
		void		UseSkill(LPCHARACTER ch, const char * pcData);
		
		void		ScriptAnswer(LPCHARACTER ch, const void * pvData);
		void		ScriptButton(LPCHARACTER ch, const void * pvData);
		void		ScriptSelectItem(LPCHARACTER ch, const void * pvData);

		void		QuestInputString(LPCHARACTER ch, const void * pvData);
		void		QuestConfirm(LPCHARACTER ch, const void* pvData);
		void		Target(LPCHARACTER ch, const char * pcData);
		void		TargetDrop(LPCHARACTER ch, const char * pcData);
		void		ChestDropInfo(LPCHARACTER ch, const char * c_pData);
		void		SafeboxCheckin(LPCHARACTER ch, const char * c_pData);
		void		SafeboxCheckout(LPCHARACTER ch, const char * c_pData, bool bMall);
		void		SafeboxItemMove(LPCHARACTER ch, const char * data);
		int32_t			Messenger(LPCHARACTER ch, const char* c_pData, size_t uiBytes);

		void 		PartyInvite(LPCHARACTER ch, const char * c_pData);
		void 		PartyInviteAnswer(LPCHARACTER ch, const char * c_pData);
		void		PartyRemove(LPCHARACTER ch, const char * c_pData);
		void		PartySetState(LPCHARACTER ch, const char * c_pData);
		void		PartyUseSkill(LPCHARACTER ch, const char * c_pData);
		void		PartyParameter(LPCHARACTER ch, const char * c_pData);

		int32_t			Guild(LPCHARACTER ch, const char * data, size_t uiBytes);
		void		AnswerMakeGuild(LPCHARACTER ch, const char* c_pData);

		void		Fishing(LPCHARACTER ch, const char* c_pData);
		void		ItemGive(LPCHARACTER ch, const char* c_pData);
		void		Hack(LPCHARACTER ch, const char * c_pData);
		int32_t			MyShop(LPCHARACTER ch, const char * c_pData, size_t uiBytes);

		void		Refine(LPCHARACTER ch, const char* c_pData);
#ifdef ENABLE_ACCE_SYSTEM
		void		Acce(LPCHARACTER pkChar, const char* c_pData);
#endif
};

class CInputDB : public CInputProcessor
{
public:
	virtual bool Process(LPDESC d, const void * c_pvOrig, int32_t iBytes, int32_t & r_iBytesProceed);
	virtual uint8_t GetType() { return INPROC_DB; }

protected:
	virtual int32_t	Analyze(LPDESC d, uint8_t bHeader, const char * c_pData);

protected:
	void		MapLocations(const char * c_pData);
	void		LoginSuccess(uint32_t dwHandle, const char *data);
	void		PlayerCreateFailure(LPDESC d, uint8_t bType);	// 0 = 일반 실패 1 = 이미 있음
	void		PlayerDeleteSuccess(LPDESC d, const char * data);
	void		PlayerDeleteFail(LPDESC d);
	void		PlayerLoad(LPDESC d, const char* data);
	void		PlayerCreateSuccess(LPDESC d, const char * data);
	void		Boot(const char* data);
	void		QuestLoad(LPDESC d, const char * c_pData);
	void		SafeboxLoad(LPDESC d, const char * c_pData);
	void		SafeboxChangeSize(LPDESC d, const char * c_pData);
	void		SafeboxWrongPassword(LPDESC d);
	void		SafeboxChangePasswordAnswer(LPDESC d, const char* c_pData);
	void		MallLoad(LPDESC d, const char * c_pData);
	void		EmpireSelect(LPDESC d, const char * c_pData);
	void		P2P(const char * c_pData);
	void		ItemLoad(LPDESC d, const char * c_pData);
	void		AffectLoad(LPDESC d, const char * c_pData);
	void		ActivityLoad(LPDESC d, const char* c_pData);

	void		GuildLoad(const char * c_pData);
	void		GuildSkillUpdate(const char* c_pData);
	void		GuildSkillRecharge();
	void		GuildExpUpdate(const char* c_pData);
	void		GuildAddMember(const char* c_pData);
	void		GuildRemoveMember(const char* c_pData);
	void		GuildChangeGrade(const char* c_pData);
	void		GuildChangeMemberData(const char* c_pData);
	void		GuildDisband(const char* c_pData);
	void		GuildLadder(const char* c_pData);
	void		GuildWar(const char* c_pData);
	void		GuildWarScore(const char* c_pData);
	void		GuildSkillUsableChange(const char* c_pData);
	void		GuildMoneyChange(const char* c_pData);
	void		GuildWithdrawMoney(const char* c_pData);
	void		GuildWarReserveAdd(TGuildWarReserve * p);
	void		GuildWarReserveUpdate(TGuildWarReserve * p);
	void		GuildWarReserveDelete(uint32_t dwID);
	void		GuildWarBet(TPacketGDGuildWarBet * p);
	void		GuildChangeMaster(TPacketChangeGuildMaster* p);

	void		LoginAlready(LPDESC d, const char * c_pData);

	void		PartyCreate(const char* c_pData);
	void		PartyDelete(const char* c_pData);
	void		PartyAdd(const char* c_pData);
	void		PartyRemove(const char* c_pData);
	void		PartyStateChange(const char* c_pData);
	void		PartySetMemberLevel(const char* c_pData);

	void		Time(const char * c_pData);

	void		ReloadProto(const char * c_pData);
	void		ChangeName(LPDESC d, const char * data);

	void		AuthLogin(LPDESC d, const char * c_pData);
	void		ItemAward(const char * c_pData);

	void		ChangeEmpirePriv(const char* c_pData);
	void		ChangeGuildPriv(const char* c_pData);
	void		ChangeCharacterPriv(const char* c_pData);

	void		SetEventFlag(const char* c_pData);

	void		CreateObject(const char * c_pData);
	void		DeleteObject(const char * c_pData);

	void		Notice(const char * c_pData);

	void		MarriageAdd(TPacketMarriageAdd * p);
	void		MarriageUpdate(TPacketMarriageUpdate * p);
	void		MarriageRemove(TPacketMarriageRemove * p);

	void		WeddingRequest(TPacketWeddingRequest* p);
	void		WeddingReady(TPacketWeddingReady* p);
	void		WeddingStart(TPacketWeddingStart* p);
	void		WeddingEnd(TPacketWeddingEnd* p);

	// MYSHOP_PRICE_LIST
	/// 아이템 가격정보 리스트 요청에 대한 응답 패킷(HEADER_DG_MYSHOP_PRICELIST_RES) 처리함수
	/**
	* @param	d 아이템 가격정보 리스트를 요청한 플레이어의 descriptor
	* @param	p 패킷데이터의 포인터
	*/
	void		MyshopPricelistRes( LPDESC d, const TPacketMyshopPricelistHeader* p );
	// END_OF_MYSHOP_PRICE_LIST
	//
	//RELOAD_ADMIN
	void ReloadAdmin( const char * c_pData );
	//END_RELOAD_ADMIN

	void		DetailLog(const TPacketNeedLoginLogInfo* info);
	// 독일 선물 기능 테스트
	void		ItemAwardInformer(TPacketItemAwardInfromer* data);

	void		RespondChannelStatus(LPDESC desc, const char* pcData);

	protected:
		uint32_t		m_dwHandle;
};

class CInputP2P : public CInputProcessor
{
	public:
		CInputP2P();
		virtual uint8_t	GetType() { return INPROC_P2P; }

	protected:
		virtual int32_t	Analyze(LPDESC d, uint8_t bHeader, const char * c_pData);

	public:
		void		Setup(LPDESC d, const char * c_pData);
		void		Login(LPDESC d, const char * c_pData);
		void		Logout(LPDESC d, const char * c_pData);
		int32_t			Relay(LPDESC d, const char * c_pData, size_t uiBytes);
		int32_t			Notice(LPDESC d, const char * c_pData, size_t uiBytes, bool bBigFont=false);
		int32_t			Guild(LPDESC d, const char* c_pData, size_t uiBytes);
		void		Shout(const char * c_pData);
		void		Disconnect(const char * c_pData);
		void		MessengerAdd(const char * c_pData);
		void		MessengerRemove(const char * c_pData);
		void		FindPosition(LPDESC d, const char* c_pData);
		void		WarpCharacter(const char* c_pData);
		void		GuildWarZoneMapIndex(const char* c_pData);
		void		Transfer(const char * c_pData);
		void		XmasWarpSanta(const char * c_pData);
		void		XmasWarpSantaReply(const char * c_pData);
		void		LoginPing(LPDESC d, const char * c_pData);
		void		BlockChat(const char * c_pData);
		void		IamAwake(LPDESC d, const char * c_pData);

		void		UpdateRights(const char * c_pData);

		void		MessengerRequest(const char * c_pData);
		void		MessengerRequestFail(const char * c_pData);


	protected:
		CPacketInfoGG 	m_packetInfoGG;
};

class CInputAuth : public CInputProcessor
{
	public:
		CInputAuth();
		virtual uint8_t GetType() { return INPROC_AUTH; }

	protected:
		virtual int32_t	Analyze(LPDESC d, uint8_t bHeader, const char * c_pData);

	public:
		void		Login(LPDESC d, const char * c_pData);

};
