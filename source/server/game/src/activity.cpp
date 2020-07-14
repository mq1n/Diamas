#include "stdafx.h"
#include "activity.h"
#include "char.h"
#include "desc_client.h"

std::map<uint32_t, std::vector<uint32_t>> ActivityHandler::killMap;

ActivityHandler::ActivityHandler(LPCHARACTER player) : m_activityTotal(0), m_lastUpdate(std::time(nullptr)), m_doSave(false)
{
	m_player = player;
	m_activityToday = {{ 0 }};
};

void ActivityHandler::MarkMonsterKill(LPCHARACTER mob)
{
	double delta = ActivityDeltaByLevelDiff(GetPlayer()->GetLevel() - mob->GetLevel());

	//Metin
	if (mob->IsStone())
	{
		AddPoints(ACTIVITY_PVM, ACTIVITY_CAP * 1.0 / 250.0 * delta);
	}
	//Boss
	else if (mob->GetMobRank() >= MOB_RANK_BOSS)
	{
		// Lv 5 mob will give 1/3 * cap in best case scenario
		// Boss will give 1/4 * cap in best case scenario

		AddPoints(ACTIVITY_PVM, 1.0 / (3 + 5 - mob->GetMobRank()) * ACTIVITY_CAP * 1 / 100 * delta);
	}
	//No boss nor metin
	else 
	{
		++m_monstersKilled;
		if (m_monstersKilled > MOB_KILLS_FOR_ACTIVITY_AWARD)
		{
			m_monstersKilled = 0;
			AddPoints(ACTIVITY_PVM, 1.0 / 5.0 * static_cast<double>(ACTIVITY_CAP) * 1.0 / 100.0 * delta);
		}
	}
}

void ActivityHandler::MarkPlayerKill(LPCHARACTER victim, bool isUnderGW)
{
	if (!victim) 
	{
		sys_err("Killed nullptr!");
		return;
	}

	if (!CountsForActivity(GetPlayer(), victim))
		return;

	AddPoints(isUnderGW ? ACTIVITY_GK : ACTIVITY_PVP, ACTIVITY_CAP * 0.5 / 100 * ActivityDeltaByLevelDiff(GetPlayer()->GetLevel() - victim->GetLevel()));
	RecordKill(GetPlayer()->GetPlayerID(), victim->GetPlayerID());
}

void ActivityHandler::MarkFishing(bool success)
{
	double rate = success ? 1.0 / 12 : 1.0 / 50;
	AddPoints(ACTIVITY_OTHER_PVE, ACTIVITY_CAP * rate);
}

void ActivityHandler::MarkMining()
{
	AddPoints(ACTIVITY_OTHER_PVE, ACTIVITY_CAP * 1 / 10);
}

void ActivityHandler::Load(TActivityTable * data)
{
	m_activityToday[ACTIVITY_PVP] = data->today.pvp;
	m_activityToday[ACTIVITY_GK] = data->today.gk;
	m_activityToday[ACTIVITY_PVM] = data->today.pve;
	m_activityToday[ACTIVITY_OTHER_PVE] = data->today.other;
	
	m_activityTotal = data->total;
	m_lastUpdate = data->lastUpdate;
	
	//Run a quick activity check (daily loss/counter reset)
	time_t now = std::time(nullptr);

	ActivityTimeCheck(now);
}

/*
	diff is expected to be MYLEVEL - TARGET
*/
double ActivityHandler::ActivityDeltaByLevelDiff(int32_t diff)
{
	//Negative difference is a good thing. We are below the target's level
	diff = MINMAX(-15, diff, 15);

	int32_t delta[] =
	{
		100,	//-15
		98,		//-14
		94,		//-13
		91,		//-12
		89,		//-11
		86,		//-10
		80,		//-9
		76,		//-8
		72,		//-7
		68,		//-6
		61,		//-5
		54,		//-4
		48,		//-3
		40,		//-2
		33,		//-1
		33,		//0
		30,		//+1
		28,		//+2
		26,		//+3
		24,		//+4
		22,		//+5
		20,		//+6
		18,		//+7
		16,		//+8
		14,		//+9
		12,		//+10
		5,		//+11
		4,		//+12
		3,		//+13
		2,		//+14
		1,		//+15
	};

	return delta[diff + 15];
}

