#include "stdafx.h"
#include "abuse.h"
#include "config.h"
#include "utils.h"
#include "char.h"
#include "desc.h"
#include "sectree.h"
#include "log.h"

#include <algorithm>
#include <string>

AbuseController::AbuseController(LPCHARACTER ch): m_mshStartZ(0), m_pickupBotSuspectCount(0)
{
	m_ch = ch;
	m_lastUpdatePulse = 0;
	m_lastAttackAnalysis = 0;

	//Mob pulling
	m_lastSuspiciousAttackTime = 0;
	m_nextAttackCooldown = 0;
	m_nextDetectStart = 0;
	m_vecPullInfo.clear();

	//Ghost mode
	m_lastDeathTime = 0;
	m_nextDeadWalkLog = 0;
	m_nextDeadPickupLog = 0;

	//Wallhacking
	m_whIgnoreUntil = 0;
	m_whStartX = 0;
	m_whStartY = 0;
	m_whStartZ = 0;
	m_whHackCount = 0;
	m_whLastDetection = 0;
	m_whContinuousInvalid = 0;
	m_whNextDecrease = 0;

	//Movement speedhacking
	m_mshStartDetect = 0;
	m_mshLastDetect = 0;
	m_mshFirstDetect = 0;
	m_mshStartX = 0;
	m_mshStartY = 0;
	m_mshIgnoreUntil = 0;
	m_mshRates.clear();

	//Fishbotting
	m_isSuspectedFishbot = false;
	m_fishesLeftToBubbleHide = 0;
	m_bubbleHideTimes = 0;
	m_bubbleHideTimesLeft = 0;

	//WaitDmg
	m_waitDmgSuspect = false;
	m_lastMoveAttackPacketTime = 0;
	m_ignoreWaitDmgUntil = 0;
}

void AbuseController::SuspiciousAttackCooldown()
{
	m_lastSuspiciousAttackTime = 0;
	m_vecPullInfo.clear();
}

void AbuseController::InitiatedFight(const GPOS& mobPos, bool aggressive)
{
	uint32_t now = get_unix_ms_time();

	if (m_nextDetectStart > now)
		return;

	m_lastSuspiciousAttackTime = now;

	// Fetch our position
	const GPOS& playerPos = m_ch->GetXYZ();

	// Store this attack
	PullInfo pinfo;
	pinfo.mobPos = mobPos;
	pinfo.playerPos = playerPos;
	pinfo.isAggressive = aggressive;
	m_vecPullInfo.push_back(pinfo);
}

/*
This function analyzes static player distances to monsters and determines
whether they are legal or not.
*/
bool analyzeStaticMov(const std::vector<int32_t>& distances, std::string& log)
{
	if (distances.size() < 5)
		return false;

	int32_t average = 0;
	for (const int32_t& d : distances)
		average += d;

	average /= distances.size();

	if (average > 1000) {
		log += " static" + std::to_string(distances.size())
			+ " avgstatic" + std::to_string(average);

		return true;
	}

	return false;
};

