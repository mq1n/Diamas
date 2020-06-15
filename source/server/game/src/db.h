#ifndef __INC_METIN_II_DB_MANAGER_H__
#define __INC_METIN_II_DB_MANAGER_H__

#include "../../libsql/include/AsyncSQL.h"
#include "../../common/length.h"
#include <functional>

enum EQuertyType
{
	QUERY_TYPE_RETURN			= 0,
	QUERY_TYPE_FUNCTION			= 1,
	QUERY_TYPE_AFTER_FUNCTION	= 2,
};

enum EQid
{
	QID_SAFEBOX_SIZE			= 0,
	QID_AUTH_LOGIN				= 1,
	QID_BLOCK_CHAT_LIST			= 2,
};

typedef struct SUseTime
{
	uint32_t	dwLoginKey;
	char        szLogin[LOGIN_MAX_LEN+1];
	uint32_t       dwUseSec;
	char        szIP[MAX_HOST_LENGTH+1];
} TUseTime;

class CQueryInfo
{
	public:
		int32_t	iQueryType;
};

class CReturnQueryInfo : public CQueryInfo
{
	public:
		int32_t	iType;
		uint32_t	dwIdent;
		void			*	pvData;
};

class CFuncQueryInfo : public CQueryInfo
{
	public:
	std::function<void(SQLMsg*)> f;
};

class CFuncAfterQueryInfo : public CQueryInfo
{
	public:
	std::function<void()> f;
};

class CLoginData;


class DBManager : public singleton<DBManager>
{
	public:
		DBManager();
		virtual ~DBManager();

		bool			IsConnected();

		bool			Connect(const char * host, const int32_t port, const char * user, const char * pwd, const char * db);
		void			Query(const char * c_pszFormat, ...);

		SQLMsg *		DirectQuery(const char * c_pszFormat, ...);
		void			ReturnQuery(int32_t iType, uint32_t dwIdent, void* pvData, const char * c_pszFormat, ...);

		void			Process();
		void			AnalyzeReturnQuery(SQLMsg * pmsg);

		void			LoginPrepare(LPDESC d, uint32_t * pdwClientKey, int32_t * paiPremiumTimes = nullptr);
		void			SendAuthLogin(LPDESC d);
		void			SendLoginPing(const char * c_pszLogin);

		void			InsertLoginData(CLoginData * pkLD);
		void			DeleteLoginData(CLoginData * pkLD);
		CLoginData *	GetLoginData(uint32_t dwKey);

		uint32_t			CountQuery()		{ return m_sql.CountQuery(); }
		uint32_t			CountQueryResult()	{ return m_sql.CountResult(); }
		void			ResetQueryResult()	{ m_sql.ResetQueryFinished(); }

		void FuncQuery(std::function<void(SQLMsg*)> f, const char * c_pszFormat, ...); 
		void FuncAfterQuery(std::function<void()> f, const char * c_pszFormat, ...);

		size_t EscapeString(char* dst, size_t dstSize, const char *src, size_t srcSize);

	private:
		SQLMsg *				PopResult();

		CAsyncSQL				m_sql;
		CAsyncSQL				m_sql_direct;
		bool					m_bIsConnect;

		std::map<uint32_t, CLoginData *>		m_map_pkLoginData;
		std::vector<TUseTime>			m_vec_kUseTime;
};

// ACCOUNT_DB
class AccountDB : public singleton<AccountDB>
{
	public:
		AccountDB();

		bool IsConnected();
		bool Connect(const char * host, const int32_t port, const char * user, const char * pwd, const char * db);
		bool ConnectAsync(const char * host, const int32_t port, const char * user, const char * pwd, const char * db, const char * locale);

		SQLMsg* DirectQuery(const char * query);		
		void ReturnQuery(int32_t iType, uint32_t dwIdent, void * pvData, const char * c_pszFormat, ...);
		void AsyncQuery(const char* query);

		void SetLocale(const std::string & stLocale);

		void Process();

	private:
		SQLMsg * PopResult();
		void AnalyzeReturnQuery(SQLMsg * pMsg);

		CAsyncSQL2	m_sql_direct;
		CAsyncSQL2	m_sql;
		bool		m_IsConnect;

};
//END_ACCOUNT_DB

#endif
