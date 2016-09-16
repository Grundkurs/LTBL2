#pragma once

#include "Quadtree.hpp"

namespace ltbl
{
    
class LightSystem;
class LightPointEmission : public QuadtreeOccupant, public sf::Sprite
{
    public:
		LightPointEmission(LightSystem& system);

		sf::FloatRect getAABB() const;

        void render(const sf::View& view,
                    sf::RenderTexture& lightTempTexture, sf::RenderTexture& emissionTempTexture, sf::RenderTexture& antumbraTempTexture,
                    const std::vector<QuadtreeOccupant*>& shapes);

		void setLocalCastCenter(sf::Vector2f const& localCenter);
		sf::Vector2f getLocalCastCenter() const;

		sf::Vector2f getCastCenter() const;

		void remove();

	private:
		LightSystem& mSystem;

		sf::Vector2f mLocalCastCenter;

		float mSourceRadius;

		float mShadowOverExtendMultiplier;
};

} // namespace lum
