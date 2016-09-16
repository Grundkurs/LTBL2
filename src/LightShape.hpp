#pragma once

#include "Quadtree.hpp"

namespace lum
{

class LightShape : public QuadtreeOccupant, public sf::ConvexShape
{
	public:
		typedef std::shared_ptr<LightShape> Ptr;

		LightShape();

		void setRenderLightOver(bool renderLightOver);
		bool renderLightOver() const;

		sf::FloatRect getAABB() const;

	private:
		bool mRenderLightOver;
};

} // namespace lum