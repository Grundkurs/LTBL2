#pragma once

namespace ltbl
{

class LightSystem;
class BaseLight
{
	public:
		BaseLight(LightSystem& system);
		
		LightSystem& getSystem();
		
		void remove();
		
		void setTurnedOn(bool turnedOn);
		bool isTurnedOn() const;
		void toggleTurnedOn();
		
	private:
		LightSystem& mSystem;
		bool mTurnedOn;
};

} // namespace ltbl