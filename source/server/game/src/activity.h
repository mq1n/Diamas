#pragma once

const int32_t MOB_KILLS_FOR_ACTIVITY_AWARD = 20;
const int32_t ACTIVITY_CAP = 100;
const int32_t MAX_ACTIVITY = ACTIVITY_CAP * 10;
const int32_t REMEMBER_PVP_KILLS = 10;

enum ActivityType 
{
	ACTIVITY_PVP, //Kingdom vs Kingdom & Player vs Player
	ACTIVITY_GK, //Guild
	ACTIVITY_PVM, //Metins, bosses, monster killing
	ACTIVITY_OTHER_PVE, //Fishing, mining
	
	MAX_ACTIVITY_TYPE
};

class ActivityHandler
{
public:
	static std::map<uint32_t, std::vector<uint32_t>> killMap; //<pid, {killedPID, killedPID2, ..., killedPID10}>
	static bool CountsForActivity(LPCHARACTER killer, LPCHARACTER victim);
	static void RecordKill(uint32_t killerPID, uint32_t victimPID);

public:
	ActivityHandler(LPCHARACTER player);

	//Makers
	void MarkMonsterKill(LPCHARACTER mob);
	void MarkFishing(bool success);
	void MarkPlayerKill(LPCHARACTER killed, bool isUnderGW);
	void MarkMining();

	//Loading
	void Load(TActivityTable * data);
	
	//Setters & getters
	int32_t GetActivity() const { return m_activityTotal; };
	void AlterActivity(int32_t var) { m_activityTotal += var; };

	//Saver
	void Save();

private:
	//Helpers
	double ActivityDeltaByLevelDiff(int32_t diff);

	//Binding character
	LPCHARACTER m_player;
	LPCHARACTER GetPlayer() const { return m_player; }

	//Control activity
	void AddPoints(ActivityType type, double am);
	void ActivityTimeCheck(const time_t curTime);

	std::array<int32_t, MAX_ACTIVITY_TYPE> m_activityToday;
	int32_t m_activityTotal;
	int32_t m_monstersKilled;
	time_t m_lastUpdate;

	/*
		Whether we sould be sending a packet to the server on Save() or not.
		Set to true when something is changed, set to false while there are no changes.
	*/
	bool m_doSave;
};

typedef std::shared_ptr <ActivityHandler> spActivityHandler;
