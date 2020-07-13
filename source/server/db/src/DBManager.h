#pragma once
#include <mysql/mysql.h>
#include "../../../common/common_incl.hpp"
#include "../../libsql/include/AsyncSQL.h"
#include <memory>

#define SQL_SAFE_LENGTH(size)	(size * 2 + 1)
#define QUERY_SAFE_LENGTH(size)	(1024 + SQL_SAFE_LENGTH(size))

struct CQueryInfo
{
	int32_t	iType;
	uint32_t	dwIdent;
	void* pvData;
};

enum eSQL_SLOT
{
	SQL_PLAYER,
	SQL_ACCOUNT,
	SQL_COMMON,
	SQL_MAX_NUM,
};

class CDBManager : public CSingleton<CDBManager>
{
public:
	CDBManager();
	virtual ~CDBManager();

	void Quit();

	bool Connect(uint32_t slot, const char * host, int32_t port, const char* dbname, const char* user, const char* pass);

	void ReturnQuery(const char * c_pszQuery, int32_t iType, uint32_t dwIdent, void * pvData, uint32_t slot = SQL_PLAYER);
	void AsyncQuery(const char * c_pszQuery, uint32_t slot = SQL_PLAYER);
	SQLMsg * DirectQuery(const char * c_pszQuery, uint32_t slot = SQL_PLAYER);

	SQLMsg * PopResult();
	SQLMsg * PopResult(uint32_t slot);

	uint32_t EscapeString(void * to, const void * from, uint32_t length, uint32_t slot = SQL_PLAYER);

	uint32_t CountReturnQuery(uint32_t slot) const;
	uint32_t CountReturnResult(uint32_t slot) const;
	uint32_t CountReturnQueryFinished(uint32_t slot) const;

	uint32_t CountAsyncQuery(uint32_t slot) const;
	uint32_t CountAsyncResult(uint32_t slot) const;
	uint32_t CountAsyncQueryFinished(uint32_t slot) const;

	void ResetCounter();

private:
	CAsyncSQL m_mainSQL[SQL_MAX_NUM];
	CAsyncSQL m_directSQL[SQL_MAX_NUM];
	CAsyncSQL m_asyncSQL[SQL_MAX_NUM];
};
