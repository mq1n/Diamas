#pragma once

class CLoginData
{
	public:
		CLoginData();

		void            SetClientKey(const uint32_t * c_pdwClientKey);

		const uint32_t *   GetClientKey();
		void            SetKey(uint32_t dwKey);
		uint32_t           GetKey();

		void		SetLogin(const char * c_pszLogin);
		const char *	GetLogin();

		void            SetConnectedPeerHandle(uint32_t dwHandle);
		uint32_t		GetConnectedPeerHandle();

		void            SetLogonTime();
		uint32_t		GetLogonTime();

		void		SetIP(const char * c_pszIP);
		const char *	GetIP();

		void		SetRemainSecs(int32_t l);
		int32_t		GetRemainSecs();

		void		SetDeleted(bool bSet);
		bool		IsDeleted();

		void		SetPremium(int32_t * paiPremiumTimes);
		int32_t		GetPremium(uint8_t type);
		int32_t *		GetPremiumPtr();


	private:
		uint32_t           m_dwKey;
		uint32_t           m_adwClientKey[4];
		uint32_t           m_dwConnectedPeerHandle;
		uint32_t           m_dwLogonTime;
		char		m_szIP[MAX_HOST_LENGTH+1];
		bool		m_bDeleted;
		std::string	m_stLogin;
		int32_t		m_aiPremiumTimes[PREMIUM_MAX_NUM];
};