void AbuseController::AnalyzeAttacks()
{
	m_lastAttackAnalysis = thecore_pulse();

	if (m_vecPullInfo.size() < 5) { //Not worth analyzing, easy to get false positives. 
		if (!m_vecPullInfo.empty() && m_lastSuspiciousAttackTime + 3 * 1000 < get_unix_ms_time())
			SuspiciousAttackCooldown();

		return;
	}

	/*
	Define monster clusters: We arbitrarily pick a monster and, given
	small radius around it, group all the monsters as a single one.

	Count the aggressive ones while we are at it
	*/
	std::vector<GPOS> monsterClusters;
	int32_t aggresiveMobs = 0;

	for (auto & i : m_vecPullInfo)
	{
		const GPOS& mobPos = i.mobPos;
		if (i.isAggressive)
			++aggresiveMobs;

		bool fit = false;
		for (const auto c : monsterClusters)
		{
			if (DISTANCE_APPROX(c.x - mobPos.x, c.y - mobPos.y) < 200) {
				fit = true;
				break;
			}
		}

		//We couldn't fit this mob into a cluster. Create a new cluster centered on it.
		if (!fit) {
			GPOS cluster(mobPos.x, mobPos.y, 0);
			monsterClusters.push_back(cluster);
		}
	}

	/*
	Compare distances between the different monster
	clusters that we have generated above (each to each).
	Afterwards, extract average/median.
	*/
	std::vector<int32_t> distances;
	for (size_t i = 0, size = monsterClusters.size(); i < size; ++i)
	{
		for (size_t j = i + 1; j < size; ++j)
		{
			int32_t dist = DISTANCE_APPROX(monsterClusters[j].x - monsterClusters[i].x, monsterClusters[j].y - monsterClusters[i].y);
			distances.push_back(dist);
		}
	}

	size_t distancesCount = distances.size();

	int32_t median = getMedian(distances, distancesCount);
	int32_t average = getAverage(distances, distancesCount);

	bool isHack = false;
	int32_t aggresiveRatio = aggresiveMobs / m_vecPullInfo.size();
	std::string log = "MOB_PULL";

	if (median > 4000 + aggresiveRatio * 1000 || // If median is this high, it is a likely hack.
		(average > 4500 + aggresiveRatio * 1000 && median > 2000 + aggresiveRatio * 1000)) // There are high values on the distribution
	{
		isHack = true;
	}
	else
	{
		/*
		We got here, so median/average are inconclusive. Let's now check the distance
		between the player and the clusters. We'll take a small radius and consider
		the player "not moving" if they don't go outside there.

		Because mob pulling basically consists of sending many attack
		packets at the same time, the player's position will not change.

		If it changes, we can happily discard the old basePosition as m_vecPulledMonsters
		vector iteration implies a *linear forward progression in time*.
		*/
		GPOS basePosition = m_vecPullInfo[0].playerPos;

		std::vector<int32_t> vecDistances;
		for (size_t i = 1, size = m_vecPullInfo.size(); i < size && !isHack; ++i)
		{
			const GPOS& playerPos = m_vecPullInfo[i].playerPos;
			const int32_t threshold = m_vecPullInfo[i].isAggressive ? 750 : 300;

			if (DISTANCE_APPROX(playerPos.x - basePosition.x, playerPos.y - basePosition.y) > threshold)
			{
				//Player moved! Mayday, mayday! Quickly analyze and reset!
				isHack = analyzeStaticMov(vecDistances, log); //(can modify log)

				vecDistances.clear();
				basePosition = playerPos;
				continue;
			}

			const GPOS& mobPos = m_vecPullInfo[i].mobPos;
			vecDistances.push_back(DISTANCE_APPROX(mobPos.x - basePosition.x, mobPos.y - basePosition.y));
		}

		if (!isHack)
			isHack = analyzeStaticMov(vecDistances, log); //(can modify log)
	}

	//Too prone to false positives with too little monster clusters.
	if (isHack && monsterClusters.size() < 3)
		isHack = false;

	if (isHack)
	{
		log += " avg" + std::to_string(average)
			+ " med" + std::to_string(median)
			+ " dc" + std::to_string(distancesCount)
			+ " mcc" + std::to_string(monsterClusters.size())
			+ " m" + std::to_string(m_ch->GetMapIndex());

		LogManager::Instance().HackLog(log.c_str(), m_ch);

		// Make sure we don't analyze the same piece of data anymore
		m_vecPullInfo.clear();

		if (g_bIsTestServer) 
		{
			sys_log(0, "MOB_PULL!: %s", log.c_str());
			m_ch->ChatPacket(CHAT_TYPE_BIG_NOTICE, "Mob pull detected: %s", log.c_str());
		}
	}

	//Cooldown
	SuspiciousAttackCooldown();
}

void AbuseController::DeadWalk()
{
	if (JustDied())
		return;

	uint32_t now = get_unix_ms_time();
	if (m_nextDeadWalkLog > now)
		return;

	std::string info = "DEAD_WALK t" + std::to_string(now - m_lastDeathTime);
	LogManager::Instance().HackLog(info.c_str(), m_ch);
	m_ch->GetDesc()->DelayedDisconnect(10);

	m_nextDeadWalkLog = now + 3 * 1000;
}

