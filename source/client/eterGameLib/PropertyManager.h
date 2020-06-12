#pragma once
#include "Property.h"

class CPropertyManager : public CSingleton<CPropertyManager>
{
	using TPropertyCRCMap	= std::map<uint32_t, CProperty*>;
	using TCRCSet			= std::set<uint32_t>;

public:
	CPropertyManager();
	virtual ~CPropertyManager();

	void			Clear();

	void			ReserveCRC(uint32_t dwCRC);
	uint32_t		GetUniqueCRC(const char * c_szSeed);

	bool			Initialize(const std::string& listFilename);
	bool			Register(const char * c_pszFileName, CProperty ** ppProperty = nullptr);

	bool			Get(uint32_t dwCRC, CProperty ** ppProperty);
	bool			Get(const char * c_pszFileName, CProperty ** ppProperty);

	bool			Erase(uint32_t dwCRC);
	bool			Erase(const char* c_pszFileName);

protected:
	bool			m_bInitialized;
	TPropertyCRCMap								m_PropertyByCRCMap;
	TCRCSet										m_ReservedCRCSet;
};
