#include "LightDirectionEmission.hpp"
#include "LightShape.hpp"
#include "LightSystem.hpp"

#include <assert.h>

namespace ltbl
{

LightDirectionEmission::LightDirectionEmission(LightSystem& system)
	: mSystem(system)
	, mCastDirection(0.f, 1.f)
	, mCastAngle(90.f)
	, mSourceRadius(5.0f)
	, mSourceDistance(100.0f)
{
}

void LightDirectionEmission::render(const sf::View &view, sf::RenderTexture &lightTempTexture, sf::RenderTexture &antumbraTempTexture, const std::vector<QuadtreeOccupant*> &shapes, float shadowExtension)
{
    lightTempTexture.setView(view);
    lightTempTexture.clear(sf::Color::White);

    // Mask off light shape (over-masking - mask too much, reveal penumbra/antumbra afterwards)
    for (unsigned i = 0; i < shapes.size(); i++) 
	{
        LightShape* pLightShape = static_cast<LightShape*>(shapes[i]);
		if (pLightShape != nullptr && pLightShape->isTurnedOn() && pLightShape->isAwake())
		{
			// Get boundaries
			std::vector<LightSystem::Penumbra> penumbras;
			std::vector<int> innerBoundaryIndices;
			std::vector<int> outerBoundaryIndices;
			std::vector<sf::Vector2f> innerBoundaryVectors;
			std::vector<sf::Vector2f> outerBoundaryVectors;

			LightSystem::getPenumbrasDirection(penumbras, innerBoundaryIndices, innerBoundaryVectors, outerBoundaryIndices, outerBoundaryVectors, *pLightShape, mCastDirection, mSourceRadius, mSourceDistance);

			if (innerBoundaryIndices.size() != 2 || outerBoundaryIndices.size() != 2)
			{
				continue;
			}

			antumbraTempTexture.clear(sf::Color::White);
			antumbraTempTexture.setView(view);

			float maxDist = 0.0f;
			for (unsigned j = 0; j < pLightShape->getPointCount(); j++)
			{
				maxDist = std::max(maxDist, vectorMagnitude(view.getCenter() - pLightShape->getTransform().transformPoint(pLightShape->getPoint(j))));
			}
			float totalShadowExtension = shadowExtension + maxDist;

			sf::ConvexShape maskShape;
			maskShape.setPointCount(4);
			maskShape.setPoint(0, pLightShape->getTransform().transformPoint(pLightShape->getPoint(innerBoundaryIndices[0])));
			maskShape.setPoint(1, pLightShape->getTransform().transformPoint(pLightShape->getPoint(innerBoundaryIndices[1])));
			maskShape.setPoint(2, pLightShape->getTransform().transformPoint(pLightShape->getPoint(innerBoundaryIndices[1])) + vectorNormalize(innerBoundaryVectors[1]) * totalShadowExtension);
			maskShape.setPoint(3, pLightShape->getTransform().transformPoint(pLightShape->getPoint(innerBoundaryIndices[0])) + vectorNormalize(innerBoundaryVectors[0]) * totalShadowExtension);

			maskShape.setFillColor(sf::Color::Black);

			antumbraTempTexture.draw(maskShape);

			sf::VertexArray vertexArray;
			vertexArray.setPrimitiveType(sf::PrimitiveType::Triangles);
			vertexArray.resize(3);

			{
				sf::RenderStates states;
				states.blendMode = sf::BlendAdd;
				states.shader = &mSystem.getUnshadowShader();

				// Unmask with penumbras
				for (unsigned j = 0; j < penumbras.size(); j++)
				{
					mSystem.getUnshadowShader().setUniform("lightBrightness", penumbras[j]._lightBrightness);
					mSystem.getUnshadowShader().setUniform("darkBrightness", penumbras[j]._darkBrightness);

					vertexArray[0].position = penumbras[j]._source;
					vertexArray[1].position = penumbras[j]._source + vectorNormalize(penumbras[j]._lightEdge) * totalShadowExtension;
					vertexArray[2].position = penumbras[j]._source + vectorNormalize(penumbras[j]._darkEdge) * totalShadowExtension;

					vertexArray[0].texCoords = sf::Vector2f(0.0f, 1.0f);
					vertexArray[1].texCoords = sf::Vector2f(1.0f, 0.0f);
					vertexArray[2].texCoords = sf::Vector2f(0.0f, 0.0f);

					antumbraTempTexture.draw(vertexArray, states);
				}
			}

			antumbraTempTexture.display();

			// Multiply back to lightTempTexture
			lightTempTexture.setView(lightTempTexture.getDefaultView());
			lightTempTexture.draw(sf::Sprite(antumbraTempTexture.getTexture()), sf::BlendMultiply);
			lightTempTexture.setView(view);
		}
    }

    for (unsigned i = 0; i < shapes.size(); i++) 
	{
        LightShape* pLightShape = static_cast<LightShape*>(shapes[i]);
        if (pLightShape->renderLightOver() && pLightShape->isAwake() && pLightShape->isTurnedOn()) 
		{
            pLightShape->setFillColor(sf::Color::White);

            lightTempTexture.draw(*pLightShape);
        }
    }

    // Multiplicatively blend the light over the shadows
    lightTempTexture.setView(lightTempTexture.getDefaultView());
    lightTempTexture.draw(*this, sf::BlendMultiply);

    lightTempTexture.display();
}

void LightDirectionEmission::setCastDirection(sf::Vector2f const& castDirection)
{
	mCastDirection = vectorNormalize(castDirection);
	mCastAngle = _radToDeg * std::atan2(mCastDirection.y, mCastDirection.x);
}

sf::Vector2f LightDirectionEmission::getCastDirection() const
{
	return mCastDirection;
}

void LightDirectionEmission::setCastAngle(float angle)
{
	mCastAngle = angle;
	float radAngle = angle * _degToRad;
	mCastDirection = sf::Vector2f(std::cos(radAngle), std::sin(radAngle));
}

float LightDirectionEmission::getCastAngle() const
{
	return mCastAngle;
}

void LightDirectionEmission::remove()
{
	mSystem.removeLight(this);
}

} // namespace lum