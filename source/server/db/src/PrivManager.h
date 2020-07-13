#pragma once
#include "Peer.h"
#include <queue>
#include <utility>

struct TPrivEmpireData
{
    uint8_t type;
    int32_t value;
    bool bRemoved;
    uint8_t empire;
    time_t end_time_sec;

    TPrivEmpireData(uint8_t type, int32_t value, uint8_t empire, time_t end_time_sec) :
		type(type), value(value), bRemoved(false), empire(empire), end_time_sec(end_time_sec)
    {
	}
};

struct TPrivGuildData
{
    uint8_t type;
    int32_t value;
    bool bRemoved;
    uint32_t guild_id;
    time_t end_time_sec;

    TPrivGuildData(uint8_t type, int32_t value, uint32_t guild_id, time_t _end_time_sec) :
		type(type), value(value), bRemoved(false), guild_id(guild_id), end_time_sec(_end_time_sec )
    {
	}
};

struct TPrivCharData
{
    uint8_t type;
    int32_t value;
    bool bRemoved;
    uint32_t pid;

    TPrivCharData(uint8_t type, int32_t value, uint32_t pid) :
		type(type), value(value), bRemoved(false), pid(pid)
    {
	}
};

class CPrivManager : public CSingleton<CPrivManager>
{
    public: 
	CPrivManager();
	virtual ~CPrivManager();

	// ADD_GUILD_PRIV_TIME
	void AddGuildPriv(uint32_t guild_id, uint8_t type, int32_t value, time_t time_sec);
	// END_OF_ADD_GUILD_PRIV_TIME

	// ADD_EMPIRE_PRIV_TIME	
	void AddEmpirePriv(uint8_t empire, uint8_t type, int32_t value, time_t time_sec);
	// END_OF_ADD_EMPIRE_PRIV_TIME
	
	void AddCharPriv(uint32_t pid, uint8_t type, int32_t value);

	void Update();

	void SendPrivOnSetup(CPeer* peer);

private:

	// ADD_GUILD_PRIV_TIME
	void SendChangeGuildPriv(uint32_t guild_id, uint8_t type, int32_t value, time_t end_time_sec);
	// END_OF_ADD_GUILD_PRIV_TIME

	// ADD_EMPIRE_PRIV_TIME	
	void SendChangeEmpirePriv(uint8_t empire, uint8_t type, int32_t value, time_t end_time_sec);
	// END_OF_ADD_EMPIRE_PRIV_TIME

	void SendChangeCharPriv(uint32_t pid, uint8_t type, int32_t value);

	typedef std::pair<time_t, TPrivCharData *> stPairChar;
	typedef std::pair<time_t, TPrivGuildData*> stPairGuild;
	typedef std::pair<time_t, TPrivEmpireData*> stPairEmpire;

	std::priority_queue<stPairChar, std::vector<stPairChar>, std::greater<stPairChar> >
	    m_pqPrivChar;
	std::priority_queue<stPairGuild, std::vector<stPairGuild>, std::greater<stPairGuild> > 
	    m_pqPrivGuild;
	std::priority_queue<stPairEmpire, std::vector<stPairEmpire>, std::greater<stPairEmpire> >
	    m_pqPrivEmpire;

	TPrivEmpireData* m_aaPrivEmpire[MAX_PRIV_NUM][EMPIRE_MAX_NUM];
	std::map<uint32_t, TPrivGuildData*> m_aPrivGuild[MAX_PRIV_NUM];
	std::map<uint32_t, TPrivCharData*> m_aPrivChar[MAX_PRIV_NUM];
};
