#pragma once

class IFlyEventHandler
{
public:
	IFlyEventHandler() {}
	virtual ~IFlyEventHandler() {}

	// Call by ActorInstance
	virtual void OnSetFlyTarget() {}
	virtual void OnShoot(uint32_t dwSkillIndex) {}

	virtual void OnNoTarget() {}
	virtual void OnNoArrow() {}

	// Call by FlyingInstance
	virtual void OnExplodingOutOfRange() {}
	virtual void OnExplodingAtBackground() {}
	virtual void OnExplodingAtAnotherTarget(uint32_t dwSkillIndex, uint32_t dwVID) {}
	virtual void OnExplodingAtTarget(uint32_t dwSkillIndex) {}
};