void AbuseController::DeadPickup()
{
	if (JustDied())
		return;

	uint32_t now = get_unix_ms_time();
	if (m_nextDeadPickupLog > now)
		return;

	std::string info = "DEAD_PICKUP #%d t" + std::to_string(now - m_lastDeathTime);

	m_ch->GetDesc()->DelayedDisconnect(10);
	LogManager::Instance().HackLog(info.c_str(), m_ch);

	m_nextDeadPickupLog = now + 3 * 1000;
}

/************************************************************************/
/* WALL HACKING                                                         */
/************************************************************************/

void AbuseController::VerifyCoordinates(LPSECTREE tree, int32_t x, int32_t y, int32_t z)
{
	uint32_t now = get_unix_ms_time();
	if (m_whIgnoreUntil > now)
		return;

	auto canMove = !tree->IsAttr(x, y, ATTR_BLOCK | ATTR_OBJECT);
	if (canMove)
	{
		m_whContinuousInvalid = 0;

		//Decrease
		if (m_whHackCount > 0 && m_whNextDecrease < now)
		{
			m_whHackCount -= 1 + std::min(10000, static_cast<int32_t>(now - m_whNextDecrease)) / 2000;
			if (m_whHackCount < 0)
				m_whHackCount = 0;

			m_whNextDecrease = now + 2 * 2000;
			if (g_bIsTestServer)
				sys_log(0, "Decrease hack count for %s, new value: %d", m_ch->GetName(), m_whHackCount);
		}

		return; //Nothing more to do
	}

	//If we are on supposed block area, let's check our surroundings
	for (int32_t dx = -100; dx < 100; ++dx)
	{
		for (int32_t dy = -100; dy < 100; ++dy)
		{
			if (!tree->IsAttr(x + dx, y + dy, ATTR_BLOCK | ATTR_OBJECT))
			{
				m_whContinuousInvalid = 0;
				return; //It is all ok because its very close to a movement enabled area (possible misdetection)
			}
		}
	}

	if (g_bIsTestServer)
		sys_log(0, "Coordinates %d x %d for %s don't check out", x, y, m_ch->GetName());

	//If its our first hack walling in a while, or the first one at all, just initialize and don't do more.
	if (m_whLastDetection + 15 * 1000 < now || !m_whStartX || !m_whStartY)
	{
		m_whLastDetection = now;
		m_whNextDecrease = now + 60 * 1000;
		m_whContinuousInvalid = 0;
		m_whStartX = x;
		m_whStartY = y;
		m_whStartZ = z;
		return;
	}

	//Increase hack times
	++m_whHackCount;
	++m_whContinuousInvalid;

	//Store times
	m_whLastDetection = now;
	m_whNextDecrease = now + 15 * 1000; //time when it starts counting down the hack counts

	if (m_whHackCount >= 5)
	{
		bool punish = false;
		int32_t hackDist = DISTANCE_APPROX(m_whStartX - x, m_whStartY - y);

		if (m_whHackCount > 50) { //Player had lots of detections. Deserved
			punish = true;
		}
		else if (abs(m_whHackCount - m_whContinuousInvalid) < m_whHackCount / 5)
		{
			int32_t heightDif = abs(m_whStartZ - z);

			if (g_bIsTestServer) {
				sys_log(0, "Hacking values: dist %d, zdif: %d, count: %d, continuous: %d", hackDist, heightDif, m_whHackCount, m_whContinuousInvalid);
			}

			if (hackDist > 1500 || //Went too far
				heightDif > 100) //Very sudden height difference
			{
				punish = true;
			}
		}

		if (punish)
		{
			if (g_bIsTestServer) {
				m_ch->ChatPacket(CHAT_TYPE_BIG_NOTICE, "Wall hacker! [%d, %d]", m_whHackCount, m_whContinuousInvalid);
				m_ch->ChatPacket(CHAT_TYPE_INFO, "d%d", hackDist);
			}

			std::string info = "WALL_HACK c" + std::to_string(m_whHackCount);
			info += " i" + std::to_string(m_whContinuousInvalid);
			info += " d" + std::to_string(hackDist);
			info += " sx" + std::to_string(m_whStartX);
			info += " sy" + std::to_string(m_whStartY);
			info += " cx" + std::to_string(x);
			info += " cy" + std::to_string(y);
			info += " m" + std::to_string(m_ch->GetMapIndex());

			LogManager::Instance().HackLog(info.c_str(), m_ch);

			//Disconnect after some minutes
			if (hackDist > 2000)
			{
				LPDESC d = m_ch->GetDesc();
				if (d) {
					m_ch->ChatPacket(CHAT_TYPE_NOTICE, "Beachte die Schranken dieser Welt.");
					d->DelayedDisconnect(number(10, 15));
				}
				else {
					sys_err("Error while trying to disconnect [%s] due to wallhacking", m_ch->GetName());
				}
			}

			//Reset and wait 1 minute before processing this again.
			m_whHackCount = 0;
			m_whIgnoreUntil = g_bIsTestServer ? now : now + 10 * 1000;
		}
	}
}

