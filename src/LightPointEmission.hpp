#pragma once

#include "Quadtree.hpp"

namespace lum
{
    
class LightPointEmission : public QuadtreeOccupant, public sf::Sprite
{
    public:
		typedef std::shared_ptr<LightPointEmission> Ptr;

		LightPointEmission();

		sf::FloatRect getAABB() const;

        void render(const sf::View& view,
                    sf::RenderTexture& lightTempTexture, sf::RenderTexture& emissionTempTexture, sf::RenderTexture& antumbraTempTexture,
                    const std::vector<QuadtreeOccupant*>& shapes,
                    sf::Shader& unshadowShader, sf::Shader& lightOverShapeShader);

		void setLocalCastCenter(sf::Vector2f const& localCenter);
		sf::Vector2f getLocalCastCenter() const;

		sf::Vector2f getCastCenter() const;

	private:
		sf::Vector2f mLocalCastCenter;

		float mSourceRadius;

		float mShadowOverExtendMultiplier;
};

} // namespace lum
