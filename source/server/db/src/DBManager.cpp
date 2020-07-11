#include "stdafx.h"
#include "DBManager.h"
#include "ClientManager.h"


extern std::string g_stLocale;

CDBManager::CDBManager()
{
}

CDBManager::~CDBManager()
{
}

void CDBManager::Quit()
{
	for (int32_t i = 0; i < SQL_MAX_NUM; ++i)
	{
		m_mainSQL[i].Quit();
		m_asyncSQL[i].Quit();
		m_directSQL[i].Quit();
	}
}

SQLMsg* CDBManager::PopResult()
{
	SQLMsg* p;

	for (int32_t i = 0; i < SQL_MAX_NUM; ++i)
		if (m_mainSQL[i].PopResult(&p))
			return p;

	return nullptr;
}

SQLMsg* CDBManager::PopResult(uint32_t slot)
{
	SQLMsg * p;
	if (m_mainSQL[slot].PopResult(&p))
		return p;

	return nullptr;
}

bool CDBManager::Connect(uint32_t slot, const char* db_address, const int32_t db_port, const char * db_name, const char * user, const char * pwd)
{
	assert(db_address != nullptr && "Argument check");
	assert(db_name != nullptr && "Argument check");
	assert(user != nullptr && "Argument check");
	assert(pwd != nullptr && "Argument check");
	assert(slot < SQL_MAX_NUM && "Out of bounds");

	if (!m_directSQL[slot].Setup(db_address, user, pwd, db_name, g_stLocale.c_str(), true, db_port))
		return false;

	if (!m_mainSQL[slot].Setup(db_address, user, pwd, db_name, g_stLocale.c_str(), false, db_port))
		return false;

	if (!m_asyncSQL[slot].Setup(db_address, user, pwd, db_name, g_stLocale.c_str(), false, db_port))
		return false;

	return true;
}

SQLMsg * CDBManager::DirectQuery(const char * c_pszQuery, uint32_t slot)
{
	assert(slot < SQL_MAX_NUM && "Out of bounds");
	return m_directSQL[slot].DirectQuery(c_pszQuery);
}

void CDBManager::ReturnQuery(const char* c_pszQuery, int32_t iType, uint32_t dwIdent, void * udata, uint32_t slot)
{
	assert(slot < SQL_MAX_NUM && "Out of bounds");

	CQueryInfo * p = new CQueryInfo;

	p->iType = iType;
	p->dwIdent = dwIdent;
	p->pvData = udata;

	m_mainSQL[slot].ReturnQuery(c_pszQuery, p);
}

void CDBManager::AsyncQuery(const char * c_pszQuery, uint32_t slot)
{
	assert(slot < SQL_MAX_NUM && "Out of bounds");
	m_asyncSQL[slot].AsyncQuery(c_pszQuery);
}

uint32_t CDBManager::EscapeString(void *to, const void *from, uint32_t length, uint32_t slot)
{
	assert(slot < SQL_MAX_NUM && "Out of bounds");
	return mysql_real_escape_string(m_directSQL[slot].GetSQLHandle(),
		static_cast<char*>(to),
		static_cast<const char*>(from),
		length);
}

uint32_t CDBManager::CountReturnQuery(uint32_t slot) const
{
	assert(slot < SQL_MAX_NUM && "Out of bounds");
	return m_mainSQL[slot].CountQuery();
}

uint32_t CDBManager::CountReturnResult(uint32_t slot) const
{
	assert(slot < SQL_MAX_NUM && "Out of bounds");
	return m_mainSQL[slot].CountResult();
}

uint32_t CDBManager::CountReturnQueryFinished(uint32_t slot) const
{
	assert(slot < SQL_MAX_NUM && "Out of bounds");
	return m_mainSQL[slot].CountQueryFinished();
}

uint32_t CDBManager::CountAsyncQuery(uint32_t slot) const
{
	assert(slot < SQL_MAX_NUM && "Out of bounds");
	return m_asyncSQL[slot].CountQuery();
}

uint32_t CDBManager::CountAsyncResult(uint32_t slot) const
{
	assert(slot < SQL_MAX_NUM && "Out of bounds");
	return m_asyncSQL[slot].CountResult();
}

uint32_t CDBManager::CountAsyncQueryFinished(uint32_t slot) const
{
	assert(slot < SQL_MAX_NUM && "Out of bounds");
	return m_asyncSQL[slot].CountQueryFinished();
}

void CDBManager::ResetCounter()
{
	for (uint32_t i = 0; i < SQL_MAX_NUM; ++i) {
		m_mainSQL[i].ResetQueryFinished();
		m_asyncSQL[i].ResetQueryFinished();
	}
}