/************************************************************************/
/* Movement speed hacking                                               */
/************************************************************************/

void AbuseController::ResetMovementSpeedhackChecker(bool full /* = false */)
{
	m_mshStartX = 0;
	m_mshStartY = 0;
	m_mshStartDetect = 0;

	if (full)
	{
		m_mshFirstDetect = 0;
		m_mshRates.clear();
	}
}

bool AbuseController::CanPickupItem()
{
	if (get_unix_ms_time() - m_mshLastDetect > 3000)
		return true;

	size_t count = m_mshRates.size();
	if (count < 3)
		return true;

	return getMedian(m_mshRates, count) > 0.5;
}

void AbuseController::SignalMovement(uint32_t lastStopTime)
{
	if (lastStopTime < get_unix_ms_time() - 1000)
	{
		ResetMovementSpeedhackChecker(false);
		return;
	}
}

void AbuseController::CheckSpeedhack(int32_t x, int32_t y)
{
	uint32_t now = get_unix_ms_time();

	if (g_bDisableMovspeedHacklog || m_mshIgnoreUntil > now || m_ch->IsWalking())
		return;

	if (m_mshFirstDetect > 0 && m_mshFirstDetect + 60 * 1000 < now)
	{
		//We only want to take the info of the last 60s into consideration.
		//Clean up everything completely
		ResetMovementSpeedhackChecker(true);
	}

	//Set starting coordinates
	if (!m_mshStartDetect || !m_mshStartX || !m_mshStartY)
	{
		m_mshStartX = x;
		m_mshStartY = y;
		m_mshStartDetect = now;
		return;
	}

	int32_t dx = abs(m_mshStartX - x);
	int32_t dy = abs(m_mshStartY - y);

	float distWalked = DISTANCE_SQRT(dx, dy);
	if (distWalked < 600)
		return;

	int32_t expectedDuration = CalculateDuration(static_cast<int32_t>(m_ch->GetLimitPoint(POINT_MOV_SPEED)),
		static_cast<int32_t>((distWalked / m_ch->GetMoveMotionSpeed()) * 1000.0f));

	int32_t duration = now - m_mshStartDetect;
	double rate = static_cast<double>(duration) / expectedDuration;

	if (rate < g_dMovspeedHackThreshold)
	{
		m_mshLastDetect = now;

		if (g_bIsTestServer)
			m_ch->ChatPacket(CHAT_TYPE_GUILD, "Speedhack! Rate: %f", rate);

		if (!m_mshFirstDetect)
			m_mshFirstDetect = now;

		//Get information about the rate
		m_mshRates.push_back(rate);
		size_t count = m_mshRates.size();

		if (count >= 10)
		{
			const double median = getMedian(m_mshRates, count);
			const double average = getAverage(m_mshRates, count);

			std::string info = "MOV_SPEED_HACK r" + std::to_string(median) + " (avgr" + std::to_string(average) + ")"
				+ " t" + std::to_string((now - m_mshFirstDetect) / 1000)
				+ " (" + std::to_string(m_ch->GetX()) + ", " + std::to_string(m_ch->GetY()) + ")";

			LogManager::Instance().HackLog(info.c_str(), m_ch);

			if (g_bIsTestServer)
				m_ch->ChatPacket(CHAT_TYPE_INFO, "Speedhack detected");

			if (!g_bIsTestServer)
				m_mshIgnoreUntil = now + 30 * 1000;


			ResetMovementSpeedhackChecker(true);
		}
	}
}

