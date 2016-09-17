#pragma once

#include "Quadtree.hpp"

namespace ltbl 
{

class LightSystem;
class LightDirectionEmission : public sf::Sprite
{
	public:
		LightDirectionEmission(LightSystem& system);

		void render(const sf::View &view, sf::RenderTexture &lightTempTexture, sf::RenderTexture &antumbraTempTexture, const std::vector<QuadtreeOccupant*> &shapes, float shadowExtension);

		void setCastDirection(sf::Vector2f const& castDirection);
		sf::Vector2f getCastDirection() const;

		void setCastAngle(float angle);
		float getCastAngle() const;

		void remove();

		void setTurnedOn(bool turnedOn);
		bool isTurnedOn() const;
		void toggleTurnedOn();

	private:
		LightSystem& mSystem;

		sf::Vector2f mCastDirection;
		float mCastAngle;

		float mSourceRadius;
		float mSourceDistance;

		bool mTurnedOn;
};

} // namespace lum