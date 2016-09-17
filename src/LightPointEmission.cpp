#include "LightPointEmission.hpp"
#include "LightShape.hpp"
#include "LightSystem.hpp"

#include <assert.h>
#include <iostream>

namespace ltbl
{

LightPointEmission::LightPointEmission(LightSystem& system)
	: mSystem(system)
	, mSourceRadius(8.0f)
	, mShadowOverExtendMultiplier(1.4f)
{
}

sf::FloatRect LightPointEmission::getAABB() const 
{
	return getGlobalBounds();
}

void LightPointEmission::render(const sf::View& view,
                            sf::RenderTexture& lightTempTexture, sf::RenderTexture& emissionTempTexture, sf::RenderTexture& antumbraTempTexture,
                            const std::vector<QuadtreeOccupant*>& shapes)
{
    emissionTempTexture.clear();
    emissionTempTexture.setView(view);
    emissionTempTexture.draw(*this);
    emissionTempTexture.display();

    float shadowExtension = mShadowOverExtendMultiplier * (getAABB().width + getAABB().height);

    struct OuterEdges 
	{
        std::vector<int> _outerBoundaryIndices;
        std::vector<sf::Vector2f> _outerBoundaryVectors;
    };

    std::vector<OuterEdges> outerEdges(shapes.size());

    std::vector<int> innerBoundaryIndices;
    std::vector<sf::Vector2f> innerBoundaryVectors;
    std::vector<LightSystem::Penumbra> penumbras;

	sf::RenderStates maskRenderStates = sf::BlendNone;
	sf::RenderStates antumbraRenderStates = sf::BlendMultiply;

    //----- Emission

    lightTempTexture.clear();
    lightTempTexture.setView(view);
    lightTempTexture.draw(*this);

    //----- Shapes

    // Mask off light shape (over-masking - mask too much, reveal penumbra/antumbra afterwards)
    unsigned shapesCount = shapes.size();
    for (unsigned i = 0; i < shapesCount; ++i) 
	{
        LightShape* pLightShape = static_cast<LightShape*>(shapes[i]);
		if (pLightShape->isAwake() && pLightShape->isTurnedOn())
		{
			// Get boundaries
			innerBoundaryIndices.clear();
			innerBoundaryVectors.clear();
			penumbras.clear();

			LightSystem::getPenumbrasPoint(penumbras, innerBoundaryIndices, innerBoundaryVectors, outerEdges[i]._outerBoundaryIndices, outerEdges[i]._outerBoundaryVectors, *pLightShape, getCastCenter(), mSourceRadius);

			if (innerBoundaryIndices.size() != 2 || outerEdges[i]._outerBoundaryIndices.size() != 2)
			{
				continue;
			}

			// Render shape
			if (!pLightShape->renderLightOver())
			{
				pLightShape->setFillColor(sf::Color::Black);
				lightTempTexture.draw(*pLightShape);
			}

			sf::Vector2f as = pLightShape->getTransform().transformPoint(pLightShape->getPoint(outerEdges[i]._outerBoundaryIndices[0]));
			sf::Vector2f bs = pLightShape->getTransform().transformPoint(pLightShape->getPoint(outerEdges[i]._outerBoundaryIndices[1]));
			sf::Vector2f ad = outerEdges[i]._outerBoundaryVectors[0];
			sf::Vector2f bd = outerEdges[i]._outerBoundaryVectors[1];

			sf::Vector2f intersectionOuter;

			// Handle antumbras as a seperate case
			if (rayIntersect(as, ad, bs, bd, intersectionOuter))
			{
				sf::Vector2f asi = pLightShape->getTransform().transformPoint(pLightShape->getPoint(innerBoundaryIndices[0]));
				sf::Vector2f bsi = pLightShape->getTransform().transformPoint(pLightShape->getPoint(innerBoundaryIndices[1]));
				sf::Vector2f adi = innerBoundaryVectors[0];
				sf::Vector2f bdi = innerBoundaryVectors[1];

				antumbraTempTexture.clear(sf::Color::White);
				antumbraTempTexture.setView(view);

				sf::Vector2f intersectionInner;

				if (rayIntersect(asi, adi, bsi, bdi, intersectionInner))
				{
					sf::ConvexShape maskShape;
					maskShape.setPointCount(3);
					maskShape.setPoint(0, asi);
					maskShape.setPoint(1, bsi);
					maskShape.setPoint(2, intersectionInner);
					maskShape.setFillColor(sf::Color::Black);
					antumbraTempTexture.draw(maskShape);
				}
				else
				{
					sf::ConvexShape maskShape;
					maskShape.setPointCount(4);
					maskShape.setPoint(0, asi);
					maskShape.setPoint(1, bsi);
					maskShape.setPoint(2, bsi + vectorNormalize(bdi) * shadowExtension);
					maskShape.setPoint(3, asi + vectorNormalize(adi) * shadowExtension);
					maskShape.setFillColor(sf::Color::Black);
					antumbraTempTexture.draw(maskShape);
				}

				// Add light back for antumbra/penumbras
				sf::VertexArray vertexArray;
				vertexArray.setPrimitiveType(sf::PrimitiveType::Triangles);
				vertexArray.resize(3);

				sf::RenderStates penumbraRenderStates;
				penumbraRenderStates.blendMode = sf::BlendAdd;
				penumbraRenderStates.shader = &mSystem.getUnshadowShader();

				// Unmask with penumbras
				for (unsigned j = 0; j < penumbras.size(); j++)
				{
					mSystem.getUnshadowShader().setUniform("lightBrightness", penumbras[j]._lightBrightness);
					mSystem.getUnshadowShader().setUniform("darkBrightness", penumbras[j]._darkBrightness);

					vertexArray[0].position = penumbras[j]._source;
					vertexArray[1].position = penumbras[j]._source + vectorNormalize(penumbras[j]._lightEdge) * shadowExtension;
					vertexArray[2].position = penumbras[j]._source + vectorNormalize(penumbras[j]._darkEdge) * shadowExtension;

					vertexArray[0].texCoords = sf::Vector2f(0.0f, 1.0f);
					vertexArray[1].texCoords = sf::Vector2f(1.0f, 0.0f);
					vertexArray[2].texCoords = sf::Vector2f(0.0f, 0.0f);

					antumbraTempTexture.draw(vertexArray, penumbraRenderStates);
				}

				antumbraTempTexture.display();

				// Multiply back to lightTempTexture
				lightTempTexture.setView(lightTempTexture.getDefaultView());
				lightTempTexture.draw(sf::Sprite(antumbraTempTexture.getTexture()), antumbraRenderStates);
				lightTempTexture.setView(view);
			}
			else
			{
				sf::ConvexShape maskShape;
				maskShape.setPointCount(4);
				maskShape.setPoint(0, as);
				maskShape.setPoint(1, bs);
				maskShape.setPoint(2, bs + vectorNormalize(bd) * shadowExtension);
				maskShape.setPoint(3, as + vectorNormalize(ad) * shadowExtension);
				maskShape.setFillColor(sf::Color::Black);
				lightTempTexture.draw(maskShape);

				sf::VertexArray vertexArray;
				vertexArray.setPrimitiveType(sf::PrimitiveType::Triangles);
				vertexArray.resize(3);

				sf::RenderStates penumbraRenderStates;
				penumbraRenderStates.blendMode = sf::BlendMultiply;
				penumbraRenderStates.shader = &mSystem.getUnshadowShader();

				// Unmask with penumbras
				for (unsigned j = 0; j < penumbras.size(); j++)
				{
					mSystem.getUnshadowShader().setUniform("lightBrightness", penumbras[j]._lightBrightness);
					mSystem.getUnshadowShader().setUniform("darkBrightness", penumbras[j]._darkBrightness);

					vertexArray[0].position = penumbras[j]._source;
					vertexArray[1].position = penumbras[j]._source + vectorNormalize(penumbras[j]._lightEdge) * shadowExtension;
					vertexArray[2].position = penumbras[j]._source + vectorNormalize(penumbras[j]._darkEdge) * shadowExtension;

					vertexArray[0].texCoords = sf::Vector2f(0.0f, 1.0f);
					vertexArray[1].texCoords = sf::Vector2f(1.0f, 0.0f);
					vertexArray[2].texCoords = sf::Vector2f(0.0f, 0.0f);

					lightTempTexture.draw(vertexArray, penumbraRenderStates);
				}
			}
		}
    }

    for (unsigned i = 0; i < shapesCount; i++) 
	{
        LightShape* pLightShape = static_cast<LightShape*>(shapes[i]);

        if (pLightShape->renderLightOver()) 
		{
            pLightShape->setFillColor(sf::Color::White);
            lightTempTexture.draw(*pLightShape, &mSystem.getLightOverShapeShader());
        }
        else 
		{
            pLightShape->setFillColor(sf::Color::Black);
            lightTempTexture.draw(*pLightShape);
        }
    }

    //----- Finish

    lightTempTexture.display();
}

void LightPointEmission::setLocalCastCenter(sf::Vector2f const & localCenter)
{
	mLocalCastCenter = localCenter;
}

sf::Vector2f LightPointEmission::getLocalCastCenter() const
{
	return mLocalCastCenter;
}

sf::Vector2f LightPointEmission::getCastCenter() const
{
	sf::Transform t = getTransform();
	t.translate(getOrigin());
	return t.transformPoint(mLocalCastCenter);
}

void LightPointEmission::remove()
{
	mSystem.removeLight(this);
}

void LightPointEmission::setTurnedOn(bool turnedOn)
{
	mTurnedOn = turnedOn;
}

bool LightPointEmission::isTurnedOn() const
{
	return mTurnedOn;
}

void LightPointEmission::toggleTurnedOn()
{
	mTurnedOn = !mTurnedOn;
}

} // namespace lum