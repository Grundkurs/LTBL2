#include "BaseLight.hpp"
#include "LightSystem.hpp"

namespace ltbl
{

BaseLight::BaseLight(LightSystem& system)
	: mSystem(system)
	, mTurnedOn(true)
{
}

LightSystem& BaseLight::getSystem()
{
	return mSystem;
}

void BaseLight::remove()
{
}

void BaseLight::setTurnedOn(bool turnedOn)
{
	mTurnedOn = turnedOn;
}

bool BaseLight::isTurnedOn() const
{
	return mTurnedOn;
}

void BaseLight::toggleTurnedOn()
{
	mTurnedOn = !mTurnedOn;
}

} // namespace ltbl
