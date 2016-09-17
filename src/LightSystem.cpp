#include "LightSystem.hpp"

#include <assert.h>
#include <iostream>

namespace ltbl
{

LightSystem::LightSystem()
	: mLightShapeQuadtree(sf::FloatRect())
	, mLightPointEmissionQuadtree(sf::FloatRect())
	, mDirectionEmissionRange(1000.0f)
	, mDirectionEmissionRadiusMultiplier(1.1f)
	, mAmbientColor(sf::Color(16, 16, 16))
{
}

void LightSystem::create(const sf::FloatRect& rootRegion, const sf::Vector2u& imageSize)
{
    mLightShapeQuadtree.create(rootRegion, 10, 5);
    mLightPointEmissionQuadtree.create(rootRegion, 10, 5);

	using namespace resources;

	// Load Texture
	mPenumbraTexture.loadFromMemory(penumbraTexture, (sizeof(penumbraTexture) / sizeof(*penumbraTexture)));
	mPenumbraTexture.setSmooth(true);

	// Load Shaders
	mUnshadowShader.loadFromMemory(unshadowFragment, sf::Shader::Fragment);
	mLightOverShapeShader.loadFromMemory(lightOverShapeFragment, sf::Shader::Fragment);

	update(imageSize);
}

void LightSystem::render(sf::RenderTarget& target)
{
	sf::View view = target.getView();

	if (target.getSize() != mLightTempTexture.getSize())
	{
		update(target.getSize());
	}

	mLightShapeQuadtree.update();
	mLightPointEmissionQuadtree.update();

    mCompositionTexture.clear(mAmbientColor);
    mCompositionTexture.setView(mCompositionTexture.getDefaultView());

	mLightTempTexture.setView(view);

    //----- Point lights

	sf::FloatRect viewBounds = sf::FloatRect(view.getCenter() - view.getSize() * 0.5f, view.getSize());

	std::vector<QuadtreeOccupant*> viewPointEmissionLights;
	mLightPointEmissionQuadtree.query(viewBounds, viewPointEmissionLights);

    std::vector<QuadtreeOccupant*> lightShapes;
    sf::Sprite lightTempSprite(mLightTempTexture.getTexture());

    for (auto occupant : viewPointEmissionLights) 
	{
		LightPointEmission* pPointEmissionLight = static_cast<LightPointEmission*>(occupant);
		if (pPointEmissionLight != nullptr && pPointEmissionLight->isTurnedOn())
		{
			// Query shapes this light is affected by
			lightShapes.clear();
			mLightShapeQuadtree.query(pPointEmissionLight->getAABB(), lightShapes);

			pPointEmissionLight->render(view, mLightTempTexture, mEmissionTempTexture, mAntumbraTempTexture, lightShapes);
			mCompositionTexture.draw(lightTempSprite, sf::BlendAdd);
		}
    }

    //----- Direction lights

	sf::FloatRect centeredViewBounds = rectRecenter(viewBounds, sf::Vector2f(0.0f, 0.0f));
	float maxDim = std::max(centeredViewBounds.width, centeredViewBounds.height);
	float shadowExtension = vectorMagnitude(rectLowerBound(centeredViewBounds)) * mDirectionEmissionRadiusMultiplier;
	sf::Vector2f extendedBounds = sf::Vector2f(maxDim, maxDim) * mDirectionEmissionRadiusMultiplier;
	sf::FloatRect extendedViewBounds = rectFromBounds(-extendedBounds, extendedBounds + sf::Vector2f(mDirectionEmissionRange, 0.0f));

    for (const auto& directionEmissionLight : mDirectionEmissionLights) 
	{
        sf::ConvexShape directionShape = shapeFromRect(extendedViewBounds);
        directionShape.setPosition(view.getCenter());
        directionShape.setRotation(directionEmissionLight->getCastAngle());

        std::vector<QuadtreeOccupant*> viewLightShapes;
        mLightShapeQuadtree.query(directionShape, viewLightShapes);

        directionEmissionLight->render(view, mLightTempTexture, mAntumbraTempTexture, viewLightShapes, shadowExtension);

        mCompositionTexture.draw(sf::Sprite(mLightTempTexture.getTexture()), sf::BlendAdd);
    }

    mCompositionTexture.display();

	target.setView(target.getDefaultView());
	target.draw(sf::Sprite(mCompositionTexture.getTexture()), sf::BlendMultiply);
	target.setView(view);
}

LightShape* LightSystem::createLightShape()
{
	LightShape* shape = new LightShape(*this);
	mLightShapeQuadtree.addOccupant(shape);
	mLightShapes.insert(shape);
	return shape;
}

void LightSystem::removeShape(LightShape* shape)
{
	auto itr = mLightShapes.find(shape);
	if (itr != mLightShapes.end()) 
	{
		mLightShapeQuadtree.removeOccupant(*itr);
		mLightShapes.erase(itr);
		delete shape;
	}
}

LightPointEmission* LightSystem::createLightPointEmission()
{
	LightPointEmission* light = new LightPointEmission(*this);
	mLightPointEmissionQuadtree.addOccupant(light);
	mPointEmissionLights.insert(light);
	return light;
}

void LightSystem::removeLight(LightPointEmission* light)
{
	auto itr = mPointEmissionLights.find(light);
	if (itr != mPointEmissionLights.end())
	{
		mLightPointEmissionQuadtree.removeOccupant(*itr);
		mPointEmissionLights.erase(itr);
		delete light;
	}
}

LightDirectionEmission* LightSystem::createLightPointDirection()
{
	LightDirectionEmission* light = new LightDirectionEmission(*this);
	mDirectionEmissionLights.insert(light);
	return light;
}

void LightSystem::removeLight(LightDirectionEmission* light) 
{
    auto itr = mDirectionEmissionLights.find(light);
	if (itr != mDirectionEmissionLights.end())
	{
		mDirectionEmissionLights.erase(itr);
		delete light;
	}
}

void LightSystem::update(sf::Vector2u const& size)
{
	mUnshadowShader.setUniform("penumbraTexture", mPenumbraTexture);

	if (size.x != 0 && size.y != 0)
	{
		mLightTempTexture.create(size.x, size.y);
		mEmissionTempTexture.create(size.x, size.y);
		mAntumbraTempTexture.create(size.x, size.y);
		mCompositionTexture.create(size.x, size.y);

		mLightOverShapeShader.setUniform("emissionTexture", mEmissionTempTexture.getTexture());
		mLightOverShapeShader.setUniform("targetSizeInv", sf::Vector2f(1.0f / size.x, 1.0f / size.y));
	}
}

sf::Texture& LightSystem::getPenumbraTexture()
{
	return mPenumbraTexture;
}

sf::Shader& LightSystem::getUnshadowShader()
{
	return mUnshadowShader;
}

sf::Shader& LightSystem::getLightOverShapeShader()
{
	return mLightOverShapeShader;
}

void LightSystem::getPenumbrasPoint(std::vector<Penumbra> &penumbras, std::vector<int> &innerBoundaryIndices, std::vector<sf::Vector2f> &innerBoundaryVectors, std::vector<int> &outerBoundaryIndices, std::vector<sf::Vector2f> &outerBoundaryVectors, const LightShape& shape, const sf::Vector2f &sourceCenter, float sourceRadius)
{
	const int numPoints = shape.getPointCount();

	std::vector<bool> bothEdgesBoundaryWindings;
	bothEdgesBoundaryWindings.reserve(2);

	std::vector<bool> oneEdgeBoundaryWindings;
	oneEdgeBoundaryWindings.reserve(2);

	// Calculate front and back facing sides
	std::vector<bool> facingFrontBothEdges;
	facingFrontBothEdges.reserve(numPoints);

	std::vector<bool> facingFrontOneEdge;
	facingFrontOneEdge.reserve(numPoints);

	for (int i = 0; i < numPoints; i++) {
		sf::Vector2f point = shape.getTransform().transformPoint(shape.getPoint(i));

		sf::Vector2f nextPoint;

		if (i < numPoints - 1)
			nextPoint = shape.getTransform().transformPoint(shape.getPoint(i + 1));
		else
			nextPoint = shape.getTransform().transformPoint(shape.getPoint(0));

		sf::Vector2f firstEdgeRay;
		sf::Vector2f secondEdgeRay;
		sf::Vector2f firstNextEdgeRay;
		sf::Vector2f secondNextEdgeRay;

		{
			sf::Vector2f sourceToPoint = point - sourceCenter;

			sf::Vector2f perpendicularOffset(-sourceToPoint.y, sourceToPoint.x);

			perpendicularOffset = vectorNormalize(perpendicularOffset);
			perpendicularOffset *= sourceRadius;

			firstEdgeRay = point - (sourceCenter - perpendicularOffset);
			secondEdgeRay = point - (sourceCenter + perpendicularOffset);
		}

		{
			sf::Vector2f sourceToPoint = nextPoint - sourceCenter;

			sf::Vector2f perpendicularOffset(-sourceToPoint.y, sourceToPoint.x);

			perpendicularOffset = vectorNormalize(perpendicularOffset);
			perpendicularOffset *= sourceRadius;

			firstNextEdgeRay = nextPoint - (sourceCenter - perpendicularOffset);
			secondNextEdgeRay = nextPoint - (sourceCenter + perpendicularOffset);
		}

		sf::Vector2f pointToNextPoint = nextPoint - point;

		sf::Vector2f normal = vectorNormalize(sf::Vector2f(-pointToNextPoint.y, pointToNextPoint.x));

		// Front facing, mark it
		facingFrontBothEdges.push_back((vectorDot(firstEdgeRay, normal) > 0.0f && vectorDot(secondEdgeRay, normal) > 0.0f) || (vectorDot(firstNextEdgeRay, normal) > 0.0f && vectorDot(secondNextEdgeRay, normal) > 0.0f));
		facingFrontOneEdge.push_back((vectorDot(firstEdgeRay, normal) > 0.0f || vectorDot(secondEdgeRay, normal) > 0.0f) || vectorDot(firstNextEdgeRay, normal) > 0.0f || vectorDot(secondNextEdgeRay, normal) > 0.0f);
	}

	// Go through front/back facing list. Where the facing direction switches, there is a boundary
	for (int i = 1; i < numPoints; i++)
		if (facingFrontBothEdges[i] != facingFrontBothEdges[i - 1]) {
			innerBoundaryIndices.push_back(i);
			bothEdgesBoundaryWindings.push_back(facingFrontBothEdges[i]);
		}

	// Check looping indices separately
	if (facingFrontBothEdges[0] != facingFrontBothEdges[numPoints - 1]) {
		innerBoundaryIndices.push_back(0);
		bothEdgesBoundaryWindings.push_back(facingFrontBothEdges[0]);
	}

	// Go through front/back facing list. Where the facing direction switches, there is a boundary
	for (int i = 1; i < numPoints; i++)
		if (facingFrontOneEdge[i] != facingFrontOneEdge[i - 1]) {
			outerBoundaryIndices.push_back(i);
			oneEdgeBoundaryWindings.push_back(facingFrontOneEdge[i]);
		}

	// Check looping indices separately
	if (facingFrontOneEdge[0] != facingFrontOneEdge[numPoints - 1]) {
		outerBoundaryIndices.push_back(0);
		oneEdgeBoundaryWindings.push_back(facingFrontOneEdge[0]);
	}

	// Compute outer boundary vectors
	for (unsigned bi = 0; bi < outerBoundaryIndices.size(); bi++) {
		int penumbraIndex = outerBoundaryIndices[bi];
		bool winding = oneEdgeBoundaryWindings[bi];

		sf::Vector2f point = shape.getTransform().transformPoint(shape.getPoint(penumbraIndex));

		sf::Vector2f sourceToPoint = point - sourceCenter;

		sf::Vector2f perpendicularOffset(-sourceToPoint.y, sourceToPoint.x);

		perpendicularOffset = vectorNormalize(perpendicularOffset);
		perpendicularOffset *= sourceRadius;

		sf::Vector2f firstEdgeRay = point - (sourceCenter + perpendicularOffset);
		sf::Vector2f secondEdgeRay = point - (sourceCenter - perpendicularOffset);

		// Add boundary vector
		outerBoundaryVectors.push_back(winding ? firstEdgeRay : secondEdgeRay);
	}

	for (unsigned bi = 0; bi < innerBoundaryIndices.size(); bi++) {
		int penumbraIndex = innerBoundaryIndices[bi];
		bool winding = bothEdgesBoundaryWindings[bi];

		sf::Vector2f point = shape.getTransform().transformPoint(shape.getPoint(penumbraIndex));

		sf::Vector2f sourceToPoint = point - sourceCenter;

		sf::Vector2f perpendicularOffset(-sourceToPoint.y, sourceToPoint.x);

		perpendicularOffset = vectorNormalize(perpendicularOffset);
		perpendicularOffset *= sourceRadius;

		sf::Vector2f firstEdgeRay = point - (sourceCenter + perpendicularOffset);
		sf::Vector2f secondEdgeRay = point - (sourceCenter - perpendicularOffset);

		// Add boundary vector
		innerBoundaryVectors.push_back(winding ? secondEdgeRay : firstEdgeRay);
		sf::Vector2f outerBoundaryVector = winding ? firstEdgeRay : secondEdgeRay;

		if (innerBoundaryIndices.size() == 1)
			innerBoundaryVectors.push_back(outerBoundaryVector);

		// Add penumbras
		bool hasPrevPenumbra = false;

		sf::Vector2f prevPenumbraLightEdgeVector;

		float prevBrightness = 1.0f;

		int counter = 0;

		while (penumbraIndex != -1) {
			sf::Vector2f nextPoint;
			int nextPointIndex;

			if (penumbraIndex < numPoints - 1) {
				nextPointIndex = penumbraIndex + 1;
				nextPoint = shape.getTransform().transformPoint(shape.getPoint(penumbraIndex + 1));
			}
			else {
				nextPointIndex = 0;
				nextPoint = shape.getTransform().transformPoint(shape.getPoint(0));
			}

			sf::Vector2f pointToNextPoint = nextPoint - point;

			sf::Vector2f prevPoint;
			int prevPointIndex;

			if (penumbraIndex > 0) {
				prevPointIndex = penumbraIndex - 1;
				prevPoint = shape.getTransform().transformPoint(shape.getPoint(penumbraIndex - 1));
			}
			else {
				prevPointIndex = numPoints - 1;
				prevPoint = shape.getTransform().transformPoint(shape.getPoint(numPoints - 1));
			}

			sf::Vector2f pointToPrevPoint = prevPoint - point;

			LightSystem::Penumbra penumbra;

			penumbra._source = point;

			if (!winding) {
				if (hasPrevPenumbra)
					penumbra._lightEdge = prevPenumbraLightEdgeVector;
				else
					penumbra._lightEdge = innerBoundaryVectors.back();

				penumbra._darkEdge = outerBoundaryVector;

				penumbra._lightBrightness = prevBrightness;

				// Next point, check for intersection
				float intersectionAngle = std::acos(vectorDot(vectorNormalize(penumbra._lightEdge), vectorNormalize(pointToNextPoint)));
				float penumbraAngle = std::acos(vectorDot(vectorNormalize(penumbra._lightEdge), vectorNormalize(penumbra._darkEdge)));

				if (intersectionAngle < penumbraAngle) {
					prevBrightness = penumbra._darkBrightness = intersectionAngle / penumbraAngle;

					assert(prevBrightness >= 0.0f && prevBrightness <= 1.0f);

					penumbra._darkEdge = pointToNextPoint;

					penumbraIndex = nextPointIndex;

					if (hasPrevPenumbra) {
						std::swap(penumbra._darkBrightness, penumbras.back()._darkBrightness);
						std::swap(penumbra._lightBrightness, penumbras.back()._lightBrightness);
					}

					hasPrevPenumbra = true;

					prevPenumbraLightEdgeVector = penumbra._darkEdge;

					point = shape.getTransform().transformPoint(shape.getPoint(penumbraIndex));

					sourceToPoint = point - sourceCenter;

					perpendicularOffset = sf::Vector2f(-sourceToPoint.y, sourceToPoint.x);

					perpendicularOffset = vectorNormalize(perpendicularOffset);
					perpendicularOffset *= sourceRadius;

					firstEdgeRay = point - (sourceCenter + perpendicularOffset);
					secondEdgeRay = point - (sourceCenter - perpendicularOffset);

					outerBoundaryVector = secondEdgeRay;

					if (!outerBoundaryVectors.empty()) {
						outerBoundaryVectors[0] = penumbra._darkEdge;
						outerBoundaryIndices[0] = penumbraIndex;
					}
				}
				else {
					penumbra._darkBrightness = 0.0f;

					if (hasPrevPenumbra) {
						std::swap(penumbra._darkBrightness, penumbras.back()._darkBrightness);
						std::swap(penumbra._lightBrightness, penumbras.back()._lightBrightness);
					}

					hasPrevPenumbra = false;

					if (!outerBoundaryVectors.empty()) {
						outerBoundaryVectors[0] = penumbra._darkEdge;
						outerBoundaryIndices[0] = penumbraIndex;
					}

					penumbraIndex = -1;
				}
			}
			else {
				if (hasPrevPenumbra)
					penumbra._lightEdge = prevPenumbraLightEdgeVector;
				else
					penumbra._lightEdge = innerBoundaryVectors.back();

				penumbra._darkEdge = outerBoundaryVector;

				penumbra._lightBrightness = prevBrightness;

				// Next point, check for intersection
				float intersectionAngle = std::acos(vectorDot(vectorNormalize(penumbra._lightEdge), vectorNormalize(pointToPrevPoint)));
				float penumbraAngle = std::acos(vectorDot(vectorNormalize(penumbra._lightEdge), vectorNormalize(penumbra._darkEdge)));

				if (intersectionAngle < penumbraAngle) {
					prevBrightness = penumbra._darkBrightness = intersectionAngle / penumbraAngle;

					assert(prevBrightness >= 0.0f && prevBrightness <= 1.0f);

					penumbra._darkEdge = pointToPrevPoint;

					penumbraIndex = prevPointIndex;

					if (hasPrevPenumbra) {
						std::swap(penumbra._darkBrightness, penumbras.back()._darkBrightness);
						std::swap(penumbra._lightBrightness, penumbras.back()._lightBrightness);
					}

					hasPrevPenumbra = true;

					prevPenumbraLightEdgeVector = penumbra._darkEdge;

					point = shape.getTransform().transformPoint(shape.getPoint(penumbraIndex));

					sourceToPoint = point - sourceCenter;

					perpendicularOffset = sf::Vector2f(-sourceToPoint.y, sourceToPoint.x);

					perpendicularOffset = vectorNormalize(perpendicularOffset);
					perpendicularOffset *= sourceRadius;

					firstEdgeRay = point - (sourceCenter + perpendicularOffset);
					secondEdgeRay = point - (sourceCenter - perpendicularOffset);

					outerBoundaryVector = firstEdgeRay;

					if (!outerBoundaryVectors.empty()) {
						outerBoundaryVectors[1] = penumbra._darkEdge;
						outerBoundaryIndices[1] = penumbraIndex;
					}
				}
				else {
					penumbra._darkBrightness = 0.0f;

					if (hasPrevPenumbra) {
						std::swap(penumbra._darkBrightness, penumbras.back()._darkBrightness);
						std::swap(penumbra._lightBrightness, penumbras.back()._lightBrightness);
					}

					hasPrevPenumbra = false;

					if (!outerBoundaryVectors.empty()) {
						outerBoundaryVectors[1] = penumbra._darkEdge;
						outerBoundaryIndices[1] = penumbraIndex;
					}

					penumbraIndex = -1;
				}
			}

			penumbras.push_back(penumbra);

			counter++;
		}
	}
}

void LightSystem::getPenumbrasDirection(std::vector<Penumbra> &penumbras, std::vector<int> &innerBoundaryIndices, std::vector<sf::Vector2f> &innerBoundaryVectors, std::vector<int> &outerBoundaryIndices, std::vector<sf::Vector2f> &outerBoundaryVectors, const LightShape& shape, const sf::Vector2f &sourceDirection, float sourceRadius, float sourceDistance) {
	const int numPoints = shape.getPointCount();

	innerBoundaryIndices.reserve(2);
	innerBoundaryVectors.reserve(2);
	penumbras.reserve(2);

	std::vector<bool> bothEdgesBoundaryWindings;
	bothEdgesBoundaryWindings.reserve(2);

	// Calculate front and back facing sides
	std::vector<bool> facingFrontBothEdges;
	facingFrontBothEdges.reserve(numPoints);

	std::vector<bool> facingFrontOneEdge;
	facingFrontOneEdge.reserve(numPoints);

	for (int i = 0; i < numPoints; i++) {
		sf::Vector2f point = shape.getTransform().transformPoint(shape.getPoint(i));

		sf::Vector2f nextPoint;

		if (i < numPoints - 1)
			nextPoint = shape.getTransform().transformPoint(shape.getPoint(i + 1));
		else
			nextPoint = shape.getTransform().transformPoint(shape.getPoint(0));

		sf::Vector2f firstEdgeRay;
		sf::Vector2f secondEdgeRay;
		sf::Vector2f firstNextEdgeRay;
		sf::Vector2f secondNextEdgeRay;

		sf::Vector2f perpendicularOffset(-sourceDirection.y, sourceDirection.x);

		perpendicularOffset = vectorNormalize(perpendicularOffset);
		perpendicularOffset *= sourceRadius;

		firstEdgeRay = point - (point - sourceDirection * sourceDistance - perpendicularOffset);
		secondEdgeRay = point - (point - sourceDirection * sourceDistance + perpendicularOffset);

		firstNextEdgeRay = nextPoint - (point - sourceDirection * sourceDistance - perpendicularOffset);
		secondNextEdgeRay = nextPoint - (point - sourceDirection * sourceDistance + perpendicularOffset);

		sf::Vector2f pointToNextPoint = nextPoint - point;

		sf::Vector2f normal = vectorNormalize(sf::Vector2f(-pointToNextPoint.y, pointToNextPoint.x));

		// Front facing, mark it
		facingFrontBothEdges.push_back((vectorDot(firstEdgeRay, normal) > 0.0f && vectorDot(secondEdgeRay, normal) > 0.0f) || (vectorDot(firstNextEdgeRay, normal) > 0.0f && vectorDot(secondNextEdgeRay, normal) > 0.0f));
		facingFrontOneEdge.push_back((vectorDot(firstEdgeRay, normal) > 0.0f || vectorDot(secondEdgeRay, normal) > 0.0f) || vectorDot(firstNextEdgeRay, normal) > 0.0f || vectorDot(secondNextEdgeRay, normal) > 0.0f);
	}

	// Go through front/back facing list. Where the facing direction switches, there is a boundary
	for (int i = 1; i < numPoints; i++)
		if (facingFrontBothEdges[i] != facingFrontBothEdges[i - 1]) {
			innerBoundaryIndices.push_back(i);
			bothEdgesBoundaryWindings.push_back(facingFrontBothEdges[i]);
		}

	// Check looping indices separately
	if (facingFrontBothEdges[0] != facingFrontBothEdges[numPoints - 1]) {
		innerBoundaryIndices.push_back(0);
		bothEdgesBoundaryWindings.push_back(facingFrontBothEdges[0]);
	}

	// Go through front/back facing list. Where the facing direction switches, there is a boundary
	for (int i = 1; i < numPoints; i++)
		if (facingFrontOneEdge[i] != facingFrontOneEdge[i - 1])
			outerBoundaryIndices.push_back(i);

	// Check looping indices separately
	if (facingFrontOneEdge[0] != facingFrontOneEdge[numPoints - 1])
		outerBoundaryIndices.push_back(0);

	for (unsigned bi = 0; bi < innerBoundaryIndices.size(); bi++) {
		int penumbraIndex = innerBoundaryIndices[bi];
		bool winding = bothEdgesBoundaryWindings[bi];

		sf::Vector2f point = shape.getTransform().transformPoint(shape.getPoint(penumbraIndex));

		sf::Vector2f perpendicularOffset(-sourceDirection.y, sourceDirection.x);

		perpendicularOffset = vectorNormalize(perpendicularOffset);
		perpendicularOffset *= sourceRadius;

		sf::Vector2f firstEdgeRay = point - (point - sourceDirection * sourceDistance + perpendicularOffset);
		sf::Vector2f secondEdgeRay = point - (point - sourceDirection * sourceDistance - perpendicularOffset);

		// Add boundary vector
		innerBoundaryVectors.push_back(winding ? secondEdgeRay : firstEdgeRay);
		sf::Vector2f outerBoundaryVector = winding ? firstEdgeRay : secondEdgeRay;

		outerBoundaryVectors.push_back(outerBoundaryVector);

		// Add penumbras
		bool hasPrevPenumbra = false;

		sf::Vector2f prevPenumbraLightEdgeVector;

		float prevBrightness = 1.0f;

		int counter = 0;

		while (penumbraIndex != -1) {
			sf::Vector2f nextPoint;
			int nextPointIndex;

			if (penumbraIndex < numPoints - 1) {
				nextPointIndex = penumbraIndex + 1;
				nextPoint = shape.getTransform().transformPoint(shape.getPoint(penumbraIndex + 1));
			}
			else {
				nextPointIndex = 0;
				nextPoint = shape.getTransform().transformPoint(shape.getPoint(0));
			}

			sf::Vector2f pointToNextPoint = nextPoint - point;

			sf::Vector2f prevPoint;
			int prevPointIndex;

			if (penumbraIndex > 0) {
				prevPointIndex = penumbraIndex - 1;
				prevPoint = shape.getTransform().transformPoint(shape.getPoint(penumbraIndex - 1));
			}
			else {
				prevPointIndex = numPoints - 1;
				prevPoint = shape.getTransform().transformPoint(shape.getPoint(numPoints - 1));
			}

			sf::Vector2f pointToPrevPoint = prevPoint - point;

			LightSystem::Penumbra penumbra;

			penumbra._source = point;

			if (!winding) {
				if (hasPrevPenumbra)
					penumbra._lightEdge = prevPenumbraLightEdgeVector;
				else
					penumbra._lightEdge = innerBoundaryVectors.back();

				penumbra._darkEdge = outerBoundaryVector;

				penumbra._lightBrightness = prevBrightness;

				// Next point, check for intersection
				float intersectionAngle = std::acos(vectorDot(vectorNormalize(penumbra._lightEdge), vectorNormalize(pointToNextPoint)));
				float penumbraAngle = std::acos(vectorDot(vectorNormalize(penumbra._lightEdge), vectorNormalize(penumbra._darkEdge)));

				if (intersectionAngle < penumbraAngle) {
					prevBrightness = penumbra._darkBrightness = intersectionAngle / penumbraAngle;

					assert(prevBrightness >= 0.0f && prevBrightness <= 1.0f);

					penumbra._darkEdge = pointToNextPoint;

					penumbraIndex = nextPointIndex;

					if (hasPrevPenumbra) {
						std::swap(penumbra._darkBrightness, penumbras.back()._darkBrightness);
						std::swap(penumbra._lightBrightness, penumbras.back()._lightBrightness);
					}

					hasPrevPenumbra = true;

					prevPenumbraLightEdgeVector = penumbra._darkEdge;

					point = shape.getTransform().transformPoint(shape.getPoint(penumbraIndex));

					perpendicularOffset = sf::Vector2f(-sourceDirection.y, sourceDirection.x);

					perpendicularOffset = vectorNormalize(perpendicularOffset);
					perpendicularOffset *= sourceRadius;

					firstEdgeRay = point - (point - sourceDirection * sourceDistance + perpendicularOffset);
					secondEdgeRay = point - (point - sourceDirection * sourceDistance - perpendicularOffset);

					outerBoundaryVector = secondEdgeRay;
				}
				else {
					penumbra._darkBrightness = 0.0f;

					if (hasPrevPenumbra) {
						std::swap(penumbra._darkBrightness, penumbras.back()._darkBrightness);
						std::swap(penumbra._lightBrightness, penumbras.back()._lightBrightness);
					}

					hasPrevPenumbra = false;

					penumbraIndex = -1;
				}
			}
			else {
				if (hasPrevPenumbra)
					penumbra._lightEdge = prevPenumbraLightEdgeVector;
				else
					penumbra._lightEdge = innerBoundaryVectors.back();

				penumbra._darkEdge = outerBoundaryVector;

				penumbra._lightBrightness = prevBrightness;

				// Next point, check for intersection
				float intersectionAngle = std::acos(vectorDot(vectorNormalize(penumbra._lightEdge), vectorNormalize(pointToPrevPoint)));
				float penumbraAngle = std::acos(vectorDot(vectorNormalize(penumbra._lightEdge), vectorNormalize(penumbra._darkEdge)));

				if (intersectionAngle < penumbraAngle) {
					prevBrightness = penumbra._darkBrightness = intersectionAngle / penumbraAngle;

					assert(prevBrightness >= 0.0f && prevBrightness <= 1.0f);

					penumbra._darkEdge = pointToPrevPoint;

					penumbraIndex = prevPointIndex;

					if (hasPrevPenumbra) {
						std::swap(penumbra._darkBrightness, penumbras.back()._darkBrightness);
						std::swap(penumbra._lightBrightness, penumbras.back()._lightBrightness);
					}

					hasPrevPenumbra = true;

					prevPenumbraLightEdgeVector = penumbra._darkEdge;

					point = shape.getTransform().transformPoint(shape.getPoint(penumbraIndex));

					perpendicularOffset = sf::Vector2f(-sourceDirection.y, sourceDirection.x);

					perpendicularOffset = vectorNormalize(perpendicularOffset);
					perpendicularOffset *= sourceRadius;

					firstEdgeRay = point - (point - sourceDirection * sourceDistance + perpendicularOffset);
					secondEdgeRay = point - (point - sourceDirection * sourceDistance - perpendicularOffset);

					outerBoundaryVector = firstEdgeRay;
				}
				else {
					penumbra._darkBrightness = 0.0f;

					if (hasPrevPenumbra) {
						std::swap(penumbra._darkBrightness, penumbras.back()._darkBrightness);
						std::swap(penumbra._lightBrightness, penumbras.back()._lightBrightness);
					}

					hasPrevPenumbra = false;

					penumbraIndex = -1;
				}
			}

			penumbras.push_back(penumbra);

			counter++;
		}
	}
}

} // namespace ltbl