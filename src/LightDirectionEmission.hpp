#pragma once

#include "Quadtree.hpp"

namespace lum 
{
	
class LightDirectionEmission : public sf::Sprite
{
	public:
		typedef std::shared_ptr<LightDirectionEmission> Ptr;

		LightDirectionEmission();

		void render(const sf::View &view, sf::RenderTexture &lightTempTexture, sf::RenderTexture &antumbraTempTexture, const std::vector<QuadtreeOccupant*> &shapes, sf::Shader &unshadowShader, float shadowExtension);

		void setCastDirection(sf::Vector2f const& castDirection);
		sf::Vector2f getCastDirection() const;

		void setCastAngle(float angle);
		float getCastAngle() const;

	private:
		sf::Vector2f mCastDirection;
		float mCastAngle;

		float mSourceRadius;
		float mSourceDistance;
};

} // namespace lum