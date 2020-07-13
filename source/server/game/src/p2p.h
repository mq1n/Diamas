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

		// �Ʒ� Register* Unregister* pair���� ���������� ��� ���� ��ƾ�� ����Ѵ�.
		// ���� ��������� ǥ���ϱ� ���� ��
		void			RegisterAcceptor(LPDESC d);
		void			UnregisterAcceptor(LPDESC d);

		void			RegisterConnector(LPDESC d);
		void			UnregisterConnector(LPDESC d);

		void			EraseUserByDesc(LPDESC d);	// �ش� desc�� �ִ� �������� �����.

		void			FlushOutput();

		void			Boot(LPDESC d);	// p2p ó���� �ʿ��� ������ �����ش�. (�� ĳ������ �α��� ���� ��)

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
