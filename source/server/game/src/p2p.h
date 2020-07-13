#pragma once
#include <unordered_map>
#include "input.h"
#include "../../../common/stl.h"

typedef struct _CCI
{
	char	szName[CHARACTER_NAME_MAX_LEN + 1];
	uint32_t	dwPID;
	uint8_t	bEmpire;
	int32_t	lMapIndex;
	uint8_t	bChannel;

	LPDESC	pkDesc;
	int32_t		iLevel;
} CCI;

class P2P_MANAGER : public CSingleton<P2P_MANAGER>
{
	typedef std::unordered_map<std::string, CCI *, stringhash> TCCIMap;
	typedef std::unordered_map<uint32_t, CCI*> TPIDCCIMap;

	public:
		P2P_MANAGER();
		~P2P_MANAGER();

		// 아래 Register* Unregister* pair들은 내부적으로 사실 같은 루틴을 사용한다.
		// 단지 명시적으로 표시하기 위한 것
		void			RegisterAcceptor(LPDESC d);
		void			UnregisterAcceptor(LPDESC d);

		void			RegisterConnector(LPDESC d);
		void			UnregisterConnector(LPDESC d);

		void			EraseUserByDesc(LPDESC d);	// 해당 desc에 있는 유저들을 지운다.

		void			FlushOutput();

		void			Boot(LPDESC d);	// p2p 처리에 필요한 정보를 보내준다. (전 캐릭터의 로그인 정보 등)

		void			Send(const void * c_pvData, int32_t iSize, LPDESC except = nullptr);
		bool			Send(const void * c_pvData, int32_t iSize, uint16_t wPort, bool bP2PPort = false);
		void			SendByPID(uint32_t dwPID, const void * c_pvData, int32_t iSize);

		void			Login(LPDESC d, const TPacketGGLogin * p);
		void			Logout(const char * c_pszName);

		CCI *			Find(const char * c_pszName);
		CCI *			FindByPID(uint32_t pid);

		int32_t				GetCount();
		int32_t				GetEmpireUserCount(int32_t idx);
		int32_t				GetDescCount();
		void			GetP2PHostNames(std::string& hostNames);

		const TPIDCCIMap*	GetP2PCCIMap() { return &m_map_dwPID_pkCCI; }

		LPDESC			GetP2PDescByMapIndex(int32_t lMapIndex);

	private:
		void			Logout(CCI * pkCCI);

		CInputProcessor *	m_pkInputProcessor;
		int32_t			m_iHandleCount;

		std::unordered_set<LPDESC> m_set_pkPeers;
		TCCIMap			m_map_pkCCI;
		TPIDCCIMap		m_map_dwPID_pkCCI;
		int32_t			m_aiEmpireUserCount[EMPIRE_MAX_NUM];
};
