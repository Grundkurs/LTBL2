#pragma once

#include "Math.hpp"
#include "Quadtree.hpp"
#include "LightPointEmission.hpp"
#include "LightDirectionEmission.hpp"
#include "LightShape.hpp"
#include "LightResources.hpp"

namespace ltbl
{

class LightSystem : sf::NonCopyable
{
    public:
        struct Penumbra 
		{
            sf::Vector2f _source;
            sf::Vector2f _lightEdge;
            sf::Vector2f _darkEdge;
            float _lightBrightness;
            float _darkBrightness;
            float _distance;
        };

    public:
		LightSystem();

        void create(const sf::FloatRect& rootRegion, const sf::Vector2u& imageSize);

        void render(sf::RenderTarget& target);

		LightShape* createLightShape();
		void removeShape(LightShape* shape);

		LightPointEmission* createLightPointEmission();
		void removeLight(LightPointEmission* light);

		LightDirectionEmission* createLightPointDirection();
		void removeLight(LightDirectionEmission* light);

		void update(sf::Vector2u const& size = sf::Vector2u());

		sf::Texture& getPenumbraTexture();
		sf::Shader& getUnshadowShader();
		sf::Shader& getLightOverShapeShader();

	private:
		static void getPenumbrasPoint(std::vector<Penumbra> &penumbras, std::vector<int> &innerBoundaryIndices, std::vector<sf::Vector2f> &innerBoundaryVectors, std::vector<int> &outerBoundaryIndices, std::vector<sf::Vector2f> &outerBoundaryVectors, const LightShape& shape, const sf::Vector2f &sourceCenter, float sourceRadius);
		static void getPenumbrasDirection(std::vector<Penumbra> &penumbras, std::vector<int> &innerBoundaryIndices, std::vector<sf::Vector2f> &innerBoundaryVectors, std::vector<int> &outerBoundaryIndices, std::vector<sf::Vector2f> &outerBoundaryVectors, const LightShape& shape, const sf::Vector2f &sourceDirection, float sourceRadius, float sourceDistance);

	private:
		sf::Texture mPenumbraTexture;
		sf::Shader mUnshadowShader;
		sf::Shader mLightOverShapeShader;

		Quadtree mLightShapeQuadtree;
		Quadtree mLightPointEmissionQuadtree;

		std::unordered_set<LightPointEmission*> mPointEmissionLights;
		std::unordered_set<LightDirectionEmission*> mDirectionEmissionLights;
		std::unordered_set<LightShape*> mLightShapes;

		sf::RenderTexture mLightTempTexture;
		sf::RenderTexture mEmissionTempTexture;
		sf::RenderTexture mAntumbraTempTexture;
		sf::RenderTexture mCompositionTexture;

		float mDirectionEmissionRange;
		float mDirectionEmissionRadiusMultiplier;
		sf::Color mAmbientColor;

	private:
		friend class LightPointEmission;
		friend class LightDirectionEmission;
		friend class LightShape;
};

} // namespace ltbl
