#pragma once

#include "Quadtree.hpp"
#include "BaseLight.hpp"

namespace ltbl 
{

class LightDirectionEmission : public BaseLight
{
	public:
		LightDirectionEmission(LightSystem& system);

		void setColor(const sf::Color& color);
		const sf::Color& getColor() const;

		void render(const sf::View &view, sf::RenderTexture &lightTempTexture, sf::RenderTexture &antumbraTempTexture, const std::vector<QuadtreeOccupant*> &shapes, float shadowExtension);

		void setCastDirection(sf::Vector2f const& castDirection);
		sf::Vector2f getCastDirection() const;

		void setCastAngle(float angle);
		float getCastAngle() const;

		void remove();

	private:
		sf::RectangleShape mShape;

		sf::Vector2f mCastDirection;
		float mCastAngle;

		float mSourceRadius;
		float mSourceDistance;
};

} // namespace ltbl