#pragma once
#include "../../libthecore/include/utils.h"
#include "typedef.h"
#include "gposition.h"
#include <vector>
#include <memory>

class AbuseController
{
public:
	AbuseController(LPCHARACTER ch);

private:
	LPCHARACTER m_ch;
	uint32_t m_lastUpdatePulse;

	/************************************************************************/
	/* Mob puller                                                           */
	/************************************************************************/
private:
	uint32_t m_lastAttackAnalysis;
	uint32_t m_lastSuspiciousAttackTime;

	uint32_t m_nextAttackCooldown;
	uint32_t m_nextDetectStart;

	struct PullInfo {
		GPOS mobPos;
		GPOS playerPos;
		bool isAggressive;
	};
	std::vector<PullInfo> m_vecPullInfo;

	void AnalyzeAttacks();
public:
	void InitiatedFight(const GPOS& mobPos, bool aggressive);
	void SuspiciousAttackCooldown();

	/************************************************************************/
	/* Ghost mode (Activities while dead)									*/
	/************************************************************************/
private:
	uint32_t m_lastDeathTime;
	uint32_t m_nextDeadWalkLog;
	uint32_t m_nextDeadPickupLog;

public:
	void DeadWalk();
	void DeadPickup();

	void MarkDead() { m_lastDeathTime = get_dword_time(); }
	bool JustDied() const { return m_lastDeathTime + 1500 > get_dword_time(); }

	/************************************************************************/
	/* Wall hacking                                                         */
	/************************************************************************/
private:
	int32_t m_whStartX;
	int32_t m_whStartY;
	int32_t m_whStartZ;
	int32_t m_whHackCount;
	uint32_t m_whLastDetection;
	uint32_t m_whIgnoreUntil;
	int32_t m_whContinuousInvalid;
	uint32_t m_whNextDecrease;

public:
	void VerifyCoordinates(LPSECTREE tree, int32_t x, int32_t y, int32_t z);

	/************************************************************************/
	/* Movement speed hacking                                               */
	/************************************************************************/
private:
	int32_t m_mshStartX;
	int32_t m_mshStartY;
	int32_t m_mshStartZ;
	std::vector <double> m_mshRates;
	uint32_t m_mshStartDetect;
	uint32_t m_mshLastDetect;
	uint32_t m_mshIgnoreUntil;
	uint32_t m_mshFirstDetect;

public:
	bool CanPickupItem();
	void SignalMovement(uint32_t lastStopTime);
	void CheckSpeedhack(int32_t x, int32_t y);
	void ResetMovementSpeedhackChecker(bool full = false);
	void Analyze();

	/************************************************************************/
	/* Fishbotting                                                          */
	/************************************************************************/
private:
	bool m_isSuspectedFishbot;
	int32_t m_fishesLeftToBubbleHide;
	int32_t m_bubbleHideTimesLeft;
	int32_t m_bubbleHideTimes;

public:
	bool IsSuspectedFishbot() const { return m_isSuspectedFishbot; }
	bool IsHideFishingBubble() const;
	void SuspectFishbotBy(LPCHARACTER ch);
	void EndSuspectFishbot();
	void NotifyHiddenBubble();
	void NotifyFishedWithBubbleHide(bool valid, int32_t ms);

	/************************************************************************/
	/* WaitDmg						                                        */
	/************************************************************************/
private:
	bool m_waitDmgSuspect;
	uint32_t m_lastMoveAttackPacketTime;
	uint32_t m_ignoreWaitDmgUntil;
	std::vector<uint32_t> m_moveAttackTimes;
public:
	//Motion of attacking / combo / skill
	void ReceiveMoveAttackPacket() { m_lastMoveAttackPacketTime = get_dword_time(); };

	//Plain, raw attack packet.
	void ReceiveAttackPacket();

	/************************************************************************/
	/* Pickup bot                                                           */
	/************************************************************************/

private:
	int32_t m_pickupBotSuspectCount;

public:
	void AutoPickupSuspect();
};

template <typename T>
T getMedian(std::vector<T>& values, size_t vectorSize)
{
	T median = 0;

	if (vectorSize > 0)
	{
		//Sort the vector
		std::sort(values.begin(), values.end());

		//Extract the median
		if (vectorSize % 2 == 0) {
			median = (values[vectorSize / 2 - 1] + values[vectorSize / 2]) / 2;
		}
		else {
			median = values[static_cast<size_t>(std::floor(vectorSize / 2))];
		}
	}

	return median;
}

template <typename T>
T getAverage(std::vector<T>& values, size_t vectorSize)
{
	T average = 0;

	if (vectorSize > 0)
	{
		for (const auto value : values) {
			average += value;
		}

		average /= vectorSize;
	}

	return average;
}

typedef std::shared_ptr<AbuseController> spAbuseController;
