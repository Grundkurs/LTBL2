#pragma once

#include "Quadtree.hpp"
#include "BaseLight.hpp"

namespace ltbl
{
    
class LightPointEmission : public QuadtreeOccupant, public BaseLight, public sf::Drawable
{
    public:
		LightPointEmission(LightSystem& system);

		sf::FloatRect getAABB() const;

		void setTexture(sf::Texture& texture);
		const sf::Texture* getTexture() const;

		void setTextureRect(const sf::IntRect& rect);
		const sf::IntRect& getTextureRect() const;

		void setColor(const sf::Color& color);
		const sf::Color& getColor() const;

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

        void render(const sf::View& view,
                    sf::RenderTexture& lightTempTexture, sf::RenderTexture& emissionTempTexture, sf::RenderTexture& antumbraTempTexture,
                    const std::vector<QuadtreeOccupant*>& shapes);

		void setLocalCastCenter(sf::Vector2f const& localCenter);
		sf::Vector2f getLocalCastCenter() const;

		sf::Vector2f getCastCenter() const;

		void remove();

	private:
		void draw(sf::RenderTarget& target, sf::RenderStates states) const;

	private:
		sf::Sprite mSprite;

		sf::Vector2f mLocalCastCenter;

		float mSourceRadius;

		float mShadowOverExtendMultiplier;
};

} // namespace ltbl