void ActivityHandler::AddPoints(ActivityType type, double am)
{
	if (type >= MAX_ACTIVITY_TYPE)
		return;

	//Current time
	time_t now = std::time(nullptr);
	ActivityTimeCheck(now);

	//Add the points
	int32_t iAm = MINMAX(0, static_cast<int32_t>(am), ACTIVITY_CAP - m_activityToday[type]);
	if (iAm < 1)
	{
		sys_log(1, "Activity cap already reached for type %d (Trying to add %d on player %s)", type, iAm, GetPlayer()->GetName());
		return;
	}

	m_activityToday[type] += iAm;
	m_activityTotal = MINMAX(0, m_activityTotal + iAm, MAX_ACTIVITY);

	//Set last update to now
	m_lastUpdate = now;

	//Mark to save
	m_doSave = true;
}

void ActivityHandler::Save()
{
	if (!m_doSave)
		return;

	TActivityTable table;
	table.pid = GetPlayer()->GetPlayerID();
	table.lastUpdate = m_lastUpdate;

	table.today.pve = m_activityToday[ACTIVITY_PVM];
	table.today.pvp = m_activityToday[ACTIVITY_PVP];
	table.today.gk = m_activityToday[ACTIVITY_GK];
	table.today.other = m_activityToday[ACTIVITY_OTHER_PVE];
	
	table.total = std::min(m_activityTotal, MAX_ACTIVITY);

	db_clientdesc->DBPacket(HEADER_GD_SAVE_ACTIVITY, 0, &table, sizeof(TActivityTable));

	//No need to save again until something changes
	m_doSave = false;
}

void ActivityHandler::ActivityTimeCheck(const time_t timestamp)
{
	//If different year, or different day of the year...
	tm curTime = *std::gmtime(&timestamp);
	tm lastUpdate = *std::gmtime(&m_lastUpdate);

	if (curTime.tm_year > lastUpdate.tm_year || curTime.tm_yday > lastUpdate.tm_yday)
	{
		//...reset today's activity caps
		m_activityToday[ACTIVITY_PVM] = 0;
		m_activityToday[ACTIVITY_PVP] = 0;
		m_activityToday[ACTIVITY_GK] = 0;
		m_activityToday[ACTIVITY_OTHER_PVE] = 0;

		//...reduce activity for each day passed
		if (m_activityTotal > 0)
			m_activityTotal -= ACTIVITY_CAP / 2 * ((curTime.tm_year - lastUpdate.tm_year) * 365 + curTime.tm_yday - lastUpdate.tm_yday);

		m_activityTotal = std::max(0, m_activityTotal);
		
		//Mark last update
		m_lastUpdate = timestamp;

		//Mark to save
		m_doSave = true;
	}
}

bool ActivityHandler::CountsForActivity(LPCHARACTER killer, LPCHARACTER victim)
{
	if (!killer->IsPC() || !victim->IsPC())
		return false;
	
	//Same pc - doesn't count
#if 0
	if (killer->GetDesc()->GetHWIDHash() == victim->GetDesc()->GetHWIDHash())
	{
		sys_log(0, "Char %s dueling char %s which is running on the same pc - No activity awarded", killer->GetName(), victim->GetName());
		return false;
	}
#endif
		
	auto it = killMap.find(killer->GetPlayerID()); //Find killer in map
	if (it == killMap.end())
		return true;

	//Find victim in recent ones. Only if not found it will count
	return std::find(it->second.begin(), it->second.end(), victim->GetPlayerID()) == it->second.end();
}

void ActivityHandler::RecordKill(uint32_t killerPID, uint32_t victimPID)
{
	auto it = killMap.find(killerPID); //Find killer in map
	if (it == killMap.end())
	{
		killMap.insert({ killerPID, { victimPID } });
		return;
	}

	std::vector<uint32_t>& victims = it->second;
	if (std::find(victims.begin(), victims.end(), victimPID) != victims.end()) //Sanity check. Should not happen.
	{
		sys_err("Trying to record activity kill even though the victim is already there.");
		return;
	}

	//Cap reached, remove the first
	if (victims.size() >= REMEMBER_PVP_KILLS)
		victims.erase(victims.begin()); 

	victims.push_back(victimPID);
}
