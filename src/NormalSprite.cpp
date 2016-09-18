#include "NormalSprite.hpp"

namespace ltbl
{

NormalSprite::NormalSprite()
	: BaseLight()
	, sf::Sprite()
	, mTexture(nullptr)
	, mNormalsTexture(nullptr)
	, mNeedRenderNormals(false)
{
}

void NormalSprite::setTexture(sf::Texture& texture, bool resetRect)
{
	sf::Sprite::setTexture(texture, resetRect);
	mTexture = &texture;
}

void NormalSprite::setNormalsTexture(sf::Texture& normalsTexture)
{
	mNormalsTexture = &normalsTexture;
}

const sf::Texture* NormalSprite::getNormalsTexture() const
{
	return mNormalsTexture;
}

void NormalSprite::render(sf::RenderTarget& target, sf::RenderStates states)
{
	target.draw(*this, states);
	mNeedRenderNormals = true;
}

void NormalSprite::renderNormals(sf::RenderTarget& target, sf::RenderStates states)
{
	if (mNormalsTexture != nullptr && mNeedRenderNormals)
	{
		sf::Sprite::setTexture(*mNormalsTexture);
		target.draw(*this, states);
		sf::Sprite::setTexture(*mTexture);
	}
	mNeedRenderNormals = true;
}

} // namespace ltbl