/*
Function regularly called from CHARACTER_MANAGER::Update.
Takes care of analyzing possible recent abuse entries.
*/
void AbuseController::Analyze()
{
	uint32_t now = thecore_pulse();
	if (m_lastUpdatePulse + PASSES_PER_SEC(1) > now)
		return;

	if (m_lastAttackAnalysis + PASSES_PER_SEC(3) < now)
		AnalyzeAttacks();

	m_lastUpdatePulse = now;
}

/************************************************************************/
/* FISHBOT CONTROL                                                      */
/************************************************************************/
void AbuseController::SuspectFishbotBy(LPCHARACTER suspiciousCH)
{
	if (!suspiciousCH || !suspiciousCH->IsGM())
		return;

	sys_log(0, "Abuse: %s suspects that %s might be a fishbot.", suspiciousCH->GetName(), m_ch->GetName());

	m_isSuspectedFishbot = true;
	m_fishesLeftToBubbleHide = number(1, 4);
	m_bubbleHideTimesLeft = 2;
}

void AbuseController::EndSuspectFishbot()
{
	sys_log(0, "Abuse: %s is no longer being tracked for fishbot.", m_ch->GetName());
	m_isSuspectedFishbot = false;
}

bool AbuseController::IsHideFishingBubble() const
{
	return m_fishesLeftToBubbleHide == 0;
}

void AbuseController::NotifyHiddenBubble()
{
	++m_bubbleHideTimes;
	--m_bubbleHideTimesLeft;
}

void AbuseController::NotifyFishedWithBubbleHide(bool valid, int32_t ms)
{
	if (valid)
	{
		auto reason = "POSSIBLE_FISHBOT [req] " + std::to_string(ms) + "ms";
		LogManager::Instance().HackLog(reason.c_str(), m_ch);

		if (m_bubbleHideTimes < 7) //If we didn't scan this guy to death already, let's try to confirm
			m_bubbleHideTimes += 2;
	}

	if (m_bubbleHideTimesLeft <= 0) {
		EndSuspectFishbot();
		return;
	}

	//Get a new fishes left to bubble hide
	m_fishesLeftToBubbleHide = number(1, 7);
}

void AbuseController::ReceiveAttackPacket()
{
	uint32_t now = get_unix_ms_time();
	uint32_t dif = now - m_lastMoveAttackPacketTime;

	if (m_ignoreWaitDmgUntil > now)
		return;

	if (!m_waitDmgSuspect && dif < 5000)
		return; //Not enough to get us suspicious

	if (!m_waitDmgSuspect)
		m_waitDmgSuspect = true;

	m_moveAttackTimes.push_back(dif);
	size_t count = m_moveAttackTimes.size();

	if (count >= 25) {
		const double median = getMedian(m_moveAttackTimes, count);

		if (median <= 1200)
			m_ignoreWaitDmgUntil = now + number(300, 600) * 1000; //Clean. Ignore for some time [5-10m].
		else if (median <= 2100)
			m_ignoreWaitDmgUntil = now + number(120, 300) * 1000; //Semiclean.
		else if (median <= 3000)
			m_ignoreWaitDmgUntil = now + number(30, 60) * 1000; //Uh....
		else {
			//Gotcha!
			std::string desc = "WAITDMG " + std::to_string(median);
			desc += " (" + std::to_string(m_moveAttackTimes[0]) + ", " + std::to_string(m_moveAttackTimes[count - 1]) + ")";

			LogManager::Instance().HackLog(desc.c_str(), m_ch);

			m_ch->GetDesc()->DelayedDisconnect(5);

			m_ignoreWaitDmgUntil = now + 300 * 1000; //5m, we know (think) this guy is hacking already!
		}

		m_moveAttackTimes.clear();
	}
}

/************************************************************************/
/*					PICKUP BOT											*/
/************************************************************************/
void AbuseController::AutoPickupSuspect()
{
	++m_pickupBotSuspectCount;
	if (m_pickupBotSuspectCount >= 10)
	{
		//Fishy!
		LogManager::Instance().HackLog("POSSIBLE_PICKUP_BOT", m_ch);
		m_pickupBotSuspectCount = 0;
	}
}