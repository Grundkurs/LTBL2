#include "LightShape.hpp"

namespace lum
{

LightShape::LightShape()
	: mRenderLightOver(true)
{
}

void LightShape::setRenderLightOver(bool renderLightOver)
{
	mRenderLightOver = renderLightOver;
}

bool LightShape::renderLightOver() const
{
	return mRenderLightOver;
}

sf::FloatRect LightShape::getAABB() const
{
	return getGlobalBounds();
}

} // namespace lum
