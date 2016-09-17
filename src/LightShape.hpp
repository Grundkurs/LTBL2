#pragma once

#include "Quadtree.hpp"
#include "BaseLight.hpp"

namespace ltbl
{

class LightShape : public QuadtreeOccupant, public BaseLight, public sf::Drawable
{
	public:
		LightShape(LightSystem& system);

		void setPointCount(unsigned int pointCount);
		unsigned int getPointCount() const;

		void setPoint(unsigned int index, const sf::Vector2f& point);
		sf::Vector2f getPoint(unsigned int index) const;

		void setFillColor(sf::Color const& color);
		const sf::Color& getFillColor() const;

		const sf::Transform& getTransform() const;

		void setPosition(const sf::Vector2f& position);
		void setPosition(float x, float y);
		void move(const sf::Vector2f& movement);
		void move(float x, float y);
		const sf::Vector2f& getPosition() const;

		void setRotation(float rotation);
		float getRotation() const;

		void setScale(const sf::Vector2f& scale);
		void setScale(float x, float y);
		void scale(const sf::Vector2f& scale);
		void scale(float x, float y);
		const sf::Vector2f& getScale() const;

		void setOrigin(const sf::Vector2f& origin);
		void setOrigin(float x, float y);
		const sf::Vector2f& getOrigin() const; 

		void setRenderLightOver(bool renderLightOver);
		bool renderLightOver() const;

		sf::FloatRect getAABB() const;

		void remove();

	private:
		void draw(sf::RenderTarget& target, sf::RenderStates states) const;

	private:
		sf::ConvexShape mShape;
		bool mRenderLightOver;
};

} // namespace ltbl