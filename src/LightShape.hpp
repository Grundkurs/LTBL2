#pragma once

#include "Quadtree.hpp"

namespace ltbl
{

class LightSystem;
class LightShape : public QuadtreeOccupant, public sf::ConvexShape
{
	public:
		LightShape(LightSystem& system);

		void setRenderLightOver(bool renderLightOver);
		bool renderLightOver() const;

		sf::FloatRect getAABB() const;

		void remove();

	private:
		LightSystem& mSystem;
		bool mRenderLightOver;
};

} // namespace ltbl