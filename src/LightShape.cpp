#include "LightShape.hpp"
#include "LightSystem.hpp"

namespace ltbl
{

LightShape::LightShape(LightSystem& system)
	: BaseLight(system)
	, mShape()
	, mRenderLightOver(true)
{
}

void LightShape::setPointCount(unsigned int pointCount)
{
	mShape.setPointCount(pointCount);
	quadtreeAABBChanged();
}

unsigned int LightShape::getPointCount() const
{
	return mShape.getPointCount();
}

void LightShape::setPoint(unsigned int index, const sf::Vector2f & point)
{
	mShape.setPoint(index, point);
	quadtreeAABBChanged();
}

sf::Vector2f LightShape::getPoint(unsigned int index) const
{
	return mShape.getPoint(index);
}

void LightShape::setFillColor(sf::Color const& color)
{
	mShape.setFillColor(color);
}

const sf::Color& LightShape::getFillColor() const
{
	return mShape.getFillColor();
}

const sf::Transform& LightShape::getTransform() const
{
	return mShape.getTransform();
}

void LightShape::setPosition(const sf::Vector2f& position)
{
	mShape.setPosition(position);
	quadtreeAABBChanged();
}

void LightShape::setPosition(float x, float y)
{
	mShape.setPosition(x, y);
	quadtreeAABBChanged();
}

void LightShape::move(const sf::Vector2f& movement)
{
	mShape.move(movement);
	quadtreeAABBChanged();
}

void LightShape::move(float x, float y)
{
	mShape.move(x, y);
	quadtreeAABBChanged();
}

const sf::Vector2f& LightShape::getPosition() const
{
	return mShape.getPosition();
}

void LightShape::setRotation(float rotation)
{
	mShape.setRotation(rotation);
	quadtreeAABBChanged();
}

float LightShape::getRotation() const
{
	return mShape.getRotation();
}

void LightShape::setScale(const sf::Vector2f& scale)
{
	mShape.setScale(scale);
	quadtreeAABBChanged();
}

void LightShape::setScale(float x, float y)
{
	mShape.setScale(x, y);
	quadtreeAABBChanged();
}

void LightShape::scale(const sf::Vector2f& scale)
{
	mShape.scale(scale);
	quadtreeAABBChanged();
}

void LightShape::scale(float x, float y)
{
	mShape.scale(x, y);
	quadtreeAABBChanged();
}

const sf::Vector2f& LightShape::getScale() const
{
	return mShape.getScale();
}

void LightShape::setOrigin(const sf::Vector2f& origin)
{
	mShape.setOrigin(origin);
	quadtreeAABBChanged();
}

void LightShape::setOrigin(float x, float y)
{
	mShape.setOrigin(x, y);
	quadtreeAABBChanged();
}

const sf::Vector2f& LightShape::getOrigin() const
{
	return mShape.getOrigin();
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
	return mShape.getGlobalBounds();
}

void LightShape::remove()
{
	getSystem().removeShape(this);
}

void LightShape::draw(sf::RenderTarget& target, sf::RenderStates states) const
{
	target.draw(mShape, states);
}

} // namespace ltbl
